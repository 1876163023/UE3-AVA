/*=============================================================================
	UnModel.cpp: Unreal model functions
	Copyright 1997-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

/*-----------------------------------------------------------------------------
	Struct serializers.
-----------------------------------------------------------------------------*/

FArchive& operator<<( FArchive& Ar, FBspSurf& Surf )
{
	Ar << Surf.Material;
	Ar << Surf.PolyFlags;	
	if( Ar.Ver() < VER_DEFAULTPOLYFLAGS_CHANGE )
	{
		Surf.PolyFlags |= PF_DefaultFlags;
	}
	Ar << Surf.pBase << Surf.vNormal;
	Ar << Surf.vTextureU << Surf.vTextureV;
	Ar << Surf.iBrushPoly;
	Ar << Surf.Actor;
	Ar << Surf.Plane;
	Ar << Surf.ShadowMapScale;

	return Ar;
}

FArchive& operator<<( FArchive& Ar, FPoly& Poly )
{
	INT LegacyNumVertices = Poly.Vertices.Num();
	if(Ar.Ver() < VER_FPOLYVERTEXARRAY)
	{
		Ar << LegacyNumVertices;
	}
	Ar << Poly.Base << Poly.Normal << Poly.TextureU << Poly.TextureV;
	if(Ar.Ver() < VER_FPOLYVERTEXARRAY)
	{
		if(Ar.IsLoading())
		{
			Poly.Vertices.Empty(LegacyNumVertices);
			Poly.Vertices.Add(LegacyNumVertices);
		}
		for( INT i=0; i<LegacyNumVertices; i++ )
		{	
			Ar << Poly.Vertices(i);
		}
	}
	else
	{
		Ar << Poly.Vertices;
	}
	Ar << Poly.PolyFlags;
	if( Ar.Ver() < VER_DEFAULTPOLYFLAGS_CHANGE )
	{
		Poly.PolyFlags |= PF_DefaultFlags;
	}
	Ar << Poly.Actor << Poly.ItemName;
	Ar << Poly.Material;
	Ar << Poly.iLink << Poly.iBrushPoly;
	Ar << Poly.ShadowMapScale;

	return Ar;
}

FArchive& operator<<( FArchive& Ar, FBspNode& N )
{
	// @warning BulkSerialize: FBSPNode is serialized as memory dump
	// See TArray::BulkSerialize for detailed description of implied limitations.

	// Serialize in the order of variable declaration so the data is compatible with BulkSerialize
	Ar	<< N.Plane 
		<< N.ZoneMask
		<< N.iVertPool
		<< N.iSurf
		<< N.iVertexIndex
		<< N.ComponentIndex 
		<< N.ComponentNodeIndex
		<< N.iChild[0]
		<< N.iChild[1]
		<< N.iChild[2]
		<< N.iCollisionBound
		<< N.iZone[0]
		<< N.iZone[1]
		<< N.NumVertices
		<< N.NodeFlags
		<< N.iLeaf[0]
		<< N.iLeaf[1]
		//<@ ava specific ; 2006. 11. 17
		<< N.iCluster;
		//>@ ava

	if( Ar.IsLoading() )
	{
		//@warning: this code needs to be in sync with UModel::Serialize as we use bulk serialization.
		N.NodeFlags &= ~(NF_IsNew|NF_IsFront|NF_IsBack);
	}

	return Ar;
}

//<@ ava specific ; 206. 11. 16 changmin
FArchive& operator<<( FArchive& Ar, FLegacyBspNode& N )
{
	// @warning BulkSerialize: FBSPNode is serialized as memory dump
	// See TArray::BulkSerialize for detailed description of implied limitations.

	// Serialize in the order of variable declaration so the data is compatible with BulkSerialize
	Ar	<< N.Plane 
		<< N.ZoneMask
		<< N.iVertPool
		<< N.iSurf
		<< N.iVertexIndex
		<< N.ComponentIndex 
		<< N.ComponentNodeIndex
		<< N.iChild[0]
		<< N.iChild[1]
		<< N.iChild[2]
		<< N.iCollisionBound
		<< N.iZone[0]
		<< N.iZone[1]
		<< N.NumVertices
		<< N.NodeFlags
		<< N.iLeaf[0]
		<< N.iLeaf[1];

		if( Ar.IsLoading() )
		{
			//@warning: this code needs to be in sync with UModel::Serialize as we use bulk serialization.
			N.NodeFlags &= ~(NF_IsNew|NF_IsFront|NF_IsBack);
		}

		return Ar;
}
//>@ ava

