/**
*	LensFlareRendering.cpp: LensFlare rendering functionality.
*	Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved.
*/

#include "EnginePrivate.h"
#include "EngineMaterialClasses.h"
#include "LensFlare.h"
#include "TileRendering.h"
#include "ScenePrivate.h"

/** Sorting helper */
IMPLEMENT_COMPARE_CONSTREF(FLensFlareElementOrder,LensFlareRendering,{ return A.RayDistance > B.RayDistance ? 1 : -1; });

/**
*	FLensFlareDynamicData
*/
FLensFlareDynamicData::FLensFlareDynamicData(const ULensFlareComponent* InLensFlareComp, FLensFlareSceneProxy* InProxy) :
VertexData(NULL)
{
	SourceElement = NULL;
	appMemzero(&Reflections, sizeof(TArrayNoInit<FLensFlareElement*>));

	if (InLensFlareComp && InLensFlareComp->Template)
	{
		ULensFlare* LensFlare = InLensFlareComp->Template;
		check(LensFlare);
		SourceElement = &(LensFlare->SourceElement);
		Reflections.AddZeroed(LensFlare->Reflections.Num());
		for (INT ElementIndex = 0; ElementIndex < LensFlare->Reflections.Num(); ElementIndex++)
		{
			Reflections(ElementIndex) = &(LensFlare->Reflections(ElementIndex));
		}

		INT ElementCount = 1 + LensFlare->Reflections.Num();
		VertexData = new FLensFlareVertex[ElementCount * 4];
	}

	SortElements();

	// Create the vertex factory for rendering the lens flares
	VertexFactory = new FLensFlareVertexFactory();
}

FLensFlareDynamicData::~FLensFlareDynamicData()
{
	// Clean up
	delete [] VertexData;
	delete VertexFactory;
	Reflections.Empty();
}

DWORD FLensFlareDynamicData::GetMemoryFootprint( void ) const
{
	DWORD CalcSize = 
		sizeof(this) + 
		sizeof(FLensFlareElement) +							// Source
		sizeof(FLensFlareElement) * Reflections.Num() +		// Reflections
		sizeof(FLensFlareVertex) * (1 + Reflections.Num());	// VertexData

	return CalcSize;
}

void FLensFlareDynamicData::InitializeRenderResources(const ULensFlareComponent* InLensFlareComp, FLensFlareSceneProxy* InProxy)
{
	if (VertexFactory)
	{
		BeginInitResource(VertexFactory);
	}
}

void FLensFlareDynamicData::RenderThread_InitializeRenderResources(FLensFlareSceneProxy* InProxy)
{
	if (VertexFactory && (VertexFactory->IsInitialized() == FALSE))
	{
		VertexFactory->Init();
	}
}

void FLensFlareDynamicData::ReleaseRenderResources(const ULensFlareComponent* InLensFlareComp, FLensFlareSceneProxy* InProxy)
{
	if (VertexFactory)
	{
		BeginReleaseResource(VertexFactory);
	}
}

void FLensFlareDynamicData::RenderThread_ReleaseRenderResources()
{
	if (VertexFactory)
	{
		VertexFactory->Release();
	}
}

/**
*	Render thread only draw call
*	
*	@param	Proxy		The scene proxy for the lens flare
*	@param	PDI			The PrimitiveDrawInterface
*	@param	View		The SceneView that is being rendered
*	@param	DPGIndex	The DrawPrimitiveGroup being rendered
*	@param	Flags		Rendering flags
*/
void FLensFlareDynamicData::Render(FLensFlareSceneProxy* Proxy, FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex, DWORD Flags)
{
	//@todo. Fill in or remove...
}

/** Render the source element. */
void FLensFlareDynamicData::RenderSource(FLensFlareSceneProxy* Proxy, FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex, DWORD Flags)
{
	// Determine the position of the source
	FVector	SourcePosition(0.0f);
	SourcePosition = Proxy->GetLocalToWorld().TransformFVector(SourcePosition);

	FVector CameraUp	= -View->InvViewProjectionMatrix.TransformNormal(FVector(1.0f,0.0f,0.0f)).SafeNormal();
	FVector CameraRight	= -View->InvViewProjectionMatrix.TransformNormal(FVector(0.0f,1.0f,0.0f)).SafeNormal();

}

