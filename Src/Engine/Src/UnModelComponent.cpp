/*=============================================================================
	UnModelComponent.cpp: Model component implementation
	Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

IMPLEMENT_CLASS(UModelComponent);

FModelElement::FModelElement(UModelComponent* InComponent,UMaterialInstance* InMaterial):
	//<@ ava specific; 2006. 12. 01 changmin
	Next(NULL),
	Sibling(NULL),
	//>@ ava
	Component(InComponent),
	Material(InMaterial)
{}

/**
 * Serializer.
 */
FArchive& operator<<(FArchive& Ar,FModelElement& Element)
{
	if(Ar.Ver() < VER_LIGHTMAP_NON_UOBJECT)
	{
		UObject* LightMap = NULL;
		Ar << LightMap;
	}
	else
	{
		Ar << Element.LightMap;
	}
	Ar << *(UObject**)&Element.Component << *(UObject**)&Element.Material << Element.Nodes;
	Ar << Element.ShadowMaps;		
	Ar << Element.IrrelevantLights;
	return Ar;
}

UModelComponent::UModelComponent(UModel* InModel,INT InZoneIndex,WORD InComponentIndex,DWORD MaskedSurfaceFlags,const TArray<WORD>& InNodes):
	Model(InModel),
	ZoneIndex(InZoneIndex),
	ComponentIndex(InComponentIndex),
	Nodes(InNodes)
{
	// Model components are transacted.
	SetFlags( RF_Transactional );

	// Propagate model surface flags to component.
	bForceDirectLightMap	= (MaskedSurfaceFlags & PF_ForceLightMap)			? TRUE : FALSE;
	bAcceptsLights			= (MaskedSurfaceFlags & PF_AcceptsLights)			? TRUE : FALSE;
	bAcceptsDynamicLights	= (MaskedSurfaceFlags & PF_AcceptsDynamicLights)	? TRUE : FALSE;
	
	TMap<UMaterialInstance*,FModelElement*> MaterialToElementMap;

	// Find the BSP nodes which are in this component's zone.
	for(INT NodeIndex = 0;NodeIndex < Nodes.Num();NodeIndex++)
	{
		FBspNode& Node = Model->Nodes(Nodes(NodeIndex));
		FBspSurf& Surf = Model->Surfs(Node.iSurf);

		// Find an element with the same material as this node.
		FModelElement* Element = MaterialToElementMap.FindRef(Surf.Material);
		if(!Element)
		{
			// If there's no matching element, create a new element.
			Element = MaterialToElementMap.Set(Surf.Material,new(Elements) FModelElement(this,Surf.Material));
		}

		// Add the node to the element.
		Element->Nodes.AddItem(Nodes(NodeIndex));
	}
	
	BuildRenderData();
}

//<@ ava specific ; 2006. 11. 17 changmin
UModelComponent::UModelComponent(UModel* InModel,INT InZoneIndex,WORD InComponentIndex,DWORD MaskedSurfaceFlags,const TArray<WORD>& InNodes, INT ClusterNumber ):
Model(InModel),
ZoneIndex(InZoneIndex),
ComponentIndex(InComponentIndex),
Nodes(InNodes),
ClusterNumber( ClusterNumber )
{
	// Propagate model surface flags to component.
	bForceDirectLightMap	= (MaskedSurfaceFlags & PF_ForceLightMap)			? TRUE : FALSE;
	bAcceptsLights			= (MaskedSurfaceFlags & PF_AcceptsLights)			? TRUE : FALSE;
	bAcceptsDynamicLights	= (MaskedSurfaceFlags & PF_AcceptsDynamicLights)	? TRUE : FALSE;

	TMap<UMaterialInstance*,FModelElement*> MaterialToElementMap;

	// Find the BSP nodes which are in this component's zone.
	for(INT NodeIndex = 0;NodeIndex < Nodes.Num();NodeIndex++)
	{
		FBspNode& Node = Model->Nodes(Nodes(NodeIndex));
		FBspSurf& Surf = Model->Surfs(Node.iSurf);

		// Find an element with the same material as this node.
		FModelElement* Element = MaterialToElementMap.FindRef(Surf.Material);
		if(!Element)
		{
			// If there's no matching element, create a new element.
			Element = MaterialToElementMap.Set(Surf.Material,new(Elements) FModelElement(this,Surf.Material));
		}

		// Add the node to the element.
		Element->Nodes.AddItem(Nodes(NodeIndex));
	}

	BuildRenderData();
}
//>@ ava