FArchive& operator<<( FArchive& Ar, FZoneProperties& P )
{
	Ar	<< P.ZoneActor
		<< P.Connectivity
		<< P.Visibility
		<< P.LastRenderTime;
	return Ar;
}

//!{ 2006-05-08	Çã Ã¢ ¹Î
FArchive& operator<<( FArchive& Ar, FBspNodeLightMap& Map )
{
	Ar << Map.SizeX
	   << Map.SizeY;

	Map.Luxels.BulkSerialize(Ar);

	Ar << Map.WorldToLuxel;

	return Ar;
}
//!} 2006-05-08	Çã Ã¢ ¹Î


/*---------------------------------------------------------------------------------------
	UModel object implementation.
---------------------------------------------------------------------------------------*/

void UModel::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );

	Ar << Bounds;

	Vectors.BulkSerialize( Ar );
	Points.BulkSerialize( Ar );

	//<@ ava specific ; 2006. 11. 17 changmin
	if( Ar.LicenseeVer() < VER_AVA_ADD_CLUSTERNUMBER_TO_FBSPNODE )
	{
		TArray<FLegacyBspNode> OldNodes;
		OldNodes.BulkSerialize( Ar );

		Nodes.Empty();
		Nodes.AddZeroed( OldNodes.Num() );
		for( INT NodeIndex = 0; NodeIndex < OldNodes.Num(); ++NodeIndex )
		{	
			appMemcpy( &Nodes(NodeIndex), &OldNodes(NodeIndex), sizeof( FLegacyBspNode ) );
			Nodes(NodeIndex).iCluster = INDEX_NONE;
		}
	}
	else
	{
		Nodes.BulkSerialize( Ar );
	}
	//>@ ava
	
	if( Ar.IsLoading() )
	{
		for( INT NodeIndex=0; NodeIndex<Nodes.Num(); NodeIndex++ )
		{
			Nodes(NodeIndex).NodeFlags &= ~(NF_IsNew|NF_IsFront|NF_IsBack);
		}
	}
	Ar << Surfs;
	Verts.BulkSerialize( Ar );
	Ar << NumSharedSides << NumZones;
	for( INT i=0; i<NumZones; i++ )
	{
		Ar << Zones[i];
	}
	Ar << Polys;

	LeafHulls.BulkSerialize( Ar );
	Leaves.BulkSerialize( Ar );
	Ar << RootOutside << Linked;
	PortalNodes.BulkSerialize( Ar );
	Edges.BulkSerialize(Ar);

	//!{ 2006-05-08	Çã Ã¢ ¹Î
	if( Ar.LicenseeVer() >= VER_AVA_SAMPLE_AMBIENT && Ar.LicenseeVer() < VER_AVA_REALLY_REMOVE_SAMPLE_AMBIENT && Ar.IsLoading())
	{
		TArray<FBspNodeLightMap> LegacyBspNodeLightMaps;
		Ar << LegacyBspNodeLightMaps;		
	}
	//!} 2006-05-08	Çã Ã¢ ¹Î

	if( Ar.Ver() < VER_USE_UMA_RESOURCE_ARRAY_MESH_DATA &&
		Ar.IsLoading() )
	{
		// build vertex buffers for legacy load
		BuildVertexBuffers();
	}
	else
	{
		Ar << NumVertices; 
		// load/save vertex buffer
		Ar << VertexBuffer;
	}

	//<@ ava specific ; 2006. 12. 18 changmin
	if( Ar.LicenseeVer() >= VER_AVA_PVS_RENDERING )
	{
		Ar << NumClusters;
		Ar << LeafBytes;
		Ar << VisBytes;
		Ar << LeafColors;
	}
	if( Ar.LicenseeVer() >= VER_AVA_PVS_RENDERING_PLACE_STATICMESH )
	{
		Ar << StaticMeshLeaves;
	}
	//>@ ava

	//<@ ava specific ; 2008. 2. 25 changmin
	if( Ar.LicenseeVer() >= VER_AVA_ADD_LEAF_PORTAL_RENDERING )
	{
		Ar << Portals;
		Ar << LeafInfos;
	}
	//>@ ava
}

void UModel::PostLoad()
{
	Super::PostLoad();
	
	// Ensure that if my Outer (usually a brush) is not loaded in game, then I am not.
	// This is to fix some content where add/subtract brushes were losing these flags on their UModels.
	UObject* Outer = GetOuter();
	check(Outer);
	if( Outer->HasAllFlags(RF_NotForClient | RF_NotForServer) )
	{
		SetFlags(RF_NotForClient | RF_NotForServer);
		Polys->SetFlags(RF_NotForClient | RF_NotForServer);
	}

	if( !GIsUCC && !HasAnyFlags(RF_ClassDefaultObject) )
	{
		if( !UEngine::ShadowVolumesAllowed() )
		{
            RemoveShadowVolumeData();
		}

		UpdateVertices();
		if(!Edges.Num())
		{
			BuildShadowData();
		}
	}
}

