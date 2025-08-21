#include "avaGame.h"

#include "avaBulletTrailComponent.h"
#include "avaBulletTrailRender.h"

IMPLEMENT_CLASS(UavaBulletTrailComponent);

UavaBulletTrailComponent::UavaBulletTrailComponent()
{
	Context = new FBulletTrailRenderContext( this );
}

void UavaBulletTrailComponent::FinishDestroy()
{	
	delete Context;

	Super::FinishDestroy();
}

void UavaBulletTrailComponent::Fire(const FVector& Source, const FVector& Destination, const FVector& InitVel )
{
	new(BulletTrails)FBulletTrailEntry( Source, Destination, InitVel, Speed, Size, AddDeltaTime );	
}

/** Returns true if the prim is using a material with unlit distortion */
UBOOL UavaBulletTrailComponent::HasUnlitDistortion()
{
	UMaterial* Material = this->Material ? this->Material->GetMaterial() : NULL;
	return ( Material && Material->LightingModel == MLM_Unlit && Material->HasDistortion() );	
}

/** Returns true if the prim is using a material with unlit translucency */
UBOOL UavaBulletTrailComponent::HasUnlitTranslucency()
{
	UMaterial* Material = this->Material ? this->Material->GetMaterial() : NULL;
	return (Material && (Material->LightingModel == MLM_Unlit) && IsTranslucentBlendMode((EBlendMode)Material->BlendMode) );
}

void UavaBulletTrailComponent::Tick( FLOAT DeltaTime )
{
	Super::Tick( DeltaTime );

	FLOAT CurrentTime = GWorld->GetTimeSeconds();		

	UBOOL Changed = FALSE;

	Context->Segments.Empty( BulletTrails.Num() );
	Context->Indices.Empty( BulletTrails.Num() * 6 );	

	for (INT i=0; i<BulletTrails.Num(); ++i)
	{
		const FBulletTrailEntry& e = BulletTrails(i);

		if (!Context->AddEntry( e ))
		{
			BulletTrails.Remove( i );
			--i;

			Changed = TRUE;
			continue;
		}
	}		

	if (Changed || BulletTrails.Num() > 0)
		ConditionalUpdateTransform();

	UpdateDynamicData();		
}


UBOOL FBulletTrailRenderContext::AddEntry( const FBulletTrailEntry& e )
{
	FLOAT CurrentTime = GWorld->GetTimeSeconds();	

	// Intensity = FMin( Intensity0 * expf( - DeltaTime / HalfLife ), 1 );

	// HalfLife * -Log( Cut-off / Intensity ) * Speed = DeltaTime

	FLOAT DeltaTime = CurrentTime - e.FiredTime + e.AddDeltaTime;

	FLOAT CutoffTime = Component->HalfLife * -appLoge( 0.2f * Component->Intensity );		

	if (DeltaTime - CutoffTime > e.TTL)
	{
		return FALSE;
	}

	INT V = Segments.Num() * 4;

	Indices.AddItem( V );
	Indices.AddItem( V + 1 );
	Indices.AddItem( V + 2 );

	Indices.AddItem( V );
	Indices.AddItem( V + 2 );
	Indices.AddItem( V + 3 );
	
	Segments.AddItem( 
		FBulletTrailSegment( 
			e.Source + e.Direction * Min( e.Speed * Max( 0.0f, DeltaTime - CutoffTime ), e.Length ), 
			e.Source + e.Direction * Min( e.Speed * DeltaTime, e.Length ), 
			1, 0, e.Size, e.Size ) );		

	return TRUE;
}



void UavaBulletTrailComponent::UpdateBounds()
{
	FBox BoundingBox;
	BoundingBox.Init();

	for (INT i=0; i<BulletTrails.Num(); ++i)
	{
		const FBulletTrailEntry& e = BulletTrails(i);

		BoundingBox += e.Source;
		BoundingBox += e.Destination;
	}

	Bounds = FBoxSphereBounds(BoundingBox);	
}

void UavaBulletTrailComponent::Precache()
{
}

UBOOL UavaBulletTrailComponent::IsValidComponent() const
{
	return TRUE;
}

