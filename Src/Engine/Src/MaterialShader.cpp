/*=============================================================================
	MaterialShader.h: Material shader definitions.
	Copyright 2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"

//
// Globals
//
TDynamicMap<FGuid,FMaterialShaderMap*> FMaterialShaderMap::GIdToMaterialShaderMap[SP_NumPlatforms];

void FMaterialPixelShaderParameters::Bind(const FMaterial* Material,const FShaderParameterMap& ParameterMap)
{
	// Bind uniform scalar expression parameters.
	//for(INT ParameterIndex = 0;ParameterIndex < Material->UniformScalarExpressions.Num();ParameterIndex++)
	for(INT ParameterIndex = 0;ParameterIndex < Material->GetUniformScalarExpressions().Num();ParameterIndex++)
	{
		FShaderParameter ShaderParameter;
		FString ParameterName = FString::Printf(TEXT("UniformScalar_%u"),ParameterIndex);
		ShaderParameter.Bind(ParameterMap,*ParameterName,TRUE);
		if(ShaderParameter.IsBound())
		{
			FUniformParameter* UniformParameter = new(UniformParameters) FUniformParameter;
			UniformParameter->Type = MCT_Float;
			UniformParameter->Index = ParameterIndex;
			UniformParameter->ShaderParameter = ShaderParameter;
		}
	}

	// Bind uniform vector expression parameters.
	//for(INT ParameterIndex = 0;ParameterIndex < Material->UniformVectorExpressions.Num();ParameterIndex++)
	for(INT ParameterIndex = 0;ParameterIndex < Material->GetUniformVectorExpressions().Num();ParameterIndex++)
	{
		FShaderParameter ShaderParameter;
		FString ParameterName = FString::Printf(TEXT("UniformVector_%u"),ParameterIndex);
		ShaderParameter.Bind(ParameterMap,*ParameterName,TRUE);
		if(ShaderParameter.IsBound())
		{
			FUniformParameter* UniformParameter = new(UniformParameters) FUniformParameter;
			UniformParameter->Type = MCT_Float4;
			UniformParameter->Index = ParameterIndex;
			UniformParameter->ShaderParameter = ShaderParameter;
		}
	}
	
	// Bind uniform 2D texture parameters.
	//for(INT ParameterIndex = 0;ParameterIndex < Material->Uniform2DTextureExpressions.Num();ParameterIndex++)
	for(INT ParameterIndex = 0;ParameterIndex < Material->GetUniform2DTextureExpressions().Num();ParameterIndex++)
	{
		FShaderParameter ShaderParameter;
		FString ParameterName = FString::Printf(TEXT("Texture2D_%u"),ParameterIndex);
		ShaderParameter.Bind(ParameterMap,*ParameterName,TRUE);
		if(ShaderParameter.IsBound())
		{
			FUniformParameter* UniformParameter = new(UniformParameters) FUniformParameter;
			UniformParameter->Type = MCT_Texture2D;
			UniformParameter->Index = ParameterIndex;
			UniformParameter->ShaderParameter = ShaderParameter;
		}
	}

	// Bind uniform cube texture parameters.
	//for(INT ParameterIndex = 0;ParameterIndex < Material->UniformCubeTextureExpressions.Num();ParameterIndex++)
	for(INT ParameterIndex = 0;ParameterIndex < Material->GetUniformCubeTextureExpressions().Num();ParameterIndex++)
	{
		FShaderParameter ShaderParameter;
		FString ParameterName = FString::Printf(TEXT("TextureCube_%u"),ParameterIndex);
		ShaderParameter.Bind(ParameterMap,*ParameterName,TRUE);
		if(ShaderParameter.IsBound())
		{
			FUniformParameter* UniformParameter = new(UniformParameters) FUniformParameter;
			UniformParameter->Type = MCT_TextureCube;
			UniformParameter->Index = ParameterIndex;
			UniformParameter->ShaderParameter = ShaderParameter;
		}
	}

	// only used if Material has a Transform expression 
	LocalToWorldParameter.Bind(ParameterMap,TEXT("LocalToWorldMatrix"),TRUE);
	WorldToViewParameter.Bind(ParameterMap,TEXT("WorldToViewMatrix"),TRUE);

	SceneTextureParameters.Bind(ParameterMap);

	// Only used for two-sided materials.
	TwoSidedSignParameter.Bind(ParameterMap,TEXT("TwoSidedSign"),TRUE);
}

void FMaterialPixelShaderParameters::Set(FCommandContextRHI* Context,FShader* PixelShader,const FMaterialRenderContext& MaterialRenderContext) const
{
	// Set the uniform parameters.
	const FMaterial* Material = MaterialRenderContext.MaterialInstance->GetMaterial();
	for(INT ParameterIndex = 0;ParameterIndex < UniformParameters.Num();ParameterIndex++)
	{
		const FUniformParameter& UniformParameter = UniformParameters(ParameterIndex);
		switch(UniformParameter.Type)
		{
		case MCT_Float:
		{
			FLinearColor Value;
			//if (!Material || (UniformParameter.Index >= Material->UniformScalarExpressions.Num()))
			if (!Material || (UniformParameter.Index >= Material->GetUniformScalarExpressions().Num()))
			{
#if DO_CHECK
				static UBOOL s_bWarnedOnce_Scalar = FALSE;
				if (!s_bWarnedOnce_Scalar)
				{
					debugf(TEXT("vvvvvvvvvvvv PLEASE SUBMIT THIS TO TTP vvvvvvvvvvvv"));
					debugf(TEXT(" "));
					warnf(TEXT("Material accessing scalar beyond ScalarExpressions size (%d/%d) - %s"), 
						UniformParameter.Index, Material ? Material->GetUniformScalarExpressions().Num() : 0,
						Material ? *Material->GetFriendlyName() : TEXT("NO MATERIAL!!!"));
					debugf(TEXT(" "));
					debugf(TEXT("^^^^^^^^^^^^ PLEASE SUBMIT THIS TO TTP ^^^^^^^^^^^^"));
					debugf(TEXT(" "));
					s_bWarnedOnce_Scalar = TRUE;
				}
#endif	//#if DO_CHECK
				Value = FLinearColor(0.0f, 0.0f, 0.0f);
			}
			else
			{
				//Material->UniformScalarExpressions(UniformParameter.Index)->GetNumberValue(MaterialRenderContext,Value);
				(Material->GetUniformScalarExpressions())(UniformParameter.Index)->GetNumberValue(MaterialRenderContext,Value);
			}
			SetPixelShaderValue(Context,PixelShader->GetPixelShader(),UniformParameter.ShaderParameter,Value.R);
			break;
		}
		case MCT_Float4:
		{
			FLinearColor Value;			
			//if (!Material || (UniformParameter.Index >= Material->UniformVectorExpressions.Num()))
			if (!Material || (UniformParameter.Index >= Material->GetUniformVectorExpressions().Num()))
			{
#if DO_CHECK
				static UBOOL s_bWarnedOnce_Vector = FALSE;
				if (!s_bWarnedOnce_Vector)
				{
					debugf(TEXT("vvvvvvvvvvvv PLEASE SUBMIT THIS TO TTP vvvvvvvvvvvv"));
					debugf(TEXT(" "));
					warnf(TEXT("Material accessing vector beyond VectorExpressions size (%d/%d) - %s"), 
						UniformParameter.Index, Material ? Material->GetUniformVectorExpressions().Num() : 0,
						Material ? *Material->GetFriendlyName() : TEXT("NO MATERIAL!!!"));
					debugf(TEXT(" "));
					debugf(TEXT("^^^^^^^^^^^^ PLEASE SUBMIT THIS TO TTP ^^^^^^^^^^^^"));
					debugf(TEXT(" "));
					s_bWarnedOnce_Vector = TRUE;
				}
#endif	//#if DO_CHECK
				Value = FLinearColor(5.0f, 0.0f, 5.0f);
			}
			else
			{
				//Material->UniformVectorExpressions(UniformParameter.Index)->GetNumberValue(MaterialRenderContext,Value);
				(Material->GetUniformVectorExpressions())(UniformParameter.Index)->GetNumberValue(MaterialRenderContext,Value);
			}
			SetPixelShaderValue(Context,PixelShader->GetPixelShader(),UniformParameter.ShaderParameter,Value);
			break;
		}
		case MCT_Texture2D:
		{
			const FTexture* Value = NULL;
			//if (!Material || (UniformParameter.Index >= Material->Uniform2DTextureExpressions.Num()))
			if (!Material || (UniformParameter.Index >= Material->GetUniform2DTextureExpressions().Num()))
			{
#if DO_CHECK
				static UBOOL s_bWarnedOnce_Texture = FALSE;
				if (!s_bWarnedOnce_Texture)
				{
					debugf(TEXT("vvvvvvvvvvvv PLEASE SUBMIT THIS TO TTP vvvvvvvvvvvv"));
					debugf(TEXT(" "));
					warnf(TEXT("Material accessing texture beyond TextureExpressions size (%d/%d) - %s"), 
						UniformParameter.Index, Material ? Material->GetUniform2DTextureExpressions().Num() : 0, 
						Material ? *Material->GetFriendlyName() : TEXT("NO MATERIAL!!!"));
					debugf(TEXT(" "));
					debugf(TEXT("^^^^^^^^^^^^ PLEASE SUBMIT THIS TO TTP ^^^^^^^^^^^^"));
					debugf(TEXT(" "));
					s_bWarnedOnce_Texture = TRUE;
				}
#endif	//#if DO_CHECK
				Value = GWhiteTexture;
			}
			if (!Value)
			{
				//Material->Uniform2DTextureExpressions(UniformParameter.Index)->GetTextureValue(MaterialRenderContext,&Value);
				(Material->GetUniform2DTextureExpressions())(UniformParameter.Index)->GetTextureValue(MaterialRenderContext,&Value);
			}
			if(!Value)
			{
				Value = GWhiteTexture;
			}
			SetTextureParameter(Context,PixelShader->GetPixelShader(),UniformParameter.ShaderParameter,Value);
			break;
		}
		case MCT_TextureCube:
		{
			const FTexture* Value = NULL;
			//if (!Material || (UniformParameter.Index >= Material->UniformCubeTextureExpressions.Num()))
			if (!Material || (UniformParameter.Index >= Material->GetUniformCubeTextureExpressions().Num()))
			{
#if DO_CHECK
				static UBOOL s_bWarnedOnce_Cube = FALSE;
				if (!s_bWarnedOnce_Cube)
				{
					debugf(TEXT("vvvvvvvvvvvv PLEASE SUBMIT THIS TO TTP vvvvvvvvvvvv"));
					debugf(TEXT(" "));
					warnf(TEXT("Material accessing cube texture beyond CubeTextureExpressions size (%d/%d)  - %s"), 
						UniformParameter.Index, Material ? Material->GetUniformCubeTextureExpressions().Num() : 0, 
						Material ? *Material->GetFriendlyName() : TEXT("NO MATERIAL!!!"));
					debugf(TEXT(" "));
					debugf(TEXT("^^^^^^^^^^^^ PLEASE SUBMIT THIS TO TTP ^^^^^^^^^^^^"));
					debugf(TEXT(" "));
					s_bWarnedOnce_Cube = TRUE;
				}
#endif	//#if DO_CHECK
				Value = GWhiteTextureCube;
			}
			if (!Value)
			{
				//Material->UniformCubeTextureExpressions(UniformParameter.Index)->GetTextureValue(MaterialRenderContext,&Value);
				(Material->GetUniformCubeTextureExpressions())(UniformParameter.Index)->GetTextureValue(MaterialRenderContext,&Value);
			}
			if(!Value)
			{
				Value = GWhiteTextureCube;
			}
			if((INT)Value == -1)
			{
				EnvCube_Reserve(UniformParameter.ShaderParameter);
			}
			else
			{
				SetTextureParameter(Context,PixelShader->GetPixelShader(),UniformParameter.ShaderParameter,Value);
			}			
			break;
		}
		};
	}

	// set view matrix for use by view space Transform expressions
	if( Material && 
		(Material->GetTransformsUsed() & UsedCoord_View) )
	{
		SetPixelShaderValues(Context,PixelShader->GetPixelShader(),WorldToViewParameter,(FVector4*)&MaterialRenderContext.View->ViewMatrix,3);
	}

	SceneTextureParameters.Set(Context,MaterialRenderContext.View,PixelShader);
}

UBOOL GShouldUploadLocalToWorldPixelShaderParameters = FALSE;
/**
* Set local transforms for rendering a material with a single mesh
* @param Context - command context
* @param MaterialRenderContext - material specific info for setting the shader
* @param LocalToWorld - l2w for rendering a single mesh
*/
void FMaterialPixelShaderParameters::SetLocalTransforms(
	FCommandContextRHI* Context,
	FShader* PixelShader,
	const FMaterialInstance* MaterialInstance,
	const FMatrix& LocalToWorld,
	UBOOL bBackFace
	) const
{
	const FMaterial* Material = MaterialInstance->GetMaterial();
	// set world matrix for use by world/view space Transform expressions
	if( GShouldUploadLocalToWorldPixelShaderParameters || 
		Material && 
		(Material->GetTransformsUsed() & UsedCoord_World) ||
		(Material->GetTransformsUsed() & UsedCoord_View) )
	{
		SetPixelShaderValues(Context,PixelShader->GetPixelShader(),LocalToWorldParameter,(FVector4*)&LocalToWorld,3);
	}
	// Set the two-sided sign parameter.
	SetPixelShaderValue(Context,PixelShader->GetPixelShader(),TwoSidedSignParameter,bBackFace ? -1.0f : +1.0f);
}

