/*=============================================================================
AvaRadiosityAdapter.cpp: Unreal to vrad interface
 2006-05-02	허 창 민
=============================================================================*/

#include "EditorPrivate.h"
#include "EnginePhysicsClasses.h"
#include "EngineMaterialClasses.h"
#include "UnTerrain.h"

#include "../../AvaRad/utils/vrad_launcher/ivrad_launcher.h"

#pragma optimize("", off)

static TMap< UMaterialInstance*, INT > GMaterialToTexMap;
static TMap< UTexture2D*, FLinearColor > GDiffuseReflectivity;
static TMap< UTexture2D*, FLinearColor > GEmissiveBrightness;

INT AvaRadiosityAdapter::FindOrAddTexData( UMaterialInstance* MaterialInstance )
{
	INT TexDataIndex = 0;

	if (MaterialInstance)
	{
		const INT* Index = GMaterialToTexMap.Find( MaterialInstance );		

		if (Index != NULL) return *Index;

		UMaterialInstanceConstant* MIC = NULL;
		UMaterial* Material = NULL;

		UMaterialInstance* MI = MaterialInstance;
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
			return TexDataIndex;

		struct FTextureCollectingMaterialCompiler : public FMaterialCompiler
		{
			UMaterial* Material;
			UTexture2D* ResultTexture;
			TArray<UMaterialExpression*> ExpressionStack;
			UMaterialInstanceConstant* MIC;

			// Contstructor
			FTextureCollectingMaterialCompiler( UMaterial* InMaterial, UMaterialInstanceConstant* MIC )
				:	Material( InMaterial ), ResultTexture( NULL ), MIC( MIC )
			{}
			
			virtual INT Error(const TCHAR* Text) 
			{
// error가 나도.. 가도 되요~ 2008. 1. 7 changmin
//#if !defined(XBOX) && !defined(EXCEPTIONS_DISABLED)
//				throw Text; 
//#endif
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
				if (!ResultTexture) ResultTexture = Cast<UTexture2D>( Texture );
				return 0; 
			}

			virtual INT TextureParameter(FName ParameterName,UTexture* DefaultTexture) 
			{
				for (UMaterialInstanceConstant* p = MIC; p != NULL && !ResultTexture; p = Cast<UMaterialInstanceConstant>( p->Parent ))
				{
					for (INT i=0; i<p->NewTextureParameterValues.Num(); ++i)
					{
						if (p->NewTextureParameterValues(i).ParameterName == ParameterName)
						{
							ResultTexture = Cast<UTexture2D>( p->NewTextureParameterValues(i).ParameterValue );
							break;
						}
					}
				}

				if (!ResultTexture)
					ResultTexture = Cast<UTexture2D>( DefaultTexture );
				
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

		} Compiler( Material, Cast<UMaterialInstanceConstant>( MaterialInstance ) );			

		const FLinearColor* Reflectivity = NULL;
		const FLinearColor* Brightness = NULL;

		AvaTextureData TextureData;
		TextureData.SourceMaterial_ = MaterialInstance;			
		TextureData.SizeX_ = 0;
		TextureData.SizeY_ = 0;

		Material->MaterialResource->CompileProperty( MP_DiffuseColor, &Compiler );

		FLinearColor InstantReflectivity;
		if (Compiler.ResultTexture)
		{			
			UTexture2D* Texture = Compiler.ResultTexture;

			Reflectivity = GDiffuseReflectivity.Find( Texture );
			if (!Reflectivity)
			{
				InstantReflectivity = Texture->SampleAverageColor();	
				GDiffuseReflectivity.Set( Texture, InstantReflectivity );

				Reflectivity = &InstantReflectivity;
			}

			TextureData.SizeX_ = Texture->SizeX;
			TextureData.SizeY_ = Texture->SizeY;
		}

		Compiler.ResultTexture = NULL;
		Material->MaterialResource->CompileProperty( MP_EmissiveColor, &Compiler );		

		FLinearColor InstantBrightness;
		if (Compiler.ResultTexture)
		{			
			UTexture2D* Texture = Compiler.ResultTexture;

			Brightness = GEmissiveBrightness.Find( Texture );
			if (!Brightness)
			{
				InstantBrightness = FLinearColor( Texture->SampleAverageColor() );	
				GEmissiveBrightness.Set( Texture, InstantBrightness );

				Brightness = &InstantBrightness;
			}

			TextureData.SizeX_ = Max( TextureData.SizeX_, Texture->SizeX );
			TextureData.SizeY_ = Max( TextureData.SizeY_, Texture->SizeY );
		}

		if (Reflectivity || Brightness || MaterialInstance->bOverrideBrightnessColor || MaterialInstance->bOverrideReflectivityColor)
		{				
			TextureData.Reflectivity_ = Reflectivity ? FVector( Reflectivity->R, Reflectivity->G, Reflectivity->B ) : FVector( 0, 0, 0 );
			TextureData.Brightness_ = Brightness ? FVector( Brightness->R, Brightness->G, Brightness->B ) : FVector( 0, 0, 0 );
			
			for (MI = MaterialInstance;;)
			{
				if (MI->bOverrideReflectivityColor)
				{
					FLinearColor L( MI->ReflectivityColor );

					TextureData.Reflectivity_ = FVector( L.R, L.G, L.B );

					break;
				}

				/// Is MIC?
				MIC = Cast<UMaterialInstanceConstant>( MI );
				if (!MIC)
					break;

				MI = MIC->Parent;
			}		

			for (MI = MaterialInstance;;)
			{
				if (MI->bOverrideBrightnessColor)
				{
					FLinearColor L( MI->BrightnessColor );

					TextureData.Brightness_ = FVector( L.R, L.G, L.B );
				}

				/// Is MIC?
				MIC = Cast<UMaterialInstanceConstant>( MI );
				if (!MIC)
					break;

				MI = MIC->Parent;
			}					
		
			TextureData.Brightness_ *= MaterialInstance->BrightnessScale;						

			UPhysicalMaterial* PhysicalMaterial = MaterialInstance->GetPhysicalMaterial();

			TextureData.ReflectivityScale_ = MaterialInstance->ReflectivityScale;

			if (PhysicalMaterial)
				TextureData.ReflectivityScale_ *= PhysicalMaterial->Reflectivity;											

			if (TextureData.Reflectivity_.IsZero())
				TextureData.ReflectivityScale_ = 0;

			if (GWorld && GWorld->GetWorldInfo())
			{
				TextureData.Brightness_ *= GWorld->GetWorldInfo()->Radiosity_EmissiveScale;
				TextureData.ReflectivityScale_ *= GWorld->GetWorldInfo()->Radiosity_ReflectivityScale;
			}

			TexDataIndex = TextureDatas_.AddItem( TextureData );
		}
		

		GMaterialToTexMap.Set( MaterialInstance, TexDataIndex );
	}

	return TexDataIndex;
}

bool AvaRadiosityAdapter::StartUp()
{
	//!{ 2006-03-31	허 창 민
	FString DllFolder		= "..\\Tool\\AVARAD";
	FString BinariesFolder	= "..\\..\\Binaries";

	GMaterialToTexMap.Empty();
	GDiffuseReflectivity.Empty();
	GEmissiveBrightness.Empty();

	TCHAR_CALL_OS(SetCurrentDirectoryW(*DllFolder),SetCurrentDirectoryA(TCHAR_TO_ANSI(*DllFolder)));
	FString DllName = "vrad_launcher.dll";
	hDLL_ = LoadLibrary( *DllName );
	TCHAR_CALL_OS(SetCurrentDirectoryW(*BinariesFolder),SetCurrentDirectoryA(TCHAR_TO_ANSI(*BinariesFolder)));
	if( !hDLL_ )
	{
		return false;
	}

	VRadLauncher_ = NULL;
	typedef IVRadLauncher* (*GetLauncherFn)(void);

#pragma warning(push)
#pragma warning(disable:4191) // disable: unsafe conversion from 'type of expression' to 'type required'
	GetLauncherFn GetFn = (GetLauncherFn) GetProcAddress( hDLL_, "GetLauncher" );
#pragma warning(pop)

	VRadLauncher_ = GetFn();
	if( !VRadLauncher_ )
	{
		return false;
	}
	TCHAR_CALL_OS(SetCurrentDirectoryW(*DllFolder),SetCurrentDirectoryA(TCHAR_TO_ANSI(*DllFolder)));
	bool bInit = VRadLauncher_->Initialize();
	TCHAR_CALL_OS(SetCurrentDirectoryW(*BinariesFolder),SetCurrentDirectoryA(TCHAR_TO_ANSI(*BinariesFolder)));
	return bInit;
}

void AvaRadiosityAdapter::ShutDown()
{
	VRadLauncher_->Finalize();
	FreeLibrary( hDLL_ );
	VRadLauncher_ = NULL;
	hDLL_ = NULL;
}

IMPLEMENT_COMPARE_CONSTREF( FAmbientCubeVolumeInfo, AvaRadiosityAdapter, 
{
	return A.Priority - B.Priority;
}
);

void AvaRadiosityAdapter::GatherWorldInfo(const FLightingBuildOptions& Options)
{
	UINT NumTasks = 3, TaskIndex = 0;

	GWarn->BeginSlowTask( *LocalizeUnrealEd(TEXT("GatherWorldInfoF")), TRUE );
	GWarn->StatusUpdatef(TaskIndex,NumTasks,*LocalizeUnrealEd(TEXT("GatherWorldInfoF")),TaskIndex,NumTasks);
	TaskIndex++;

	//!{ 2006-03-31	허 창 민
	// Radiosity 
	UBOOL	bAbortLightLevel	= FALSE;	
	
	INT NumSHs = 0;

	if (Options.bBakeAmbientCubes)
	{
		GWorld->GetWorldInfo()->AmbientCubeVolumeInfos.Empty();
	}	

	static FVector CubeExtent( 4, 4, 4 );

	// Iterate over all levels, Export Data 모으기
	for( INT LevelIndex=0; LevelIndex<GWorld->Levels.Num(); LevelIndex++ )
	{
		ULevel* Level = GWorld->Levels(LevelIndex);

		if( Options.ShouldBuildLightingForLevel( Level ) )
		{
			//<@ 2006. 10. 18 changmin
			// ambient cube volume 정보 찾기 위치 이동
			for( INT ActorIndex=0; ActorIndex<Level->Actors.Num(); ActorIndex++ )
			{
				// Break out of this build function early if the map build was cancelled.
				if( GEditor->GetMapBuildCancelled() )
				{
					bAbortLightLevel = TRUE;
					break;
				}

				AActor* Actor = Level->Actors(ActorIndex);

				// ambient cube volume
				AAmbientCubeVolume* AmbientCubeVolume = Cast<AAmbientCubeVolume>( Actor );

				if (AmbientCubeVolume && Options.bBakeAmbientCubes)
				{
					FAmbientCubeVolumeInfo Info;
					Info.AmbientCubeMin = AmbientCubeVolume->BrushComponent->Bounds.GetBox().Min;
					Info.AmbientCubeMax = AmbientCubeVolume->BrushComponent->Bounds.GetBox().Max;
					Info.SHStartIndex = 0;

					FVector CubeSize( AmbientCubeVolume->CubeSizeX, AmbientCubeVolume->CubeSizeY, AmbientCubeVolume->CubeSizeZ );					

					Info.CubeSize = CubeSize;

					FVector Diff = (Info.AmbientCubeMax - Info.AmbientCubeMin) / CubeSize;

					Info.NumAmbientCubeX = (INT)Diff.X;
					Info.NumAmbientCubeY = (INT)Diff.Y;
					Info.NumAmbientCubeZ = (INT)Diff.Z;
					Info.Priority = AmbientCubeVolume->Priority;

					while (Info.NumAmbientCubeX * Info.NumAmbientCubeY * Info.NumAmbientCubeZ > 1000000 && Info.NumAmbientCubeZ > 2)
					{
						Info.NumAmbientCubeZ /= 2;
					}							

					Info.NumAmbientCubeX = Max( 2, Info.NumAmbientCubeX );
					Info.NumAmbientCubeY = Max( 2, Info.NumAmbientCubeY );
					Info.NumAmbientCubeZ = Max( 2, Info.NumAmbientCubeZ );

					GWorld->GetWorldInfo()->AmbientCubeVolumeInfos.AddItem( Info );
				}
			}

			if (GWorld->GetWorldInfo()->AmbientCubeVolumeInfos.Num() > 0)
			{
				Sort<USE_COMPARE_CONSTREF(FAmbientCubeVolumeInfo,AvaRadiosityAdapter)>( &GWorld->GetWorldInfo()->AmbientCubeVolumeInfos(0), GWorld->GetWorldInfo()->AmbientCubeVolumeInfos.Num() );

				for (INT i=0; i<GWorld->GetWorldInfo()->AmbientCubeVolumeInfos.Num(); ++i)
				{
					FAmbientCubeVolumeInfo& Info = GWorld->GetWorldInfo()->AmbientCubeVolumeInfos(i);

					Info.SHStartIndex = NumSHs;

					NumSHs += Info.NumAmbientCubeX * Info.NumAmbientCubeY * Info.NumAmbientCubeZ;
				}
			}			

			if( bAbortLightLevel )
			{
				break;
			}

			//>@

			//!{ 2006-04-24	허 창 민
			// Export BspModel Vertices
			UModel* BspModel = Level->Model;
			BspModel->ExportedVertexBase = VertexBuffer_.Num();
			for( INT iVertex = 0; iVertex < BspModel->Points.Num(); ++iVertex )
			{
				VertexBuffer_.AddItem( BspModel->Points( iVertex ) );
			}
			//!} 2006-04-24	허 창 민

			// initialize node to export face map..
			BspModel->NodeToExportedFace.Empty();
			for( INT i = 0; i < BspModel->Nodes.Num(); ++i )
			{
				BspModel->NodeToExportedFace.AddItem( -1 );
			}

			// export BSP.
			if( Options.bBuildBSP && !Options.bOnlyBuildSelectedActors )
			{
				for( INT ComponentIndex=0; ComponentIndex<Level->ModelComponents.Num(); ComponentIndex++ )
				{
					// Break out of this build function early if the map build was canceled.
					if( GEditor->GetMapBuildCancelled() )
					{
						bAbortLightLevel = TRUE;
						break;
					}

					UModelComponent* ModelComponent = Level->ModelComponents(ComponentIndex);
					if( ModelComponent )
					{
						ModelComponent->ExportSurfaces( this );
					}
				}
			}

			if( bAbortLightLevel )
			{
				break;
			}

			// export Actor
			if( Options.bBuildActors && !bAbortLightLevel )
			{
				for( INT ActorIndex=0; ActorIndex<Level->Actors.Num(); ActorIndex++ )
				{
					// Break out of this build function early if the map build was cancelled.
					if( GEditor->GetMapBuildCancelled() )
					{
						bAbortLightLevel = TRUE;
						break;
					}

					AActor* Actor = Level->Actors(ActorIndex);

					// Only export selected actors if wanted.			
					if( Actor && (!Options.bOnlyBuildSelectedActors || Actor->IsSelected())	)
					{
						AStaticMeshActor*	StaticMeshActor = Cast<AStaticMeshActor>(Actor);						
						ADynamicSMActor*	DynamicSMActor	= Cast<ADynamicSMActor>(Actor);
						ATerrain*			TerrainActor	= Cast<ATerrain>(Actor);

						// export static mesh
						if( StaticMeshActor || DynamicSMActor )
						{
							for( INT iComponent = 0; iComponent < Actor->Components.Num(); ++iComponent )
							{
								UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>( Actor->Components( iComponent ) );

								if( StaticMeshComponent )
								{
									StaticMeshComponent->ExportSurfaces( this );
								}
							}
						}

						// export terrain
						if( TerrainActor )
						{
							// space for disp info 
							TerrainActor->ExportDispInfoOffset = DispInfos_.Num();

							// compute a map of terrain quad index to dispinfo
							TMap<INT,INT> QuadIndexToDispInfo;
							for( INT QuadY = 0; QuadY < TerrainActor->NumPatchesY / TerrainActor->MaxTesselationLevel; ++QuadY )
							{
								for( INT QuadX = 0; QuadX < TerrainActor->NumPatchesX / TerrainActor->MaxTesselationLevel; ++QuadX )
								{
									const INT QuadIndex = QuadY * (TerrainActor->NumPatchesX / TerrainActor->MaxTesselationLevel) + QuadX;
									FTerrainInfoData* TerrainInfo = TerrainActor->GetInfoData( QuadX * TerrainActor->MaxTesselationLevel, QuadY * TerrainActor->MaxTesselationLevel );
									if( TerrainInfo->IsVisible() )
									{
										QuadIndexToDispInfo.Set( QuadIndex, DispInfos_.Add() );
									}
								}
							}

							//<@ 2006. 10. 14 changmin
							// export terrain vertex buffer
							const INT TerrainActorVertexBufferBase = VertexBuffer_.Num();
							for( INT PatchVertexY = 0; PatchVertexY < TerrainActor->NumPatchesY + 1; PatchVertexY += TerrainActor->MaxTesselationLevel  )
							{
								for( INT PatchVertexX = 0; PatchVertexX < TerrainActor->NumPatchesX + 1; PatchVertexX += TerrainActor->MaxTesselationLevel )
								{
									FVector PatchVertex = TerrainActor->GetWorldVertex( PatchVertexX, PatchVertexY );
									VertexBuffer_.AddItem( PatchVertex );
								}

							}
							//>@ 

							for( INT ComponentIndex = 0; ComponentIndex < TerrainActor->TerrainComponents.Num(); ++ComponentIndex )
							{
								UTerrainComponent* TerrainComp = TerrainActor->TerrainComponents(ComponentIndex);
								//<@ 2006. 10. 14 changmin
								TerrainComp->ExportSurfaces( this, TerrainActorVertexBufferBase, QuadIndexToDispInfo );
								//>@
							}
						}
					}
				}	// end for actor
			}
		}	
	}

	GWarn->StatusUpdatef(TaskIndex,NumTasks,*LocalizeUnrealEd(TEXT("GatherWorldInfoF")),TaskIndex,NumTasks);
	TaskIndex++;

	// export light
	INT ExportId = 0;
	LightToExportId.Empty();

	for(TSparseArray<ULightComponent*>::TConstIterator LightIt(GWorld->StaticLightList);LightIt;++LightIt)
	{
		ULightComponent* Light = *LightIt;

		// skip dynamic lights and non lightmapped light
		if( ! Light->HasStaticLighting() || !Light->UseDirectLightMap )
		{
			continue;
		}

		// save map
		const int LightId = ExportId++;
		LightToExportId.Set( Light, LightId );

		UPointLightComponent* PointLight = NULL;
		USpotLightComponent* SpotLight;

		SpotLight = Cast<USpotLightComponent>(Light);
		if( SpotLight )
		{
			FVector Position = SpotLight->GetPosition();
			FLinearColor Intensity = FLinearColor(SpotLight->LightColor) * SpotLight->Brightness;
			float Radius = SpotLight->Radius;

			VRadLauncher_->AddSpotLight(	Position.X, Position.Y, Position.Z,
				Intensity.R, Intensity.G, Intensity.B,
				Radius, SpotLight->FalloffExponent,
				SpotLight->GetDirection().X, SpotLight->GetDirection().Y, SpotLight->GetDirection().Z, 
				SpotLight->InnerConeAngle, SpotLight->OuterConeAngle,
				//SpotLight->UseDirectLightMap,
				(!SpotLight->bUseCascadedShadowmap),	// 2007. 10. 22 changmin ; add cascaded shadow map
				LightId );

			continue;
		}

		PointLight = Cast<UPointLightComponent>(Light);
		if( PointLight )
		{
			FVector Position = PointLight->GetPosition();
			FLinearColor Intensity = FLinearColor(PointLight->LightColor) * PointLight->Brightness;
			float Radius = PointLight->Radius;

			VRadLauncher_->AddPointLight(	Position.X, Position.Y, Position.Z,
				Intensity.R, Intensity.G, Intensity.B,
				Radius, PointLight->FalloffExponent,
				//PointLight->UseDirectLightMap,	// 2007. 10. 22 changmin ; add cascaded shadow map
				!PointLight->bUseCascadedShadowmap,
				LightId );

			continue;
		}		

		USunLightComponent* DirectionalLight = NULL;
		DirectionalLight = Cast<USunLightComponent>(Light);
		if (DirectionalLight)
		{
			FLinearColor Intensity = FLinearColor(DirectionalLight->LightColor) * DirectionalLight->Brightness;
			FLinearColor Ambient = FLinearColor(DirectionalLight->AmbientLightColor) * DirectionalLight->AmbientBrightness;			

			VRadLauncher_->SetSunLight_( 
				DirectionalLight->GetDirection().X, 
				DirectionalLight->GetDirection().Y, 
				DirectionalLight->GetDirection().Z, 
				Intensity.R, Intensity.G, Intensity.B,
				Ambient.R, Ambient.G, Ambient.B,
				//DirectionalLight->UseDirectLightMap,
				!DirectionalLight->bUseCascadedShadowmap,	// 2007. 10. 22 changmin ; add cascaded shadow map
				LightId,
				FTCHARToANSI(*DirectionalLight->SkyEnvironmentMap),
				DirectionalLight->SkyMax,
				DirectionalLight->SkyScale,
				DirectionalLight->Yaw,
				DirectionalLight->HDRTonemapScale
				);

			// 2007. 12. 21 changmin
			// add cascaded shadow
			// 아직 sun light밖에 지원 안하므로 여기에 넣는다.
			if( DirectionalLight->bUseCascadedShadowmap )
				bSupportsCascadedShadow_ = TRUE;
			//> 
		}
	}

	GWarn->StatusUpdatef(TaskIndex,NumTasks,*LocalizeUnrealEd(TEXT("GatherWorldInfoF")),TaskIndex,NumTasks);
	TaskIndex++;

	if (Options.bBakeAmbientCubes)
	{					
		TArray<FVector> Points;

		Points.AddZeroed( NumSHs );		

		GWorld->GetWorldInfo()->AmbientSHs.Empty( NumSHs );
		GWorld->GetWorldInfo()->AmbientSHs.AddZeroed( NumSHs );

		for (INT i=0; i<GWorld->GetWorldInfo()->AmbientCubeVolumeInfos.Num(); ++i)
		{
			const FAmbientCubeVolumeInfo& Info = GWorld->GetWorldInfo()->AmbientCubeVolumeInfos(i);

			INT nX = Info.NumAmbientCubeX;
			INT nY = Info.NumAmbientCubeY;
			INT nZ = Info.NumAmbientCubeZ;

#define ComputeIndex( X, Y, Z ) ( (X) + ((Y) + (Z) * nY) * nX ) 

			FVector SamplePoint;

			for (INT X=0; X<nX; ++X)
			{
				SamplePoint.X = Info.AmbientCubeMin.X + (Info.AmbientCubeMax.X - Info.AmbientCubeMin.X) * ((FLOAT)X / (nX-1));

				for (INT Y=0; Y<nY; ++Y)
				{
					SamplePoint.Y = Info.AmbientCubeMin.Y + (Info.AmbientCubeMax.Y - Info.AmbientCubeMin.Y) * ((FLOAT)Y / (nY-1));

					for (INT Z=0; Z<nZ; ++Z)
					{
						SamplePoint.Z = Info.AmbientCubeMin.Z + (Info.AmbientCubeMax.Z - Info.AmbientCubeMin.Z) * ((FLOAT)Z / (nZ-1));						

						Points( ComputeIndex( X, Y, Z ) + Info.SHStartIndex ) = SamplePoint;
					}
				}
			}

			VRadLauncher_->SetAmbientCubePoints( NumSHs, (FLOAT*)&Points(0) );
		}			
	}	
	else
	{		
		VRadLauncher_->SetAmbientCubePoints( 0, NULL );
	}

	GWarn->StatusUpdatef(TaskIndex,NumTasks,*LocalizeUnrealEd(TEXT("GatherWorldInfoF")),TaskIndex,NumTasks);
	TaskIndex++;

	checkSlow(TaskIndex == NumTasks+1);

	GWarn->EndSlowTask();
}

bool AvaRadiosityAdapter::IsCanceled()
{
	return !!GEditor->GetMapBuildCancelled();
}

bool AvaRadiosityAdapter::Render(const FLightingBuildOptions& Options)
{
	// Export to vrad
	VRadLauncher_->SetIndexBuffer( IndexBuffer_.Num(), &IndexBuffer_(0) );
	VRadLauncher_->SetVertexBuffer( VertexBuffer_.Num(), (float*)&VertexBuffer_(0) );
	VRadLauncher_->SetPlaneBuffer( PlaneBuffer_.Num(), (float*)&PlaneBuffer_(0) );

	// 
	VRadLauncher_->SetBlackMeshIndexBuffer( BlackMeshIndexBuffer_.Num(), &BlackMeshIndexBuffer_(0) );
	VRadLauncher_->SetBlackMeshTexDataIndexBuffer( BlackMeshTexDataIndex_.Num(), &BlackMeshTexDataIndex_(0) );

	VRadLauncher_->SetBlackMeshVertexBuffer( BlackMeshVertexBuffer_.Num(), (float*)&BlackMeshVertexBuffer_(0) );
	VRadLauncher_->SetBlackMeshTriangleCounts( BlackMeshTriangleCounts_.Num(), &BlackMeshTriangleCounts_(0) );
	VRadLauncher_->SetBlackMeshVertexCounts( BlackMeshVertexCounts_.Num(), &BlackMeshVertexCounts_(0) );
	VRadLauncher_->SetBlackMeshSampleToAreaRatios( BlackMeshSampleToAreaRatios_.Num(), &BlackMeshSampleToAreaRatios_(0) );
	VRadLauncher_->SetBlackMeshSampleVerticesFlags( BlackMeshSampleVerticesFlags_.Num(), &BlackMeshSampleVerticesFlags_(0) );
	VRadLauncher_->SetBlackMeshVertexTangentBasisBuffer(
		BlackMeshVertexBuffer_.Num(),
		(float*)&BlackMeshVertexTangentXBuffer_(0),
		(float*)&BlackMeshVertexTangentYBuffer_(0),
		(float*)&BlackMeshVertexTangentZBuffer_(0)
		);

	for( INT iFace = 0; iFace < FaceInfos_.Num(); ++iFace )
	{
		AvaFaceInfo& F = FaceInfos_(iFace);
		VRadLauncher_->AddFace( F.IndexBufferOffset, F.NumVertices, F.iPlane, F.iSurfaceMapInfo, F.iDispInfo,
			F.Emission.X, F.Emission.Y, F.Emission.Z,
			F.LightmapInfo.Width, F.LightmapInfo.Height );

		AvaSurfaceMapInfo& S = SurfaceMaps_(F.iSurfaceMapInfo);
		VRadLauncher_->AddTextureInfo(	S.TexturemapUAxis.X, S.TexturemapUAxis.Y, S.TexturemapUAxis.Z,
			S.TexturemapVAxis.X, S.TexturemapVAxis.Y, S.TexturemapVAxis.Z,
			S.LightmapUAxis.X, S.LightmapUAxis.Y, S.LightmapUAxis.Z, S.LightmapUOffset,
			S.LightmapVAxis.X, S.LightmapVAxis.Y, S.LightmapVAxis.Z, S.LightmapVOffset,
			1,
			S.ShadowMapScale,
			F.iTextureData,
			S.Flags);

	}

	// disp infos
	for( INT iDispInfo = 0; iDispInfo < DispInfos_.Num(); ++iDispInfo )
	{
		AvaDispInfo& DispInfo = DispInfos_(iDispInfo);

		VRadLauncher_->AddDispInfo( DispInfo.StartPosition.X, DispInfo.StartPosition.Y, DispInfo.StartPosition.Z,
			DispInfo.DispVertexStart,
			DispInfo.Power,
			DispInfo.NeighborIndex,
			DispInfo.NeighborOrientation,
			DispInfo.Span,
			DispInfo.NeighborSpan
			);
	}

	// disp vertices
	for( INT iDispVert = 0; iDispVert < DispVertexes_.Num(); ++iDispVert )
	{
		AvaDispVertex& DV = DispVertexes_(iDispVert);

		VRadLauncher_->AddDispVertex( DV.DispVector.X, DV.DispVector.Y, DV.DispVector.Z, DV.DispDistance, DV.Alpha );
	}

	// default texture data : 0번
	//VRadLauncher_->AddTextureData( 100, 100, 0.9f, 0.9f, 0.9f, 1.0f );

	// 실제 texture data들
	for( INT iTextureData = 0; iTextureData < TextureDatas_.Num(); ++iTextureData )
	{
		AvaTextureData& TD = TextureDatas_(iTextureData);

		VRadLauncher_->AddTextureData(	TD.SizeX_, TD.SizeY_,
			TD.Reflectivity_.X, TD.Reflectivity_.Y, TD.Reflectivity_.Z,
			TD.Brightness_.X, TD.Brightness_.Y, TD.Brightness_.Z, 
			TD.ReflectivityScale_ );
	}

	// vrad options
	int options = 0;
	if( Options.bUseRayTraceLighting )
	{
		options |= USE_RAYTRACE;
	}

	if( !Options.bUseMultiThread )
	{
		options |= FORCE_SINGLE_THREAD;
	}

	if( Options.bFast)
	{
		options |= DO_FAST;
	}

	if( Options.bExtra)
	{
		options |= DO_EXTRA;
	}

	// ivrad_launcher의 값과 , LightingBuildOptions과 맞춰야 한다.
	switch( Options.AmbientType )
	{
	case 0:
		options |= SAMPLE_SKY_NONE;
		break;
	case 1:
		options |= SAMPLE_SKY_SIMPLE;
		break;
	case 2:
		options |= SAMPLE_SKY_FULL;
		break;
	default:
		options |= SAMPLE_SKY_NONE;
	}
	
	VRadLauncher_->SetOptions( options, 0 /* deprecated grid size */ );

	for (INT i=0; i<Options.Clusters.Num(); ++i)
	{
		VRadLauncher_->AddCluster( TCHAR_TO_ANSI( *Options.Clusters(i) ) );
	}	

	// bounce light
	bool IsOk = VRadLauncher_->Run( this );

	if( IsOk )
	{
		// get result from vrad
		for( INT iFace = 0; iFace < FaceInfos_.Num(); ++iFace )
		{
			AvaFaceInfo& F = FaceInfos_( iFace );

			INT Width = 0, Height = 0;

			unsigned char* LightmapData = NULL;
			unsigned char* LightmapDataWithoutSun = NULL;
			unsigned char* NegativeLightmapData = NULL;

			VRadLauncher_->GetLightmap( iFace, &Width, &Height, &LightmapData );
			VRadLauncher_->GetLightmapWithoutSun( iFace, &Width, &Height, &LightmapDataWithoutSun );
			VRadLauncher_->GetNegativeLightmap( iFace, &Width, &Height, &NegativeLightmapData );

			F.LightmapInfo.Lightmap = LightmapData;
			F.LightmapInfo.NegativeLightmap = NegativeLightmapData;
			F.LightmapInfo.LightmapWithoutSun = LightmapDataWithoutSun;
			F.LightmapInfo.Width = Width;
			F.LightmapInfo.Height = Height;

			//< shadow map 기능 제거 2008. 2. 20 changmin
			//INT NumShadows = 0;
			//VRadLauncher_->GetShadowmapCount(iFace, &NumShadows );
			//for( INT ShadowIndex = 0 ; ShadowIndex < NumShadows; ++ShadowIndex )
			//{
			//	AvaShadowMapInfo ShadowMapInfo;
			//	VRadLauncher_->GetShadowmap(iFace, ShadowIndex, &ShadowMapInfo.ExportLightId, &Width, &Height, &ShadowMapInfo.ShadowMap );
			//	F.ShadowMaps.AddItem( ShadowMapInfo );
			//}
			//>@ 

			F.bSunVisible = VRadLauncher_->GetFaceSunVisibility( iFace );
		}

		VRadLauncher_->GetBlackMeshVertexLightMap( &BlackMeshVertexLightMap_ );

		VRadLauncher_->GetBlackMeshVertexLightMapWithoutSun( &BlackMeshVertexLightMapWithoutSun_ );

		//<@ 2007. 11. 12 changmin
		VRadLauncher_->GetBlackMeshSunVisibility( &BlackMeshSunVisibility_ );
		//>@ ava

		if (Options.bBakeAmbientCubes)
		{
			const FLOAT* AmbientCubes = VRadLauncher_->GetAmbientCubes();

			if (AmbientCubes != NULL)
			{
				memcpy( &GWorld->GetWorldInfo()->AmbientSHs(0), AmbientCubes, sizeof(FAmbientSH) * GWorld->GetWorldInfo()->AmbientSHs.Num() );
			}			
		}

		FHDRSunInfo* HDRSunInfo = VRadLauncher_->GetHDRSunInfo();

		if (HDRSunInfo)
		{
			for(TSparseArray<ULightComponent*>::TConstIterator LightIt(GWorld->StaticLightList);LightIt;++LightIt)			
			{
				ULightComponent* Light = *LightIt;
				
				if (!Light->HasStaticLighting())
				{
					continue;
				}				

				USunLightComponent* DirectionalLight = NULL;
				DirectionalLight = Cast<USunLightComponent>(Light);
				if (DirectionalLight)
				{
					FVector Direction( HDRSunInfo->Direction[0], HDRSunInfo->Direction[1], HDRSunInfo->Direction[2] );

					debugf(NAME_Log, TEXT("sun direction ( %f, %f, %f )"), HDRSunInfo->Direction[0], HDRSunInfo->Direction[1], HDRSunInfo->Direction[2] );

					FLinearColor Color( HDRSunInfo->Color[0], HDRSunInfo->Color[1], HDRSunInfo->Color[2] );

					Color *= DirectionalLight->HDRTonemapScale;

					FLOAT MaxValue = Clamp( Max3( Color.R, Color.G, Color.B ), 0.1f, 256.0f );

					FLOAT H = FVector2D( Direction.X, Direction.Y ).Size();

					DirectionalLight->GetOwner()->Rotation.Yaw = 65536 / 4 - (appAtan2( Direction.X, Direction.Y ) * 65535 / (2*M_PI) + 32768);
					DirectionalLight->GetOwner()->Rotation.Pitch = appAtan2( H, Direction.Z ) * 65535 / (2*M_PI) - 65536 / 4;					
					
					DirectionalLight->LightColor = FColor( Color / MaxValue );
					DirectionalLight->Brightness = Clamp( MaxValue, 0.1f, 256.0f );

					DirectionalLight->ConditionalUpdateTransform();						
				}
			}
		}		
	}
	else
	{
		// 

	}

	return IsOk;
}


void AvaLightmapInfo::GetSample( INT X, INT Y, INT iBump, FLOAT* R, FLOAT* G, FLOAT* B)
{
	if( Lightmap )
	{
		INT NumLuxels  = Width * Height;

		unsigned char* RadiositySample = &Lightmap[ iBump * NumLuxels * 4 ];
		RadiositySample += ( Y * Width + X ) * 4;

		signed char Exponent = ((signed char)RadiositySample[3]);

		*R = (FLOAT)RadiositySample[0] * pow( 2.0f, Exponent);
		*G = (FLOAT)RadiositySample[1] * pow( 2.0f, Exponent);
		*B = (FLOAT)RadiositySample[2] * pow( 2.0f, Exponent);
	}
	else
	{
		*R = 0.0f;
		*G = 0.0f;
		*B = 0.0f;
	}
}

void AvaLightmapInfo::GetSampleWithoutSun( INT X, INT Y, INT iBump, FLOAT* R, FLOAT* G, FLOAT* B)
{
	if( Lightmap )
	{
		INT NumLuxels  = Width * Height;

		unsigned char* RadiositySample = &LightmapWithoutSun[ iBump * NumLuxels * 4 ];
		RadiositySample += ( Y * Width + X ) * 4;

		signed char Exponent = ((signed char)RadiositySample[3]);

		*R = (FLOAT)RadiositySample[0] * pow( 2.0f, Exponent);
		*G = (FLOAT)RadiositySample[1] * pow( 2.0f, Exponent);
		*B = (FLOAT)RadiositySample[2] * pow( 2.0f, Exponent);
	}
	else
	{
		*R = 0.0f;
		*G = 0.0f;
		*B = 0.0f;
	}
}

void AvaLightmapInfo::GetNegativeSample( INT X, INT Y, INT iBump, FLOAT* R, FLOAT* G, FLOAT* B)
{
	if( NegativeLightmap )
	{
		INT NumLuxels  = Width * Height;

		unsigned char* RadiositySample = &NegativeLightmap[ iBump * NumLuxels * 4 ];
		RadiositySample += ( Y * Width + X ) * 4;

		signed char Exponent = ((signed char)RadiositySample[3]);

		*R = (FLOAT)RadiositySample[0] * pow( 2.0f, Exponent);
		*G = (FLOAT)RadiositySample[1] * pow( 2.0f, Exponent);
		*B = (FLOAT)RadiositySample[2] * pow( 2.0f, Exponent);
	}
	else
	{
		*R = 0.0f;
		*G = 0.0f;
		*B = 0.0f;
	}
}

void AvaRadiosityAdapter::GetRadiositySamples(TArray<AvaSample>* Samples)
{
	FString FileName(TEXT("samples0.txt"));
	FString SamplesString;

	if( appLoadFileToString( SamplesString, *FileName ) )
	{
		TArray<INT> NumberStarts;

		UBOOL IsNumberStart = FALSE;
		
		for( INT iChar = 0; iChar < SamplesString.Len(); ++iChar )
		{
			TCHAR& C = SamplesString[iChar];

			if( C == '-' || C == '.' || appIsDigit(C) )
			{
				if( !IsNumberStart )
				{
					NumberStarts.AddItem(iChar);
					IsNumberStart = TRUE;
				}
			}
			else
			{
				if( IsNumberStart )
				{
					SamplesString[iChar] = '\0';
					IsNumberStart = FALSE;
				}
			}
		}

		for( INT iNumber = 0; iNumber < NumberStarts.Num();  )
		{
			AvaSample sample;

			sample.NumPoints = appAtoi( &( SamplesString[ NumberStarts(iNumber++) ]) );

			for( INT point = 0; point < sample.NumPoints; ++point )
			{
				// parse positon
				FVector Position( appAtof(&( SamplesString[ NumberStarts(iNumber) ])),
								  appAtof(&( SamplesString[ NumberStarts(iNumber+1) ])),
								  appAtof(&( SamplesString[ NumberStarts(iNumber+2) ]))
								 );


				sample.Positions.AddItem( Position );

				iNumber += 3;

				// parse color// color를 여러번쓰니.. 여러번 읽어서 덮어 쓰자
				FVector SampleColor( appAtof(&( SamplesString[ NumberStarts(iNumber) ])),
								appAtof(&( SamplesString[ NumberStarts(iNumber+1) ])),
								appAtof(&( SamplesString[ NumberStarts(iNumber+2) ]))
					);
				sample.Color = SampleColor;
				iNumber += 3;	// color 는 건너뛴다
			}

			iNumber++;	// 2 element

			FVector Start( appAtof(&( SamplesString[ NumberStarts(iNumber) ])),
				appAtof(&( SamplesString[ NumberStarts(iNumber+1) ])),
				appAtof(&( SamplesString[ NumberStarts(iNumber+2) ]))
				);

			iNumber += 3;
			iNumber += 3;

			FVector End( appAtof(&( SamplesString[ NumberStarts(iNumber) ])),
				appAtof(&( SamplesString[ NumberStarts(iNumber+1) ])),
				appAtof(&( SamplesString[ NumberStarts(iNumber+2) ]))
				);

			iNumber += 3;
			iNumber += 3;

			sample.Center = Start;
			sample.NormalEnd = End;


			Samples->AddItem( sample );
		}
	}
}

// 2006. 12. 29 changmin
// parallel pvs computing
bool AvaPVSCalculator::LoadLauncher()
{
	FString DllFolder		= "..\\Tool\\AVARAD";
	FString BinariesFolder	= "..\\..\\Binaries";
	TCHAR_CALL_OS(SetCurrentDirectoryW(*DllFolder),SetCurrentDirectoryA(TCHAR_TO_ANSI(*DllFolder)));
	FString DllName = "vrad_launcher.dll";
	LauncherHandle_ = LoadLibrary( *DllName );
	TCHAR_CALL_OS(SetCurrentDirectoryW(*BinariesFolder),SetCurrentDirectoryA(TCHAR_TO_ANSI(*BinariesFolder)));
	if( !LauncherHandle_ )
	{
		return false;
	}
#pragma warning(push)
#pragma warning(disable:4191) // disable: unsafe conversion from 'type of expression' to 'type required'
	typedef IVRadLauncher* (*GetLauncherFn)(void);
	GetLauncherFn GetFn = (GetLauncherFn) GetProcAddress( LauncherHandle_, "GetLauncher" );
#pragma warning(pop)
	Launcher_ = NULL;
	Launcher_ = GetFn();
	if( !Launcher_ )
	{
		return false;
	}
	TCHAR_CALL_OS(SetCurrentDirectoryW(*DllFolder),SetCurrentDirectoryA(TCHAR_TO_ANSI(*DllFolder)));
	bool bInit = Launcher_->Initialize();
	TCHAR_CALL_OS(SetCurrentDirectoryW(*BinariesFolder),SetCurrentDirectoryA(TCHAR_TO_ANSI(*BinariesFolder)));
	return bInit;
}
bool AvaPVSCalculator::Compute( const FFullBuildGeometryOptions &Options )
{
	for (INT i=0; i<Options.Clusters.Num(); ++i)
	{
		Launcher_->AddCluster( TCHAR_TO_ANSI( *Options.Clusters(i) ) );
	}

	return Launcher_->RunPVS( this );
}

void AvaPVSCalculator::ShutDown()
{
	if( Launcher_ )
	{
		Launcher_->Finalize();
		FreeLibrary( LauncherHandle_ );
		Launcher_ = NULL;
		LauncherHandle_ = NULL;
	}
}

bool AvaPVSCalculator::IsCanceled()
{
	return !!GEditor->GetMapBuildCancelled();
}

#pragma optimize("", on)