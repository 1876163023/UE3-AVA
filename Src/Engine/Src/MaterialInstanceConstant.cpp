/*=============================================================================
	MaterialInstanceConstant.cpp: UMaterialInstanceConstant implementation.
	Copyright 2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineMaterialClasses.h"

IMPLEMENT_CLASS(UMaterialInstanceConstant);

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

/**
 * The resource used to render a UMaterialInstanceConstant.
 */
class FMaterialInstanceResource: public FMaterialInstance, public FDeferredCleanupInterface
{
public:

	struct DataType
	{
		UMaterialInstance* Parent;
		TMap<FName,FLinearColor> VectorParameterMap;
		TMap<FName,FLOAT> ScalarParameterMap;
		TMap<FName,const UTexture*> TextureParameterMap;
	};

	/** Initialization constructor. */
	FMaterialInstanceResource(UMaterialInstanceConstant* InOwner,UBOOL bInSelected):
		Owner(InOwner),
		bSelected(bInSelected),
		GameThreadParent(NULL)
	{}

	// FDeferredCleanupInterface
	virtual void FinishCleanup()
	{
		delete this;
	}

	// FMaterialInstance interface.
	virtual const FMaterial* GetMaterial() const
	{
		check(IsInRenderingThread());
		return Data.Parent->GetInstanceInterface(bSelected)->GetMaterial();
	}
	virtual UBOOL GetVectorValue(const FName& ParameterName,FLinearColor* OutValue) const
	{
		check(IsInRenderingThread());
		const FLinearColor* Value = Data.VectorParameterMap.Find(ParameterName);
		if(Value)
		{
			*OutValue = *Value;
			return 1;
		}
		else if(Data.Parent)
		{
			return Data.Parent->GetInstanceInterface(bSelected)->GetVectorValue(ParameterName,OutValue);
		}
		else
		{
			return FALSE;
		}
	}
	virtual UBOOL GetScalarValue(const FName& ParameterName,FLOAT* OutValue) const
	{
		check(IsInRenderingThread());
		const FLOAT* Value = Data.ScalarParameterMap.Find(ParameterName);
		if(Value)
		{
			*OutValue = *Value;
			return 1;
		}
		else if(Data.Parent)
		{
			return Data.Parent->GetInstanceInterface(bSelected)->GetScalarValue(ParameterName,OutValue);
		}
		else
		{
			return FALSE;
		}
	}
	virtual UBOOL GetTextureValue(const FName& ParameterName,const FTexture** OutValue) const
	{
		check(IsInRenderingThread());
		const UTexture* Value = Data.TextureParameterMap.FindRef(ParameterName);
		if(Value)
		{
			*OutValue = Value->Resource;
			return 1;
		}
		else if(Data.Parent)
		{
			return Data.Parent->GetInstanceInterface(bSelected)->GetTextureValue(ParameterName,OutValue);
		}
		else
		{
			return FALSE;
		}
	}

	// Accessors.
	void SetData(DataType InData)
	{
		Data = InData;
	}
	void SetGameThreadParent(UMaterialInstance* NewGameThreadParent)
	{
		if(GameThreadParent != NewGameThreadParent)
		{
			// Assign the new parent.
			GameThreadParent = NewGameThreadParent;
		}
	}

private:

	/** The UMaterialInstanceConstant which owns this resource. */
	UMaterialInstanceConstant* Owner;

	/** Whether this resource represents the selected version of the material instance. */
	UBOOL bSelected;

	/** The rendering thread material instance properties. */
	DataType Data;

	/** The game thread accessible parent of the material instance. */
	UMaterialInstance* GameThreadParent;
};