FArchive& operator<<(FArchive& Ar,FMaterialPixelShaderParameters& Parameters)
{
	check(Ar.Ver() >= VER_MIN_MATERIAL_PIXELSHADER);

	Ar << Parameters.UniformParameters;
	Ar << Parameters.LocalToWorldParameter;
	Ar << Parameters.WorldToViewParameter;
	Ar << Parameters.SceneTextureParameters;
	Ar << Parameters.TwoSidedSignParameter;
	return Ar;
}

FShader* FMaterialShaderType::CompileShader(
	EShaderPlatform Platform,
	const FMaterial* Material,
	const TCHAR* MaterialShaderCode,
	TArray<FString>& OutErrors
	)
{
	// Construct the shader environment.
	FShaderCompilerEnvironment Environment;
	Environment.IncludeFiles.Set(TEXT("Material.usf"),MaterialShaderCode);

	switch(Material->GetBlendMode())
	{
	case BLEND_Opaque: Environment.Definitions.Set(TEXT("MATERIALBLENDING_SOLID"),TEXT("1")); break;
	case BLEND_Masked: Environment.Definitions.Set(TEXT("MATERIALBLENDING_MASKED"),TEXT("1")); break;
	case BLEND_Translucent: Environment.Definitions.Set(TEXT("MATERIALBLENDING_TRANSLUCENT"),TEXT("1")); break;
	case BLEND_Additive: Environment.Definitions.Set(TEXT("MATERIALBLENDING_ADDITIVE"),TEXT("1")); break;
	case BLEND_Modulate: Environment.Definitions.Set(TEXT("MATERIALBLENDING_MODULATE"),TEXT("1")); break;
	default: appErrorf(TEXT("Unknown material blend mode: %u"),(INT)Material->GetBlendMode());
	}

	Environment.Definitions.Set(TEXT("MATERIAL_TWOSIDED"),Material->IsTwoSided() ? TEXT("1") : TEXT("0"));

	switch(Material->GetLightingModel())
	{
	case MLM_SHPRT: // For backward compatibility, treat the deprecated SHPRT lighting model as Phong.
	case MLM_Phong: Environment.Definitions.Set(TEXT("MATERIAL_LIGHTINGMODEL_PHONG"),TEXT("1")); break;
	case MLM_NonDirectional: Environment.Definitions.Set(TEXT("MATERIAL_LIGHTINGMODEL_NONDIRECTIONAL"),TEXT("1")); break;
	case MLM_Unlit: Environment.Definitions.Set(TEXT("MATERIAL_LIGHTINGMODEL_UNLIT"),TEXT("1")); break;
	case MLM_Custom: Environment.Definitions.Set(TEXT("MATERIAL_LIGHTINGMODEL_CUSTOM"),TEXT("1")); break;
	default: appErrorf(TEXT("Unknown material lighting model: %u"),(INT)Material->GetLightingModel());
	};

	if( Material->GetUsesEnvCube() )
	{
		Environment.Definitions.Set(TEXT("NO_FAKE_SPECULAR"),TEXT("1"));
	}

	if( Material->GetTransformsUsed() != UsedCoord_None )
	{
		// only use WORLD_COORDS code if a Transform expression was used by the material
		Environment.Definitions.Set(TEXT("WORLD_COORDS"),TEXT("1"));

		if( Material->GetTransformsUsed() == UsedCoord_World )
		{
			// GPU Skin과 버그가 있네 -_-
			//if (!Material->IsUsedWithSkeletalMesh())
			{
				// only use WORLD_COORDS code if a Transform expression was used by the material
				Environment.Definitions.Set(TEXT("ONLY_WORLD_COORDS"),TEXT("1"));
			}			
		}
	}

	//<@ ava specific ; 2007. 7. 25 changmin 추가
	// SM2Poor platform 에서는 hdr integer path를 쓴다....
	//if(	Platform == SP_PCD3D_SM2_POOR )
	if( IsHDRIntegerPath( Platform ) )
	{
		Environment.Definitions.Set(TEXT("HDR_INTEGER_PATH"), TEXT("1"));
	}
	//>@ ava

	// Compile the shader code.
	FShaderCompilerOutput Output;
	if(!FShaderType::CompileShader(Platform,Environment,Output))
	{
		OutErrors = Output.Errors;
		return NULL;
	}

	// Check for shaders with identical compiled code.
	FShader* Shader = FindShaderByCode(Output.Code);
	if(!Shader)
	{
		// Create the shader.
		Shader = (*ConstructCompiledRef)(CompiledShaderInitializerType(this,Output,Material));
	}
	return Shader;
}

