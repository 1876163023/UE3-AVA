/*=============================================================================
	MaterialShared.cpp: Shared material implementation.
	Copyright 2004-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EnvCubePrivate.h"
#include "EngineMaterialClasses.h"
#include "EnginePhysicsClasses.h"

EShaderPlatform GShaderCompilePlatform = SP_AllPlatforms;

IMPLEMENT_CLASS(UMaterialInstance);
IMPLEMENT_CLASS(AMaterialInstanceActor);

/**
 * Gathers the textures used to render the material instance.
 * @param OutTextures	Upon return contains the textures used to render the material instance.
 * @param bOnlyAddOnce	Whether to add textures that are sampled multiple times uniquely or not
 */
void UMaterialInstance::GetTextures(TArray<UTexture*>& OutTextures, UBOOL bOnlyAddOnce )
{
	UMaterial* Material = GetMaterial();
	if(!Material)
	{
		// If the material instance has no material, use the default material.
		GEngine->DefaultMaterial->GetTextures(OutTextures);
		return;
	}

	check(Material->MaterialResource);

	// Iterate over both the 2D textures and cube texture expressions.
	const TArray<TRefCountPtr<FMaterialUniformExpression> >* ExpressionsByType[2] =
	{
		&Material->MaterialResource->GetUniform2DTextureExpressions(),
		&Material->MaterialResource->GetUniformCubeTextureExpressions()
	};
	for(INT TypeIndex = 0;TypeIndex < ARRAY_COUNT(ExpressionsByType);TypeIndex++)
	{
		const TArray<TRefCountPtr<FMaterialUniformExpression> >& Expressions = *ExpressionsByType[TypeIndex];

		// Iterate over each of the material's texture expressions.
		for(INT ExpressionIndex = 0;ExpressionIndex < Expressions.Num();ExpressionIndex++)
		{
			FMaterialUniformExpression* Expression = Expressions(ExpressionIndex);

			// Evaluate the expression in terms of this material instance.
			UTexture* Texture = NULL;
			Expression->GetGameThreadTextureValue(this,&Texture);

			// Add the expression's value to the output array.
			if( bOnlyAddOnce )
			{
				OutTextures.AddUniqueItem(Texture);
			}
			else
			{
				OutTextures.AddItem(Texture);
			}
		}
	}
}

void UMaterialInstance::GetTexturesNeededForPlatform(TArray<UTexture*>& OutTextures,UBOOL bOnlyAddOnce)
{
	UMaterialInstanceConstant* MIC = NULL;
	UMaterial* Material = NULL;

	UMaterialInstance* MI = this;
	for (;;)
	{
		Material = Cast<UMaterial>( MI );

		/// found root material
		if (Material != NULL)
			break;

		/// Is MIC?
		MIC = Cast<UMaterialInstanceConstant>( MI );
		if (!MIC)
			break;

		MI = MIC->Parent;
	}		

	if (Material == NULL)
		return;

	struct FTextureCollectingMaterialCompiler : public FMaterialCompiler
	{
		UMaterial* Material;
		TArray<UTexture*>& OutTextures;
		TArray<UMaterialExpression*> ExpressionStack;
		UMaterialInstanceConstant* MIC;
		UBOOL bOnlyAddOnce;

		// Contstructor
		FTextureCollectingMaterialCompiler( UMaterial* InMaterial, UMaterialInstanceConstant* MIC, TArray<UTexture*>& OutTextures,UBOOL bOnlyAddOnce )
			:	Material( InMaterial ), MIC( MIC ), OutTextures( OutTextures ),bOnlyAddOnce(bOnlyAddOnce)
		{}

		virtual INT Error(const TCHAR* Text) 
		{
			return 0; 
		}			

		virtual INT CallExpression(UMaterialExpression* MaterialExpression,FMaterialCompiler* InCompiler) 
		{ 
			if(ExpressionStack.FindItemIndex(MaterialExpression) == INDEX_NONE)
			{					
				ExpressionStack.Push(MaterialExpression);
				MaterialExpression->Compile(this);
				check(ExpressionStack.Pop() == MaterialExpression);
			}				

			return 0; 
		}

		virtual EMaterialValueType GetType(INT Code) { return MCT_Unknown; }
		virtual INT ForceCast(INT Code,EMaterialValueType DestType) { return 0; }

		virtual INT VectorParameter(FName ParameterName,const FLinearColor& DefaultValue) { return 0; }
		virtual INT ScalarParameter(FName ParameterName,FLOAT DefaultValue) { return 0; }

		virtual INT Constant(FLOAT X) { return 0; }
		virtual INT Constant2(FLOAT X,FLOAT Y) { return 0; }
		virtual INT Constant3(FLOAT X,FLOAT Y,FLOAT Z) { return 0; }
		virtual INT Constant4(FLOAT X,FLOAT Y,FLOAT Z,FLOAT W) { return 0; }

		virtual INT SceneTime() { return 0; }
		virtual INT PeriodicHint(INT PeriodicCode) { return PeriodicCode; }

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

		virtual INT Texture(UTexture* Texture) 
		{
			if (Texture)
			{
				if (bOnlyAddOnce) 
					OutTextures.AddUniqueItem(Texture);
				else
					OutTextures.AddItem(Texture);
			}
			
			return 0; 
		}

		virtual INT TextureParameter(FName ParameterName,UTexture* DefaultTexture) 
		{
			for (UMaterialInstanceConstant* p = MIC; p != NULL; p = Cast<UMaterialInstanceConstant>( p->Parent ))
			{
				for (INT i=0; i<p->NewTextureParameterValues.Num(); ++i)
				{
					if (p->NewTextureParameterValues(i).ParameterName == ParameterName)
					{
						Texture( p->NewTextureParameterValues(i).ParameterValue );
						break;
					}
				}
			}

			Texture( DefaultTexture );

			return 0; 
		}

		virtual	INT SceneTextureSample( BYTE TexType, INT CoordinateIdx) { return 0; }
		virtual	INT SceneTextureDepth( UBOOL bNormalize, INT CoordinateIdx) { return 0; }
		virtual	INT EnvCube( INT CoordinateIdx) { return 0; }
		virtual	INT PixelDepth(UBOOL bNormalize) { return 0; }
		virtual	INT DestColor() { return 0; }
		virtual	INT DestDepth(UBOOL bNormalize) { return 0; }
		virtual INT DepthBiasedAlpha( INT SrcAlphaIdx, INT BiasIdx, INT BiasScaleIdx ) { return 0; }
		virtual INT DepthBiasedBlend( INT SrcColorIdx, INT BiasIdx, INT BiasScaleIdx ) { return 0; }

		virtual INT VertexColor() { return 0; }

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
		virtual INT TransformVector(BYTE CoordType,INT A) { return 0; }

		virtual INT GameTime() { return 0; }
		virtual INT RealTime() { return 0; }
		virtual INT FlipBookOffset(UTexture* InFlipBook) { return 0; }

		virtual INT LensFlareIntesity() { return 0; }			
		virtual INT LensFlareRadialDistance() { return 0; }			
		virtual INT LensFlareRayDistance() { return 0; }			
		virtual INT LensFlareSourceDistance() { return 0; }			
	};

	FMaterial::AvaMaterialInfo OldInfo = Material->MaterialResource->MaterialInfos[GRHIShaderPlatform];

	for (INT MaterialPropertyIndex=0; MaterialPropertyIndex<MP_MAX; ++MaterialPropertyIndex)
	{
		FTextureCollectingMaterialCompiler Compiler( Material, Cast<UMaterialInstanceConstant>( this ), OutTextures, bOnlyAddOnce );

		Material->MaterialResource->CompileProperty( (EMaterialProperty)MaterialPropertyIndex, &Compiler );		
	}

	Material->MaterialResource->MaterialInfos[GRHIShaderPlatform] = OldInfo;
}

UBOOL UMaterialInstance::UseWithSkeletalMesh()
{
	UMaterial* Material = GetMaterial();
	if(Material)
	{
		return Material->UseWithSkeletalMesh();
	}
	else
	{
		return FALSE;
	}
}

UBOOL UMaterialInstance::UseWithParticleSystem()
{
	UMaterial* Material = GetMaterial();
	if(Material)
	{
		return Material->UseWithParticleSystem();
	}
	else
	{
		return FALSE;
	}
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

void UMaterialInstance::execGetMaterial _ParamList_ScriptFunctionImpl // 20080221 dEAthcURe|CD // ( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	*(UMaterial**) Result = GetMaterial();
}


void UMaterialInstance::execGetPhysicalMaterial _ParamList_ScriptFunctionImpl // 20080221 dEAthcURe|CD // ( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	*(UPhysicalMaterial**) Result = GetPhysicalMaterial();
}

UBOOL UMaterialInstance::GetVectorParameterValue(FName ParameterName, FLinearColor& OutValue)
{
	return FALSE;
}

UBOOL UMaterialInstance::GetScalarParameterValue(FName ParameterName, FLOAT& OutValue)
{
	return FALSE;
}

UBOOL UMaterialInstance::GetTextureParameterValue(FName ParameterName, UTexture*& OutValue)
{
	return FALSE;
}

TLinkedList<FMaterialUniformExpressionType*>*& FMaterialUniformExpressionType::GetTypeList()
{
	static TLinkedList<FMaterialUniformExpressionType*>* TypeList = NULL;
	return TypeList;
}

FMaterialUniformExpressionType::FMaterialUniformExpressionType(
	const TCHAR* InName,
	SerializationConstructorType InSerializationConstructor
	):
	Name(InName),
	SerializationConstructor(InSerializationConstructor)
{
	(new TLinkedList<FMaterialUniformExpressionType*>(this))->Link(GetTypeList());
}

FArchive& operator<<(FArchive& Ar,FMaterialUniformExpression*& Ref)
{
	// Serialize the expression type.
	if(Ar.IsSaving())
	{
		// Write the type name.
		check(Ref);
		FName TypeName(Ref->GetType()->Name);
		Ar << TypeName;
	}
	else if(Ar.IsLoading())
	{
		// Read the type name.
		FName TypeName;
		Ar << TypeName;

		// Find the expression type with a matching name.
		FMaterialUniformExpressionType* Type = NULL;
		for(TLinkedList<FMaterialUniformExpressionType*>::TIterator TypeIt(FMaterialUniformExpressionType::GetTypeList());TypeIt;TypeIt.Next())
		{
			if(!appStrcmp(TypeIt->Name,*TypeName.ToString()))
			{
				Type = *TypeIt;
				break;
			}
		}
		check(Type);

		// Construct a new instance of the expression type.
		Ref = (*Type->SerializationConstructor)();
	}

	// Serialize the expression.
	Ref->Serialize(Ar);

	return Ar;
}

INT FMaterialCompiler::Errorf(const TCHAR* Format,...)
{
	TCHAR	ErrorText[2048];
	GET_VARARGS(ErrorText,ARRAY_COUNT(ErrorText),Format,Format);
	return Error(ErrorText);
}

//
//	FExpressionInput::Compile
//

