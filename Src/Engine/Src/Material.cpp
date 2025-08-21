/*=============================================================================
	UnMaterial.cpp: Shader implementation.
	Copyright 2004-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineMaterialClasses.h"
#include "EngineDecalClasses.h"

IMPLEMENT_CLASS(UMaterial);

UBOOL GOptimizingShaderForGameRuntime = FALSE;

FMaterialResource::FMaterialResource(UMaterial* InMaterial):
	Material(InMaterial)
{
}

INT FMaterialResource::CompileProperty(EMaterialProperty Property,FMaterialCompiler* Compiler) const
{
	INT SelectionColorIndex = Compiler->ComponentMask(Compiler->VectorParameter(NAME_SelectionColor,FLinearColor::Black),1,1,1,0);

	//<@ ava specific ; 2007. 1. 22 changmin

	// epic's original
	//switch(Property)
	//{
	//case MP_EmissiveColor:
	//	return Compiler->Add(Compiler->ForceCast(Material->EmissiveColor.Compile(Compiler,FColor(0,0,0)),MCT_Float3),SelectionColorIndex);
	//case MP_Opacity: return Material->Opacity.Compile(Compiler,1.0f);
	//case MP_OpacityMask: return Material->OpacityMask.Compile(Compiler,1.0f);
	//case MP_Distortion: return Material->Distortion.Compile(Compiler,FVector2D(0,0));
	//case MP_TwoSidedLightingMask: return Compiler->Mul(Compiler->ForceCast(Material->TwoSidedLightingMask.Compile(Compiler,0.0f),MCT_Float),Material->TwoSidedLightingColor.Compile(Compiler,FColor(255,255,255)));
	//case MP_DiffuseColor:
	//	return Compiler->Mul(Compiler->ForceCast(Material->DiffuseColor.Compile(Compiler,FColor(128,128,128)),MCT_Float3),Compiler->Sub(Compiler->Constant(1.0f),SelectionColorIndex));
	//case MP_SpecularColor: return Material->SpecularColor.Compile(Compiler,FColor(0,0,0));
	//case MP_SpecularPower: return Material->SpecularPower.Compile(Compiler,15.0f);
	//case MP_Normal: return Material->Normal.Compile(Compiler,FVector(0,0,1));
	//case MP_CustomLighting: return Material->CustomLighting.Compile(Compiler,FColor(0,0,0));
	//case MP_AmbientMask : return Material->AmbientMask.Compile(Compiler,1.0f);
	//default:
	//	return INDEX_NONE;
	//};

	switch(Property)
	{
	case MP_EmissiveColor:
		{
			UBOOL bHasValidExpression = FALSE;
			INT EmissiveCodeIndex = Material->EmissiveColor.Compile( Compiler, FColor(0,0,0), &bHasValidExpression );

			if (GOptimizingShaderForGameRuntime)
				return EmissiveCodeIndex;

			return Compiler->Add(Compiler->ForceCast(EmissiveCodeIndex, MCT_Float3), SelectionColorIndex);
		}
		
	case MP_Opacity:
		{
			UBOOL bHasValidExpression = FALSE;
			return Material->Opacity.Compile( Compiler, 1.0f, &bHasValidExpression );
		}
		
	case MP_OpacityMask:
		{
			UBOOL bHasValidExpression = FALSE;
			INT OpacityMaskCodeIndex = Material->OpacityMask.Compile( Compiler, 1.0f, &bHasValidExpression );
			SetUsesMask( bHasValidExpression );
			return OpacityMaskCodeIndex;
		}
	case MP_Distortion:
		{
			UBOOL bHasValidExpression = FALSE;
			INT DistortionCodeIndex = Material->Distortion.Compile( Compiler, FVector2D(0,0), &bHasValidExpression );
			SetUsesDistortion( bHasValidExpression );
			return DistortionCodeIndex;
		}
		
	case MP_TwoSidedLightingMask:
		{
			UBOOL bHasValidExpression = FALSE;
			INT TSLMCodeIndex = Material->TwoSidedLightingMask.Compile( Compiler, 0.0f, &bHasValidExpression );
			INT TSLCCodeIndex = Material->TwoSidedLightingColor.Compile( Compiler, FColor( 255, 255, 255), &bHasValidExpression );
			return Compiler->Mul(Compiler->ForceCast(TSLMCodeIndex,MCT_Float), TSLCCodeIndex);
		}
	case MP_DiffuseColor:
		{
			UBOOL bHasValidExpression = FALSE;
			INT DiffuseColorCodeIndex = Material->DiffuseColor.Compile(Compiler, FColor(128,128,128), &bHasValidExpression);

			if (GOptimizingShaderForGameRuntime)
				return DiffuseColorCodeIndex;
			else
				return Compiler->Mul(Compiler->ForceCast(DiffuseColorCodeIndex,MCT_Float3),Compiler->Sub(Compiler->Constant(1.0f),SelectionColorIndex));
		}
	case MP_SpecularColor:
		{
			UBOOL bHasValidExpression = FALSE;
			INT SpecularColorCodeIndex = Material->SpecularColor.Compile(Compiler,FColor(0,0,0),&bHasValidExpression);
			SetUsesSpecular( bHasValidExpression );
			return SpecularColorCodeIndex;
		}
	case MP_SpecularPower:
		{
			UBOOL bHasValidExpression = FALSE;
			INT SpecularPowerCodeIndex = Material->SpecularPower.Compile(Compiler,15.0f,&bHasValidExpression);
			return SpecularPowerCodeIndex;
		}
	case MP_Normal:
		{
			UBOOL bHasValidExpression = FALSE;
			INT NormalCodeIndex = Material->Normal.Compile(Compiler,FVector(0,0,1),&bHasValidExpression);
			SetUsesNormal( bHasValidExpression );
			return NormalCodeIndex;
		}
	case MP_CustomLighting:
		{
			UBOOL bHasValidExpression = FALSE;
			INT CodeIndex = Material->CustomLighting.Compile(Compiler, FColor(0,0,0), &bHasValidExpression);
			return CodeIndex;
		}
	case MP_AmbientMask :
		{
			UBOOL bHasValidExpression = FALSE;
			INT CodeIndex = Material->AmbientMask.Compile( Compiler, 1.0f, &bHasValidExpression );
			return CodeIndex;

		}
	default:
		return INDEX_NONE;
	};
	//>@ ava
}

/**
 * A resource which represents the default instance of a UMaterial to the renderer.
 * Note that default parameter values are stored in the FMaterialUniformExpressionXxxParameter objects now.
 * This resource is only responsible for the selection color.
 */
