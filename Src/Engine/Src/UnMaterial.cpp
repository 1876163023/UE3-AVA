/*=============================================================================
	UnMaterial.cpp: Shader implementation.
	Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineMaterialClasses.h"
#include "EnginePhysicsClasses.h"

IMPLEMENT_CLASS(UMaterialInstance);
IMPLEMENT_CLASS(UMaterial);
IMPLEMENT_CLASS(UMaterialInstanceConstant);
IMPLEMENT_CLASS(UMaterialExpression);
IMPLEMENT_CLASS(UMaterialExpressionTextureSample);
IMPLEMENT_CLASS(UMaterialExpressionMeshEmitterVertexColor);
IMPLEMENT_CLASS(UMaterialExpressionMultiply);
IMPLEMENT_CLASS(UMaterialExpressionDivide);
IMPLEMENT_CLASS(UMaterialExpressionSubtract);
IMPLEMENT_CLASS(UMaterialExpressionLinearInterpolate);
IMPLEMENT_CLASS(UMaterialExpressionAdd);
IMPLEMENT_CLASS(UMaterialExpressionTextureCoordinate);
IMPLEMENT_CLASS(UMaterialExpressionComponentMask);
IMPLEMENT_CLASS(UMaterialExpressionDotProduct);
IMPLEMENT_CLASS(UMaterialExpressionCrossProduct);
IMPLEMENT_CLASS(UMaterialExpressionClamp);
IMPLEMENT_CLASS(UMaterialExpressionConstant);
IMPLEMENT_CLASS(UMaterialExpressionConstant2Vector);
IMPLEMENT_CLASS(UMaterialExpressionConstant3Vector);
IMPLEMENT_CLASS(UMaterialExpressionConstant4Vector);
IMPLEMENT_CLASS(UMaterialExpressionTime);
IMPLEMENT_CLASS(UMaterialExpressionCameraVector);
IMPLEMENT_CLASS(UMaterialExpressionReflectionVector);
IMPLEMENT_CLASS(UMaterialExpressionPanner);
IMPLEMENT_CLASS(UMaterialExpressionRotator);
IMPLEMENT_CLASS(UMaterialExpressionSine);
IMPLEMENT_CLASS(UMaterialExpressionCosine);
IMPLEMENT_CLASS(UMaterialExpressionBumpOffset);
IMPLEMENT_CLASS(UMaterialExpressionAppendVector);
IMPLEMENT_CLASS(UMaterialExpressionFloor);
IMPLEMENT_CLASS(UMaterialExpressionCeil);
IMPLEMENT_CLASS(UMaterialExpressionFrac);
IMPLEMENT_CLASS(UMaterialExpressionAbs);
IMPLEMENT_CLASS(UMaterialExpressionDepthBiasBlend);
IMPLEMENT_CLASS(UMaterialExpressionDepthBiasedBlend);
IMPLEMENT_CLASS(UMaterialExpressionDesaturation);
IMPLEMENT_CLASS(UMaterialExpressionVectorParameter);
IMPLEMENT_CLASS(UMaterialExpressionScalarParameter);
IMPLEMENT_CLASS(UMaterialExpressionNormalize);
IMPLEMENT_CLASS(UMaterialExpressionVertexColor);
IMPLEMENT_CLASS(UMaterialExpressionParticleSubUV);
IMPLEMENT_CLASS(UMaterialExpressionMeshSubUV);
IMPLEMENT_CLASS(UMaterialExpressionTextureSampleParameter);
IMPLEMENT_CLASS(UMaterialExpressionTextureSampleParameter2D);
IMPLEMENT_CLASS(UMaterialExpressionTextureSampleParameter3D);
IMPLEMENT_CLASS(UMaterialExpressionTextureSampleParameterCube);
IMPLEMENT_CLASS(UMaterialExpressionTextureSampleParameterMovie);
IMPLEMENT_CLASS(UMaterialExpressionFlipBookSample);
IMPLEMENT_CLASS(UMaterialExpressionLightVector);
IMPLEMENT_CLASS(UMaterialExpressionScreenPosition);
IMPLEMENT_CLASS(UMaterialExpressionPixelDepth);
IMPLEMENT_CLASS(UMaterialExpressionDestColor);
IMPLEMENT_CLASS(UMaterialExpressionDestDepth);
IMPLEMENT_CLASS(UMaterialExpressionPower);
IMPLEMENT_CLASS(UMaterialExpressionSquareRoot);
IMPLEMENT_CLASS(UMaterialExpressionIf);
IMPLEMENT_CLASS(UMaterialExpressionOneMinus);
IMPLEMENT_CLASS(UMaterialExpressionSceneTexture);
IMPLEMENT_CLASS(UMaterialExpressionSceneDepth);
IMPLEMENT_CLASS(UMaterialExpressionTransform);
IMPLEMENT_CLASS(UMaterialExpressionComment);
IMPLEMENT_CLASS(UMaterialExpressionCompound);
IMPLEMENT_CLASS(UMaterialExpressionFresnel);
IMPLEMENT_CLASS(AMaterialInstanceActor);

#define GET_EXPRESSIONS( ExpressionInput )										\
	if( ExpressionInput.Expression && !ExpressionInput.Expression->Compiling )	\
	{																			\
		ExpressionInput.Expression->Compiling = 1;								\
		ExpressionInput.Expression->GetExpressions( Expressions );				\
		ExpressionInput.Expression->Compiling = 0;								\
	}																	

#define REMOVE_REFERENCE_TO( ExpressionInput, ToBeRemovedExpression )			\
	if( ExpressionInput.Expression == ToBeRemovedExpression )					\
	{																			\
		ExpressionInput.Expression = NULL;										\
	}


INT UMaterialInstance::GetWidth() const
{
	return ME_PREV_THUMBNAIL_SZ+(ME_STD_BORDER*2);
}

INT UMaterialInstance::GetHeight() const
{
	return ME_PREV_THUMBNAIL_SZ+ME_CAPTION_HEIGHT+(ME_STD_BORDER*2);
}

//
//	UMaterialInstance::execGetMaterial
//

void UMaterialInstance::execGetMaterial(FFrame& Stack,RESULT_DECL)
{
	P_FINISH;
	*(UMaterial**) Result = GetMaterial();
}


void UMaterialInstance::execGetPhysicalMaterial(FFrame& Stack,RESULT_DECL)
{
	P_FINISH;
	*(UPhysicalMaterial**) Result = GetPhysicalMaterial();
}


/**
 * Protects the members of a UMaterialInstanceConstant from re-entrance.
 */
struct FMICReentranceGuard
{
	UMaterialInstanceConstant*	Material;
	FMICReentranceGuard(UMaterialInstanceConstant* InMaterial):
		Material(InMaterial)
	{
		check(!Material->ReentrantFlag);
		Material->ReentrantFlag = 1;
	}
	~FMICReentranceGuard()
	{
		Material->ReentrantFlag = 0;
	}
};

struct FConstantMaterialInstance: FMaterialInstance
{
	UMaterialInstanceConstant* Instance;
	UMaterialInstance* Parent;
	UBOOL Selected;

	// FMaterialInstance interface.
	virtual UBOOL GetVectorValue(const FName& ParameterName,FLinearColor* OutValue) const
	{
		FLinearColor* Value = Instance->VectorValueMap.Find(ParameterName);
		if(Value)
		{
			*OutValue = *Value;
			return 1;
		}
		else if(Instance->ReentrantFlag)
		{
			return 0;
		}
		else
		{
			FMICReentranceGuard Guard(Instance);
			return Parent->GetInstanceInterface(Selected)->GetVectorValue(ParameterName,OutValue);
		}
	}
	virtual UBOOL GetScalarValue(const FName& ParameterName,FLOAT* OutValue) const
	{
		FLOAT* Value = Instance->ScalarValueMap.Find(ParameterName);
		if(Value)
		{
			*OutValue = *Value;
			return 1;
		}
		else if(Instance->ReentrantFlag)
		{
			return 0;
		}
		else
		{
			FMICReentranceGuard Guard(Instance);
			return Parent->GetInstanceInterface(Selected)->GetScalarValue(ParameterName,OutValue);
		}
	}
	virtual UBOOL GetTextureValue(const FName& ParameterName,FTextureBase** OutValue) const
	{
		UTexture** Value = Instance->TextureValueMap.Find(ParameterName);
		if(Value && *Value)
		{
			*OutValue = (*Value)->GetTexture();
			return 1;
		}
		else if(Instance->ReentrantFlag)
		{
			return 0;
		}
		else
		{
			FMICReentranceGuard Guard(Instance);
			return Parent->GetInstanceInterface(Selected)->GetTextureValue(ParameterName,OutValue);
		}
	}

	void Update()
	{
		if(Instance->Parent)
		{
			Parent = Instance->Parent;
		}
		else
		{
			if(GEngine)
			{
				Parent = GEngine->DefaultMaterial;
			}
			else
			{
				// A material instance was loaded with an invalid GEngine.
				// This is probably because loading the default properties for the GEngine class lead to a material instance being loaded before GEngine has been created.
				// In this case, we'll just pull the default material config value straight from the INI.
				Parent = LoadObject<UMaterialInstance>(NULL,TEXT("engine-ini:Engine.Engine.DefaultMaterialName"),NULL,LOAD_None,NULL);
			}
		}
		Material = Parent->GetInstanceInterface(Selected)->Material;
	}

	FConstantMaterialInstance(UMaterialInstanceConstant* InInstance,UBOOL InSelected):
		Instance(InInstance),
		Selected(InSelected)
	{
		Update();
	}
};

UMaterialInstanceConstant::UMaterialInstanceConstant()
{
	// GIsUCCMake is not set when the class is initialized
	if(!GIsUCCMake && !HasAnyFlags(RF_ClassDefaultObject))
	{
		MaterialInstances[0] = new FConstantMaterialInstance(this,0);
		if(GIsEditor)
		{
			MaterialInstances[1] = new FConstantMaterialInstance(this,1);
		}
	}
}


UMaterial* UMaterialInstanceConstant::GetMaterial()
{
	if(ReentrantFlag)
	{
		return GEngine->DefaultMaterial;
	}

	FMICReentranceGuard	Guard(this);
	if(Parent)
	{
		return Parent->GetMaterial();
	}
	else
	{
		return GEngine->DefaultMaterial;
	}
}


FMaterialInstance* UMaterialInstanceConstant::GetInstanceInterface(UBOOL Selected) const
{
	check(!Selected || GIsEditor);
	return MaterialInstances[Selected];
}


UPhysicalMaterial* UMaterialInstanceConstant::GetPhysicalMaterial() const
{
	if(PhysMaterial)
	{
		return PhysMaterial;
	}
	else if(Parent)
	{
		// If no physical material has been associated with this instance, simply use the parent's physical material.
		return Parent->GetPhysicalMaterial();
	}
	else
	{
		return NULL;
	}
}

void UMaterialInstanceConstant::SetVectorParameterValue(FName ParameterName, FLinearColor Value)
{
	VectorValueMap.Set(ParameterName,Value);

	for (INT ValueIndex = 0;ValueIndex < VectorParameterValues.Num();ValueIndex++)
	{
		if (VectorParameterValues(ValueIndex).ParameterName == ParameterName)
		{
			VectorParameterValues(ValueIndex).ParameterValue = Value;
			return;
		}
	}

	FVectorParameterValue*	NewParameterValue = new(VectorParameterValues) FVectorParameterValue;
	NewParameterValue->ParameterName = ParameterName;
	NewParameterValue->ParameterValue = Value;
}

void UMaterialInstanceConstant::SetScalarParameterValue(FName ParameterName, float Value)
{
	ScalarValueMap.Set(ParameterName,Value);

	for (INT ValueIndex = 0;ValueIndex < ScalarParameterValues.Num();ValueIndex++)
	{
		if (ScalarParameterValues(ValueIndex).ParameterName == ParameterName)
		{
			ScalarParameterValues(ValueIndex).ParameterValue = Value;
			return;
		}
	}

	FScalarParameterValue*	NewParameterValue = new(ScalarParameterValues) FScalarParameterValue;
	NewParameterValue->ParameterName = ParameterName;
	NewParameterValue->ParameterValue = Value;
}

//
//  UMaterialInstanceConstant::SetTextureParameterValue
//
void UMaterialInstanceConstant::SetTextureParameterValue(FName ParameterName, UTexture* Value)
{
	// verify parent material
	if( !Parent || Parent == GEngine->DefaultMaterial )
	{
		debugf( NAME_Warning, TEXT("Parent material not set for %s"), *GetFullName() );		
	}
	
	// verify the texture and parameter usage
	UMaterial* ParentMat = Parent ? Parent->GetMaterial() : NULL;
	if( ParentMat )
	{
		UBOOL bFoundParam = FALSE;
		// get all linked expressions
		TArray<const UMaterialExpression*> MatExpressions;
        ParentMat->GetExpressions( MatExpressions );
		// find the matching parameter by name
		for( INT ExpIdx=0; ExpIdx < MatExpressions.Num(); ExpIdx++ )
		{
			// only care about texture sampler outputs
			UMaterialExpressionTextureSampleParameter* TexParam = Cast<UMaterialExpressionTextureSampleParameter>((UObject*)MatExpressions(ExpIdx));
			// check for a match
			if( TexParam &&
				TexParam->ParameterName == ParameterName )
			{
				// see if the input texture value is valid
				if( !TexParam->TextureIsValid( Value ) )
				{
					debugf( NAME_Warning, TEXT("%s: texture parameter %s is a %s and %s"), 
						Parent->GetName(), *ParameterName, *TexParam->GetCaption(), TexParam->GetRequirements() );
				}
				bFoundParam = TRUE;
				break;
			}
		}
		// no match found
		if( !bFoundParam )
		{
			debugf( NAME_Warning, TEXT("%s: has no texture parameter called %s"), Parent->GetName(), *ParameterName );
		}
	}

    TextureValueMap.Set(ParameterName,Value);

	for(INT ValueIndex = 0;ValueIndex < TextureParameterValues.Num();ValueIndex++)
	{
		if(TextureParameterValues(ValueIndex).ParameterName == ParameterName)
		{
			TextureParameterValues(ValueIndex).ParameterValue = Value;
			return;
		}
	}

	FTextureParameterValue*	NewParameterValue = new(TextureParameterValues) FTextureParameterValue;
	NewParameterValue->ParameterName = ParameterName;
	NewParameterValue->ParameterValue = Value;
}

/** Removes all parameter values */
void UMaterialInstanceConstant::ClearParameterValues()
{
	VectorValueMap.Empty();
	ScalarValueMap.Empty();
	TextureValueMap.Empty();
	VectorParameterValues.Empty();
	ScalarParameterValues.Empty();
	TextureParameterValues.Empty();
}