/** Render the reflection elements. */
void FLensFlareDynamicData::RenderReflections(FLensFlareSceneProxy* Proxy, FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex, DWORD Flags)
{
	// Determine the position of the source
	FVector	WorldPosition = Proxy->GetLocalToWorld().GetOrigin();
	FVector ScreenPosition = View->WorldToScreen(WorldPosition);
	FVector2D PixelPosition;
	View->ScreenToPixel(ScreenPosition, PixelPosition);

#ifdef WITH_LENSFLARE_DEBUG
	debugf(TEXT("LensFlare: WP = %8.5f,%8.5f,%8.5f - ScreenPosition = %8.5f,%8.5f,%8.5f - Pixel = %f,%f"), 
		WorldPosition.X, WorldPosition.Y, WorldPosition.Z,
		ScreenPosition.X, ScreenPosition.Y, ScreenPosition.Z,
		PixelPosition.X, PixelPosition.Y);
#endif	//#ifdef WITH_LENSFLARE_DEBUG

	FPlane StartTemp = View->Project(WorldPosition);
#ifdef WITH_LENSFLARE_DEBUG
	debugf(TEXT("LensFlare: StartTemp = %8.5f,%8.5f,%8.5f"), StartTemp.X, StartTemp.Y, StartTempZ);
#endif	//#ifdef WITH_LENSFLARE_DEBUG

	// Draw a line reflected about the center
	if (Proxy->GetRenderDebug() == TRUE)
	{	
		DrawWireStar(PDI, WorldPosition, 25.0f, FColor(255,0,0), SDPG_Foreground);
	}

	FVector StartLine(StartTemp.X, StartTemp.Y, 0.0f);
	FPlane EndTemp(-StartTemp.X, -StartTemp.Y, StartTemp.Z, 1.0f);
	FVector4 EndLine = View->InvViewProjectionMatrix.TransformFVector4(EndTemp);
	EndLine.X /= EndLine.W;
	EndLine.Y /= EndLine.W;
	EndLine.Z /= EndLine.W;
	EndLine.W = 1.0f;
	FVector2D StartPos = FVector2D(StartLine);

	if (Proxy->GetRenderDebug() == TRUE)
	{	
		PDI->DrawLine(WorldPosition, EndLine, FLinearColor(1.0f, 1.0f, 0.0f), SDPG_Foreground);
		DrawWireStar(PDI, EndLine, 25.0f, FColor(0,255,0), SDPG_Foreground);
	}

	FTileRenderer TileRenderer;
	// draw a full-view tile (the TileRenderer uses SizeX, not RenderTargetSizeX)
	check(View->SizeX == View->RenderTargetSizeX);

	FLOAT LocalAspectRatio = View->SizeX / View->SizeY;

	const FMaterialInstance* MaterialInstance;
	FVector2D DrawSize;
	FLensFlareElementValues LookupValues;

	FMeshElement Mesh;
	Mesh.UseDynamicData			= TRUE;
	Mesh.IndexBuffer			= NULL;
	Mesh.VertexFactory			= VertexFactory;
	Mesh.DynamicVertexStride	= sizeof(FLensFlareVertex);
	Mesh.DynamicIndexData		= NULL;
	Mesh.DynamicIndexStride		= 0;
	Mesh.LCI					= NULL;
	Mesh.LocalToWorld			= FMatrix::Identity;//Proxy->GetLocalToWorld();
	Mesh.WorldToLocal			= FMatrix::Identity;//Proxy->GetWorldToLocal();	
	Mesh.FirstIndex				= 0;
	Mesh.MinVertexIndex			= 0;
	Mesh.MaxVertexIndex			= 3;
	Mesh.ParticleType			= PET_None;
	Mesh.ReverseCulling			= Proxy->GetLocalToWorldDeterminant() < 0.0f ? TRUE : FALSE;
	Mesh.CastShadow				= Proxy->GetCastShadow();
	Mesh.DepthPriorityGroup		= (ESceneDepthPriorityGroup)DPGIndex;
	Mesh.NumPrimitives			= 2;
	Mesh.Type					= PT_TriangleFan;

	for (INT ElementIndex = 0; ElementIndex < ElementOrder.Num(); ElementIndex++)
	{
		FLensFlareElementOrder& ElementOrderEntry = ElementOrder(ElementIndex);

		FLensFlareElement* Element;
		FVector ElementPosition;
		FLensFlareVertex* Vertices;

		if (ElementOrderEntry.ElementIndex >= 0)
		{
			Vertices = &(VertexData[4 + ElementOrderEntry.ElementIndex* 4]);
			Element = Reflections(ElementOrderEntry.ElementIndex);
			if (Element)
			{
				ElementPosition = FVector(
					((StartPos * (1.0f - Element->RayDistance)) +
					(-StartPos * Element->RayDistance)), 0.0f
					);
			}
		}
		else
		{
			Vertices = &(VertexData[0]);
			Element = SourceElement;
			ElementPosition = StartLine;
		}

		if (Element)
		{
			GetElementValues(ElementPosition, StartLine, View, Element, LookupValues);

			if (LookupValues.LFMaterial)
			{
				MaterialInstance = LookupValues.LFMaterial->GetInstanceInterface(Proxy->GetSelected());
				DrawSize = FVector2D(Element->Size) * LookupValues.Scaling;

				FVector4 ElementProjection = FVector4(ElementPosition.X, ElementPosition.Y, 0.1f, 1.0f);
				FPlane ElementPosition = View->InvViewProjectionMatrix.TransformFVector4(ElementProjection);
				FVector SpritePosition = FVector(ElementPosition.X / ElementPosition.W,
					ElementPosition.Y / ElementPosition.W,
					ElementPosition.Z / ElementPosition.W);
				DrawSize /= ElementPosition.W;

				static UBOOL s_bScaleRotBy360 = FALSE;
				if (s_bScaleRotBy360 == TRUE)
				{
					//				LookupValues.Rotation /= 360.0f;
					LookupValues.Rotation *= (2 * PI);
				}

				Vertices[0].Position = SpritePosition;
				Vertices[0].OldPosition = SpritePosition;
				Vertices[0].Size.X = DrawSize.X;
				Vertices[0].Size.Y = DrawSize.Y;
				Vertices[0].Size.Z = LookupValues.AxisScaling.X;
				Vertices[0].Size.W = LookupValues.AxisScaling.Y;
				Vertices[0].Rotation = LookupValues.Rotation;
				Vertices[0].TexCoord = FVector2D(0.0f, 0.0f);
				Vertices[0].Color = LookupValues.Color;
				Vertices[0].RadialDist_SourceRatio.X = LookupValues.RadialDistance;
				Vertices[0].RadialDist_SourceRatio.Y = LookupValues.SourceDistance;
				Vertices[0].RadialDist_SourceRatio.Z = Element->RayDistance;
				Vertices[0].RadialDist_SourceRatio.W = 0.0f;

				Vertices[1].Position = SpritePosition;
				Vertices[1].OldPosition = SpritePosition;
				Vertices[1].Size.X = DrawSize.X;
				Vertices[1].Size.Y = DrawSize.Y;
				Vertices[1].Size.Z = LookupValues.AxisScaling.X;
				Vertices[1].Size.W = LookupValues.AxisScaling.Y;
				Vertices[1].Rotation = LookupValues.Rotation;
				Vertices[1].TexCoord = FVector2D(0.0f, 1.0f);
				Vertices[1].Color = LookupValues.Color;
				Vertices[1].RadialDist_SourceRatio.X = LookupValues.RadialDistance;
				Vertices[1].RadialDist_SourceRatio.Y = LookupValues.SourceDistance;
				Vertices[1].RadialDist_SourceRatio.Z = Element->RayDistance;
				Vertices[1].RadialDist_SourceRatio.W = 0.0f;

				Vertices[2].Position = SpritePosition;
				Vertices[2].OldPosition = SpritePosition;
				Vertices[2].Size.X = DrawSize.X;
				Vertices[2].Size.Y = DrawSize.Y;
				Vertices[2].Size.Z = LookupValues.AxisScaling.X;
				Vertices[2].Size.W = LookupValues.AxisScaling.Y;
				Vertices[2].Rotation = LookupValues.Rotation;
				Vertices[2].TexCoord = FVector2D(1.0f, 1.0f);
				Vertices[2].Color = LookupValues.Color;
				Vertices[2].RadialDist_SourceRatio.X = LookupValues.RadialDistance;
				Vertices[2].RadialDist_SourceRatio.Y = LookupValues.SourceDistance;
				Vertices[2].RadialDist_SourceRatio.Z = Element->RayDistance;
				Vertices[2].RadialDist_SourceRatio.W = 0.0f;

				Vertices[3].Position = SpritePosition;
				Vertices[3].OldPosition = SpritePosition;
				Vertices[3].Size.X = DrawSize.X;
				Vertices[3].Size.Y = DrawSize.Y;
				Vertices[3].Size.Z = LookupValues.AxisScaling.X;
				Vertices[3].Size.W = LookupValues.AxisScaling.Y;
				Vertices[3].Rotation = LookupValues.Rotation;
				Vertices[3].TexCoord = FVector2D(1.0f, 0.0f);
				Vertices[3].Color = LookupValues.Color;
				Vertices[3].RadialDist_SourceRatio.X = LookupValues.RadialDistance;
				Vertices[3].RadialDist_SourceRatio.Y = LookupValues.SourceDistance;
				Vertices[3].RadialDist_SourceRatio.Z = Element->RayDistance;
				Vertices[3].RadialDist_SourceRatio.W = 0.0f;

				Mesh.DynamicVertexData		= Vertices;
				Mesh.DepthPriorityGroup		= (ESceneDepthPriorityGroup)DPGIndex;
				Mesh.MaterialInstance	= MaterialInstance;

				DrawRichMesh(
					PDI, 
					Mesh, 
					FLinearColor(1.0f, 0.0f, 0.0f),	//WireframeColor,
					FLinearColor(1.0f, 1.0f, 0.0f),	//LevelColor,
					FLinearColor(1.0f, 1.0f, 1.0f),	//PropertyColor,		
					Proxy->GetPrimitiveSceneInfo(),
					Proxy->GetSelected()
					);
			}
		}
	}
}