static void UpdateMICResources(UMaterialInstanceConstant* Instance)
{
	for(INT Selected = 0;Selected < 2;Selected++)
	{
		if(!Instance->Resources[Selected])
		{
			continue;
		}

		// Find the instance's parent.
		UMaterialInstance* Parent = NULL;
		if(Instance->Parent)
		{
			Parent = Instance->Parent;
		}

		// Don't use the instance's parent if it has a circular dependency on the instance.
		if(Parent && Parent->IsDependent(Instance))
		{
			Parent = NULL;
		}

		// If the instance doesn't have a valid parent, use the default material as the parent.
		if(!Parent)
		{
			if(GEngine && GEngine->DefaultMaterial)
			{
				Parent = GEngine->DefaultMaterial;
			}
			else
			{
				// A material instance was loaded with an invalid GEngine.
				// This is probably because loading the default properties for the GEngine class lead to a material instance being loaded
				// before GEngine has been created.  In this case, we'll just pull the default material config value straight from the INI.
				Parent = LoadObject<UMaterialInstance>(NULL,TEXT("engine-ini:Engine.Engine.DefaultMaterialName"),NULL,LOAD_None,NULL);
			}
		}

		check(Parent);

		// Set the FMaterialInstance's game thread parent.
		Instance->Resources[Selected]->SetGameThreadParent(Parent);

		// Enqueue a rendering command with the updated material instance data.
		TSetResourceDataContext<FMaterialInstanceResource> NewData(Instance->Resources[Selected]);
		NewData->Parent = Parent;
		for(INT ValueIndex = 0;ValueIndex < Instance->VectorParameterValues.Num();ValueIndex++)
		{
			NewData->VectorParameterMap.Set(
				Instance->VectorParameterValues(ValueIndex).ParameterName,
				Instance->VectorParameterValues(ValueIndex).ParameterValue
				);
		}
		for(INT ValueIndex = 0;ValueIndex < Instance->ScalarParameterValues.Num();ValueIndex++)
		{
			NewData->ScalarParameterMap.Set(
				Instance->ScalarParameterValues(ValueIndex).ParameterName,
				Instance->ScalarParameterValues(ValueIndex).ParameterValue
				);
		}
		for (INT ValueIndex = 0; ValueIndex < Instance->NewTextureParameterValues.Num(); ValueIndex++)
		{
			if(Instance->NewTextureParameterValues(ValueIndex).ParameterValue)
			{
				NewData->TextureParameterMap.Set(
					Instance->NewTextureParameterValues(ValueIndex).ParameterName,
					Instance->NewTextureParameterValues(ValueIndex).ParameterValue
					);
			}
		}
	}
}

