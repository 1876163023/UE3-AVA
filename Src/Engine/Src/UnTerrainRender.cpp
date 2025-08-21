/*=============================================================================
	UnTerrain.cpp: Terrain rendering code.
	Copyright 2003-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
#include "EnginePrivate.h"
#include "UnTerrain.h"
#include "UnTerrainRender.h"
#include "LevelUtils.h"
#include "EngineDecalClasses.h"
#include "UnDecalRenderData.h"
#include "ScenePrivate.h"

//
// Check to ensure the encoding of tessellation levels is not going to be incorrect.
//
#if (TERRAIN_MAXTESSELATION > (2 << 6))
	#error Terrain tessellation size too big!
#endif

#if __INTEL_BYTE_ORDER__ || PS3
	#define	UBYTE4_BYTEORDER_XYZW		1
#else
	#define	UBYTE4_BYTEORDER_XYZW		0
#endif

/**
 *	TesselationLevel
 *	Determines the level of tesselation to use for a terrain patch.
 *
 *	@param	Z		The distance from the viewpoint
 *
 *	@return UINT	The tessellation level to utilize
 */
static UINT TesselationLevel(FLOAT Z, INT MinTessLevel)
{
	if (Z < 4096.0f)
	{
		return 16;
	}
	else
	if (Z < 8192.0f)
	{
		return Max<INT>(MinTessLevel, 8);
	}
	else
	if (Z < 16384.0f)
	{
		return Max<INT>(MinTessLevel, 4);
	}
	else
	if (Z < 32768.0f)
	{
		return Max<INT>(MinTessLevel, 2);
	}
	return Max<INT>(MinTessLevel, 1);
}

//
//	FTerrainMaterialResource
//

/**
 *	CompileTerrainMaterial
 *	Compiles a single terrain material.
 *
 *	@param	Property			The EMaterialProperty that the material is being compiled for
 *	@param	Compiler			The FMaterialCompiler* used to compile the material
 *	@param	TerrainMaterial		The UTerrainMaterial* to the terrain material that is being compiled
 *	@param	Highlighted			TRUE if the component is to be highlighted
 *	@param	HighlightColor		The color to use for the highlight
 *
 *	@return	INT					The resulting code index for the compiled material
 */
INT FTerrainMaterialResource::CompileTerrainMaterial(EMaterialProperty Property,FMaterialCompiler* Compiler,UTerrainMaterial* TerrainMaterial,UBOOL Highlighted, FColor& HighlightColor) const
{
	// FTerrainMaterialCompiler - A proxy compiler that overrides the texture coordinates used by the layer's material with the layer's texture coordinates.

	struct FTerrainMaterialCompiler: FProxyMaterialCompiler
	{
		UTerrainMaterial*	TerrainMaterial;

		FTerrainMaterialCompiler(FMaterialCompiler* InCompiler,UTerrainMaterial* InTerrainMaterial):
			FProxyMaterialCompiler(InCompiler),
			TerrainMaterial(InTerrainMaterial)
		{}

		// Override texture coordinates used by the material with the texture coordinate specified by the terrain material.

		virtual INT TextureCoordinate(UINT CoordinateIndex)
		{
			INT	BaseUV;
			switch(TerrainMaterial->MappingType)
			{
			case TMT_Auto:
			case TMT_XY: BaseUV = Compiler->TextureCoordinate(TERRAIN_UV_XY); break;
			case TMT_XZ: BaseUV = Compiler->TextureCoordinate(TERRAIN_UV_XZ); break;
			case TMT_YZ: BaseUV = Compiler->TextureCoordinate(TERRAIN_UV_YZ); break;
			default: appErrorf(TEXT("Invalid mapping type %u"),TerrainMaterial->MappingType); return INDEX_NONE;
			};

			FLOAT	Scale = TerrainMaterial->MappingScale == 0.0f ? 1.0f : TerrainMaterial->MappingScale;

			return Compiler->Add(
					Compiler->AppendVector(
						Compiler->Dot(BaseUV,Compiler->Constant2(+appCos(TerrainMaterial->MappingRotation * PI / 180.0) / Scale,+appSin(TerrainMaterial->MappingRotation * PI / 180.0) / Scale)),
						Compiler->Dot(BaseUV,Compiler->Constant2(-appSin(TerrainMaterial->MappingRotation * PI / 180.0) / Scale,+appCos(TerrainMaterial->MappingRotation * PI / 180.0) / Scale))
						),
					Compiler->Constant2(TerrainMaterial->MappingPanU,TerrainMaterial->MappingPanV)
					);
		}

		virtual INT FlipBookOffset(UTexture* InFlipBook)
		{
			return INDEX_NONE;
		}

		INT LensFlareIntesity()
		{
			return INDEX_NONE;
		}

		INT LensFlareRadialDistance()
		{
			return INDEX_NONE;
		}

		INT LensFlareRayDistance()
		{
			return INDEX_NONE;
		}

		INT LensFlareSourceDistance()
		{
			return INDEX_NONE;
		}
	};

	UMaterialInstance*			MaterialInstance = TerrainMaterial ? (TerrainMaterial->Material ? TerrainMaterial->Material : GEngine->DefaultMaterial)  : GEngine->DefaultMaterial;
	UMaterial*					Material = MaterialInstance->GetMaterial();
	FTerrainMaterialCompiler	ProxyCompiler(Compiler,TerrainMaterial);

	INT	Result = Compiler->ForceCast(Material->MaterialResource->CompileProperty(Property,&ProxyCompiler),GetMaterialPropertyType(Property));

	extern UBOOL GOptimizingShaderForGameRuntime;
	if(Highlighted && !GOptimizingShaderForGameRuntime)
	{
		FLinearColor SelectionColor(HighlightColor.ReinterpretAsLinear());
		switch(Property)
		{
		case MP_EmissiveColor:			
			Result = Compiler->Add(Result,Compiler->Constant3(SelectionColor.R,SelectionColor.G,SelectionColor.B));
			break;
		case MP_DiffuseColor:
			Result = Compiler->Mul(Result,Compiler->Constant3(1.0f - SelectionColor.R,1.0f - SelectionColor.G,1.0f - SelectionColor.B));
			break;
		};
	}

	return Result;
}

/**
 *	CompileProperty
 *	Compiles the resource for the given property type using the given compiler.
 *
 *	@param	Property			The EMaterialProperty that the material is being compiled for
 *	@param	Compiler			The FMaterialCompiler* used to compile the material
 *
 *	@return	INT					The resulting code index for the compiled resource
 */
INT FTerrainMaterialResource::CompileProperty(EMaterialProperty Property,FMaterialCompiler* Compiler) const
{
	// Count the number of terrain materials included in this material.

	INT	NumMaterials = 0;
	for(INT MaterialIndex = 0;MaterialIndex < Mask.Num();MaterialIndex++)
	{
		if(Mask.Get(MaterialIndex))
		{
			NumMaterials++;
		}
	}

	if(NumMaterials == 1)
	{
		for(INT MaterialIndex = 0;MaterialIndex < Mask.Num();MaterialIndex++)
		{
			if(Mask.Get(MaterialIndex))
			{
				if (MaterialIndex < Terrain->WeightedMaterials.Num())
				{
					FTerrainWeightedMaterial* WeightedMaterial = &(Terrain->WeightedMaterials(MaterialIndex));
					return CompileTerrainMaterial(Property,Compiler,WeightedMaterial->Material,
						WeightedMaterial->Highlighted,
						WeightedMaterial->HighlightColor);
				}
				else
				{
					debugf(TEXT("Terrain CompileProperty> Invalid MaterialIndex %2d - %s!"),
						MaterialIndex, *(Terrain->GetPathName()));
				}
			}
		}
#if defined(_TERRAIN_CATCH_MISSING_MATERIALS_)
		appErrorf(TEXT("Single material has disappeared!"));
#else	//#if defined(_TERRAIN_CATCH_MISSING_MATERIALS_)
		debugf(TEXT("Terrain CompileProperty> Single material has disappeared for %s!"), *(Terrain->GetPathName()));
#endif	//#if defined(_TERRAIN_CATCH_MISSING_MATERIALS_)
		return INDEX_NONE;
	}
	else if(NumMaterials > 1)
	{
		if ((Property == MP_Normal) && (Terrain->NormalMapLayer != -1))
		{
			if (Terrain->NormalMapLayer >= Terrain->Layers.Num())
			{
				warnf(TEXT("Terrain NormalMapLayer index %2d outside of Layers count %2d for terrain %s"),
					Terrain->NormalMapLayer, Terrain->Layers.Num(), *(Terrain->GetFullName()));
			}
			else
			{
				// Grab the layer indexed by the NormalMapLayer
				FTerrainLayer& Layer = Terrain->Layers(Terrain->NormalMapLayer);
				UTerrainLayerSetup* LayerSetup = Layer.Setup;
				if (LayerSetup)
				{
					if (LayerSetup->Materials.Num() > 0)
					{
						//@todo. Allow selection of 'sub' materials in layers? (Procedural has multiple mats...)
						FTerrainFilteredMaterial& TFilteredMat = LayerSetup->Materials(0);
						UTerrainMaterial* TMat = TFilteredMat.Material;
						for (INT WeightedIndex = 0; WeightedIndex < Terrain->WeightedMaterials.Num(); WeightedIndex++)
						{
							FTerrainWeightedMaterial* WeightedMaterial = &(Terrain->WeightedMaterials(WeightedIndex));
							if (WeightedMaterial->Material == TMat)
							{
								return CompileTerrainMaterial(Property,Compiler,WeightedMaterial->Material,
									WeightedMaterial->Highlighted, WeightedMaterial->HighlightColor);
							}
						}
					}
				}
			}

			// Have all failure cases compile using the stanard compilation path.
		}

		FString RootName;
		FName WeightTextureName;

		UTexture2D* DefaultTexture = LoadObject<UTexture2D>(NULL, TEXT("EngineMaterials.DefaultDiffuse"), NULL, LOAD_None, NULL);

		INT	Result = INDEX_NONE;
		INT MaskArg;
		INT MulArgA;
		INT	MulArgB;
		INT	IndividualResult;
		INT TextureCodeIndex;
		FTerrainWeightedMaterial* WeightedMaterial;
		INT TextureCoordArg = Compiler->TextureCoordinate(0);

		INT WeightMapIndex = 0;
		for(INT MaterialIndex = 0;MaterialIndex < Mask.Num();MaterialIndex++)
		{
			if(Mask.Get(MaterialIndex))
			{
				if (MaterialIndex < Terrain->WeightedMaterials.Num())
				{
					RootName = FString::Printf(TEXT("TWeightMap%d"), MaterialIndex / 4);
					WeightTextureName = FName(*RootName);

					WeightedMaterial = &(Terrain->WeightedMaterials(MaterialIndex));
					TextureCodeIndex = Compiler->TextureParameter(WeightTextureName, DefaultTexture);
					MaskArg = Compiler->TextureSample(
									TextureCodeIndex, 
									TextureCoordArg								
									);
					UBOOL bRed = FALSE;
					UBOOL bGreen = FALSE;
					UBOOL bBlue = FALSE;
					UBOOL bAlpha = FALSE;
					INT DataIndex = MaterialIndex % 4;
					switch (DataIndex)
					{
					case 0:		bBlue = TRUE;		break;
					case 1:		bGreen = TRUE;		break;
					case 2:		bRed = TRUE;		break;
					case 3:		bAlpha = TRUE;		break;
					}
					MulArgA = Compiler->ComponentMask(
								MaskArg, 
								bRed, bGreen, bBlue, bAlpha
								);
					MulArgB = CompileTerrainMaterial(
								Property,Compiler,
								WeightedMaterial->Material,
								WeightedMaterial->Highlighted,
								WeightedMaterial->HighlightColor
								);

					IndividualResult = Compiler->Mul(MulArgA, MulArgB);

					//@todo.SAS. Implement multipass rendering option here?
					if (Result == INDEX_NONE)
					{
						Result = IndividualResult;
					}
					else
					{
						Result = Compiler->Add(Result,IndividualResult);
					}
				}
				else
				{
					debugf(TEXT("Terrain CompileProperty> Invalid MaterialIndex %2d - %s!"),
						MaterialIndex, *(Terrain->GetPathName()));
				}
			}
		}

		return Result;
	}
	else
	{
		return GEngine->DefaultMaterial->GetInstanceInterface(0)->GetMaterial()->CompileProperty(Property,Compiler);
	}
}

/**
 *	GetFriendlyName
 *
 *	@return FString		The resource's friendly name
 */
FString FTerrainMaterialResource::GetFriendlyName() const
{
	// Concatenate the TerrainMaterial names.
	FString MaterialNames;
	for(INT MaterialIndex = 0;MaterialIndex < Mask.Num();MaterialIndex++)
	{
		if(Mask.Get(MaterialIndex))
		{
			if(MaterialNames.Len())
			{
				MaterialNames += TEXT("+");
			}
			if (MaterialIndex < Terrain->WeightedMaterials.Num())
			{
				FTerrainWeightedMaterial& WeightedMat = Terrain->WeightedMaterials(MaterialIndex);
				UTerrainMaterial* TerrainMaterial = WeightedMat.Material;
				if (TerrainMaterial)
				{
					MaterialNames += TerrainMaterial->GetName();
				}
				else
				{
					MaterialNames += TEXT("***NULLMAT***");
				}
			}
			else
			{
					MaterialNames += TEXT("***BADMATINDEX***");
			}
		}
	}
	return FString::Printf(TEXT("TerrainMaterialResource:%s"),*MaterialNames);;
}