/**
*	Retrieve the various values for the given element
*
*	@param	ScreenPosition		The position of the element in screen space
*	@param	SourcePosition		The position of the source in screen space
*	@param	View				The SceneView being rendered
*	@param	Element				The element of interest
*	@param	Values				The values to fill in
*
*	@return	UBOOL				TRUE if successful
*/
UBOOL FLensFlareDynamicData::GetElementValues(FVector ScreenPosition, FVector SourcePosition, const FSceneView* View, FLensFlareElement* Element, FLensFlareElementValues& Values)
{
	check(Element);

	// The lookup value will be in the range of [0..~1.4] with 
	// 0.0 being right on top of the source (looking directly at the source)
	// 1.0 being on the opposite edge of the screen horizontally or verically
	// 1.4 being on the opposite edge of the screen diagonally
	FLOAT LookupValue;

	Values.RadialDistance = FVector2D(ScreenPosition.X, ScreenPosition.Y).Size();
	FVector2D SourceTemp = FVector2D(SourcePosition.X - ScreenPosition.X, SourcePosition.Y - ScreenPosition.Y);
	SourceTemp /= 2.0f;
	Values.SourceDistance = SourceTemp.Size();
	if (Element->bUseSourceDistance)
	{
		LookupValue = Values.SourceDistance;
	}
	else
	{
		LookupValue = Values.RadialDistance;
	}

#if defined(_LENSFLARE_DEBUG_LOOKUP_)
	debugf(TEXT("LookupValue = %8.5f (Method = %d) for %s"),
		LookupValue, Element->bUseSourceDistance ? 0 : 1, *(Element->ElementName.ToString()));
#endif	//#if defined(_LENSFLARE_DEBUG_LOOKUP_)

	INT MaterialIndex = appTrunc(Element->LFMaterialIndex.GetValue(LookupValue));
	if ((MaterialIndex >= 0) && (MaterialIndex < Element->LFMaterials.Num()))
	{
		Values.LFMaterial = Element->LFMaterials(MaterialIndex);
	}
	else
	{
		Values.LFMaterial = Element->LFMaterials(0);
	}

	Values.Scaling = Element->Scaling.GetValue(LookupValue);
	Values.AxisScaling = Element->AxisScaling.GetValue(LookupValue);
	Values.Rotation = Element->Rotation.GetValue(LookupValue);
	FVector Color = Element->Color.GetValue(LookupValue);
	FLOAT Alpha = Element->Alpha.GetValue(LookupValue);
	Values.Color = FLinearColor(Color.X, Color.Y, Color.Z, Alpha);
	Values.Offset = Element->Offset.GetValue(LookupValue);

	return FALSE;
}

