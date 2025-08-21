#ifndef DECAL_RENDERING_H
#define DECAL_RENDERING_H

struct FDrawBatchedDecalContext
{
	FCommandContextRHI* Context;
	FPrimitiveDrawInterface* PDI;
	FMeshElement* MeshElement;		
	FLinearColor LevelColor, PropertyColor;
	FPrimitiveSceneInfo* PrimitiveSceneInfo;
};

void DrawBatchedDecals(
					   FDrawBatchedDecalContext* Context,		
					   INT NumVerticesToRender,
					   INT NumIndicesToRender,
					   const TArray<FDecalInteraction*>& DPGDecals,
					   INT& DecalIndex,
					   INT Batch,
					   UBOOL bUsingVertexLightmapping = FALSE
					   );

UBOOL IsAffordableForDecalBatcher( INT NumVertices, INT NumIndices );

#endif