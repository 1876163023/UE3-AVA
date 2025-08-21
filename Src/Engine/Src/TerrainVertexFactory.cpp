/*=============================================================================
	TerrainVertexFactory.cpp: Terrain vertex factory implementation.
	Copyright 2005-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "UnTerrain.h"
#include "UnTerrainRender.h"

/** Vertex factory with vertex stream components for terrain vertices */
// FRenderResource interface.
void FTerrainVertexFactory::InitRHI()
{
	// list of declaration items
	FVertexDeclarationElementList Elements;

	// position decls
	Elements.AddItem(AccessStreamComponent(Data.PositionComponent, VEU_Position));
	// displacement
	Elements.AddItem(AccessStreamComponent(Data.DisplacementComponent, VEU_BlendWeight));
	// gradients
	Elements.AddItem(AccessStreamComponent(Data.GradientComponent, VEU_Tangent));

	// create the actual device decls
	//@todo.SAS. Include shadow map and light map
	InitDeclaration(Elements,FALSE,FALSE);
}

/** Shader parameters for use with FTerrainVertexFactory */
class FTerrainVertexFactoryShaderParameters : public FVertexFactoryShaderParameters
{
public:
	/**
	* Bind shader constants by name
	* @param	ParameterMap - mapping of named shader constants to indices
	*/
	virtual void Bind(const FShaderParameterMap& ParameterMap)
	{
		LocalToWorldParameter.Bind(ParameterMap,TEXT("LocalToWorld"));
		WorldToLocalParameter.Bind(ParameterMap,TEXT("WorldToLocal"),TRUE);
		ShadowCoordinateScaleBiasParameter.Bind(ParameterMap,TEXT("ShadowCoordinateScaleBias"),TRUE);
		InvMaxTessLevel_ZScale_Parameter.Bind(ParameterMap,TEXT("InvMaxTesselationLevel_ZScale"));
		InvTerrainSize_SectionBase_Parameter.Bind(ParameterMap,TEXT("InvTerrainSize_SectionBase"));
		LightMapCoordScaleBiasParameter.Bind(ParameterMap,TEXT("LightMapCoordinateScaleBias"), TRUE);
	}

	/**
	* Serialize shader params to an archive
	* @param	Ar - archive to serialize to
	*/
	virtual void Serialize(FArchive& Ar)
	{
		Ar << LocalToWorldParameter;
		Ar << WorldToLocalParameter;
		Ar << ShadowCoordinateScaleBiasParameter;
		Ar << InvMaxTessLevel_ZScale_Parameter;
		Ar << InvTerrainSize_SectionBase_Parameter;
		Ar << LightMapCoordScaleBiasParameter;
	}

	/**
	* Set any shader data specific to this vertex factory
	*/
	virtual void Set(FCommandContextRHI* Context,FShader* VertexShader,const FVertexFactory* VertexFactory,const FSceneView* View) const
	{
		FTerrainVertexFactory* TerrainVF = (FTerrainVertexFactory*)VertexFactory;
		FTerrainObject* TerrainObject = TerrainVF->GetTerrainObject();
		if (InvMaxTessLevel_ZScale_Parameter.IsBound())
		{
			FVector2D Value(
				1.0f, //1.0f / (FLOAT)(TerrainVF->GetTessellationLevel())
				TerrainObject->GetTerrainHeightScale()
				);
			SetVertexShaderValue(Context,VertexShader->GetVertexShader(),InvMaxTessLevel_ZScale_Parameter,Value);
		}
		if (InvTerrainSize_SectionBase_Parameter.IsBound())
		{
			FVector4 Value;
			if (GPlatformNeedsPowerOfTwoTextures) // power of two
			{
				Value.X = 1.0f / (1 << appCeilLogTwo(TerrainObject->GetNumVerticesX()));
				Value.Y = 1.0f / (1 << appCeilLogTwo(TerrainObject->GetNumVerticesY()));
			}
			else
			{
				Value.X = 1.0f / TerrainObject->GetNumVerticesX();
				Value.Y = 1.0f / TerrainObject->GetNumVerticesY();
			}
			Value.Z = TerrainObject->GetComponentSectionBaseX();
			Value.W = TerrainObject->GetComponentSectionBaseY();
			SetVertexShaderValue(Context,VertexShader->GetVertexShader(),InvTerrainSize_SectionBase_Parameter,Value);
		}
		if (LightMapCoordScaleBiasParameter.IsBound())
		{
			FVector4 Value;

			INT LightMapRes = TerrainObject->GetLightMapResolution();
			Value.X = (FLOAT)LightMapRes / ((FLOAT)TerrainObject->GetComponentTrueSectionSizeX() * (FLOAT)LightMapRes + 1.0f);
			Value.Y = (FLOAT)LightMapRes / ((FLOAT)TerrainObject->GetComponentTrueSectionSizeY() * (FLOAT)LightMapRes + 1.0f);
			Value.Z = 0.0f;
			Value.W = 0.0f;
			SetVertexShaderValue(Context,VertexShader->GetVertexShader(),LightMapCoordScaleBiasParameter,Value);
		}
		if (ShadowCoordinateScaleBiasParameter.IsBound())
		{
			FVector4 Value(
				TerrainObject->GetShadowCoordinateScaleX(),
				TerrainObject->GetShadowCoordinateScaleY(),
				TerrainObject->GetShadowCoordinateBiasY(),
				TerrainObject->GetShadowCoordinateBiasX()
				);
			Value = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
			SetVertexShaderValue(Context,VertexShader->GetVertexShader(),ShadowCoordinateScaleBiasParameter,Value);
		}
	}
	