/**
*	Sort the contained elements along the ray
*/
void FLensFlareDynamicData::SortElements()
{
	ElementOrder.Empty();
	// Put the source in first...
	if (SourceElement && (SourceElement->LFMaterials.Num() > 0) && SourceElement->LFMaterials(0))
	{
		new(ElementOrder)FLensFlareElementOrder(-1, SourceElement->RayDistance);
	}

	for (INT ElementIndex = 0; ElementIndex < Reflections.Num(); ElementIndex++)
	{
		FLensFlareElement* Element = Reflections(ElementIndex);
		if (Element && (Element->LFMaterials.Num() > 0))
		{
			new(ElementOrder)FLensFlareElementOrder(ElementIndex, Element->RayDistance);
		}
	}

	Sort<USE_COMPARE_CONSTREF(FLensFlareElementOrder,LensFlareRendering)>(&(ElementOrder(0)),ElementOrder.Num());
}

//
//	Scene Proxies
//
/** Initialization constructor. */
FLensFlareSceneProxy::FLensFlareSceneProxy(const ULensFlareComponent* Component) :
FPrimitiveSceneProxy(Component)
, Owner(Component->GetOwner())
, bSelected(Component->IsOwnerSelected())
, bCastShadow(Component->CastShadow)
, bHasTranslucency(Component->HasUnlitTranslucency())
, bHasDistortion(Component->HasUnlitDistortion())
, bUsesSceneColor(bHasTranslucency && Component->UsesSceneColor())
, DynamicData(NULL)
, LastDynamicData(NULL)
{
	if (Component->Template == NULL)
	{
		return;
	}

	SourceDPG = Component->Template->SourceDPG;
	ReflectionsDPG = Component->Template->ReflectionsDPG;
	//	DepthPriorityGroup = ReflectionsDPG;

	bRenderDebug = Component->Template->bRenderDebugLines;

	// Make a local copy of the template elements...
	DynamicData = new FLensFlareDynamicData(Component, this);
	check(DynamicData);
	//	DynamicData->InitializeRenderResources(Component, this);
	if (DynamicData)
	{
		if (IsInGameThread())
		{
			DynamicData->InitializeRenderResources(NULL, this);
		}
		else
		{
			DynamicData->RenderThread_InitializeRenderResources(this);
		}
	}
}