/**
 *	GetVectorValue
 *	Retrives the vector value for the given parameter name.
 *
 *	@param	ParameterName		The name of the vector parameter to retrieve
 *	@param	OutValue			Pointer to a FLinearColor where the value should be placed.
 *
 *	@return	UBOOL				TRUE if parameter was found, FALSE if not
 */
UBOOL FTerrainMaterialResource::GetVectorValue(const FName& ParameterName,FLinearColor* OutValue) const
{
	for(INT MaterialIndex = 0;MaterialIndex < Terrain->WeightedMaterials.Num();MaterialIndex++)
	{
		UTerrainMaterial* TerrainMaterial = Terrain->WeightedMaterials(MaterialIndex).Material;
		UMaterialInstance* Material = TerrainMaterial ? 
			(TerrainMaterial->Material ? TerrainMaterial->Material : GEngine->DefaultMaterial) : 
			GEngine->DefaultMaterial;
		if(Material && Material->GetInstanceInterface(0)->GetVectorValue(ParameterName,OutValue))
		{
			return TRUE;
		}
	}
	return FALSE;
}

/**
 *	GetScalarValue
 *	Retrives the Scalar value for the given parameter name.
 *
 *	@param	ParameterName		The name of the Scalar parameter to retrieve
 *	@param	OutValue			Pointer to a FLOAT where the value should be placed.
 *
 *	@return	UBOOL				TRUE if parameter was found, FALSE if not
 */
UBOOL FTerrainMaterialResource::GetScalarValue(const FName& ParameterName,FLOAT* OutValue) const
{
	for(INT MaterialIndex = 0;MaterialIndex < Terrain->WeightedMaterials.Num();MaterialIndex++)
	{
		UMaterialInstance* Material = Terrain->WeightedMaterials(MaterialIndex).Material->Material;
		if(Material && Material->GetInstanceInterface(0)->GetScalarValue(ParameterName,OutValue))
		{
			return TRUE;
		}
	}
	return FALSE;
}

/**
 *	GetTextureValue
 *	Retrives the Texture value for the given parameter name.
 *	Checks for the name "TWeightMap*" to catch instances of weight map retrieval requests.
 *
 *	@param	ParameterName		The name of the Texture parameter to retrieve
 *	@param	OutValue			Pointer to a FTexture* where the value should be placed.
 *
 *	@return	UBOOL				TRUE if parameter was found, FALSE if not
 */
UBOOL FTerrainMaterialResource::GetTextureValue(const FName& ParameterName,const FTexture** OutValue) const
{
	FString LocalName = ParameterName.ToString();
	TCHAR* String = appStrstr(*LocalName, TEXT("TWeightMap"));
	if (String)
	{
		String += appStrlen(TEXT("TWeightMap"));
		INT Index = appAtoi(String);
		if (Index < WeightMaps.Num())
		{
			UTexture2D* Texture2D = WeightMaps(Index);
			if (Texture2D)
			{
				FTexture* Texture = (FTexture*)(Texture2D->Resource);
				if (Texture)
				{
					*OutValue = Texture;
					return TRUE;
				}
			}
		}
	}

	for(INT MaterialIndex = 0;MaterialIndex < Terrain->WeightedMaterials.Num();MaterialIndex++)
	{
		FTerrainWeightedMaterial& TWMat = Terrain->WeightedMaterials(MaterialIndex);
		if (TWMat.Material)
		{
			UMaterialInstance* Material = TWMat.Material->Material;
			if (Material)
			{
				const FMaterialInstance* MatInst = Material->GetInstanceInterface(FALSE);
				if (MatInst)
				{
					if (MatInst->GetTextureValue(ParameterName,OutValue))
					{
						return TRUE;
					}
				}
			}
		}
	}
	return FALSE;
}

FArchive& operator<<(FArchive& Ar,FTerrainMaterialResource& R)
{
	if (Ar.Ver() >= VER_TERRAIN_MATERIALRESOURCE_SERIALIZE)
	{
		R.Serialize(Ar);
	}

	Ar << (UObject*&)R.Terrain << R.Mask;
	if( Ar.Ver() < VER_RENDERING_REFACTOR )
	{
		FGuid PersistentGuid;
		Ar << PersistentGuid;
	}

	if (Ar.Ver() >= VER_TERRAIN_SERIALIZE_MATRES_GUIDS)
	{
		Ar << R.MaterialIds;
	}
	else
	{
		R.MaterialIds.Empty();
	}

	return Ar;
}

void FTerrainMaterialResource::PostLoad()
{
	// Initialize the material's shader map.
	InitShaderMap();

	// Walk the material Ids and check for validity
	UBOOL bTossShader = FALSE;
	if (MaterialIds.Num() > 0)
	{
		INT IdIndex = 0;
		for(INT MaterialIndex = 0;MaterialIndex < Mask.Num();MaterialIndex++)
		{
			if(Mask.Get(MaterialIndex))
			{
				if (MaterialIndex < Terrain->WeightedMaterials.Num())
				{
					FTerrainWeightedMaterial& WeightedMat = Terrain->WeightedMaterials(MaterialIndex);
					UTerrainMaterial* TerrainMaterial = WeightedMat.Material;
					if (TerrainMaterial)
					{
						UMaterialInstance* MatRes = TerrainMaterial->Material;
						if (MatRes)
						{
							UMaterial* Material = MatRes->GetMaterial();
							if (Material && Material->MaterialResource)
							{
								if (IdIndex < MaterialIds.Num())
								{
									FGuid CheckGuid = MaterialIds(IdIndex++);
									if (Material->MaterialResource->GetId() != CheckGuid)
									{
										bTossShader = TRUE;
										MaterialIds.Empty();
										break;
									}
								}
							}
							else
							{
								bTossShader = TRUE;
								break;
							}
						}
					}
					else
					{
						bTossShader = TRUE;
						break;
					}
				}
				else
				{
					bTossShader = TRUE;
					break;
				}
			}
		}
	}
	else
	{
		bTossShader = TRUE;
	}

	if (bTossShader == TRUE)
	{
		FMaterialShaderMap* LocalShaderMap = GetShaderMap();
		if (LocalShaderMap)
		{
			LocalShaderMap->Empty();
		}
	}

	UpdateMaterialDependencies();
}

/** 
 *	Called before the resource is saved. 
 */
void FTerrainMaterialResource::PreSave()
{
	// Walk the masks and store the GUIDs
	MaterialIds.Empty();
	for(INT MaterialIndex = 0;MaterialIndex < Mask.Num();MaterialIndex++)
	{
		if(Mask.Get(MaterialIndex))
		{
			if (MaterialIndex < Terrain->WeightedMaterials.Num())
			{
				FTerrainWeightedMaterial& WeightedMat = Terrain->WeightedMaterials(MaterialIndex);
				UTerrainMaterial* TerrainMaterial = WeightedMat.Material;
				if (TerrainMaterial)
				{
					UMaterialInstance* MatRes = TerrainMaterial->Material;
					if (MatRes)
					{
						UMaterial* Material = MatRes->GetMaterial();
						if (Material && Material->MaterialResource)
						{
							MaterialIds.AddItem(Material->MaterialResource->GetId());
						}
						else
						{
							FGuid BaseId(0,0,0,0);
							MaterialIds.AddItem(BaseId);
						}
					}
				}
				else
				{
					FGuid BaseId(0,0,0,0);
					MaterialIds.AddItem(BaseId);
				}
			}
			else
			{
				FGuid BaseId(0,0,0,0);
				MaterialIds.AddItem(BaseId);
			}
		}
	}
}

void FTerrainMaterialResource::UpdateMaterialDependencies()
{
	check(!MaterialDependencies.Num());

	// Build a list of the materials this terrain material depends on.
	for(INT MaterialIndex = 0;MaterialIndex < Mask.Num();MaterialIndex++)
	{
		if(Mask.Get(MaterialIndex))
		{
			const FMaterialInstance* MaterialInstance = GEngine->DefaultMaterial->GetInstanceInterface(FALSE);
			if (MaterialIndex < Terrain->WeightedMaterials.Num())
			{
				FTerrainWeightedMaterial* WeightedMaterial = &(Terrain->WeightedMaterials(MaterialIndex));
				const UTerrainMaterial* const TerrainMaterial = WeightedMaterial->Material;
				if(TerrainMaterial)
				{
					if(TerrainMaterial->Material)
					{
						MaterialInstance = TerrainMaterial->Material->GetInstanceInterface(FALSE);
					}
				}
			}
			MaterialDependencies.AddUniqueItem(MaterialInstance);
		}
	}
}

/**
 *	ATerrain::GetCachedMaterial
 *	Retrives the cached material for the given Mask.
 *	If the material is not in the cache, it will generate it.
 *
 *	@param	Mask				The FTerrainMaterialMask for the material the caller wishes to retrive.
 *
 *	@return	FMaterialInstance*	The cached material.
 */
FMaterialInstance* ATerrain::GetCachedMaterial(const FTerrainMaterialMask& Mask, UBOOL& bIsTerrainResource)
{
	for (INT MaterialIndex = 0; MaterialIndex < CachedMaterialCount; MaterialIndex++)
	{
		if (CachedMaterials[MaterialIndex])
		{
			if (CachedMaterials[MaterialIndex]->GetMask() == Mask)
			{
				// If the material ShaderMap is NULL, the shaders have not been generated...
				if (CachedMaterials[MaterialIndex]->GetShaderMap() == NULL)
				{
					// This is probably a terrain material that's too complex, so just return default material.
					bIsTerrainResource = FALSE;
					return GEngine->DefaultMaterial->GetInstanceInterface(FALSE);
				}
				else
				{
					bIsTerrainResource = TRUE;
					return CachedMaterials[MaterialIndex];
				}
			}
		}
	}
#if CONSOLE
	// Do not try to compile materials when running on consoles
	bIsTerrainResource = FALSE;
	return GEngine->DefaultMaterial->GetInstanceInterface(FALSE);
#else
	INT EmptySlot = -1;
	for (INT MaterialIndex = 0; MaterialIndex < CachedMaterialCount; MaterialIndex++)
	{
		if (CachedMaterials[MaterialIndex] == NULL)
		{
			EmptySlot = MaterialIndex;
			break;
		}
	}

	if (EmptySlot == -1)
	{
		debugf(TEXT("Increasing size of CachedMaterials for %s"), *GetName());

		// Need to allocate a bigger array...
		INT	NewCachedMaterialCount	= CachedMaterialCount + TERRAIN_CACHED_MATERIAL_INCREMENT;
		INT Index;

		// Retain existing materials...
	    FTerrainMaterialResource** OldCachedMaterials = CachedMaterials;
	    FTerrainMaterialResource** NewCachedMaterials = new FTerrainMaterialResource*[NewCachedMaterialCount];
		check(NewCachedMaterials);
		for (Index = 0; Index < CachedMaterialCount; Index++)
		{
			NewCachedMaterials[Index] = CachedMaterials[Index];
		}
		EmptySlot = CachedMaterialCount;
		for (; Index < NewCachedMaterialCount; Index++)
		{
			NewCachedMaterials[Index] = NULL;
		}

		CachedMaterialCount	= NewCachedMaterialCount;
		CachedMaterials		= NewCachedMaterials;
	    delete [] OldCachedMaterials;
	}

	check(EmptySlot >= 0);

	// Generate a new material resource for this mask
	CachedMaterials[EmptySlot] = new FTerrainMaterialResource(this,Mask);

	// Cache the shaders
	CachedMaterials[EmptySlot]->CacheShaders();

	if (CachedMaterials[EmptySlot]->GetShaderMap() == NULL)
	{
		bIsTerrainResource = FALSE;
		return GEngine->DefaultMaterial->GetInstanceInterface(FALSE);
	}
	else
	{
		bIsTerrainResource = TRUE;
		return CachedMaterials[EmptySlot];
	}
#endif
}

/**
 *	FTerrainVertex
 *	The vertex structure used for terrain. 
 */
struct FTerrainVertex
{
#if UBYTE4_BYTEORDER_XYZW
	BYTE	X,
			Y,
			Z_LOBYTE,
			Z_HIBYTE;
#else
	BYTE	Z_HIBYTE,
			Z_LOBYTE,
			Y,
			X;
#endif
	FLOAT	Displacement;
	SWORD	GradientX,
			GradientY;
};

//
//	FTerrainVertexBuffer
//

/**
 *	InitRHI
 *	Initialize the render hardware interface for the terrain vertex buffer.
 */
void FTerrainVertexBuffer::InitRHI()
{
	if (bIsDynamic == TRUE)
	{
		return;
	}

	SCOPE_CYCLE_COUNTER(STAT_TerrainSmoothTime);
	
	MaxVertexCount = (Component->SectionSizeX * MaxTessellation + 1) * 
		(Component->SectionSizeY * MaxTessellation + 1);

	// Create the buffer rendering resource
	UINT Stride = sizeof(FTerrainVertex);
	UINT Size = MaxVertexCount * Stride;

	VertexBufferRHI = RHICreateVertexBuffer(Size, NULL, FALSE);

	// Fill it...
	//@todo.SAS. Should we do this now, and likely repack it the first rendered frame
	// or just defer the packing until we determine the proper tessellation level for
	// the component (which occurs during the draw call)?
	FillData(MaxTessellation);
}