void UModel::PreEditUndo()
{
	Super::PreEditUndo();

	// Release the model's resources.
	BeginReleaseResources();
	ReleaseResourcesFence.Wait();
}

void UModel::PostEditUndo()
{
	Super::PostEditUndo();

	// Reinitialize the model's resources.
	UpdateVertices();
}

/**
* Used by various commandlets to purge Editor only data from the object.
*/
void UModel::StripData(UE3::EPlatformType TargetPlatform)
{
	Super::StripData(TargetPlatform);

	// remove shadow volume data based on engine config setting
	if( !UEngine::ShadowVolumesAllowed() )
	{
		RemoveShadowVolumeData();
	}
}

void UModel::ModifySurf( INT Index, INT UpdateMaster )
{
	Surfs.ModifyItem( Index );
	FBspSurf& Surf = Surfs(Index);
	if( UpdateMaster && Surf.Actor )
	{
		Surf.Actor->Brush->Polys->Element.ModifyItem( Surf.iBrushPoly );
	}
}
void UModel::ModifyAllSurfs( INT UpdateMaster )
{
	for( INT i=0; i<Surfs.Num(); i++ )
		ModifySurf( i, UpdateMaster );

}
void UModel::ModifySelectedSurfs( INT UpdateMaster )
{
	for( INT i=0; i<Surfs.Num(); i++ )
		if( Surfs(i).PolyFlags & PF_Selected )
			ModifySurf( i, UpdateMaster );

}

UBOOL UModel::Rename( const TCHAR* InName, UObject* NewOuter, ERenameFlags Flags )
{
	// Also rename the UPolys.
    if (NewOuter && Polys && Polys->GetOuter() == GetOuter())
	{
		if (Polys->Rename(*MakeUniqueObjectName(NewOuter, Polys->GetClass()).ToString(), NewOuter, Flags) == FALSE)
		{
			return FALSE;
		}
	}

    return Super::Rename( InName, NewOuter, Flags );
}

/**
 * Called after duplication & serialization and before PostLoad. Used to make sure UModel's FPolys
 * get duplicated as well.
 */
void UModel::PostDuplicate()
{
	Super::PostDuplicate();
	if( Polys )
	{
		Polys = CastChecked<UPolys>(UObject::StaticDuplicateObject( Polys, Polys, GetOuter(), NULL ));
	}
}

void UModel::BeginDestroy()
{
	Super::BeginDestroy();
	BeginReleaseResources();
}

UBOOL UModel::IsReadyForFinishDestroy()
{
	return ReleaseResourcesFence.GetNumPendingFences() == 0 && Super::IsReadyForFinishDestroy();
}

IMPLEMENT_CLASS(UModel);

/*---------------------------------------------------------------------------------------
	UModel implementation.
---------------------------------------------------------------------------------------*/

//
// Lock a model.
//
void UModel::Modify( UBOOL bAlwaysMarkDirty/*=FALSE*/ )
{
	Super::Modify(bAlwaysMarkDirty);

	// Modify all child objects.
	if( Polys )
	{
		Polys->Modify(bAlwaysMarkDirty);
	}
}

//
// Empty the contents of a model.
//
void UModel::EmptyModel( INT EmptySurfInfo, INT EmptyPolys )
{
	Nodes			.Empty();
	LeafHulls		.Empty();
	Leaves			.Empty();
	Verts			.Empty();
	PortalNodes		.Empty();

	if( EmptySurfInfo )
	{
		Vectors.Empty();
		Points.Empty();
		Surfs.Empty();
	}
	if( EmptyPolys )
	{
		Polys = new( GetOuter(), NAME_None, RF_Transactional )UPolys;
	}

	// Init variables.
	NumSharedSides	= 4;
	NumZones = 0;
	for( INT i=0; i<FBspNode::MAX_ZONES; i++ )
	{
		Zones[i].ZoneActor    = NULL;
		Zones[i].Connectivity = FZoneSet::IndividualZone(i);
		Zones[i].Visibility   = FZoneSet::AllZones();
	}
}