FLensFlareSceneProxy::~FLensFlareSceneProxy()
{
	if (DynamicData)
	{
		if (IsInGameThread())
		{
			DynamicData->ReleaseRenderResources(NULL, this);
			FlushRenderingCommands();
		}
		else
		{
			DynamicData->RenderThread_ReleaseRenderResources();
		}
	}
	delete DynamicData;
}

/**
* Draws the primitive's static elements.  This is called from the game thread once when the scene proxy is created.
* The static elements will only be rendered if GetViewRelevance declares static relevance.
* Called in the game thread.
* @param PDI - The interface which receives the primitive elements.
*/
void FLensFlareSceneProxy::DrawStaticElements(FStaticPrimitiveDrawInterface* PDI)
{

}

/** 
* Draw the scene proxy as a dynamic element
*
* @param	PDI - draw interface to render to
* @param	View - current view
* @param	DPGIndex - current depth priority 
* @param	Flags - optional set of flags from EDrawDynamicElementFlags
*/
void FLensFlareSceneProxy::DrawDynamicElements(FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex)
{
	DWORD Flags = 0;

	if (View->Family->ShowFlags & SHOW_LensFlares)
	{
		if (DynamicData != NULL)
		{
			if (DPGIndex == SourceDPG)
			{
				// Render the source in the dynamic data
				DynamicData->RenderSource(this, PDI, View, DPGIndex, Flags);
			}

			if (DPGIndex == ReflectionsDPG)
			{
				//if ((Flags & DontAllowStaticMeshElementData) == 0)
				{
					// Render each reflection in the dynamic data
					DynamicData->RenderReflections(this, PDI, View, DPGIndex, Flags);
				}
			}
		}

		if ((DPGIndex == SDPG_Foreground) && (View->Family->ShowFlags & SHOW_Bounds) && (GIsGame_RenderThread || !Owner || Owner->IsSelected()))
		{
			// Draw the static mesh's bounding box and sphere.
			DrawWireBox(PDI,PrimitiveSceneInfo->Bounds.GetBox(), FColor(72,72,255),SDPG_Foreground);
			DrawCircle(PDI,PrimitiveSceneInfo->Bounds.Origin,FVector(1,0,0),FVector(0,1,0),FColor(255,255,0),PrimitiveSceneInfo->Bounds.SphereRadius,32,SDPG_Foreground);
			DrawCircle(PDI,PrimitiveSceneInfo->Bounds.Origin,FVector(1,0,0),FVector(0,0,1),FColor(255,255,0),PrimitiveSceneInfo->Bounds.SphereRadius,32,SDPG_Foreground);
			DrawCircle(PDI,PrimitiveSceneInfo->Bounds.Origin,FVector(0,1,0),FVector(0,0,1),FColor(255,255,0),PrimitiveSceneInfo->Bounds.SphereRadius,32,SDPG_Foreground);
		}
	}
}