UMaterialInstanceConstant::UMaterialInstanceConstant()
{
	// GIsUCCMake is not set when the class is initialized
	if(!GIsUCCMake && !HasAnyFlags(RF_ClassDefaultObject))
	{
		Resources[0] = new FMaterialInstanceResource(this,FALSE);
		if(GIsEditor)
		{
			Resources[1] = new FMaterialInstanceResource(this,TRUE);
		}
		UpdateMICResources(this);
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

UBOOL UMaterialInstanceConstant::IsDependent(UMaterialInstance* TestDependency)
{
	if(TestDependency == this)
	{
		return TRUE;
	}
	else if(Parent)
	{
		if(ReentrantFlag)
		{
			return TRUE;
		}

		FMICReentranceGuard	Guard(this);
		return Parent->IsDependent(TestDependency);
	}
	else
	{
		return FALSE;
	}
}

FMaterialInstance* UMaterialInstanceConstant::GetInstanceInterface(UBOOL Selected) const
{
	check(!Selected || GIsEditor);
	return Resources[Selected];
}

UPhysicalMaterial* UMaterialInstanceConstant::GetPhysicalMaterial()
{
	if(ReentrantFlag)
	{
		return GEngine->DefaultMaterial->GetPhysicalMaterial();
	}

	FMICReentranceGuard	Guard(this);
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
	UBOOL bSetParameterValue = FALSE;

	// Check for an existing value for the named parameter in the array.
	for (INT ValueIndex = 0;ValueIndex < VectorParameterValues.Num();ValueIndex++)
	{
		if (VectorParameterValues(ValueIndex).ParameterName == ParameterName)
		{
			VectorParameterValues(ValueIndex).ParameterValue = Value;
			bSetParameterValue = TRUE;
			break;
		}
	}

	if(!bSetParameterValue)
	{
		// If there's no element for the named parameter in array yet, add one.
		FVectorParameterValue*	NewParameterValue = new(VectorParameterValues) FVectorParameterValue;
		NewParameterValue->ParameterName = ParameterName;
		NewParameterValue->ParameterValue = Value;
	}

	// Update the material instance data in the rendering thread.
	UpdateMICResources(this);
}

void UMaterialInstanceConstant::SetScalarParameterValue(FName ParameterName, float Value)
{
	UBOOL bSetParameterValue = FALSE;

	// Check for an existing value for the named parameter in the array.
	for (INT ValueIndex = 0;ValueIndex < ScalarParameterValues.Num();ValueIndex++)
	{
		if (ScalarParameterValues(ValueIndex).ParameterName == ParameterName)
		{
			ScalarParameterValues(ValueIndex).ParameterValue = Value;
			bSetParameterValue = TRUE;
			break;
		}
	}

	if(!bSetParameterValue)
	{
		// If there's no element for the named parameter in array yet, add one.
		FScalarParameterValue*	NewParameterValue = new(ScalarParameterValues) FScalarParameterValue;
		NewParameterValue->ParameterName = ParameterName;
		NewParameterValue->ParameterValue = Value;
	}

	// Update the material instance data in the rendering thread.
	UpdateMICResources(this);
}

//
//  UMaterialInstanceConstant::SetTextureParameterValue
//
void UMaterialInstanceConstant::SetTextureParameterValue(FName ParameterName, UTexture* Value)
{
	UBOOL bSetParameterValue = FALSE;

	// Check for an existing value for the named parameter in the array.
	for(INT ValueIndex = 0;ValueIndex < NewTextureParameterValues.Num();ValueIndex++)
	{
		if(NewTextureParameterValues(ValueIndex).ParameterName == ParameterName)
		{
			NewTextureParameterValues(ValueIndex).ParameterValue = Value;
			bSetParameterValue = TRUE;
			break;
		}
	}

	if(!bSetParameterValue)
	{
		// If there's no element for the named parameter in array yet, add one.
		FTextureParameterValue*	NewParameterValue = new(NewTextureParameterValues) FTextureParameterValue;
		NewParameterValue->ParameterName = ParameterName;
		NewParameterValue->ParameterValue = Value;
		bPlatformDependencyDirty = TRUE;
	}

	// Update the material instance data in the rendering thread.
	UpdateMICResources(this);
}

/** Removes all parameter values */
void UMaterialInstanceConstant::ClearParameterValues()
{
	VectorParameterValues.Empty();
	ScalarParameterValues.Empty();
	NewTextureParameterValues.Empty();

	// Update the material instance data in the rendering thread.
	UpdateMICResources(this);
}

UBOOL UMaterialInstanceConstant::GetVectorParameterValue(FName ParameterName, FLinearColor& OutValue)
{
	if(ReentrantFlag)
	{
		return FALSE;
	}

	FLinearColor* Value = NULL;
	for (INT ValueIndex = 0;ValueIndex < VectorParameterValues.Num();ValueIndex++)
	{
		if (VectorParameterValues(ValueIndex).ParameterName == ParameterName)
		{
			Value = &VectorParameterValues(ValueIndex).ParameterValue;
			break;
		}
	}
	if(Value)
	{
		OutValue = *Value;
		return TRUE;
	}
	else if(Parent)
	{
		FMICReentranceGuard	Guard(this);
		return Parent->GetVectorParameterValue(ParameterName,OutValue);
	}
	else
	{
		return FALSE;
	}
}

UBOOL UMaterialInstanceConstant::GetScalarParameterValue(FName ParameterName, FLOAT& OutValue)
{
	if(ReentrantFlag)
	{
		return FALSE;
	}

	FLOAT* Value = NULL;
	for (INT ValueIndex = 0;ValueIndex < ScalarParameterValues.Num();ValueIndex++)
	{
		if (ScalarParameterValues(ValueIndex).ParameterName == ParameterName)
		{
			Value = &ScalarParameterValues(ValueIndex).ParameterValue;
			break;
		}
	}
	if(Value)
	{
		OutValue = *Value;
		return TRUE;
	}
	else if(Parent)
	{
		FMICReentranceGuard	Guard(this);
		return Parent->GetScalarParameterValue(ParameterName,OutValue);
	}
	else
	{
		return FALSE;
	}
}

UBOOL UMaterialInstanceConstant::GetTextureParameterValue(FName ParameterName, UTexture*& OutValue)
{
	if(ReentrantFlag)
	{
		return FALSE;
	}

	UTexture** Value = NULL;
	for (INT ValueIndex = 0;ValueIndex < NewTextureParameterValues.Num();ValueIndex++)
	{
		if (NewTextureParameterValues(ValueIndex).ParameterName == ParameterName)
		{
			Value = &NewTextureParameterValues(ValueIndex).ParameterValue;
			break;
		}
	}
	if(Value && *Value)
	{
		OutValue = *Value;
		return TRUE;
	}
	else if(Parent)
	{
		FMICReentranceGuard	Guard(this);
		return Parent->GetTextureParameterValue(ParameterName,OutValue);
	}
	else
	{
		return FALSE;
	}
}


void UMaterialInstanceConstant::SetParent(UMaterialInstance* NewParent)
{
	Parent = NewParent;
	UpdateMICResources(this);

	bPlatformDependencyDirty = TRUE;
}

void UMaterialInstanceConstant::PostLoad()
{
	// Ensure that the instance's parent is PostLoaded before the instance.
	if(Parent)
	{
		Parent->ConditionalPostLoad();
	}

	if(TextureParameterValues.Num() > 0)
	{
		NewTextureParameterValues = TextureParameterValues;
		TextureParameterValues.Empty();

		DeterminePlatformTextureDependencies();
	}

	// We have to make sure the resources are created for all used textures.
	for( INT ValueIndex=0; ValueIndex<NewTextureParameterValues.Num(); ValueIndex++ )
	{
		// Make sure the texture is postloaded so the resource isn't null.
		UTexture* Texture = NewTextureParameterValues(ValueIndex).ParameterValue;
		if( Texture )
		{
			Texture->ConditionalPostLoad();
		}
	}
 
	Super::PostLoad();

	UpdateMICResources(this);
}

void UMaterialInstanceConstant::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);
	UpdateMICResources(this);
}