//
// Create a new model and allocate all objects needed for it.
//
UModel::UModel( ABrush* Owner, UBOOL InRootOutside )
:	Nodes		( this )
,	Verts		( this )
,	Vectors		( this )
,	Points		( this )
,	Surfs		( this )
,	VertexBuffer( this )
,	ShadowVertexBuffer( this )
,	RootOutside	( InRootOutside )
{
	SetFlags( RF_Transactional );
	EmptyModel( 1, 1 );
	if( Owner )
	{
		check(Owner->BrushComponent);
		Owner->Brush = this;
		Owner->InitPosRotScale();
	}
	if( GIsEditor && !GIsGame )
	{
		UpdateVertices();
	}
}

/**
 * Static constructor called once per class during static initialization via IMPLEMENT_CLASS
 * macro. Used to e.g. emit object reference tokens for realtime garbage collection or expose
 * properties for native- only classes.
 */
void UModel::StaticConstructor()
{
	UClass* Class = GetClass();
	Class->EmitObjectReference( STRUCT_OFFSET( UModel, Polys ) );
	DWORD SkipIndexIndex = Class->EmitStructArrayBegin( STRUCT_OFFSET( UModel, Surfs ), sizeof(FBspSurf) );
	Class->EmitObjectReference( STRUCT_OFFSET( FBspSurf, Material ) );
	Class->EmitObjectReference( STRUCT_OFFSET( FBspSurf, Actor ) );
	Class->EmitStructArrayEnd( SkipIndexIndex );
}

//
// Build the model's bounds (min and max).
//
void UModel::BuildBound()
{
	if( Polys && Polys->Element.Num() )
	{
		TArray<FVector> Points;
		for( INT i=0; i<Polys->Element.Num(); i++ )
			for( INT j=0; j<Polys->Element(i).Vertices.Num(); j++ )
				Points.AddItem(Polys->Element(i).Vertices(j));
		Bounds = FBoxSphereBounds( &Points(0), Points.Num() );
	}
}

//
// Transform this model by its coordinate system.
//
void UModel::Transform( ABrush* Owner )
{
	check(Owner);

	Polys->Element.ModifyAllItems();

	for( INT i=0; i<Polys->Element.Num(); i++ )
		Polys->Element( i ).Transform( Owner->PrePivot, Owner->Location);

}

/**
 * Returns the scale dependent texture factor used by the texture streaming code.	
 */
FLOAT UModel::GetStreamingTextureFactor()
{
	FLOAT MaxTextureRatio = 0.f;

	if( Polys )
	{
		for( INT PolyIndex=0; PolyIndex<Polys->Element.Num(); PolyIndex++ )
		{
			FPoly& Poly = Polys->Element(PolyIndex);

			// Assumes that all triangles on a given poly share the same texture scale.
			if( Poly.Vertices.Num() > 2 )
			{
				FLOAT		L1	= (Poly.Vertices(0) - Poly.Vertices(1)).Size(),
							L2	= (Poly.Vertices(0) - Poly.Vertices(2)).Size();

				FVector2D	UV0	= FVector2D( (Poly.Vertices(0) - Poly.Base) | Poly.TextureU, (Poly.Vertices(0) - Poly.Base) | Poly.TextureV ),
							UV1	= FVector2D( (Poly.Vertices(1) - Poly.Base) | Poly.TextureU, (Poly.Vertices(1) - Poly.Base) | Poly.TextureV ),
							UV2	= FVector2D( (Poly.Vertices(2) - Poly.Base) | Poly.TextureU, (Poly.Vertices(2) - Poly.Base) | Poly.TextureV );

				if( Abs(L1 * L2) > Square(SMALL_NUMBER) )
				{
					FLOAT		T1	= (UV0 - UV1).Size() / 128,
								T2	= (UV0 - UV2).Size() / 128;

					if( Abs(T1 * T2) > Square(SMALL_NUMBER) )
					{
						MaxTextureRatio = Max3( MaxTextureRatio, L1 / T1, L2 / T2 );
					}
				}
			}
		}
	}

	return MaxTextureRatio;
}

/*---------------------------------------------------------------------------------------
	UModel basic implementation.
---------------------------------------------------------------------------------------*/

//
// Shrink all stuff to its minimum size.
//
void UModel::ShrinkModel()
{
	Vectors		.Shrink();
	Points		.Shrink();
	Verts		.Shrink();
	Nodes		.Shrink();
	Surfs		.Shrink();
	if( Polys     ) Polys    ->Element.Shrink();
	LeafHulls	.Shrink();
	PortalNodes	.Shrink();
}