UBOOL UMaterialInstanceConstant::GetVectorParameterValue(FName ParameterName, FLinearColor& OutValue)
{
	FLinearColor* Value = VectorValueMap.Find(ParameterName);
	if(Value)
	{
		OutValue = *Value;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

UBOOL UMaterialInstanceConstant::GetScalarParameterValue(FName ParameterName, FLOAT& OutValue)
{
	FLOAT* Value = ScalarValueMap.Find(ParameterName);
	if(Value)
	{
		OutValue = *Value;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

UBOOL UMaterialInstanceConstant::GetTextureParameterValue(FName ParameterName, UTexture*& OutValue)
{
	UTexture** Value = TextureValueMap.Find(ParameterName);
	if(Value && *Value)
	{
		OutValue = *Value;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


void UMaterialInstanceConstant::SetParent(UMaterialInstance* NewParent)
{
	Parent = NewParent;
	if(!GIsUCCMake)
	{
		for(INT Selected = 0;Selected < (GIsEditor ? 2 : 1);Selected++)
		{
			((FConstantMaterialInstance*)MaterialInstances[Selected])->Update();
		}
	}
}

void UMaterialInstanceConstant::PostLoad()
{
	// Ensure that the instance's parent is PostLoaded before the instance.
	if(Parent)
	{
		Parent->ConditionalPostLoad();
	}

	Super::PostLoad();
	VectorValueMap.Empty();
	ScalarValueMap.Empty();
	TextureValueMap.Empty();
	for(INT ValueIndex = 0;ValueIndex < VectorParameterValues.Num();ValueIndex++)
	{
		VectorValueMap.Set(VectorParameterValues(ValueIndex).ParameterName,VectorParameterValues(ValueIndex).ParameterValue);
	}
	for(INT ValueIndex = 0;ValueIndex < ScalarParameterValues.Num();ValueIndex++)
	{
		ScalarValueMap.Set(ScalarParameterValues(ValueIndex).ParameterName,ScalarParameterValues(ValueIndex).ParameterValue);
	}
	for (INT ValueIndex = 0; ValueIndex < TextureParameterValues.Num(); ValueIndex++)
	{
		TextureValueMap.Set(TextureParameterValues(ValueIndex).ParameterName, TextureParameterValues(ValueIndex).ParameterValue);
	}
	if(!GIsUCCMake)
	{
		for(INT Selected = 0;Selected < (GIsEditor ? 2 : 1);Selected++)
		{
			((FConstantMaterialInstance*)MaterialInstances[Selected])->Update();
		}
	}
}

void UMaterialInstanceConstant::PostEditChange(UProperty* PropertyThatChanged)
{
	VectorValueMap.Empty();
	ScalarValueMap.Empty();
	TextureValueMap.Empty();
	for(INT ValueIndex = 0;ValueIndex < VectorParameterValues.Num();ValueIndex++)
	{
		VectorValueMap.Set(VectorParameterValues(ValueIndex).ParameterName,VectorParameterValues(ValueIndex).ParameterValue);
	}
	for(INT ValueIndex = 0;ValueIndex < ScalarParameterValues.Num();ValueIndex++)
	{
		ScalarValueMap.Set(ScalarParameterValues(ValueIndex).ParameterName,ScalarParameterValues(ValueIndex).ParameterValue);
	}
	for (INT ValueIndex = 0; ValueIndex < TextureParameterValues.Num(); ValueIndex++)
	{
		TextureValueMap.Set(TextureParameterValues(ValueIndex).ParameterName, TextureParameterValues(ValueIndex).ParameterValue);
	}
	if(!GIsUCCMake)
	{
		for(INT Selected = 0;Selected < (GIsEditor ? 2 : 1);Selected++)
		{
			((FConstantMaterialInstance*)MaterialInstances[Selected])->Update();
		}
	}
	Super::PostEditChange(PropertyThatChanged);
}

void UMaterialInstanceConstant::Destroy()
{
	if(!GIsUCCMake&&!HasAnyFlags(RF_ClassDefaultObject))
	{
		delete MaterialInstances[0];
		if(GIsEditor)
		{
			delete MaterialInstances[1];
		}
	}
	Super::Destroy();
}

/**
* Resumes playback of the movie or flipbook texture
* given by the ParamName
*/
void UMaterialInstanceConstant::Play( FName ParamName )
{
	UTexture** pTexture = TextureValueMap.Find( ParamName );
	if( pTexture && *pTexture )
	{
		UTextureMovie* pMovieTex = Cast<UTextureMovie>( *pTexture );
		UTextureFlipBook* pFlipBookTex = Cast<UTextureFlipBook>( *pTexture );
		if( pMovieTex )
		{
			pMovieTex->Play();
		}
		else if( pFlipBookTex )
		{
			pFlipBookTex->Play();			
		}
	}
}

/**
* Pauses playback of the movie or flipbook texture
* given by the ParamName
*/
void UMaterialInstanceConstant::Pause( FName ParamName )
{
	UTexture** pTexture = TextureValueMap.Find( ParamName );
	if( pTexture && *pTexture )
	{
		UTextureMovie* pMovieTex = Cast<UTextureMovie>( *pTexture );
		UTextureFlipBook* pFlipBookTex = Cast<UTextureFlipBook>( *pTexture );
		if( pMovieTex )
		{
			pMovieTex->Pause();
		}
		else if( pFlipBookTex )
		{
			pFlipBookTex->Pause();			
		}
	}
}

/**
* Stops playback of the movie or flipbook texture
* given by the ParamName
*/
void UMaterialInstanceConstant::Stop( FName ParamName )
{
	UTexture** pTexture = TextureValueMap.Find( ParamName );
	if( pTexture && *pTexture )
	{
		UTextureMovie* pMovieTex = Cast<UTextureMovie>( *pTexture );
		UTextureFlipBook* pFlipBookTex = Cast<UTextureFlipBook>( *pTexture );
		if( pMovieTex )
		{
			pMovieTex->Stop();
		}
		else if( pFlipBookTex )
		{
			pFlipBookTex->Stop();			
		}
	}
}

//
//	FMaterialExpressionGuard - Traps material compiler errors and re-entrancy in the context of an expression.
//

struct FMaterialExpressionGuard: FMaterialCompilerGuard
{
	UMaterialExpression*	Expression;
	FMaterialCompiler*		Compiler;

	// Constructor.

	FMaterialExpressionGuard(UMaterialExpression* InExpression,FMaterialCompiler* InCompiler):
		Expression(InExpression),
		Compiler(InCompiler)
	{
		if(Expression->Compiling)
			Compiler->Errorf(TEXT("Cyclic material expression detected."));
		Expression->Errors.Empty();
		Expression->Compiling = 1;
		Compiler->EnterGuard(this);
	}

	~FMaterialExpressionGuard()
	{
		Compiler->ExitGuard();
		Expression->Compiling = 0;
	}

	// FMaterialCompilerGuard interface.

	virtual void Error(const TCHAR* Text)
	{
		new(Expression->Errors) FString(FString::Printf(TEXT("%s: %s"),Expression->GetName(),Text));
	}
};

//
//	FExpressionInput::Compile
//

INT FExpressionInput::Compile(FMaterialCompiler* Compiler)
{
	if(Expression)
	{
		FMaterialExpressionGuard	Guard(Expression,Compiler);

		if(Mask)
			return Compiler->ComponentMask(Expression->Compile(Compiler),MaskR,MaskG,MaskB,MaskA);
		else
			return Expression->Compile(Compiler);
	}
	else
		return INDEX_NONE;
}

//
//	FColorMaterialInput::Compile
//

INT FColorMaterialInput::Compile(FMaterialCompiler* Compiler,const FColor& Default)
{
	if(UseConstant)
	{
		FLinearColor	LinearColor(Constant);
		return Compiler->Constant3(LinearColor.R,LinearColor.G,LinearColor.B);
	}
	else if(Expression)
		return FExpressionInput::Compile(Compiler);
	else
	{
		FLinearColor	LinearColor(Default);
		return Compiler->Constant3(LinearColor.R,LinearColor.G,LinearColor.B);
	}
}

//
//	FScalarMaterialInput::Compile
//

INT FScalarMaterialInput::Compile(FMaterialCompiler* Compiler,FLOAT Default)
{
	if(UseConstant)
		return Compiler->Constant(Constant);
	else if(Expression)
		return FExpressionInput::Compile(Compiler);
	else
		return Compiler->Constant(Default);
}

//
//	FVectorMaterialInput::Compile
//

INT FVectorMaterialInput::Compile(FMaterialCompiler* Compiler,const FVector& Default)
{
	if(UseConstant)
		return Compiler->Constant3(Constant.X,Constant.Y,Constant.Z);
	else if(Expression)
		return FExpressionInput::Compile(Compiler);
	else
		return Compiler->Constant3(Default.X,Default.Y,Default.Z);
}

//
//	FVector2MaterialInput::Compile
//

INT FVector2MaterialInput::Compile(FMaterialCompiler* Compiler,const FVector2D& Default)
{
	if(UseConstant)
		return Compiler->Constant2(Constant.X,Constant.Y);
	else if(Expression)
		return FExpressionInput::Compile(Compiler);
	else
		return Compiler->Constant2(Default.X,Default.Y);
}

struct FMaterialResource: FMaterial
{
	UMaterial* Material;

	void Update(UBOOL bUpdateResource=TRUE)
	{
		SourceSHM = Material->SHM;
		TwoSided = Material->TwoSided;
		Wireframe = Material->Wireframe;
		BlendMode = Material->BlendMode;
		LightingModel = Material->LightingModel;
		OpacityMaskClipValue = Material->OpacityMaskClipValue;

		// check for distortion in material 
		bHasDistortion = FALSE;
		// can only have distortion with translucent blend modes
		if( IsTranslucent() )
		{
			// check for a distortion value
			if( Material->Distortion.Expression ||
				(Material->Distortion.UseConstant && !Material->Distortion.Constant.IsNearlyZero()) )
			{
				bHasDistortion = TRUE;
			}
		}

		// check for scene color usage in material 
		bUsesSceneColor = FALSE;
		TArray<const UMaterialExpression*> MaterialExpressions;
		Material->GetExpressions( MaterialExpressions );
		for( INT ExpressionIndex=0; ExpressionIndex < MaterialExpressions.Num(); ExpressionIndex++ )
		{
			// check for any scene texture or dest color expressions
			if( MaterialExpressions(ExpressionIndex)->IsA(UMaterialExpressionSceneTexture::StaticClass()) ||
				MaterialExpressions(ExpressionIndex)->IsA(UMaterialExpressionDestColor::StaticClass()) )
			{
				bUsesSceneColor = TRUE;
				break;
			}
		}
		
		if (bUpdateResource)
		{
			GResourceManager->UpdateResource(this);
		}
	}

	FMaterialResource(UMaterial* InMaterial):
		Material(InMaterial)
	{
		Update(FALSE);
	}

	virtual INT CompileProperty(EMaterialProperty Property,FMaterialCompiler* Compiler)
	{
		switch(Property)
		{
		case MP_EmissiveColor:
#if !CONSOLE
			if( !GIsUCC )
			{
				return Compiler->Add(Compiler->ForceCast(Material->EmissiveColor.Compile(Compiler,FColor(0,0,0)),MCT_Float3),Compiler->GetSelectionColorIndex());
			}
			else
#endif
			{
				return Material->EmissiveColor.Compile(Compiler,FColor(0,0,0));
			}		
		case MP_Opacity: return Material->Opacity.Compile(Compiler,1.0f);
		case MP_OpacityMask: return Material->OpacityMask.Compile(Compiler,1.0f);
		case MP_Distortion: return Material->Distortion.Compile(Compiler,FVector2D(0,0));
		case MP_TwoSidedLightingMask: return Compiler->Mul(Compiler->ForceCast(Material->TwoSidedLightingMask.Compile(Compiler,0.0f),MCT_Float),Material->TwoSidedLightingColor.Compile(Compiler,FColor(255,255,255)));
		case MP_DiffuseColor:
#if !CONSOLE
			if( !GIsUCC )
			{
				return Compiler->Mul(Compiler->ForceCast(Material->DiffuseColor.Compile(Compiler,FColor(128,128,128)),MCT_Float3),Compiler->Sub(Compiler->Constant(1.0f),Compiler->GetSelectionColorIndex()));
			}
			else
#endif
			{
				return Material->DiffuseColor.Compile(Compiler,FColor(128,128,128));
			}
		case MP_SpecularColor: return Material->SpecularColor.Compile(Compiler,FColor(128,128,128));
		case MP_SpecularPower: return Material->SpecularPower.Compile(Compiler,15.0f);
		case MP_Normal: return Material->Normal.Compile(Compiler,FVector(0,0,1));
		case MP_SHM:
			if(Material->SHM)
			{
				const FPlane&	Scale = Material->SHM->CoefficientTextures(0).CoefficientScale,
								Bias = Material->SHM->CoefficientTextures(0).CoefficientBias;

				INT TextureIndex;
				INT TextureCodeIndex = Compiler->Texture(&Material->SHM->CoefficientTextures(0), TextureIndex);
				return Compiler->Add(
						Compiler->Mul(
							Compiler->TextureSample(
								TextureCodeIndex,
								Compiler->TextureCoordinate(0)
								),
							Compiler->Constant4(Scale.X,Scale.Y,Scale.Z,Scale.W)
							),
						Compiler->Constant4(Bias.X,Bias.Y,Bias.Z,Bias.W)
						);
			}
			else
				return Compiler->Constant(0);
		case MP_CustomLighting: return Material->CustomLighting.Compile(Compiler,FColor(0,0,0));
		case MP_AmbientMask : return Material->AmbientMask.Compile(Compiler,1.0f);
		default:
			return INDEX_NONE;
		};
	}

	// FResource interface.

	virtual FString DescribeResource()
	{
		return Material->GetPathName();
	}

	virtual FGuid GetPersistentId()
	{
		return Material->PersistentIds[(GIsEditor && !GIsUCC) ? 1 : 0];
	}
};

struct FDefaultMaterialInstance: FMaterialInstance
{
	TMap<FName,FLinearColor> VectorValueMap;
	TMap<FName,FLOAT> ScalarValueMap;

	// FMaterialInstance interface.
	virtual UBOOL GetVectorValue(const FName& ParameterName,FLinearColor* OutValue) const
	{
		const FLinearColor* Value = VectorValueMap.Find(ParameterName);
		if(Value)
		{
			*OutValue = *Value;
			return 1;
		}
		return 0;
	}
	virtual UBOOL GetScalarValue(const FName& ParameterName,FLOAT* OutValue) const
	{
		const FLOAT* Value = ScalarValueMap.Find(ParameterName);
		if(Value)
		{
			*OutValue = *Value;
			return 1;
		}
		return 0;
	}

	void Update(UMaterial* SourceMaterial,UBOOL Selected)
	{
		static FLinearColor SelectionColor(10.0f/255.0f,5.0f/255.0f,60.0f/255.0f,1);

		Material = SourceMaterial->MaterialResource;
		VectorValueMap.Empty();
		ScalarValueMap.Empty();

#if !CONSOLE
		if( Selected )
		{
			VectorValueMap.Set(NAME_SelectionColor,SelectionColor);
		}
		else
		{
			VectorValueMap.Set(NAME_SelectionColor,FLinearColor::Black);
		}
#endif

		TArray<const UMaterialExpression*> MaterialExpressions;
		SourceMaterial->GetExpressions( MaterialExpressions );

		for( INT ExpressionIndex=0; ExpressionIndex<MaterialExpressions.Num(); ExpressionIndex++ )
		{
			const UMaterialExpressionVectorParameter* VectorParameter = ConstCast<UMaterialExpressionVectorParameter>(MaterialExpressions(ExpressionIndex));
			const UMaterialExpressionScalarParameter* ScalarParameter = ConstCast<UMaterialExpressionScalarParameter>(MaterialExpressions(ExpressionIndex));

			if( VectorParameter )
			{
				VectorValueMap.Set( VectorParameter->ParameterName, VectorParameter->DefaultValue );
			}
			else if( ScalarParameter )
			{
				ScalarValueMap.Set( ScalarParameter->ParameterName, ScalarParameter->DefaultValue );
			}
		}
	}

	FDefaultMaterialInstance(UMaterial* SourceMaterial,UBOOL Selected)
	{
		Update(SourceMaterial,Selected);
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	UMaterial
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * UMaterial's constructor calls through to InitMaterialResources, which calls AllocateMaterialResource
 * to initialize the MaterialResource member.  However, virtual member functions cannot be called in
 * a straightforward manner from a baseclass constructor because the vtable of derived classes has not
 * yet been initialized.  The solution is for UMaterial derived classes to call up to their parent's
 * constructor passing TRUE, indicating to UMaterial that it should delay allocating resources.
 * Then, child classes call through to InitMaterialResources, which will call the correct AllocateMaterialResource
 * as vtables will have been initialized at that point.  In other words:
 *
 * class UMyMaterial : public UMaterial
 * {
 * public:
 *		UMyMaterial(UBOOL bDelayMaterialResourceAllocation)
 *          : UMaterial( TRUE )
 *      {
 *	        InitMaterialResources( bDelayMaterialResourceAllocation );
 *      }
 * }
 */
UMaterial::UMaterial(UBOOL bDelayMaterialResourceAllocation)
{
	InitMaterialResources( bDelayMaterialResourceAllocation );
}

void UMaterial::InitMaterialResources(UBOOL bDelayMaterialResourceAllocation)
{
	if ( !bDelayMaterialResourceAllocation && !HasAnyFlags(RF_ClassDefaultObject) )
	{
		PersistentIds[0] = appCreateGuid();
		PersistentIds[1] = appCreateGuid();
		// Ask derived class to allocate the resource.  This is called from the derived
		// class constructor and is valid because child classes tell their parents
		// to delay material resource allocation.
		MaterialResource = AllocateMaterialResource();
		DefaultMaterialInstances[0] = new FDefaultMaterialInstance(this,0);
		if(GIsEditor)
		{
			DefaultMaterialInstances[1] = new FDefaultMaterialInstance(this,1);
		}
	}
}

FMaterialPointer UMaterial::AllocateMaterialResource()
{
	return new FMaterialResource(this);
}

/**
 * Recursively gathers all UMaterialExpression objects used by this material.
 *
 * @param	Expressions		Reference to array of material expressions to add to
 */
void UMaterial::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	GET_EXPRESSIONS( DiffuseColor			);
	GET_EXPRESSIONS( SpecularColor			);
	GET_EXPRESSIONS( SpecularPower			);
	GET_EXPRESSIONS( Normal					);
	GET_EXPRESSIONS( EmissiveColor			);
	GET_EXPRESSIONS( Opacity				);
	GET_EXPRESSIONS( OpacityMask			);
	GET_EXPRESSIONS( Distortion				);
	GET_EXPRESSIONS( TwoSidedLightingMask	);
	GET_EXPRESSIONS( TwoSidedLightingColor	);
	GET_EXPRESSIONS( CustomLighting			);
	GET_EXPRESSIONS( AmbientMask			);
}

/**
 * Fills in the StreamingTextures array by using a dummy material compiler that parses the material
 * and collects textures that can be streamed.
 */
void UMaterial::CollectStreamingTextures()
{
	StreamingTextures.Empty();

	struct FTextureCollectingMaterialCompiler : public FMaterialCompiler
	{
		UMaterial* Material;

		// Contstructor
		FTextureCollectingMaterialCompiler( UMaterial* InMaterial )
		:	Material( InMaterial )
		{}

		// Gather textures.
		virtual INT Texture(FTextureBase* Texture, INT& TextureIndex) 
		{ 
			if (Texture->CanBeStreamed())
			{
				TextureIndex = Material->StreamingTextures.AddItem(Texture);
			}
			return 0; 
		}
		virtual INT TextureUnpackMin(INT TextureIndex)			{ return 0;	}
		virtual INT TextureUnpackMaxMinusMin(INT TextureIndex)	{ return 0; }
		virtual INT TextureOffsetParameter()					{ return 0; }
		virtual INT TextureScaleParameter()						{ return 0; }
		virtual INT FlipBookOffset(INT TextureIndex)			{ return 0; }
		virtual INT FlipBookScale(INT TextureIndex)				{ return 0; }

		// Not implemented (don't need to).
		virtual INT Error(const TCHAR* Text) 
		{ 
#if !defined(XBOX) && !defined(EXCEPTIONS_DISABLED)
			throw Text; 
#endif
			return 0; 
		}
		virtual void EnterGuard(FMaterialCompilerGuard* Guard) {}
		virtual void ExitGuard() {}
		virtual INT GetSelectionColorIndex() { return 0; }
		virtual EMaterialCodeType GetType(INT Code) { return MCT_Unknown; }
		virtual INT ForceCast(INT Code,EMaterialCodeType DestType) { return 0; }
		virtual INT VectorParameter(FName ParameterName) { return 0; }
		virtual INT ScalarParameter(FName ParameterName) { return 0; }
		virtual INT Constant(FLOAT X) { return 0; }
		virtual INT Constant2(FLOAT X,FLOAT Y) { return 0; }
		virtual INT Constant3(FLOAT X,FLOAT Y,FLOAT Z) { return 0; }
		virtual INT Constant4(FLOAT X,FLOAT Y,FLOAT Z,FLOAT W) { return 0; }
		virtual INT SceneTime() { return 0; }
		virtual INT Sine(INT X) { return 0; }
		virtual INT Cosine(INT X) { return 0; }
		virtual INT Floor(INT X) { return 0; }
		virtual INT Ceil(INT X) { return 0; }
		virtual INT Frac(INT X) { return 0; }
		virtual INT Abs(INT X) { return 0; }
		virtual INT ReflectionVector() { return 0; }
		virtual INT CameraVector() { return 0; }
		virtual INT LightVector() { return 0; }
		virtual INT ScreenPosition( UBOOL bScreenAlign ) { return 0; }
		virtual INT If(INT A,INT B,INT AGreaterThanB,INT AEqualsB,INT ALessThanB) { return 0; }
		virtual INT TextureCoordinate(UINT CoordinateIndex) { return 0; }
		virtual INT TextureSample(INT Texture,INT Coordinate) { return 0; }
		virtual INT TextureParameter(FTextureBase* DefaultTexture, FName ParameterName, INT& TextureIndex) 
		{ 
			if (DefaultTexture->CanBeStreamed())
			{
				//@todo streaming: need to handle texture parameter switching.
				TextureIndex = Material->StreamingTextures.AddItem(DefaultTexture);
			}
			return 0; 
		}
		virtual	INT SceneTextureSample(BYTE /*TexType*/,INT /*CoordinateIdx*/) { return 0; }
		virtual	INT SceneTextureDepth( UBOOL /*bNormalize*/, INT /*CoordinateIdx*/) { return 0;	}
		virtual	INT PixelDepth(UBOOL /*bNormalize*/) { return 0; }
		virtual	INT DestColor() { return 0; }
		virtual	INT DestDepth(UBOOL /*bNormalize*/) { return 0;	}
		virtual INT VertexColor() { return 0; }
		virtual INT MeshEmitterVertexColor() { return 0; }
		virtual INT Add(INT A,INT B) { return 0; }
		virtual INT Sub(INT A,INT B) { return 0; }
		virtual INT Mul(INT A,INT B) { return 0; }
		virtual INT Div(INT A,INT B) { return 0; }
		virtual INT Dot(INT A,INT B) { return 0; }
		virtual INT Cross(INT A,INT B) { return 0; }
		virtual INT Power(INT Base,INT Exponent) { return 0; }
		virtual INT SquareRoot(INT X) { return 0; }
		virtual INT Lerp(INT X,INT Y,INT A) { return 0; }
		virtual INT Min(INT A,INT B) { return 0; }
		virtual INT Max(INT A,INT B) { return 0; }
		virtual INT Clamp(INT X,INT A,INT B) { return 0; }
		virtual INT ComponentMask(INT Vector,UBOOL R,UBOOL G,UBOOL B,UBOOL A) { return 0; }
		virtual INT AppendVector(INT A,INT B) { return 0; }
		virtual INT TransformVector(BYTE /*CoordType*/,INT /*A*/) { return 0; }
	} Compiler( this );

	for(INT PropertyIndex = 0;PropertyIndex < MP_MAX;PropertyIndex++)
	{
#if !EXCEPTIONS_DISABLED
		try
#endif
		{
			MaterialResource->CompileProperty((EMaterialProperty)PropertyIndex, &Compiler);
		}
#if !EXCEPTIONS_DISABLED
		catch(const TCHAR*)
		{
		}
#endif
	}

	StreamingTextures.Shrink();
}

//!{ 2006-04-11	Çã Ã¢ ¹Î
/**
* Fills in the DiffuseColorTextures array by using a dummy material compiler that parses the material
* and collects textures that be used for diffuse color.
*/
void UMaterial::CollectDiffuseColorTextures()
{
	DiffuseColorTextures.Empty();

	struct FTextureCollectingMaterialCompiler : public FMaterialCompiler
	{
		UMaterial* Material;

		// Contstructor
		FTextureCollectingMaterialCompiler( UMaterial* InMaterial )
			:	Material( InMaterial )
		{}

		// Gather textures.
		virtual INT Texture(FTextureBase* Texture, INT& TextureIndex) 
		{ 
			TextureIndex = Material->DiffuseColorTextures.AddItem(Texture);
			return 0; 
		}
		virtual INT TextureUnpackMin(INT TextureIndex)			{ return 0;	}
		virtual INT TextureUnpackMaxMinusMin(INT TextureIndex)	{ return 0; }
		virtual INT TextureOffsetParameter()					{ return 0; }
		virtual INT TextureScaleParameter()						{ return 0; }
		virtual INT FlipBookOffset(INT TextureIndex)			{ return 0; }
		virtual INT FlipBookScale(INT TextureIndex)				{ return 0; }

		// Not implemented (don't need to).
		virtual INT Error(const TCHAR* Text) 
		{ 
#if !defined(XBOX) && !defined(EXCEPTIONS_DISABLED)
			throw Text; 
#endif
			return 0; 
		}
		virtual void EnterGuard(FMaterialCompilerGuard* Guard) {}
		virtual void ExitGuard() {}
		virtual INT GetSelectionColorIndex() { return 0; }
		virtual EMaterialCodeType GetType(INT Code) { return MCT_Unknown; }
		virtual INT ForceCast(INT Code,EMaterialCodeType DestType) { return 0; }
		virtual INT VectorParameter(FName ParameterName) { return 0; }
		virtual INT ScalarParameter(FName ParameterName) { return 0; }
		virtual INT Constant(FLOAT X) { return 0; }
		virtual INT Constant2(FLOAT X,FLOAT Y) { return 0; }
		virtual INT Constant3(FLOAT X,FLOAT Y,FLOAT Z) { return 0; }
		virtual INT Constant4(FLOAT X,FLOAT Y,FLOAT Z,FLOAT W) { return 0; }
		virtual INT SceneTime() { return 0; }
		virtual INT Sine(INT X) { return 0; }
		virtual INT Cosine(INT X) { return 0; }
		virtual INT Floor(INT X) { return 0; }
		virtual INT Ceil(INT X) { return 0; }
		virtual INT Frac(INT X) { return 0; }
		virtual INT Abs(INT X) { return 0; }
		virtual INT ReflectionVector() { return 0; }
		virtual INT CameraVector() { return 0; }
		virtual INT LightVector() { return 0; }
		virtual INT ScreenPosition( UBOOL bScreenAlign ) { return 0; }
		virtual INT If(INT A,INT B,INT AGreaterThanB,INT AEqualsB,INT ALessThanB) { return 0; }
		virtual INT TextureCoordinate(UINT CoordinateIndex) { return 0; }
		virtual INT TextureSample(INT Texture,INT Coordinate) { return 0; }
		virtual INT TextureParameter(FTextureBase* DefaultTexture, FName ParameterName, INT& TextureIndex) 
		{ 
			//@todo streaming: need to handle texture parameter switching.
			TextureIndex = Material->DiffuseColorTextures.AddItem(DefaultTexture);
			
			return 0; 
		}
		virtual	INT SceneTextureSample(BYTE /*TexType*/,INT /*CoordinateIdx*/) { return 0; }
		virtual	INT SceneTextureDepth( UBOOL /*bNormalize*/, INT /*CoordinateIdx*/) { return 0;	}
		virtual	INT PixelDepth(UBOOL /*bNormalize*/) { return 0; }
		virtual	INT DestColor() { return 0; }
		virtual	INT DestDepth(UBOOL /*bNormalize*/) { return 0;	}
		virtual INT VertexColor() { return 0; }
		virtual INT MeshEmitterVertexColor() { return 0; }
		virtual INT Add(INT A,INT B) { return 0; }
		virtual INT Sub(INT A,INT B) { return 0; }
		virtual INT Mul(INT A,INT B) { return 0; }
		virtual INT Div(INT A,INT B) { return 0; }
		virtual INT Dot(INT A,INT B) { return 0; }
		virtual INT Cross(INT A,INT B) { return 0; }
		virtual INT Power(INT Base,INT Exponent) { return 0; }
		virtual INT SquareRoot(INT X) { return 0; }
		virtual INT Lerp(INT X,INT Y,INT A) { return 0; }
		virtual INT Min(INT A,INT B) { return 0; }
		virtual INT Max(INT A,INT B) { return 0; }
		virtual INT Clamp(INT X,INT A,INT B) { return 0; }
		virtual INT ComponentMask(INT Vector,UBOOL R,UBOOL G,UBOOL B,UBOOL A) { return 0; }
		virtual INT AppendVector(INT A,INT B) { return 0; }
		virtual INT TransformVector(BYTE /*CoordType*/,INT /*A*/) { return 0; }
	} Compiler( this );

#if !EXCEPTIONS_DISABLED
	try
#endif
	{
		MaterialResource->CompileProperty(MP_DiffuseColor, &Compiler);
	}
#if !EXCEPTIONS_DISABLED
	catch(const TCHAR*)
	{
	}
#endif

	DiffuseColorTextures.Shrink();
}
//!} 2006-04-11	Çã Ã¢ ¹Î

FMaterialInstance* UMaterial::GetInstanceInterface(UBOOL Selected) const
{
	check(!Selected || GIsEditor);
	return DefaultMaterialInstances[Selected];
}

UPhysicalMaterial* UMaterial::GetPhysicalMaterial() const
{
	return PhysMaterial;
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

#ifdef XBOX
	SHM = NULL; //@todo xenon: implement cooking of SHMs for Xenon so they can be enabled again
#endif

	((FMaterialResource*)MaterialResource)->Update(FALSE);

	if (!GIsUCC)
	{
		for(INT Selected = 0;Selected < (GIsEditor ? 2 : 1);Selected++)
		{
			((FDefaultMaterialInstance*)DefaultMaterialInstances[Selected])->Update(this,Selected);
		}
	}

	CollectStreamingTextures();

	//!{ 2006-04-11	Çã Ã¢ ¹Î
	CollectDiffuseColorTextures();
	//!} 2006-04-11	Çã Ã¢ ¹Î
	//!{ 2006-05-03	Çã Ã¢ ¹Î
	bIsDiffuseReflectivityUpdated = FALSE;
	//!} 2006-05-03	Çã Ã¢ ¹Î
}

//
//	UMaterial::PostEditChange
//

void UMaterial::PostEditChange(UProperty* PropertyThatChanged)
{
	PersistentIds[0] = appCreateGuid();
	PersistentIds[1] = appCreateGuid();

	if( !GIsUCC )
	{
		((FMaterialResource*)MaterialResource)->Update();
		for(INT Selected = 0;Selected < (GIsEditor ? 2 : 1);Selected++)
		{
			((FDefaultMaterialInstance*)DefaultMaterialInstances[Selected])->Update(this,Selected);
		}
	}

	CollectStreamingTextures();

	//!{ 2006-04-11	Çã Ã¢ ¹Î
	CollectDiffuseColorTextures();
	bIsDiffuseReflectivityUpdated = FALSE;
	//!} 2006-04-11	Çã Ã¢ ¹Î
	Super::PostEditChange(PropertyThatChanged);
}

//
//	UMaterial::Destroy
//

void UMaterial::Destroy()
{
	if ( !HasAnyFlags(RF_ClassDefaultObject) )
	{
		delete MaterialResource;
		delete DefaultMaterialInstances[0];
		if(GIsEditor)
		{
			delete DefaultMaterialInstances[1];
		}
	}
	else
	{
		check(MaterialResource == NULL);
		check(DefaultMaterialInstances[0]==NULL);
	}
	Super::Destroy();
}

//
//	UMaterialExpression::PostEditChange
//

void UMaterialExpression::PostEditChange(UProperty* PropertyThatChanged)
{
	check(GetOuterUMaterial());
	GetOuterUMaterial()->PostEditChange(NULL);
	Super::PostEditChange(PropertyThatChanged);
}

/**
 * Recursively gathers all UMaterialExpression objects referenced by this expression.
 * Including self.
 *
 * @param	Expressions		Reference to array of material expressions to add to
 */	
void UMaterialExpression::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	Expressions.AddItem( this );
}

//
//	UMaterialExpression::GetOutputs
//

void UMaterialExpression::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	new(Outputs) FExpressionOutput(0);
}

//
//	UMaterialExpression::GetWidth
//

INT UMaterialExpression::GetWidth() const
{
	return ME_STD_THUMBNAIL_SZ+(ME_STD_BORDER*2);
}

//
//	UMaterialExpression::GetHeight
//

INT UMaterialExpression::GetHeight() const
{
	TArray<FExpressionOutput>	Outputs;
	GetOutputs(Outputs);
	return Max(ME_CAPTION_HEIGHT + (Outputs.Num() * ME_STD_TAB_HEIGHT),ME_CAPTION_HEIGHT+ME_STD_THUMBNAIL_SZ+(ME_STD_BORDER*2));
}

//
//	UMaterialExpression::UsesLeftGutter
//

UBOOL UMaterialExpression::UsesLeftGutter() const
{
	return 0;
}

//
//	UMaterialExpression::UsesRightGutter
//

UBOOL UMaterialExpression::UsesRightGutter() const
{
	return 0;
}

//
//	UMaterialExpression::GetCaption
//

FString UMaterialExpression::GetCaption() const
{
	return TEXT("Expression");
}

INT UMaterialExpression::CompilerError(FMaterialCompiler* Compiler, const TCHAR* pcMessage)
{
	return Compiler->Errorf(TEXT("%s> %s"), Desc.Len() > 0 ? *Desc : *GetCaption(), pcMessage);
}

//
//	UMaterialExpressionTextureSample::Compile
//
INT UMaterialExpressionTextureSample::Compile(FMaterialCompiler* Compiler)
{
#if EXCEPTIONS_DISABLED
	// if we can't throw the error below, attempt to thwart the error by using the default texture
	// @todo: handle missing cubemaps and 3d textures?
	if (!Texture)
	{
		if (!GWorld)
		{
			debugf(TEXT("Missing texture in a material (%s), but GWorld is NULL, which we need to get the default texture"), *Desc);
		}
		else
		{
			debugf(TEXT("Using default texture instead of real texture!"));
			Texture = GWorld->GetWorldInfo()->DefaultTexture;
		}
	}
#endif

	if (Texture)
	{
		INT TextureIndex;
		INT TextureCodeIndex = Compiler->Texture(Texture->GetTexture(), TextureIndex);

		INT ArgA = Compiler->TextureSample(
						TextureCodeIndex,
						Coordinates.Expression ? Coordinates.Compile(Compiler) : Compiler->TextureCoordinate(0)
						);
		INT ArgB = Compiler->Constant4(
						Texture->UnpackMax.X - Texture->UnpackMin.X,
						Texture->UnpackMax.Y - Texture->UnpackMin.Y,
						Texture->UnpackMax.Z - Texture->UnpackMin.Z,
						Texture->UnpackMax.W - Texture->UnpackMin.W
						);
		INT ArgC =  Compiler->Constant4(
						Texture->UnpackMin.X,
						Texture->UnpackMin.Y,
						Texture->UnpackMin.Z,
						Texture->UnpackMin.W
						);

		return Compiler->Add(Compiler->Mul(ArgA, ArgB), ArgC);
	}
	else
	{
		if (Desc.Len() > 0)
		{
			return Compiler->Errorf(TEXT("%s> Missing input texture"), *Desc);
		}
		else
		{
			return Compiler->Errorf(TEXT("TextureSample> Missing input texture"));
		}
	}
}

//
//	UMaterialExpressionTextureSample::GetOutputs
//

void UMaterialExpressionTextureSample::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	new(Outputs) FExpressionOutput(1,1,1,1,0);
	new(Outputs) FExpressionOutput(1,1,0,0,0);
	new(Outputs) FExpressionOutput(1,0,1,0,0);
	new(Outputs) FExpressionOutput(1,0,0,1,0);
	new(Outputs) FExpressionOutput(1,0,0,0,1);
}

INT UMaterialExpressionTextureSample::GetWidth() const
{
	return ME_STD_THUMBNAIL_SZ+(ME_STD_BORDER*2);
}

/**
 * Recursively gathers all UMaterialExpression objects referenced by this expression.
 * Including self.
 *
 * @param	Expressions		Reference to array of material expressions to add to
 */	
void UMaterialExpressionTextureSample::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	Expressions.AddItem( this );
	GET_EXPRESSIONS( Coordinates );
}


/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionTextureSample::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Coordinates, Expression );
}

//
//	UMaterialExpressionTextureSample::GetCaption
//

FString UMaterialExpressionTextureSample::GetCaption() const
{
	return TEXT("Texture Sample");
}

/**
 * Recursively gathers all UMaterialExpression objects referenced by this expression.
 * Including self.
 *
 * @param	Expressions		Reference to array of material expressions to add to
 */	
void UMaterialExpressionAdd::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	Expressions.AddItem( this );
	GET_EXPRESSIONS( A );
	GET_EXPRESSIONS( B );
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionAdd::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( A, Expression );
	REMOVE_REFERENCE_TO( B, Expression );
}

