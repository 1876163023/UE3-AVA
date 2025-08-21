#include "avaGame.h"
#include "EngineMaterialClasses.h"
#include "UnNet.h"

#include "avaShatterGlass.h"
#include "avaShatterGlassRender.h"

static struct FShatterGlassVertexFactory : FLocalVertexFactory 
{
	FShatterGlassVertexFactory() 
	{
		DataType Data;	

		Data.PositionComponent			= STRUCTMEMBER_VERTEXSTREAMCOMPONENT( ((FVertexBuffer*)-1), FShatterGlassVertex, Position, VET_Float3 );
		Data.TangentBasisComponents[0]	= STRUCTMEMBER_VERTEXSTREAMCOMPONENT( ((FVertexBuffer*)-1), FShatterGlassVertex, TangentX, VET_PackedNormal );
		Data.TangentBasisComponents[1]	= STRUCTMEMBER_VERTEXSTREAMCOMPONENT( ((FVertexBuffer*)-1), FShatterGlassVertex, TangentY, VET_PackedNormal );
		Data.TangentBasisComponents[2]	= STRUCTMEMBER_VERTEXSTREAMCOMPONENT( ((FVertexBuffer*)-1), FShatterGlassVertex, TangentZ, VET_PackedNormal );

		Data.TextureCoordinates.Empty();
		Data.TextureCoordinates.AddItem( STRUCTMEMBER_VERTEXSTREAMCOMPONENT( ((FVertexBuffer*)-1), FShatterGlassVertex, UV, VET_Float2 ) );		
		Data.TextureCoordinates.AddItem( STRUCTMEMBER_VERTEXSTREAMCOMPONENT( ((FVertexBuffer*)-1), FShatterGlassVertex, DetailUV, VET_Float2 ) );		

		SetData( Data );
	}

} GShatterGlassVertexFactory;

FShatterGlassSceneProxy::FShatterGlassSceneProxy( UavaShatterGlassComponent* Component )
: Component( Component ), FPrimitiveSceneProxy( Component ), DynamicData( NULL ), bHasTranslucency( Component->HasUnlitTranslucency() ), bHasDistortion( Component->HasUnlitDistortion() )
{		
}		

FShatterGlassSceneProxy::~FShatterGlassSceneProxy()
{
	delete DynamicData;
}

void FShatterGlassSceneProxy::DrawDynamicElements( FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DepthPriorityGroup ) 
{
	// Determine the DPG the primitive should be drawn in for this view.
	BYTE PrimitiveDPG = GetDepthPriorityGroup(View);				

	if (DepthPriorityGroup == PrimitiveDPG)
	{
		if (View->Family->ShowFlags & SHOW_Editor)
		{
			if (!Component->bSolidRenderListUpdated)
			{
				Component->UpdateSolidRenderList();
				Component->bSolidRenderListUpdated = TRUE;
			}			

			if (Component->GetOwner()->IsSelected())
			{
				for (INT i=0; i<Component->SolidRenderList.Num(); ++i)
				{
					const FSolidRenderInfo& sri = Component->SolidRenderList(i);
					Component->RenderOneBlock( PDI, DepthPriorityGroup, sri.Base, sri.X, sri.Z );
				}
			}			
		}

		for (INT i=0; i<13; ++i)
		{
			if (DynamicData->Inner[i].NumIndices == 0) continue;

			FMeshElement MeshElement;										
			MeshElement.MaterialInstance = DynamicData->Inner[i].MaterialResource;
			MeshElement.LocalToWorld = FMatrix::Identity;
			MeshElement.WorldToLocal = FMatrix::Identity;			
			MeshElement.DepthPriorityGroup = (ESceneDepthPriorityGroup)PrimitiveDPG;			

			FDynamicMeshContext DynamicMeshContext;
			if (IsAffordableForDynamicMesh( sizeof(FShatterGlassVertex) * DynamicData->Inner[i].NumVertices, sizeof(WORD) * DynamicData->Inner[i].NumIndices, 0 ))
			{
				AllocDynamicMesh( DynamicMeshContext, sizeof(FShatterGlassVertex), DynamicData->Inner[i].NumVertices, sizeof(WORD), DynamicData->Inner[i].NumIndices );

				// Alloc failed
				if (DynamicMeshContext.VertexBuffer == NULL)
					continue;

				appMemcpy( DynamicMeshContext.VertexBuffer, DynamicData->Inner[i].Vertices, sizeof(FShatterGlassVertex) * DynamicData->Inner[i].NumVertices );
				DynamicMeshContext.NumVertices += DynamicData->Inner[i].NumVertices;

				appMemcpy( 
					(WORD*)DynamicMeshContext.IndexBuffer, 
					DynamicData->Inner[i].Indices, 
					DynamicData->Inner[i].NumIndices * sizeof(WORD) 
					);

				DynamicMeshContext.NumIndices += DynamicData->Inner[i].NumIndices;

				CommitDynamicMesh( DynamicMeshContext );

				DynamicMeshContext.GenerateMeshElement( MeshElement, GShatterGlassVertexFactory );

				PDI->DrawMesh(MeshElement);			
			}
		}					
	}
}

void FShatterGlassSceneProxy::UpdateData(FShatterGlassRenderData* NewDynamicData)
{
	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		ShatterGlassUpdateDataCommand,
		FShatterGlassSceneProxy*, Proxy, this,
		FShatterGlassRenderData*, NewDynamicData, NewDynamicData,
	{
		Proxy->UpdateData_RenderThread(NewDynamicData);
	}
	);
}

void FShatterGlassSceneProxy::UpdateData_RenderThread(FShatterGlassRenderData* NewDynamicData)
{
	if (DynamicData != NewDynamicData)
	{
		delete DynamicData;
	}
	DynamicData = NewDynamicData;	
}

FPrimitiveViewRelevance FShatterGlassSceneProxy::GetViewRelevance(const FSceneView* View)
{
	FPrimitiveViewRelevance Result;
	const EShowFlags ShowFlags = View->Family->ShowFlags;
	if (IsShown(View))
	{
		Result.bDynamicRelevance = TRUE;
		Result.SetDPG(SDPG_World,TRUE);
		if (!(View->Family->ShowFlags & SHOW_Wireframe) && (View->Family->ShowFlags & SHOW_Materials))
		{
			Result.bTranslucentRelevance = bHasTranslucency;
			Result.bDistortionRelevance = bHasDistortion;
		}
	}
	return Result;
}