/**
 * Callback used to allow object register its direct object references that are not already covered by
 * the token stream.
 *
 * @param ObjectArray	array to add referenced objects to via AddReferencedObject
 */
void UModelComponent::AddReferencedObjects( TArray<UObject*>& ObjectArray )
{
	Super::AddReferencedObjects( ObjectArray );
	for( INT ElementIndex=0; ElementIndex<Elements.Num(); ElementIndex++ )
	{
		const FModelElement& Element = Elements(ElementIndex);
		AddReferencedObject( ObjectArray, Element.Component );
		AddReferencedObject( ObjectArray, Element.Material );
		for( INT ShadowMapIndex=0; ShadowMapIndex<Element.ShadowMaps.Num(); ShadowMapIndex++ )
		{
			AddReferencedObject( ObjectArray, Element.ShadowMaps(ShadowMapIndex) );
		}
		if(Element.LightMap != NULL)
		{
			Element.LightMap->AddReferencedObjects(ObjectArray);
		}
	}
}

void UModelComponent::CommitSurfaces()
{
	FComponentReattachContext ReattachContext(this);

	TArray<INT> InvalidElements;

    // Find nodes that are from surfaces which have been invalidated.
	TMap<WORD,FModelElement*> InvalidNodes;
	for(INT ElementIndex = 0;ElementIndex < Elements.Num();ElementIndex++)
	{
		FModelElement& Element = Elements(ElementIndex);
		TArray<WORD> NewNodes;
		for(INT NodeIndex = 0;NodeIndex < Element.Nodes.Num();NodeIndex++)
		{
			FBspNode& Node = Model->Nodes(Element.Nodes(NodeIndex));
			FBspSurf& Surf = Model->Surfs(Node.iSurf);
			if(Surf.Material != Element.Material)
			{
				// This node's material changed, remove it from the element and put it on the invalid node list.
				InvalidNodes.Set(Element.Nodes(NodeIndex),&Element);

				// Mark the node's original element as being invalid.
				InvalidElements.AddUniqueItem(ElementIndex);
			}
			else
			{
				NewNodes.AddItem(Element.Nodes(NodeIndex));
			}
		}
		Element.Nodes = NewNodes;
	}

	// Reassign the invalid nodes to appropriate mesh elements.
	for(TMap<WORD,FModelElement*>::TConstIterator It(InvalidNodes);It;++It)
	{
		FBspNode& Node = Model->Nodes(It.Key());
		FBspSurf& Surf = Model->Surfs(Node.iSurf);
		FModelElement* OldElement = It.Value();

		// Find an element which has the same material and lights as the invalid node.
		FModelElement* NewElement = NULL;
		for(INT ElementIndex = 0;ElementIndex < Elements.Num();ElementIndex++)
		{
			FModelElement& Element = Elements(ElementIndex);
			if(Element.Material != Surf.Material)
				continue;
			if(Element.ShadowMaps != OldElement->ShadowMaps)
				continue;
			if(Element.LightMap != OldElement->LightMap)
				continue;
			if(Element.IrrelevantLights != OldElement->IrrelevantLights)
				continue;
			// This element's material and lights match the node.
			NewElement = &Element;
		}

		// If no matching element was found, create a new element.
		if(!NewElement)
		{
			NewElement = new(Elements) FModelElement(this,Surf.Material);
			NewElement->ShadowMaps = OldElement->ShadowMaps;
			NewElement->LightMap = OldElement->LightMap;
			NewElement->IrrelevantLights = OldElement->IrrelevantLights;
		}

		NewElement->Nodes.AddItem(It.Key());
		InvalidElements.AddUniqueItem(Elements.Num() - 1);
	}

	// Rebuild the render data for the elements which have changed.
	BuildRenderData();

	ShrinkElements();
}

void UModelComponent::ShrinkElements()
{
	// Find elements which have no nodes remaining.
	for(INT ElementIndex = 0;ElementIndex < Elements.Num();ElementIndex++)
	{
		FModelElement& Element = Elements(ElementIndex);
		if(!Element.Nodes.Num())
		{
			// This element contains no nodes, remove it.
			Elements.Remove(ElementIndex--);
		}
	}
}