/** 
 * Initialize the dynamic RHI for this rendering resource 
 */
void FTerrainVertexBuffer::InitDynamicRHI()
{
	if (bIsDynamic == FALSE)
	{
		return;
	}

	SCOPE_CYCLE_COUNTER(STAT_TerrainSmoothTime);
	
	MaxVertexCount = (Component->SectionSizeX * MaxTessellation + 1) * 
		(Component->SectionSizeY * MaxTessellation + 1);

	// Create the buffer rendering resource
	UINT Stride = sizeof(FTerrainVertex);
	UINT Size = MaxVertexCount * Stride;

	VertexBufferRHI = RHICreateVertexBuffer(Size, NULL, TRUE);

	// Fill it...
	//@todo.SAS. Should we do this now, and likely repack it the first rendered frame
	// or just defer the packing until we determine the proper tessellation level for
	// the component (which occurs during the draw call)?
	bRepackRequired = TRUE;
}

/** 
 * Release the dynamic RHI for this rendering resource 
 */
void FTerrainVertexBuffer::ReleaseDynamicRHI()
{
	if (bIsDynamic == FALSE)
	{
		return;
	}

	if (IsValidRef(VertexBufferRHI) == TRUE)
	{
		VertexBufferRHI.Release();
		bRepackRequired = TRUE;
	}
}

/**
 *	FillData
 *	Fills in the data for the vertex buffer
 *
 *	@param	TessellationLevel	The tessellation level to generate the vertex data for.
 *
 *	@return	UBOOL				TRUE if successful, FALSE if failed
 */
UBOOL FTerrainVertexBuffer::FillData(INT TessellationLevel)
{
	check(IsValidRef(VertexBufferRHI) == TRUE);
	check(TessellationLevel <= MaxTessellation);
	check(TessellationLevel > 0);

	SCOPE_CYCLE_COUNTER(STAT_TerrainSmoothTime);

	ATerrain* Terrain = Component->GetTerrain();
	
	VertexCount = (Component->SectionSizeX * TessellationLevel + 1) * 
		(Component->SectionSizeY * TessellationLevel + 1);

	// Create the buffer rendering resource
	UINT Stride = sizeof(FTerrainVertex);
	UINT Size = VertexCount * Stride;

	// Lock the buffer.
	void* Buffer = RHILockVertexBuffer(VertexBufferRHI, 0, Size);

	// Determine the actual step size between true vertices for the component
	INT QuadSizeX = Component->TrueSectionSizeX / Component->SectionSizeX;
	INT QuadSizeY = Component->TrueSectionSizeY / Component->SectionSizeY;

	INT PackedCount = 0;

	// Initialize the vertex data
	FTerrainVertex* FirstVertex = (FTerrainVertex*)Buffer;
	FTerrainVertex* DestVertex = (FTerrainVertex*)Buffer;

	// Handle non-uniform scaling
	FLOAT ScaleX = Terrain->DrawScale3D.Z / Terrain->DrawScale3D.X;
	FLOAT ScaleY = Terrain->DrawScale3D.Z / Terrain->DrawScale3D.Y;

	for (INT Y = 0; Y <= Component->SectionSizeY; Y++)
	{
		for (INT X = 0; X <= Component->SectionSizeX; X++)
		{
			for (INT SmoothY = 0; SmoothY < (Y < Component->SectionSizeY ? TessellationLevel : 1); SmoothY++)
			{
				for (INT SmoothX = 0; SmoothX < (X < Component->SectionSizeX ? TessellationLevel : 1); SmoothX++)
				{
					INT OffsetX = SmoothX * (Terrain->MaxTesselationLevel / TessellationLevel);
					INT OffsetY = SmoothY * (Terrain->MaxTesselationLevel / TessellationLevel);
					INT LocalX = X * QuadSizeX + OffsetX;
					INT LocalY = Y * QuadSizeY + OffsetY;
					INT TrueX = Component->SectionBaseX + LocalX;
					INT TrueY = Component->SectionBaseY + LocalY;

					DestVertex->X = LocalX;
					DestVertex->Y = LocalY;
					
					WORD Z = Terrain->Height(TrueX, TrueY);
					DestVertex->Z_LOBYTE = Z & 255;
					DestVertex->Z_HIBYTE = Z >> 8;

					DestVertex->Displacement = Terrain->GetCachedDisplacement(TrueX, TrueY, 0, 0);

					const WORD GridLength = Terrain->MaxTesselationLevel / TessellationLevel;
					const FLOAT HeightNegX = (FLOAT)Terrain->Height(TrueX - GridLength,TrueY);
					const FLOAT HeightPosX = (FLOAT)Terrain->Height(TrueX + GridLength,TrueY);
					const FLOAT HeightNegY = (FLOAT)Terrain->Height(TrueX,TrueY - GridLength);
					const FLOAT HeightPosY = (FLOAT)Terrain->Height(TrueX,TrueY + GridLength);
					DestVertex->GradientX = (SWORD)appTrunc((HeightPosX - HeightNegX) / (FLOAT)GridLength / 2.0f * ScaleX);
					DestVertex->GradientY = (SWORD)appTrunc((HeightPosY - HeightNegY) / (FLOAT)GridLength / 2.0f * ScaleY);

					DestVertex++;

					PackedCount++;
				}
			}
		}
	}

	check(PackedCount == VertexCount);

	// Unlock the buffer.
	RHIUnlockVertexBuffer(VertexBufferRHI);

	CurrentTessellation = TessellationLevel;

	bRepackRequired = FALSE;

	return TRUE;
}

// FTerrainTessellationIndexBuffer
struct FTerrainTessellationIndexBuffer: FIndexBuffer
{
	/** Terrain object that owns this buffer		*/
	FTerrainObject*		TerrainObject;
	/** The Max tessellation level required			*/
	INT					MaxTesselationLevel;
	/** The current tessellation level packed		*/
	INT					CurrentTessellationLevel;
	/** Total number of triangles packed			*/
	INT					NumTriangles;
	/** Max size of the allocated buffer			*/
	INT					MaxSize;
	/** Flag indicating that data must be repacked	*/
	UBOOL				RepackRequired;
	/** If TRUE, will not be filled at creation		*/
	UBOOL				bDeferredFillData;
	INT					VertexColumnStride;
	INT					VertexRowStride;
	/** Flag indicating it is dynamic						*/
	UBOOL				bIsDynamic;
	/** Flag indicating it is for rendering the collision	*/
	UBOOL				bIsCollisionLevel;


	// Constructor.
	FTerrainTessellationIndexBuffer(FTerrainObject* InTerrainObject,UINT InMaxTesselationLevel,UBOOL bInDeferredFillData, UBOOL bInIsDynamic = TRUE) :
		  TerrainObject(InTerrainObject)
		, MaxTesselationLevel(InMaxTesselationLevel)
		, RepackRequired(bInIsDynamic)
		, bDeferredFillData(bInDeferredFillData)
		, bIsDynamic(bInIsDynamic)
		, bIsCollisionLevel(FALSE)
	{
		SetCurrentTessellationLevel(InMaxTesselationLevel);
	}


	// RenderResource interface
	/**
	 *	InitRHI
	 *	Initialize the render hardware interface for the index buffer.
	 */
	virtual void InitRHI()
	{
		if (bIsDynamic == TRUE)
		{
			return;
		}

		SCOPE_CYCLE_COUNTER(STAT_TerrainSmoothTime);

		DetermineMaxSize();

		if (MaxSize > 0)
		{
			INT Stride = sizeof(WORD);
			IndexBufferRHI = RHICreateIndexBuffer(Stride, MaxSize, NULL, FALSE);
			if (bDeferredFillData == FALSE)
			{
				PrimeBuffer();
				FillData();
			}
		}
		else
		{
			NumTriangles = 0;
		}
	}

	/**
	 *	InitDynamicRHI
	 *	Initialize the render hardware interface for the dynamic index buffer.
	 */
	virtual void InitDynamicRHI()
	{
		if (bIsDynamic == FALSE)
		{
			return;
		}

		SCOPE_CYCLE_COUNTER(STAT_TerrainSmoothTime);

		DetermineMaxSize();

		if (MaxSize > 0)
		{
			INT Stride = sizeof(WORD);
			IndexBufferRHI = RHICreateIndexBuffer(Stride, MaxSize, NULL, TRUE);
			if (bDeferredFillData == FALSE)
			{
				PrimeBuffer();
				FillData();
			}
		}
	}

	/**
	 *	ReleaseDynamicRHI
	 *	Release the dynamic resource
	 */
	virtual void ReleaseDynamicRHI()
	{
		if (bIsDynamic == FALSE)
		{
			return;
		}

		if (IsValidRef(IndexBufferRHI) == TRUE)
		{
			IndexBufferRHI.Release();
			RepackRequired = TRUE;
		}
	}

	/**
	 *	SetMaxTesselationLevel
	 *	Sets the maximum tessellation level for the index buffer.
	 *
	 *	@param	InMaxTesselationLevel	The max level to set it to.
	 */
	inline void SetMaxTesselationLevel(INT InMaxTesselationLevel)
	{
		MaxTesselationLevel	= InMaxTesselationLevel;
	}

	/**
	 *	SetCurrentTessellationLevel
	 *	Sets the current tessellation level for the index buffer.
	 *	This is the tessellation level that the indices are packed for.
	 *
	 *	@param	InCurrentTesselationLevel	The max level to set it to.
	 */
	inline void SetCurrentTessellationLevel(INT InCurrentTessellationLevel)
	{
		check(TerrainObject);
		check(TerrainObject->TerrainComponent);

        CurrentTessellationLevel	= InCurrentTessellationLevel;
		VertexColumnStride			= Square(CurrentTessellationLevel);
		VertexRowStride				= 
			(TerrainObject->TerrainComponent->SectionSizeX * VertexColumnStride) + 
			CurrentTessellationLevel;
	}

	/**
	 *	GetCachedTesselationLevel
	 *	Retrieves the tessellation level for the patch at the given X,Y
	 *
	 *	@param	X,Y		The indices of the component of interest
	 *
	 *	@return	UINT	The tessellation level cached
	 */
	FORCEINLINE INT GetCachedTesselationLevel(INT X,INT Y) const
	{
		if (bIsCollisionLevel == FALSE)
		{
			return TerrainObject->GetTessellationLevel(
				(Y + 1) * (TerrainObject->TerrainComponent->SectionSizeX + 2) + (X + 1));
		}
		else
		{
			return MaxTesselationLevel;
		}
	}

	// GetVertexIndex
	inline WORD GetVertexIndex(INT PatchX,INT PatchY,INT InteriorX,INT InteriorY) const
	{
		if (InteriorX >= CurrentTessellationLevel)
		{
			return GetVertexIndex(PatchX + 1,PatchY,InteriorX - CurrentTessellationLevel,InteriorY);
		}
		if (InteriorY >= CurrentTessellationLevel)
		{
			return GetVertexIndex(PatchX,PatchY + 1,InteriorX,InteriorY - CurrentTessellationLevel);
		}
		return ((PatchY * VertexRowStride) + 
			(PatchX * ((PatchY < TerrainObject->TerrainComponent->SectionSizeY) ? VertexColumnStride : CurrentTessellationLevel))) + 
			(InteriorY * ((PatchX < TerrainObject->TerrainComponent->SectionSizeX) ? CurrentTessellationLevel : 1)) + 
			InteriorX;
	}

	void DetermineMaxSize()
	{
		check(TerrainObject);
		check(TerrainObject->TerrainComponent);

		// Determine the number of triangles at this tesselation level.
		INT TriCount = 0;
		ATerrain* LocalTerrain = CastChecked<ATerrain>(TerrainObject->TerrainComponent->GetOwner());
		INT LocalTessellationLevel = MaxTesselationLevel;
		INT StepSize = TerrainObject->GetMaxTessellationLevel() / CurrentTessellationLevel;
		INT StepSizeX = TerrainObject->TerrainComponent->TrueSectionSizeX / TerrainObject->TerrainComponent->SectionSizeX;
		INT StepSizeY = TerrainObject->TerrainComponent->TrueSectionSizeY / TerrainObject->TerrainComponent->SectionSizeY;
		for (INT Y = 0; Y < TerrainObject->TerrainComponent->SectionSizeY; Y++)
		{
			for (INT X = 0; X < TerrainObject->TerrainComponent->SectionSizeX; X++)
			{
				// If we are in the editor, allocate as though ALL patches could be visible.
				// This will allow for editing visibility without deleting/recreating index buffers while painting.
				if (GIsGame == TRUE)
				{
					if (LocalTerrain->IsTerrainQuadVisible(
						TerrainObject->TerrainComponent->SectionBaseX + X * StepSizeX,
						TerrainObject->TerrainComponent->SectionBaseY + Y * StepSizeY) == FALSE)
					{
						continue;
					}
				}

				TriCount += 2 * Square<INT>(LocalTessellationLevel - 2); // Interior triangles.
				for (UINT EdgeComponent = 0; EdgeComponent < 2; EdgeComponent++)
				{
					for (UINT EdgeSign = 0; EdgeSign < 2; EdgeSign++)
					{
						TriCount += LocalTessellationLevel - 2 + LocalTessellationLevel;
					}
				}
			}
		}

		MaxSize = TriCount * 3 * sizeof(WORD);
	}