FMaterialShaderMap* FMaterialShaderMap::FindId(const FGuid& Id, EShaderPlatform Platform)
{
	return GIdToMaterialShaderMap[Platform].FindRef(Id);
}

FMaterialShaderMap::~FMaterialShaderMap()
{
	GIdToMaterialShaderMap[Platform].Remove(MaterialId);
}

UBOOL FMaterialShaderMap::Compile(const FMaterial* Material,const TCHAR* MaterialShaderCode,EShaderPlatform InPlatform,TArray<FString>& OutErrors,UBOOL bSilent)
{
#if CONSOLE
	appErrorf( TEXT("Trying to compile %s at run-time, which is not supported on consoles!"), *Material->GetFriendlyName() );
	return FALSE;
#else
	UBOOL bSuccess = TRUE;

	// Store the material name for debugging purposes.
	FriendlyName = Material->GetFriendlyName();

	// Iterate over all vertex factory types.
	for(TLinkedList<FVertexFactoryType*>::TIterator VertexFactoryTypeIt(FVertexFactoryType::GetTypeList());VertexFactoryTypeIt;VertexFactoryTypeIt.Next())
	{
		FVertexFactoryType* VertexFactoryType = *VertexFactoryTypeIt;

		if(VertexFactoryType->IsUsedWithMaterials())
		{
			FMeshMaterialShaderMap* MeshShaderMap = NULL;

			// look for existing map for this vertex factory type
			for (INT ShaderMapIndex = 0; ShaderMapIndex < MeshShaderMaps.Num(); ShaderMapIndex++)
			{
				if (MeshShaderMaps(ShaderMapIndex).GetVertexFactoryType() == VertexFactoryType)
				{
					MeshShaderMap = &MeshShaderMaps(ShaderMapIndex);
					break;
				}
			}

			if (MeshShaderMap == NULL)
			{
				// Create a new mesh material shader map.
				MeshShaderMap = new(MeshShaderMaps) FMeshMaterialShaderMap;
			}

			// Compile all mesh material shaders for this material and vertex factory type combo, and cache them in the shader map.
			if(!MeshShaderMap->Compile(Material,MaterialShaderCode,VertexFactoryType,InPlatform,OutErrors,bSilent))
			{
				bSuccess = FALSE;
			}
		}
	}

	// Iterate over all material shader types.
	for(TLinkedList<FShaderType*>::TIterator ShaderTypeIt(FShaderType::GetTypeList());ShaderTypeIt;ShaderTypeIt.Next())
	{
		FMaterialShaderType* ShaderType = ShaderTypeIt->GetMaterialShaderType();
		if (ShaderType && 
			ShaderType->ShouldCache(InPlatform,Material) && 
			Material->ShouldCache(InPlatform, ShaderType, NULL)
			)
		{
			// Compile this material shader for this material.
			TArray<FString> ShaderErrors;

			// only compile the sahder if we don't already have it
			if (!HasShader(ShaderType))
			{
			    FShader* Shader = ShaderType->CompileShader(InPlatform,Material,MaterialShaderCode,ShaderErrors);
			    if(Shader)
			    {
				    AddShader(ShaderType,Shader);
			    }
			    else
			    {
				    for(INT ErrorIndex = 0;ErrorIndex < ShaderErrors.Num();ErrorIndex++)
				    {
					    OutErrors.AddUniqueItem(ShaderErrors(ErrorIndex));
				    }
				    bSuccess = FALSE;
			    }
			}
		}
	}

	// Reinitialize the vertex factory map.
	InitVertexFactoryMap();

	// Register this shader map in the global map with the material's GUID.
	MaterialId = Material->GetId();
	Platform = InPlatform;
	GIdToMaterialShaderMap[Platform].Set(MaterialId,this);

	// Add the persistent shaders to the local shader cache.
	if (Material->IsPersistent())
	{
		GetLocalShaderCache(Platform)->AddMaterialShaderMap(this);
	}

	return bSuccess;
#endif
}