//
//  UMaterialExpressionTextureSampleParameter
//
INT UMaterialExpressionTextureSampleParameter::Compile(FMaterialCompiler* Compiler)
{
	if (Texture == NULL)
	{
		SetDefaultTexture();
	}

	if (Texture == NULL)
	{
		return CompilerError(Compiler, GetRequirements());
	}

    if (Texture)
    {
        if (!TextureIsValid(Texture))
        {
            return CompilerError(Compiler, GetRequirements());
        }
    }

	if (!ParameterName.IsValid() || (ParameterName.GetIndex() == NAME_None))
	{
		return UMaterialExpressionTextureSample::Compile(Compiler);
	}

	// We have to guarantee that the texture is added properly...
	INT TextureIndex;
	INT TextureCodeIndex = Compiler->TextureParameter(Texture->GetTexture(), ParameterName, TextureIndex);
	return Compiler->Add(
			Compiler->Mul(
				Compiler->TextureSample(
					TextureCodeIndex, 
					Coordinates.Expression ? Coordinates.Compile(Compiler) : Compiler->TextureCoordinate(0)
					),
					Compiler->TextureUnpackMaxMinusMin(TextureIndex)
				),
			Compiler->TextureUnpackMin(TextureIndex)
			);
}

FString UMaterialExpressionTextureSampleParameter::GetCaption() const
{
    return TEXT("Parameter TextureSample");
}