class FDefaultMaterialInstance : public FMaterialInstance
{
public:

	// FMaterialInstance interface.
	virtual const class FMaterial* GetMaterial() const
	{
		const FMaterialResource * MaterialResource = Material->MaterialResource;
		if (MaterialResource && MaterialResource->GetShaderMap())
		{
			return MaterialResource;
		}
		
		// this check is to stop the infinite "retry to compile DefaultMaterial" which can occur when MSP types are mismatched or another similiar error state
		check(this != GEngine->DefaultMaterial->GetInstanceInterface(bSelected));	
		return GEngine->DefaultMaterial->GetInstanceInterface(bSelected)->GetMaterial();		
	}
	virtual UBOOL GetVectorValue(const FName& ParameterName,FLinearColor* OutValue) const
	{
		if(Material->MaterialResource && Material->MaterialResource->GetShaderMap())
		{
			if(ParameterName == NAME_SelectionColor)
			{
				static const FLinearColor SelectionColor(10.0f/255.0f,5.0f/255.0f,60.0f/255.0f,1);
				*OutValue = bSelected ? SelectionColor : FLinearColor::Black;
				return TRUE;
			}
			return FALSE;
		}
		else
		{
			return GEngine->DefaultMaterial->GetInstanceInterface(bSelected)->GetVectorValue(ParameterName,OutValue);
		}
	}
	virtual UBOOL GetScalarValue(const FName& ParameterName,FLOAT* OutValue) const
	{
		if(Material->MaterialResource && Material->MaterialResource->GetShaderMap())
		{
			return FALSE;
		}
		else
		{
			return GEngine->DefaultMaterial->GetInstanceInterface(bSelected)->GetScalarValue(ParameterName,OutValue);
		}
	}
	virtual UBOOL GetTextureValue(const FName& ParameterName,const FTexture** OutValue) const
	{
		if(Material->MaterialResource && Material->MaterialResource->GetShaderMap())
		{
			return FALSE;
		}
		else
		{
			return GEngine->DefaultMaterial->GetInstanceInterface(bSelected)->GetTextureValue(ParameterName,OutValue);
		}
	}

