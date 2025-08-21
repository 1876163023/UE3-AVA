#include "avaGame.h"

IMPLEMENT_CLASS(UavaUIVideo);
IMPLEMENT_CLASS(UavaUIKillCam);
IMPLEMENT_CLASS(UavaUIChatCam);

AavaPlayerController* GetavaPlayerOwner(INT Index=-1);

void DrawThickBox( FCanvas* Canvas, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT Thickness, const FLinearColor& DrawColor );

static FVector LocalToWorld(FVector const& LocalVect, FRotator const& SystemRot)
{
	return FRotationMatrix(SystemRot).TransformNormal( LocalVect );
}

void UavaUIVideo::FinishDestroy()
{
	if (ViewState != NULL)
	{
		FlushRenderingCommands();

		ViewState->Destroy();
	}	
	
	Super::FinishDestroy();
}

// implemented in UnPlayer.cpp
void InterpolatePostProcessSettings( FCurrentPostProcessVolumeInfo& CurrentPPInfo, 							   
									APostProcessVolume* NewVolume, 
									const FPostProcessSettings& NewSettings,
									FLOAT PP_DesaturationMultiplier,
									FLOAT PP_HighlightsMultiplier,
									FLOAT PP_MidTonesMultiplier,
									FLOAT PP_ShadowsMultiplier );

void UavaUIVideo::Render_Widget( FCanvas* Canvas )
{		
	const UBOOL bCurrentActive = Alpha > 0 && ViewXL > 0 && ViewYL > 0;	
	const UBOOL bJustStarted = (!bActive && (bCurrentActive ^ bActive));

	bActive = bCurrentActive;

	if (Alpha <= 0)
		return;
	else
	{
		struct FScreenPercentageGuard
		{
			UBOOL bNeedsRestore;
			FLOAT OrgScreenPercentage;
			FScreenPercentageGuard()
			{
				//Enter();
			}

			~FScreenPercentageGuard()
			{
				//Leave();
			}

			void Enter()
			{
				if (GSystemSettings->NeedsUpscale())
				{
					bNeedsRestore = TRUE;
					FlushRenderingCommands();

					OrgScreenPercentage = GSystemSettings->ScreenPercentage;

					GSystemSettings->ScreenPercentage = 100.0f;
				}			
				else
				{
					bNeedsRestore = FALSE;
				}				
			}

			void Leave()
			{			
				if (bNeedsRestore)
				{
					FlushRenderingCommands();
					GSystemSettings->ScreenPercentage = OrgScreenPercentage;
				}				
			}
		} ScopedScreenPercentage;

		if (ViewState == NULL)
		{
			ViewState = AllocateViewState();		
			ActorVisibilityHistory.Init();
		}

		INT ScaledX = appTrunc(ViewX), ScaledY = appTrunc(ViewY);
		UINT ScaledXL = appTrunc(ViewXL), ScaledYL = appTrunc(ViewYL);

		// Take screen percentage option into account if percentage != 100.
		GSystemSettings->ScaleScreenCoords(ScaledX,ScaledY,ScaledXL,ScaledYL);
		
		FViewport* Viewport = static_cast<FViewport*>( Canvas->GetRenderTarget() );
		FSceneViewFamilyContext ViewFamily(
			Viewport,
			GWorld->Scene,
			GEngine->GameViewport->ShowFlags & ~(SHOW_PostProcess | SHOW_Fog),	// Post process & fog는 끕니다. :) performance 때문에 껐습니다.
			GWorld->GetTimeSeconds(),
			GWorld->GetRealTimeSeconds(), NULL, FALSE, FALSE, FALSE, TRUE/*DisableForeground*/ );	

		check(Viewport);	

		// scale distances for cull distance purposes by the ratio of our current FOV to the default FOV
		FLOAT LODDistanceFactor = FOV / 90.0f;

		FMatrix ViewMatrix = FTranslationMatrix(-ViewLocation);
		ViewMatrix = ViewMatrix * FInverseRotationMatrix(ViewRotation);
		ViewMatrix = ViewMatrix * FMatrix(
			FPlane(0,	0,	1,	0),
			FPlane(1,	0,	0,	0),
			FPlane(0,	1,	0,	0),
			FPlane(0,	0,	0,	1));	

		FMatrix ProjectionMatrix;

		FLOAT CurViewAspectRatio = ((FLOAT)Viewport->GetSizeX()) / ((FLOAT)Viewport->GetSizeY());
		ProjectionMatrix = FPerspectiveMatrix(
			FOV * (FLOAT)PI / 360.0f,
			ScaledXL * Viewport->GetDesiredAspectRatio() / CurViewAspectRatio,
			ScaledYL,
			NEAR_CLIPPING_PLANE
			);

		FLinearColor OverlayColor(0,0,0,0);
		FLinearColor ColorScale(FLinearColor::White);	

		// Update the player's post process settings.
		FPostProcessSettings NewSettings;
		APostProcessVolume* NewVolume;
		NewVolume = GWorld->GetWorldInfo()->GetPostProcessSettings(ViewLocation,TRUE,NewSettings);
		
		InterpolatePostProcessSettings( CurrentPPInfo, NewVolume, NewSettings, 1.0f, 1.0f, 1.0f, 1.0f );

		TArray<FPrimitiveSceneInfo*> HiddenPrimitives;		

		FSceneView* View = new FSceneView(
			&ViewFamily,
			ViewState,
			&ActorVisibilityHistory,
			NULL,
			GEngine->DefaultPostProcess,
			&CurrentPPInfo.LastSettings,
			NULL,
			ScaledX,
			ScaledY,
			ScaledXL,
			ScaledYL,
			ViewMatrix,
			ProjectionMatrix,
			ProjectionMatrix,
			FOV,		
			FLinearColor::Black,
			OverlayColor,
			ColorScale,
			HiddenPrimitives,
			FALSE,
			LODDistanceFactor,
			1,
			1,
			Alpha
			);

		ViewFamily.Views.AddItem(View);

		if (View != NULL)
		{
			if (bJustStarted && View->State)
			{
				ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER( CopyExposureDataCommand, FSceneViewStateInterface*, Dest, ViewState, FSceneViewStateInterface*, Src, View->State,
				{
					void CopyExposureData( FSceneViewStateInterface*, FSceneViewStateInterface* );

					CopyExposureData( Dest, Src );
				});
			}

			View->State = ViewState;

			// Tonemapping!
			void ApplyTonemapParameters( FSceneView* View );
			ApplyTonemapParameters( View );

			// Update level streaming.
			GWorld->UpdateLevelStreaming( &ViewFamily );

			PreRenderScene(Canvas, View);

			BeginRenderingViewFamily(Canvas,&ViewFamily);			

			PostRenderScene(Canvas);			
		}		
	}
}

