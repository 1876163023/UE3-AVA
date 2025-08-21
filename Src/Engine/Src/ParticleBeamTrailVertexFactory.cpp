/*=============================================================================
	ParticleBeamTrailVertexFactory.cpp: Particle vertex factory implementation.
	Copyright 2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
#include "EnginePrivate.h"

/** The RHI vertex declaration used to render the factory normally. */
FVertexDeclarationRHIRef FParticleBeamTrailVertexFactory::BeamTrailParticleDeclaration;

UBOOL FParticleBeamTrailVertexFactory::ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType)
{
	return Material->IsUsedWithParticleSystem() || Material->IsSpecialEngineMaterial();
}

/**
 *	Initialize the Render Hardare Interface for this vertex factory
 */
void FParticleBeamTrailVertexFactory::InitRHI()
{
	CreateCachedBeamTrailParticleDeclaration();
	SetDeclaration(BeamTrailParticleDeclaration);
}

UBOOL FParticleBeamTrailVertexFactory::CreateCachedBeamTrailParticleDeclaration()
{
	if (IsValidRef(BeamTrailParticleDeclaration) == FALSE)
	{
		FVertexDeclarationElementList Elements;

		INT	Offset = 0;
		/** The stream to read the vertex position from.		*/
		Elements.AddItem(FVertexElement(0,Offset,VET_Float3,VEU_Position,0));
		Offset += sizeof(FLOAT) * 3;
		/** The stream to read the vertex old position from.	*/
		Elements.AddItem(FVertexElement(0,Offset,VET_Float3,VEU_Normal,0));
		Offset += sizeof(FLOAT) * 3;
		/** The stream to read the vertex size from.			*/
		Elements.AddItem(FVertexElement(0,Offset,VET_Float3,VEU_Tangent,0));
		Offset += sizeof(FLOAT) * 3;
		/** The stream to read the texture coordinates from.	*/
		Elements.AddItem(FVertexElement(0,Offset,VET_Float2,VEU_TextureCoordinate,0));
		Offset += sizeof(FLOAT) * 2;
		/** The stream to read the rotation from.				*/
		Elements.AddItem(FVertexElement(0,Offset,VET_Float1,VEU_BlendWeight,0));
		Offset += sizeof(FLOAT) * 1;
		/** The stream to read the color from.					*/
		Elements.AddItem(FVertexElement(0,Offset,VET_Float4,VEU_TextureCoordinate,1));
		Offset += sizeof(FLOAT) * 4;

		// Create the vertex declaration for rendering the factory normally.
		BeamTrailParticleDeclaration = RHICreateVertexDeclaration(Elements);
	}

	return TRUE;
}

void FParticleBeamTrailVertexFactoryShaderParameters::Set(FCommandContextRHI* Context,FShader* VertexShader,const FVertexFactory* VertexFactory,const FSceneView* View) const
{
	FParticleBeamTrailVertexFactory* ParticleVF = (FParticleBeamTrailVertexFactory*)VertexFactory;

	FVector4	CameraRight, CameraUp;

	SetVertexShaderValue(Context,VertexShader->GetVertexShader(),CameraWorldPositionParameter,FVector4(View->ViewOrigin, 0.0f));

	if (CameraRightParameter.IsBound() || CameraUpParameter.IsBound())
	{
		if (ParticleVF->GetLockAxis() == TRUE)
		{
			CameraUp	= FVector4(ParticleVF->GetLockAxisUp(), 0.0f);
			CameraRight	= FVector4(ParticleVF->GetLockAxisRight(), 0.0f);
		}
		else
		{
			CameraUp	= -View->InvViewProjectionMatrix.TransformNormal(FVector(1.0f,0.0f,0.0f)).SafeNormal();
			CameraRight	= -View->InvViewProjectionMatrix.TransformNormal(FVector(0.0f,1.0f,0.0f)).SafeNormal();
		}
		SetVertexShaderValue(Context,VertexShader->GetVertexShader(),CameraRightParameter,CameraRight);
		SetVertexShaderValue(Context,VertexShader->GetVertexShader(),CameraUpParameter,CameraUp);
	}

	SetVertexShaderValue(Context,VertexShader->GetVertexShader(),ScreenAlignmentParameter,FVector4((FLOAT)ParticleVF->GetScreenAlignment(),0.0f,0.0f,0.0f));
}

IMPLEMENT_VERTEX_FACTORY_TYPE(FParticleBeamTrailVertexFactory,FParticleBeamTrailVertexFactoryShaderParameters,"ParticleBeamTrailVertexFactory",TRUE,FALSE, VER_SIMPLE_ELEMENT_SHADER_RECOMPILE);