	// Constructor.
	FDefaultMaterialInstance(UMaterial* InMaterial,UBOOL bInSelected):
		Material(InMaterial),
		bSelected(bInSelected)
	{}

private:

	UMaterial* Material;
	UBOOL bSelected;
};

UMaterial::UMaterial()
{
	if(!HasAnyFlags(RF_ClassDefaultObject))
	{
		DefaultMaterialInstances[FALSE] = new FDefaultMaterialInstance(this,FALSE);
		if(GIsEditor)
		{
			DefaultMaterialInstances[TRUE] = new FDefaultMaterialInstance(this,TRUE);
		}
	}
}

/** @return TRUE if the material uses distortion */
UBOOL UMaterial::HasDistortion() const
{
    return bUsesDistortion && IsTranslucentBlendMode((EBlendMode)BlendMode);
}

/** @return TRUE if the material uses the scene color texture */
UBOOL UMaterial::UsesSceneColor() const
{
	return bUsesSceneColor;
}

UBOOL UMaterial::UsesEnvCube() const
{
	return bUsesEnvCube;
}

FMaterialResource* UMaterial::AllocateResource()
{
	return new FMaterialResource(this);
}

UBOOL UMaterial::UseWithSkeletalMesh()
{
	// Check that the material has been flagged for use with skeletal meshes.
	if(!bUsedWithSkeletalMesh && !bUsedAsSpecialEngineMaterial)
	{
		if(GIsEditor)
		{
			// If the flag is missing in the editor, set it, and recompile shaders.
			bUsedWithSkeletalMesh = TRUE;
			MaterialResource->InitShaderMap();
			MarkPackageDirty();
		}
		else
		{
			debugf(TEXT("Material %s used with skeletal mesh, but missing bUsedWithSkeletalMesh=True"),*GetPathName());

			// If the flag is missing in the game, return failure.
			return FALSE;
		}
	}

	return TRUE;
}

UBOOL UMaterial::UseWithParticleSystem()
{
	// Check that the material has been flagged for use with skeletal meshes.
	if(!bUsedWithParticleSystem && !bUsedAsSpecialEngineMaterial)
	{
		if(GIsEditor)
		{
			// If the flag is missing in the editor, set it, and recompile shaders.
			bUsedWithParticleSystem = TRUE;
			MaterialResource->InitShaderMap();
			MarkPackageDirty();
		}
		else
		{
			debugf(TEXT("Material %s used with particle system, but missing bUsedWithParticleSystem=True"),*GetPathName());

			// If the flag is missing in the game, return failure.
			return FALSE;
		}
	}

	return TRUE;
}

UMaterial* UMaterial::GetMaterial()
{
	// This check corresponds to the check in GetInstanceInterface.
	if(!MaterialResource || !MaterialResource->GetShaderMap())
	{
		return GEngine ? GEngine->DefaultMaterial : NULL;
	}
	else
	{
		return this;
	}
}

UBOOL UMaterial::GetVectorParameterValue(FName ParameterName, FLinearColor& OutValue)
{
	return DefaultMaterialInstances[FALSE]->GetVectorValue(ParameterName,&OutValue);
}

UBOOL UMaterial::GetScalarParameterValue(FName ParameterName, FLOAT& OutValue)
{
	return DefaultMaterialInstances[FALSE]->GetScalarValue(ParameterName,&OutValue);
}

UBOOL UMaterial::GetTextureParameterValue(FName ParameterName, UTexture*& OutValue)
{
	UBOOL bSuccess = FALSE;
	for(INT ExpressionIndex = 0;ExpressionIndex < Expressions.Num();ExpressionIndex++)
	{
		UMaterialExpressionTextureSampleParameter* TextureSampleParameter =
			Cast<UMaterialExpressionTextureSampleParameter>(Expressions(ExpressionIndex));

		if(TextureSampleParameter && TextureSampleParameter->ParameterName == ParameterName)
		{
			OutValue = TextureSampleParameter->Texture;
			bSuccess = TRUE;
			break;
		}
	}
	return bSuccess;
}

FMaterialInstance* UMaterial::GetInstanceInterface(UBOOL Selected) const
{
	check(!Selected || GIsEditor);
	return DefaultMaterialInstances[Selected];
}