	void PrimeBuffer()
	{
		SCOPE_CYCLE_COUNTER(STAT_TerrainSmoothTime);

		// Determine the number of triangles at this tesselation level.
		NumTriangles = DetermineTriangleCount();
	}

	// Determine triangle count
	UINT DetermineTriangleCount()
	{
		NumTriangles = 0;
		if (TerrainObject != NULL)
		{
			// Determine the number of triangles at this tesselation level.
			ATerrain* LocalTerrain = CastChecked<ATerrain>(TerrainObject->TerrainComponent->GetOwner());
			INT StepSize = TerrainObject->GetMaxTessellationLevel() / CurrentTessellationLevel;
			INT StepSizeX = TerrainObject->TerrainComponent->TrueSectionSizeX / TerrainObject->TerrainComponent->SectionSizeX;
			INT StepSizeY = TerrainObject->TerrainComponent->TrueSectionSizeY / TerrainObject->TerrainComponent->SectionSizeY;
			for (INT Y = 0; Y < TerrainObject->TerrainComponent->SectionSizeY; Y++)
			{
				for (INT X = 0; X < TerrainObject->TerrainComponent->SectionSizeX; X++)
				{
					if (LocalTerrain->IsTerrainQuadVisible(
						TerrainObject->TerrainComponent->SectionBaseX + X * StepSizeX,
						TerrainObject->TerrainComponent->SectionBaseY + Y * StepSizeY) == FALSE)
					{
						continue;
					}

					INT TesselationLevel = GetCachedTesselationLevel(X,Y);
					NumTriangles += 2 * Square(TesselationLevel - 2); // Interior triangles.

					for (UINT EdgeComponent = 0; EdgeComponent < 2; EdgeComponent++)
					{
						for (UINT EdgeSign = 0; EdgeSign < 2; EdgeSign++)
						{
							NumTriangles += 
								TesselationLevel - 2 + 
								Min(TesselationLevel, 
									GetCachedTesselationLevel(
										X + (EdgeComponent == 0 ? (EdgeSign ? 1 : -1) : 0),
										Y + (EdgeComponent == 1 ? (EdgeSign ? 1 : -1) : 0)
										)
									);
						}
					}
				}
			}
		}
		return NumTriangles;
	}

	// TesselateEdge
	INT TesselateEdge(WORD*& DestIndex, UINT& CurrentOffset, UINT EdgeTesselation, UINT TesselationLevel,
		UINT X, UINT Y, UINT EdgeX, UINT EdgeY, UINT EdgeInnerX, UINT EdgeInnerY, UINT InnerX, UINT InnerY,
		UINT DeltaX, UINT DeltaY, UINT VertexOrder)
	{
		INT PackedCount = 0;
		check(EdgeTesselation <= TesselationLevel);

		UINT	EdgeVertices[TERRAIN_MAXTESSELATION + 1];
		UINT	InnerVertices[TERRAIN_MAXTESSELATION - 1];

		for (UINT VertexIndex = 0;VertexIndex <= EdgeTesselation;VertexIndex++)
		{
			EdgeVertices[VertexIndex] = GetVertexIndex(
				EdgeX,
				EdgeY,
				EdgeInnerX + VertexIndex * DeltaX * CurrentTessellationLevel / EdgeTesselation,
				EdgeInnerY + VertexIndex * DeltaY * CurrentTessellationLevel / EdgeTesselation
				);
		}
		for (UINT VertexIndex = 1;VertexIndex <= TesselationLevel - 1;VertexIndex++)
		{
			InnerVertices[VertexIndex - 1] = GetVertexIndex(
				X,
				Y,
				InnerX + (VertexIndex - 1) * DeltaX * CurrentTessellationLevel / TesselationLevel,
				InnerY + (VertexIndex - 1) * DeltaY * CurrentTessellationLevel / TesselationLevel
				);
		}
		UINT	EdgeVertexIndex = 0,
				InnerVertexIndex = 0;
		while(EdgeVertexIndex < EdgeTesselation || InnerVertexIndex < (TesselationLevel - 2))
		{
			UINT	EdgePercent = EdgeVertexIndex * (TesselationLevel - 1),
					InnerPercent = (InnerVertexIndex + 1) * EdgeTesselation;

			if (EdgePercent < InnerPercent)
			{
				check(EdgeVertexIndex < EdgeTesselation);
				EdgeVertexIndex++;
				*DestIndex++ = EdgeVertices[EdgeVertexIndex - (1 - VertexOrder)];
				PackedCount++;
				*DestIndex++ = EdgeVertices[EdgeVertexIndex - VertexOrder];
				PackedCount++;
				*DestIndex++ = InnerVertices[InnerVertexIndex];
				PackedCount++;

				CurrentOffset += 3;
			}
			else
			{
				check(InnerVertexIndex < TesselationLevel - 2);
				InnerVertexIndex++;
				*DestIndex++ = InnerVertices[InnerVertexIndex - VertexOrder];
				PackedCount++;
				*DestIndex++ = InnerVertices[InnerVertexIndex - (1 - VertexOrder)];
				PackedCount++;
				*DestIndex++ = EdgeVertices[EdgeVertexIndex];
				PackedCount++;

				CurrentOffset += 3;
			}
		};

		return PackedCount;
	}

	// FIndexBuffer interface.
	virtual void FillData()
	{
		if (NumTriangles <= 0)
		{
			return;
		}

		check(TerrainObject);
		check(TerrainObject->TerrainComponent);

		SCOPE_CYCLE_COUNTER(STAT_TerrainSmoothTime);

		INT Stride = sizeof(WORD);
		INT Size = NumTriangles * 3 * Stride;

		check(Size <= MaxSize);

		// Lock the buffer.
		void* Buffer = RHILockIndexBuffer(IndexBufferRHI, 0, Size);
		check(Buffer);

		//
		WORD*	DestIndex		= (WORD*)Buffer;
		UINT	CurrentOffset	= 0;

		ATerrain* LocalTerrain = CastChecked<ATerrain>(TerrainObject->TerrainComponent->GetOwner());
		INT StepSize = TerrainObject->GetMaxTessellationLevel() / CurrentTessellationLevel;
		INT StepSizeX = TerrainObject->TerrainComponent->TrueSectionSizeX / TerrainObject->TerrainComponent->SectionSizeX;
		INT StepSizeY = TerrainObject->TerrainComponent->TrueSectionSizeY / TerrainObject->TerrainComponent->SectionSizeY;
		for (INT Y = 0;Y < TerrainObject->TerrainComponent->SectionSizeY;Y++)
		{
			for (INT X = 0;X < TerrainObject->TerrainComponent->SectionSizeX;X++)
			{
				if (LocalTerrain->IsTerrainQuadVisible(
					TerrainObject->TerrainComponent->SectionBaseX + X * StepSizeX,
					TerrainObject->TerrainComponent->SectionBaseY + Y * StepSizeY) == FALSE)
				{
					continue;
				}

				INT	TesselationLevel;
				INT	EdgeTesselationNegX;
				INT	EdgeTesselationPosX;
				INT	EdgeTesselationNegY;
				INT	EdgeTesselationPosY;
				if (TerrainObject->MinTessellationLevel == TerrainObject->MaxTessellationLevel)
				{
					TesselationLevel = TerrainObject->MaxTessellationLevel;
					EdgeTesselationNegX = TesselationLevel;
					EdgeTesselationPosX = TesselationLevel;
					EdgeTesselationNegY = TesselationLevel;
					EdgeTesselationPosY = TesselationLevel;
				}
				else
				if (bIsCollisionLevel)
				{
					TesselationLevel = MaxTesselationLevel;
					EdgeTesselationNegX = TesselationLevel;
					EdgeTesselationPosX = TesselationLevel;
					EdgeTesselationNegY = TesselationLevel;
					EdgeTesselationPosY = TesselationLevel;
				}
				else
				{
					TesselationLevel = 
							Max<INT>(
								GetCachedTesselationLevel(X,Y),
								TerrainObject->MinTessellationLevel
								);
					EdgeTesselationNegX = 
							Max<INT>(
								Min(TesselationLevel,GetCachedTesselationLevel(X - 1,Y)),
								TerrainObject->MinTessellationLevel
								);
					EdgeTesselationPosX = 
							Max<INT>(
								Min(TesselationLevel,GetCachedTesselationLevel(X + 1,Y)),
								TerrainObject->MinTessellationLevel
								);
					EdgeTesselationNegY = 
							Max<INT>(
								Min(TesselationLevel,GetCachedTesselationLevel(X,Y - 1)),
								TerrainObject->MinTessellationLevel
								);
					EdgeTesselationPosY = 
							Max<INT>(
								Min(TesselationLevel,GetCachedTesselationLevel(X,Y + 1)),
								TerrainObject->MinTessellationLevel
								);
				}
				check(TesselationLevel > 0);

				if ((TesselationLevel == EdgeTesselationNegX)	 && 
					(EdgeTesselationNegX == EdgeTesselationPosX) && 
					(EdgeTesselationPosX == EdgeTesselationNegY) && 
					(EdgeTesselationNegY == EdgeTesselationPosY))
				{
					INT	TesselationFactor = CurrentTessellationLevel / TesselationLevel;

					WORD	IndexCache[2][TERRAIN_MAXTESSELATION + 1];
					INT		NextCacheLine = 1;
					INT		CurrentVertexIndex = GetVertexIndex(X,Y,0,0);

					for (INT SubX = 0; SubX < TesselationLevel; SubX++, CurrentVertexIndex += TesselationFactor)
					{
						IndexCache[0][SubX] = CurrentVertexIndex;
					}

					IndexCache[0][TesselationLevel] = GetVertexIndex(X + 1,Y,0,0);

					for (INT SubY = 0; SubY < TesselationLevel; SubY++)
					{
						CurrentVertexIndex = GetVertexIndex(X,Y,0,(SubY + 1) * TesselationFactor);

						for (INT SubX = 0; SubX < TesselationLevel; SubX++, CurrentVertexIndex += TesselationFactor)
						{
							IndexCache[NextCacheLine][SubX] = CurrentVertexIndex;
						}

						IndexCache[NextCacheLine][TesselationLevel] = GetVertexIndex(X + 1,Y,0,(SubY + 1) * TesselationFactor);

						for (INT SubX = 0; SubX < TesselationLevel; SubX++)
						{
							WORD	V00 = IndexCache[1 - NextCacheLine][SubX],
									V10 = IndexCache[1 - NextCacheLine][SubX + 1],
									V01 = IndexCache[NextCacheLine][SubX],
									V11 = IndexCache[NextCacheLine][SubX + 1];

							*DestIndex++ = V00;
							*DestIndex++ = V01;
							*DestIndex++ = V11;

							*DestIndex++ = V00;
							*DestIndex++ = V11;
							*DestIndex++ = V10;
						}

						NextCacheLine = 1 - NextCacheLine;
					}
				}
				else
				{
					INT	TesselationFactor = CurrentTessellationLevel / TesselationLevel;

					// Interior triangles.

					for (INT SubX = 1; SubX < TesselationLevel - 1; SubX++)
					{
						for (INT SubY = 1; SubY < TesselationLevel - 1; SubY++)
						{
							WORD	V00 = GetVertexIndex(X,Y,SubX * TesselationFactor,SubY * TesselationFactor),
									V10 = GetVertexIndex(X,Y,(SubX + 1) * TesselationFactor,SubY * TesselationFactor),
									V01 = GetVertexIndex(X,Y,SubX * TesselationFactor,(SubY + 1) * TesselationFactor),
									V11 = GetVertexIndex(X,Y,(SubX + 1) * TesselationFactor,(SubY + 1) * TesselationFactor);

							*DestIndex++ = V00;
							*DestIndex++ = V01;
							*DestIndex++ = V11;

							*DestIndex++ = V00;
							*DestIndex++ = V11;
							*DestIndex++ = V10;
						}
					}

					// Edges.
					TesselateEdge(DestIndex,CurrentOffset,EdgeTesselationNegX,TesselationLevel,X,Y,X,Y,0,0,CurrentTessellationLevel / TesselationLevel,CurrentTessellationLevel / TesselationLevel,0,1,0);
					TesselateEdge(DestIndex,CurrentOffset,EdgeTesselationPosX,TesselationLevel,X,Y,X + 1,Y,0,0,CurrentTessellationLevel - CurrentTessellationLevel / TesselationLevel,CurrentTessellationLevel / TesselationLevel,0,1,1);
					TesselateEdge(DestIndex,CurrentOffset,EdgeTesselationNegY,TesselationLevel,X,Y,X,Y,0,0,CurrentTessellationLevel / TesselationLevel,CurrentTessellationLevel / TesselationLevel,1,0,1);
					TesselateEdge(DestIndex,CurrentOffset,EdgeTesselationPosY,TesselationLevel,X,Y,X,Y + 1,0,0,CurrentTessellationLevel / TesselationLevel,CurrentTessellationLevel - CurrentTessellationLevel / TesselationLevel,1,0,0);
				}
			}
		}

		// Unlock the buffer.
		RHIUnlockIndexBuffer(IndexBufferRHI);
		RepackRequired = FALSE;
	}
};