INT FExpressionInput::Compile(FMaterialCompiler* Compiler)
{
	if(Expression)
	{
		if(Mask)
		{
			INT ExpressionResult = Compiler->CallExpression(Expression,Compiler);
			if(ExpressionResult != INDEX_NONE)
			{
				return Compiler->ComponentMask(
					ExpressionResult,
					MaskR,MaskG,MaskB,MaskA
					);
			}
			else
			{
				return INDEX_NONE;
			}
		}
		else
		{
			return Compiler->CallExpression(Expression,Compiler);
		}
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
	{
		return FExpressionInput::Compile(Compiler);
	}
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
	{
		return Compiler->Constant(Constant);
	}
	else if(Expression)
	{
		return FExpressionInput::Compile(Compiler);
	}
	else
	{
		return Compiler->Constant(Default);
	}
}

//
//	FVectorMaterialInput::Compile
//

INT FVectorMaterialInput::Compile(FMaterialCompiler* Compiler,const FVector& Default)
{
	if(UseConstant)
	{
		return Compiler->Constant3(Constant.X,Constant.Y,Constant.Z);
	}
	else if(Expression)
	{
		return FExpressionInput::Compile(Compiler);
	}
	else
	{
		return Compiler->Constant3(Default.X,Default.Y,Default.Z);
	}
}

//
//	FVector2MaterialInput::Compile
//

INT FVector2MaterialInput::Compile(FMaterialCompiler* Compiler,const FVector2D& Default)
{
	if(UseConstant)
	{
		return Compiler->Constant2(Constant.X,Constant.Y);
	}
	else if(Expression)
	{
		return FExpressionInput::Compile(Compiler);
	}
	else
	{
		return Compiler->Constant2(Default.X,Default.Y);
	}
}

//<@ ava specific ; 2007. 1. 22 changmin
INT FColorMaterialInput::Compile(FMaterialCompiler *Compiler,const FColor &Default, UBOOL *bHasValidExpression )
{
	if(UseConstant)	//? 사용되는 곳이 없는 것 같음.. 처리 안함..
	{
		FLinearColor	LinearColor(Constant);
		return Compiler->Constant3(LinearColor.R,LinearColor.G,LinearColor.B);
	}
	else if(Expression)
	{
		INT CodeIndex = FExpressionInput::Compile(Compiler);
		if( CodeIndex == INDEX_NONE )
		{
			*bHasValidExpression = FALSE;
			FLinearColor	LinearColor(Default);
			return Compiler->Constant3(LinearColor.R,LinearColor.G,LinearColor.B);
		}
		else
		{
			*bHasValidExpression = TRUE;
			return CodeIndex;
		}
	}
	else
	{
		*bHasValidExpression = FALSE;
		FLinearColor	LinearColor(Default);
		return Compiler->Constant3(LinearColor.R,LinearColor.G,LinearColor.B);
	}
}
INT FScalarMaterialInput::Compile(FMaterialCompiler* Compiler,FLOAT Default, UBOOL *bHasValidExpression )
{
	if(UseConstant)	// ? 사용되는 곳이 없는 것 같음.. 처리 안함..
	{
		return Compiler->Constant(Constant);
	}
	else if(Expression)
	{
		INT CodeIndex = FExpressionInput::Compile(Compiler);
		if( CodeIndex == INDEX_NONE )
		{
			*bHasValidExpression = FALSE;
			return Compiler->Constant(Default);
		}
		else
		{
			*bHasValidExpression = TRUE;
			return CodeIndex;
		}
	}
	else
	{
		*bHasValidExpression = FALSE;
		return Compiler->Constant(Default);
	}
}
INT FVectorMaterialInput::Compile(FMaterialCompiler* Compiler,const FVector& Default, UBOOL *bHasValidExpression )
{
	if(UseConstant)	//? 사용되는 곳이 없는 것 같음.. 처리 안함..
	{
		return Compiler->Constant3(Constant.X,Constant.Y,Constant.Z);
	}
	else if(Expression)
	{
		INT CodeIndex = FExpressionInput::Compile(Compiler);
		if( CodeIndex == INDEX_NONE )
		{
			*bHasValidExpression = FALSE;
			return Compiler->Constant3(Default.X,Default.Y,Default.Z);
		}
		else
		{
			*bHasValidExpression = TRUE;
			return CodeIndex;
		}
	}
	else
	{
		*bHasValidExpression = FALSE;
		return Compiler->Constant3(Default.X,Default.Y,Default.Z);
	}
}
INT FVector2MaterialInput::Compile(FMaterialCompiler* Compiler,const FVector2D& Default, UBOOL *bHasValidExpression )
{
	if(UseConstant)	//? 사용되는 곳이 없는 것 같음.. 처리안함
	{
		return Compiler->Constant2(Constant.X,Constant.Y);
	}
	else if(Expression)
	{
		INT CodeIndex = FExpressionInput::Compile(Compiler);
		if( CodeIndex == INDEX_NONE )
		{
			*bHasValidExpression = FALSE;
			return Compiler->Constant2(Default.X, Default.Y);
		}
		else
		{
			*bHasValidExpression = TRUE;
			return CodeIndex;
		}
	}
	else
	{
		*bHasValidExpression = FALSE;
		return Compiler->Constant2(Default.X,Default.Y);
	}
}
//>@ ava


EMaterialValueType GetMaterialPropertyType(EMaterialProperty Property)
{
	switch(Property)
	{
	case MP_EmissiveColor: return MCT_Float3;
	case MP_Opacity: return MCT_Float;
	case MP_OpacityMask: return MCT_Float;
	case MP_Distortion: return MCT_Float2;
	case MP_TwoSidedLightingMask: return MCT_Float3;
	case MP_DiffuseColor: return MCT_Float3;
	case MP_SpecularColor: return MCT_Float3;
	case MP_SpecularPower: return MCT_Float;
	case MP_Normal: return MCT_Float3;
	case MP_CustomLighting: return MCT_Float3;

	//!{ 2006-08-14	허 창 민
	// 이것을 추가하지 않아서, terrain material이 compile 되지 않았다.
	case MP_AmbientMask: return MCT_Float1;
	//!} 2006-08-14	허 창 민
	};
	return MCT_Unknown;
}

/**
 * Null any material expression references for this material
 */
void FMaterial::RemoveExpressions()
{
	//<@ ava specific ; 2007. 1. 24 changmin
	GetTextureDependencyLengthMap().Empty();
	//TextureDependencyLengthMap.Empty();
	//>@ ava
}

//<@ ava specific ; 2007. 4. 3 changmin
UBOOL UseSameMaterialWithCurrentPlatform( INT Platform )
{
	return (Platform == GRHIShaderPlatform)
		|| ( Platform == SP_PCD3D_SM2_POOR	&& GRHIShaderPlatform == SP_PCD3D_SM2 )
		|| ( Platform == SP_PCD3D_SM2		&& GRHIShaderPlatform == SP_PCD3D_SM2_POOR );
}
//>@ ava

void FMaterial::Serialize(FArchive& Ar)
{
	// 우리가 원하는 platform 인 SP_PCD3D가 0번이므로 이렇게 합니다..
	// 예전버전을 loading할 때는 SP_PD3D만 data로 있으므로.. 그것만 loading하고,
	// 그 후에는 모든 버전을 다 저장합니다.
	const INT NumPlatforms = (Ar.LicenseeVer() < VER_AVA_ADD_MATERIAL_FALLBACK && Ar.IsLoading() ) ? SP_PCD3D + 1: 
		(Ar.LicenseeVer() < VER_AVA_ADD_POOR_SM2 && Ar.IsLoading() ) ? SP_PCD3D_SM2 + 1: 
		SP_NumPlatforms;	

	for( INT PlatformIndex = 0; PlatformIndex < NumPlatforms; ++PlatformIndex )
	{
		EShaderPlatform TargetPlatform = (EShaderPlatform)PlatformIndex;		

		FMaterial::AvaMaterialInfo& MaterialInfo = MaterialInfos[TargetPlatform];

		if(Ar.Ver() >= VER_FMATERIAL_COMPILATION_ERRORS)
		{
			Ar << MaterialInfo.CompileErrors;			
		}

		if(Ar.Ver() >= VER_MATERIAL_TEXTUREDEPENDENCYLENGTH)
		{
			Ar << MaterialInfo.TextureDependencyLengthMap;
			Ar << MaterialInfo.MaxTextureDependencyLength;						
		}

		Ar << Id;
		Ar << MaterialInfo.NumUserTexCoords;		

		//<@ ava speicifc ; 2007. 1. 24 changmin
		// game 에서는 save가 불리지 않는다는 전제가 있습니다~
		check( (Ar.IsSaving() && GIsGame) != TRUE );
		const UBOOL bSkipLoadReferencedObject = !GIsEditor && GIsGame && !UseSameMaterialWithCurrentPlatform(PlatformIndex);
		if( bSkipLoadReferencedObject )
		{
			// SetSkipObjectRef는 reference되는 object를 loading하지 않기 위해,
			// 원래 data의 UObject Reference를 NULL 로 만듭니다.
			
			Ar.SetSkipObjectRef( TRUE );
			//debugf( NAME_Log, TEXT("Serialize Material : %s"), *GetFriendlyName() );
		}
		//>@ ava

		Ar << MaterialInfo.UniformVectorExpressions;
		Ar << MaterialInfo.UniformScalarExpressions;
		Ar << MaterialInfo.Uniform2DTextureExpressions;
		Ar << MaterialInfo.UniformCubeTextureExpressions;		

		////<@ ava specific ; 2007. 1. 24 changmin
		// game 에서는 save가 불리지 않는다는 전제가 있습니다~
		if( bSkipLoadReferencedObject )
		{
			Ar.SetSkipObjectRef( FALSE );
		}
		//>@ ava 

		if( Ar.Ver() > VER_RENDERING_REFACTOR )
		{
			Ar << MaterialInfo.bUsesSceneColor;
			Ar << MaterialInfo.bUsesSceneDepth;
			Ar << MaterialInfo.UsingTransforms;			
		}

		if( Ar.LicenseeVer() >= VER_AVA_ENVCUBE)
		{
			Ar << MaterialInfo.bUsesEnvCube;			
		}

		if(Ar.Ver() >= VER_MIN_COMPILEDMATERIAL && Ar.LicenseeVer() >= LICENSEE_VER_MIN_COMPILEDMATERIAL)
		{
			MaterialInfo.bValidCompilationOutput = TRUE;			
		}

		if( Ar.Ver() >= VER_TEXTUREDENSITY )
		{
			Ar << MaterialInfo.TextureLookups;			
		}

		//<@ ava specific ; 2007. 1. 24 changmin
		if( Ar.LicenseeVer() >= VER_AVA_ADD_MATERIAL_FALLBACK )
		{
			Ar << MaterialInfo.bUsesSpecular;
			Ar << MaterialInfo.bUsesNormal;
			Ar << MaterialInfo.bUsesDistortion;
			Ar << MaterialInfo.bUsesMask;			
		}
		//>@ ava
	}

	if (Ar.IsLoading())
	{
		if (!GIsEditor && NumPlatforms != SP_NumPlatforms)
		{
			debugf( NAME_Warning, TEXT("Material(%s) doesn't have proper fallback infos"), *GetFriendlyName() );
		}	

		//<@ ava specific ; 2007. 7. 13 changnmin
		// sm2_poor 도 다른 expression을 가지게 되어서... 아래를 없애요~

		//!!! 왜 !!!
		//MaterialInfos[SP_PCD3D_SM2_POOR] = MaterialInfos[SP_PCD3D_SM2];

		//>@ ava
	}	
}

UBOOL FMaterial::InitShaderMap()
{
	if(!Id.IsValid())
	{
		// If the material doesn't have a valid ID(loaded from an old package), create a new one.
		Id = appCreateGuid();
	}

#if !USE_SEEKFREE_LOADING
	// make sure the shader cache is loaded
	GetLocalShaderCache(GRHIShaderPlatform);
#endif

	// Find the material's cached shader map.	
	ShaderMap = FMaterialShaderMap::FindId(Id, GRHIShaderPlatform);

#if FINAL_RELEASE
	UBOOL bValidCompilationOutput = TRUE;	
#else
	//<@ ava specific ; 2007. 1. 24 changmin
	// platform 별로 알고 있어야 할 data 가 아닌데..
	// bValidCompilationOutput 은 package update에 대한 플래그임.
	UBOOL &bValidCompilationOutput = MaterialInfos[GRHIShaderPlatform].bValidCompilationOutput;
	//>@ ava
#endif



	if(!bValidCompilationOutput || !ShaderMap || !ShaderMap->IsComplete(this))
	{
		UBOOL bRet = FALSE;

		if(bValidCompilationOutput)
		{
			const TCHAR* ShaderMapCondition;
			if(ShaderMap)
			{
				ShaderMapCondition = TEXT("Incomplete");
			}
			else
			{
				ShaderMapCondition = TEXT("Missing");
			}
			debugf(TEXT("%s cached shader map for material %s"),ShaderMapCondition,*GetFriendlyName());
		}
		else
		{
			debugf(TEXT("Material %s has outdated uniform expressions; regenerating."),*GetFriendlyName());

			// Material이 out-dated되어있다네~ 저장해야죠~
			bRet = TRUE;
		}

#if (CONSOLE || FINAL_RELEASE)
		debugf(TEXT("Can't compile %s on console, will attempt to use default material instead"), *GetFriendlyName());
#else
		// If there's no cached shader map for this material, compile a new one.
		// This will only happen if the local shader cache happens to get out of sync with a modified material package, which should only
		// happen in exceptional cases; the editor crashed between saving the shader cache and the modified material package, etc.
		Compile(GRHIShaderPlatform, ShaderMap);
#endif
		// make sure the shader cache is saved out
		GetLocalShaderCache(GRHIShaderPlatform)->MarkDirty();		

		return bRet;
	}
	else
	{
		// Initialize the shaders used by the material.
		// Use lazy initialization in the editor; Init is called by FShader::Get***Shader.
		if(!GIsEditor)
		{
			ShaderMap->BeginInit();
		}

		return FALSE;
	}
}

#pragma ENABLE_OPTIMIZATION

UBOOL IsTranslucentBlendMode(EBlendMode BlendMode)
{
	return BlendMode != BLEND_Opaque && BlendMode != BLEND_Masked;
}

UBOOL FMaterialResource::InitShaderMap()
{
	if (FMaterial::InitShaderMap())
	{
		if (Material)
		{
			Material->MarkPackageDirty();
		}
	}

	return FALSE;
}

UBOOL FMaterialResource::IsTwoSided() const { return Material->TwoSided; }
UBOOL FMaterialResource::IsWireframe() const { return Material->Wireframe; }
UBOOL FMaterialResource::IsLightFunction() const { return Material->bUsedAsLightFunction; }
UBOOL FMaterialResource::IsSpecialEngineMaterial() const { return Material->bUsedAsSpecialEngineMaterial; }
UBOOL FMaterialResource::IsTerrainMaterial() const { return FALSE; }
UBOOL FMaterialResource::IsDecalMaterial() const
{
	return FALSE;
}
UBOOL FMaterialResource::IsUsedWithSkeletalMesh() const
{
	return Material->bUsedWithSkeletalMesh;
}
UBOOL FMaterialResource::IsUsedWithParticleSystem() const
{
	return Material->bUsedWithParticleSystem;
}

UBOOL FMaterialResource::IsUsedWithLensFlare() const
{
	return Material->bUsedWithLensFlare;
}

//<@ ava specific ; 2007. 1. 18 changmin
UBOOL FMaterialResource::NeedsBumpedLightmap() const
{
	const UBOOL bUsesUE3Specular = !GetUsesEnvCube() && GetUsesSpecular();
	return bUsesUE3Specular || GetUsesNormal();
}
//>@ ava

/**
 * Should shaders compiled for this material be saved to disk?
 */
UBOOL FMaterialResource::IsPersistent() const { return TRUE; }

EBlendMode FMaterialResource::GetBlendMode() const { return (EBlendMode)Material->BlendMode; }

EMaterialLightingModel FMaterialResource::GetLightingModel() const { return (EMaterialLightingModel)Material->LightingModel; }

FLOAT FMaterialResource::GetOpacityMaskClipValue() const { return Material->OpacityMaskClipValue; }


/**
* Check for distortion use
* @return TRUE if material uses distoriton
*/
UBOOL FMaterialResource::IsDistorted() const { return Material->bUsesDistortion; }

/**
 * Check if the material is masked and uses an expression or a constant that's not 1.0f for opacity.
 * @return TRUE if the material uses opacity
 */
UBOOL FMaterialResource::IsMasked() const { return Material->bIsMasked; }

FString FMaterialResource::GetFriendlyName() const { return *Material->GetFullName(); }

/** Allows the resource to do things upon compile. */
UBOOL FMaterialResource::Compile( EShaderPlatform Platform, TRefCountPtr<FMaterialShaderMap>& OutShaderMap, UBOOL bForceCompile)
{
	UBOOL bOk = FMaterial::Compile( Platform, OutShaderMap, bForceCompile );
	if ( bOk )
	{
		RebuildTextureLookupInfo( Material );
	}
	return bOk;
}

/** @return the number of components in a vector type. */
UINT GetNumComponents(EMaterialValueType Type)
{
	switch(Type)
	{
		case MCT_Float:
		case MCT_Float1: return 1;
		case MCT_Float2: return 2;
		case MCT_Float3: return 3;
		case MCT_Float4: return 4;
		default: return 0;
	}
}

/** @return the vector type containing a given number of components. */
EMaterialValueType GetVectorType(UINT NumComponents)
{
	switch(NumComponents)
	{
		case 1: return MCT_Float;
		case 2: return MCT_Float2;
		case 3: return MCT_Float3;
		case 4: return MCT_Float4;
		default: return MCT_Unknown;
	};
}

/**
 */
class FMaterialUniformExpressionConstant: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionConstant);
public:
	FMaterialUniformExpressionConstant() {}
	FMaterialUniformExpressionConstant(const FLinearColor& InValue,BYTE InValueType):
		Value(InValue),
		ValueType(InValueType)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << Value << ValueType;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		OutValue = Value;
	}
	virtual UBOOL IsConstant() const
	{
		return TRUE;
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionConstant* OtherConstant = (FMaterialUniformExpressionConstant*)OtherExpression;
		return OtherConstant->ValueType == ValueType && OtherConstant->Value == Value;
	}

private:
	FLinearColor Value;
	BYTE ValueType;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionConstant);

/**
 * A texture expression.
 */
class FMaterialUniformExpressionTexture: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionTexture);
public:

	FMaterialUniformExpressionTexture() {}
	FMaterialUniformExpressionTexture(UTexture* InValue):
		Value(InValue)
	{
		if (!Value)
		{
			appDebugBreak();
		}
	}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << Value;
	}
	virtual void GetTextureValue(const FMaterialRenderContext& Context,const FTexture** OutValue) const
	{
		*OutValue = Value ? Value->Resource : NULL;
	}
	virtual void GetGameThreadTextureValue(UMaterialInstance* MaterialInstance,UTexture** OutValue) const
	{
		*OutValue = Value;
	}
	virtual UBOOL IsConstant() const
	{
		return FALSE;
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionTexture* OtherParameter = (FMaterialUniformExpressionTexture*)OtherExpression;
		return Value == OtherParameter->Value;
	}

private:
	UTexture* Value;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionTexture);



/**
* A texture expression.
*/
class FMaterialUniformExpressionEnvCube : public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionEnvCube);
public:

	FMaterialUniformExpressionEnvCube() {}	

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{		
	}

	virtual void GetTextureValue(const FMaterialRenderContext& Context,const FTexture** OutValue) const
	{		
		*OutValue = ((const FTexture*)-1);
	}
	virtual void GetGameThreadTextureValue(UMaterialInstance* MaterialInstance,UTexture** OutValue) const
	{
		*OutValue = NULL;
	}
	virtual UBOOL IsConstant() const
	{
		return FALSE;
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionEnvCube* OtherParameter = (FMaterialUniformExpressionEnvCube*)OtherExpression;
		return TRUE;
	}
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionEnvCube);

/**
 */
class FMaterialUniformExpressionTime: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionTime);
public:

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		OutValue.R = Context.CurrentTime;
	}
	virtual UBOOL IsConstant() const
	{
		return FALSE;
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		return TRUE;
	}
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionTime);

/**
 */
class FMaterialUniformExpressionRealTime: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionRealTime);
public:

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		OutValue.R = Context.CurrentRealTime;
	}
	virtual UBOOL IsConstant() const
	{
		return FALSE;
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		return TRUE;
	}
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionRealTime);

/**
 */