UBOOL UMaterialExpressionTextureSampleParameter::TextureIsValid( UTexture* /*InTexture*/ )
{
    return false;
}

const TCHAR* UMaterialExpressionTextureSampleParameter::GetRequirements()
{
    return TEXT("Invalid texture type");
}

/**
 *	Sets the default texture if none is set
 */
void UMaterialExpressionTextureSampleParameter::SetDefaultTexture()
{
	// Does nothing in the base case...
}

//
//  UMaterialExpressionTextureSampleParameter2D
//
FString UMaterialExpressionTextureSampleParameter2D::GetCaption() const
{
    return TEXT("2D Sample");
}

UBOOL UMaterialExpressionTextureSampleParameter2D::TextureIsValid( UTexture* InTexture )
{
	UBOOL Result=false;
	if (InTexture)		
    {
        if( InTexture->GetClass() == UTexture2D::StaticClass() ) {
			Result = true;;
		}
		if( InTexture->IsA(UTextureRenderTarget::StaticClass()) )	{
			Result = true;
		}
	}
    return Result;
}

const TCHAR* UMaterialExpressionTextureSampleParameter2D::GetRequirements()
{
    return TEXT("Requires Texture2D");
}

/**
 *	Sets the default texture if none is set
 */
void UMaterialExpressionTextureSampleParameter2D::SetDefaultTexture()
{
	Texture = LoadObject<UTexture2D>(NULL, TEXT("EngineResources.DefaultTexture"), NULL, LOAD_None, NULL);
}

//
//  UMaterialExpressionTextureSampleParameter3D
//
FString UMaterialExpressionTextureSampleParameter3D::GetCaption() const
{
    return TEXT("3D Sample");
}

UBOOL UMaterialExpressionTextureSampleParameter3D::TextureIsValid( UTexture* InTexture )
{
    if (InTexture)
    {
        return (InTexture->GetClass() == UTexture3D::StaticClass());
    }
    return false;
}

const TCHAR* UMaterialExpressionTextureSampleParameter3D::GetRequirements()
{
    return TEXT("Requires Texture3D");
}

//
//  UMaterialExpressionTextureSampleParameterCube
//
FString UMaterialExpressionTextureSampleParameterCube::GetCaption() const
{
    return TEXT("Cube Sample");
}

UBOOL UMaterialExpressionTextureSampleParameterCube::TextureIsValid( UTexture* InTexture )
{
	UBOOL Result=false;
    if (InTexture)
    {
		if( InTexture->GetClass() == UTextureCube::StaticClass() ) {
			Result = true;
		}
		if( InTexture->IsA(UTextureRenderTargetCube::StaticClass()) ) {
			Result = true;
		}
    }
    return Result;
}

const TCHAR* UMaterialExpressionTextureSampleParameterCube::GetRequirements()
{
    return TEXT("Requires TextureCube");
}

/**
 *	Sets the default texture if none is set
 */
void UMaterialExpressionTextureSampleParameterCube::SetDefaultTexture()
{
	Texture = LoadObject<UTextureCube>(NULL, TEXT("EngineResources.DefaultTextureCube"), NULL, LOAD_None, NULL);
}

/**
* Textual description for this material expression
*
* @return	Caption text
*/	
FString UMaterialExpressionTextureSampleParameterMovie::GetCaption() const
{
    return TEXT("Movie Sample");
}

/**
* Return true if the texture is a movie texture
*
* @return	true/false
*/	
UBOOL UMaterialExpressionTextureSampleParameterMovie::TextureIsValid( UTexture* InTexture )
{
	UBOOL Result=false;
    if (InTexture)
    {
		Result = (InTexture->GetClass() == UTextureMovie::StaticClass());
    }
    return Result;
}

/**
* Called when TextureIsValid==false
*
* @return	Descriptive error text
*/	
const TCHAR* UMaterialExpressionTextureSampleParameterMovie::GetRequirements()
{
    return TEXT("Requires TextureMovie");
}

//
//	UMaterialExpressionFlipBookSample
//
INT UMaterialExpressionFlipBookSample::Compile(FMaterialCompiler* Compiler)
{
	if (Texture)
	{
		if (!Texture->IsA(UTextureFlipBook::StaticClass()))
		{
			return Compiler->Errorf(TEXT("FlipBookSample> Texture is not a FlipBook"));
		}

		//                                 | Mul |
		//	FlipBookScale-->CompMask(XY)-->|A    |----\      | Add |   |Sample|
		//	TextureCoord(0)--------------->|B    |     \---->|A    |-->|Coord |
		//	FlipBookOffset->CompMask(XY)-------------------->|B    |   |      |
		// Out	 = sub-UV sample of the input texture
		//
		INT TextureIndex;
		INT TextureCodeIndex	= Compiler->Texture(Texture->GetTexture(), TextureIndex);
		return Compiler->Add(
				Compiler->Mul(
					Compiler->TextureSample(
						TextureCodeIndex,
						Compiler->Add(
							Compiler->Mul(
								Coordinates.Expression ? Coordinates.Compile(Compiler) : Compiler->TextureCoordinate(0),
								Compiler->ComponentMask(
									Compiler->FlipBookScale(TextureIndex),
									1, 
									1, 
									0, 
									0
									)
								),
							Compiler->ComponentMask(
								Compiler->FlipBookOffset(TextureIndex),
								1, 
								1, 
								0, 
								0
								)
							)
						),
					Compiler->Constant4(
						Texture->UnpackMax.X - Texture->UnpackMin.X,
						Texture->UnpackMax.Y - Texture->UnpackMin.Y,
						Texture->UnpackMax.Z - Texture->UnpackMin.Z,
						Texture->UnpackMax.W - Texture->UnpackMin.W
						)
					),
				Compiler->Constant4(
					Texture->UnpackMin.X,
					Texture->UnpackMin.Y,
					Texture->UnpackMin.Z,
					Texture->UnpackMin.W
					)
				);
	}
	else
	{
		if (Desc.Len() > 0)
		{
			return Compiler->Errorf(TEXT("%s> Missing input texture"), *Desc);
		}
		else
		{
			return Compiler->Errorf(TEXT("TextureSample> Missing input texture"));
		}
	}
}

FString UMaterialExpressionFlipBookSample::GetCaption() const
{
    return TEXT("FlipBook Sample");
}

//
//	UMaterialExpressionAdd::Compile
//

INT UMaterialExpressionAdd::Compile(FMaterialCompiler* Compiler)
{
	if(!A.Expression)
		return Compiler->Errorf(TEXT("Missing Add input A"));
	else if(!B.Expression)
		return Compiler->Errorf(TEXT("Missing Add input B"));
	else
	{
		INT Arg1 = A.Compile(Compiler);
		INT Arg2 = B.Compile(Compiler);
		return Compiler->Add(
			Arg1,
			Arg2
			);
	}
}

//
//	UMaterialExpressionAdd::GetCaption
//

FString UMaterialExpressionAdd::GetCaption() const
{
	return TEXT("Add");
}


//
//	UMaterialExpressionMeshEmitterVertexColor::Compile
//
INT UMaterialExpressionMeshEmitterVertexColor::Compile(FMaterialCompiler* Compiler)
{
		return Compiler->MeshEmitterVertexColor();
}

//
//	UMaterialExpressionMeshEmitterVertexColor::GetCaption
//
FString UMaterialExpressionMeshEmitterVertexColor::GetCaption() const
{
	return TEXT("MeshEmit VertColor");
}

//
//	UMaterialExpressionMeshEmitterVertexColor::GetOutputs
//
void UMaterialExpressionMeshEmitterVertexColor::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	new(Outputs) FExpressionOutput(1,1,1,1,0);
	new(Outputs) FExpressionOutput(1,1,0,0,0);
	new(Outputs) FExpressionOutput(1,0,1,0,0);
	new(Outputs) FExpressionOutput(1,0,0,1,0);
	new(Outputs) FExpressionOutput(1,0,0,0,1);
}

//
//	UMaterialExpressionMultiply::Compile
//

INT UMaterialExpressionMultiply::Compile(FMaterialCompiler* Compiler)
{
	if(!A.Expression)
		return Compiler->Errorf(TEXT("Missing Multiply input A"));
	else if(!B.Expression)
		return Compiler->Errorf(TEXT("Missing Multiply input B"));
	else
	{
		INT Arg1 = A.Compile(Compiler);
		INT Arg2 = B.Compile(Compiler);
		return Compiler->Mul(
			Arg1,
			Arg2
			);
	}
}

//
//	UMaterialExpressionMultiply::GetCaption
//

FString UMaterialExpressionMultiply::GetCaption() const
{
	return TEXT("Multiply");
}

/**
 * Recursively gathers all UMaterialExpression objects referenced by this expression.
 * Including self.
 *
 * @param	Expressions		Reference to array of material expressions to add to
 */	