void UMaterialInstanceConstant::FinishDestroy()
{
	if(!GIsUCCMake&&!HasAnyFlags(RF_ClassDefaultObject))
	{
		BeginCleanup(Resources[0]);
		if(GIsEditor)
		{
			BeginCleanup(Resources[1]);
		}
	}
	Super::FinishDestroy();
}

FArchive& operator<<( FArchive& Ar, FTextureParameterValue& E )
{
	Ar << E.PlatformDependency;
	Ar << E.ParameterName;

	const UBOOL bSkipLoadReferencedObject = GIsGame && Ar.IsLoading() && ((E.PlatformDependency & (1 << GRHIShaderPlatform)) == 0);

	if (bSkipLoadReferencedObject)
	{
		Ar.SetSkipObjectRef( TRUE );
	}

	Ar << E.ParameterValue;

	if (bSkipLoadReferencedObject)
	{
		Ar.SetSkipObjectRef( FALSE );
	}

	return Ar;
}

void UMaterialInstanceConstant::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if (bPlatformDependencyDirty)
	{
		DeterminePlatformTextureDependencies();
	}

	if (!(Ar.IsLoading() && Ar.LicenseeVer() < VER_AVA_DONOTLOAD_NONRELEVANT_TEXTURES))
	{
		Ar << NewTextureParameterValues;
	}	
}