void UModel::BeginReleaseResources()
{
	// Release the index buffers.
	for(TDynamicMap<UMaterialInstance*,FRawIndexBuffer32>::TIterator IndexBufferIt(MaterialIndexBuffers);IndexBufferIt;++IndexBufferIt)
	{
		BeginReleaseResource(&IndexBufferIt.Value());
	}

	//<@ ava specific ; 2006. 9. 27 changmin
	for(TDynamicMap<WORD, FRawIndexBuffer32>::TIterator IndexBufferIt(ComponentIndexBuffers); IndexBufferIt; ++IndexBufferIt)
	{
		BeginReleaseResource(&IndexBufferIt.Value());
	}
	//>@ ava

	// Release the vertex buffer and factory.
	BeginReleaseResource(&VertexBuffer);
	BeginReleaseResource(&VertexFactory);

	// Release the shadow vertex buffer and factory.
	BeginReleaseResource(&ShadowVertexBuffer);

	// Use a fence to keep track of the release progress.
	ReleaseResourcesFence.BeginFence();
}

void UModel::UpdateVertices()
{
	// Wait for pending resource release commands to execute.
	ReleaseResourcesFence.Wait();

	// rebuild vertex buffer if the resource array is not static 
	if( GIsEditor && !GIsGame && !VertexBuffer.Vertices.IsStatic() )
	{	
		BuildVertexBuffers();
	}
	// we should have the same # of vertices in the loaded vertex buffer
	check(NumVertices == VertexBuffer.Vertices.Num());	
	BeginInitResource(&VertexBuffer);
	if( GIsEditor && !GIsGame )
	{
		// needed since we may call UpdateVertices twice and the first time
		// NumVertices might be 0. 
		BeginUpdateResourceRHI(&VertexBuffer);
	}

	// Set up the vertex factory.
	TSetResourceDataContext<FLocalVertexFactory> VertexFactoryData(&VertexFactory);
	VertexFactoryData->PositionComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&VertexBuffer,FModelVertex,Position,VET_Float3);
	VertexFactoryData->TangentBasisComponents[0] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&VertexBuffer,FModelVertex,TangentX,VET_PackedNormal);
	VertexFactoryData->TangentBasisComponents[1] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&VertexBuffer,FModelVertex,TangentY,VET_PackedNormal);
	VertexFactoryData->TangentBasisComponents[2] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&VertexBuffer,FModelVertex,TangentZ,VET_PackedNormal);
	VertexFactoryData->TextureCoordinates.Empty();
	VertexFactoryData->TextureCoordinates.AddItem(STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&VertexBuffer,FModelVertex,TexCoord,VET_Float2));
	VertexFactoryData->ShadowMapCoordinateComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&VertexBuffer,FModelVertex,ShadowTexCoord,VET_Float2);
	VertexFactoryData.Commit();
	BeginInitResource(&VertexFactory);

	if( UEngine::ShadowVolumesAllowed() )
	{
		// Set up the shadow vertex buffer and its vertex factory.
		BeginInitResource(&ShadowVertexBuffer);

		if( GIsEditor && !GIsGame )
		{
			// This is to force it to update. First time UpdateVertices() is called,
			// we may have 0 vertices so the vertex buffer doesn't get created.
			// Second time (from PostLoad) we need this call, because BeginInitResource
			// will not do anything.
			BeginUpdateResourceRHI(&ShadowVertexBuffer);
		}
	}
}

//<@ ava specific 