// @debug
UBOOL GDumpIsComplete=FALSE;
UBOOL FMaterialShaderMap::IsComplete(const FMaterial* Material) const
{
	UBOOL bIsComplete = TRUE;

	// Iterate over all vertex factory types.
	for(TLinkedList<FVertexFactoryType*>::TIterator VertexFactoryTypeIt(FVertexFactoryType::GetTypeList());VertexFactoryTypeIt;VertexFactoryTypeIt.Next())
	{
		FVertexFactoryType* VertexFactoryType = *VertexFactoryTypeIt;

		if(VertexFactoryType->IsUsedWithMaterials())
		{
			// Find the shaders for this vertex factory type.
			const FMeshMaterialShaderMap* MeshShaderMap = GetMeshShaderMap(VertexFactoryType);
			if(!MeshShaderMap || !MeshShaderMap->IsComplete(Platform,Material,VertexFactoryType))
			{
				// @debug
				if (GDumpIsComplete && !MeshShaderMap) 
				{
					debugf(TEXT("Don't have VF %s"), VertexFactoryType->GetName());
				}
				bIsComplete = FALSE;
				break;
			}
		}
	}

	// Iterate over all material shader types.
	for(TLinkedList<FShaderType*>::TIterator ShaderTypeIt(FShaderType::GetTypeList());ShaderTypeIt;ShaderTypeIt.Next())
	{
		// Find this shader type in the material's shader map.
		FMaterialShaderType* ShaderType = ShaderTypeIt->GetMaterialShaderType();
		if (ShaderType && 
			ShaderType->ShouldCache(Platform,Material) && 
			Material->ShouldCache(Platform, ShaderType, NULL) &&
			!HasShader(ShaderType)
			)
		{
			// @debug
			if (GDumpIsComplete)
			{
				debugf(TEXT("Don't have Shader %s"), ShaderType->GetName());
			}
			bIsComplete = FALSE;
			break;
		}
	}

	return bIsComplete;
}