static INT AddUniqueMask(TArray<FTerrainMaterialMask>& Array,const FTerrainMaterialMask& Mask)
{
	INT	Index = Array.FindItemIndex(Mask);
	if(Index == INDEX_NONE)
	{
		Index = Array.Num();
		new(Array) FTerrainMaterialMask(Mask);
	}
	return Index;
}

/** builds/updates a list of unique blended material combinations used by quads in this terrain section and puts them in the BatchMaterials array,
 * then fills PatchBatches with the index from that array that should be used for each patch. Also updates FullBatch with the index of the full mask.
 */
void UTerrainComponent::UpdatePatchBatches()
{
	ATerrain* Terrain = GetTerrain();
	FTerrainMaterialMask FullMask(Terrain->WeightedMaterials.Num());
	check(Terrain->WeightedMaterials.Num()<=64);

	PatchBatches.Empty(TrueSectionSizeX * TrueSectionSizeY);
	BatchMaterials.Empty();

	for (INT Y = SectionBaseY; Y < SectionBaseY + TrueSectionSizeY; Y++)
	{
		for (INT X = SectionBaseX; X < SectionBaseX + TrueSectionSizeX; X++)
		{
			FTerrainMaterialMask	Mask(Terrain->WeightedMaterials.Num());

			for (INT MaterialIndex = 0; MaterialIndex < Terrain->WeightedMaterials.Num(); MaterialIndex++)
			{
				FTerrainWeightedMaterial& WeightedMaterial = Terrain->WeightedMaterials(MaterialIndex);
				UINT TotalWeight =	(UINT)WeightedMaterial.Weight(X + 0,Y + 0) +
									(UINT)WeightedMaterial.Weight(X + 1,Y + 0) +
									(UINT)WeightedMaterial.Weight(X + 0,Y + 1) +
									(UINT)WeightedMaterial.Weight(X + 1,Y + 1);
				Mask.Set(MaterialIndex, Mask.Get(MaterialIndex) || TotalWeight > 0);
				FullMask.Set(MaterialIndex, FullMask.Get(MaterialIndex) || TotalWeight > 0);
			}

			PatchBatches.AddItem(AddUniqueMask(BatchMaterials, Mask));
		}
	}

	check(PatchBatches.Num() == (TrueSectionSizeX * TrueSectionSizeY));

	FullBatch = AddUniqueMask(BatchMaterials, FullMask);
}

void UTerrainComponent::Attach()
{
	ATerrain* Terrain = Cast<ATerrain>(GetOwner());
	check(Terrain);

	if (DetachFence.GetNumPendingFences() > 0)
	{
		FlushRenderingCommands();
		if (DetachFence.GetNumPendingFences() > 0)
		{
			debugf(TEXT("TerrainComponent::Attach> Still have DetachFence pending???"));
		}
	}

	UpdatePatchBatches();

	UINT NumPatches	= TrueSectionSizeX * TrueSectionSizeY;
	UINT NumBatches	= PatchBatches.Num();

	check(NumPatches == NumBatches);

	PatchCachedTessellationValues = ::new INT[NumPatches];
	PatchBatchOffsets = ::new UINT[NumBatches];
	WorkingOffsets = ::new UINT[NumBatches];
	PatchBatchTriangles = ::new INT[NumBatches];
	appMemzero( PatchBatchTriangles, sizeof(INT) * NumBatches );
	TesselationLevels = ::new BYTE[(SectionSizeX + 2) * (SectionSizeY + 2)];

	TerrainObject = ::new FTerrainObject(this, Terrain->MaxTesselationLevel);
	check(TerrainObject);

	Super::Attach();

	TerrainObject->InitResources();
	TerrainObject->SetIsDeadInGameThread(FALSE);
}

void UTerrainComponent::UpdateTransform()
{
	Super::UpdateTransform();

#if GEMINI_TODO
	GResourceManager->UpdateResource(&TerrainObject->VertexBuffer);
#endif
}

void UTerrainComponent::Detach()
{
	delete [] PatchBatchOffsets;
	PatchBatchOffsets = NULL;
	delete [] WorkingOffsets;
	WorkingOffsets = NULL;
	delete [] PatchCachedTessellationValues;
	PatchCachedTessellationValues = NULL;
	delete [] PatchBatchTriangles;
	PatchBatchTriangles = NULL;

	delete [] TesselationLevels;
	TesselationLevels = NULL;

	// Take care of the TerrainObject for this component
	if (TerrainObject)
	{
		// Begin releasing the RHI resources used by this skeletal mesh component.
		// This doesn't immediately destroy anything, since the rendering thread may still be using the resources.
		TerrainObject->ReleaseResources();

		TerrainObject->SetIsDeadInGameThread(TRUE);

		// Begin a deferred delete of MeshObject.  BeginCleanup will call MeshObject->FinishDestroy after the above release resource
		// commands execute in the rendering thread.
		BeginCleanup(TerrainObject);

		TerrainObject = NULL;
	}

	Super::Detach();
}

/**
 * Returns context specific material.
 *
 * @param	Material	material to apply context sensitive changes
 * @param	Context		relevant context (used to retrieve show flags)
 * @return	material adapted depending on context like selection or show flags
 */
FMaterialInstance* UTerrainComponent::GetContextSpecificMaterial( FMaterialInstance* Material, const FSceneView* View )
{
	// Try to find a color for level coloration.
	FColor* LevelColor = NULL;
#if GEMINI_TODO
	if ( View->Family->ShowFlags & SHOW_LevelColoration )
	{
		if ( Owner )
		{
			ULevel* Level = Owner->GetLevel();
			ULevelStreaming* LevelStreaming = FLevelUtils::FindStreamingLevel( Level );
			if ( LevelStreaming )
			{
				LevelColor = &LevelStreaming->DrawColor;
			}
		}
	}

		Material = Context.GetRenderMaterial(Material,FColor(255,255,255),IsOwnerSelected(),&Lights,LevelColor);
#endif

	return Material;
}

//
//	FTerrainObject
//
FTerrainObject::~FTerrainObject()
{
	appFree(TessellationLevels);
	delete FullVertexBuffer;
	FullVertexBuffer = NULL;
	delete FullIndexBuffer;
	FullIndexBuffer = NULL;
	delete SmoothIndexBuffer;
	SmoothIndexBuffer = NULL;
	delete VertexBuffer;
	VertexBuffer = NULL;
	delete VertexFactory;
	VertexFactory = NULL;
	delete DecalVertexFactory;
	DecalVertexFactory = NULL;
	delete CollisionVertexBuffer;
	CollisionVertexBuffer = NULL;
	delete CollisionSmoothIndexBuffer;
	CollisionSmoothIndexBuffer = NULL;
}

void FTerrainObject::Init()
{
	check(TerrainComponent);

	ATerrain* Terrain = TerrainComponent->GetTerrain();

	ComponentSectionSizeX		= TerrainComponent->SectionSizeX;
	ComponentSectionSizeY		= TerrainComponent->SectionSizeY;
	ComponentSectionBaseX		= TerrainComponent->SectionBaseX;
	ComponentSectionBaseY		= TerrainComponent->SectionBaseY;
	ComponentTrueSectionSizeX	= TerrainComponent->TrueSectionSizeX;
	ComponentTrueSectionSizeY	= TerrainComponent->TrueSectionSizeY;
	NumVerticesX				= Terrain->NumVerticesX;
	NumVerticesY				= Terrain->NumVerticesY;
	MaxTessellationLevel		= Terrain->MaxTesselationLevel;
	MinTessellationLevel		= Terrain->MinTessellationLevel;
	EditorTessellationLevel		= Terrain->EditorTessellationLevel;
	TerrainHeightScale			= TERRAIN_ZSCALE;
	TessellationDistanceScale	= Terrain->TesselationDistanceScale;
	LightMapResolution			= Terrain->StaticLightingResolution;

//	ShadowCoordinateScale		= ;
//	ShadowCoordinateBias		= ;

	INT TessellationLevelsCount		= (ComponentSectionSizeX + 2) * (ComponentSectionSizeY + 2);
	TessellationLevels = (BYTE*)appRealloc(TessellationLevels, TessellationLevelsCount);
	check(TessellationLevels);

	//
	if (GIsEditor && (Terrain->EditorTessellationLevel != 0))
	{
		if (Terrain->EditorTessellationLevel <= MaxTessellationLevel)
		{
			MaxTessellationLevel = Terrain->EditorTessellationLevel;
			MinTessellationLevel = Terrain->EditorTessellationLevel;
		}
	}

	// Prep the tessellation level of each terrain quad.
	INT Index = 0;
	for (INT Y = -1; Y <= ComponentSectionSizeY; Y++)
	{
		for (INT X = -1; X <= ComponentSectionSizeX; X++)
		{
			TessellationLevels[Index++] = MaxTessellationLevel;
		}
	}

	FullVertexFactory.SetTerrainObject(this);
	FullVertexFactory.SetTessellationLevel(1);

	FullDecalVertexFactory.SetTerrainObject(this);
	FullDecalVertexFactory.SetTessellationLevel(1);
}

void FTerrainObject::InitResources()
{
	FullVertexBuffer = ::new FTerrainFullVertexBuffer(this, TerrainComponent, 1);
	check(FullVertexBuffer);
	BeginInitResource(FullVertexBuffer);

	// update vertex factory components and sync it
	TSetResourceDataContext<FTerrainVertexFactory> FullVertexFactoryData(&FullVertexFactory);
	// position
	FullVertexFactoryData->PositionComponent = FVertexStreamComponent(
#if UBYTE4_BYTEORDER_XYZW
		FullVertexBuffer, STRUCT_OFFSET(FTerrainVertex, X), sizeof(FTerrainVertex), VET_UByte4);
#else
		FullVertexBuffer, STRUCT_OFFSET(FTerrainVertex, Z_HIBYTE), sizeof(FTerrainVertex), VET_UByte4);
#endif
	// displacement
	FullVertexFactoryData->DisplacementComponent = FVertexStreamComponent(
		FullVertexBuffer, STRUCT_OFFSET(FTerrainVertex, Displacement), sizeof(FTerrainVertex), VET_Float1);
	// gradients
	FullVertexFactoryData->GradientComponent = FVertexStreamComponent(
		FullVertexBuffer, STRUCT_OFFSET(FTerrainVertex, GradientX), sizeof(FTerrainVertex), VET_Short2);

	// copy it
	FullVertexFactoryData.Commit();
	// init rendering resource	
	BeginInitResource(&FullVertexFactory);

	//// Decals

	// update vertex factory components and sync it
	TSetResourceDataContext<FTerrainDecalVertexFactory> FullDecalVertexFactoryData(&FullDecalVertexFactory);
	// position
	FullDecalVertexFactoryData->PositionComponent = FVertexStreamComponent(
#if UBYTE4_BYTEORDER_XYZW
		FullVertexBuffer, STRUCT_OFFSET(FTerrainVertex, X), sizeof(FTerrainVertex), VET_UByte4);
#else
		FullVertexBuffer, STRUCT_OFFSET(FTerrainVertex, Z_HIBYTE), sizeof(FTerrainVertex), VET_UByte4);
#endif
	// displacement
	FullDecalVertexFactoryData->DisplacementComponent = FVertexStreamComponent(
		FullVertexBuffer, STRUCT_OFFSET(FTerrainVertex, Displacement), sizeof(FTerrainVertex), VET_Float1);
	// gradients
	FullDecalVertexFactoryData->GradientComponent = FVertexStreamComponent(
		FullVertexBuffer, STRUCT_OFFSET(FTerrainVertex, GradientX), sizeof(FTerrainVertex), VET_Short2);

	// copy it
	FullDecalVertexFactoryData.Commit();
	// init rendering resource	
	BeginInitResource(&FullDecalVertexFactory);

	//// Decals

	// Allocate it...
	FullIndexBuffer = new FTerrainIndexBuffer(this);
	check(FullIndexBuffer);
	BeginInitResource(FullIndexBuffer);

	//
	ATerrain* Terrain = TerrainComponent->GetTerrain();

	if ((GIsGame == TRUE) && (MinTessellationLevel == MaxTessellationLevel))
	{
		VertexBuffer = ::new FTerrainVertexBuffer(this, TerrainComponent, MaxTessellationLevel);
	}
	else
	{
		VertexBuffer = ::new FTerrainVertexBuffer(this, TerrainComponent, MaxTessellationLevel, TRUE);
	}
	check(VertexBuffer);
	BeginInitResource(VertexBuffer);

	VertexFactory = ::new FTerrainVertexFactory();
	check(VertexFactory);
	VertexFactory->SetTerrainObject(this);
	VertexFactory->SetTessellationLevel(MaxTessellationLevel);

	// update vertex factory components and sync it
	TSetResourceDataContext<FTerrainVertexFactory> VertexFactoryData(VertexFactory);
	// position
	VertexFactoryData->PositionComponent = FVertexStreamComponent(
#if UBYTE4_BYTEORDER_XYZW
		VertexBuffer, STRUCT_OFFSET(FTerrainVertex, X), sizeof(FTerrainVertex), VET_UByte4);