/**
* Initialize vertex buffer data from UModel data
*/
void UModel::BuildVertexBuffers()
{
	//<@ ava specific;material ¼ø¼­·Î vertex packing ¼ø¼­¸¦ ¹Ù²Û´Ù.
#define AVA_VERTEX_PACKING 1
#if AVA_VERTEX_PACKING
	TDynamicMap<UMaterialInstance*, TArray<INT> > MaterialNodes;
	for(INT NodeIndex = 0;NodeIndex < Nodes.Num();NodeIndex++)
	{
		FBspNode &Node = Nodes(NodeIndex);
		FBspSurf &Surf = Surfs(Node.iSurf);
		TArray<INT> *NodesInSameMaterial = MaterialNodes.Find( Surf.Material );
		if( !NodesInSameMaterial )
		{
			NodesInSameMaterial = &MaterialNodes.Set( Surf.Material, TArray<INT>() );
		}
		NodesInSameMaterial->AddItem( NodeIndex );
	}

	NumVertices = 0;
	for(TDynamicMap<UMaterialInstance*,TArray<INT> >::TIterator NodesIt(MaterialNodes);NodesIt;++NodesIt)
	{
		TArray<INT> &NodesInSameMaterial = NodesIt.Value();
		for(INT iNode = 0; iNode < NodesInSameMaterial.Num() ; ++iNode )
		{
			FBspNode& Node = Nodes(NodesInSameMaterial(iNode));
			FBspSurf& Surf = Surfs(Node.iSurf);
			Node.iVertexIndex = NumVertices;
			NumVertices += (Surf.PolyFlags & PF_TwoSided) ? (Node.NumVertices * 2) : Node.NumVertices;
		}
	}

	// size vertex buffer data
	VertexBuffer.Vertices.Empty(NumVertices);
	VertexBuffer.Vertices.Add(NumVertices);

	if(NumVertices > 0)
	{
		// Initialize the vertex data
		FModelVertex* DestVertex = (FModelVertex*)VertexBuffer.Vertices.GetData();

		for(TDynamicMap<UMaterialInstance*,TArray<INT> >::TIterator NodesIt(MaterialNodes);NodesIt;++NodesIt)
		{
			TArray<INT> &NodesInSameMaterial = NodesIt.Value();
			for(INT iNode = 0; iNode < NodesInSameMaterial.Num() ; ++iNode )
			{
				FBspNode& Node = Nodes(NodesInSameMaterial(iNode));
				FBspSurf& Surf = Surfs(Node.iSurf);
				const FVector& TextureBase = Points(Surf.pBase);
				const FVector& TextureX = Vectors(Surf.vTextureU);
				const FVector& TextureY = Vectors(Surf.vTextureV);
				const FVector& Normal = Vectors(Surf.vNormal).SafeNormal();
				const FVector TextureXNormalized = TextureX.SafeNormal();
				const FVector TextureYNormalized = TextureY.SafeNormal();

				for(UINT VertexIndex = 0;VertexIndex < Node.NumVertices;VertexIndex++)
				{
					const FVert& Vert = Verts(Node.iVertPool + VertexIndex);
					const FVector& Position = Points(Vert.pVertex);
					DestVertex->Position = Position;
					DestVertex->TexCoord.X = ((Position - TextureBase) | TextureX) / 128.0f;
					DestVertex->TexCoord.Y = ((Position - TextureBase) | TextureY) / 128.0f;
					DestVertex->ShadowTexCoord = Vert.ShadowTexCoord;
					DestVertex->TangentX = TextureXNormalized;
					DestVertex->TangentY = TextureYNormalized;
					DestVertex->TangentZ = Normal;
					DestVertex++;
				}

				if(Surf.PolyFlags & PF_TwoSided)
				{
					for(INT VertexIndex = Node.NumVertices - 1;VertexIndex >= 0;VertexIndex--)
					{
						const FVert& Vert = Verts(Node.iVertPool + VertexIndex);
						const FVector& Position = Points(Vert.pVertex);
						DestVertex->Position = Position;
						DestVertex->TexCoord.X = ((Position - TextureBase) | TextureX) / 128.0f;
						DestVertex->TexCoord.Y = ((Position - TextureBase) | TextureY) / 128.0f;
						DestVertex->ShadowTexCoord = Vert.BackfaceShadowTexCoord;
						DestVertex->TangentX = TextureX;
						DestVertex->TangentY = TextureY;
						DestVertex->TangentZ = -Normal;
						DestVertex++;
					}
				}
			}
		}
	}
	//>@ ava
#else
	// Calculate the size of the vertex buffer and the base vertex index of each node.
	NumVertices = 0;
	for(INT NodeIndex = 0;NodeIndex < Nodes.Num();NodeIndex++)
	{
		FBspNode& Node = Nodes(NodeIndex);
		FBspSurf& Surf = Surfs(Node.iSurf);
		Node.iVertexIndex = NumVertices;
		NumVertices += (Surf.PolyFlags & PF_TwoSided) ? (Node.NumVertices * 2) : Node.NumVertices;
	}

	// size vertex buffer data
	VertexBuffer.Vertices.Empty(NumVertices);
	VertexBuffer.Vertices.Add(NumVertices);

	if(NumVertices > 0)
	{
		// Initialize the vertex data
		FModelVertex* DestVertex = (FModelVertex*)VertexBuffer.Vertices.GetData();
		for(INT NodeIndex = 0;NodeIndex < Nodes.Num();NodeIndex++)
		{
			FBspNode& Node = Nodes(NodeIndex);
			FBspSurf& Surf = Surfs(Node.iSurf);
			const FVector& TextureBase = Points(Surf.pBase);
			const FVector& TextureX = Vectors(Surf.vTextureU);
			const FVector& TextureY = Vectors(Surf.vTextureV);
			const FVector& Normal = Vectors(Surf.vNormal).SafeNormal();
			const FVector TextureXNormalized = TextureX.SafeNormal();
			const FVector TextureYNormalized = TextureY.SafeNormal();

			for(UINT VertexIndex = 0;VertexIndex < Node.NumVertices;VertexIndex++)
			{
				const FVert& Vert = Verts(Node.iVertPool + VertexIndex);
				const FVector& Position = Points(Vert.pVertex);
				DestVertex->Position = Position;
				DestVertex->TexCoord.X = ((Position - TextureBase) | TextureX) / 128.0f;
				DestVertex->TexCoord.Y = ((Position - TextureBase) | TextureY) / 128.0f;
				DestVertex->ShadowTexCoord = Vert.ShadowTexCoord;
				DestVertex->TangentX = TextureXNormalized;
				DestVertex->TangentY = TextureYNormalized;
				DestVertex->TangentZ = Normal;
				DestVertex++;
			}

			if(Surf.PolyFlags & PF_TwoSided)
			{
				for(INT VertexIndex = Node.NumVertices - 1;VertexIndex >= 0;VertexIndex--)
				{
					const FVert& Vert = Verts(Node.iVertPool + VertexIndex);
					const FVector& Position = Points(Vert.pVertex);
					DestVertex->Position = Position;
					DestVertex->TexCoord.X = ((Position - TextureBase) | TextureX) / 128.0f;
					DestVertex->TexCoord.Y = ((Position - TextureBase) | TextureY) / 128.0f;
					DestVertex->ShadowTexCoord = Vert.BackfaceShadowTexCoord;
					DestVertex->TangentX = TextureX;
					DestVertex->TangentY = TextureY;
					DestVertex->TangentZ = -Normal;
					DestVertex++;
				}
			}
		}
	}

#endif
}