UPhysicalMaterial* UMaterial::GetPhysicalMaterial()
{
	return PhysMaterial;
}

/**
 * @param	OutParameterNames		Storage array for the parameter names we are returning.
 *
 * @return	Returns a array of vector parameter names used in this material.
 */
void UMaterial::GetAllVectorParameterNames(TArray<FName> &OutParameterNames)
{
	OutParameterNames.Empty();
	for(INT ExpressionIndex = 0;ExpressionIndex < Expressions.Num();ExpressionIndex++)
	{
		UMaterialExpressionVectorParameter* VectorSampleParameter =
			Cast<UMaterialExpressionVectorParameter>(Expressions(ExpressionIndex));

		if(VectorSampleParameter)
		{
			OutParameterNames.AddItem(VectorSampleParameter->ParameterName);
		}
	}
}

/**
 * @param	OutParameterNames		Storage array for the parameter names we are returning.
 *
 * @return	Returns a array of scalar parameter names used in this material.
 */
void UMaterial::GetAllScalarParameterNames(TArray<FName> &OutParameterNames)
{
	OutParameterNames.Empty();
	for(INT ExpressionIndex = 0;ExpressionIndex < Expressions.Num();ExpressionIndex++)
	{
		UMaterialExpressionScalarParameter* ScalarSampleParameter =
			Cast<UMaterialExpressionScalarParameter>(Expressions(ExpressionIndex));

		if(ScalarSampleParameter)
		{
			OutParameterNames.AddItem(ScalarSampleParameter->ParameterName);
		}
	}
}

/**
 * @param	OutParameterNames		Storage array for the parameter names we are returning.
 *
 * @return	Returns a array of texture parameter names used in this material.
 */
void UMaterial::GetAllTextureParameterNames(TArray<FName> &OutParameterNames)
{
	OutParameterNames.Empty();
	for(INT ExpressionIndex = 0;ExpressionIndex < Expressions.Num();ExpressionIndex++)
	{
		UMaterialExpressionTextureSampleParameter* TextureSampleParameter =
			Cast<UMaterialExpressionTextureSampleParameter>(Expressions(ExpressionIndex));

		if(TextureSampleParameter)
		{
			OutParameterNames.AddItem(TextureSampleParameter->ParameterName);
		}
	}
}

/**
 * Called before serialization on save to propagate referenced textures. This is not done
 * during content cooking as the material expressions used to retrieve this information will
 * already have been dissociated via RemoveExpressions
 */
void UMaterial::PreSave()
{
	Super::PreSave();

	if( !IsTemplate() )
	{
		// Ensure that the ReferencedTextures array is up to date.
		ReferencedTextures.Empty();
		GetTextures(ReferencedTextures);
	}
}

void UMaterial::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if(!MaterialResource)
	{
		if(!IsTemplate())
		{
			// Construct the material resource.
			MaterialResource = AllocateResource();
		}
	}

	if(MaterialResource)
	{
		if(Ar.Ver() >= VER_RENDERING_REFACTOR)
		{
			if (Ar.IsLoading() == TRUE)
			{
				UBOOL bSerialize = TRUE;
				if (Ar.Ver() < VER_DECAL_REFACTOR)
				{
					if (IsA(UDecalMaterial::StaticClass()) == TRUE)
					{
						bSerialize = FALSE;
					}
				}
				
				if (bSerialize == TRUE)
				{
					// Serialize the material resource.
					MaterialResource->Serialize(Ar);
				}
			}
			else
			{
				// Serialize the material resource.
				MaterialResource->Serialize(Ar);
			}
		}
		else
		{
			// Copy over the legacy persistent ID to the FMaterialResource.
			MaterialResource->SetId(PersistentIds[0]);
		}
	}

	if (Ar.IsLoading() && (Ar.Ver() < VER_RENDERING_REFACTOR))
	{
		// check for distortion in material 
		bUsesDistortion = FALSE;
		// can only have distortion with translucent blend modes
		if(IsTranslucentBlendMode((EBlendMode)BlendMode))
		{
			// check for a distortion value
			if( Distortion.Expression ||
				(Distortion.UseConstant && !Distortion.Constant.IsNearlyZero()) )
			{
				bUsesDistortion = TRUE;
			}
		}
	}	

	if ( Ar.IsLoading() && Ar.Ver() < VER_MATERIAL_ISMASKED_FLAG )
	{
		// Check if the material is masked and uses a custom opacity (that's not 1.0f).
		bIsMasked = EBlendMode(BlendMode) == BLEND_Masked && (OpacityMask.Expression || (OpacityMask.UseConstant && OpacityMask.Constant<0.999f));
	}

	if( Ar.IsLoading() && Ar.Ver() < VER_MATERIAL_USES_SCENECOLOR_FLAG )
	{
		// check for scene color usage
		bUsesSceneColor = FALSE;		
		for( INT ExpressionIdx=0; ExpressionIdx < Expressions.Num(); ExpressionIdx++ )
		{
			UMaterialExpression * Expr = Expressions(ExpressionIdx);
			UMaterialExpressionSceneTexture* SceneTextureExpr = Cast<UMaterialExpressionSceneTexture>(Expr);
			UMaterialExpressionDestColor* DestColorExpr = Cast<UMaterialExpressionDestColor>(Expr);			
			if( SceneTextureExpr || DestColorExpr )
			{
				bUsesSceneColor = TRUE;
				break;
			}			
		}
	}
}

