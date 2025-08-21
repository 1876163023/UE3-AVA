/*=============================================================================
	LocalVertexFactory.cpp: Local vertex factory implementation
	Copyright 2005-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "LocalVertexFactoryShaderParms.h"

void FLocalVertexFactory::InitRHI()
{
	if(Data.PositionComponent.VertexBuffer != Data.TangentBasisComponents[0].VertexBuffer)
	{
		FVertexDeclarationElementList PositionOnlyStreamElements;
		PositionOnlyStreamElements.AddItem(AccessPositionStreamComponent(Data.PositionComponent,VEU_Position));
		InitPositionDeclaration(PositionOnlyStreamElements);
	}

	FVertexDeclarationElementList Elements;
	if(Data.PositionComponent.VertexBuffer != NULL)
	{
		Elements.AddItem(AccessStreamComponent(Data.PositionComponent,VEU_Position));
	}

	EVertexElementUsage TangentBasisUsages[3] = { VEU_Tangent, VEU_Binormal, VEU_Normal };
	for(INT AxisIndex = 0;AxisIndex < 3;AxisIndex++)
	{
		if(Data.TangentBasisComponents[AxisIndex].VertexBuffer != NULL)
		{
			Elements.AddItem(AccessStreamComponent(Data.TangentBasisComponents[AxisIndex],TangentBasisUsages[AxisIndex]));
		}
	}

	if(Data.TextureCoordinates.Num())
	{
		for(UINT CoordinateIndex = 0;CoordinateIndex < Data.TextureCoordinates.Num();CoordinateIndex++)
		{
			Elements.AddItem(AccessStreamComponent(
				Data.TextureCoordinates(CoordinateIndex),
				VEU_TextureCoordinate,
				CoordinateIndex
				));
		}

		for(UINT CoordinateIndex = Data.TextureCoordinates.Num();CoordinateIndex < MAX_TEXCOORDS;CoordinateIndex++)
		{
			Elements.AddItem(AccessStreamComponent(
				Data.TextureCoordinates(Data.TextureCoordinates.Num() - 1),
				VEU_TextureCoordinate,
				CoordinateIndex
				));
		}
	}

	if(Data.ShadowMapCoordinateComponent.VertexBuffer)
	{
		Elements.AddItem(AccessStreamComponent(Data.ShadowMapCoordinateComponent,VEU_Color));
	}
	else if(Data.TextureCoordinates.Num())
	{
		Elements.AddItem(AccessStreamComponent(Data.TextureCoordinates(0),VEU_Color));
	}

	InitDeclaration(Elements);
}

IMPLEMENT_VERTEX_FACTORY_TYPE(FLocalVertexFactory,FLocalVertexFactoryShaderParameters,"LocalVertexFactory",TRUE,TRUE, VER_SIMPLE_ELEMENT_SHADER_RECOMPILE);
IMPLEMENT_VERTEX_FACTORY_TYPE(FLocalShadowVertexFactory,FLocalVertexFactoryShaderParameters,"LocalVertexFactory",FALSE,FALSE, VER_SIMPLE_ELEMENT_SHADER_RECOMPILE);
