#include "EngineMaterialClasses.h"

extern INT GBSPDynamicBatchCount;

enum EBSPRenderPhase 
{ 
	BSP_NODRAW, 
	BSP_OVERRIDEMATERIAL, 
	BSP_EMISSIVE, 
	BSP_LIGHTING,
};

extern EBSPRenderPhase CurrentPhase;
extern const ULightComponent* GCurrentLightComponent;
extern FMaterialInstance* GFirstMaterial;
extern INT GNumMaterials;
extern UINT GMaxBspVertexIndex;

void BspRendering_StartBatch( EBSPRenderPhase Phase, const ULightComponent* LightComponent = NULL );
FORCEINLINE EBSPRenderPhase BspRendering_GetPhase()
{
	return CurrentPhase;
}

FORCEINLINE UBOOL UsingSameLightmapPage( const FModelElement* A, const FModelElement* B )
{
	const FLightMap *LightMapA = A->LightMap;
	const FLightMap *LightMapB = B->LightMap;

	if (LightMapA == LightMapB) return TRUE;
	if (!LightMapA || !LightMapB) return FALSE;

	if (LightMapA->GetLightMap2D()->GetTexture(0) == LightMapB->GetLightMap2D()->GetTexture(0)) return TRUE;

	return FALSE;
}

FORCEINLINE void BspRendering_Add( const FModelElement& ModelElement )
{
	{
		SCOPE_CYCLE_COUNTER(STAT_BSPAddTime);
		if( CurrentPhase == BSP_NODRAW )
		{
			return;
		}

		if( CurrentPhase == BSP_LIGHTING )
		{
			if (!GCurrentLightComponent->AffectsBounds(ModelElement.BoundingBox))
				return;
		}

		UMaterialInstance *MaterialInstance = ModelElement.Material ? ModelElement.Material : GEngine->DefaultMaterial;

		FMaterialInstance *RenderMaterial = NULL;
		UMaterial *SimpleMaterial = Cast<UMaterial>(MaterialInstance);
		UMaterialInstanceConstant *MIC = Cast<UMaterialInstanceConstant>(MaterialInstance);

		if( SimpleMaterial )
		{
#if FINAL_RELEASE
			if (!GIsEditor && SimpleMaterial->bUsedAsSpecialEngineMaterial)
				return;
#endif

			RenderMaterial = SimpleMaterial->GetInstanceInterface(FALSE);
		}
		else if( MIC )
		{
			RenderMaterial = MIC->GetInstanceInterface( FALSE );
		}
		else
		{
			// error!!!
			check(0 && TEXT("Error : ...") );
		}

#if FINAL_RELEASE
		if (!GIsEditor && !RenderMaterial)
			return;
#endif

		// only opaque for depth drawing :)
		if (CurrentPhase == BSP_OVERRIDEMATERIAL && (!RenderMaterial || RenderMaterial->GetMaterial()->GetBlendMode() != BLEND_Opaque))
			return;

		if( CurrentPhase == BSP_OVERRIDEMATERIAL )
		{
			RenderMaterial = GEngine->DefaultMaterial->GetInstanceInterface(FALSE);
		}

		//FMaterialInstance *RenderMaterial = MaterialInstance->GetMaterial()->GetInstanceInterface(FALSE);

		// first access in this frame.
		if( RenderMaterial->FrameCount != GBSPDynamicBatchCount )
		{		
			GNumMaterials++;

			// add to material list.
			RenderMaterial->Next = GFirstMaterial;
			GFirstMaterial = RenderMaterial;
			RenderMaterial->FrameCount = GBSPDynamicBatchCount;

			// initialize render batch
			RenderMaterial->FirstElement = NULL;
			RenderMaterial->MinVertexIndex = 0xffffffff;
			RenderMaterial->MaxVertexIndex = 0;
		}

		// 같은 page를 사용하는 것을 발견하면 그것의 list에 prepend
		// Material -> FirstElement -> Next -> Next [Same lightmap page]
		//			    | 
		//			   Sibling -> Next -> Next
		//				|
		//			   Sibling...
		FModelElement const** List = (FModelElement const**)&RenderMaterial->FirstElement;	

		for (;;)
		{
			if (!*List)
			{
				*List = &ModelElement;
				ModelElement.Sibling = NULL;
				ModelElement.Next = NULL;
				break;
			}
			else  
			{
				// Emissive pass에서만 lightmap이 같은지 검사한다.
				if (CurrentPhase != BSP_EMISSIVE || UsingSameLightmapPage( &ModelElement, *List ))
				{
					ModelElement.Next = *List;			
					ModelElement.Sibling = (*List)->Sibling;
					(*List) = &ModelElement;			
					break;
				}
				else
				{
					List = &(*List)->Sibling;
				}
			}
		}	

		RenderMaterial->MinVertexIndex = Min( RenderMaterial->MinVertexIndex, ModelElement.MinVertexIndex );
		RenderMaterial->MaxVertexIndex = Max( RenderMaterial->MaxVertexIndex, ModelElement.MaxVertexIndex );

		GMaxBspVertexIndex = Max( GMaxBspVertexIndex, ModelElement.MaxVertexIndex );
	}
}

void BspRendering_EndBatch( FPrimitiveDrawInterface *PDI );
extern UBOOL GUsingBSPDynamicBatch;