void FMaterialShaderMap::GetShaderList(TMap<FGuid,FShader*>& OutShaders) const
{
	TShaderMap<FMaterialShaderType>::GetShaderList(OutShaders);
	for(INT Index = 0;Index < MeshShaderMaps.Num();Index++)
	{
		MeshShaderMaps(Index).GetShaderList(OutShaders);
	}
}

void FMaterialShaderMap::BeginInit()
{
	TShaderMap<FMaterialShaderType>::BeginInit();
	for(INT MapIndex = 0;MapIndex < MeshShaderMaps.Num();MapIndex++)
	{
		MeshShaderMaps(MapIndex).BeginInit();
	}
}

/**
 * Removes all entries in the cache with exceptions based on a shader type
 * @param ShaderType - The shader type to flush or keep (depending on second param)
 * @param bFlushAllButShaderType - TRUE if all shaders EXCEPT the given type should be flush. FALSE will flush ONLY the given shader type
 */
void FMaterialShaderMap::FlushShadersByShaderType(FShaderType* ShaderType, UBOOL bFlushAllButShaderType)
{
	// flush from all the vertex factory shader maps
	for(INT Index = 0;Index < MeshShaderMaps.Num();Index++)
	{
		MeshShaderMaps(Index).FlushShadersByShaderType(ShaderType, bFlushAllButShaderType);
	}

	// flush if flushing all but the given type, go over other shader types and remove them
	if (bFlushAllButShaderType)
	{
		// flush from this shader map
		for(TLinkedList<FShaderType*>::TIterator ShaderTypeIt(FShaderType::GetTypeList());ShaderTypeIt;ShaderTypeIt.Next())
		{
			if (ShaderType != *ShaderTypeIt && ShaderTypeIt->GetMaterialShaderType())
			{
				RemoveShaderType(ShaderTypeIt->GetMaterialShaderType());
			}
		}
	}
	// otherwise just remove this type
	else if (ShaderType->GetMaterialShaderType())
	{
		RemoveShaderType(ShaderType->GetMaterialShaderType());	
	}
}

