/*=============================================================================
AvaModelRender.cpp: Ava Bsp rendering batcher.
Copyright ?2003-2006 Redduck. All Rights Reserved.
=============================================================================*/
#include "EnginePrivate.h"
#include "EngineDecalClasses.h"
#include "unDecalRenderData.h"
#include "LevelUtils.h"
#include "HModel.h"

#include "ScenePrivate.h"

//#include "EngineMaterialClasses.h"
#include "BSPDynamicBatch.h"

// Min desired space in index buffer 100 tris :)
#define NUM_PRIMITIVES_PER_BATCH (4096)

FColor LevelColor;
FColor PropertyColor;

FMaterialInstance* GFirstMaterial = NULL;
INT GNumMaterials = 0;

EBSPRenderPhase CurrentPhase = BSP_NODRAW;

INT GBSPDynamicBatchCount = 0;
UINT GMaxBspVertexIndex = 0;

#if FINAL_RELEASE
#	define GEnableFastBSPRenderingPath 1
#else
#	define GEnableFastBSPRenderingPath !GIsEditor
#endif

class AvaLightCache: public FLightCacheInterface
{
public:

	/** Initialization constructor. */
	AvaLightCache()
	{}

	void BindElement(const FModelElement& ModelElement)
	{
		if (CurrentPhase == BSP_OVERRIDEMATERIAL)
		{
			check(FALSE);
		}
		else if (CurrentPhase == BSP_EMISSIVE)
		{
			LightMap = ModelElement.LightMap;			

			if (!GEnableFastBSPRenderingPath)
			{
				StaticLightInteractionMap.Empty();

				if(LightMap)
				{
					for(INT LightIndex = 0;LightIndex < LightMap->LightGuids.Num();LightIndex++)
					{
						StaticLightInteractionMap.Set(LightMap->LightGuids(LightIndex),FLightInteraction::LightMap());
					}
				}	
			}			
		}
		else if (CurrentPhase == BSP_LIGHTING)
		{
			LightMap = NULL;			

			if (!GEnableFastBSPRenderingPath)
			{
				StaticLightInteractionMap.Empty();
			}			
		}		
	}

	// FLightCacheInterface.
	virtual FLightInteraction GetInteraction(const FLightSceneInfo* LightSceneInfo) const
	{
		extern UBOOL GUseCascadedShadow;

		if (GEnableFastBSPRenderingPath)
			return (LightSceneInfo->bStaticLighting	&&
			!(LightSceneInfo->bUseCascadedShadowmap || GUseCascadedShadow)) ? FLightInteraction::LightMap() : FLightInteraction::Uncached();

		//<@ ava specific ; 2007. 10. 31 changmin
		// add cascaded shadow
		if( LightSceneInfo->bUseCascadedShadowmap && GUseCascadedShadow )
			return FLightInteraction::Uncached();
		//>@ ava

		// Check for a static light interaction.
		const FLightInteraction* Interaction = StaticLightInteractionMap.Find(LightSceneInfo->LightmapGuid);
		if(Interaction)
		{
			return *Interaction;
		}
		
		return FLightInteraction::Uncached();
	}

	virtual const FLightMap* GetLightMap() const
	{
		return LightMap;
	}

private:
	/** A map from persistent light IDs to information about the light's interaction with the model element. */
	TMap<FGuid,FLightInteraction> StaticLightInteractionMap;

	/** The light-map used by the element. */
	const FLightMap* LightMap;	
};




const ULightComponent* GCurrentLightComponent;

void BspRendering_StartBatch( EBSPRenderPhase Phase, const ULightComponent* LightComponent )
{
	GNumMaterials = 0;
	GCurrentLightComponent = LightComponent;
	GFirstMaterial = NULL;
	GBSPDynamicBatchCount++;
	CurrentPhase = Phase;
	GMaxBspVertexIndex = 0;
}

struct LightmapBatch
{
	const FLightMap			*Lightmap;
	FBspNodeRenderData		*FirstRenderData;
	struct LightmapBatch	*Next;
	INT Padding;
};

struct FSortBin
{
	FSortBin( const FVertexFactory* InVertexFactory, FMaterialInstance* InMaterialInstance )
		: MaterialInstance(InMaterialInstance)
	{			
		const FMaterialShaderMap* MaterialShaderIndex = InMaterialInstance->GetMaterial()->GetShaderMap();

		if (MaterialShaderIndex)
		{
			const FMeshMaterialShaderMap* MeshShaderIndex = MaterialShaderIndex->GetMeshShaderMap(InVertexFactory->GetType());

			if (MeshShaderIndex)
			{
				VertexShader = MeshShaderIndex->GetShader(&TEmissiveVertexShader<FLightMapTexturePolicy>::StaticType);
				PixelShader = MeshShaderIndex->GetShader(&TEmissivePixelShader<FLightMapTexturePolicy>::StaticType);					
				return;
			}					
		}		

		VertexShader = NULL;
		PixelShader = NULL;
	}