struct FBulletTrailVertex
{
	FVector			Position;				// 12 bytes
	FPackedNormal	TangentX, TangentY, TangentZ;	// 12 bytes

	/** Decal mesh texture coordinates. */
	FVector2D		UV;						// 8 bytes

	FBulletTrailVertex() {}
	FBulletTrailVertex(const FVector& InPosition,
		const FVector2D& InUV )
		:	Position( InPosition )		
		,	UV( InUV )		
	{}
};

static struct FBulletTrailVertexFactory : FLocalVertexFactory 
{
	FBulletTrailVertexFactory() 
	{
		DataType Data;	

		Data.PositionComponent			= STRUCTMEMBER_VERTEXSTREAMCOMPONENT( ((FVertexBuffer*)-1), FBulletTrailVertex, Position, VET_Float3 );
		Data.TangentBasisComponents[0]	= STRUCTMEMBER_VERTEXSTREAMCOMPONENT( ((FVertexBuffer*)-1), FBulletTrailVertex, TangentX, VET_PackedNormal );
		Data.TangentBasisComponents[1]	= STRUCTMEMBER_VERTEXSTREAMCOMPONENT( ((FVertexBuffer*)-1), FBulletTrailVertex, TangentY, VET_PackedNormal );
		Data.TangentBasisComponents[2]	= STRUCTMEMBER_VERTEXSTREAMCOMPONENT( ((FVertexBuffer*)-1), FBulletTrailVertex, TangentZ, VET_PackedNormal );

		Data.TextureCoordinates.Empty();
		Data.TextureCoordinates.AddItem( STRUCTMEMBER_VERTEXSTREAMCOMPONENT( ((FVertexBuffer*)-1), FBulletTrailVertex, UV, VET_Float2 ) );		

		SetData( Data );
	}

} GBulletVertexFactory;

/**
* A static mesh component scene proxy.
*/
class FBulletTrailSceneProxy : public FPrimitiveSceneProxy
{
public:
	virtual UBOOL IsCacheable() const
	{
		return TRUE;
	}	

	FBulletTrailRenderData* DynamicData;
	UBOOL bHasTranslucency;
	UBOOL bHasDistortion;

	/** Initialization constructor. */
	FBulletTrailSceneProxy(UavaBulletTrailComponent* Component):
		FPrimitiveSceneProxy(Component), DynamicData( NULL ), bHasTranslucency( Component->HasUnlitTranslucency() ), bHasDistortion( Component->HasUnlitDistortion() )
	{		
	}

	~FBulletTrailSceneProxy()
	{
		delete DynamicData;
	}	