#else
		VertexBuffer, STRUCT_OFFSET(FTerrainVertex, Z_HIBYTE), sizeof(FTerrainVertex), VET_UByte4);
#endif
	// displacement
	VertexFactoryData->DisplacementComponent = FVertexStreamComponent(
		VertexBuffer, STRUCT_OFFSET(FTerrainVertex, Displacement), sizeof(FTerrainVertex), VET_Float1);
	// gradients
	VertexFactoryData->GradientComponent = FVertexStreamComponent(
		VertexBuffer, STRUCT_OFFSET(FTerrainVertex, GradientX), sizeof(FTerrainVertex), VET_Short2);

	// copy it
	VertexFactoryData.Commit();
	// init rendering resource	
	BeginInitResource(VertexFactory);

	//// Decals
	DecalVertexFactory = ::new FTerrainDecalVertexFactory();
	check(DecalVertexFactory);
	DecalVertexFactory->SetTerrainObject(this);
	DecalVertexFactory->SetTessellationLevel(MaxTessellationLevel);

	// update vertex factory components and sync it
	TSetResourceDataContext<FTerrainDecalVertexFactory> DecalVertexFactoryData(DecalVertexFactory);
	// position
	DecalVertexFactoryData->PositionComponent = FVertexStreamComponent(
#if UBYTE4_BYTEORDER_XYZW
		VertexBuffer, STRUCT_OFFSET(FTerrainVertex, X), sizeof(FTerrainVertex), VET_UByte4);
#else
		VertexBuffer, STRUCT_OFFSET(FTerrainVertex, Z_HIBYTE), sizeof(FTerrainVertex), VET_UByte4);
#endif
	// displacement
	DecalVertexFactoryData->DisplacementComponent = FVertexStreamComponent(
		VertexBuffer, STRUCT_OFFSET(FTerrainVertex, Displacement), sizeof(FTerrainVertex), VET_Float1);
	// gradients
	DecalVertexFactoryData->GradientComponent = FVertexStreamComponent(
		VertexBuffer, STRUCT_OFFSET(FTerrainVertex, GradientX), sizeof(FTerrainVertex), VET_Short2);

	// copy it
	DecalVertexFactoryData.Commit();
	// init rendering resource	
	BeginInitResource(DecalVertexFactory);
	//// Decals

	INT BatchMatCount = TerrainComponent->BatchMaterials.Num();

	// Create the tessellation index buffer
#if CONSOLE
	SmoothIndexBuffer = ::new FTerrainTessellationIndexBuffer(this, MaxTessellationLevel, FALSE, FALSE);
#else	//#if CONSOLE
	if ((GIsGame == TRUE) && (MinTessellationLevel == MaxTessellationLevel))
	{
		SmoothIndexBuffer = ::new FTerrainTessellationIndexBuffer(this, MaxTessellationLevel, FALSE, FALSE);
	}
	else
	{
		SmoothIndexBuffer = ::new FTerrainTessellationIndexBuffer(this, MaxTessellationLevel, FALSE);
	}
#endif	//#if CONSOLE
	check(SmoothIndexBuffer);
	BeginInitResource(SmoothIndexBuffer);

	if (bIsShowingCollision)
	{
		// Setup the vertex buffer
		CollisionVertexBuffer = ::new FTerrainVertexBuffer(this, TerrainComponent, Terrain->CollisionTesselationLevel);
		check(CollisionVertexBuffer);
		CollisionVertexBuffer->SetIsCollisionLevel(TRUE);
		BeginInitResource(CollisionVertexBuffer);

		// Setup the vertex factory
		CollisionVertexFactory = ::new FTerrainVertexFactory();
		check(CollisionVertexFactory);
		CollisionVertexFactory->SetTerrainObject(this);
		CollisionVertexFactory->SetTessellationLevel(Terrain->CollisionTesselationLevel);
		// update vertex factory components and sync it
		TSetResourceDataContext<FTerrainVertexFactory> VertexFactoryData(CollisionVertexFactory);
		// position
		VertexFactoryData->PositionComponent = FVertexStreamComponent(
#if UBYTE4_BYTEORDER_XYZW
			CollisionVertexBuffer, STRUCT_OFFSET(FTerrainVertex, X), sizeof(FTerrainVertex), VET_UByte4);
#else
			CollisionVertexBuffer, STRUCT_OFFSET(FTerrainVertex, Z_HIBYTE), sizeof(FTerrainVertex), VET_UByte4);
#endif
		// displacement
		VertexFactoryData->DisplacementComponent = FVertexStreamComponent(
			CollisionVertexBuffer, STRUCT_OFFSET(FTerrainVertex, Displacement), sizeof(FTerrainVertex), VET_Float1);
		// gradients
		VertexFactoryData->GradientComponent = FVertexStreamComponent(
			CollisionVertexBuffer, STRUCT_OFFSET(FTerrainVertex, GradientX), sizeof(FTerrainVertex), VET_Short2);

		// copy it
		VertexFactoryData.Commit();
		// init rendering resource	
		BeginInitResource(CollisionVertexFactory);

		//@todo. Store this at the terrain level as it can be used to render ALL components.
		CollisionSmoothIndexBuffer = ::new FTerrainTessellationIndexBuffer(this, Terrain->CollisionTesselationLevel, FALSE);
		check(CollisionSmoothIndexBuffer);
		CollisionSmoothIndexBuffer->bIsCollisionLevel = TRUE;
		BeginInitResource(CollisionSmoothIndexBuffer);
	}
}

void FTerrainObject::ReleaseResources()
{
	if (FullIndexBuffer)
	{
		BeginReleaseResource(FullIndexBuffer);
	}
	BeginReleaseResource(&FullVertexFactory);
	BeginReleaseResource(&FullDecalVertexFactory);
	if (FullVertexBuffer)
	{
		BeginReleaseResource(FullVertexBuffer);
	}

	if (SmoothIndexBuffer)
	{
		BeginReleaseResource(SmoothIndexBuffer);
	}

	if (VertexFactory)
	{
		BeginReleaseResource(VertexFactory);
	}
	if (DecalVertexFactory)
	{
		BeginReleaseResource(DecalVertexFactory);
	}
	if (VertexBuffer)
	{
		BeginReleaseResource(VertexBuffer);
	}

	if (CollisionVertexFactory)
	{
		BeginReleaseResource(CollisionVertexFactory);
	}
	if (CollisionVertexBuffer)
	{
		BeginReleaseResource(CollisionVertexBuffer);
	}
	if (CollisionSmoothIndexBuffer)
	{
		BeginReleaseResource(CollisionSmoothIndexBuffer);
	}
}

void FTerrainObject::Update()
{
/***
	// create the new dynamic data for use by the rendering thread
	// this data is only deleted when another update is sent
	FDynamicTerrainData* NewDynamicData = new FDynamicTerrainData(this);

	// queue a call to update this data
	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		TerrainObjectUpdateDataCommand,
		FTerrainObject*, TerrainObject, this,
		FDynamicTerrainData*, NewDynamicData, NewDynamicData,
		{
			TerrainObject->UpdateDynamicData_RenderThread(NewDynamicData);
		}
	);
***/
}

const FVertexFactory* FTerrainObject::GetVertexFactory() const
{
	return VertexFactory;
}

static UBOOL s_bGenerateTerrainResourcesEveryFrame = FALSE;
UBOOL FTerrainObject::UpdateResources(INT TessellationLevel, UBOOL bRepackRequired)
{
	UBOOL bReturn = TRUE;

	// Handle the vertex buffer
	check(VertexBuffer);
	if ((VertexBuffer->GetCurrentTessellation() != TessellationLevel) ||
		(VertexBuffer->GetRepackRequired() == TRUE))
	{
		// We need to repack this
		VertexBuffer->FillData(TessellationLevel);

		// Update the vertex factory
		check(VertexFactory);
		VertexFactory->SetTessellationLevel(MaxTessellationLevel);

		// Update the decal vertex factory
		check(DecalVertexFactory);
		DecalVertexFactory->SetTessellationLevel(MaxTessellationLevel);

		// Force the index buffer(s) to repack as well
		bRepackRequired = TRUE;

		VertexBuffer->ClearRepackRequired();
	}

	// Handle the index buffer(s)
	if (bRepackRequired == TRUE)
	{
		//@todo.SAS. Fix this usage pattern...
		// This will prevent deleting and recreating the index buffers each time 
		// a repack is required.

		// Check the tessellation level of each index buffer vs. the cached tessellation level
		INT MaxTessellation = MaxTessellationLevel;
		if (SmoothIndexBuffer)
		{
			if (SmoothIndexBuffer->MaxTesselationLevel != MaxTessellation)
			{
				SmoothIndexBuffer->Release();
				delete SmoothIndexBuffer;
#if CONSOLE
				SmoothIndexBuffer = ::new FTerrainTessellationIndexBuffer(this, MaxTessellationLevel, TRUE, FALSE);
#else	//#if CONSOLE
				SmoothIndexBuffer = ::new FTerrainTessellationIndexBuffer(this, MaxTessellationLevel, TRUE);
#endif	//#if CONSOLE
			}

			SmoothIndexBuffer->SetCurrentTessellationLevel(TessellationLevel);
			SmoothIndexBuffer->PrimeBuffer();
			if ((SmoothIndexBuffer->NumTriangles > 0) && (IsValidRef(SmoothIndexBuffer->IndexBufferRHI) == FALSE))
			{
				debugf(TEXT("INVALID TERRAIN INDEX BUFFER 0x%08x!"), SmoothIndexBuffer);
			}
			if (SmoothIndexBuffer->NumTriangles > 0)
			{
				SmoothIndexBuffer->FillData();
			}
		}
	}

	bReturn = (VertexBuffer && VertexFactory && DecalVertexFactory);

	return bReturn;
}

FPrimitiveSceneProxy* UTerrainComponent::CreateSceneProxy()
{
	FTerrainComponentSceneProxy* TCompProxy = NULL;

	// only create a scene proxy for rendering if properly initialized
	if (TerrainObject)
	{
		TCompProxy = ::new FTerrainComponentSceneProxy(this);
		TCompProxy->UpdateData(this);
	}

	return (FPrimitiveSceneProxy*)TCompProxy;
}

//
//	FTerrainIndexBuffer
//
void FTerrainIndexBuffer::InitRHI()
{
	SCOPE_CYCLE_COUNTER(STAT_TerrainSmoothTime);

	INT SectBaseX = TerrainObject->GetComponentSectionBaseX();
	INT SectBaseY = TerrainObject->GetComponentSectionBaseY();
	INT	SectSizeX = TerrainObject->GetComponentSectionSizeX();
	INT	SectSizeY = TerrainObject->GetComponentSectionSizeY();

	UINT Stride = sizeof(WORD);
	UINT Size = 2 * 3 * SectSizeX * SectSizeY * Stride;

	IndexBufferRHI = RHICreateIndexBuffer(Stride, Size, NULL, FALSE);

	// Lock the buffer.
	WORD* DestIndex = (WORD*)(RHILockIndexBuffer(IndexBufferRHI, 0, Size));

	// Fill in the index data
	// Zero initialize index buffer if we're rendering it for the very first time as we're going to render more quads than are
	// visible. Zeroing them out will cause the extraneous triangles to be degenerate so it won't have an effect on visuals.
	if (NumVisibleTriangles == INDEX_NONE)
	{
		appMemzero(DestIndex, Size);
	}

	NumVisibleTriangles = 0;

	for (INT Y = 0; Y < SectSizeY; Y++)
	{
		for (INT X = 0; X < SectSizeX; X++)
		{
			if (TerrainObject->TerrainComponent->GetTerrain()->IsTerrainQuadVisible(SectBaseX + X, SectBaseY + Y) == FALSE)
			{
				continue;
			}

			INT	V1 = Y*(SectSizeX+1) + X;
			INT	V2 = V1+1;
			INT	V3 = (Y+1)*(SectSizeX+1) + (X+1);
			INT	V4 = V3-1;

			*DestIndex++ = V1;
			*DestIndex++ = V4;
			*DestIndex++ = V3;
			NumVisibleTriangles++;

			*DestIndex++ = V3;
			*DestIndex++ = V2;
			*DestIndex++ = V1;
			NumVisibleTriangles++;
		}
	}

	// Unlock the buffer.
	RHIUnlockIndexBuffer(IndexBufferRHI);
}

//
//	FTerrainComponentSceneProxy
//
FTerrainComponentSceneProxy::FTerrainBatchInfo::FTerrainBatchInfo(
	UTerrainComponent* Component, INT BatchIndex)
{
	ATerrain* Terrain = Component->GetTerrain();

	FTerrainMaterialMask Mask(1);

	if (BatchIndex == -1)
	{
		Mask = Component->BatchMaterials(Component->FullBatch);
	}
	else
	{
		Mask = Component->BatchMaterials(BatchIndex);
	}

	// Fetch the material instance
	MaterialInstance = Terrain->GetCachedMaterial(Mask, bIsTerrainMaterialResourceInstance);

	// Fetch the required weight maps
	WeightMaps.Empty();
	for (INT MaterialIndex = 0; MaterialIndex < Mask.Num(); MaterialIndex++)
	{
		if (Mask.Get(MaterialIndex))
		{
			FTerrainWeightedMaterial* WeightedMaterial = &(Terrain->WeightedMaterials(MaterialIndex));
			check(WeightedMaterial);
			INT TextureIndex = MaterialIndex / 4;
			if (TextureIndex < Terrain->WeightedTextureMaps.Num())
			{
				UTerrainWeightMapTexture* Texture = Terrain->WeightedTextureMaps(TextureIndex);
				check(Texture && TEXT("Terrain weight texture map not present!"));
				WeightMaps.AddUniqueItem(Texture);
			}
		}
	}
}