	/**
	* 
	*/
	virtual void SetLocalTransforms(FCommandContextRHI* Context,FShader* VertexShader,const FMatrix& LocalToWorld,const FMatrix& WorldToLocal) const
	{
		SetVertexShaderValue(Context,VertexShader->GetVertexShader(),LocalToWorldParameter,LocalToWorld);
		SetVertexShaderValues(Context,VertexShader->GetVertexShader(),WorldToLocalParameter,(FVector4*)&WorldToLocal,3);
	}

private:
	INT	TessellationLevel;
	FShaderParameter LocalToWorldParameter;
	FShaderParameter WorldToLocalParameter;
	FShaderParameter ShadowCoordinateScaleBiasParameter;
	FShaderParameter InvMaxTessLevel_ZScale_Parameter;
	FShaderParameter InvTerrainSize_SectionBase_Parameter;
	FShaderParameter LightMapCoordScaleBiasParameter;
};

/** bind terrain vertex factory to its shader file and its shader parameters */
IMPLEMENT_VERTEX_FACTORY_TYPE(FTerrainVertexFactory, FTerrainVertexFactoryShaderParameters, "TerrainVertexFactory", TRUE, TRUE, VER_RECOMPILE_TERRAIN_SHADERS);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FTerrainDecalVertexFactory
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class FTerrainDecalVertexFactoryShaderParameters : public FTerrainVertexFactoryShaderParameters
{
public:
	typedef FTerrainVertexFactoryShaderParameters Super;

	/**
	* Bind shader constants by name
	* @param	ParameterMap - mapping of named shader constants to indices
	*/
	virtual void Bind(const FShaderParameterMap& ParameterMap)
	{
		Super::Bind( ParameterMap );
		WorldToDecalParameter.Bind( ParameterMap, TEXT("WorldToDecal"), TRUE );
		DecalLocationParameter.Bind( ParameterMap, TEXT("DecalLocation"), TRUE );
		DecalOffsetParameter.Bind( ParameterMap, TEXT("DecalOffset"), TRUE );
	}

	/**
	* Serialize shader params to an archive
	* @param	Ar - archive to serialize to
	*/
	virtual void Serialize(FArchive& Ar)
	{
		Super::Serialize( Ar );
		Ar << WorldToDecalParameter;
		Ar << DecalLocationParameter;
		Ar << DecalOffsetParameter;
	}

	/**
	* Set any shader data specific to this vertex factory
	*/
	virtual void Set(FCommandContextRHI* Context,FShader* VertexShader,const FVertexFactory* VertexFactory,const FSceneView* View) const
	{
		Super::Set( Context, VertexShader, VertexFactory, View );

		FTerrainDecalVertexFactory* TerrainDVF = (FTerrainDecalVertexFactory*)VertexFactory;
		if ( WorldToDecalParameter.IsBound() )
		{
			SetVertexShaderValue( Context, VertexShader->GetVertexShader(), WorldToDecalParameter, TerrainDVF->GetDecalMatrix() );
		}
		if ( DecalLocationParameter.IsBound() )
		{
			SetVertexShaderValue( Context, VertexShader->GetVertexShader(), DecalLocationParameter, TerrainDVF->GetDecalLocation() );
		}
		if ( DecalOffsetParameter.IsBound() )
		{
			SetVertexShaderValue( Context, VertexShader->GetVertexShader(), DecalOffsetParameter, TerrainDVF->GetDecalOffset() );
		}
	}

private:
	FShaderParameter WorldToDecalParameter;
	FShaderParameter DecalLocationParameter;
	FShaderParameter DecalOffsetParameter;
};

/**
 * Should we cache the material's shader type on this platform with this vertex factory? 
 */
UBOOL FTerrainDecalVertexFactory::ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType)
{
	// Only compile decal materials and special engine materials for a terrain decal vertex factory.
	// The special engine materials must be compiled for the terrain decal vertex factory because they are used with it for wireframe, etc.
	return Material->IsDecalMaterial() || Material->IsSpecialEngineMaterial();
}

/** bind terrain decal vertex factory to its shader file and its shader parameters */
IMPLEMENT_VERTEX_FACTORY_TYPE(FTerrainDecalVertexFactory, FTerrainDecalVertexFactoryShaderParameters, "TerrainDecalVertexFactory", TRUE, TRUE, VER_RECOMPILE_TERRAIN_SHADERS);