void UMaterialExpressionMultiply::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	Expressions.AddItem( this );
	GET_EXPRESSIONS( A );
	GET_EXPRESSIONS( B );
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionMultiply::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( A, Expression );
	REMOVE_REFERENCE_TO( B, Expression );
}

///////////////////////////////////////////////////////////////////////////////
// UMaterialExpressionDestColor
///////////////////////////////////////////////////////////////////////////////
/**
 *	Compile the expression
 *
 *	@param	Compiler	The compiler to utilize
 *
 *	@return				The index of the resulting code chunk.
 *						INDEX_NONE if error.
 */
INT UMaterialExpressionDestColor::Compile(FMaterialCompiler* Compiler)
{
	// resulting index to compiled code chunk
	// add the code chunk for the scene color sample        
	INT Result = Compiler->DestColor();
	return Result;
}

/**
 *	Retrieve the outputs this expression supplies.
 *
 *	@param	Outputs		The array to insert the available outputs in.
 */
void UMaterialExpressionDestColor::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	// RGB
	new(Outputs) FExpressionOutput(1,1,1,1,0);
	// R
	new(Outputs) FExpressionOutput(1,1,0,0,0);
	// G
	new(Outputs) FExpressionOutput(1,0,1,0,0);
	// B
	new(Outputs) FExpressionOutput(1,0,0,1,0);
	// A
	new(Outputs) FExpressionOutput(1,0,0,0,1);
}

/**
 *	Get the caption to display on this material expression
 *
 *	@return			An FString containing the display caption
 */
FString UMaterialExpressionDestColor::GetCaption() const
{
	return TEXT("DestColor");
}

///////////////////////////////////////////////////////////////////////////////
// UMaterialExpressionDestDepth
///////////////////////////////////////////////////////////////////////////////
/**
 *	Compile the expression
 *
 *	@param	Compiler	The compiler to utilize
 *
 *	@return				The index of the resulting code chunk.
 *						INDEX_NONE if error.
 */
INT UMaterialExpressionDestDepth::Compile(FMaterialCompiler* Compiler)
{
	// resulting index to compiled code chunk
	// add the code chunk for the scene depth sample        
	INT Result = Compiler->DestDepth(bNormalize);
	return Result;
}

/**
 *	Retrieve the outputs this expression supplies.
 *
 *	@param	Outputs		The array to insert the available outputs in.
 */
void UMaterialExpressionDestDepth::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	// Depth
	new(Outputs) FExpressionOutput(1,1,0,0,0);
}

/**
 *	Get the caption to display on this material expression
 *
 *	@return			An FString containing the display caption
 */
FString UMaterialExpressionDestDepth::GetCaption() const
{
	return TEXT("DestDepth");
}

//
//	UMaterialExpressionDivide::Compile
//

INT UMaterialExpressionDivide::Compile(FMaterialCompiler* Compiler)
{
	if(!A.Expression)
		return Compiler->Errorf(TEXT("Missing Divide input A"));
	else if(!B.Expression)
		return Compiler->Errorf(TEXT("Missing Divide input B"));
	else
	{
		INT Arg1 = A.Compile(Compiler);
		INT Arg2 = B.Compile(Compiler);
		return Compiler->Div(
			Arg1,
			Arg2
			);
	}
}

//
//	UMaterialExpressionDivide::GetCaption
//

FString UMaterialExpressionDivide::GetCaption() const
{
	return TEXT("Divide");
}

/**
 * Recursively gathers all UMaterialExpression objects referenced by this expression.
 * Including self.
 *
 * @param	Expressions		Reference to array of material expressions to add to
 */	
void UMaterialExpressionDivide::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	Expressions.AddItem( this );
	GET_EXPRESSIONS( A );
	GET_EXPRESSIONS( B );
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionDivide::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( A, Expression );
	REMOVE_REFERENCE_TO( B, Expression );
}

//
//	UMaterialExpressionSubtract::Compile
//

INT UMaterialExpressionSubtract::Compile(FMaterialCompiler* Compiler)
{
	if(!A.Expression)
		return Compiler->Errorf(TEXT("Missing Subtract input A"));
	else if(!B.Expression)
		return Compiler->Errorf(TEXT("Missing Subtract input B"));
	else
	{
		INT Arg1 = A.Compile(Compiler);
		INT Arg2 = B.Compile(Compiler);
		return Compiler->Sub(
			Arg1,
			Arg2
			);
	}
}

//
//	UMaterialExpressionSubtract::GetCaption
//

FString UMaterialExpressionSubtract::GetCaption() const
{
	return TEXT("Subtract");
}

/**
 * Recursively gathers all UMaterialExpression objects referenced by this expression.
 * Including self.
 *
 * @param	Expressions		Reference to array of material expressions to add to
 */	
void UMaterialExpressionSubtract::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	Expressions.AddItem( this );
	GET_EXPRESSIONS( A );
	GET_EXPRESSIONS( B );
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionSubtract::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( A, Expression );
	REMOVE_REFERENCE_TO( B, Expression );
}

//
//	UMaterialExpressionLinearInterpolate::Compile
//

INT UMaterialExpressionLinearInterpolate::Compile(FMaterialCompiler* Compiler)
{
	if(!A.Expression)
		return Compiler->Errorf(TEXT("Missing LinearInterpolate input A"));
	else if(!B.Expression)
		return Compiler->Errorf(TEXT("Missing LinearInterpolate input B"));
	else if(!Alpha.Expression)
		return Compiler->Errorf(TEXT("Missing LinearInterpolate input Alpha"));
	else
	{
		INT Arg1 = A.Compile(Compiler);
		INT Arg2 = B.Compile(Compiler);
		INT Arg3 = Alpha.Compile(Compiler);
		return Compiler->Lerp(
			Arg1,
			Arg2,
			Arg3
			);
	}
}

//
//	UMaterialExpressionLinearInterpolate::GetCaption
//

FString UMaterialExpressionLinearInterpolate::GetCaption() const
{
	return TEXT("Linear Interpolat");
}

/**
 * Recursively gathers all UMaterialExpression objects referenced by this expression.
 * Including self.
 *
 * @param	Expressions		Reference to array of material expressions to add to
 */	
void UMaterialExpressionLinearInterpolate::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	Expressions.AddItem( this );
	GET_EXPRESSIONS( A );
	GET_EXPRESSIONS( B );
	GET_EXPRESSIONS( Alpha );
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionLinearInterpolate::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( A, Expression );
	REMOVE_REFERENCE_TO( B, Expression );
	REMOVE_REFERENCE_TO( Alpha, Expression );
}

//
//	UMaterialExpressionConstant::Compile
//

INT UMaterialExpressionConstant::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->Constant(R);
}

//
//	UMaterialExpressionConstant::GetCaption
//

FString UMaterialExpressionConstant::GetCaption() const
{
	return TEXT("Constant");
}

//
//	UMaterialExpressionConstant2Vector::Compile
//

INT UMaterialExpressionConstant2Vector::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->Constant2(R,G);
}

//
//	UMaterialExpressionConstant2Vector::GetCaption
//

FString UMaterialExpressionConstant2Vector::GetCaption() const
{
	return TEXT("Constant 2 Vector");
}

//
//	UMaterialExpressionConstant3Vector::Compile
//

INT UMaterialExpressionConstant3Vector::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->Constant3(R,G,B);
}

//
//	UMaterialExpressionConstant3Vector::GetCaption
//

FString UMaterialExpressionConstant3Vector::GetCaption() const
{
	return TEXT("Constant 3 Vector");
}

//
//	UMaterialExpressionConstant4Vector::Compile
//

INT UMaterialExpressionConstant4Vector::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->Constant4(R,G,B,A);
}

//
//	UMaterialExpressionConstant4Vector::GetCaption
//

FString UMaterialExpressionConstant4Vector::GetCaption() const
{
	return TEXT("Constant 4 Vector");
}

//
//	UMaterialExpressionClamp::Compile
//

INT UMaterialExpressionClamp::Compile(FMaterialCompiler* Compiler)
{
	if(!Input.Expression)
		return Compiler->Errorf(TEXT("Missing Clamp input"));
	else
	{
		if(!Min.Expression && !Max.Expression)
			return Input.Compile(Compiler);
		else if(!Min.Expression)
		{
			INT Arg1 = Input.Compile(Compiler);
			INT Arg2 = Max.Compile(Compiler);
			return Compiler->Min(
				Arg1,
				Arg2
				);
		}
		else if(!Max.Expression)
		{
			INT Arg1 = Input.Compile(Compiler);
			INT Arg2 = Min.Compile(Compiler);
			return Compiler->Max(
				Arg1,
				Arg2
				);
		}
		else
		{
			INT Arg1 = Input.Compile(Compiler);
			INT Arg2 = Min.Compile(Compiler);
			INT Arg3 = Max.Compile(Compiler);
			return Compiler->Clamp(
				Arg1,
				Arg2,
				Arg3
				);
		}
	}
}

//
//	UMaterialExpressionClamp::GetCaption
//

FString UMaterialExpressionClamp::GetCaption() const
{
	return TEXT("Clamp");
}

/**
 * Recursively gathers all UMaterialExpression objects referenced by this expression.
 * Including self.
 *
 * @param	Expressions		Reference to array of material expressions to add to
 */	
void UMaterialExpressionClamp::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	Expressions.AddItem( this );
	GET_EXPRESSIONS( Input );
	GET_EXPRESSIONS( Min );
	GET_EXPRESSIONS( Max );
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionClamp::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Input, Expression );
	REMOVE_REFERENCE_TO( Min, Expression );
	REMOVE_REFERENCE_TO( Max, Expression );
}

//
//	UMaterialExpressionTextureCoordinate::Compile
//

INT UMaterialExpressionTextureCoordinate::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->Mul(Compiler->TextureCoordinate(CoordinateIndex),Compiler->Constant(Tiling));
}

//
//	UMaterialExpressionTextureCoordinate::GetCaption
//

FString UMaterialExpressionTextureCoordinate::GetCaption() const
{
	return TEXT("Texture Coordinate");
}

//
//	UMaterialExpressionDotProduct::Compile
//

INT UMaterialExpressionDotProduct::Compile(FMaterialCompiler* Compiler)
{
	if(!A.Expression)
		return Compiler->Errorf(TEXT("Missing DotProduct input A"));
	else if(!B.Expression)
		return Compiler->Errorf(TEXT("Missing DotProduct input B"));
	else
	{
		INT Arg1 = A.Compile(Compiler);
		INT Arg2 = B.Compile(Compiler);
		return Compiler->Dot(
			Arg1,
			Arg2
			);
	}
}

//
//	UMaterialExpressionDotProduct::GetCaption
//

FString UMaterialExpressionDotProduct::GetCaption() const
{
	return TEXT("Dot Product");
}

/**
 * Recursively gathers all UMaterialExpression objects referenced by this expression.
 * Including self.
 *
 * @param	Expressions		Reference to array of material expressions to add to
 */	
void UMaterialExpressionDotProduct::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	Expressions.AddItem( this );
	GET_EXPRESSIONS( A );
	GET_EXPRESSIONS( B );
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionDotProduct::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( A, Expression );
	REMOVE_REFERENCE_TO( B, Expression );
}

//
//	UMaterialExpressionCrossProduct::Compile
//

INT UMaterialExpressionCrossProduct::Compile(FMaterialCompiler* Compiler)
{
	if(!A.Expression)
		return Compiler->Errorf(TEXT("Missing CrossProduct input A"));
	else if(!B.Expression)
		return Compiler->Errorf(TEXT("Missing CrossProduct input B"));
	else
	{
		INT Arg1 = A.Compile(Compiler);
		INT Arg2 = B.Compile(Compiler);
		return Compiler->Cross(
			Arg1,
			Arg2
			);
	}
}

//
//	UMaterialExpressionCrossProduct::GetCaption
//

FString UMaterialExpressionCrossProduct::GetCaption() const
{
	return TEXT("Cross Product");
}

/**
 * Recursively gathers all UMaterialExpression objects referenced by this expression.
 * Including self.
 *
 * @param	Expressions		Reference to array of material expressions to add to
 */	
void UMaterialExpressionCrossProduct::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	Expressions.AddItem( this );
	GET_EXPRESSIONS( A );
	GET_EXPRESSIONS( B );
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionCrossProduct::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( A, Expression );
	REMOVE_REFERENCE_TO( B, Expression );
}

//
//	UMaterialExpressionComponentMask::Compile
//

INT UMaterialExpressionComponentMask::Compile(FMaterialCompiler* Compiler)
{
	if(!Input.Expression)
		return Compiler->Errorf(TEXT("Missing ComponentMask input"));
	else
		return Compiler->ComponentMask(
			Input.Compile(Compiler),
			R,
			G,
			B,
			A
			);
}

//
//	UMaterialExpressionComponentMask::GetCaption
//

FString UMaterialExpressionComponentMask::GetCaption() const
{
	return TEXT("Component Mask");
}

/**
 * Recursively gathers all UMaterialExpression objects referenced by this expression.
 * Including self.
 *
 * @param	Expressions		Reference to array of material expressions to add to
 */	
void UMaterialExpressionComponentMask::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	Expressions.AddItem( this );
	GET_EXPRESSIONS( Input );
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionComponentMask::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Input, Expression );
}

//
//	UMaterialExpressionTime::Compile
//

INT UMaterialExpressionTime::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->SceneTime();
}

//
//	UMaterialExpressionTime::GetCaption
//

FString UMaterialExpressionTime::GetCaption() const
{
	return TEXT("Time");
}

//
//	UMaterialExpressionCameraVector::Compile
//

INT UMaterialExpressionCameraVector::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->CameraVector();
}

//
//	UMaterialExpressionCameraVector::GetCaption
//

FString UMaterialExpressionCameraVector::GetCaption() const
{
	return TEXT("Camera Vector");
}

//
//	UMaterialExpressionReflectionVector::Compile
//

INT UMaterialExpressionReflectionVector::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->ReflectionVector();
}

//
//	UMaterialExpressionReflectionVector::GetCaption
//

FString UMaterialExpressionReflectionVector::GetCaption() const
{
	return TEXT("Reflection Vector");
}

//
//	UMaterialExpressionPanner::Compile
//

INT UMaterialExpressionPanner::Compile(FMaterialCompiler* Compiler)
{
	INT Arg1 = Compiler->PeriodicHint(Compiler->Mul(Time.Expression ? Time.Compile(Compiler) : Compiler->SceneTime(),Compiler->Constant(SpeedX)));
	INT Arg2 = Compiler->PeriodicHint(Compiler->Mul(Time.Expression ? Time.Compile(Compiler) : Compiler->SceneTime(),Compiler->Constant(SpeedY)));
	INT Arg3 = Coordinate.Expression ? Coordinate.Compile(Compiler) : Compiler->TextureCoordinate(0);
	return Compiler->Add(
			Compiler->AppendVector(
				Arg1,
				Arg2
				),
			Arg3
			);
}

/**
 * Recursively gathers all UMaterialExpression objects referenced by this expression.
 * Including self.
 *
 * @param	Expressions		Reference to array of material expressions to add to
 */	
void UMaterialExpressionPanner::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	Expressions.AddItem( this );
	GET_EXPRESSIONS( Coordinate );
	GET_EXPRESSIONS( Time );
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionPanner::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Coordinate, Expression );
	REMOVE_REFERENCE_TO( Time, Expression );
}

//
//	UMaterialExpressionPanner::GetCaption
//

FString UMaterialExpressionPanner::GetCaption() const
{
	return TEXT("Panner");
}

//
//	UMaterialExpressionRotator::Compile
//