void UavaUIKillCam::Render_Widget( FCanvas* Canvas )
{
	FLOAT OldAlpha = Alpha;

	Alpha = 0.0f;
	
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();

	if (PlayerOwner == NULL)
		return;

	if ( PlayerOwner->KillCamTargetActor == NULL && PlayerOwner->KillCamStartedTime != 0)
	{
		PlayerOwner->KillCamStartedTime = 0.0f;			
	}		

	FLOAT& Timer = PlayerOwner->KillCamStartedTime;
	
	const FLOAT CurrentTime = GWorld->GetTimeSeconds();

	if (Timer > StartedTime)
	{
		StartedTime = CurrentTime;
	}	

	// Progress 계산
	Progress = (CurrentTime - StartedTime) / Max( 0.01f, Duration );		

	if (Progress > 1.0f || PlayerOwner->KillCamTargetActor == NULL)
	{
		return;
	}			

	FVector TargetLocation = PlayerOwner->KillCamTargetActor->Location;					

	if (bKillerCamera ^ PlayerOwner->KillCam_bIsKiller)
		return;

	if (!bKillerCamera)
	{
		// TargetLocation이 Collision Height만큼 높게 설정되어있다. 40은 대략 맞는 수치 :)
		TargetLocation.Z -= 40;
	}

	ViewX = RenderBounds[UIFACE_Left];
	ViewXL = RenderBounds[UIFACE_Right] - ViewX;
	ViewY = RenderBounds[UIFACE_Top];
	ViewYL = RenderBounds[UIFACE_Bottom] - ViewY;
	
	if (Progress < FadeIn)
	{
		Alpha = Square( Progress / FadeIn );		
	}
	else if (Progress > 1 - FadeOut)
	{
		Alpha = 1 - Square(( Progress - ( 1 - FadeOut )) / FadeOut );
	}				
	else
	{
		Alpha = 1.0f;
	}

	if (Alpha > 0 && OldAlpha <= 0)
	{
		FLOAT Distance = Min( (ViewLocation - TargetLocation).Size(), CameraDistance );

		KillCamLocation = (ViewLocation - TargetLocation).SafeNormal() * Distance + TargetLocation;
	}		

	if (bKillerCamera)
	{					
		KillCamLocation = TargetLocation + PlayerOwner->KillCamTargetActor->Rotation.Vector() * CameraDistance;					
	}

	ViewLocation = KillCamLocation;				
	ViewRotation = (TargetLocation - KillCamLocation).Rotation();				
	
	const FLOAT SourceFOV = 90.0f;
	const FLOAT TargetFOV = appAtan2( bKillerCamera ? Killer_FOV : Killee_FOV, (TargetLocation - KillCamLocation).Size() ) * 180.0f / PI * 2.0f;
	const FLOAT FOV_Alpha = Clamp( Progress * FOV_Speed, 0.f, 1.f );

	FOV = (Min( SourceFOV, TargetFOV ) - SourceFOV) * FOV_Alpha + SourceFOV;

	Super::Render_Widget( Canvas );
	
	DrawThickBox( Canvas, ViewX - Border_Thickness, ViewY - Border_Thickness, ViewXL + 2 * Border_Thickness, ViewYL + 2 * Border_Thickness, Border_Thickness, FLinearColor( Border_Color.R, Border_Color.G, Border_Color.B, Alpha ) );
}