class FMaterialUniformExpressionVectorParameter: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionVectorParameter);
public:

	FMaterialUniformExpressionVectorParameter() {}
	FMaterialUniformExpressionVectorParameter(FName InParameterName,const FLinearColor& InDefaultValue):
		ParameterName(InParameterName),
		DefaultValue(InDefaultValue)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << ParameterName << DefaultValue;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		OutValue.R = OutValue.G = OutValue.B = OutValue.A = 0;

		if(!Context.MaterialInstance->GetVectorValue(ParameterName, &OutValue))
		{
			OutValue = DefaultValue;
		}
	}
	virtual UBOOL IsConstant() const
	{
		return FALSE;
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionVectorParameter* OtherParameter = (FMaterialUniformExpressionVectorParameter*)OtherExpression;
		return ParameterName == OtherParameter->ParameterName && DefaultValue == OtherParameter->DefaultValue;
	}

private:
	FName ParameterName;
	FLinearColor DefaultValue;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionVectorParameter);

/**
 */
class FMaterialUniformExpressionScalarParameter: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionScalarParameter);
public:

	FMaterialUniformExpressionScalarParameter() {}
	FMaterialUniformExpressionScalarParameter(FName InParameterName,FLOAT InDefaultValue):
		ParameterName(InParameterName),
		DefaultValue(InDefaultValue)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << ParameterName << DefaultValue;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		OutValue.R = OutValue.G = OutValue.B = OutValue.A = 0;

		if(!Context.MaterialInstance->GetScalarValue(ParameterName, &OutValue.R))
		{
			OutValue.R = DefaultValue;
		}
	}
	virtual UBOOL IsConstant() const
	{
		return FALSE;
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionScalarParameter* OtherParameter = (FMaterialUniformExpressionScalarParameter*)OtherExpression;
		return ParameterName == OtherParameter->ParameterName && DefaultValue == OtherParameter->DefaultValue;
	}

private:
	FName ParameterName;
	FLOAT DefaultValue;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionScalarParameter);

/**
 * A texture parameter expression.
 */
class FMaterialUniformExpressionTextureParameter: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionTextureParameter);
public:

	FMaterialUniformExpressionTextureParameter() {}
	FMaterialUniformExpressionTextureParameter(FName InParameterName,UTexture* InDefaultValue):
		ParameterName(InParameterName),
		DefaultValue(InDefaultValue)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << ParameterName << DefaultValue;
	}
	virtual void GetTextureValue(const FMaterialRenderContext& Context,const FTexture** OutValue) const
	{
		*OutValue = NULL;
		if(!Context.MaterialInstance->GetTextureValue(ParameterName,OutValue) && DefaultValue)
		{
			*OutValue = DefaultValue->Resource;
		}
	}
	virtual void GetGameThreadTextureValue(UMaterialInstance* MaterialInstance,UTexture** OutValue) const
	{
		*OutValue = NULL;
		if(!MaterialInstance->GetTextureParameterValue(ParameterName,*OutValue))
		{
			*OutValue = DefaultValue;
		}
	}
	virtual UBOOL IsConstant() const
	{
		return FALSE;
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionTextureParameter* OtherParameter = (FMaterialUniformExpressionTextureParameter*)OtherExpression;
		return ParameterName == OtherParameter->ParameterName && DefaultValue == OtherParameter->DefaultValue;
	}

private:
	FName ParameterName;
	UTexture* DefaultValue;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionTextureParameter);

/**
 * A lfipbook texture parameter expression.
 */
class FMaterialUniformExpressionFlipBookTextureParameter : public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionFlipBookTextureParameter);
public:

	FMaterialUniformExpressionFlipBookTextureParameter() {}
	FMaterialUniformExpressionFlipBookTextureParameter(UTextureFlipBook* InTexture) :
		FlipBookTexture(InTexture)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << FlipBookTexture;
	}

	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		OutValue.R = OutValue.G = OutValue.B = OutValue.A = 0;

		if (FlipBookTexture)
		{
			FlipBookTexture->GetTextureOffset_RenderThread(OutValue);
		}
	}
	virtual UBOOL IsConstant() const
	{
		return FALSE;
	}

	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionFlipBookTextureParameter* OtherParameter = (FMaterialUniformExpressionFlipBookTextureParameter*)OtherExpression;
		return (FlipBookTexture == OtherParameter->FlipBookTexture);
	}

private:
	UTextureFlipBook* FlipBookTexture;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionFlipBookTextureParameter);

/**
 */
class FMaterialUniformExpressionSine: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionSine);
public:

	FMaterialUniformExpressionSine() {}
	FMaterialUniformExpressionSine(FMaterialUniformExpression* InX,UBOOL bInIsCosine):
		X(InX),
		bIsCosine(bInIsCosine)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << X << bIsCosine;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		FLinearColor ValueX;
		X->GetNumberValue(Context,ValueX);
		OutValue.R = bIsCosine ? appCos(ValueX.R) : appSin(ValueX.R);
	}
	virtual UBOOL IsConstant() const
	{
		return X->IsConstant();
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionSine* OtherSine = (FMaterialUniformExpressionSine*)OtherExpression;
		return X == OtherSine->X && bIsCosine == OtherSine->bIsCosine;
	}

private:
	TRefCountPtr<FMaterialUniformExpression> X;
	UBOOL bIsCosine;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionSine);

/**
 */
class FMaterialUniformExpressionSquareRoot: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionSquareRoot);
public:

	FMaterialUniformExpressionSquareRoot() {}
	FMaterialUniformExpressionSquareRoot(FMaterialUniformExpression* InX):
		X(InX)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << X;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		FLinearColor ValueX;
		X->GetNumberValue(Context,ValueX);
		OutValue.R = appSqrt(ValueX.R);
	}
	virtual UBOOL IsConstant() const
	{
		return X->IsConstant();
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionSquareRoot* OtherSqrt = (FMaterialUniformExpressionSquareRoot*)OtherExpression;
		return X == OtherSqrt->X;
	}

private:
	TRefCountPtr<FMaterialUniformExpression> X;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionSquareRoot);

/**
 */
enum EFoldedMathOperation
{
	FMO_Add,
	FMO_Sub,
	FMO_Mul,
	FMO_Div,
	FMO_Dot
};

class FMaterialUniformExpressionFoldedMath: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionFoldedMath);
public:

	FMaterialUniformExpressionFoldedMath() {}
	FMaterialUniformExpressionFoldedMath(FMaterialUniformExpression* InA,FMaterialUniformExpression* InB,BYTE InOp):
		A(InA),
		B(InB),
		Op(InOp)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << A << B << Op;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		FLinearColor ValueA, ValueB;
		A->GetNumberValue(Context, ValueA);
		B->GetNumberValue(Context, ValueB);

		switch(Op)
		{
			case FMO_Add: OutValue = ValueA + ValueB; break;
			case FMO_Sub: OutValue = ValueA - ValueB; break;
			case FMO_Mul: OutValue = ValueA * ValueB; break;
			case FMO_Div: 
				OutValue.R = ValueA.R / ValueB.R;
				OutValue.G = ValueA.G / ValueB.G;
				OutValue.B = ValueA.B / ValueB.B;
				OutValue.A = ValueA.A / ValueB.A;
				break;
			case FMO_Dot: 
				{
					FLOAT DotProduct = ValueA.R * ValueB.R + ValueA.G * ValueB.G + ValueA.B * ValueB.B + ValueA.A * ValueB.A;
					OutValue.R = OutValue.G = OutValue.B = OutValue.A = DotProduct;
				}
				break;
			default: appErrorf(TEXT("Unknown folded math operation: %08x"),(INT)Op);
		};
	}
	virtual UBOOL IsConstant() const
	{
		return A->IsConstant() && B->IsConstant();
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpressionFoldedMath* OtherExpression)
	{
		FMaterialUniformExpressionFoldedMath* OtherMath = (FMaterialUniformExpressionFoldedMath*)OtherExpression;
		return A == OtherMath->A && B == OtherMath->B && Op == OtherMath->Op;
	}

private:
	TRefCountPtr<FMaterialUniformExpression> A;
	TRefCountPtr<FMaterialUniformExpression> B;
	BYTE Op;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionFoldedMath);

/**
 * A hint that only the fractional part of this expession's value matters.
 */
class FMaterialUniformExpressionPeriodic: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionPeriodic);
public:

	FMaterialUniformExpressionPeriodic() {}
	FMaterialUniformExpressionPeriodic(FMaterialUniformExpression* InX):
		X(InX)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << X;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		FLinearColor TempValue;
		X->GetNumberValue(Context,TempValue);

		OutValue.R = TempValue.R - appFloor(TempValue.R);
		OutValue.G = TempValue.G - appFloor(TempValue.G);
		OutValue.B = TempValue.B - appFloor(TempValue.B);
		OutValue.A = TempValue.A - appFloor(TempValue.A);
	}
	virtual UBOOL IsConstant() const
	{
		return X->IsConstant();
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionPeriodic* OtherPeriodic = (FMaterialUniformExpressionPeriodic*)OtherExpression;
		return X == OtherPeriodic->X;
	}

private:
	TRefCountPtr<FMaterialUniformExpression> X;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionPeriodic);

/**
 */
class FMaterialUniformExpressionAppendVector: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionAppendVector);
public:

	FMaterialUniformExpressionAppendVector() {}
	FMaterialUniformExpressionAppendVector(FMaterialUniformExpression* InA,FMaterialUniformExpression* InB,UINT InNumComponentsA):
		A(InA),
		B(InB),
		NumComponentsA(InNumComponentsA)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << A << B << NumComponentsA;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		FLinearColor	ValueA, ValueB;
		A->GetNumberValue(Context, ValueA);
		B->GetNumberValue(Context, ValueB);

		OutValue.R = NumComponentsA >= 1 ? ValueA.R : (&ValueB.R)[0 - NumComponentsA];
		OutValue.G = NumComponentsA >= 2 ? ValueA.G : (&ValueB.R)[1 - NumComponentsA];
		OutValue.B = NumComponentsA >= 3 ? ValueA.B : (&ValueB.R)[2 - NumComponentsA];
		OutValue.A = NumComponentsA >= 4 ? ValueA.A : (&ValueB.R)[3 - NumComponentsA];
	}
	virtual UBOOL IsConstant() const
	{
		return A->IsConstant() && B->IsConstant();
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionAppendVector* OtherAppend = (FMaterialUniformExpressionAppendVector*)OtherExpression;
		return A == OtherAppend->A && B == OtherAppend->B && NumComponentsA == OtherAppend->NumComponentsA;
	}

private:
	TRefCountPtr<FMaterialUniformExpression> A;
	TRefCountPtr<FMaterialUniformExpression> B;
	UINT NumComponentsA;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionAppendVector);

/**
 */
class FMaterialUniformExpressionMin: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionMin);
public:

	FMaterialUniformExpressionMin() {}
	FMaterialUniformExpressionMin(FMaterialUniformExpression* InA,FMaterialUniformExpression* InB):
		A(InA),
		B(InB)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << A << B;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		FLinearColor	ValueA, ValueB;
		A->GetNumberValue(Context, ValueA);
		B->GetNumberValue(Context, ValueB);

		OutValue.R = Min(ValueA.R, ValueB.R);
		OutValue.G = Min(ValueA.G, ValueB.G);
		OutValue.B = Min(ValueA.B, ValueB.B);
		OutValue.A = Min(ValueA.A, ValueB.A);
	}
	virtual UBOOL IsConstant() const
	{
		return A->IsConstant() && B->IsConstant();
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionMin* OtherMin = (FMaterialUniformExpressionMin*)OtherExpression;
		return A == OtherMin->A && B == OtherMin->B;
	}

private:
	TRefCountPtr<FMaterialUniformExpression> A;
	TRefCountPtr<FMaterialUniformExpression> B;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionMin);

/**
 */
class FMaterialUniformExpressionMax: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionMax);
public:

	FMaterialUniformExpressionMax() {}
	FMaterialUniformExpressionMax(FMaterialUniformExpression* InA,FMaterialUniformExpression* InB):
		A(InA),
		B(InB)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << A << B;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		FLinearColor	ValueA, ValueB;
		A->GetNumberValue(Context, ValueA);
		B->GetNumberValue(Context, ValueB);

		OutValue.R = Max(ValueA.R, ValueB.R);
		OutValue.G = Max(ValueA.G, ValueB.G);
		OutValue.B = Max(ValueA.B, ValueB.B);
		OutValue.A = Max(ValueA.A, ValueB.A);
	}
	virtual UBOOL IsConstant() const
	{
		return A->IsConstant() && B->IsConstant();
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionMax* OtherMax = (FMaterialUniformExpressionMax*)OtherExpression;
		return A == OtherMax->A && B == OtherMax->B;
	}

private:
	TRefCountPtr<FMaterialUniformExpression> A;
	TRefCountPtr<FMaterialUniformExpression> B;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionMax);

/**
 */
class FMaterialUniformExpressionClamp: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionClamp);
public:

	FMaterialUniformExpressionClamp() {}
	FMaterialUniformExpressionClamp(FMaterialUniformExpression* InInput,FMaterialUniformExpression* InMin,FMaterialUniformExpression* InMax):
		Input(InInput),
		Min(InMin),
		Max(InMax)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << Input << Min << Max;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		FLinearColor	ValueMin, ValueMax, ValueInput;
		Min->GetNumberValue(Context, ValueMin);
		Max->GetNumberValue(Context, ValueMax);
		Input->GetNumberValue(Context, ValueInput);

		OutValue.R = Clamp(ValueInput.R, ValueMin.R, ValueMax.R);
		OutValue.G = Clamp(ValueInput.G, ValueMin.G, ValueMax.G);
		OutValue.B = Clamp(ValueInput.B, ValueMin.B, ValueMax.B);
		OutValue.A = Clamp(ValueInput.A, ValueMin.A, ValueMax.A);
	}
	virtual UBOOL IsConstant() const
	{
		return Input->IsConstant() && Min->IsConstant() && Max->IsConstant();
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionClamp* OtherClamp = (FMaterialUniformExpressionClamp*)OtherExpression;
		return Input == OtherClamp->Input && Min == OtherClamp->Min && Max == OtherClamp->Max;
	}

private:
	TRefCountPtr<FMaterialUniformExpression> Input;
	TRefCountPtr<FMaterialUniformExpression> Min;
	TRefCountPtr<FMaterialUniformExpression> Max;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionClamp);

/**
 */
class FMaterialUniformExpressionFloor: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionFloor);
public:

	FMaterialUniformExpressionFloor() {}
	FMaterialUniformExpressionFloor(FMaterialUniformExpression* InX):
		X(InX)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << X;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		X->GetNumberValue(Context, OutValue);

		OutValue.R = appFloor(OutValue.R);
		OutValue.G = appFloor(OutValue.G);
		OutValue.B = appFloor(OutValue.B);
		OutValue.A = appFloor(OutValue.A);
	}
	virtual UBOOL IsConstant() const
	{
		return X->IsConstant();
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionFloor* OtherFloor = (FMaterialUniformExpressionFloor*)OtherExpression;
		return X == OtherFloor->X;
	}

private:
	TRefCountPtr<FMaterialUniformExpression> X;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionFloor);

/**
 */
class FMaterialUniformExpressionCeil: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionCeil);
public:

	FMaterialUniformExpressionCeil() {}
	FMaterialUniformExpressionCeil(FMaterialUniformExpression* InX):
		X(InX)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << X;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		X->GetNumberValue(Context, OutValue);

		OutValue.R = appCeil(OutValue.R);
		OutValue.G = appCeil(OutValue.G);
		OutValue.B = appCeil(OutValue.B);
		OutValue.A = appCeil(OutValue.A);
	}
	virtual UBOOL IsConstant() const
	{
		return X->IsConstant();
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionCeil* OtherCeil = (FMaterialUniformExpressionCeil*)OtherExpression;
		return X == OtherCeil->X;
	}