INT UMaterialExpressionRotator::Compile(FMaterialCompiler* Compiler)
{
	INT	Cosine = Compiler->Cosine(Compiler->Mul(Time.Expression ? Time.Compile(Compiler) : Compiler->SceneTime(),Compiler->Constant(Speed))),
		Sine = Compiler->Sine(Compiler->Mul(Time.Expression ? Time.Compile(Compiler) : Compiler->SceneTime(),Compiler->Constant(Speed))),
		RowX = Compiler->AppendVector(Cosine,Compiler->Mul(Compiler->Constant(-1.0f),Sine)),
		RowY = Compiler->AppendVector(Sine,Cosine),
		Origin = Compiler->Constant2(CenterX,CenterY),
		BaseCoordinate = Coordinate.Expression ? Coordinate.Compile(Compiler) : Compiler->TextureCoordinate(0);

	INT Arg1 = Compiler->Dot(RowX,Compiler->Sub(Compiler->ComponentMask(BaseCoordinate,1,1,0,0),Origin));
	INT Arg2 = Compiler->Dot(RowY,Compiler->Sub(Compiler->ComponentMask(BaseCoordinate,1,1,0,0),Origin));

	if(Compiler->GetType(BaseCoordinate) == MCT_Float3)
		return Compiler->AppendVector(
				Compiler->Add(
					Compiler->AppendVector(
						Arg1,
						Arg2
						),
					Origin
					),
				Compiler->ComponentMask(BaseCoordinate,0,0,1,0)
				);
	else
	{
		INT Arg1 = Compiler->Dot(RowX,Compiler->Sub(BaseCoordinate,Origin));
		INT Arg2 = Compiler->Dot(RowY,Compiler->Sub(BaseCoordinate,Origin));

		return Compiler->Add(
				Compiler->AppendVector(
					Arg1,
					Arg2
					),
				Origin
				);
	}
}

/**
 * Recursively gathers all UMaterialExpression objects referenced by this expression.
 * Including self.
 *
 * @param	Expressions		Reference to array of material expressions to add to
 */	
void UMaterialExpressionRotator::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	Expressions.AddItem( this );
	GET_EXPRESSIONS( Coordinate );
	GET_EXPRESSIONS( Time );
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionRotator::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Coordinate, Expression );
	REMOVE_REFERENCE_TO( Time, Expression );
}

//
//	UMaterialExpressionRotator::GetCaption
//

FString UMaterialExpressionRotator::GetCaption() const
{
	return TEXT("Rotator");
}

//
//	UMaterialExpressionSine::Compile
//

INT UMaterialExpressionSine::Compile(FMaterialCompiler* Compiler)
{
	if(!Input.Expression)
		return Compiler->Errorf(TEXT("Missing Sine input"));
	return Compiler->Sine(Period > 0.0f ? Compiler->Mul(Input.Compile(Compiler),Compiler->Constant(2.0f * (FLOAT)PI / Period)) : Input.Compile(Compiler));
}

/**
 * Recursively gathers all UMaterialExpression objects referenced by this expression.
 * Including self.
 *
 * @param	Expressions		Reference to array of material expressions to add to
 */	
void UMaterialExpressionSine::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	Expressions.AddItem( this );
	GET_EXPRESSIONS( Input );
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionSine::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Input, Expression );
}

//
//	UMaterialExpressionSine::GetCaption
//

FString UMaterialExpressionSine::GetCaption() const
{
	return TEXT("Sine");
}

//
//	UMaterialExpressionCosine::Compile
//

INT UMaterialExpressionCosine::Compile(FMaterialCompiler* Compiler)
{
	if(!Input.Expression)
	{
		return Compiler->Errorf(TEXT("Missing Cosine input"));
	}
	return Compiler->Cosine(Compiler->Mul(Input.Compile(Compiler),Period > 0.0f ? Compiler->Constant(2.0f * (FLOAT)PI / Period) : 0));
}

/**
 * Recursively gathers all UMaterialExpression objects referenced by this expression.
 * Including self.
 *
 * @param	Expressions		Reference to array of material expressions to add to
 */	
void UMaterialExpressionCosine::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	Expressions.AddItem( this );
	GET_EXPRESSIONS( Input );
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionCosine::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Input, Expression );
}

//
//	UMaterialExpressionCosine::GetCaption
//

FString UMaterialExpressionCosine::GetCaption() const
{
	return TEXT("Cosine");
}

//
//	UMaterialExpressionBumpOffset::Compile
//

INT UMaterialExpressionBumpOffset::Compile(FMaterialCompiler* Compiler)
{
	if(!Height.Expression)
		return Compiler->Errorf(TEXT("Missing Height input"));

	return Compiler->Add(
			Compiler->Mul(
				Compiler->ComponentMask(Compiler->CameraVector(),1,1,0,0),
				Compiler->Add(
					Compiler->Mul(
						Compiler->Constant(HeightRatio),
						Compiler->ForceCast(Height.Compile(Compiler),MCT_Float1)
						),
					Compiler->Constant(-ReferencePlane * HeightRatio)
					)
				),
			Coordinate.Expression ? Coordinate.Compile(Compiler) : Compiler->TextureCoordinate(0)
			);
}

//
//	UMaterialExpressionBumpOffset::GetCaption
//

FString UMaterialExpressionBumpOffset::GetCaption() const
{
	return TEXT("BumpOffset");
}

/**
 * Recursively gathers all UMaterialExpression objects referenced by this expression.
 * Including self.
 *
 * @param	Expressions		Reference to array of material expressions to add to
 */	
void UMaterialExpressionBumpOffset::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	Expressions.AddItem( this );
	GET_EXPRESSIONS( Height );
	GET_EXPRESSIONS( Coordinate );
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionBumpOffset::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Height, Expression );
	REMOVE_REFERENCE_TO( Coordinate, Expression );
}

//
//	UMaterialExpressionAppendVector::Compile
//

INT UMaterialExpressionAppendVector::Compile(FMaterialCompiler* Compiler)
{
	if(!A.Expression)
		return Compiler->Errorf(TEXT("Missing AppendVector input A"));
	else if(!B.Expression)
		return Compiler->Errorf(TEXT("Missing AppendVector input B"));
	else
	{
		INT Arg1 = A.Compile(Compiler);
		INT Arg2 = B.Compile(Compiler);
		return Compiler->AppendVector(
			Arg1,
			Arg2
			);
	}
}

//
//	UMaterialExpressionAppendVector::GetCaption
//

FString UMaterialExpressionAppendVector::GetCaption() const
{
	return TEXT("AppendVector");
}

/**
 * Recursively gathers all UMaterialExpression objects referenced by this expression.
 * Including self.
 *
 * @param	Expressions		Reference to array of material expressions to add to
 */	
void UMaterialExpressionAppendVector::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	Expressions.AddItem( this );
	GET_EXPRESSIONS( A );
	GET_EXPRESSIONS( B );
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionAppendVector::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( A, Expression );
	REMOVE_REFERENCE_TO( B, Expression );
}

//
//	UMaterialExpressionFloor::Compile
//

INT UMaterialExpressionFloor::Compile(FMaterialCompiler* Compiler)
{
	if(!Input.Expression)
		return Compiler->Errorf(TEXT("Missing Floor input"));
	return Compiler->Floor(Input.Compile(Compiler));
}

//
//	UMaterialExpressionFloor::GetCaption
//

FString UMaterialExpressionFloor::GetCaption() const
{
	return TEXT("Floor");
}

/**
 * Recursively gathers all UMaterialExpression objects referenced by this expression.
 * Including self.
 *
 * @param	Expressions		Reference to array of material expressions to add to
 */	
void UMaterialExpressionFloor::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	Expressions.AddItem( this );
	GET_EXPRESSIONS( Input );
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionFloor::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Input, Expression );
}

//
//	UMaterialExpressionCeil::Compile
//

INT UMaterialExpressionCeil::Compile(FMaterialCompiler* Compiler)
{
	if(!Input.Expression)
		return Compiler->Errorf(TEXT("Missing Ceil input"));
	return Compiler->Ceil(Input.Compile(Compiler));
}

/**
 * Recursively gathers all UMaterialExpression objects referenced by this expression.
 * Including self.
 *
 * @param	Expressions		Reference to array of material expressions to add to
 */	
void UMaterialExpressionCeil::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	Expressions.AddItem( this );
	GET_EXPRESSIONS( Input );
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionCeil::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Input, Expression );
}

//
//	UMaterialExpressionCeil::GetCaption
//

FString UMaterialExpressionCeil::GetCaption() const
{
	return TEXT("Ceil");
}

//
//	UMaterialExpressionFrac::Compile
//

INT UMaterialExpressionFrac::Compile(FMaterialCompiler* Compiler)
{
	if(!Input.Expression)
		return Compiler->Errorf(TEXT("Missing Frac input"));
	return Compiler->Frac(Input.Compile(Compiler));
}

/**
 * Recursively gathers all UMaterialExpression objects referenced by this expression.
 * Including self.
 *
 * @param	Expressions		Reference to array of material expressions to add to
 */	
void UMaterialExpressionFrac::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	Expressions.AddItem( this );
	GET_EXPRESSIONS( Input );
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionFrac::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Input, Expression );
}

//
//	UMaterialExpressionFrac::GetCaption
//

FString UMaterialExpressionFrac::GetCaption() const
{
	return TEXT("Frac");
}

///////////////////////////////////////////////////////////
// UMaterialExpressionDepthBiasBlend
///////////////////////////////////////////////////////////
#define _DEPTHBIAS_USE_ONE_MINUS_BIAS_
/**
 *	Compile the material expression
 *
 *	@param	Compiler	Pointer to the material compiler to use
 *
 *	@return	INT			The compiled code index
 */	
INT UMaterialExpressionDepthBiasBlend::Compile(FMaterialCompiler* Compiler)
{
	//   +----------------+
	//   | DepthBiasBlend |
	//   +----------------+
	// --| RGB       Bias |--       BiasCalc = (1 - Bias) or -Bias... still determining.
	// --| R    BiasScale |--
	// --| G              |
	// --| B              |
	// --| Alpha          |
	//   +----------------+                     +-----+     +----------+
	//                      +------------+      | Sub |  /--| DstDepth |
	//        +-----+    /--| PixelDepth |      |    A|-/   +----------+-----------+
	//        | If  |   /   +------------+  /---|    B|-----| BiasCalc * BiasScale |
	// RGB ---|    A|--/  /----------------/    +-----+     +----------------------+
	//        |    B|----/  +-----+     +------------+                    +----------+
	//        |    >|--\    | If  |  /--| DstDepth   |     +--------+   /-| DstColor |
	//        |    =|-------|    A|-/   +------------+     | Lerp   |  /  +----------+
	//        |    <|-\     |    B|-----| PixelDepth |     |       A|-/ /-| SrcColor |   +----------+      +------------+
	//        +-----+ |     |    >|-\   +------------+     |       B|--/  +----------+   | Subtract |   /--| DstDepth   |
	//                |     |    =|------------------------|   Alpha|-\                  |         A|--/   +------------+
	//                |     |    <|--\                     +--------+  \              /--|         B|------| PixelDepth |
	//                |     +-----+   \                                 \   +-----+  /   +----------+      +------------+
	//                |  +----------+  \   +----------+                  \--| /  A|-/   +----------------------+
	//                \--| SrcColor |   \--| DstColor |                     |    B|-----| BiasCalc * BiasScale |
	//                   +----------+      +----------+                     +-----+     +----------------------+
	//
    //@todo. Add support for this
    //BITFIELD bNormalize:1;

#if EXCEPTIONS_DISABLED
	// if we can't throw the error below, attempt to thwart the error by using the default texture
	// @todo: handle missing cubemaps and 3d textures?
	if (!Texture)
	{
		debugf(TEXT("Using default texture instead of real texture!"));
		Texture = GWorld->GetWorldInfo()->DefaultTexture;
	}
#endif

	if (Texture)
	{
		INT SrcTextureIndex;
		INT SrcTextureCodeIndex = Compiler->Texture(Texture->GetTexture(), SrcTextureIndex);

		INT ArgA	=	Compiler->TextureSample(
							SrcTextureCodeIndex,
							Coordinates.Expression ? 
								Coordinates.Compile(Compiler) : 
								Compiler->TextureCoordinate(0)
							);
		INT ArgB	=	Compiler->Constant4(
							Texture->UnpackMax.X - Texture->UnpackMin.X,
							Texture->UnpackMax.Y - Texture->UnpackMin.Y,
							Texture->UnpackMax.Z - Texture->UnpackMin.Z,
							Texture->UnpackMax.W - Texture->UnpackMin.W
							);
		INT ArgC	=	Compiler->Constant4(
							Texture->UnpackMin.X,
							Texture->UnpackMin.Y,
							Texture->UnpackMin.Z,
							Texture->UnpackMin.W
							);


		INT	Arg_SrcSample	=	Compiler->Add(Compiler->Mul(ArgA, ArgB), ArgC);
		INT Arg_DstSample	=	Compiler->DestColor();
		INT	Arg_SrcDepth	=	Compiler->PixelDepth(bNormalize);
		INT	Arg_DstDepth	=	Compiler->DestDepth(bNormalize);
		INT	Arg_Constant_0	=	Compiler->Constant(0.0f);
		INT	Arg_Constant_1	=	Compiler->Constant(1.0f);

#if defined(_DEPTHBIAS_USE_ONE_MINUS_BIAS_)
		INT	Arg_Bias		=	(Bias.Expression) ? 
									Compiler->Sub(Arg_Constant_1, Bias.Compile(Compiler)) : 
									Arg_Constant_1;
#else	//#if defined(_DEPTHBIAS_USE_ONE_MINUS_BIAS_)
		INT	Arg_Bias		=	(Bias.Expression) ? 
									Compiler->Sub(Arg_Constant_0, Bias.Compile(Compiler)) : 
									Arg_Constant_0;
#endif	//#if defined(_DEPTHBIAS_USE_ONE_MINUS_BIAS_)
		
		INT	Arg_BiasScaleConstant		=	Compiler->Constant(BiasScale);
		INT	Arg_ScaledBias				=	Compiler->Mul(Arg_Bias, Arg_BiasScaleConstant);
		INT	Arg_Sub_DstDepth_Bias		=	Compiler->Sub(Arg_DstDepth, Arg_ScaledBias);
		INT	Arg_Sub_DstDepth_SrcDepth	=	Compiler->Sub(Arg_DstDepth, Arg_SrcDepth);
		INT	Arg_Div_DZ_Sub_SZ_Bias		=	Compiler->Div(Arg_Sub_DstDepth_SrcDepth, Arg_ScaledBias);
		INT Arg_ClampedLerpBias			=	Compiler->Clamp(Arg_Div_DZ_Sub_SZ_Bias, Arg_Constant_0, Arg_Constant_1);

		INT	Arg_Lerp_Dst_Src_Color		=	
								Compiler->Lerp(
									Arg_DstSample, 
									Arg_SrcSample, 
									Arg_ClampedLerpBias
									);

		INT	Arg_If_DZ_SZ	=	Compiler->If(
									Arg_DstDepth, 
									Arg_SrcDepth, 
									Arg_Lerp_Dst_Src_Color, 
									Arg_Lerp_Dst_Src_Color, 
									Arg_DstSample
									);

		INT	Arg_If_SZ_DZ_Add_Bias	= Compiler->If(
										Arg_SrcDepth,
										Arg_Sub_DstDepth_Bias,
										Arg_If_DZ_SZ,
										Arg_If_DZ_SZ,
										Arg_SrcSample
										);

		return Arg_If_SZ_DZ_Add_Bias;
	}
	else
	{
		if (Desc.Len() > 0)
		{
			return Compiler->Errorf(TEXT("%s> Missing input texture"), *Desc);
		}
		else
		{
			return Compiler->Errorf(TEXT("TextureSample> Missing input texture"));
		}
	}
}