void UModelComponent::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar << Model;
	Ar << ZoneIndex << Elements;
	Ar << ComponentIndex << Nodes;

	//<@ ava specific ; 2006. 11. 17 changmin
	if( Ar.LicenseeVer() >= VER_AVA_ADD_CLUSTERNUMBER_TO_FBSPNODE )
	{
		Ar << ClusterNumber;
	}

	//>@ ava
}

void UModelComponent::PostLoad()
{
	Super::PostLoad();

	// Fix for old model components which weren't created with transactional flag.
	SetFlags( RF_Transactional );

	// BuildRenderData relies on the model having been post-loaded, so we ensure this by calling ConditionalPostLoad.
	check(Model);
	Model->ConditionalPostLoad();

	if (!GIsUCC)
	{
		BuildRenderData();
	}
}

void UModelComponent::PostEditUndo()
{
	// Rebuild the component's render data after applying a transaction to it.
	BuildRenderData();

	Super::PostEditUndo();
}

/**
 * Selects all surfaces that are part of this model component.
 */
void UModelComponent::SelectAllSurfaces()
{
	check(Model);
	for( INT NodeIndex=0; NodeIndex<Nodes.Num(); NodeIndex++ )
	{
		FBspNode& Node = Model->Nodes(Nodes(NodeIndex));
		FBspSurf& Surf = Model->Surfs(Node.iSurf);
		Model->ModifySurf( Node.iSurf, 0 );
		Surf.PolyFlags |= PF_Selected;
	}
}

//<@ ava specific ; 2006. 12. 05 changmin
void UModelComponent::UpdateLightmapTextureCoordinate()
{
	for(INT ElementIndex = 0;ElementIndex < Elements.Num();ElementIndex++)
	{
		FModelElement& Element = Elements(ElementIndex);
		FLightMap2D *LightMap2D = Element.LightMap ? const_cast<FLightMap2D*>(Element.LightMap->GetLightMap2D()) : NULL;
		if( LightMap2D )
		{
			FVector2D CoordinateScale	= LightMap2D->GetCoordinateScale();
			FVector2D CoordinateBias	= LightMap2D->GetCoordinateBias();

			for( INT NodeIndex = 0; NodeIndex < Element.Nodes.Num(); ++NodeIndex )
			{
				FBspNode& Node = Model->Nodes(Element.Nodes(NodeIndex));
				FBspSurf& Surf = Model->Surfs(Node.iSurf);
		
				UBOOL IsFrontVisible = !(Surf.PolyFlags & PF_TwoSided) || Node.iZone[1] == ZoneIndex || ZoneIndex == INDEX_NONE;
				UBOOL IsBackVisible = (Surf.PolyFlags & PF_TwoSided) && (Node.iZone[0] == ZoneIndex || ZoneIndex == INDEX_NONE);

				if(IsFrontVisible)
				{
					for(INT VertexIndex = 0;VertexIndex < Node.NumVertices;VertexIndex++)
					{
						FVert& Vert = Model->Verts(Node.iVertPool + VertexIndex);
						Vert.ShadowTexCoord *= CoordinateScale;
						Vert.ShadowTexCoord += CoordinateBias;
					}
				}
				if(IsBackVisible)
				{
					for(INT VertexIndex = 0;VertexIndex < Node.NumVertices;VertexIndex++)
					{
						FVert& Vert = Model->Verts(Node.iVertPool + VertexIndex);
						Vert.BackfaceShadowTexCoord *= CoordinateScale;
						Vert.BackfaceShadowTexCoord += CoordinateBias;
					}
				}
			}
		}
	}
}

void UModelComponent::ResetLightmapTextureCoordinateScaleAndBias()
{
	for(INT ElementIndex = 0;ElementIndex < Elements.Num();ElementIndex++)
	{
		FModelElement& Element = Elements(ElementIndex);
		FLightMap2D *LightMap2D = Element.LightMap ? const_cast<FLightMap2D*>(Element.LightMap->GetLightMap2D()) : NULL;
		if( LightMap2D )
		{
			LightMap2D->ResetCoordinateScaleAndBias();
		}
	}
}
//>@ ava