void UMaterialInstanceConstant::DeterminePlatformTextureDependencies()
{
	bPlatformDependencyDirty = FALSE;	

	for (INT ValueIndex = 0; ValueIndex < NewTextureParameterValues.Num(); ValueIndex++)
	{
		NewTextureParameterValues(ValueIndex).PlatformDependency = 0xffffffff;
	}

	if (Parent)
	{
		UMaterial* Material = Parent->GetMaterial();

		if (Material)
		{
			struct FTextureCollectingMaterialCompiler : public FMaterialCompiler
			{
				UMaterial* Material;				
				TArray<UMaterialExpression*> ExpressionStack;				
				TArray<FName> TextureParameterNames;

				// Contstructor
				FTextureCollectingMaterialCompiler( UMaterial* InMaterial )
					:	Material( InMaterial )
				{}

				virtual INT Error(const TCHAR* Text) { return 0; }

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

				virtual INT Texture(UTexture* Texture) { return 0; }				

				virtual INT TextureParameter(FName ParameterName,UTexture* DefaultTexture) 
				{
					TextureParameterNames.AddUniqueItem( ParameterName );

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
			
			if (Material->MaterialResource)
			{
				// RHI를 바꾸기 때문에 어쩔 수 없음
				if (GIsEditor || !GIsGame)
				{
					FlushRenderingCommands();
				}				

				EShaderPlatform OldPlatform = GRHIShaderPlatform;
				for (INT PlatformIndex=0; PlatformIndex<SP_AllPlatforms; ++PlatformIndex)
				{
					// Game이 아니면 타 플랫폼에 대한 처리 skip :)
					if (!GIsEditor && GIsGame)
					{
						if (PlatformIndex != GRHIShaderPlatform)
							continue;			
					}
					else
					{
						GRHIShaderPlatform = (EShaderPlatform)PlatformIndex;	
					}

					GShaderCompilePlatform = GRHIShaderPlatform;

					FTextureCollectingMaterialCompiler Compiler( Material );

					FMaterial::AvaMaterialInfo OldInfo = Material->MaterialResource->MaterialInfos[GRHIShaderPlatform];

					for (INT MaterialPropertyIndex=0; MaterialPropertyIndex<MP_MAX; ++MaterialPropertyIndex)
					{
						Material->MaterialResource->CompileProperty( (EMaterialProperty)MaterialPropertyIndex, &Compiler );
					}					
					
					Material->MaterialResource->MaterialInfos[GRHIShaderPlatform] = OldInfo;

					/*
					debugf( NAME_Log, TEXT( "Determining dependencies for MIC %s"), *GetFullName() );

					for (INT i=0; i<Compiler.TextureParameterNames.Num(); ++i)
					{
						debugf( NAME_Log, TEXT("  %s"), *Compiler.TextureParameterNames(i).ToString() );
					}
					*/

					for (INT ValueIndex = 0; ValueIndex < NewTextureParameterValues.Num(); ValueIndex++)
					{
						INT Index;
						if (!Compiler.TextureParameterNames.FindItem( NewTextureParameterValues(ValueIndex).ParameterName, Index ))
						{							
							NewTextureParameterValues(ValueIndex).PlatformDependency &= ~(1<<PlatformIndex);

							// Game에서는 obj gc를 통해 날릴 수 있는 체크 목적으로 NULL로 set해봄.
							if (GIsGame && (PlatformIndex == OldPlatform))
							{
								debugf( NAME_Log, TEXT("%s isn't needed for platform %d : %s"), *NewTextureParameterValues(ValueIndex).ParameterName.ToString(), PlatformIndex, NewTextureParameterValues(ValueIndex).ParameterValue ? *NewTextureParameterValues(ValueIndex).ParameterValue->GetFullName() : TEXT("(none)") );
								
								NewTextureParameterValues(ValueIndex).ParameterValue = NULL;
							}
						}
					}
				}								

				if (GIsEditor || !GIsGame)
				{
					GRHIShaderPlatform = OldPlatform;
				}	

				GShaderCompilePlatform = GRHIShaderPlatform;
			}			
		}
	}	
}