/**
 *	Get the outputs associated with the expression
 *
 *	@param	Outputs		The array that contains the output expression
 */	
void UMaterialExpressionDepthBiasBlend::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	new(Outputs) FExpressionOutput(1,1,1,1,0);
	new(Outputs) FExpressionOutput(1,1,0,0,0);
	new(Outputs) FExpressionOutput(1,0,1,0,0);
	new(Outputs) FExpressionOutput(1,0,0,1,0);
}

/**
 */	
INT UMaterialExpressionDepthBiasBlend::GetWidth() const
{
	return ME_STD_THUMBNAIL_SZ+(ME_STD_BORDER*2);
}

/**
 */	
FString UMaterialExpressionDepthBiasBlend::GetCaption() const
{
	return TEXT("DepthBiasBlend");
}

/**
 * Recursively gathers all UMaterialExpression objects referenced by this expression.
 * Including self.
 *
 * @param	Expressions		Reference to array of material expressions to add to
 */	
void UMaterialExpressionDepthBiasBlend::GetExpressions(TArray<const UMaterialExpression*>& Expressions) const
{
	Expressions.AddItem(this);
	GET_EXPRESSIONS(Coordinates);
	GET_EXPRESSIONS(Bias);
}
	
/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionDepthBiasBlend::RemoveReferenceTo(UMaterialExpression* Expression)
{
	Super::RemoveReferenceTo(Expression);
	REMOVE_REFERENCE_TO(Bias, Expression);
	REMOVE_REFERENCE_TO(Coordinates, Expression);
}

///////////////////////////////////////////////////////////
// UMaterialExpressionDepthBiasedBlend
///////////////////////////////////////////////////////////
//## BEGIN PROPS MaterialExpressionDepthBiasedBlend
//BITFIELD bNormalize:1;
//FLOAT BiasScale;
//FExpressionInput SourceRGB;
//FExpressionInput SourceA;
//FExpressionInput Bias;
//## END PROPS MaterialExpressionDepthBiasedBlend
/**
 *	Compile the material expression
 *
 *	@param	Compiler	Pointer to the material compiler to use
 *
 *	@return	INT			The compiled code index
 */	
INT UMaterialExpressionDepthBiasedBlend::Compile(FMaterialCompiler* Compiler)
{
	INT	Arg_Constant_0	= Compiler->Constant(0.0f);
	INT	Arg_Constant_1	= Compiler->Constant(1.0f);
	INT	Arg_SourceRGB	= RGB.Expression	? RGB.Expression->Compile(Compiler)		: Compiler->Constant3(0.f, 0.f, 0.f);
	INT	Arg_SourceA		= Alpha.Expression	? Alpha.Expression->Compile(Compiler)	: Compiler->Constant(1.f);

	INT	Arg_SrcSample	=	Arg_SourceRGB;
	INT Arg_DstSample	=	Compiler->DestColor();
	INT	Arg_SrcDepth	=	Compiler->PixelDepth(bNormalize);
	INT	Arg_DstDepth	=	Compiler->DestDepth(bNormalize);

	EMaterialCodeType	Type_SrcSample	= Compiler->GetType(Arg_SrcSample);
	if (Type_SrcSample == MCT_Float4)
	{
		Arg_SrcSample	= Compiler->ComponentMask(Arg_SrcSample, 1, 1, 1, 0);
	}
	EMaterialCodeType	Type_DstSample	= Compiler->GetType(Arg_DstSample);
	if (Type_DstSample == MCT_Float4)
	{
		Arg_DstSample	= Compiler->ComponentMask(Arg_DstSample, 1, 1, 1, 0);
	}

#if defined(_DEPTHBIAS_USE_ONE_MINUS_BIAS_)
	INT	Arg_Bias		=	(Bias.Expression) ? 
								Compiler->Sub(Arg_Constant_1, Bias.Compile(Compiler)) : 
								Arg_Constant_1;
#else	//#if defined(_DEPTHBIAS_USE_ONE_MINUS_BIAS_)
	INT	Arg_Bias		=	(Bias.Expression) ? 
								Compiler->Sub(Arg_Constant_0, Bias.Compile(Compiler)) : 
								Arg_Constant_0;
#endif	//#if defined(_DEPTHBIAS_USE_ONE_MINUS_BIAS_)
	
	INT	Arg_BiasScaleConstant		=	Compiler->Constant(BiasScale);
	INT	Arg_ScaledBias				=	Compiler->Mul(Arg_Bias, Arg_BiasScaleConstant);
	INT	Arg_Sub_DstDepth_Bias		=	Compiler->Sub(Arg_DstDepth, Arg_ScaledBias);
	INT	Arg_Sub_DstDepth_SrcDepth	=	Compiler->Sub(Arg_DstDepth, Arg_SrcDepth);
	INT	Arg_Div_DZ_Sub_SZ_Bias		=	Compiler->Div(Arg_Sub_DstDepth_SrcDepth, Arg_ScaledBias);
	INT Arg_ClampedLerpBias			=	Compiler->Clamp(Arg_Div_DZ_Sub_SZ_Bias, Compiler->Constant(0.0f), Compiler->Constant(1.0f));

	INT	Arg_Lerp_Dst_Src_Color		=	
							Compiler->Lerp(
								Arg_DstSample, 
								Arg_SrcSample, 
								Arg_ClampedLerpBias
								);

	INT	Arg_If_DZ_SZ	=	Compiler->If(
								Arg_DstDepth, 
								Arg_SrcDepth, 
								Arg_Lerp_Dst_Src_Color, 
								Arg_Lerp_Dst_Src_Color, 
								Arg_DstSample
								);

	INT	Arg_If_SZ_DZ_Add_Bias	= Compiler->If(
									Arg_SrcDepth,
									Arg_Sub_DstDepth_Bias,
									Arg_If_DZ_SZ,
									Arg_If_DZ_SZ,
									Arg_SrcSample
									);

	INT	Arg_RGB_Component	= Compiler->ComponentMask(Arg_If_SZ_DZ_Add_Bias, 1, 1, 1, 0);
	EMaterialCodeType	Type_Alpha	= Compiler->GetType(Arg_SourceA);
	if (Type_Alpha == MCT_Float4)
	{
		Arg_SourceA	= Compiler->ComponentMask(Arg_SourceA, 0, 0, 0, 1);
	}

	return Compiler->AppendVector(Arg_RGB_Component, Arg_SourceA);
}

/**
 *	Get the outputs associated with the expression
 *
 *	@param	Outputs		The array that contains the output expression
 */	
void UMaterialExpressionDepthBiasedBlend::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	new(Outputs) FExpressionOutput(1,1,1,1,0);
	new(Outputs) FExpressionOutput(1,1,0,0,0);
	new(Outputs) FExpressionOutput(1,0,1,0,0);
	new(Outputs) FExpressionOutput(1,0,0,1,0);
	new(Outputs) FExpressionOutput(1,0,0,0,1);
}

/**
 */	
INT UMaterialExpressionDepthBiasedBlend::GetWidth() const
{
	return ME_STD_THUMBNAIL_SZ+(ME_STD_BORDER*2);
}

/**
 */	
FString UMaterialExpressionDepthBiasedBlend::GetCaption() const
{
	return TEXT("DepthBiasedBlend");
}

/**
 * Recursively gathers all UMaterialExpression objects referenced by this expression.
 * Including self.
 *
 * @param	Expressions		Reference to array of material expressions to add to
*/	
void UMaterialExpressionDepthBiasedBlend::GetExpressions(TArray<const UMaterialExpression*>& Expressions) const
{
	Expressions.AddItem(this);
	GET_EXPRESSIONS(RGB);
	GET_EXPRESSIONS(Alpha);
	GET_EXPRESSIONS(Bias);
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionDepthBiasedBlend::RemoveReferenceTo(UMaterialExpression* Expression)
{
	Super::RemoveReferenceTo(Expression);
	REMOVE_REFERENCE_TO(Bias, Expression);
	REMOVE_REFERENCE_TO(Alpha, Expression);
	REMOVE_REFERENCE_TO(RGB, Expression);
}

//
//	UMaterialExpressionDesaturation::Compile
//

INT UMaterialExpressionDesaturation::Compile(FMaterialCompiler* Compiler)
{
	if(!Input.Expression)
		return Compiler->Errorf(TEXT("Missing Desaturation input"));

	INT	Color = Input.Compile(Compiler),
		Grey = Compiler->Dot(Color,Compiler->Constant3(LuminanceFactors.R,LuminanceFactors.G,LuminanceFactors.B));

	if(Percent.Expression)
		return Compiler->Lerp(Color,Grey,Percent.Compile(Compiler));
	else
		return Grey;
}

/**
 * Recursively gathers all UMaterialExpression objects referenced by this expression.
 * Including self.
 *
 * @param	Expressions		Reference to array of material expressions to add to
 */	
void UMaterialExpressionDesaturation::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	Expressions.AddItem( this );
	GET_EXPRESSIONS( Input );
	GET_EXPRESSIONS( Percent );
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionDesaturation::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Input, Expression );
	REMOVE_REFERENCE_TO( Percent, Expression );
}

//
//	UMaterialExpressionVectorParameter::Compile
//

INT UMaterialExpressionVectorParameter::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->VectorParameter(ParameterName);
}

//
//	UMaterialExpressionVectorParameter::GetOutputs
//

void UMaterialExpressionVectorParameter::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	new(Outputs) FExpressionOutput(1,1,1,1,0);
	new(Outputs) FExpressionOutput(1,1,0,0,0);
	new(Outputs) FExpressionOutput(1,0,1,0,0);
	new(Outputs) FExpressionOutput(1,0,0,1,0);
	new(Outputs) FExpressionOutput(1,0,0,0,1);
}

//
//	UMaterialExpressionScalarParameter::Compile
//

INT UMaterialExpressionScalarParameter::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->ScalarParameter(ParameterName);
}

//
//	UMaterialExpressionNormalize::Compile
//

INT UMaterialExpressionNormalize::Compile(FMaterialCompiler* Compiler)
{
	if(!VectorInput.Expression)
		return Compiler->Errorf(TEXT("Missing Normalize input"));

	INT	V = VectorInput.Compile(Compiler);

	return Compiler->Div(V,Compiler->SquareRoot(Compiler->Dot(V,V)));
}

/**
 * Recursively gathers all UMaterialExpression objects referenced by this expression.
 * Including self.
 *
 * @param	Expressions		Reference to array of material expressions to add to
 */	
void UMaterialExpressionNormalize::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	Expressions.AddItem( this );
	GET_EXPRESSIONS( VectorInput );
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionNormalize::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( VectorInput, Expression );
}

INT UMaterialExpressionVertexColor::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->VertexColor();
}

void UMaterialExpressionVertexColor::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	new(Outputs) FExpressionOutput(1,1,1,1,0);
	new(Outputs) FExpressionOutput(1,1,0,0,0);
	new(Outputs) FExpressionOutput(1,0,1,0,0);
	new(Outputs) FExpressionOutput(1,0,0,1,0);
	new(Outputs) FExpressionOutput(1,0,0,0,1);
}

FString UMaterialExpressionVertexColor::GetCaption() const
{
	return TEXT("Vertex Color");
}

//
//	MaterialExpressionParticleSubUV
//
INT UMaterialExpressionParticleSubUV::Compile(FMaterialCompiler* Compiler)
{
	if (Texture)
	{
		// Out	 = linear interpolate... using 2 sub-images of the texture
		// A	 = RGB sample texture with TexCoord0
		// B	 = RGB sample texture with TexCoord1
		// Alpha = x component of TexCoord2
		//
		INT TextureIndexA;
		INT TextureCodeIndexA = Compiler->Texture(Texture->GetTexture(), TextureIndexA);
		INT TextureIndexB;
		INT TextureCodeIndexB = Compiler->Texture(Texture->GetTexture(), TextureIndexB);

		return 
			Compiler->Lerp(
			// Lerp_A, 
			Compiler->Add(
				Compiler->Mul(
					Compiler->TextureSample(
						TextureCodeIndexA,  
						Compiler->TextureCoordinate(0)
					),
				Compiler->Constant4(
					Texture->UnpackMax.X - Texture->UnpackMin.X,
					Texture->UnpackMax.Y - Texture->UnpackMin.Y,
					Texture->UnpackMax.Z - Texture->UnpackMin.Z,
					Texture->UnpackMax.W - Texture->UnpackMin.W
					)
				),
				Compiler->Constant4(
					Texture->UnpackMin.X,
					Texture->UnpackMin.Y,
					Texture->UnpackMin.Z,
					Texture->UnpackMin.W
				)
			),
			// Lerp_B, 
			Compiler->Add(
				Compiler->Mul(
					Compiler->TextureSample(
						TextureCodeIndexB, 
						Compiler->TextureCoordinate(1)
					),
				Compiler->Constant4(
					Texture->UnpackMax.X - Texture->UnpackMin.X,
					Texture->UnpackMax.Y - Texture->UnpackMin.Y,
					Texture->UnpackMax.Z - Texture->UnpackMin.Z,
					Texture->UnpackMax.W - Texture->UnpackMin.W
					)
				),
				Compiler->Constant4(
					Texture->UnpackMin.X,
					Texture->UnpackMin.Y,
					Texture->UnpackMin.Z,
					Texture->UnpackMin.W
				)
			),
			// Lerp 'alpha' comes from the 3rd texture set...
			Compiler->ComponentMask(
				Compiler->TextureCoordinate(2), 
				1, 0, 0, 0
			)
		);

	}
	else
	{
		return Compiler->Errorf(TEXT("Missing ParticleSubUV input texture"));
	}
}

void UMaterialExpressionParticleSubUV::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	new(Outputs) FExpressionOutput(1,1,1,1,0);
	new(Outputs) FExpressionOutput(1,1,0,0,0);
	new(Outputs) FExpressionOutput(1,0,1,0,0);
	new(Outputs) FExpressionOutput(1,0,0,1,0);
	new(Outputs) FExpressionOutput(1,0,0,0,1);
}

INT UMaterialExpressionParticleSubUV::GetWidth() const
{
	return ME_STD_THUMBNAIL_SZ+(ME_STD_BORDER*2);
}

FString UMaterialExpressionParticleSubUV::GetCaption() const
{
	return TEXT("Particle SubUV");
}

//
//	UMaterialExpressionMeshSubUV
//
INT UMaterialExpressionMeshSubUV::Compile(FMaterialCompiler* Compiler)
{
	if (Texture)
	{
		//                         | Mul |
		//	VectorParam(Scale)---->|A    |----\           | Add |   |Sample|
		//	TextureCoord(0)------->|B    |     \--------->|A    |-->|Coord |
		//	VectorParam(SubUVMeshOffset)-->CompMask(XY)-->|B    |   |      |

		// Out	 = sub-UV sample of the input texture
		//
		INT TextureIndex;
		INT TextureCodeIndex	= Compiler->Texture(Texture->GetTexture(), TextureIndex);
		INT	CoordinateIndex		= 0;
		if (Coordinates.Expression)
		{
			UMaterialExpressionTextureCoordinate* pkTexCoord = 
				CastChecked<UMaterialExpressionTextureCoordinate>(Coordinates.Expression);
			CoordinateIndex = pkTexCoord->CoordinateIndex;
		}

		return Compiler->Add(
				Compiler->Mul(
					Compiler->TextureSample(
						TextureCodeIndex,
						Compiler->Add(
							Compiler->Mul(
								Coordinates.Expression ? Coordinates.Compile(Compiler) : Compiler->TextureCoordinate(0),
								Compiler->ComponentMask(
									Compiler->TextureScaleParameter(),
									1, 
									1, 
									0, 
									0
									)
								),
							Compiler->ComponentMask(
								Compiler->TextureOffsetParameter(), 
								1, 
								1, 
								0, 
								0
								)
							)
						),
					Compiler->Constant4(
						Texture->UnpackMax.X - Texture->UnpackMin.X,
						Texture->UnpackMax.Y - Texture->UnpackMin.Y,
						Texture->UnpackMax.Z - Texture->UnpackMin.Z,
						Texture->UnpackMax.W - Texture->UnpackMin.W
						)
					),
				Compiler->Constant4(
					Texture->UnpackMin.X,
					Texture->UnpackMin.Y,
					Texture->UnpackMin.Z,
					Texture->UnpackMin.W
					)
				);
	}
	else
	{
		return Compiler->Errorf(TEXT("%s missing texture"), *GetCaption());
	}
}