FTerrainComponentSceneProxy::FTerrainBatchInfo::~FTerrainBatchInfo()
{
	// Just empty the array...
	WeightMaps.Empty();
}

FTerrainComponentSceneProxy::FTerrainMaterialInfo::FTerrainMaterialInfo(UTerrainComponent* Component)
{
	// Set to the size of the BatchInfoArray (Batch count + 1 for full)
	BatchInfoArray.Empty(Component->BatchMaterials.Num() + 1);
	BatchInfoArray.Add(Component->BatchMaterials.Num() + 1);

	// Set the full batch info entry
	BatchInfoArray(0) = new FTerrainBatchInfo(Component, -1);

	// Now do each batch material
	for (INT BatchIndex = 0; BatchIndex < Component->BatchMaterials.Num(); BatchIndex++)
	{
		BatchInfoArray(BatchIndex + 1) = new FTerrainBatchInfo(Component, BatchIndex);
	}

	ComponentLightInfo = new FTerrainComponentInfo(*Component);
	check(ComponentLightInfo);
}

FTerrainComponentSceneProxy::FTerrainMaterialInfo::~FTerrainMaterialInfo()
{
	delete ComponentLightInfo;
	ComponentLightInfo = NULL;

	for (INT BatchIndex = 0; BatchIndex < BatchInfoArray.Num(); BatchIndex++)
	{
		FTerrainBatchInfo* BatchInfo = BatchInfoArray(BatchIndex);
		delete BatchInfo;
		BatchInfoArray(BatchIndex) = NULL;
	}
	BatchInfoArray.Empty();
}

FTerrainComponentSceneProxy::FTerrainComponentSceneProxy(UTerrainComponent* Component) :
	  FPrimitiveSceneProxy(Component)
	, Owner(Component->GetOwner())
	, ComponentOwner(Component)
	, TerrainObject(Component->TerrainObject)
	, bSelected(Component->IsOwnerSelected())
	, LevelColor(255,255,255)
	, PropertyColor(255,255,255)	
	, bCastShadow(Component->CastShadow)
	, SelectedWireframeMaterialInstance(
		GEngine->WireframeMaterial->GetInstanceInterface(FALSE), 
		GetSelectionColor(FLinearColor::White,TRUE)
		)
	, DeselectedWireframeMaterialInstance(
		GEngine->WireframeMaterial->GetInstanceInterface(FALSE), 
		GetSelectionColor(FLinearColor::White,FALSE)
		)
	, CurrentMaterialInfo(NULL)
	, MaxTessellation(0)
{
	// Try to find a color for level coloration.
	if (Owner)
	{
		ULevel* Level = Owner->GetLevel();
		ULevelStreaming* LevelStreaming = FLevelUtils::FindStreamingLevel(Level);
		if (LevelStreaming)
		{
			LevelColor = LevelStreaming->DrawColor;
		}
	}

	// Get a color for property coloration.
	GEngine->GetPropertyColorationColor( (UObject*)Component, PropertyColor );
}

UBOOL FTerrainComponentSceneProxy::IsCacheable() const
{
	return TRUE;
}

UBOOL FTerrainComponentSceneProxy::IsStillValid( const UPrimitiveComponent* InComponent ) const
{
	const UTerrainComponent* Component = static_cast<const UTerrainComponent*>(InComponent);

	return GIsGame && FPrimitiveSceneProxy::IsStillValid( InComponent );		
}

FTerrainComponentSceneProxy::~FTerrainComponentSceneProxy()
{
	delete CurrentMaterialInfo;
	CurrentMaterialInfo = NULL;
}

void FTerrainComponentSceneProxy::DrawDynamicElements(FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex)
{
	SCOPE_CYCLE_COUNTER(STAT_TerrainRenderTime);
	
	// Determine the DPG the primitive should be drawn in for this view.
	BYTE PrimitiveDPG = GetDepthPriorityGroup(View);

	if (PrimitiveDPG == DPGIndex)
	{
		if (!TerrainObject || 
			((View->Family->ShowFlags & SHOW_Terrain) == 0))
		{
			return;
		}		  

		//@todo.SAS. Block on the thread that is doing tessellation.

		//@todo.SAS. Is this REALLY bad or not?
		ATerrain* Terrain = Cast<ATerrain>(Owner);
		check(Terrain);

		// Render it...
		const FMatrix&	LocalToView = LocalToWorld * View->ViewMatrix;
		MaxTessellation = TerrainObject->MinTessellationLevel;
		if (TerrainObject->MinTessellationLevel != TerrainObject->MaxTessellationLevel)
		{
			// Calculate the maximum tessellation level for this sector.
			if (View->ProjectionMatrix.M[3][3] < 1.0f)
			{
				for (INT X = 0; X < 2; X++)
				{
					for (INT Y = 0; Y < 2; Y++)
					{
						for (INT Z = 0; Z < 2; Z++)
						{
							MaxTessellation = Max(
								MaxTessellation,
								Min(
									(UINT)TerrainObject->MaxTessellationLevel,
									TesselationLevel(View->ViewMatrix.TransformFVector(
										FVector(
											PrimitiveSceneInfo->Bounds.GetBoxExtrema(X).X,
											PrimitiveSceneInfo->Bounds.GetBoxExtrema(Y).Y,
											PrimitiveSceneInfo->Bounds.GetBoxExtrema(Z).Z)).Z * 
										TerrainObject->TessellationDistanceScale,
										TerrainObject->MinTessellationLevel)
									)
								);
						}
					}
				}
			}
		}

		INT TerrainMaxTessLevel = TerrainObject->GetMaxTessellationLevel();
		INT SectBaseX = TerrainObject->GetComponentSectionBaseX();
		INT SectBaseY = TerrainObject->GetComponentSectionBaseY();
		INT SectSizeX = TerrainObject->GetComponentSectionSizeX();
		INT SectSizeY = TerrainObject->GetComponentSectionSizeY();
		INT NumVertsX = TerrainObject->GetNumVerticesX();
		INT NumVertsY = TerrainObject->GetNumVerticesY();

		// Setup a vertex buffer/factory for this tessellation level.
		if (1)//((MaxTessellation > 1) || (s_bUseBatches == FALSE))
		{
			UBOOL bRepackRequired = FALSE;
			// Determine the tessellation level of each terrain quad.
			INT Index = 0;
			FVector TerrainVector;
			TerrainVector.Y = -1.0f;

			INT QuadSizeX = TerrainObject->TerrainComponent->TrueSectionSizeX / SectSizeX;
			INT QuadSizeY = TerrainObject->TerrainComponent->TrueSectionSizeY / SectSizeY;
			INT OffsetX = QuadSizeX / 2;
			INT OffsetY = QuadSizeY / 2;

			TerrainVector.Y *= QuadSizeY;
			for(INT Y = -1;Y <= SectSizeY;Y++, TerrainVector.Y += QuadSizeY)
			{
				TerrainVector.X = -1.0f * QuadSizeX;
				for(INT X = -1;X <= SectSizeX;X++, TerrainVector.X += QuadSizeX)
				{
					if (((SectBaseX + X * QuadSizeX + OffsetX) >= NumVertsX) || 
						((SectBaseY + Y * QuadSizeY + OffsetY) >= NumVertsY) || 
						((SectBaseX + X * QuadSizeX + OffsetX) < 0) || 
						((SectBaseY + Y * QuadSizeY + OffsetY) < 0))
					{
						TerrainObject->TessellationLevels[Index++] = TerrainMaxTessLevel;
						continue;
					}

					const INT TerrainOldTessellationLevel = TerrainObject->TessellationLevels[Index];

					// A load-hit-store is inevitable here because Terrain->Height indexes a WORD array which we have to expand to 32-bit before using a conversion function
					FLOAT ZDistance = 0.0f;
					if (View->ProjectionMatrix.M[3][3] < 1.0f)
					{
						FLOAT Height = Terrain->Height(SectBaseX + X * QuadSizeX + OffsetX, SectBaseY + Y * QuadSizeY + OffsetY);
						TerrainVector.X = X * QuadSizeX + OffsetX;
						TerrainVector.Y = Y * QuadSizeY + OffsetY;
						TerrainVector.Z = (-32768.0f + Height) * TERRAIN_ZSCALE;
						FVector ViewVector = LocalToView.TransformFVector(TerrainVector);
						ZDistance = ViewVector.Z * TerrainObject->GetTessellationDistanceScale();
					}
					if ((GIsEditor == TRUE) && (Abs(ZDistance) < KINDA_SMALL_NUMBER))
					{
						TerrainObject->TessellationLevels[Index++] = (BYTE)(TerrainObject->MinTessellationLevel);
					}
					else
					{
						TerrainObject->TessellationLevels[Index++] = (BYTE)Min<UINT>(
								TerrainMaxTessLevel,
								TesselationLevel(ZDistance, TerrainObject->MinTessellationLevel)
								);
					}

					if (TerrainOldTessellationLevel != TerrainObject->TessellationLevels[Index-1])
					{
						bRepackRequired = TRUE;
					}
				}
			}

			// If there are an unequal number of index buffers compared to BatchMaterials, force a repack.
			if (TerrainObject->SmoothIndexBuffer->RepackRequired == TRUE)
			{
				bRepackRequired = TRUE;
			}

			// Update the resources for this render call
			TerrainObject->UpdateResources(MaxTessellation, bRepackRequired);

			// Render
			Meshes.Empty( 1 );

			UBOOL bShowingCollision = ((TerrainObject->bIsShowingCollision == TRUE) && 
					((View->Family->ShowFlags & SHOW_TerrainCollision) != 0));

			const INT TriCount = TerrainObject->SmoothIndexBuffer->NumTriangles;
			FMeshElement Mesh;
			Mesh.NumPrimitives = TriCount;
			if (TriCount != 0)
			{
				INC_DWORD_STAT_BY(STAT_TerrainTriangles,TriCount);

				Mesh.IndexBuffer = TerrainObject->SmoothIndexBuffer;
				Mesh.VertexFactory = TerrainObject->VertexFactory;
				Mesh.DynamicVertexData = NULL;
				Mesh.DynamicVertexStride = 0;
				Mesh.DynamicIndexData = NULL;
				Mesh.DynamicIndexStride = 0;
				Mesh.LCI = CurrentMaterialInfo->ComponentLightInfo;

				// The full batch is stored at entry 0
				FTerrainBatchInfo* BatchInfo = CurrentMaterialInfo->BatchInfoArray(0);
				FMaterialInstance* MaterialInstance = BatchInfo->MaterialInstance;
				if (bShowingCollision && (GEngine->bRenderTerrainCollisionAsOverlay == FALSE))
				{
					UMaterial* CollisionMat = GEngine->TerrainCollisionMaterial;
					if (CollisionMat)
					{
						MaterialInstance = CollisionMat->GetInstanceInterface(0);
						Mesh.LCI = NULL;
					}
				}
				if (MaterialInstance == NULL)
				{
					MaterialInstance = GEngine->DefaultMaterial->GetInstanceInterface(0);
				}
				Mesh.MaterialInstance = MaterialInstance;

				Mesh.LocalToWorld = LocalToWorld;
				Mesh.WorldToLocal = LocalToWorld.Inverse();				
				Mesh.FirstIndex = 0;
				Mesh.MinVertexIndex = 0;
				check(TerrainObject->VertexBuffer);
				Mesh.MaxVertexIndex = TerrainObject->VertexBuffer->GetVertexCount() - 1;
				Mesh.UseDynamicData = FALSE;
				Mesh.ReverseCulling = LocalToWorldDeterminant < 0.0f ? TRUE : FALSE;;
				Mesh.CastShadow = bCastShadow;
				Mesh.Type = PT_TriangleList;
				Mesh.DepthPriorityGroup = (ESceneDepthPriorityGroup)PrimitiveDPG;

				DrawRichMesh(PDI,Mesh,FLinearColor::White,LevelColor,PropertyColor,PrimitiveSceneInfo,bSelected);
			}
			Meshes.AddItem( Mesh );

			if (bShowingCollision)
			{
				if ((TerrainObject->CollisionVertexFactory != NULL) &&
					(TerrainObject->CollisionSmoothIndexBuffer != NULL) &&
					(TerrainObject->CollisionVertexBuffer != NULL))
				{
					const INT TriCount = TerrainObject->CollisionSmoothIndexBuffer->NumTriangles;
					FMeshElement Mesh;
					Mesh.NumPrimitives = TriCount;
					if (TriCount != 0)
					{
						INC_DWORD_STAT_BY(STAT_TerrainTriangles,TriCount);

						Mesh.IndexBuffer = TerrainObject->CollisionSmoothIndexBuffer;
						Mesh.VertexFactory = TerrainObject->CollisionVertexFactory;
						Mesh.DynamicVertexData = NULL;
						Mesh.DynamicVertexStride = 0;
						Mesh.DynamicIndexData = NULL;
						Mesh.DynamicIndexStride = 0;
						Mesh.LCI = CurrentMaterialInfo->ComponentLightInfo;

						FTerrainBatchInfo* BatchInfo = CurrentMaterialInfo->BatchInfoArray(0);
						FMaterialInstance* MaterialInstance = BatchInfo->MaterialInstance;
						if (MaterialInstance == NULL)
						{
							MaterialInstance = GEngine->DefaultMaterial->GetInstanceInterface(0);
						}
						if (GEngine->bRenderTerrainCollisionAsOverlay == TRUE)
						{
							UMaterial* CollisionMat = GEngine->TerrainCollisionMaterial;
							if (CollisionMat)
							{
								MaterialInstance = CollisionMat->GetInstanceInterface(0);
								Mesh.LCI = NULL;
							}
						}
						if (MaterialInstance == NULL)
						{
							MaterialInstance = GEngine->DefaultMaterial->GetInstanceInterface(0);
						}
						Mesh.MaterialInstance = MaterialInstance;

						Mesh.LocalToWorld = LocalToWorld;
						Mesh.WorldToLocal = LocalToWorld.Inverse();						
						Mesh.FirstIndex = 0;
						Mesh.MinVertexIndex = 0;
						check(TerrainObject->CollisionVertexBuffer);
						Mesh.MaxVertexIndex = TerrainObject->CollisionVertexBuffer->GetVertexCount() - 1;
						Mesh.UseDynamicData = FALSE;
						Mesh.ReverseCulling = LocalToWorldDeterminant < 0.0f ? TRUE : FALSE;;
						Mesh.CastShadow = bCastShadow;
						Mesh.Type = PT_TriangleList;
						Mesh.DepthPriorityGroup = (ESceneDepthPriorityGroup)PrimitiveDPG;

						DrawRichMesh(PDI,Mesh,FLinearColor::White,LevelColor,PropertyColor,PrimitiveSceneInfo,bSelected);
					}
				}
			}
		}
		else 
		{
			if (TerrainObject->FullIndexBuffer == NULL)
			{
				// Allocate it...
  				TerrainObject->FullIndexBuffer = new FTerrainIndexBuffer(TerrainObject);
				check(TerrainObject->FullIndexBuffer);

				TerrainObject->FullIndexBuffer->Init();
			}

			Meshes.Empty( 1 );

			const INT TriCount = TerrainObject->FullIndexBuffer->NumVisibleTriangles;
			FMeshElement Mesh;
			Mesh.NumPrimitives = TriCount;
			if (TriCount != 0)
			{
				INC_DWORD_STAT_BY(STAT_TerrainTriangles,TriCount);

				Mesh.IndexBuffer = TerrainObject->FullIndexBuffer;
				Mesh.VertexFactory = &(TerrainObject->FullVertexFactory),
				Mesh.DynamicVertexData = NULL;
				Mesh.DynamicVertexStride = 0;
				Mesh.DynamicIndexData = NULL;
				Mesh.DynamicIndexStride = 0;

				// The full batch is stored at entry 0
				FTerrainBatchInfo* BatchInfo = CurrentMaterialInfo->BatchInfoArray(0);
				FMaterialInstance* MaterialInstance = BatchInfo->MaterialInstance;
				if (MaterialInstance == NULL)
				{
					MaterialInstance = GEngine->DefaultMaterial->GetInstanceInterface(0);
				}
				Mesh.MaterialInstance = MaterialInstance;

				Mesh.LCI = CurrentMaterialInfo->ComponentLightInfo;
				Mesh.LocalToWorld = LocalToWorld;
				Mesh.WorldToLocal = LocalToWorld.Inverse();				
				Mesh.FirstIndex = 0;
				Mesh.MinVertexIndex = 0;
				check(TerrainObject->FullVertexBuffer);
				Mesh.MaxVertexIndex = TerrainObject->FullVertexBuffer->GetVertexCount() - 1;
				Mesh.UseDynamicData = FALSE;
				Mesh.ReverseCulling = LocalToWorldDeterminant < 0.0f ? TRUE : FALSE;;
				Mesh.CastShadow = bCastShadow;
				Mesh.Type = PT_TriangleList;
				Mesh.DepthPriorityGroup = (ESceneDepthPriorityGroup)PrimitiveDPG;

				DrawRichMesh(PDI,Mesh,FLinearColor::White,LevelColor,PropertyColor,PrimitiveSceneInfo,bSelected);
			}
			Meshes.AddItem( Mesh );
		}

		if (View->Family->ShowFlags & SHOW_TerrainPatches)
		{
			// Draw a rectangle around the component...
			DrawWireBox(PDI, PrimitiveSceneInfo->Bounds.GetBox(), FColor(255, 255, 0), DPGIndex);
		}

		if(View->Family->ShowFlags & SHOW_Foliage)
		{
			DrawFoliage(PDI,View);
		}
	}
}

