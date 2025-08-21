/*=============================================================================
	ParticleSubUVVertexFactory.h: Particle SubUV vertex factory definitions.
	Copyright 2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/**
 *	
 */
class FParticleSubUVVertexFactory : public FParticleVertexFactory
{
	//@todo.SAS. Need to implement this once VertexFactory classes can declare 'non-compilable' materials.
	DECLARE_VERTEX_FACTORY_TYPE(FParticleSubUVVertexFactory);

public:
	/**
	 * Should we cache the material's shadertype on this platform with this vertex factory? 
	 */
	static UBOOL ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType);

	// FRenderResource interface.
	virtual void InitRHI();

	static UBOOL CreateCachedSubUVParticleDeclaration();

protected:
	/** The RHI vertex declaration used to render the factory normally. */
	static FVertexDeclarationRHIRef SubUVParticleDeclaration;

private:
};