void FLensFlareSceneProxy::DrawShadowVolumes(FShadowVolumeDrawInterface* SVDI,const FSceneView* View,const class FLightSceneInfo* Light)
{

}

FPrimitiveViewRelevance FLensFlareSceneProxy::GetViewRelevance(const FSceneView* View)
{
	FPrimitiveViewRelevance Result;
	const EShowFlags ShowFlags = View->Family->ShowFlags;
	if (IsShown(View) && (ShowFlags & SHOW_LensFlares))
	{
		Result.bDynamicRelevance = TRUE;
//		Result.bStaticRelevance = TRUE;
		Result.bStaticRelevance = FALSE;
		//Result.bUsesDynamicMeshElementData = TRUE;

		Result.SetDPG(SourceDPG, TRUE);
		Result.SetDPG(ReflectionsDPG, TRUE);

		if (!(View->Family->ShowFlags & SHOW_Wireframe) && (View->Family->ShowFlags & SHOW_Materials))
		{
			Result.bTranslucentRelevance = bHasTranslucency;
			Result.bDistortionRelevance = bHasDistortion;
			Result.bUsesSceneColor = bUsesSceneColor;
		}
		if (View->Family->ShowFlags & SHOW_Bounds)
		{
			Result.SetDPG(SDPG_Foreground,TRUE);
		}
	}

	// Lens flares never cast shadows...*
	Result.bShadowRelevance = FALSE;

	return Result;
}

/**
*	Called when the rendering thread adds the proxy to the scene.
*	This function allows for generating renderer-side resources.
*/
UBOOL FLensFlareSceneProxy::CreateRenderThreadResources()
{
	return TRUE;
}

/**
*	Called when the rendering thread removes the dynamic data from the scene.
*/
UBOOL FLensFlareSceneProxy::ReleaseRenderThreadResources()
{
	return TRUE;
}

void FLensFlareSceneProxy::UpdateData()
{

}

void FLensFlareSceneProxy::UpdateData_RenderThread()
{

}