private:
	TRefCountPtr<FMaterialUniformExpression> X;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionCeil);

/**
 */
class FMaterialUniformExpressionFrac: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionFrac);
public:

	FMaterialUniformExpressionFrac() {}
	FMaterialUniformExpressionFrac(FMaterialUniformExpression* InX):
		X(InX)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << X;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		X->GetNumberValue(Context, OutValue);

		OutValue.R = OutValue.R - appFloor(OutValue.R);
		OutValue.G = OutValue.G - appFloor(OutValue.G);
		OutValue.B = OutValue.B - appFloor(OutValue.B);
		OutValue.A = OutValue.A - appFloor(OutValue.A);
	}
	virtual UBOOL IsConstant() const
	{
		return X->IsConstant();
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionFrac* OtherFrac = (FMaterialUniformExpressionFrac*)OtherExpression;
		return X == OtherFrac->X;
	}

private:
	TRefCountPtr<FMaterialUniformExpression> X;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionFrac);

/**
 * Absolute value evaluator for a given input expression
 */
class FMaterialUniformExpressionAbs: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionAbs);
public:

	FMaterialUniformExpressionAbs() {}
	FMaterialUniformExpressionAbs( FMaterialUniformExpression* InX ):
		X(InX)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << X;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		X->GetNumberValue(Context, OutValue);
		OutValue.R = Abs<FLOAT>(OutValue.R);
		OutValue.G = Abs<FLOAT>(OutValue.G);
		OutValue.B = Abs<FLOAT>(OutValue.B);
		OutValue.A = Abs<FLOAT>(OutValue.A);
	}
	virtual UBOOL IsConstant() const
	{
		return X->IsConstant();
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionAbs* OtherAbs = (FMaterialUniformExpressionAbs*)OtherExpression;
		return X == OtherAbs->X;
	}

private:
	TRefCountPtr<FMaterialUniformExpression> X;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionAbs);

struct FShaderCodeChunk
{
	FString Code;
	TRefCountPtr<FMaterialUniformExpression> UniformExpression;
	EMaterialValueType Type;
	DWORD Flags;
	INT TextureDependencyLength;

	FShaderCodeChunk(const TCHAR* InCode,EMaterialValueType InType,DWORD InFlags,INT InTextureDependencyLength):
		Code(InCode),
		UniformExpression(NULL),
		Type(InType),
		Flags(InFlags),
		TextureDependencyLength(InTextureDependencyLength)
	{}

	FShaderCodeChunk(FMaterialUniformExpression* InUniformExpression,const TCHAR* InCode,EMaterialValueType InType,DWORD InFlags):
		Code(InCode),
		UniformExpression(InUniformExpression),
		Type(InType),
		Flags(InFlags),
		TextureDependencyLength(0)
	{}
};