void UMaterial::PostLoad()
{
	Super::PostLoad();

	// Propogate the deprecated lighting model variables to the LightingModel enumeration.
	if(Unlit)
	{
		LightingModel = MLM_Unlit;
	}
	else if(NonDirectionalLighting)
	{
		LightingModel = MLM_NonDirectional;
	}
	Unlit = 0;
	NonDirectionalLighting = 0;

	if( GetLinkerVersion() < VER_RENDERING_REFACTOR )
	{
		// Populate expression array for old content.
		if(!Expressions.Num())
		{
			for(TObjectIterator<UMaterialExpression> It;It;++It)
			{
				if(It->GetOuter() == this)
				{
					Expressions.AddItem(*It);
				}
			}
		}
	}

	// PostLoad the material resource.
	if(MaterialResource)
	{
		MaterialResource->InitShaderMap();
	}
}

void UMaterial::PreEditChange(UProperty* PropertyThatChanged)
{
	Super::PreEditChange(PropertyThatChanged);

	// Flush all pending rendering commands.
	FlushRenderingCommands();
}

void UMaterial::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);

	// check for distortion in material 
	bUsesDistortion = FALSE;
	// can only have distortion with translucent blend modes
	if(IsTranslucentBlendMode((EBlendMode)BlendMode))
	{
		// check for a distortion value
		if( Distortion.Expression ||
			(Distortion.UseConstant && !Distortion.Constant.IsNearlyZero()) )
		{
			bUsesDistortion = TRUE;
		}
	}

	// check for scene color usage
	bUsesSceneColor = FALSE;
	for( INT ExpressionIdx=0; ExpressionIdx < Expressions.Num(); ExpressionIdx++ )
	{
		UMaterialExpression * Expr = Expressions(ExpressionIdx);
		UMaterialExpressionSceneTexture* SceneTextureExpr = Cast<UMaterialExpressionSceneTexture>(Expr);
		UMaterialExpressionDestColor* DestColorExpr = Cast<UMaterialExpressionDestColor>(Expr);
		if( SceneTextureExpr || DestColorExpr )
		{
			bUsesSceneColor = TRUE;
			break;
		}
	}

	// check for env cube usage
	bUsesEnvCube = FALSE;
	for( INT ExpressionIdx=0; ExpressionIdx < Expressions.Num(); ExpressionIdx++ )
	{
		UMaterialExpression * Expr = Expressions(ExpressionIdx);		
		UMaterialExpressionEnvCube* EnvCube = Cast<UMaterialExpressionEnvCube>(Expr);
		if( EnvCube )
		{
			bUsesEnvCube = TRUE;
			break;
		}
	}

	// Check if the material is masked and uses a custom opacity (that's not 1.0f).
	bIsMasked = EBlendMode(BlendMode) == BLEND_Masked && (OpacityMask.Expression || (OpacityMask.UseConstant && OpacityMask.Constant<0.999f));

	// Construct the material resource if it hasn't been already.
	if(!MaterialResource)
	{
		MaterialResource = AllocateResource();
	}

	if (MaterialResource)
	{
		// Compile the material.
		MaterialResource->CacheShaders();

		//<@ ava specific ; 2007. 1. 25 changmin
		// compute material flags using material resource
		bUsesSceneColor	= MaterialResource->GetUsesSceneColor();
		bUsesEnvCube	= MaterialResource->GetUsesEnvCube();
		if( IsTranslucentBlendMode((EBlendMode)BlendMode) )
		{
			if( MaterialResource->GetUsesDistortion()
			|| Distortion.UseConstant && !Distortion.Constant.IsNearlyZero( ) )
			{
				bUsesDistortion = TRUE;
			}
		}
		bIsMasked = EBlendMode(BlendMode) == BLEND_Masked && ( MaterialResource->GetUsesMask() || (OpacityMask.UseConstant && OpacityMask.Constant<0.999f) );
		//>@ ava
		// make sure that any staticmeshes, etc using this material will stop using the FMaterialResource of the original 
		// material, and will use the new FMaterialResource created when we make a new UMaterial inplace
		FGlobalComponentReattachContext RecreateComponents;
	}
}