void UModel::BuildShadowData()
{
	if( UEngine::ShadowVolumesAllowed() )
	{
		// Clear the existing edges.
		Edges.Empty();

		// Create the node edges.
		TMultiMap<INT,INT> VertexToEdgeMap;
		for(INT NodeIndex = 0;NodeIndex < Nodes.Num();NodeIndex++)
		{
			FBspNode& Node = Nodes(NodeIndex);
			FBspSurf& Surf = Surfs(Node.iSurf);

			if(Surf.PolyFlags & PF_TwoSided)
				continue;

			for(INT EdgeIndex = 0;EdgeIndex < Node.NumVertices;EdgeIndex++)
			{
				INT PointIndices[2] =
				{
					Verts(Node.iVertPool + EdgeIndex).pVertex,
						Verts(Node.iVertPool + ((EdgeIndex + 1) % Node.NumVertices)).pVertex
				};

				// Find existing edges which start on this edge's ending vertex.
				TArray<INT> PotentialMatchEdges;
				VertexToEdgeMap.MultiFind(PointIndices[1],PotentialMatchEdges);

				// Check if the ending vertex of any of the existing edges match this edge's start vertex.
				INT MatchEdgeIndex = INDEX_NONE;
				for(INT OtherEdgeIndex = 0;OtherEdgeIndex < PotentialMatchEdges.Num();OtherEdgeIndex++)
				{
					const FMeshEdge& OtherEdge = Edges(PotentialMatchEdges(OtherEdgeIndex));
					if(OtherEdge.Vertices[1] == PointIndices[0] && OtherEdge.Faces[1] == INDEX_NONE)
					{
						MatchEdgeIndex = PotentialMatchEdges(OtherEdgeIndex);
						break;
					}
				}

				if(MatchEdgeIndex != INDEX_NONE)
				{
					// Set the matching edge's opposite face to this node.
					Edges(MatchEdgeIndex).Faces[1] = NodeIndex;
				}
				else
				{
					// Create a new edge.
					FMeshEdge* NewEdge = new(Edges) FMeshEdge;
					NewEdge->Vertices[0] = PointIndices[0];
					NewEdge->Vertices[1] = PointIndices[1];
					NewEdge->Faces[0] = NodeIndex;
					NewEdge->Faces[1] = INDEX_NONE;
					VertexToEdgeMap.Set(PointIndices[0],Edges.Num() - 1);
				}
			}
		}
	}
}

/** 
* Removes all vertex data needed for shadow volume rendering 
*/
void UModel::RemoveShadowVolumeData()
{
#if !CONSOLE
	Edges.Empty();
#endif
}

//<@ ava specific ; 2007. 2. 26 changmin