/**
 * Draws the primitive's decal elements.  This is called from the rendering thread for each frame of each view.
 * The dynamic elements will only be rendered if GetViewRelevance declares decal relevance.
 * Called in the rendering thread.
 *
 * @param	Context							The RHI command context to which the primitives are being rendered.
 * @param	OpaquePDI						The interface which receives the opaque primitive elements.
 * @param	TranslucentPDI					The interface which receives the translucent primitive elements.
 * @param	View							The view which is being rendered.
 * @param	DPGIndex						The DPG which is being rendered.
 * @param	bTranslucentReceiverPass		TRUE during the decal pass for translucent receivers, FALSE for opaque receivers.
 */
void FTerrainComponentSceneProxy::DrawDecalElements(
	FCommandContextRHI* Context, 
	FPrimitiveDrawInterface* OpaquePDI, 
	FPrimitiveDrawInterface* TranslucentPDI, 
	const FSceneView* View, 
	UINT DPGIndex, 
	UBOOL bTranslucentReceiverPass
	)
{
	SCOPE_CYCLE_COUNTER(STAT_DecalRenderTime);
	checkSlow( View->Family->ShowFlags & SHOW_Terrain );
	checkSlow( View->Family->ShowFlags & SHOW_Decals );

	if ( bTranslucentReceiverPass )
	{
		// Decals on terrain with translucent materials not currently supported.
		return;
	}

	// Determine the DPG the primitive should be drawn in for this view.
	BYTE PrimitiveDPG = GetDepthPriorityGroup(View);

	if (PrimitiveDPG == DPGIndex)
	{
		if ( !TerrainObject )
		{
			return;
		}

		if (MaxTessellation > 1)
		{
			for (INT BatchIndex = 0; BatchIndex < ComponentOwner->BatchMaterials.Num(); BatchIndex++)
			{
				if ( Meshes(BatchIndex).NumPrimitives > 0 )
				{
					FMeshElement Mesh( Meshes(BatchIndex) );
					Mesh.LCI = NULL;
					Mesh.VertexFactory = TerrainObject->DecalVertexFactory;
					Mesh.CastShadow = FALSE;

					for ( INT DecalIndex = 0 ; DecalIndex < Decals.Num() ; ++ DecalIndex )
					{
						const FDecalState& DecalState = Decals(DecalIndex)->DecalState;

						// Compute a scissor rect by clip the decal frustum vertices to the screen.
						// Don't draw the decal if the frustum projects off the screen.
						FVector2D MinCorner;
						FVector2D MaxCorner;
						if ( DecalState.QuadToClippedScreenSpaceAABB( View, MinCorner, MaxCorner ) )
						{
							// Set the decal scissor rect.
							RHISetScissorRect( Context, TRUE, appTrunc(MinCorner.X), appTrunc(MinCorner.Y), appTrunc(MaxCorner.X), appTrunc(MaxCorner.Y) );

							Mesh.MaterialInstance = DecalState.DecalMaterial->GetInstanceInterface(0);
							TerrainObject->DecalVertexFactory->SetDecalMatrix( DecalState.WorldTexCoordMtx );
							TerrainObject->DecalVertexFactory->SetDecalLocation( DecalState.HitLocation );
							TerrainObject->DecalVertexFactory->SetDecalOffset( FVector2D(DecalState.OffsetX, DecalState.OffsetY) );

							FPrimitiveDrawInterface* PDI;
							if ( DecalState.bHasUnlitTranslucency )
							{
								PDI = TranslucentPDI;
							}
							else
							{
								RHISetBlendState( Context, TStaticBlendState<>::GetRHI() );
								PDI = OpaquePDI;
							}

							INC_DWORD_STAT_BY(STAT_DecalTriangles,Mesh.NumPrimitives);
							DrawRichMesh(PDI,Mesh,FLinearColor(0.5f,1.0f,0.5f),FLinearColor::White,FLinearColor::White,PrimitiveSceneInfo,FALSE);
						}
					}

					// Restore the scissor rect.
					RHISetScissorRect( Context, FALSE, 0, 0, 0, 0 );
				}
			}
		}
		else 
		{
			if ( Meshes(0).NumPrimitives > 0 )
			{
				FMeshElement Mesh( Meshes(0) );
				Mesh.LCI = NULL;
				Mesh.VertexFactory = &TerrainObject->FullDecalVertexFactory;
				Mesh.CastShadow = FALSE;

				for ( INT DecalIndex = 0 ; DecalIndex < Decals.Num() ; ++ DecalIndex )
				{
					const FDecalState& DecalState = Decals(DecalIndex)->DecalState;

					// Compute a scissor rect by clip the decal frustum vertices to the screen.
					// Don't draw the decal if the frustum projects off the screen.
					FVector2D MinCorner;
					FVector2D MaxCorner;
					if ( DecalState.QuadToClippedScreenSpaceAABB( View, MinCorner, MaxCorner ) )
					{
						// Set the decal scissor rect.
						RHISetScissorRect( Context, TRUE, appTrunc(MinCorner.X), appTrunc(MinCorner.Y), appTrunc(MaxCorner.X), appTrunc(MaxCorner.Y) );

						Mesh.MaterialInstance = DecalState.DecalMaterial->GetInstanceInterface(0);
						TerrainObject->FullDecalVertexFactory.SetDecalMatrix( DecalState.WorldTexCoordMtx );
						TerrainObject->FullDecalVertexFactory.SetDecalLocation( DecalState.HitLocation );
						TerrainObject->FullDecalVertexFactory.SetDecalOffset( FVector2D(DecalState.OffsetX, DecalState.OffsetY) );

						FPrimitiveDrawInterface* PDI;
						if ( DecalState.bHasUnlitTranslucency )
						{
							PDI = TranslucentPDI;
						}
						else
						{
							RHISetBlendState( Context, TStaticBlendState<>::GetRHI() );
							PDI = OpaquePDI;
						}

						INC_DWORD_STAT_BY(STAT_DecalTriangles,Mesh.NumPrimitives);
						DrawRichMesh(PDI,Mesh,FLinearColor(0.5f,1.0f,0.5f),FLinearColor::White,FLinearColor::White,PrimitiveSceneInfo,FALSE);
					}
				}

				// Restore the scissor rect.
				RHISetScissorRect( Context, FALSE, 0, 0, 0, 0 );
			}
		}
	}
}

UBOOL FTerrainComponentSceneProxy::CreateRenderThreadResources()
{
	return TRUE;
}
	
UBOOL FTerrainComponentSceneProxy::ReleaseRenderThreadResources()
{
	return TRUE;
}

void FTerrainComponentSceneProxy::UpdateData(UTerrainComponent* Component)
{
    FTerrainMaterialInfo* NewMaterialInfo = ::new FTerrainMaterialInfo(Component);

	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		TerrainUpdateDataCommand,
		FTerrainComponentSceneProxy*, Proxy, this,
		FTerrainMaterialInfo*, NewMaterialInfo, NewMaterialInfo,
		{
			Proxy->UpdateData_RenderThread(NewMaterialInfo);
		}
		);
}

void FTerrainComponentSceneProxy::UpdateData_RenderThread(FTerrainMaterialInfo* NewMaterialInfo)
{
	// We must be done with it...
	ReleaseRenderThreadResources();
	delete CurrentMaterialInfo;

	// Set the new one
	CurrentMaterialInfo = NewMaterialInfo;
	for (INT BatchInfoIndex = 0; BatchInfoIndex < CurrentMaterialInfo->BatchInfoArray.Num(); BatchInfoIndex++)
	{
		FTerrainBatchInfo* BatchInfo = CurrentMaterialInfo->BatchInfoArray(BatchInfoIndex);
		FMaterialInstance* MaterialInstance = BatchInfo->MaterialInstance;
		if (MaterialInstance && BatchInfo && (BatchInfo->bIsTerrainMaterialResourceInstance == TRUE))
		{
			FTerrainMaterialResource* TWeightInst = (FTerrainMaterialResource*)MaterialInstance;
			TWeightInst->WeightMaps.Empty(BatchInfo->WeightMaps.Num());
			TWeightInst->WeightMaps.Add(BatchInfo->WeightMaps.Num());
			for (INT WeightMapIndex = 0; WeightMapIndex < BatchInfo->WeightMaps.Num(); WeightMapIndex++)
			{
				UTexture2D* NewWeightMap = BatchInfo->WeightMaps(WeightMapIndex);
				TWeightInst->WeightMaps(WeightMapIndex) = NewWeightMap;
			}
		}
	}
	CreateRenderThreadResources();
}