	void Update( const FSceneView* View, FDynamicMeshContext& DynamicMeshContext )
	{
		FMatrix CameraToWorld = View->ViewMatrix.Inverse();

		FBulletTrailVertex* Vertex = (FBulletTrailVertex*)DynamicMeshContext.VertexBuffer;
		WORD* Index = (WORD*)DynamicMeshContext.IndexBuffer;

		for (INT i=0; i<DynamicData->NumSegments; ++i)
		{
			const FBulletTrailSegment& Segment = DynamicData->Segments[i];

			INT VertexOffset = DynamicMeshContext.NumVertices;

			FVector	ViewOrigin	= CameraToWorld.GetOrigin();		
			FVector Up;
			FVector Right;
			Right		= Segment.P0 - Segment.P1;
			Up			= Right ^  (Segment.P0 - View->ViewOrigin);
			Up.Normalize();	

			*Vertex++ = FBulletTrailVertex( Segment.P0 - Up * Segment.S0, FVector2D( 0, Segment.U0 ) );
			*Vertex++ = FBulletTrailVertex( Segment.P0 + Up * Segment.S0, FVector2D( 1, Segment.U0 ) );
			*Vertex++ = FBulletTrailVertex( Segment.P1 - Up * Segment.S1, FVector2D( 0, Segment.U1 ) );
			*Vertex++ = FBulletTrailVertex( Segment.P1 + Up * Segment.S1, FVector2D( 1, Segment.U1 ) ); 

			*Index++ = VertexOffset + 0;
			*Index++ = VertexOffset + 1;
			*Index++ = VertexOffset + 2;

			*Index++ = VertexOffset + 0;
			*Index++ = VertexOffset + 2;
			*Index++ = VertexOffset + 3;

			DynamicMeshContext.NumVertices += 4;
			DynamicMeshContext.NumIndices += 6;
		}
	}
			
public:	
	virtual void DrawDynamicElements(FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex)
	{		
		if (DynamicData == NULL || DynamicData->NumIndices == 0) return;

		// Determine the DPG the primitive should be drawn in for this view.
		BYTE PrimitiveDPG = GetDepthPriorityGroup(View);				
		
		if( DPGIndex == PrimitiveDPG )
		{
			FMeshElement MeshElement;										
			MeshElement.MaterialInstance = DynamicData->MaterialResource;
			MeshElement.LocalToWorld = FMatrix::Identity;
			MeshElement.WorldToLocal = FMatrix::Identity;			
			MeshElement.DepthPriorityGroup = (ESceneDepthPriorityGroup)PrimitiveDPG;			

			FDynamicMeshContext DynamicMeshContext;
			if (IsAffordableForDynamicMesh( sizeof(FBulletTrailVertex) * DynamicData->NumVertices, sizeof(WORD) * DynamicData->NumIndices, 0 ))
			{
				AllocDynamicMesh( DynamicMeshContext, sizeof(FBulletTrailVertex), DynamicData->NumVertices, sizeof(WORD), DynamicData->NumIndices );

				// Alloc failed
				if (DynamicMeshContext.VertexBuffer == NULL)
					return;

				Update( View, DynamicMeshContext );

				CommitDynamicMesh( DynamicMeshContext );

				DynamicMeshContext.GenerateMeshElement( MeshElement, GBulletVertexFactory );

				PDI->DrawMesh(MeshElement);
			}			
		}		
	}
	
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View)
	{   
		FPrimitiveViewRelevance Result;
		if(IsShown(View))
		{
			Result.bDynamicRelevance = TRUE;			
			Result.SetDPG(GetDepthPriorityGroup(View),TRUE);			

			if (!(View->Family->ShowFlags & SHOW_Wireframe) && (View->Family->ShowFlags & SHOW_Materials))
			{
				Result.bTranslucentRelevance = bHasTranslucency;
				Result.bDistortionRelevance = bHasDistortion;
			}
		}
		return Result;
	}

	void UpdateData(FBulletTrailRenderData* NewDynamicData)
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			BulletTrailUpdateDataCommand,
			FBulletTrailSceneProxy*, Proxy, this,
			FBulletTrailRenderData*, NewDynamicData, NewDynamicData,
		{
			Proxy->UpdateData_RenderThread(NewDynamicData);
		}
		);
	}

	void UpdateData_RenderThread(FBulletTrailRenderData* NewDynamicData)
	{		
		if (DynamicData != NewDynamicData)
		{
			delete DynamicData;
		}
		DynamicData = NewDynamicData;	
	}


	virtual EMemoryStats GetMemoryStatType( void ) const { return( STAT_GameToRendererMallocStMSP ); }
	virtual DWORD GetMemoryFootprint( void ) const { return( sizeof( *this ) + GetAllocatedSize() ); }
	DWORD GetAllocatedSize( void ) const { return( FPrimitiveSceneProxy::GetAllocatedSize() ); }
};

FPrimitiveSceneProxy* UavaBulletTrailComponent::CreateSceneProxy()
{
	FBulletTrailSceneProxy* NewProxy = ::new FBulletTrailSceneProxy(this);
	check (NewProxy);

	if (NewProxy)
	{
		FBulletTrailRenderData* RenderData = new FBulletTrailRenderData( Context );

		NewProxy->UpdateData( RenderData );
	}

	return NewProxy;
}

void UavaBulletTrailComponent::UpdateDynamicData()
{
	FBulletTrailSceneProxy* SceneProxy = (FBulletTrailSceneProxy*)Scene_GetProxyFromInfo(SceneInfo);
	
	if (SceneProxy)
	{
		FBulletTrailRenderData* RenderData = new FBulletTrailRenderData( Context );

		SceneProxy->UpdateData( RenderData );
	}
}