void UMaterialExpressionMeshSubUV::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	new(Outputs) FExpressionOutput(1,1,1,1,0);
	new(Outputs) FExpressionOutput(1,1,0,0,0);
	new(Outputs) FExpressionOutput(1,0,1,0,0);
	new(Outputs) FExpressionOutput(1,0,0,1,0);
	new(Outputs) FExpressionOutput(1,0,0,0,1);
}

INT UMaterialExpressionMeshSubUV::GetWidth() const
{
	return ME_STD_THUMBNAIL_SZ+(ME_STD_BORDER*2);
}

FString UMaterialExpressionMeshSubUV::GetCaption() const
{
	return TEXT("Mesh SubUV");
}

INT UMaterialExpressionLightVector::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->LightVector();
}

FString UMaterialExpressionLightVector::GetCaption() const
{
	return TEXT("Light Vector");
}

INT UMaterialExpressionScreenPosition::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->ScreenPosition( ScreenAlign );
}

FString UMaterialExpressionScreenPosition::GetCaption() const
{
	return TEXT("Screen Position");
}

INT UMaterialExpressionSquareRoot::Compile(FMaterialCompiler* Compiler)
{
	if(!Input.Expression)
	{
		return Compiler->Errorf(TEXT("Missing square root input"));
	}
	return Compiler->SquareRoot(Input.Compile(Compiler));
}

FString UMaterialExpressionSquareRoot::GetCaption() const
{
	return TEXT("Square root");
}

void UMaterialExpressionSquareRoot::GetExpressions(TArray<const UMaterialExpression*>& Expressions) const
{
	Expressions.AddItem(this);
	GET_EXPRESSIONS(Input);
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionSquareRoot::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Input, Expression );
}

///////////////////////////////////////////////////////////////////////////////
// UMaterialExpressionPixelDepth
///////////////////////////////////////////////////////////////////////////////
/**
 *	Compile the expression
 *
 *	@param	Compiler	The compiler to utilize
 *
 *	@return				The index of the resulting code chunk.
 *						INDEX_NONE if error.
 */
INT UMaterialExpressionPixelDepth::Compile(FMaterialCompiler* Compiler)
{
	// resulting index to compiled code chunk
	// add the code chunk for the scene depth sample        
	INT Result = Compiler->PixelDepth(bNormalize);
	return Result;
}

/**
 *	Retrieve the outputs this expression supplies.
 *
 *	@param	Outputs		The array to insert the available outputs in.
 */
void UMaterialExpressionPixelDepth::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	// Depth
	new(Outputs) FExpressionOutput(1,1,0,0,0);
}

/**
 *	Get the caption to display on this material expression
 *
 *	@return			An FString containing the display caption
 */
FString UMaterialExpressionPixelDepth::GetCaption() const
{
	return TEXT("PixelDepth");
}

//
INT UMaterialExpressionPower::Compile(FMaterialCompiler* Compiler)
{
	if(!Base.Expression)
	{
		return Compiler->Errorf(TEXT("Missing Power Base input"));
	}
	if(!Exponent.Expression)
	{
		return Compiler->Errorf(TEXT("Missing Power Exponent input"));
	}

	INT Arg1 = Base.Compile(Compiler);
	INT Arg2 = Exponent.Compile(Compiler);
	return Compiler->Power(
		Arg1,
		Arg2
		);
}

FString UMaterialExpressionPower::GetCaption() const
{
	return TEXT("Power");
}

void UMaterialExpressionPower::GetExpressions(TArray<const UMaterialExpression*>& Expressions) const
{
	Expressions.AddItem(this);
	GET_EXPRESSIONS(Base);
	GET_EXPRESSIONS(Exponent);
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionPower::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Base, Expression );
	REMOVE_REFERENCE_TO( Exponent, Expression );
}

INT UMaterialExpressionIf::Compile(FMaterialCompiler* Compiler)
{
	if(!A.Expression)
	{
		return Compiler->Errorf(TEXT("Missing If A input"));
	}
	if(!B.Expression)
	{
		return Compiler->Errorf(TEXT("Missing If B input"));
	}
	if(!AGreaterThanB.Expression)
	{
		return Compiler->Errorf(TEXT("Missing If AGreaterThanB input"));
	}
	if(!AEqualsB.Expression)
	{
		return Compiler->Errorf(TEXT("Missing If AEqualsB input"));
	}
	if(!ALessThanB.Expression)
	{
		return Compiler->Errorf(TEXT("Missing If ALessThanB input"));
	}

	INT CompiledA = A.Compile(Compiler);
	INT CompiledB = B.Compile(Compiler);

	if(Compiler->GetType(CompiledA) != MCT_Float)
	{
		return Compiler->Errorf(TEXT("If input A must be of type float."));
	}

	if(Compiler->GetType(CompiledB) != MCT_Float)
	{
		return Compiler->Errorf(TEXT("If input B must be of type float."));
	}

	INT Arg3 = AGreaterThanB.Compile(Compiler);
	INT Arg4 = AEqualsB.Compile(Compiler);
	INT Arg5 = ALessThanB.Compile(Compiler);

	return Compiler->If(CompiledA,CompiledB,Arg3, Arg4, Arg5);
}

FString UMaterialExpressionIf::GetCaption() const
{
	return TEXT("If");
}

void UMaterialExpressionIf::GetExpressions(TArray<const UMaterialExpression*>& Expressions) const
{
	Expressions.AddItem(this);
	GET_EXPRESSIONS(A);
	GET_EXPRESSIONS(B);
	GET_EXPRESSIONS(AGreaterThanB);
	GET_EXPRESSIONS(AEqualsB);
	GET_EXPRESSIONS(ALessThanB);
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionIf::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( A, Expression );
	REMOVE_REFERENCE_TO( B, Expression );
	REMOVE_REFERENCE_TO( AGreaterThanB, Expression );
	REMOVE_REFERENCE_TO( AEqualsB, Expression );
	REMOVE_REFERENCE_TO( ALessThanB, Expression );
}

INT UMaterialExpressionOneMinus::Compile(FMaterialCompiler* Compiler)
{
	if(!Input.Expression)
	{
		return Compiler->Errorf(TEXT("Missing 1-x input"));
	}
	return Compiler->Sub(Compiler->Constant(1.0f),Input.Compile(Compiler));
}

FString UMaterialExpressionOneMinus::GetCaption() const
{
	return TEXT("1-x");
}

void UMaterialExpressionOneMinus::GetExpressions(TArray<const UMaterialExpression*>& Expressions) const
{
	Expressions.AddItem(this);
	GET_EXPRESSIONS(Input);
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionOneMinus::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Input, Expression );
}

/**
 * Creates the new shader code chunk needed for the Abs expression
 *
 * @param	Compiler - Material compiler that knows how to handle this expression
 * @return	Index to the new FMaterialCompiler::CodeChunk entry for this expression
 */	
INT UMaterialExpressionAbs::Compile( FMaterialCompiler* Compiler )
{
	INT Result=INDEX_NONE;

	if( !Input.Expression )
	{
		// an input expression must exist
		Result = Compiler->Errorf( TEXT("Missing Abs input") );
	}
	else
	{
		// evaluate the input expression first and use that as
		// the parameter for the Abs expression
		Result = Compiler->Abs( Input.Compile(Compiler) );
	}

	return Result;
}

/**
 * Textual description for this material expression
 *
 * @return	Caption text
 */	
FString UMaterialExpressionAbs::GetCaption() const
{
	return TEXT("Abs");
}

/**
 * Recursively gathers all UMaterialExpression objects referenced by this expression.
 * Including self.
 *
 * @param	Expressions	- Reference to array of material expressions to add to
 */	
void UMaterialExpressionAbs::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	Expressions.AddItem( this );
	GET_EXPRESSIONS( Input );
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionAbs::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Input, Expression );
}

///////////////////////////////////////////////////////////////////////////////
// UMaterialExpressionSceneTexture
///////////////////////////////////////////////////////////////////////////////

/**
* Create the shader code chunk for sampling the scene's lighting texture
*
* @param	Compiler - Material compiler that knows how to handle this expression
* @return	Index to the new FMaterialCompiler::CodeChunk entry for this expression
*/	
INT UMaterialExpressionSceneTexture::Compile( FMaterialCompiler* Compiler )
{
	// resulting index to compiled code chunk
	INT Result=INDEX_NONE;
	// resulting index to compiled code for the tex coordinates if available
	INT CoordIdx = INDEX_NONE;
	// if there are valid texture coordinate inputs then compile them
	if( Coordinates.Expression )
	{
		CoordIdx = Coordinates.Compile(Compiler);
	}
    // add the code chunk for the scene texture sample        
	Result = Compiler->SceneTextureSample( SceneTextureType, CoordIdx );
	return Result;
}

/**
* Fill in the array of valid outputs for this material expression
*
* @param	Outputs - array of this expression's valid outputs
*/	
void UMaterialExpressionSceneTexture::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	// RGB
	new(Outputs) FExpressionOutput(1,1,1,1,0);
	// R
	new(Outputs) FExpressionOutput(1,1,0,0,0);
	// G
	new(Outputs) FExpressionOutput(1,0,1,0,0);
	// B
	new(Outputs) FExpressionOutput(1,0,0,1,0);
	// A
	new(Outputs) FExpressionOutput(1,0,0,0,1);
}

/**
* Recursively gathers all UMaterialExpression objects referenced by this expression.
* Including self.
*
* @param	Expressions - array of material expressions to add to
*/	
void UMaterialExpressionSceneTexture::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	Expressions.AddItem( this );
	// user texture coordinates
	GET_EXPRESSIONS( Coordinates );
}

/**
* Removes references to the passed in expression.
*
* @param	Expression - expression to remove reference to
*/
void UMaterialExpressionSceneTexture::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Coordinates, Expression );
}

/**
* Text description of this expression
*/
FString UMaterialExpressionSceneTexture::GetCaption() const
{
	return TEXT("Scene Texture Sample");
}

///////////////////////////////////////////////////////////////////////////////
// UMaterialExpressionSceneDepth
///////////////////////////////////////////////////////////////////////////////

/**
* Create the shader code chunk for sampling the scene's depth
*
* @param	Compiler - Material compiler that knows how to handle this expression
* @return	Index to the new FMaterialCompiler::CodeChunk entry for this expression
*/	
INT UMaterialExpressionSceneDepth::Compile( FMaterialCompiler* Compiler )
{
	// resulting index to compiled code chunk
	INT Result=INDEX_NONE;
	// resulting index to compiled code for the tex coordinates if available
	INT CoordIdx = INDEX_NONE;
	// if there are valid texture coordinate inputs then compile them
	if( Coordinates.Expression )
	{
		CoordIdx = Coordinates.Compile(Compiler);
	}
	// add the code chunk for the scene depth sample        
	Result = Compiler->SceneTextureDepth( bNormalize, CoordIdx );
	return Result;
}

/**
* Fill in the array of valid outputs for this material expression
*
* @param	Outputs - array of this expression's valid outputs
*/	
void UMaterialExpressionSceneDepth::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	new(Outputs) FExpressionOutput(1,1,0,0,0);
}

/**
* Recursively gathers all UMaterialExpression objects referenced by this expression.
* Including self.
*
* @param	Expressions - array of material expressions to add to
*/	
void UMaterialExpressionSceneDepth::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	Expressions.AddItem( this );
	// user texture coordinates
	GET_EXPRESSIONS( Coordinates );
}

/**
* Removes references to the passed in expression.
*
* @param	Expression - expression to remove reference to
*/
void UMaterialExpressionSceneDepth::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Coordinates, Expression );
}

/**
* Text description of this expression
*/
FString UMaterialExpressionSceneDepth::GetCaption() const
{
	return TEXT("Scene Depth");
}

///////////////////////////////////////////////////////////////////////////////
// UMaterialExpressionTransform
///////////////////////////////////////////////////////////////////////////////

/**
* Create the shader code chunk for transforming an input vector to a new coordinate space
*
* @param	Compiler - Material compiler that knows how to handle this expression
* @return	Index to the new FMaterialCompiler::CodeChunk entry for this expression
*/	
INT UMaterialExpressionTransform::Compile(FMaterialCompiler* Compiler)
{
	INT Result=INDEX_NONE;

	if( !Input.Expression )
	{
		Result = Compiler->Errorf(TEXT("Missing Transform input vector"));
	}
	else
	{
		INT VecInputIdx = Input.Compile(Compiler);
		Result = Compiler->TransformVector( TransformType, VecInputIdx );
	}

	return Result;
}

/**
* Text description of this expression
*/
FString UMaterialExpressionTransform::GetCaption() const
{
	return TEXT("Coordinate Transform");
}

/**
* Recursively gathers all UMaterialExpression objects referenced by this expression.
* Including self.
*
* @param	Expressions		Reference to array of material expressions to add to
*/	
void UMaterialExpressionTransform::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	Expressions.AddItem( this );
	GET_EXPRESSIONS( Input );
}
	
/**
* Removes references to the passed in expression.
*
* @param	Expression		Expression to remove reference to
*/
void UMaterialExpressionTransform::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Input, Expression );
}

///////////////////////////////////////////////////////////////////////////////
// UMaterialExpressionComment
///////////////////////////////////////////////////////////////////////////////

/**
 * Text description of this expression.
 */
FString UMaterialExpressionComment::GetCaption() const
{
	return TEXT("Comment");
}

///////////////////////////////////////////////////////////////////////////////
// MaterialExpressionCompound
///////////////////////////////////////////////////////////////////////////////

/**
 * Text description of this expression.
 */
FString UMaterialExpressionCompound::GetCaption() const
{
	return Caption.Len() > 0 ? *Caption : TEXT("Compound Expression");
}

/**
 * Recursively gathers all UMaterialExpression objects referenced by this expression.
 * Including self.
 *
 * @param	Expressions		Reference to array of material expressions to add to
 */	
void UMaterialExpressionCompound::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	Super::GetExpressions( Expressions );
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionCompound::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	MaterialExpressions.RemoveItem( Expression );
}

/**
 * Recursively gathers all UMaterialExpression objects referenced by this expression.
 * Including self.
 *
 * @param	Expressions		Reference to array of material expressions to add to
 */	
void UMaterialExpressionFresnel::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	Expressions.AddItem( this );
	GET_EXPRESSIONS( Normal );
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionFresnel::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Normal, Expression );
}

/**
 * Spits out the proper shader code for the approximate Fresnel term
 *
 * @param Compiler the compiler compiling this expression
 */
INT UMaterialExpressionFresnel::Compile(FMaterialCompiler* Compiler)
{
	// pow(1 - max(0,Normal dot Camera),Exponent)
	//
	INT NormalArg = Normal.Expression ? Normal.Compile(Compiler) : Compiler->Constant3(0.f,0.f,1.f);
	INT DotArg = Compiler->Dot(NormalArg,Compiler->CameraVector());
	INT MaxArg = Compiler->Max(Compiler->Constant(0.f),DotArg);
	INT MinusArg = Compiler->Sub(Compiler->Constant(1.f),MaxArg);
	INT PowArg = Compiler->Power(MinusArg,Compiler->Constant(Exponent));
	return PowArg;
}