void UavaUIChatCam::Render_Widget( FCanvas* Canvas ) 
{
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();

	if (PlayerOwner == NULL)
		return;

	if ( PlayerOwner->ChatCamActor == NULL && PlayerOwner->ChatCamStartedTime != 0)
	{
		PlayerOwner->ChatCamStartedTime = 0.0f;			
	}		

	FLOAT& Timer = PlayerOwner->ChatCamStartedTime;

	const FLOAT CurrentTime = GWorld->GetTimeSeconds();

	if (Timer > StartedTime)
	{
		StartedTime = CurrentTime;
	}	
	
	// Progress 계산
	Progress = (CurrentTime - StartedTime) / Max( 0.01f, Duration );		

	Alpha = 0.0f;

	if (Progress > 1.0f)
	{
		return;
	}			

	// Calculate the player's view information.
	AavaPawn* Pawn = Cast<AavaPawn>( PlayerOwner->ChatCamActor );
	if (!Pawn)
		return;		

	static FName NAME_BipHead( TEXT("Bip01_Head") );
	static FName NAME_BipSpine3( TEXT("Bip01_Spine3") );

	FVector Location = Pawn->Mesh->GetBoneLocation( NAME_BipHead );	
	FVector Location_Spine = Pawn->Mesh->GetBoneLocation( NAME_BipSpine3 );

	Location.Z = Location_Spine.Z;
	Location.X = (Location.X + Location_Spine.X) * 0.5f;
	Location.Y = (Location.Y + Location_Spine.Y) * 0.5f;

	static FVector SpineToHeadOffset( -12.0f, 0, 13.0f );

	FVector TargetLocation = Location + LocalToWorld( SpineToHeadOffset + TargetOffset, Pawn->Rotation );
	FVector CamLocation = Location + LocalToWorld( SpineToHeadOffset + CameraLocation, Pawn->Rotation );

	ViewX = RenderBounds[UIFACE_Left];
	ViewXL = RenderBounds[UIFACE_Right] - ViewX;
	ViewY = RenderBounds[UIFACE_Top];
	ViewYL = RenderBounds[UIFACE_Bottom] - ViewY;
	
	Alpha = 1.0f;		
	if (Progress > 1 - FadeOut)
	{
		Alpha = 1 - Square(( Progress - ( 1 - FadeOut )) / FadeOut );			
	}				

	ViewLocation = CamLocation;				
	ViewRotation = (TargetLocation - CamLocation).Rotation();				

	FOV = appAtan2( Radius, (TargetLocation - CamLocation).Size() ) * 180.0f / PI * 2.0f;		

	Super::Render_Widget( Canvas );
	
	DrawThickBox( Canvas, ViewX - Border_Thickness, ViewY - Border_Thickness, ViewXL + 2 * Border_Thickness, ViewYL + 2 * Border_Thickness, Border_Thickness, FLinearColor( Border_Color.R, Border_Color.G, Border_Color.B, Alpha ) );
}	