UBOOL FMaterial::Compile(EShaderPlatform Platform, TRefCountPtr<FMaterialShaderMap>& OutShaderMap, UBOOL bForceCompile)
{
	check(GIsEditor || !GIsGame || Platform == GRHIShaderPlatform);

	FlushRenderingCommands();

	class FHLSLMaterialTranslator : public FMaterialCompiler
	{
	public:
		enum EShaderCodeChunkFlags
		{
			SCF_RGBE_4BIT_EXPONENT			= 1,
			SCF_RGBE_8BIT_EXPONENT			= 2,
			SCF_RGBE						= SCF_RGBE_4BIT_EXPONENT | SCF_RGBE_8BIT_EXPONENT,
			SCF_REQUIRES_GAMMA_CORRECTION	= 4,
			SCF_RGBL						= 8,
		};

		TIndirectArray<FShaderCodeChunk> CodeChunks;
		TArray<UMaterialExpression*> ExpressionStack;

		/** A map from material expression to the index into CodeChunks of the code for the material expression. */
		TMap<UMaterialExpression*,INT> ExpressionCodeMap;

		UBOOL bSuccess;

		FMaterial* Material;

		FHLSLMaterialTranslator(FMaterial* InMaterial):
			bSuccess(TRUE),
			Material(InMaterial)
		{
			//Material->CompileErrors.Empty();
			//Material->TextureDependencyLengthMap.Empty();
			//Material->MaxTextureDependencyLength = 0;

			//Material->NumUserTexCoords = 0;
			//Material->UniformScalarExpressions.Empty();
			//Material->UniformVectorExpressions.Empty();
			//Material->Uniform2DTextureExpressions.Empty();
			//Material->UniformCubeTextureExpressions.Empty();
			//Material->UsingTransforms = UsedCoord_None;

			//<@ ava specific ; 2007. 1. 24 changmin
			Material->GetCompileErrors().Empty();
			Material->GetTextureDependencyLengthMap().Empty();
			Material->SetMaxTextureDependencyLength(0);
			Material->SetUserTexCoordsUsed(0);
			Material->GetUniformVectorExpressions().Empty();
			Material->GetUniformScalarExpressions().Empty();
			Material->GetUniform2DTextureExpressions().Empty();
			Material->GetUniformCubeTextureExpressions().Empty();
			Material->SetTransformsUsed(UsedCoord_None);
			//>@ ava
		}

		const TCHAR* DescribeType(EMaterialValueType Type) const
		{
			switch(Type)
			{
			case MCT_Float1:		return TEXT("float1");
			case MCT_Float2:		return TEXT("float2");
			case MCT_Float3:		return TEXT("float3");
			case MCT_Float4:		return TEXT("float4");
			case MCT_Float:			return TEXT("float");
			case MCT_Texture2D:		return TEXT("texture2D");
			case MCT_TextureCube:	return TEXT("textureCube");
			default:				return TEXT("unknown");
			};
			return NULL;
		}

		// AddCodeChunk - Adds a formatted code string to the Code array and returns its index.
		INT AddCodeChunk(EMaterialValueType Type,DWORD Flags,INT TextureDependencyDepth,const TCHAR* Format,...)
		{
			INT		BufferSize		= 256;
			TCHAR*	FormattedCode	= NULL;
			INT		Result			= -1;

			while(Result == -1)
			{
				FormattedCode = (TCHAR*) appRealloc( FormattedCode, BufferSize * sizeof(TCHAR) );
				GET_VARARGS_RESULT(FormattedCode,BufferSize - 1,Format,Format,Result);
				BufferSize *= 2;
			};
			FormattedCode[Result] = 0;

			INT	CodeIndex = CodeChunks.Num();
			new(CodeChunks) FShaderCodeChunk(FormattedCode,Type,Flags,TextureDependencyDepth);

			appFree(FormattedCode);

			return CodeIndex;
		}

		// AddUniformExpression - Adds an input to the Code array and returns its index.
		INT AddUniformExpression(FMaterialUniformExpression* UniformExpression,EMaterialValueType Type,const TCHAR* Format,...)
		{
			// Search for an existing code chunk with the same uniform expression.
			for(INT ChunkIndex = 0;ChunkIndex < CodeChunks.Num();ChunkIndex++)
			{
				FMaterialUniformExpression* TestExpression = CodeChunks(ChunkIndex).UniformExpression;
				if(TestExpression && TestExpression->GetType() == UniformExpression->GetType() && TestExpression->IsIdentical(UniformExpression))
				{
					// This code chunk has an identical uniform expression to the new expression.  Reuse it.
					check(Type == CodeChunks(ChunkIndex).Type);
					delete UniformExpression;
					return ChunkIndex;
				}
			}

			INT		BufferSize		= 256;
			TCHAR*	FormattedCode	= NULL;
			INT		Result			= -1;

			while(Result == -1)
			{
				FormattedCode = (TCHAR*) appRealloc( FormattedCode, BufferSize * sizeof(TCHAR) );
				GET_VARARGS_RESULT(FormattedCode,BufferSize - 1,Format,Format,Result);
				BufferSize *= 2;
			};
			FormattedCode[Result] = 0;

			INT	CodeIndex = CodeChunks.Num();
			new(CodeChunks) FShaderCodeChunk(UniformExpression,FormattedCode,Type,0);

			appFree(FormattedCode);

			return CodeIndex;
		}

		INT AddUniformExpression(FMaterialUniformExpression* UniformExpression,DWORD Flags,EMaterialValueType Type,const TCHAR* Format,...)
		{
			// Search for an existing code chunk with the same uniform expression.
			for(INT ChunkIndex = 0;ChunkIndex < CodeChunks.Num();ChunkIndex++)
			{
				FMaterialUniformExpression* TestExpression = CodeChunks(ChunkIndex).UniformExpression;
				if(TestExpression && TestExpression->GetType() == UniformExpression->GetType() && TestExpression->IsIdentical(UniformExpression))
				{
					// This code chunk has an identical uniform expression to the new expression.  Reuse it.
					check(Type == CodeChunks(ChunkIndex).Type);
					delete UniformExpression;
					return ChunkIndex;
				}
			}

			INT		BufferSize		= 256;
			TCHAR*	FormattedCode	= NULL;
			INT		Result			= -1;

			while(Result == -1)
			{
				FormattedCode = (TCHAR*) appRealloc( FormattedCode, BufferSize * sizeof(TCHAR) );
				GET_VARARGS_RESULT(FormattedCode,BufferSize - 1,Format,Format,Result);
				BufferSize *= 2;
			};
			FormattedCode[Result] = 0;

			INT	CodeIndex = CodeChunks.Num();
			new(CodeChunks) FShaderCodeChunk(UniformExpression,FormattedCode,Type,Flags);

			appFree(FormattedCode);

			return CodeIndex;
		}

		// AccessUniformExpression - Adds code to access the value of a uniform expression to the Code array and returns its index.
		INT AccessUniformExpression(INT Index)
		{
			check(Index >= 0 && Index < CodeChunks.Num());

			const FShaderCodeChunk&	CodeChunk = CodeChunks(Index);

			check(CodeChunk.UniformExpression);

			TCHAR* FormattedCode = NULL;
			// This malloc assumes that the below appSprintf won't use more.
			FormattedCode = (TCHAR*) appMalloc( 50 * sizeof(TCHAR) );

			if(CodeChunk.Type == MCT_Float)
			{
				//<@ ava specific; 2007. 1. 24 changmin
				//INT	ScalarInputIndex = Material->UniformScalarExpressions.AddUniqueItem(CodeChunk.UniformExpression);
				INT	ScalarInputIndex = Material->GetUniformScalarExpressions().AddUniqueItem(CodeChunk.UniformExpression);
				//>@ ava

				// Update the above appMalloc if this appSprintf grows in size, e.g. %s, ...
				appSprintf(FormattedCode,TEXT("UniformScalar_%u"),ScalarInputIndex);
			}
			else if(CodeChunk.Type & MCT_Float)
			{
				//<@ ava specific ; 2007. 1. 24 changmin
				//INT VectorInputIndex = Material->UniformVectorExpressions.AddUniqueItem(CodeChunk.UniformExpression);
				INT VectorInputIndex = Material->GetUniformVectorExpressions().AddUniqueItem(CodeChunk.UniformExpression);
				//>@ ava
				const TCHAR* Mask;
				switch(CodeChunk.Type)
				{
				case MCT_Float:
				case MCT_Float1: Mask = TEXT(".r"); break;
				case MCT_Float2: Mask = TEXT(".rg"); break;
				case MCT_Float3: Mask = TEXT(".rgb"); break;
				default: Mask = TEXT(""); break;
				};
				appSprintf(FormattedCode,TEXT("UniformVector_%u%s"),VectorInputIndex,Mask);
			}
			else if(CodeChunk.Type & MCT_Texture)
			{
				INT TextureInputIndex = INDEX_NONE;
				const TCHAR* BaseName = TEXT("");
				switch(CodeChunk.Type)
				{
				case MCT_Texture2D:
					//<@ ava specific ; 2007. 1. 24 changmin
					//TextureInputIndex = Material->Uniform2DTextureExpressions.AddUniqueItem(CodeChunk.UniformExpression);
					TextureInputIndex = Material->GetUniform2DTextureExpressions().AddUniqueItem(CodeChunk.UniformExpression);
					//>@ ava
					BaseName = TEXT("Texture2D");
					break;
				case MCT_TextureCube:
					//<@ ava specific ; 2007. 1. 24 changmin
					//TextureInputIndex = Material->UniformCubeTextureExpressions.AddUniqueItem(CodeChunk.UniformExpression);
					TextureInputIndex = Material->GetUniformCubeTextureExpressions().AddUniqueItem(CodeChunk.UniformExpression);
					//>@ ava
					BaseName = TEXT("TextureCube");
					break;
				default: appErrorf(TEXT("Unrecognized texture material value type: %u"),(INT)CodeChunk.Type);
				};
				appSprintf(FormattedCode,TEXT("%s_%u"),BaseName,TextureInputIndex);
			}
			else
			{
				appErrorf(TEXT("User input of unknown type: %s"),DescribeType(CodeChunk.Type));
			}

			INT	CodeIndex = CodeChunks.Num();
			new(CodeChunks) FShaderCodeChunk(FormattedCode,CodeChunks(Index).Type,0,0);

			appFree(FormattedCode);

			return CodeIndex;
		}

		// GetParameterCode

		const TCHAR* GetParameterCode(INT Index)
		{
			check(Index >= 0 && Index < CodeChunks.Num());
			if(!CodeChunks(Index).UniformExpression || CodeChunks(Index).UniformExpression->IsConstant())
			{
				return *CodeChunks(Index).Code;
			}
			else
			{
				return *CodeChunks(AccessUniformExpression(Index)).Code;
			}
		}

		// CoerceParameter
		FString CoerceParameter(INT Index,EMaterialValueType DestType)
		{
			check(Index >= 0 && Index < CodeChunks.Num());
			const FShaderCodeChunk&	CodeChunk = CodeChunks(Index);
			if( CodeChunk.Type == DestType )
			{
				return GetParameterCode(Index);
			}
			else
			if( (CodeChunk.Type & DestType) && (CodeChunk.Type & MCT_Float) )
			{
				switch( DestType )
				{
				case MCT_Float1:
					return FString::Printf( TEXT("float(%s)"), GetParameterCode(Index) );
				case MCT_Float2:
					return FString::Printf( TEXT("float2(%s,%s)"), GetParameterCode(Index), GetParameterCode(Index) );
				case MCT_Float3:
					return FString::Printf( TEXT("float3(%s,%s,%s)"), GetParameterCode(Index), GetParameterCode(Index), GetParameterCode(Index) );
				case MCT_Float4:
					return FString::Printf( TEXT("float4(%s,%s,%s,%s)"), GetParameterCode(Index), GetParameterCode(Index), GetParameterCode(Index), GetParameterCode(Index) );
				default: 
					return FString::Printf( TEXT("%s"), GetParameterCode(Index) );
				}
			}
			else
			{
				Errorf(TEXT("Coercion failed: %s: %s -> %s"),*CodeChunk.Code,DescribeType(CodeChunk.Type),DescribeType(DestType));
				return TEXT("");
			}
		}

		// GetFixedParameterCode
		const TCHAR* GetFixedParameterCode(INT Index) const
		{
			if(Index != INDEX_NONE)
			{
				check(Index >= 0 && Index < CodeChunks.Num());
				check(!CodeChunks(Index).UniformExpression || CodeChunks(Index).UniformExpression->IsConstant());
				return *CodeChunks(Index).Code;
			}
			else
			{
				return TEXT("0");
			}
		}

		// GetParameterType
		EMaterialValueType GetParameterType(INT Index) const
		{
			check(Index >= 0 && Index < CodeChunks.Num());
			return CodeChunks(Index).Type;
		}

		// GetParameterUniformExpression
		FMaterialUniformExpression* GetParameterUniformExpression(INT Index) const
		{
			check(Index >= 0 && Index < CodeChunks.Num());
			return CodeChunks(Index).UniformExpression;
		}

		// GetParameterFlags
		DWORD GetParameterFlags(INT Index) const
		{
			check(Index >= 0 && Index < CodeChunks.Num());
			return CodeChunks(Index).Flags;
		}

		/**
		 * Finds the texture dependency length of the given code chunk.
		 * @param CodeChunkIndex - The index of the code chunk.
		 * @return The texture dependency length of the code chunk.
		 */
		INT GetTextureDependencyLength(INT CodeChunkIndex)
		{
			if(CodeChunkIndex != INDEX_NONE)
			{
				check(CodeChunkIndex >= 0 && CodeChunkIndex < CodeChunks.Num());
				return CodeChunks(CodeChunkIndex).TextureDependencyLength;
			}
			else
			{
				return 0;
			}
		}

		/**
		 * Finds the maximum texture dependency length of the given code chunks.
		 * @param CodeChunkIndex* - A list of code chunks to find the maximum texture dependency length from.
		 * @return The texture dependency length of the code chunk.
		 */
		INT GetTextureDependencyLengths(INT CodeChunkIndex0,INT CodeChunkIndex1 = INDEX_NONE,INT CodeChunkIndex2 = INDEX_NONE)
		{
			return ::Max(
					::Max(
						GetTextureDependencyLength(CodeChunkIndex0),
						GetTextureDependencyLength(CodeChunkIndex1)
						),
					GetTextureDependencyLength(CodeChunkIndex2)
					);
		}

		// GetArithmeticResultType
		EMaterialValueType GetArithmeticResultType(EMaterialValueType TypeA,EMaterialValueType TypeB)
		{
			if(!(TypeA & MCT_Float) || !(TypeB & MCT_Float))
			{
				Errorf(TEXT("Attempting to perform arithmetic on non-numeric types: %s %s"),DescribeType(TypeA),DescribeType(TypeB));
			}

			if(TypeA == TypeB)
			{
				return TypeA;
			}
			else if(TypeA & TypeB)
			{
				if(TypeA == MCT_Float)
				{
					return TypeB;
				}
				else
				{
					check(TypeB == MCT_Float);
					return TypeA;
				}
			}
			else
			{
				Errorf(TEXT("Arithmetic between types %s and %s are undefined"),DescribeType(TypeA),DescribeType(TypeB));
				return MCT_Unknown;
			}
		}

		EMaterialValueType GetArithmeticResultType(INT A,INT B)
		{
			check(A >= 0 && A < CodeChunks.Num());
			check(B >= 0 && B < CodeChunks.Num());

			EMaterialValueType	TypeA = CodeChunks(A).Type,
								TypeB = CodeChunks(B).Type;
			
			return GetArithmeticResultType(TypeA,TypeB);
		}

		// FMaterialCompiler interface.

		virtual INT Error(const TCHAR* Text)
		{
			UMaterialExpression* MaterialExpression = NULL;
			if(ExpressionStack.Num())
			{
				MaterialExpression = ExpressionStack.Top();
			}

			//Material->CompileErrors.Set(MaterialExpression,Text);
			Material->GetCompileErrors().Set(MaterialExpression,Text);
			bSuccess = FALSE;

			return INDEX_NONE;
		}

		virtual INT CallExpression(UMaterialExpression* MaterialExpression,FMaterialCompiler* Compiler)
		{
			// Check if this expression has already been translated.
			INT* ExistingCodeIndex = ExpressionCodeMap.Find(MaterialExpression);
			if(ExistingCodeIndex)
			{
				return *ExistingCodeIndex;
			}
			else
			{
				// Disallow reentrance.
				if(ExpressionStack.FindItemIndex(MaterialExpression) != INDEX_NONE)
				{
					return Error(TEXT("Reentrant expression"));
				}

				// The first time this expression is called, translate it.
				ExpressionStack.AddItem(MaterialExpression);
				INT Result = MaterialExpression->Compile(Compiler);
				check(ExpressionStack.Pop() == MaterialExpression);

				// Save the texture dependency depth for the expression.
				if (Material->IsTerrainMaterial() == FALSE)
				{
					const INT TextureDependencyLength = GetTextureDependencyLength(Result);
					//Material->TextureDependencyLengthMap.Set(MaterialExpression,TextureDependencyLength);
					//Material->MaxTextureDependencyLength = ::Max(Material->MaxTextureDependencyLength,TextureDependencyLength);
					Material->GetTextureDependencyLengthMap().Set(MaterialExpression,TextureDependencyLength);
					INT MaxTextureDependencyLength = ::Max(Material->GetMaxTextureDependencyLength(),TextureDependencyLength);
					Material->SetMaxTextureDependencyLength( MaxTextureDependencyLength );
				}

				// Cache the translation.
				ExpressionCodeMap.Set(MaterialExpression,Result);

				return Result;
			}
		}

		virtual EMaterialValueType GetType(INT Code)
		{
			if(Code != INDEX_NONE)
			{
				return GetParameterType(Code);
			}
			else
			{
				return MCT_Unknown;
			}
		}

		virtual INT ForceCast(INT Code,EMaterialValueType DestType)
		{
			if(Code == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			if(GetParameterUniformExpression(Code) && !GetParameterUniformExpression(Code)->IsConstant())
			{
				return ForceCast(AccessUniformExpression(Code),DestType);
			}

			EMaterialValueType	SourceType = GetParameterType(Code);

			if(SourceType & DestType)
			{
				return Code;
			}
			else if((SourceType & MCT_Float) && (DestType & MCT_Float))
			{
				UINT	NumSourceComponents = GetNumComponents(SourceType),
						NumDestComponents = GetNumComponents(DestType);

				if(NumSourceComponents > NumDestComponents) // Use a mask to select the first NumDestComponents components from the source.
				{
					const TCHAR*	Mask;
					switch(NumDestComponents)
					{
						case 1: Mask = TEXT(".r"); break;
						case 2: Mask = TEXT(".rg"); break;
						case 3: Mask = TEXT(".rgb"); break;
						default: appErrorf(TEXT("Should never get here!")); return INDEX_NONE;
					};

					return AddCodeChunk(DestType,0,GetTextureDependencyLength(Code),TEXT("%s%s"),GetParameterCode(Code),Mask);
				}
				else if(NumSourceComponents < NumDestComponents) // Pad the source vector up to NumDestComponents.
				{
					UINT	NumPadComponents = NumDestComponents - NumSourceComponents;
					return AddCodeChunk(
						DestType,
						0,
						GetTextureDependencyLength(Code),
						TEXT("%s(%s%s%s%s)"),
						DescribeType(DestType),
						GetParameterCode(Code),
						NumPadComponents >= 1 ? TEXT(",0") : TEXT(""),
						NumPadComponents >= 2 ? TEXT(",0") : TEXT(""),
						NumPadComponents >= 3 ? TEXT(",0") : TEXT("")
						);
				}
				else
				{
					return Code;
				}
			}
			else
			{
				return Errorf(TEXT("Cannot force a cast between non-numeric types."));
			}
		}

		virtual INT VectorParameter(FName ParameterName,const FLinearColor& DefaultValue)
		{
			return AddUniformExpression(new FMaterialUniformExpressionVectorParameter(ParameterName,DefaultValue),MCT_Float4,TEXT(""));
		}

		virtual INT ScalarParameter(FName ParameterName,FLOAT DefaultValue)
		{
			return AddUniformExpression(new FMaterialUniformExpressionScalarParameter(ParameterName,DefaultValue),MCT_Float,TEXT(""));
		}

		virtual INT FlipBookOffset(UTexture* InFlipBook)
		{
			UTextureFlipBook* FlipBook = CastChecked<UTextureFlipBook>(InFlipBook);
			return AddUniformExpression(new FMaterialUniformExpressionFlipBookTextureParameter(FlipBook), MCT_Float4, TEXT(""));
		}

		virtual INT Constant(FLOAT X)
		{
			return AddUniformExpression(new FMaterialUniformExpressionConstant(FLinearColor(X,0,0,0),MCT_Float),MCT_Float,TEXT("(%0.8f)"),X);
		}

		virtual INT Constant2(FLOAT X,FLOAT Y)
		{
			return AddUniformExpression(new FMaterialUniformExpressionConstant(FLinearColor(X,Y,0,0),MCT_Float2),MCT_Float2,TEXT("float2(%0.8f,%0.8f)"),X,Y);
		}

		virtual INT Constant3(FLOAT X,FLOAT Y,FLOAT Z)
		{
			return AddUniformExpression(new FMaterialUniformExpressionConstant(FLinearColor(X,Y,Z,0),MCT_Float3),MCT_Float3,TEXT("float3(%0.8f,%0.8f,%0.8f)"),X,Y,Z);
		}

		virtual INT Constant4(FLOAT X,FLOAT Y,FLOAT Z,FLOAT W)
		{
			return AddUniformExpression(new FMaterialUniformExpressionConstant(FLinearColor(X,Y,Z,W),MCT_Float4),MCT_Float4,TEXT("float4(%0.8f,%0.8f,%0.8f,%0.8f)"),X,Y,Z,W);
		}

		virtual INT GameTime()
		{
			return AddUniformExpression(new FMaterialUniformExpressionTime(),MCT_Float,TEXT(""));
		}

		virtual INT RealTime()
		{
			return AddUniformExpression(new FMaterialUniformExpressionRealTime(),MCT_Float,TEXT(""));
		}

		virtual INT PeriodicHint(INT PeriodicCode)
		{
			if(PeriodicCode == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			if(GetParameterUniformExpression(PeriodicCode))
			{
				return AddUniformExpression(new FMaterialUniformExpressionPeriodic(GetParameterUniformExpression(PeriodicCode)),GetParameterType(PeriodicCode),TEXT("%s"),GetParameterCode(PeriodicCode));
			}
			else
			{
				return PeriodicCode;
			}
		}

		virtual INT Sine(INT X)
		{
			if(X == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			if(GetParameterUniformExpression(X))
			{
				return AddUniformExpression(new FMaterialUniformExpressionSine(GetParameterUniformExpression(X),0),MCT_Float,TEXT("sin(%s)"),*CoerceParameter(X,MCT_Float));
			}
			else
			{
				return AddCodeChunk(MCT_Float,0,GetTextureDependencyLength(X),TEXT("sin(%s)"),*CoerceParameter(X,MCT_Float));
			}
		}

		virtual INT Cosine(INT X)
		{
			if(X == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			if(GetParameterUniformExpression(X))
			{
				return AddUniformExpression(new FMaterialUniformExpressionSine(GetParameterUniformExpression(X),1),MCT_Float,TEXT("cos(%s)"),*CoerceParameter(X,MCT_Float));
			}
			else
			{
				return AddCodeChunk(MCT_Float,0,GetTextureDependencyLength(X),TEXT("cos(%s)"),*CoerceParameter(X,MCT_Float));
			}
		}

		virtual INT Floor(INT X)
		{
			if(X == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			if(GetParameterUniformExpression(X))
			{
				return AddUniformExpression(new FMaterialUniformExpressionFloor(GetParameterUniformExpression(X)),GetParameterType(X),TEXT("floor(%s)"),GetParameterCode(X));
			}
			else
			{
				return AddCodeChunk(GetParameterType(X),0,GetTextureDependencyLength(X),TEXT("floor(%s)"),GetParameterCode(X));
			}
		}

		virtual INT Ceil(INT X)
		{
			if(X == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			if(GetParameterUniformExpression(X))
			{
				return AddUniformExpression(new FMaterialUniformExpressionCeil(GetParameterUniformExpression(X)),GetParameterType(X),TEXT("ceil(%s)"),GetParameterCode(X));
			}
			else
			{
				return AddCodeChunk(GetParameterType(X),0,GetTextureDependencyLength(X),TEXT("ceil(%s)"),GetParameterCode(X));
			}
		}

		virtual INT Frac(INT X)
		{
			if(X == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			if(GetParameterUniformExpression(X))
			{
				return AddUniformExpression(new FMaterialUniformExpressionFrac(GetParameterUniformExpression(X)),GetParameterType(X),TEXT("frac(%s)"),GetParameterCode(X));
			}
			else
			{
				return AddCodeChunk(GetParameterType(X),0,GetTextureDependencyLength(X),TEXT("frac(%s)"),GetParameterCode(X));
			}
		}

		/**
		* Creates the new shader code chunk needed for the Abs expression
		*
		* @param	X - Index to the FMaterialCompiler::CodeChunk entry for the input expression
		* @return	Index to the new FMaterialCompiler::CodeChunk entry for this expression
		*/	
		virtual INT Abs( INT X )
		{
			if(X == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			// get the user input struct for the input expression
			FMaterialUniformExpression* pInputParam = GetParameterUniformExpression(X);
			if( pInputParam )
			{
				FMaterialUniformExpressionAbs* pUniformExpression = new FMaterialUniformExpressionAbs( pInputParam );
				return AddUniformExpression( pUniformExpression, GetParameterType(X), TEXT("abs(%s)"), GetParameterCode(X) );
			}
			else
			{
				return AddCodeChunk( GetParameterType(X), 0, GetTextureDependencyLength(X), TEXT("abs(%s)"), GetParameterCode(X) );
			}
		}

		virtual INT ReflectionVector()
		{
			return AddCodeChunk(MCT_Float3,0,0,TEXT("Parameters.TangentReflectionVector"));
		}

		virtual INT CameraVector()
		{
			return AddCodeChunk(MCT_Float3,0,0,TEXT("Parameters.TangentCameraVector"));
		}

		virtual INT LightVector()
		{
			return AddCodeChunk(MCT_Float3,0,0,TEXT("Parameters.TangentLightVector"));
		}

		virtual INT ScreenPosition(  UBOOL bScreenAlign )
		{
			if( bScreenAlign )
			{
				return AddCodeChunk(MCT_Float4,0,0,TEXT("ScreenAlignedPosition(Parameters.ScreenPosition)"));		
			}
			else
			{
				return AddCodeChunk(MCT_Float4,0,0,TEXT("Parameters.ScreenPosition"));		
			}	
		}

		virtual INT If(INT A,INT B,INT AGreaterThanB,INT AEqualsB,INT ALessThanB)
		{
			if(A == INDEX_NONE || B == INDEX_NONE || AGreaterThanB == INDEX_NONE || AEqualsB == INDEX_NONE || ALessThanB == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			EMaterialValueType ResultType = GetArithmeticResultType(GetParameterType(AGreaterThanB),GetArithmeticResultType(AEqualsB,ALessThanB));

			INT CoercedAGreaterThanB = ForceCast(AGreaterThanB,ResultType);
			INT CoercedAEqualsB = ForceCast(AEqualsB,ResultType);
			INT CoercedALessThanB = ForceCast(ALessThanB,ResultType);

			return AddCodeChunk(
				ResultType,
				0,
				::Max(GetTextureDependencyLengths(A,B),GetTextureDependencyLengths(AGreaterThanB,AEqualsB,ALessThanB)),
				TEXT("((%s >= %s) ? (%s > %s ? %s : %s) : %s)"),
				GetParameterCode(A),
				GetParameterCode(B),
				GetParameterCode(A),
				GetParameterCode(B),
				GetParameterCode(CoercedAGreaterThanB),
				GetParameterCode(CoercedAEqualsB),
				GetParameterCode(CoercedALessThanB)
				);
		}

		virtual INT TextureCoordinate(UINT CoordinateIndex)
		{
			//Material->NumUserTexCoords = ::Max(CoordinateIndex + 1,Material->NumUserTexCoords);
			UINT NumUserTexCoords =  ::Max(CoordinateIndex + 1,Material->GetUserTexCoordsUsed());
			Material->SetUserTexCoordsUsed( NumUserTexCoords );
			return AddCodeChunk(MCT_Float2,0,0,TEXT("Parameters.TexCoords[%u].xy"),CoordinateIndex);
		}

		virtual INT TextureSample(INT TextureIndex,INT CoordinateIndex)
		{
			if(TextureIndex == INDEX_NONE || CoordinateIndex == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			EMaterialValueType	TextureType = GetParameterType(TextureIndex);
			DWORD				Flags		= GetParameterFlags(TextureIndex);

			FString				SampleCode;

			switch(TextureType)
			{
			case MCT_Texture2D:
				SampleCode = TEXT("tex2D(%s,%s)");
				break;
			case MCT_TextureCube:
				SampleCode = TEXT("texCUBE(%s,%s)");
				break;
			}

			if( Flags & SCF_RGBE_4BIT_EXPONENT )
			{
				SampleCode = FString::Printf( TEXT("ExpandCompressedRGBE(%s)"), *SampleCode );
			}

			if( Flags & SCF_RGBE_8BIT_EXPONENT )
			{
				SampleCode = FString::Printf( TEXT("ExpandRGBE(%s)"), *SampleCode );
			}

			if( Flags & SCF_RGBL )
			{
				SampleCode = FString::Printf( TEXT("DecodeRGBL(%s)"), *SampleCode );
			}

			if( Flags & SCF_REQUIRES_GAMMA_CORRECTION )
			{
				SampleCode = FString::Printf( TEXT("GammaCorrect(%s)"), *SampleCode );
			}

			switch(TextureType)
			{
			case MCT_Texture2D:
				return AddCodeChunk(
						MCT_Float4,
						0,
						GetTextureDependencyLength(CoordinateIndex) + 1,
						*SampleCode,
						*CoerceParameter(TextureIndex,MCT_Texture2D),
						*CoerceParameter(CoordinateIndex,MCT_Float2)
						);
			case MCT_TextureCube:
				return AddCodeChunk(
						MCT_Float4,
						0,
						GetTextureDependencyLength(CoordinateIndex) + 1,
						*SampleCode,
						*CoerceParameter(TextureIndex,MCT_TextureCube),
						*CoerceParameter(CoordinateIndex,MCT_Float3)
						);
			default:
				Errorf(TEXT("Sampling unknown texture type: %s"),DescribeType(TextureType));
				return INDEX_NONE;
			};
		}

		/**
		* Add the shader code for sampling from the scene texture
		* @param	TexType - scene texture type to sample from
		* @param	CoordinateIdx - index of shader code for user specified tex coords
		*/
		virtual INT EnvCube( INT CoordinateIndex )
		{			
			//Material->bUsesEnvCube = TRUE;
			Material->SetUsesEnvCube( TRUE );

			INT TextureIndex = AddUniformExpression(new FMaterialUniformExpressionEnvCube,0,MCT_TextureCube,TEXT(""));

			// sampler
			FString	SampleCode( TEXT("DecodeRGBL(texCUBE(%s,%s))") );
			FString ReflectionVector(TEXT("Parameters.TangentReflectionVector"));
			FString TexCoordCode( (CoordinateIndex != INDEX_NONE) ? CoerceParameter(CoordinateIndex,MCT_Float3) : ReflectionVector );
			
			// add the code string			
			return AddCodeChunk(
				MCT_Float4,
				0,
				GetTextureDependencyLength(CoordinateIndex) + 1,
				*SampleCode,
				*CoerceParameter(TextureIndex,MCT_TextureCube),
				*TexCoordCode
				);			
		}

		/**
		* Add the shader code for sampling from the scene texture
		* @param	TexType - scene texture type to sample from
        * @param	CoordinateIdx - index of shader code for user specified tex coords
		*/
		virtual INT SceneTextureSample( BYTE TexType, INT CoordinateIdx )
		{
			//Material->bUsesSceneColor = TRUE;
			Material->SetUsesSceneColor( TRUE );

			// use the scene texture type
			FString SceneTexCode;
			switch(TexType)
			{
			case SceneTex_Lighting:
				SceneTexCode = FString(TEXT("SceneColorTexture"));
				break;
			default:
				Errorf(TEXT("Scene texture type not supported."));
				return INDEX_NONE;
			}
			// sampler
			FString	SampleCode( TEXT("tex2D(%s,%s)") );
			// replace default tex coords with user specified coords if available
			FString DefaultScreenAligned(TEXT("float2(ScreenAlignedPosition(Parameters.ScreenPosition).xy)"));
			FString TexCoordCode( (CoordinateIdx != INDEX_NONE) ? CoerceParameter(CoordinateIdx,MCT_Float2) : DefaultScreenAligned );
			// add the code string
			return AddCodeChunk(
				MCT_Float4,
				0,
				GetTextureDependencyLength(CoordinateIdx) + 1,
				*SampleCode,
				*SceneTexCode,
				*TexCoordCode
				);
		}

		/**
		* Add the shader code for sampling the scene depth
		* @param	bNormalize - @todo implement
		* @param	CoordinateIdx - index of shader code for user specified tex coords
		*/
		virtual INT SceneTextureDepth( UBOOL bNormalize, INT CoordinateIdx)
		{
			//Material->bUsesSceneDepth = TRUE;
			Material->SetUsesSceneDepth( TRUE );


			// sampler
			FString	UserDepthCode( TEXT("CalcSceneDepth(%s)") );
			// replace default tex coords with user specified coords if available
			FString DefaultScreenAligned(TEXT("float2(ScreenAlignedPosition(Parameters.ScreenPosition).xy)"));
			FString TexCoordCode( (CoordinateIdx != INDEX_NONE) ? CoerceParameter(CoordinateIdx,MCT_Float2) : DefaultScreenAligned );
			// add the code string
			return AddCodeChunk(
				MCT_Float1,
				0,
				GetTextureDependencyLength(CoordinateIdx) + 1,
				*UserDepthCode,
				*TexCoordCode
				);
		}

		virtual INT PixelDepth(UBOOL bNormalize)
		{
			return AddCodeChunk(MCT_Float1, 0, 0, TEXT("Parameters.ScreenPosition.w"));		
		}

		virtual INT DestColor()
		{
			// note: can just call
			// SceneTextureSample(SceneTex_Lighting,INDEX_NONE);

			//Material->bUsesSceneColor = TRUE;
			Material->SetUsesSceneColor( TRUE );

			FString	UserColorCode(TEXT("PreviousLighting(%s)"));
			FString	ScreenPosCode(TEXT("Parameters.ScreenPosition"));
			// add the code string
			return AddCodeChunk(
				MCT_Float4,
				0,
				1,
				*UserColorCode,
				*ScreenPosCode
				);
		}

		virtual INT DestDepth(UBOOL bNormalize)
		{
			// note: can just call
			// SceneTextureDepth(FALSE,INDEX_NONE);

			//Material->bUsesSceneDepth = TRUE;
			Material->SetUsesSceneDepth( TRUE );

			FString	UserDepthCode(TEXT("PreviousDepth(%s)"));
			FString	ScreenPosCode(TEXT("Parameters.ScreenPosition"));
			// add the code string
			return AddCodeChunk(
				MCT_Float1,
				0,
				1,
				*UserDepthCode,
				*ScreenPosCode
				);
		}

		/**
		* Generates a shader code chunk for the DepthBiasedAlpha expression
		* using the given inputs
		* @param SrcAlphaIdx = index to source alpha input expression code chunk
		* @param BiasIdx = index to bias input expression code chunk
		* @param BiasScaleIdx = index to a scale expression code chunk to apply to the bias
		*/
		virtual INT DepthBiasedAlpha( INT SrcAlphaIdx, INT BiasIdx, INT BiasScaleIdx )
		{
			INT ResultIdx = INDEX_NONE;

			// all inputs must be valid expressions
			if ((SrcAlphaIdx != INDEX_NONE) &&
				(BiasIdx != INDEX_NONE) &&
				(BiasScaleIdx != INDEX_NONE))
			{
				FString CodeChunk(TEXT("DepthBiasedAlpha(Parameters,%s,%s,%s)"));
				ResultIdx = AddCodeChunk(
					MCT_Float1,
					0,
					::Max(GetTextureDependencyLengths(SrcAlphaIdx,BiasIdx,BiasScaleIdx),1),
					*CodeChunk,
					*CoerceParameter(SrcAlphaIdx,MCT_Float1),
					*CoerceParameter(BiasIdx,MCT_Float1),
					*CoerceParameter(BiasScaleIdx,MCT_Float1)
					);
			}

			return ResultIdx;
		}

		/**
		* Generates a shader code chunk for the DepthBiasedBlend expression
		* using the given inputs
		* @param SrcColorIdx = index to source color input expression code chunk
		* @param BiasIdx = index to bias input expression code chunk
		* @param BiasScaleIdx = index to a scale expression code chunk to apply to the bias
		*/
		virtual INT DepthBiasedBlend( INT SrcColorIdx, INT BiasIdx, INT BiasScaleIdx )
		{
			INT ResultIdx = INDEX_NONE;

			// all inputs must be valid expressions
			if( SrcColorIdx != INDEX_NONE && 
				BiasIdx != INDEX_NONE &&
				BiasScaleIdx != INDEX_NONE )
			{
				FString CodeChunk( TEXT("DepthBiasedBlend(Parameters,%s,%s,%s)") );
				ResultIdx = AddCodeChunk(
					MCT_Float3,
					0,
					::Max(GetTextureDependencyLengths(SrcColorIdx,BiasIdx,BiasScaleIdx),1),
					*CodeChunk,
					*CoerceParameter(SrcColorIdx,MCT_Float3),
					*CoerceParameter(BiasIdx,MCT_Float1),
					*CoerceParameter(BiasScaleIdx,MCT_Float1)
					);
			}

			return ResultIdx;			
		}

		virtual INT Texture(UTexture* Texture)
		{
			EMaterialValueType ShaderType = Texture->GetMaterialType();
			DWORD Flags = 0;

			BYTE PixelFormat = PF_Unknown;
			UTexture2D* Tex2D = Cast<UTexture2D>(Texture);
			if (Tex2D)
			{
				PixelFormat = Tex2D->Format;
			}

			if(Texture->RGBE)
			{
				Flags |= PixelFormat == PF_A8R8G8B8 ? SCF_RGBE_8BIT_EXPONENT : SCF_RGBE_4BIT_EXPONENT;
			}
			if(Texture->RGBL)
			{
				Flags |= SCF_RGBL;
			}
			if( (GPixelFormats[PixelFormat].Flags & PF_REQUIRES_GAMMA_CORRECTION) && Texture->SRGB )
			{
				Flags |= SCF_REQUIRES_GAMMA_CORRECTION;
			}

			return AddUniformExpression(new FMaterialUniformExpressionTexture(Texture),Flags,ShaderType,TEXT(""));
		}

		virtual INT TextureParameter(FName ParameterName,UTexture* DefaultValue)
		{
			EMaterialValueType ShaderType = DefaultValue->GetMaterialType();
			DWORD Flags = 0;

			BYTE PixelFormat = PF_Unknown;
			UTexture2D* Tex2D = Cast<UTexture2D>(DefaultValue);
			if (Tex2D)
			{
				PixelFormat = Tex2D->Format;
			}

			if(DefaultValue->RGBE)
			{
				Flags |= PixelFormat == PF_A8R8G8B8 ? SCF_RGBE_8BIT_EXPONENT : SCF_RGBE_4BIT_EXPONENT;
			}
			if(DefaultValue->RGBL)
			{
				Flags |= SCF_RGBL;
			}
			if( (GPixelFormats[PixelFormat].Flags & PF_REQUIRES_GAMMA_CORRECTION) && DefaultValue->SRGB )
			{
				Flags |= SCF_REQUIRES_GAMMA_CORRECTION;
			}

			return AddUniformExpression(new FMaterialUniformExpressionTextureParameter(ParameterName,DefaultValue),Flags,ShaderType,TEXT(""));
		}

		virtual INT VertexColor()
		{
			return AddCodeChunk(MCT_Float4,0,0,TEXT("Parameters.VertexColor"));
		}

		virtual INT Add(INT A,INT B)
		{
			if(A == INDEX_NONE || B == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			if(GetParameterUniformExpression(A) && GetParameterUniformExpression(B))
			{
				return AddUniformExpression(new FMaterialUniformExpressionFoldedMath(GetParameterUniformExpression(A),GetParameterUniformExpression(B),FMO_Add),GetArithmeticResultType(A,B),TEXT("(%s + %s)"),GetParameterCode(A),GetParameterCode(B));
			}
			else
			{
				return AddCodeChunk(GetArithmeticResultType(A,B),0,GetTextureDependencyLengths(A,B),TEXT("(%s + %s)"),GetParameterCode(A),GetParameterCode(B));
			}
		}

		virtual INT Sub(INT A,INT B)
		{
			if(A == INDEX_NONE || B == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			if(GetParameterUniformExpression(A) && GetParameterUniformExpression(B))
			{
				return AddUniformExpression(new FMaterialUniformExpressionFoldedMath(GetParameterUniformExpression(A),GetParameterUniformExpression(B),FMO_Sub),GetArithmeticResultType(A,B),TEXT("(%s - %s)"),GetParameterCode(A),GetParameterCode(B));
			}
			else
			{
				return AddCodeChunk(GetArithmeticResultType(A,B),0,GetTextureDependencyLengths(A,B),TEXT("(%s - %s)"),GetParameterCode(A),GetParameterCode(B));
			}
		}

		virtual INT Mul(INT A,INT B)
		{
			if(A == INDEX_NONE || B == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			if(GetParameterUniformExpression(A) && GetParameterUniformExpression(B))
			{
				return AddUniformExpression(new FMaterialUniformExpressionFoldedMath(GetParameterUniformExpression(A),GetParameterUniformExpression(B),FMO_Mul),GetArithmeticResultType(A,B),TEXT("(%s * %s)"),GetParameterCode(A),GetParameterCode(B));
			}
			else
			{
				return AddCodeChunk(GetArithmeticResultType(A,B),0,GetTextureDependencyLengths(A,B),TEXT("(%s * %s)"),GetParameterCode(A),GetParameterCode(B));
			}
		}

		virtual INT Div(INT A,INT B)
		{
			if(A == INDEX_NONE || B == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			if(GetParameterUniformExpression(A) && GetParameterUniformExpression(B))
			{
				return AddUniformExpression(new FMaterialUniformExpressionFoldedMath(GetParameterUniformExpression(A),GetParameterUniformExpression(B),FMO_Div),GetArithmeticResultType(A,B),TEXT("(%s / %s)"),GetParameterCode(A),GetParameterCode(B));
			}
			else
			{
				return AddCodeChunk(GetArithmeticResultType(A,B),0,GetTextureDependencyLengths(A,B),TEXT("(%s / %s)"),GetParameterCode(A),GetParameterCode(B));
			}
		}

		virtual INT Dot(INT A,INT B)
		{
			if(A == INDEX_NONE || B == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			if(GetParameterUniformExpression(A) && GetParameterUniformExpression(B))
			{
				return AddUniformExpression(new FMaterialUniformExpressionFoldedMath(GetParameterUniformExpression(A),GetParameterUniformExpression(B),FMO_Dot),MCT_Float,TEXT("dot(%s,%s)"),GetParameterCode(A),GetParameterCode(B));
			}
			else
			{
				return AddCodeChunk(MCT_Float,0,GetTextureDependencyLengths(A,B),TEXT("dot(%s,%s)"),GetParameterCode(A),*CoerceParameter(B,GetParameterType(A)));
			}
		}

		virtual INT Cross(INT A,INT B)
		{
			if(A == INDEX_NONE || B == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			return AddCodeChunk(MCT_Float3,0,GetTextureDependencyLengths(A,B),TEXT("cross(%s,%s)"),*CoerceParameter(A,MCT_Float3),*CoerceParameter(B,MCT_Float3));
		}

		virtual INT Power(INT Base,INT Exponent)
		{
			if(Base == INDEX_NONE || Exponent == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			return AddCodeChunk(GetParameterType(Base),0,GetTextureDependencyLengths(Base,Exponent),TEXT("pow(%s,%s)"),GetParameterCode(Base),*CoerceParameter(Exponent,MCT_Float));;
		}

		virtual INT SquareRoot(INT X)
		{
			if(X == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			if(GetParameterUniformExpression(X))
			{
				return AddUniformExpression(new FMaterialUniformExpressionSquareRoot(GetParameterUniformExpression(X)),MCT_Float,TEXT("sqrt(%s)"),*CoerceParameter(X,MCT_Float1));
			}
			else
			{
				return AddCodeChunk(MCT_Float,0,GetTextureDependencyLength(X),TEXT("sqrt(%s)"),*CoerceParameter(X,MCT_Float1));
			}
		}

		virtual INT Lerp(INT X,INT Y,INT A)
		{
			if(X == INDEX_NONE || Y == INDEX_NONE || A == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			EMaterialValueType ResultType = GetArithmeticResultType(X,Y);
			return AddCodeChunk(ResultType,0,GetTextureDependencyLengths(X,Y,A),TEXT("lerp(%s,%s,%s)"),*CoerceParameter(X,ResultType),*CoerceParameter(Y,ResultType),*CoerceParameter(A,MCT_Float1));
		}

		virtual INT Min(INT A,INT B)
		{
			if(A == INDEX_NONE || B == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			if(GetParameterUniformExpression(A) && GetParameterUniformExpression(B))
			{
				return AddUniformExpression(new FMaterialUniformExpressionMin(GetParameterUniformExpression(A),GetParameterUniformExpression(B)),GetParameterType(A),TEXT("min(%s,%s)"),GetParameterCode(A),*CoerceParameter(B,GetParameterType(A)));
			}
			else
			{
				return AddCodeChunk(GetParameterType(A),0,GetTextureDependencyLengths(A,B),TEXT("min(%s,%s)"),GetParameterCode(A),*CoerceParameter(B,GetParameterType(A)));
			}
		}

		virtual INT Max(INT A,INT B)
		{
			if(A == INDEX_NONE || B == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			if(GetParameterUniformExpression(A) && GetParameterUniformExpression(B))
			{
				return AddUniformExpression(new FMaterialUniformExpressionMax(GetParameterUniformExpression(A),GetParameterUniformExpression(B)),GetParameterType(A),TEXT("max(%s,%s)"),GetParameterCode(A),*CoerceParameter(B,GetParameterType(A)));
			}
			else
			{
				return AddCodeChunk(GetParameterType(A),0,GetTextureDependencyLengths(A,B),TEXT("max(%s,%s)"),GetParameterCode(A),*CoerceParameter(B,GetParameterType(A)));
			}
		}

		virtual INT Clamp(INT X,INT A,INT B)
		{
			if(X == INDEX_NONE || A == INDEX_NONE || B == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			if(GetParameterUniformExpression(X) && GetParameterUniformExpression(A) && GetParameterUniformExpression(B))
			{
				return AddUniformExpression(new FMaterialUniformExpressionClamp(GetParameterUniformExpression(X),GetParameterUniformExpression(A),GetParameterUniformExpression(B)),GetParameterType(X),TEXT("min(max(%s,%s),%s)"),GetParameterCode(X),*CoerceParameter(A,GetParameterType(X)),*CoerceParameter(B,GetParameterType(X)));
			}
			else
			{
				return AddCodeChunk(GetParameterType(X),0,GetTextureDependencyLengths(X,A,B),TEXT("min(max(%s,%s),%s)"),GetParameterCode(X),*CoerceParameter(A,GetParameterType(X)),*CoerceParameter(B,GetParameterType(X)));
			}
		}

		virtual INT ComponentMask(INT Vector,UBOOL R,UBOOL G,UBOOL B,UBOOL A)
		{
			if(Vector == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			EMaterialValueType	VectorType = GetParameterType(Vector);

			if(	A && (VectorType & MCT_Float) < MCT_Float4 ||
				B && (VectorType & MCT_Float) < MCT_Float3 ||
				G && (VectorType & MCT_Float) < MCT_Float2 ||
				R && (VectorType & MCT_Float) < MCT_Float1)
				Errorf(TEXT("Not enough components in (%s: %s) for component mask %u%u%u%u"),GetParameterCode(Vector),DescribeType(GetParameterType(Vector)),R,G,B,A);

			EMaterialValueType	ResultType;
			switch((R ? 1 : 0) + (G ? 1 : 0) + (B ? 1 : 0) + (A ? 1 : 0))
			{
				case 1: ResultType = MCT_Float; break;
				case 2: ResultType = MCT_Float2; break;
				case 3: ResultType = MCT_Float3; break;
				case 4: ResultType = MCT_Float4; break;
				default: Errorf(TEXT("Couldn't determine result type of component mask %u%u%u%u"),R,G,B,A); return INDEX_NONE;
			};

			return AddCodeChunk(
				ResultType,
				0,
				GetTextureDependencyLength(Vector),
				TEXT("%s.%s%s%s%s"),
				GetParameterCode(Vector),
				R ? TEXT("r") : TEXT(""),
				G ? TEXT("g") : TEXT(""),
				B ? TEXT("b") : TEXT(""),
				A ? TEXT("a") : TEXT("")
				);
		}

		virtual INT AppendVector(INT A,INT B)
		{
			if(A == INDEX_NONE || B == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			INT					NumResultComponents = GetNumComponents(GetParameterType(A)) + GetNumComponents(GetParameterType(B));
			EMaterialValueType	ResultType = GetVectorType(NumResultComponents);

			if(GetParameterUniformExpression(A) && GetParameterUniformExpression(B))
			{
				return AddUniformExpression(new FMaterialUniformExpressionAppendVector(GetParameterUniformExpression(A),GetParameterUniformExpression(B),GetNumComponents(GetParameterType(A))),ResultType,TEXT("float%u(%s,%s)"),NumResultComponents,GetParameterCode(A),GetParameterCode(B));
			}
			else
			{
				return AddCodeChunk(ResultType,0,GetTextureDependencyLengths(A,B),TEXT("float%u(%s,%s)"),NumResultComponents,GetParameterCode(A),GetParameterCode(B));
			}
		}

		/**
		* Generate shader code for transforming a vector
		*
		* @param	CoordType - type of transform to apply. see EMaterialVectorCoordTransform 
		* @param	A - index for input vector parameter's code
		*/
		virtual INT TransformVector(BYTE CoordType,INT A)
		{
			INT Result = INDEX_NONE;
			if(A != INDEX_NONE)
			{
				INT NumInputComponents = GetNumComponents(GetParameterType(A));
				// only allow float3/float4 transforms
				if( NumInputComponents < 3 )
				{
					Result = Errorf(TEXT("input must be a vector (%s: %s)"),GetParameterCode(A),DescribeType(GetParameterType(A)));
				}
				else
				{
					// code string to transform the input vector
					FString CodeStr;
					switch( CoordType )
					{
					case TRANSFORM_World:
						// transform from tangent to world space
						//Material->UsingTransforms |= UsedCoord_World;
						Material->SetTransformsUsed( Material->GetTransformsUsed() | UsedCoord_World );
						CodeStr = FString(TEXT("TransformIntoWorld(%s)"));
						//CodeStr = FString(TEXT("MulMatrix(LocalToWorldMatrix, mul(Parameters.TangentBasisInverse,%s))"));			
						break;
					case TRANSFORM_View:
						// transform from tangent to view space
						//Material->UsingTransforms |= UsedCoord_View;
						Material->SetTransformsUsed( Material->GetTransformsUsed() | UsedCoord_View );
						CodeStr = FString(TEXT("TransformIntoView(%s)"));
						//CodeStr = FString(TEXT("MulMatrix(WorldToViewMatrix,MulMatrix(LocalToWorldMatrix,mul(Parameters.TangentBasisInverse,%s)))"));
						break;
					case TRANSFORM_Local:
						// transform from tangent to local space
						//Material->UsingTransforms |= UsedCoord_Local;
						Material->SetTransformsUsed( Material->GetTransformsUsed() | UsedCoord_Local );
						CodeStr = FString(TEXT("TransformIntoLocal(%s)"));
						//CodeStr = FString(TEXT("MulMatrix(Parameters.TangentBasisInverse,%s)"));
						break;
					default:
						appErrorf( TEXT("Invalid CoordType. See EMaterialVectorCoordTransform") );
					}

					// we are only transforming vectors (not points) so only return a float3
					Result = AddCodeChunk(
						MCT_Float3,
						0,
						GetTextureDependencyLength(A),
						*CodeStr,
						*CoerceParameter(A,MCT_Float3)
						);
				}
			}
			return Result; 
		}

		INT LensFlareIntesity()
		{
			INT ResultIdx = INDEX_NONE;
			FString CodeChunk(TEXT("GetLensFlareIntensity(Parameters)"));
			ResultIdx = AddCodeChunk(
				MCT_Float1,
				0,
				0,
				*CodeChunk
				);
			return ResultIdx;
		}

		INT LensFlareRadialDistance()
		{
			INT ResultIdx = INDEX_NONE;
			FString CodeChunk(TEXT("GetLensFlareRadialDistance(Parameters)"));
			ResultIdx = AddCodeChunk(
				MCT_Float1,
				0,
				0,
				*CodeChunk
				);
			return ResultIdx;
		}

		INT LensFlareRayDistance()
		{
			INT ResultIdx = INDEX_NONE;
			FString CodeChunk(TEXT("GetLensFlareRayDistance(Parameters)"));
			ResultIdx = AddCodeChunk(
				MCT_Float1,
				0,
				0,
				*CodeChunk
				);
			return ResultIdx;
		}

		INT LensFlareSourceDistance()
		{
			INT ResultIdx = INDEX_NONE;
			FString CodeChunk(TEXT("GetLensFlareSourceDistance(Parameters)"));
			ResultIdx = AddCodeChunk(
				MCT_Float1,
				0,
				0,
				*CodeChunk
				);
			return ResultIdx;
		}
	};

	
	//<@ ava specific ; 2007. 1. 26. changmin
	// generate all platform expressions
	const EShaderPlatform CurrentPlatform = GRHIShaderPlatform;
	//FString PlatformString[SP_NumPlatforms] =
	//{
	//	TEXT("PD3D"),
	//	TEXT("PS3"),
	//	TEXT("XBOXD3D"),
	//	TEXT("PCD3D_SM2"),
	//	TEXT("PCD3D_SM2_POOR")
	//};
	if (GIsEditor || !GIsGame)
	{
		for( INT PlatformIndex = 0; PlatformIndex < SP_NumPlatforms; ++PlatformIndex )
		{
			if (GIsGame)
			{
				if (PlatformIndex != GRHIShaderPlatform)
					continue;
			}
			else
			{
				GRHIShaderPlatform		= (EShaderPlatform)PlatformIndex;
			}

			GShaderCompilePlatform	= (EShaderPlatform)PlatformIndex;
			FHLSLMaterialTranslator MaterialTranslator(this);
			INT NormalChunk					= MaterialTranslator.ForceCast(CompileProperty(MP_Normal				,&MaterialTranslator),MCT_Float3);
			INT EmissiveColorChunk			= MaterialTranslator.ForceCast(CompileProperty(MP_EmissiveColor			,&MaterialTranslator),MCT_Float3);
			INT DiffuseColorChunk			= MaterialTranslator.ForceCast(CompileProperty(MP_DiffuseColor			,&MaterialTranslator),MCT_Float3);
			INT SpecularColorChunk			= MaterialTranslator.ForceCast(CompileProperty(MP_SpecularColor			,&MaterialTranslator),MCT_Float3);
			INT SpecularPowerChunk			= MaterialTranslator.ForceCast(CompileProperty(MP_SpecularPower			,&MaterialTranslator),MCT_Float1);
			INT OpacityChunk				= MaterialTranslator.ForceCast(CompileProperty(MP_Opacity				,&MaterialTranslator),MCT_Float1);
			INT MaskChunk					= MaterialTranslator.ForceCast(CompileProperty(MP_OpacityMask			,&MaterialTranslator),MCT_Float1);
			INT DistortionChunk				= MaterialTranslator.ForceCast(CompileProperty(MP_Distortion			,&MaterialTranslator),MCT_Float2);
			INT TwoSidedLightingMaskChunk	= MaterialTranslator.ForceCast(CompileProperty(MP_TwoSidedLightingMask	,&MaterialTranslator),MCT_Float3);
			INT CustomLightingChunk			= MaterialTranslator.ForceCast(CompileProperty(MP_CustomLighting		,&MaterialTranslator),MCT_Float3);
			INT	AmbientMaskChunk			= MaterialTranslator.ForceCast(CompileProperty(MP_AmbientMask			,&MaterialTranslator),MCT_Float1);
			/*debugf(NAME_Log, TEXT("Material : %s ( platform : %s, specular = %s, normal = %s, envcube = %s, mask = %s, distortion = %s )"), *GetFriendlyName(),
			*PlatformString[PlatformIndex],
			GetUsesSpecular()	? TEXT("TRUE") : TEXT("FALSE"),
			GetUsesNormal()		? TEXT("TRUE") : TEXT("FALSE"),
			GetUsesEnvCube()	? TEXT("TRUE") : TEXT("FALSE"),
			GetUsesMask()		? TEXT("TRUE") : TEXT("FALSE"),
			GetUsesDistortion()	? TEXT("TRUE") : TEXT("FALSE"));*/
		}	

		GRHIShaderPlatform = Platform;
	}
	
	//>@ ava

	// Generate the material shader code.	
	GShaderCompilePlatform = Platform;
	FHLSLMaterialTranslator MaterialTranslator(this);
	INT NormalChunk					= MaterialTranslator.ForceCast(CompileProperty(MP_Normal				,&MaterialTranslator),MCT_Float3);
	INT EmissiveColorChunk			= MaterialTranslator.ForceCast(CompileProperty(MP_EmissiveColor			,&MaterialTranslator),MCT_Float3);
	INT DiffuseColorChunk			= MaterialTranslator.ForceCast(CompileProperty(MP_DiffuseColor			,&MaterialTranslator),MCT_Float3);
	INT SpecularColorChunk			= MaterialTranslator.ForceCast(CompileProperty(MP_SpecularColor			,&MaterialTranslator),MCT_Float3);
	INT SpecularPowerChunk			= MaterialTranslator.ForceCast(CompileProperty(MP_SpecularPower			,&MaterialTranslator),MCT_Float1);
	INT OpacityChunk				= MaterialTranslator.ForceCast(CompileProperty(MP_Opacity				,&MaterialTranslator),MCT_Float1);
	INT MaskChunk					= MaterialTranslator.ForceCast(CompileProperty(MP_OpacityMask			,&MaterialTranslator),MCT_Float1);
	INT DistortionChunk				= MaterialTranslator.ForceCast(CompileProperty(MP_Distortion			,&MaterialTranslator),MCT_Float2);
	INT TwoSidedLightingMaskChunk	= MaterialTranslator.ForceCast(CompileProperty(MP_TwoSidedLightingMask	,&MaterialTranslator),MCT_Float3);
	INT CustomLightingChunk			= MaterialTranslator.ForceCast(CompileProperty(MP_CustomLighting		,&MaterialTranslator),MCT_Float3);
	INT	AmbientMaskChunk			= MaterialTranslator.ForceCast(CompileProperty(MP_AmbientMask			,&MaterialTranslator),MCT_Float1);	

	UBOOL bSuccess = MaterialTranslator.bSuccess;

	FString InputsString = TEXT("");
	//for(INT VectorIndex = 0;VectorIndex < UniformVectorExpressions.Num();VectorIndex++)
	for(INT VectorIndex = 0;VectorIndex < GetUniformVectorExpressions().Num();VectorIndex++)
	{
		InputsString += FString::Printf(TEXT("float4 UniformVector_%i;\r\n"),VectorIndex);
	}
	//for(INT ScalarIndex = 0;ScalarIndex < UniformScalarExpressions.Num();ScalarIndex++)
	for(INT ScalarIndex = 0;ScalarIndex < GetUniformScalarExpressions().Num();ScalarIndex++)
	{
		InputsString += FString::Printf(TEXT("float UniformScalar_%i;\r\n"),ScalarIndex);
	}
	//for(INT TextureIndex = 0;TextureIndex < Uniform2DTextureExpressions.Num();TextureIndex++)
	for(INT TextureIndex = 0;TextureIndex < GetUniform2DTextureExpressions().Num();TextureIndex++)
	{
		InputsString += FString::Printf(TEXT("sampler2D Texture2D_%i;\r\n"),TextureIndex);
	}
	//for(INT TextureIndex = 0;TextureIndex < UniformCubeTextureExpressions.Num();TextureIndex++)
	for(INT TextureIndex = 0;TextureIndex < GetUniformCubeTextureExpressions().Num();TextureIndex++)
	{
		InputsString += FString::Printf(TEXT("samplerCUBE TextureCube_%i;\r\n"),TextureIndex);
	}

	FString MaterialTemplate = LoadShaderSourceFile(TEXT("MaterialTemplate"));

	const TCHAR * NormalCodeChunk = MaterialTranslator.GetFixedParameterCode(NormalChunk);
	const TCHAR * EmissiveColorCodeChunk = MaterialTranslator.GetFixedParameterCode(EmissiveColorChunk);
	const TCHAR * DiffuseColorCodeChunk = MaterialTranslator.GetFixedParameterCode(DiffuseColorChunk);
	const TCHAR * SpecularColorCodeChunk = MaterialTranslator.GetFixedParameterCode(SpecularColorChunk);
	const TCHAR * SpecularPowerCodeChunk = MaterialTranslator.GetFixedParameterCode(SpecularPowerChunk);
	const TCHAR * OpacityCodeChunk = MaterialTranslator.GetFixedParameterCode(OpacityChunk);
	const TCHAR * MaskCodeChunk = MaterialTranslator.GetFixedParameterCode(MaskChunk);
	const TCHAR * AmbientMaskCodeChunk = MaterialTranslator.GetFixedParameterCode(AmbientMaskChunk);
	const TCHAR * DistortionCodeChunk = MaterialTranslator.GetFixedParameterCode(DistortionChunk);
	const TCHAR * TwoSidedLightingMaskCodeChunk = MaterialTranslator.GetFixedParameterCode(TwoSidedLightingMaskChunk);
	const TCHAR * CustomLightingCodeChunk = MaterialTranslator.GetFixedParameterCode(CustomLightingChunk);
	const TCHAR * OpacityMaskClipValue = *FString::Printf(TEXT("%.5f"),GetOpacityMaskClipValue());


	FString MaterialShaderCode = FString::Printf(
		*MaterialTemplate,
		GetUserTexCoordsUsed(), //UserTexCoords,
		*InputsString,
		NormalCodeChunk,
		EmissiveColorCodeChunk,
		DiffuseColorCodeChunk,
		SpecularColorCodeChunk,
		SpecularPowerCodeChunk,
		OpacityCodeChunk,
		MaskCodeChunk,
		*FString::Printf(TEXT("%.5f"),GetOpacityMaskClipValue()),
		DistortionCodeChunk,
		TwoSidedLightingMaskCodeChunk,
		CustomLightingCodeChunk,
		AmbientMaskCodeChunk
		);

	if(bSuccess)
	{
		bSuccess = FallbackCompile(Platform, OutShaderMap, MaterialShaderCode, bForceCompile, IsSM2Platform( Platform ));

		//if the material didn't compile unchanged and this is the SM2 platform, try to remove components of the material until it compiles
		if (!bSuccess && IsSM2Platform( Platform ))
		{
			//<@ avav specific ; 2007. 1. 23 changmin
			SetUsesSpecular( FALSE );
			//>@ ava
			//remove specular
			MaterialShaderCode = FString::Printf(
				*MaterialTemplate,
				GetUserTexCoordsUsed(), //UserTexCoords,
				*InputsString,
				NormalCodeChunk,
				EmissiveColorCodeChunk,
				DiffuseColorCodeChunk,
				TEXT("float3(0,0,0)"),
				TEXT("0"),
				OpacityCodeChunk,
				MaskCodeChunk,
				*FString::Printf(TEXT("%.5f"),GetOpacityMaskClipValue()),
				DistortionCodeChunk,
				TwoSidedLightingMaskCodeChunk,
				CustomLightingCodeChunk,
				AmbientMaskCodeChunk
				);

			//CompileErrors.Empty();
			GetCompileErrors().Empty();
			bSuccess = FallbackCompile(Platform, OutShaderMap, MaterialShaderCode, bForceCompile, TRUE);

			if (!bSuccess)
			{
				//<@ ava specific ; 2007. 1. 23 changmin
				SetUsesSpecular( FALSE );
				SetUsesNormal( FALSE );
				//>@ ava
				//remove specular and normal
				MaterialShaderCode = FString::Printf(
					*MaterialTemplate,
					GetUserTexCoordsUsed(), //UserTexCoords,
					*InputsString,
					TEXT("float3(0,0,1)"),
					EmissiveColorCodeChunk,
					DiffuseColorCodeChunk,
					TEXT("float3(0,0,0)"),
					TEXT("0"),
					OpacityCodeChunk,
					MaskCodeChunk,
					*FString::Printf(TEXT("%.5f"),GetOpacityMaskClipValue()),
					DistortionCodeChunk,
					TwoSidedLightingMaskCodeChunk,
					CustomLightingCodeChunk,
					AmbientMaskCodeChunk
					);

				//CompileErrors.Empty();
				GetCompileErrors().Empty();
				bSuccess = FallbackCompile(Platform, OutShaderMap, MaterialShaderCode, bForceCompile, TRUE);

				if (!bSuccess)
				{
					//remove specular, normal and diffuse
					MaterialShaderCode = FString::Printf(
						*MaterialTemplate,
						GetUserTexCoordsUsed(), //UserTexCoords,
						*InputsString,
						TEXT("float3(0,0,1)"),
						EmissiveColorCodeChunk,
						TEXT("float3(0,0,0)"),
						TEXT("float3(0,0,0)"),
						TEXT("0"),
						OpacityCodeChunk,
						MaskCodeChunk,
						*FString::Printf(TEXT("%.5f"),GetOpacityMaskClipValue()),
						DistortionCodeChunk,
						TwoSidedLightingMaskCodeChunk,
						CustomLightingCodeChunk,
						AmbientMaskCodeChunk
						);

					//CompileErrors.Empty();
					GetCompileErrors().Empty();
					bSuccess = FallbackCompile(Platform, OutShaderMap, MaterialShaderCode, bForceCompile, TRUE);
				}
			}

			if (!bSuccess)
			{
				debugf(TEXT("     SM2 Fallback for Material %s failed!"), *GetFriendlyName());//, *MaterialShaderCode);
			}
		}
	}

	// Restore RHI shader platform 
	if (GIsEditor || !GIsGame)
	{
		GRHIShaderPlatform = CurrentPlatform;
	}	

	return bSuccess;
}

/**
* Compiles OutShaderMap using the shader code from MaterialShaderCode on Platform
*
* @param OutShaderMap - the shader map to compile
* @param MaterialShaderCode - a filled out instance of MaterialTemplate.usf to compile
* @param bForceCompile - force discard previous results 
* @param bSilent - indicates that no error message should be outputted on shader compile failure
*/
UBOOL FMaterial::FallbackCompile( 
	 EShaderPlatform Platform, 
	 TRefCountPtr<FMaterialShaderMap>& OutShaderMap, 
	 FString MaterialShaderCode, 
	 UBOOL bForceCompile, 
	 UBOOL bSilent)
{
	GShaderCompilePlatform = Platform;

	FMaterialShaderMap* ExistingShaderMap = NULL;

	// if we want to force compile the material, there's no reason to check for an existing one
	if (bForceCompile)
	{
		UShaderCache::FlushId(Id, Platform);
		ShaderMap = NULL;
	}
	else
	{
		// see if it's already compiled
		ExistingShaderMap = FMaterialShaderMap::FindId(Id, Platform);
	}

	OutShaderMap = ExistingShaderMap;
	if (!OutShaderMap)
	{
		// Create a shader map for the material on the given platform
		OutShaderMap = new FMaterialShaderMap;
	}

	UBOOL bSuccess = TRUE;
	if(!ExistingShaderMap || !ExistingShaderMap->IsComplete(this))
	{
		// Compile the shaders for the material.
		TArray<FString> ShaderErrors;
		bSuccess = OutShaderMap->Compile(this,*MaterialShaderCode,Platform,ShaderErrors,bSilent);
		for(INT ErrorIndex = 0;ErrorIndex < ShaderErrors.Num();ErrorIndex++)
		{
			//CompileErrors.Add(NULL,*ShaderErrors(ErrorIndex));
			GetCompileErrors().Add(NULL,*ShaderErrors(ErrorIndex));
		}
	}

	if(bSuccess)
	{
		// Initialize the shaders.
		// Use lazy initialization in the editor; Init is called by FShader::Get***Shader.
		if(!GIsEditor)
		{
			OutShaderMap->BeginInit();
		}
	}
	else
	{
		// Clear the shader map if the compilation failed, to ensure that the incomplete shader map isn't used.
		OutShaderMap = NULL;
	}

	// Store that we have up date compilation output.
	//bValidCompilationOutput = TRUE;
	MaterialInfos[Platform].bValidCompilationOutput = TRUE;

	return bSuccess;
}

/**
 * Caches the material shaders for the current platform.
 */
void FMaterial::CacheShaders()
{
	//flush the render command queue before changing the ShaderMap, since the rendering thread may be reading from it
	FlushRenderingCommands();

	if(ShaderMap)
	{
		// If there's a shader map for this material, flush any shaders cached for it.
		UShaderCache::FlushId(Id);
		ShaderMap = NULL;
	}

	// Discard the ID and make a new one.
	Id = appCreateGuid();

	// Reset the compile errors array.
	//CompileErrors.Empty();
	GetCompileErrors().Empty();

	// Compile the material shaders for the current platform.
	Compile(GRHIShaderPlatform, ShaderMap);
}

/**
 * Should the shader for this material with the given platform, shader type and vertex 
 * factory type combination be compiled
 *
 * @param Platform		The platform currently being compiled for
 * @param ShaderType	Which shader is being compiled
 * @param VertexFactory	Which vertex factory is being compiled (can be NULL)
 *
 * @return TRUE if the shader should be compiled
 */
UBOOL FMaterial::ShouldCache(EShaderPlatform Platform, const FShaderType* ShaderType, const FVertexFactoryType* VertexFactoryType) const
{
	// @GEMINI_TODO: Hook up to the precompiler to see if this material is used with this vertex factory
	// to stop compiling shaders for materials that are never on the given vertex factory
	return TRUE;
}

//
// FColoredMaterialInstance implementation.
//

const FMaterial* FColoredMaterialInstance::GetMaterial() const
{
	return Parent->GetMaterial();
}

/** Rebuilds the information about all texture lookups. */
void FMaterial::RebuildTextureLookupInfo( UMaterial *Material )
{
	//<@ ava specific ; 2007. 1. 24 changmin
	FTextureLookupInfo &TextureLookups = MaterialInfos[GRHIShaderPlatform].TextureLookups;
	//>@ ava

	TextureLookups.Empty();

	INT NumExpressions = Material->Expressions.Num();
	for(INT ExpressionIndex = 0;ExpressionIndex < NumExpressions; ExpressionIndex++)
	{
		UMaterialExpression* Expression = Material->Expressions(ExpressionIndex);
		UMaterialExpressionTextureSample* TextureSample = Cast<UMaterialExpressionTextureSample>(Expression);
		UMaterialExpressionTextureSampleParameter2D* TextureSampleParameter = Cast<UMaterialExpressionTextureSampleParameter2D>(Expression);

		if(TextureSample)
		{
			FTextureLookup Lookup;
			Lookup.ResolutionMultiplier = 1.0f;
			Lookup.TexCoordIndex = 0;
			Lookup.TextureIndex = -1;

			//@TODO: Check to see if this texture lookup is actually used.

			if ( TextureSample->Coordinates.Expression )
			{
				UMaterialExpressionTextureCoordinate* TextureCoordinate =
					Cast<UMaterialExpressionTextureCoordinate>( TextureSample->Coordinates.Expression );
				if ( TextureCoordinate )
				{
					// Use the specified texcoord.
					Lookup.TexCoordIndex = TextureCoordinate->CoordinateIndex;
					Lookup.ResolutionMultiplier = TextureCoordinate->Tiling;
				}
				else
				{
					// Too complex texcoord expression, ignore.
					continue;
				}
			}

			// Find where the texture is stored in the Uniform2DTextureExpressions array.
			if ( TextureSampleParameter )
			{
				FMaterialUniformExpressionTextureParameter TextureExpression(TextureSampleParameter->ParameterName, TextureSampleParameter->Texture);
				//Lookup.TextureIndex = FindExpression( Uniform2DTextureExpressions, TextureExpression );
				Lookup.TextureIndex = FindExpression( GetUniform2DTextureExpressions(), TextureExpression );
			}
			else if ( TextureSample->Texture )
			{
				FMaterialUniformExpressionTexture TextureExpression(TextureSample->Texture);
				//Lookup.TextureIndex = FindExpression( Uniform2DTextureExpressions, TextureExpression );
				Lookup.TextureIndex = FindExpression( GetUniform2DTextureExpressions(), TextureExpression );
			}

			if ( Lookup.TextureIndex >= 0 )
			{
				TextureLookups.AddItem( Lookup );
			}
		}
	}
}

/** Returns the index to the Expression in the Expressions array, or -1 if not found. */
INT FMaterial::FindExpression( const TArray<TRefCountPtr<FMaterialUniformExpression> >&Expressions, const FMaterialUniformExpression &Expression )
{
	for (INT ExpressionIndex = 0; ExpressionIndex < Expressions.Num(); ++ExpressionIndex)
	{
		if ( Expressions(ExpressionIndex)->IsIdentical(&Expression) )
		{
			return ExpressionIndex;
		}
	}
	return -1;
}


/** Serialize a texture lookup info. */
void FMaterial::FTextureLookup::Serialize(FArchive& Ar)
{
	Ar << TexCoordIndex;
	Ar << TextureIndex;
	Ar << ResolutionMultiplier;
}

FArchive& operator<<(FArchive& Ar, FMaterial::FTextureLookup& Ref)
{
	Ref.Serialize( Ar );
	return Ar;
}

UBOOL FColoredMaterialInstance::GetVectorValue(const FName& ParameterName,FLinearColor* OutValue) const
{
	if(ParameterName == NAME_Color)
	{
		*OutValue = Color;
		return TRUE;
	}
	else
	{
		return Parent->GetVectorValue(ParameterName,OutValue);
	}
}

UBOOL FColoredMaterialInstance::GetScalarValue(const FName& ParameterName,FLOAT* OutValue) const
{
	return Parent->GetScalarValue(ParameterName,OutValue);
}

UBOOL FColoredMaterialInstance::GetTextureValue(const FName& ParameterName,const FTexture** OutValue) const
{
	return Parent->GetTextureValue(ParameterName,OutValue);
}

//
// FTexturedMaterialInstance implementation.
//

const FMaterial* FTexturedMaterialInstance::GetMaterial() const
{
	return Parent->GetMaterial();
}

UBOOL FTexturedMaterialInstance::GetVectorValue(const FName& ParameterName,FLinearColor* OutValue) const
{
	return Parent->GetVectorValue(ParameterName,OutValue);
}
	
UBOOL FTexturedMaterialInstance::GetScalarValue(const FName& ParameterName,FLOAT* OutValue) const
{
	return Parent->GetScalarValue(ParameterName,OutValue);
}

UBOOL FTexturedMaterialInstance::GetTextureValue(const FName& ParameterName,const FTexture** OutValue) const
{
	if(ParameterName == NAME_Texture)
	{
		*OutValue = Texture;
		return TRUE;
	}
	else
	{
		return Parent->GetTextureValue(ParameterName,OutValue);
	}
}