void UMaterial::BeginDestroy()
{
	Super::BeginDestroy();
	if(MaterialResource)
	{
		MaterialResource->ReleaseFence.BeginFence();
	}
}

UBOOL UMaterial::IsReadyForFinishDestroy()
{
	return Super::IsReadyForFinishDestroy() && (!MaterialResource || !MaterialResource->ReleaseFence.GetNumPendingFences());
}

void UMaterial::FinishDestroy()
{
	if(MaterialResource)
	{
		check(!MaterialResource->ReleaseFence.GetNumPendingFences());
	}

	delete MaterialResource;
	delete DefaultMaterialInstances[FALSE];
	delete DefaultMaterialInstances[TRUE];
	Super::FinishDestroy();
}

/**
 * @return		Sum of the size of textures referenced by this material.
 */
INT UMaterial::GetResourceSize()
{
	INT ResourceSize = 0;
	TArray<UTexture*> ReferencedTextures;
	for ( INT ExpressionIndex= 0 ; ExpressionIndex < Expressions.Num() ; ++ExpressionIndex )
	{
		UMaterialExpressionTextureSample* TextureSample = Cast<UMaterialExpressionTextureSample>( Expressions(ExpressionIndex) );
		if ( TextureSample && TextureSample->Texture )
		{
			UTexture* Texture						= TextureSample->Texture;
			const UBOOL bTextureAlreadyConsidered	= ReferencedTextures.ContainsItem( Texture );
			if ( !bTextureAlreadyConsidered )
			{
				ReferencedTextures.AddItem( Texture );
				ResourceSize += Texture->GetResourceSize();
			}
		}
	}
	return ResourceSize;
}

/**
 * Used by various commandlets to purge Editor only data from the object.
 */
void UMaterial::StripData(UE3::EPlatformType TargetPlatform)
{
	Super::StripData(TargetPlatform); 
}

/**
 * Null any material expression references for this material
 */
void UMaterial::RemoveExpressions()
{
	if( MaterialResource )
	{
		MaterialResource->RemoveExpressions();
	}

	// Remove all non-parameter expressions from the material's expressions array.
	for(INT ExpressionIndex = 0;ExpressionIndex < Expressions.Num();ExpressionIndex++)
	{
		UMaterialExpression* Expression = Expressions(ExpressionIndex);
		
		// Skip the expression if it is a parameter expression
		if(Expression)
		{
			if(Expression->IsA(UMaterialExpressionScalarParameter::StaticClass()))
			{
				continue;
			}
			if(Expression->IsA(UMaterialExpressionVectorParameter::StaticClass()))
			{
				continue;
			}
			if(Expression->IsA(UMaterialExpressionTextureSampleParameter::StaticClass()))
			{
				continue;
			}
		}

		// Otherwise, remove the expression.
		Expressions.Remove(ExpressionIndex--,1);
	}
	Expressions.Shrink();

	DiffuseColor.Expression = NULL;
	SpecularColor.Expression = NULL;
	SpecularPower.Expression = NULL;
	Normal.Expression = NULL;
	EmissiveColor.Expression = NULL;
	Opacity.Expression = NULL;
	OpacityMask.Expression = NULL;
	Distortion.Expression = NULL;
	CustomLighting.Expression = NULL;
	TwoSidedLightingMask.Expression = NULL;
	TwoSidedLightingColor.Expression = NULL;
	AmbientMask.Expression = NULL;
}