void DumpBspTree_R( UModel *Bsp, INT NodeIndex, FString Tag, FString CoTag )
{
	const FBspNode &Node = Bsp->Nodes( NodeIndex );
	EName OutputOption = Bsp->Surfs(Node.iSurf).PolyFlags & PF_Semisolid ? NAME_Warning : NAME_Log;

	UBOOL bHint = (Node.NodeFlags & NF_Hint) != 0;
	FString OutFormat = Tag + FString::Printf( TEXT("Node(%i, H:%i) : Child(F:%i, B:%i), Leaf(F:%i, B:%i), Cluster(%i)"), NodeIndex, bHint, Node.iChild[1], Node.iChild[0], Node.iLeaf[1], Node.iLeaf[0], Node.iCluster );
	debugf( OutputOption, *OutFormat );
	INT CoplanerNodeIndex = Node.iPlane;
	while( CoplanerNodeIndex != INDEX_NONE )
	{
		FBspNode &CoplanerNode = Bsp->Nodes( CoplanerNodeIndex );
		bHint = (CoplanerNode.NodeFlags & NF_Hint) != 0;
		EName OutputOption = Bsp->Surfs(CoplanerNode.iSurf).PolyFlags & PF_Semisolid ? NAME_Warning : NAME_Log;
		FString OutFormat = CoTag + FString::Printf( TEXT("Node(%i, H:%i) : Child(F:%i, B:%i), Leaf(F:%i, B:%i), Cluster(%i)"), CoplanerNodeIndex, bHint, CoplanerNode.iChild[1], CoplanerNode.iChild[0], CoplanerNode.iLeaf[1], CoplanerNode.iLeaf[0], CoplanerNode.iCluster );
		debugf( OutputOption, *OutFormat );
		CoplanerNodeIndex = CoplanerNode.iPlane;
	}
	if( Node.iFront != INDEX_NONE )
	{
		FString FrontCoTag = Tag + FString::Printf(TEXT("(%i):C->"), Node.iFront);
		FString FrontTag = Tag + FString::Printf(TEXT("(%i):F-->"), NodeIndex);
		DumpBspTree_R( Bsp, Node.iFront, FrontTag, FrontCoTag );
	}
	if( Node.iBack != INDEX_NONE )
	{
		FString BackCoTag = Tag + FString::Printf(TEXT("(%i):C->"), Node.iBack);
		FString BackTag = Tag + FString::Printf(TEXT("(%i):B-->"), NodeIndex);
		DumpBspTree_R( Bsp, Node.iBack, BackTag, BackCoTag );
	}
}
void UModel::DumpBspTree()
{
	debugf( NAME_Warning, TEXT("--------------------- dump bsp tree --------------------") );
	DumpBspTree_R(this, 0, FString(), FString(TEXT(":C->")));
}

void UModel::DumpBspInfo()
{
	debugf( NAME_Warning, TEXT("-------------------- DUMP BSP INFO-------------------") );
	debugf( NAME_Log, *FString::Printf(TEXT("Num Vertices  : %d"), NumVertices ) );
	debugf( NAME_Log, *FString::Printf(TEXT("Num Materials : %d"), MaterialIndexBuffers.Num()) );
	debugf( NAME_Log, *FString::Printf(TEXT("Num Nodes     : %d"), Nodes.Num()) );
	if( GWorld->PersistentLevel->StructuralModel )
	{
		debugf( NAME_Log, *FString::Printf(TEXT("Num Clusters  : %d"), GWorld->PersistentLevel->StructuralModel->NumClusters) );
	}
	debugf( NAME_Log, *FString::Printf(TEXT("Num Components: %d"), GWorld->PersistentLevel->ModelComponents.Num()) );

	INT ElementCount = 0;
	for( INT ComponentIndex = 0; ComponentIndex < GWorld->PersistentLevel->ModelComponents.Num(); ++ComponentIndex )
	{
		const UModelComponent *Component = GWorld->PersistentLevel->ModelComponents(ComponentIndex);
		ElementCount += Component->GetElements().Num();
	}
	debugf( NAME_Log, *FString::Printf(TEXT("Num Elements: %d"), ElementCount) );
}
//>@ ava

//<@ ava specific ; 2006. 11. 17 changmin
INT UModel::GetCurrentLeaf( const FVector& Position )
{
	if( Nodes.Num() )
	{
		INT CurrentNode = 0;
		while( 1 )
		{
			const FBspNode& Node = Nodes(CurrentNode);
			if( Node.Plane.PlaneDot( Position ) >= 0.0f )
			{
				if( Node.iFront != INDEX_NONE )
				{
					CurrentNode = Node.iFront;
				}
				else
				{
					return Node.iCluster;
				}
			}
			else
			{
				if( Node.iBack != INDEX_NONE )
				{
					CurrentNode = Node.iBack;
				}
				else
				{
					return Node.iCluster;
				}
			}
		}
	}

	return INDEX_NONE;
}
//>@ ava