	const FShader* VertexShader;
	const FShader* PixelShader;
	FMaterialInstance* MaterialInstance;	
};

INT Compare( const FSortBin& A, const FSortBin& B )
{
	COMPAREDRAWINGPOLICYMEMBERS(VertexShader);
	COMPAREDRAWINGPOLICYMEMBERS(PixelShader);
	COMPAREDRAWINGPOLICYMEMBERS(MaterialInstance);

	return 0;
}

IMPLEMENT_COMPARE_CONSTREF(FSortBin,AvaModelRender,{ return ::Compare(A,B); });

void BspRendering_EndBatch( FPrimitiveDrawInterface *PDI )
{
	SCOPE_CYCLE_COUNTER(STAT_BSPFlush);

	if( CurrentPhase == BSP_NODRAW )
	{
		return;
	}
	/*
	 * Batch를 Commit 하는 조건
	 * 1. Material Change.
	 * 2. LCI Change.
     * 3. Model Change.(Matrix Change)
	 */

	// output data : 다음을 설정해 주어야 합니다.
	//FMeshElement MeshElement;
	//MeshElement.IndexBuffer = ModelElement.IndexBuffer;
	//MeshElement.VertexFactory = &Component->GetModel()->VertexFactory;
	//MeshElement.MaterialInstance = Elements(ElementIndex).GetMaterial()->GetInstanceInterface(FALSE);
	//MeshElement.LCI = &Elements(ElementIndex);
	//MeshElement.LocalToWorld = Component->LocalToWorld;
	//MeshElement.WorldToLocal = Component->WorldToLocal;
	//MeshElement.PreviousLocalToWorld = Component->PreviousLocalToWorld;
	//MeshElement.FirstIndex = ModelElement.FirstIndex;
	//MeshElement.NumPrimitives = ModelElement.NumTriangles;
	//MeshElement.MinVertexIndex = ModelElement.MinVertexIndex;
	//MeshElement.MaxVertexIndex = ModelElement.MaxVertexIndex;
	//MeshElement.Type = PT_TriangleList;
	//MeshElement.DepthPriorityGroup = PrimitiveDPG;

	FMeshElement RenderMesh;

	// batching context
	// 우리는 PersistentLevel의 Bsp만을 사용합니다. 이 가정이 달라지면.. 이것을 고쳐야 합니다.
	UModel *CurrentModel = GWorld->PersistentLevel->Model;
	RenderMesh.LocalToWorld			= FMatrix::Identity;
	RenderMesh.WorldToLocal			= FMatrix::Identity;	
	RenderMesh.DepthPriorityGroup	= SDPG_World;		// bsp dpg는 world만 허용합니다.

	AvaLightCache LightCache;
	
	// start batch.	
#define FlushDynamicIndexBuffer( bFinalize )																\
	if (bBufferGrabbed)																						\
	{																										\
		if( BatchContext.NumIndices )																		\
		{																									\
			CommitDynamicMesh( BatchContext );																\
			BatchContext.GenerateMeshElement( RenderMesh, CurrentModel->VertexFactory );					\
			DrawRichMesh(PDI, RenderMesh, FLinearColor::White, LevelColor, PropertyColor, NULL, FALSE);		\
			bBufferGrabbed = FALSE;																			\
		}																									\
		else if( bFinalize )																				\
		{																									\
			CommitDynamicMesh( BatchContext );																\
			bBufferGrabbed = FALSE;																			\
		}																									\
	}
//#define GrabDynamicIndexBuffer()																			\
//	if (!bBufferGrabbed)																					\
//	{																										\
//		AllocDynamicMesh( BatchContext, 0, 0, IndexSize, NUM_PRIMITIVES_PER_BATCH * 3 );					\
//		NumAvailablePrimitivesInBuffer = BatchContext.MaxIndices / 3;										\
//		bBufferGrabbed = TRUE;																				\
//	}	

#define GrabDynamicIndexBuffer()																			\
	if (!bBufferGrabbed)																					\
	{																										\
	AllocDynamicMesh( BatchContext, 0, 0, IndexSize, NUM_PRIMITIVES_PER_BATCH * 3 );						\
	NumAvailablePrimitivesInBuffer = (BatchContext.MaxIndices == 0) ? 0 : NUM_PRIMITIVES_PER_BATCH;			\
	bBufferGrabbed = TRUE;																					\
	}	

	static TArray<FSortBin> SortBins;	

	if (GNumMaterials > 0 && CurrentPhase == BSP_EMISSIVE)
	{
		SortBins.Reserve( GNumMaterials );
		SortBins.Empty( SortBins.Num() + SortBins.GetSlack() );	

		for(FMaterialInstance *Material = GFirstMaterial; Material; Material = Material->Next )
		{
			FSortBin* SortBin = new(SortBins) FSortBin( &CurrentModel->VertexFactory, Material  );
		}

		Sort<USE_COMPARE_CONSTREF(FSortBin,AvaModelRender)>( &SortBins(0), SortBins.Num() );

		GFirstMaterial = SortBins(0).MaterialInstance;
		
		FMaterialInstance** PrevMaterial = &GFirstMaterial;

		for (INT MaterialIndex=0; MaterialIndex<GNumMaterials; ++MaterialIndex)
		{
			*PrevMaterial = SortBins(MaterialIndex).MaterialInstance;

			PrevMaterial = &(*PrevMaterial)->Next;
		}

		*PrevMaterial = NULL;
	}
	

	UBOOL bBufferGrabbed = FALSE;
	UINT IndexSize = sizeof(DWORD);
	// this zero's make compiler happy :)
	INT NumAvailablePrimitivesInBuffer = 0;
	FDynamicMeshContext	BatchContext = {0};	

	// set initial value.	
	INT MaterialCount = 0;
	static INT PreviousMaterialCount = 0;

	//<@ ava specific ; 2007. 11. 14 changmin
	// add cascaded shadow
	extern UBOOL GUseCascadedShadow;
	extern UBOOL GDrawingShadowDepth;
	const UBOOL bUseNodeCulling = (GUseCascadedShadow && GCurrentLightComponent && GCurrentLightComponent->bUseCascadedShadowmap);
								//|| (GUseCascadedShadow && GDrawingShadowDepth ); // Shadow Depth 는 Component Culling을 사용합니다. Static Mesh가 bsp 를 가릴 경우,계산이 애매함..
	//>@ ava
	

	switch( CurrentPhase )
	{
	case BSP_OVERRIDEMATERIAL:
		IndexSize = (GMaxBspVertexIndex > 0xffff) ? sizeof(DWORD) : sizeof(WORD);
		switch( IndexSize )
		{
		case sizeof(WORD):
			for(FMaterialInstance *Material = GFirstMaterial; Material; Material = Material->Next )
			{
				// draw setting
				RenderMesh.BaseVertexIndex	= 0;
				RenderMesh.MaterialInstance = Material;		
				RenderMesh.LCI = NULL;

				// for each LCI
				for( const FModelElement *ElementOuter = (const FModelElement*)Material->FirstElement; ElementOuter; ElementOuter = ElementOuter->Sibling )
				{				
					// for each elements
					for( const FModelElement *Element = ElementOuter; Element; Element = Element->Next )
					{

						GrabDynamicIndexBuffer();

						//<@ ava specific ; 2007. 11. 14 changmin
						// add cascaded shadow ; sun lit culling
						const TArray<FBspNodeRenderData>* RenderDataList = NULL;
						if( bUseNodeCulling )
						{
							RenderDataList = &Element->SunLitNodeRenderData;
							if( Element->Next )
							{
								PREFETCH( Element->Next->SunLitNodeRenderData.GetTypedData() );
							}
						}
						else
						{
							RenderDataList = &Element->NodeRenderData;
							if( Element->Next )
							{
								PREFETCH( Element->Next->NodeRenderData.GetTypedData() );
							}
						}
						//>@ ava

						RenderMesh.bDrawnShared = FALSE;

						WORD *Indices16 = &(BatchContext.CurrentIndex<WORD>());
						//for (INT NodeIndex = 0; NodeIndex < Element->NodeRenderData.Num(); ++NodeIndex)
						for (INT NodeIndex = 0; NodeIndex < RenderDataList->Num(); ++NodeIndex)
						{
							const FBspNodeRenderData &RenderData = (*RenderDataList)(NodeIndex);
							if( !RenderData.bIsTwoSided )
							{
								const INT NumTriangles = RenderData.NumVertices - 2;
								if (NumTriangles > NumAvailablePrimitivesInBuffer)
								{
									FlushDynamicIndexBuffer(FALSE);
									GrabDynamicIndexBuffer();

									RenderMesh.bDrawnShared = TRUE;

									Indices16 = &(BatchContext.CurrentIndex<WORD>());
								}
								const unsigned short BaseIndex = RenderData.VertexStart;
								switch(NumTriangles)
								{
								case 1:
									*Indices16++ = (BaseIndex);
									*Indices16++ = (BaseIndex + 2);
									*Indices16++ = (BaseIndex + 1);
									break;
								case 2:
									*Indices16++ = (BaseIndex);
									*Indices16++ = (BaseIndex + 2);
									*Indices16++ = (BaseIndex + 1);
									*Indices16++ = (BaseIndex);
									*Indices16++ = (BaseIndex + 3);
									*Indices16++ = (BaseIndex + 2);
									break;
								default:
									for(INT VertexIndex = 2;VertexIndex < RenderData.NumVertices;VertexIndex++)
									{
										*Indices16++ = (BaseIndex);
										*Indices16++ = (BaseIndex + VertexIndex);
										*Indices16++ = (BaseIndex + VertexIndex - 1);							
									}
									break;
								}
								BatchContext.NumIndices += NumTriangles * 3;
								NumAvailablePrimitivesInBuffer -= NumTriangles;
								BatchContext.MinVertexIndex = Min<UINT>(BaseIndex, BatchContext.MinVertexIndex);
								BatchContext.MaxVertexIndex = Max<UINT>(BaseIndex + RenderData.NumVertices - 1, BatchContext.MaxVertexIndex);
							}
							else
							{
								const INT NumTriangles = (RenderData.NumVertices - 2) * 2;
								if (NumTriangles > NumAvailablePrimitivesInBuffer)
								{
									FlushDynamicIndexBuffer(FALSE);
									GrabDynamicIndexBuffer();

									RenderMesh.bDrawnShared = TRUE;

									Indices16 = &(BatchContext.CurrentIndex<WORD>());
								}
								const UINT BaseIndex	= RenderData.VertexStart;
								const UINT BaseIndex2	= BaseIndex + RenderData.NumVertices;
								for(INT VertexIndex = 2;VertexIndex < RenderData.NumVertices;VertexIndex++)
								{
									*Indices16++ = (BaseIndex);
									*Indices16++ = (BaseIndex + VertexIndex);
									*Indices16++ = (BaseIndex + VertexIndex - 1);

									*Indices16++ = (BaseIndex2);
									*Indices16++ = (BaseIndex2 + VertexIndex);
									*Indices16++ = (BaseIndex2 + VertexIndex - 1);
								}
								BatchContext.NumIndices += NumTriangles * 3;
								NumAvailablePrimitivesInBuffer -= NumTriangles;
								BatchContext.MinVertexIndex = Min<UINT>(BaseIndex, BatchContext.MinVertexIndex);
								BatchContext.MaxVertexIndex = Max<UINT>(BaseIndex2 + RenderData.NumVertices - 1, BatchContext.MaxVertexIndex);
							}
						}				
						// 다 써버렸네! Flush :)
						if (!NumAvailablePrimitivesInBuffer)
						{
							FlushDynamicIndexBuffer( FALSE );
						}				
					}		
				} // lightmap loop
			} // material loop
			break;
		case sizeof(DWORD):
			for(FMaterialInstance *Material = GFirstMaterial; Material; Material = Material->Next )
			{
				// draw setting
				RenderMesh.BaseVertexIndex	= 0;
				RenderMesh.MaterialInstance = Material;		
				RenderMesh.LCI = NULL;

				// for each LCI
				for( const FModelElement *ElementOuter = (const FModelElement*)Material->FirstElement; ElementOuter; ElementOuter = ElementOuter->Sibling )
				{				
					// for each elements
					for( const FModelElement *Element = ElementOuter; Element; Element = Element->Next )
					{

						//PREFETCH( Element->Next );

						GrabDynamicIndexBuffer();

						//<@ ava specific ; 2007. 11. 14 changmin
						// add cascaded shadow ; sun lit culling
						const TArray<FBspNodeRenderData>* RenderDataList = NULL;
						if( bUseNodeCulling )
						{
							RenderDataList = &Element->SunLitNodeRenderData;
							if( Element->Next )
							{
								PREFETCH( Element->Next->SunLitNodeRenderData.GetTypedData() );
							}
						}
						else
						{
							RenderDataList = &Element->NodeRenderData;
							if( Element->Next )
							{
								PREFETCH( Element->Next->NodeRenderData.GetTypedData() );
							}
						}
						//>@ ava

						DWORD *Indices32 = &(BatchContext.CurrentIndex<DWORD>());
						for (INT NodeIndex = 0; NodeIndex < RenderDataList->Num(); ++NodeIndex)
						{
							const FBspNodeRenderData &RenderData = (*RenderDataList)(NodeIndex);
							if( !RenderData.bIsTwoSided )
							{
								const INT NumTriangles = RenderData.NumVertices - 2;
								if (NumTriangles > NumAvailablePrimitivesInBuffer)
								{
									FlushDynamicIndexBuffer(FALSE);
									GrabDynamicIndexBuffer();
									Indices32 = &(BatchContext.CurrentIndex<DWORD>());
								}
								const UINT BaseIndex = RenderData.VertexStart;
								switch(NumTriangles)
								{
								case 1:
									*Indices32++ = (BaseIndex);
									*Indices32++ = (BaseIndex + 2);
									*Indices32++ = (BaseIndex + 1);
									break;
								case 2:
									*Indices32++ = (BaseIndex);
									*Indices32++ = (BaseIndex + 2);
									*Indices32++ = (BaseIndex + 1);
									*Indices32++ = (BaseIndex);
									*Indices32++ = (BaseIndex + 3);
									*Indices32++ = (BaseIndex + 2);
									break;
								default:
									for(INT VertexIndex = 2;VertexIndex < RenderData.NumVertices;VertexIndex++)
									{
										*Indices32++ = (BaseIndex);
										*Indices32++ = (BaseIndex + VertexIndex);
										*Indices32++ = (BaseIndex + VertexIndex - 1);							
									}
									break;
								}
								BatchContext.NumIndices += NumTriangles * 3;
								NumAvailablePrimitivesInBuffer -= NumTriangles;
								BatchContext.MinVertexIndex = Min(BaseIndex, BatchContext.MinVertexIndex);
								BatchContext.MaxVertexIndex = Max(BaseIndex + RenderData.NumVertices - 1, BatchContext.MaxVertexIndex);
							}
							else
							{
								const INT NumTriangles = (RenderData.NumVertices - 2) * 2;
								if (NumTriangles > NumAvailablePrimitivesInBuffer)
								{
									FlushDynamicIndexBuffer(FALSE);
									GrabDynamicIndexBuffer();
									Indices32 = &(BatchContext.CurrentIndex<DWORD>());
								}
								const UINT BaseIndex	= RenderData.VertexStart;
								const UINT BaseIndex2	= BaseIndex + RenderData.NumVertices;
								for(INT VertexIndex = 2;VertexIndex < RenderData.NumVertices;VertexIndex++)
								{
									*Indices32++ = (BaseIndex);
									*Indices32++ = (BaseIndex + VertexIndex);
									*Indices32++ = (BaseIndex + VertexIndex - 1);
									*Indices32++ = (BaseIndex2);
									*Indices32++ = (BaseIndex2 + VertexIndex);
									*Indices32++ = (BaseIndex2 + VertexIndex - 1);
								}
								BatchContext.NumIndices += NumTriangles * 3;
								NumAvailablePrimitivesInBuffer -= NumTriangles;
								BatchContext.MinVertexIndex = Min(BaseIndex, BatchContext.MinVertexIndex);
								BatchContext.MaxVertexIndex = Max(BaseIndex2 + RenderData.NumVertices - 1, BatchContext.MaxVertexIndex);
							}
						}				
						// 다 써버렸네! Flush :)
						if (!NumAvailablePrimitivesInBuffer)
						{
							FlushDynamicIndexBuffer( FALSE );
						}				
					}		
				} // lightmap loop
			} // material loop
			break;
		}
		
		break;

	case BSP_EMISSIVE:
		for(FMaterialInstance *Material = GFirstMaterial; Material; Material = Material->Next )
		{
			const INT MaterialVertexBase	= Material->MinVertexIndex;
			const INT MaterialVertexCount	= Material->MaxVertexIndex - MaterialVertexBase + 1;

			FlushDynamicIndexBuffer( FALSE );
			// initialize values for this material batch
			RenderMesh.BaseVertexIndex = MaterialVertexBase;
			RenderMesh.MaterialInstance = Material;

			// 이 단위로 Index buffer format을 결정합니다.
			if ( MaterialVertexCount > 0xffff)
			{
				IndexSize = sizeof(DWORD);
			}
			else
			{
				IndexSize = sizeof(WORD);
			}

			switch( IndexSize )
			{
			case sizeof(WORD):
				// for each LCI
				for( const FModelElement *ElementOuter = (const FModelElement*)Material->FirstElement; ElementOuter; ElementOuter = ElementOuter->Sibling )
				{				
					FlushDynamicIndexBuffer( FALSE );

					// set light cache
					LightCache.BindElement( *ElementOuter );
					RenderMesh.LCI = &LightCache;

					// for each elements
					for( const FModelElement *Element = ElementOuter; Element; Element = Element->Next )
					{
						if( Element->Next )
						{
							PREFETCH( Element->Next->NodeRenderData.GetTypedData() );
						}
						//PREFETCH( Element->Next );

						GrabDynamicIndexBuffer();

						WORD	*Indices16 = &(BatchContext.CurrentIndex<WORD>());
						for (INT NodeIndex = 0; NodeIndex < Element->NodeRenderData.Num(); ++NodeIndex)
						{
							const FBspNodeRenderData &RenderData = Element->NodeRenderData(NodeIndex);
							if( !RenderData.bIsTwoSided )
							{
								const INT NumTriangles = RenderData.NumVertices - 2;
								if (NumTriangles > NumAvailablePrimitivesInBuffer)
								{
									FlushDynamicIndexBuffer(FALSE);
									GrabDynamicIndexBuffer();
									Indices16 = &(BatchContext.CurrentIndex<WORD>());
								}
								check( RenderData.VertexStart >= MaterialVertexBase );
								const unsigned short BaseIndex = RenderData.VertexStart - MaterialVertexBase;
								switch(NumTriangles)
								{
								case 1:
									*Indices16++ = (BaseIndex);
									*Indices16++ = (BaseIndex + 2);
									*Indices16++ = (BaseIndex + 1);
									break;
								case 2:
									*Indices16++ = (BaseIndex);
									*Indices16++ = (BaseIndex + 2);
									*Indices16++ = (BaseIndex + 1);
									*Indices16++ = (BaseIndex);
									*Indices16++ = (BaseIndex + 3);
									*Indices16++ = (BaseIndex + 2);
									break;
								default:
									for(INT VertexIndex = 2;VertexIndex < RenderData.NumVertices;VertexIndex++)
									{
										*Indices16++ = (BaseIndex);
										*Indices16++ = (BaseIndex + VertexIndex);
										*Indices16++ = (BaseIndex + VertexIndex - 1);							
									}
									break;
								}
								BatchContext.NumIndices += NumTriangles * 3;
								NumAvailablePrimitivesInBuffer -= NumTriangles;
								BatchContext.MinVertexIndex = Min<UINT>(BaseIndex, BatchContext.MinVertexIndex);
								BatchContext.MaxVertexIndex = Max<UINT>(BaseIndex + RenderData.NumVertices - 1, BatchContext.MaxVertexIndex);
							}
							else
							{
								const INT NumTriangles = (RenderData.NumVertices - 2) * 2;
								if (NumTriangles > NumAvailablePrimitivesInBuffer)
								{
									FlushDynamicIndexBuffer(FALSE);
									GrabDynamicIndexBuffer();
									Indices16 = &(BatchContext.CurrentIndex<WORD>());
								}
								check( RenderData.VertexStart >= MaterialVertexBase );
								const unsigned short BaseIndex	= RenderData.VertexStart - MaterialVertexBase;
								const unsigned short BaseIndex2	= BaseIndex + RenderData.NumVertices;
								for(INT VertexIndex = 2;VertexIndex < RenderData.NumVertices;VertexIndex++)
								{
									*Indices16++ = (BaseIndex);
									*Indices16++ = (BaseIndex + VertexIndex);
									*Indices16++ = (BaseIndex + VertexIndex - 1);

									*Indices16++ = (BaseIndex2);
									*Indices16++ = (BaseIndex2 + VertexIndex);
									*Indices16++ = (BaseIndex2 + VertexIndex - 1);
								}
								BatchContext.NumIndices += NumTriangles * 3;
								NumAvailablePrimitivesInBuffer -= NumTriangles;
								BatchContext.MinVertexIndex = Min<UINT>(BaseIndex, BatchContext.MinVertexIndex);
								BatchContext.MaxVertexIndex = Max<UINT>(BaseIndex2 + RenderData.NumVertices - 1, BatchContext.MaxVertexIndex);
							}
						}				

						// 다 써버렸네! Flush :)
						if (!NumAvailablePrimitivesInBuffer)
						{
							FlushDynamicIndexBuffer( FALSE );
						}				
					}		
				} // lightmap loop
				break;
			case sizeof(DWORD):
				// for each LCI
				for( const FModelElement *ElementOuter = (const FModelElement*)Material->FirstElement; ElementOuter; ElementOuter = ElementOuter->Sibling )
				{				
					FlushDynamicIndexBuffer( FALSE );
					// set light cache
					LightCache.BindElement( *ElementOuter );
					RenderMesh.LCI = &LightCache;

					// for each elements
					for( const FModelElement *Element = ElementOuter; Element; Element = Element->Next )
					{
						if( Element->Next )
						{
							PREFETCH( Element->Next->NodeRenderData.GetTypedData() );
						}
						//PREFETCH( Element->Next );

						GrabDynamicIndexBuffer();

						DWORD	*Indices32 = &(BatchContext.CurrentIndex<DWORD>());
						for (INT NodeIndex = 0; NodeIndex < Element->NodeRenderData.Num(); ++NodeIndex)
						{
							const FBspNodeRenderData &RenderData = Element->NodeRenderData(NodeIndex);
							if( !RenderData.bIsTwoSided )
							{
								const INT NumTriangles = RenderData.NumVertices - 2;
								if (NumTriangles > NumAvailablePrimitivesInBuffer)
								{
									FlushDynamicIndexBuffer(FALSE);
									GrabDynamicIndexBuffer();
									Indices32 = &(BatchContext.CurrentIndex<DWORD>());
								}
								check( RenderData.VertexStart >= MaterialVertexBase );
								const UINT BaseIndex = RenderData.VertexStart - MaterialVertexBase;
								switch(NumTriangles)
								{
								case 1:
									*Indices32++ = (BaseIndex);
									*Indices32++ = (BaseIndex + 2);
									*Indices32++ = (BaseIndex + 1);
									break;
								case 2:
									*Indices32++ = (BaseIndex);
									*Indices32++ = (BaseIndex + 2);
									*Indices32++ = (BaseIndex + 1);
									*Indices32++ = (BaseIndex);
									*Indices32++ = (BaseIndex + 3);
									*Indices32++ = (BaseIndex + 2);
									break;
								default:
									for(INT VertexIndex = 2;VertexIndex < RenderData.NumVertices;VertexIndex++)
									{
										*Indices32++ = (BaseIndex);
										*Indices32++ = (BaseIndex + VertexIndex);
										*Indices32++ = (BaseIndex + VertexIndex - 1);							
									}
									break;
								}
								BatchContext.NumIndices += NumTriangles * 3;
								NumAvailablePrimitivesInBuffer -= NumTriangles;
								BatchContext.MinVertexIndex = Min(BaseIndex, BatchContext.MinVertexIndex);
								BatchContext.MaxVertexIndex = Max(BaseIndex + RenderData.NumVertices - 1, BatchContext.MaxVertexIndex);
							}
							else
							{
								const INT NumTriangles = (RenderData.NumVertices - 2) * 2;
								if (NumTriangles > NumAvailablePrimitivesInBuffer)
								{
									FlushDynamicIndexBuffer(FALSE);
									GrabDynamicIndexBuffer();
									Indices32 = &(BatchContext.CurrentIndex<DWORD>());
								}
								check( RenderData.VertexStart >= MaterialVertexBase );
								const UINT BaseIndex = RenderData.VertexStart - MaterialVertexBase;
								const UINT BaseIndex2 = BaseIndex + RenderData.NumVertices;
								for(INT VertexIndex = 2;VertexIndex < RenderData.NumVertices;VertexIndex++)
								{
									*Indices32++ = (BaseIndex);
									*Indices32++ = (BaseIndex + VertexIndex);
									*Indices32++ = (BaseIndex + VertexIndex - 1);							

									*Indices32++ = (BaseIndex2);
									*Indices32++ = (BaseIndex2 + VertexIndex);
									*Indices32++ = (BaseIndex2 + VertexIndex - 1);
								}
								BatchContext.NumIndices += NumTriangles * 3;
								NumAvailablePrimitivesInBuffer -= NumTriangles;
								BatchContext.MinVertexIndex = Min(BaseIndex, BatchContext.MinVertexIndex);
								BatchContext.MaxVertexIndex = Max(BaseIndex2 + RenderData.NumVertices - 1, BatchContext.MaxVertexIndex);
							}
						}				

						// 다 써버렸네! Flush :)
						if (!NumAvailablePrimitivesInBuffer)
						{
							FlushDynamicIndexBuffer( FALSE );
						}				
					}		
				} // lightmap loop
				break;
			} // end of switch(IndexSize)
		} // material loop
		break;

	case BSP_LIGHTING:
		for(FMaterialInstance *Material = GFirstMaterial; Material; Material = Material->Next )
		{
			const INT MaterialVertexBase	= Material->MinVertexIndex;
			const INT MaterialVertexCount	= Material->MaxVertexIndex - MaterialVertexBase + 1;

			FlushDynamicIndexBuffer( FALSE );

			// initialize values for this material batch
			RenderMesh.MaterialInstance = Material;		
			RenderMesh.BaseVertexIndex = MaterialVertexBase;

			// 이 단위로 Index buffer format을 결정합니다.
			if ( MaterialVertexCount > 0xffff)
			{
				IndexSize = sizeof(DWORD);
			}
			else
			{
				IndexSize = sizeof(WORD);
			}		

			// for each LCI
			for( const FModelElement *ElementOuter = (const FModelElement*)Material->FirstElement; ElementOuter; ElementOuter = ElementOuter->Sibling )
			{				
				// for each elements
				for( const FModelElement *Element = ElementOuter; Element; Element = Element->Next )
				{
					PREFETCH( Element->Next );

					// 일단 무조건 flush하자. :)
					//FlushDynamicIndexBuffer( FALSE );
					LightCache.BindElement( *Element );
					RenderMesh.LCI = &LightCache;


					GrabDynamicIndexBuffer();			

					WORD	*Indices16 = &(BatchContext.CurrentIndex<WORD>());
					DWORD	*Indices32 = &(BatchContext.CurrentIndex<DWORD>());

					//<@ ava specific ; 2007. 11. 14 changmin
					// add cascaded shadow ; sun lit culling
					const TArray<FBspNodeRenderData>* RenderDataList = NULL;
					if( bUseNodeCulling )
					{
						RenderDataList = &Element->SunLitNodeRenderData;
					}
					else
					{
						RenderDataList = &Element->NodeRenderData;
					}
					//>@ ava
					for (INT NodeIndex = 0; NodeIndex < RenderDataList->Num(); ++NodeIndex)
					{
						const FBspNodeRenderData &RenderData = (*RenderDataList)(NodeIndex);
						if( !RenderData.bIsTwoSided )
						{
							const INT NumTriangles = RenderData.NumVertices - 2;
							if (NumTriangles > NumAvailablePrimitivesInBuffer)
							{
								FlushDynamicIndexBuffer(FALSE);
								GrabDynamicIndexBuffer();
								Indices16 = &(BatchContext.CurrentIndex<WORD>());
								Indices32 = &(BatchContext.CurrentIndex<DWORD>());
							}
							check( RenderData.VertexStart >= MaterialVertexBase );
							const UINT BaseIndex = RenderData.VertexStart - MaterialVertexBase;

							if (IndexSize == sizeof(WORD))
							{
								switch(NumTriangles)
								{
								case 1:
									*Indices16++ = (BaseIndex);
									*Indices16++ = (BaseIndex + 2);
									*Indices16++ = (BaseIndex + 1);
									break;
								case 2:
									*Indices16++ = (BaseIndex);
									*Indices16++ = (BaseIndex + 2);
									*Indices16++ = (BaseIndex + 1);
									*Indices16++ = (BaseIndex);
									*Indices16++ = (BaseIndex + 3);
									*Indices16++ = (BaseIndex + 2);
									break;
								default:
									for(INT VertexIndex = 2;VertexIndex < RenderData.NumVertices;VertexIndex++)
									{
										*Indices16++ = (BaseIndex);
										*Indices16++ = (BaseIndex + VertexIndex);
										*Indices16++ = (BaseIndex + VertexIndex - 1);							
									}
									break;
								}
							}
							else if (IndexSize == sizeof(DWORD))
							{
								switch(NumTriangles)
								{
								case 1:
									*Indices32++ = (BaseIndex);
									*Indices32++ = (BaseIndex + 2);
									*Indices32++ = (BaseIndex + 1);
									break;
								case 2:
									*Indices32++ = (BaseIndex);
									*Indices32++ = (BaseIndex + 2);
									*Indices32++ = (BaseIndex + 1);
									*Indices32++ = (BaseIndex);
									*Indices32++ = (BaseIndex + 3);
									*Indices32++ = (BaseIndex + 2);
									break;
								default:
									for(INT VertexIndex = 2;VertexIndex < RenderData.NumVertices;VertexIndex++)
									{
										*Indices32++ = (BaseIndex);
										*Indices32++ = (BaseIndex + VertexIndex);
										*Indices32++ = (BaseIndex + VertexIndex - 1);							
									}
									break;
								}
							}

							BatchContext.NumIndices += NumTriangles * 3;
							NumAvailablePrimitivesInBuffer -= NumTriangles;
							BatchContext.MinVertexIndex = Min<UINT>(BaseIndex, BatchContext.MinVertexIndex);
							BatchContext.MaxVertexIndex = Max<UINT>(BaseIndex + RenderData.NumVertices - 1, BatchContext.MaxVertexIndex);
						}
						else
						{
							const INT NumTriangles = (RenderData.NumVertices - 2) * 2;
							if (NumTriangles > NumAvailablePrimitivesInBuffer)
							{
								FlushDynamicIndexBuffer(FALSE);
								GrabDynamicIndexBuffer();
								Indices16 = &(BatchContext.CurrentIndex<WORD>());
								Indices32 = &(BatchContext.CurrentIndex<DWORD>());
							}
							check( RenderData.VertexStart >= MaterialVertexBase );
							const UINT BaseIndex = RenderData.VertexStart - MaterialVertexBase;
							const UINT BaseIndex2 = BaseIndex + RenderData.NumVertices;
							if (IndexSize == sizeof(WORD))
							{
								for(INT VertexIndex = 2;VertexIndex < RenderData.NumVertices;VertexIndex++)
								{
									*Indices16++ = (BaseIndex);
									*Indices16++ = (BaseIndex + VertexIndex);
									*Indices16++ = (BaseIndex + VertexIndex - 1);

									*Indices16++ = (BaseIndex2);
									*Indices16++ = (BaseIndex2 + VertexIndex);
									*Indices16++ = (BaseIndex2 + VertexIndex - 1);
								}
							}
							else if (IndexSize == sizeof(DWORD))
							{
								for(INT VertexIndex = 2;VertexIndex < RenderData.NumVertices;VertexIndex++)
								{
									*Indices32++ = (BaseIndex);
									*Indices32++ = (BaseIndex + VertexIndex);
									*Indices32++ = (BaseIndex + VertexIndex - 1);							

									*Indices32++ = (BaseIndex2);
									*Indices32++ = (BaseIndex2 + VertexIndex);
									*Indices32++ = (BaseIndex2 + VertexIndex - 1);
								}
							}
							BatchContext.NumIndices += NumTriangles * 3;
							NumAvailablePrimitivesInBuffer -= NumTriangles;
							BatchContext.MinVertexIndex = Min(BaseIndex, BatchContext.MinVertexIndex);
							BatchContext.MaxVertexIndex = Max(BaseIndex2 + RenderData.NumVertices - 1, BatchContext.MaxVertexIndex);

						}
					}				

					// 다 써버렸네! Flush :)
					if (!NumAvailablePrimitivesInBuffer)
					{
						FlushDynamicIndexBuffer( FALSE );
					}				
				}		
			} // lightmap loop
		} // material loop

		break;

	default:
		check(0);
		break;
	}

	// 마지막 :)
	FlushDynamicIndexBuffer( TRUE );

	if( CurrentPhase == BSP_EMISSIVE )
	{
		if( MaterialCount )
			PreviousMaterialCount = MaterialCount;
	}

	CurrentPhase = BSP_NODRAW;
}