/**
 * Removes all entries in the cache with exceptions based on a vertex factory type
 * @param ShaderType - The shader type to flush or keep (depending on second param)
 * @param bFlushAllButVertexFactoryType - TRUE if all shaders EXCEPT the given type should be flush. FALSE will flush ONLY the given vertex factory type
 */
void FMaterialShaderMap::FlushShadersByVertexFactoryType(FVertexFactoryType* VertexFactoryType, UBOOL bFlushAllButVertexFactoryType)
{
	for(INT Index = 0;Index < MeshShaderMaps.Num();Index++)
	{
		FVertexFactoryType* VFType = MeshShaderMaps(Index).GetVertexFactoryType();
		// determine if this shaders vertex factory type should be flushed
		if ((bFlushAllButVertexFactoryType && VFType != VertexFactoryType) ||
			(!bFlushAllButVertexFactoryType && VFType == VertexFactoryType))
		{
			// remove the shader map
			MeshShaderMaps.Remove(Index);
			// fix up the counter
			Index--;
		}
	}

	// reset the vertex factory map to remove references to the removed maps
	InitVertexFactoryMap();
}

void FMaterialShaderMap::Serialize(FArchive& Ar)
{
	TShaderMap<FMaterialShaderType>::Serialize(Ar);
	Ar << MeshShaderMaps;
	Ar << MaterialId;
	Ar << FriendlyName;

	// serialize the platform enum as a BYTE
	INT TempPlatform = (INT)Platform;
	Ar << TempPlatform;
	Platform = (EShaderPlatform)TempPlatform;

	if(Ar.IsLoading())
	{
		// When loading, reinitialize VertexFactoryShaderMap from the new contents of VertexFactoryShaders.
		InitVertexFactoryMap();

		// Register this shader map in the global map with the material's ID.
		GIdToMaterialShaderMap[Platform].Set(MaterialId,this);
	}
}

const FMeshMaterialShaderMap* FMaterialShaderMap::GetMeshShaderMap(FVertexFactoryType* VertexFactoryType) const
{
	FMeshMaterialShaderMap* MeshShaderIndex = VertexFactoryMap.FindRef(VertexFactoryType);
	if(!MeshShaderIndex)
	{
		debugf(
			TEXT("Mesh shaders for material \'%s\' with vertex factory %s are not cached!"),
			*FriendlyName,
			VertexFactoryType->GetName()
			);
		return NULL;
	}
	return MeshShaderIndex;
}

void FMaterialShaderMap::InitVertexFactoryMap()
{
	VertexFactoryMap.Empty();
	for(INT Index = 0;Index < MeshShaderMaps.Num();Index++)
	{
		VertexFactoryMap.Set(MeshShaderMaps(Index).GetVertexFactoryType(),&MeshShaderMaps(Index));
	}
}
