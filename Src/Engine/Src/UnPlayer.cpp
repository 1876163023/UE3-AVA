/*=============================================================================
	UnPlayer.cpp: Unreal player implementation.
	Copyright 2004-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineUserInterfaceClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineUIPrivateClasses.h"
#include "EngineParticleClasses.h"
#include "EngineInterpolationClasses.h"
#include "EngineAnimClasses.h"
#include "EngineDSPClasses.h"

// needed for adding components when typing "show paths" in game
#include "EngineAIClasses.h"

#include "UnStatChart.h"
#include "UnTerrain.h"
#include "UnSubtitleManager.h"
#include "UnNet.h"

#include "ScenePrivate.h"
#include "BSPDynamicBatch.h"

IMPLEMENT_CLASS(UPlayer);
IMPLEMENT_CLASS(ULocalPlayer);





/** High-res screenshot variables */
UBOOL		GIsTiledScreenshot = FALSE;
INT			GScreenshotResolutionMultiplier = 2;
INT			GScreenshotMargin = 64;	// In pixels
INT			GScreenshotTile = 0;
FIntRect	GScreenshotRect;		// Rendered tile
/** TRUE if we should grab a screenshot at the end of the frame */
UBOOL		GScreenShotRequest = FALSE;

//<@ ava specific ; 2006. 9. 12 changmin
UBOOL		GIsMinimapshot = FALSE;
UINT		GMinimapshotWidth = 64;
UINT		GMinimapshotHeight = 64;
UBOOL		GHasMinimapshot = FALSE;
//>@ ava

//<@ ava specific ; 2007. 4. 12 changmin
UBOOL		GDumpHdrScene = FALSE;
//>@ ava

UBOOL		ULocalPlayer::bOverrideView = FALSE;
FVector		ULocalPlayer::OverrideLocation;
FRotator	ULocalPlayer::OverrideRotation;
FLOAT		ULocalPlayer::OverrideFOV = 90.0f;

IMPLEMENT_CLASS(UGameViewportClient);

/** Whether to show the FPS counter */
UBOOL GShowFpsCounter = FALSE;
/** Whether to show the level stats */
static UBOOL GShowLevelStats = FALSE;	

/** Whether to show the CPU thread and GPU frame times */
UBOOL GShowUnitTimes = FALSE;

#ifdef _BT_TEST_BY_CRAZY
UBOOL GShowBandwidthStat = FALSE;
#endif

/**
*	Renders the FPS counter
*
*	@param Viewport	The viewport to render to
*	@param Canvas	Canvas object to use for rendering
*	@param X		Suggested X coordinate for where to start drawing
*	@param Y		Suggested Y coordinate for where to start drawing
*	@return			Y coordinate of the next line after this output
*/
INT DrawFPSCounter( FViewport* Viewport, FCanvas* Canvas, INT X, INT Y );

/**
*	Draws frame times for the overall frame, gamethread, renderthread and GPU.
*	The gamethread time excludes idle time while it's waiting for the render thread.
*	The renderthread time excludes idle time while it's waiting for more commands from the gamethread or waiting for the GPU to swap backbuffers.
*	The GPU time is a measurement on the GPU from the beginning of the first drawcall to the end of the last drawcall. It excludes
*	idle time while waiting for VSYNC. However, it will include any starvation time between drawcalls.
*
*	@param Viewport	The viewport to render to
*	@param Canvas	Canvas object to use for rendering
*	@param X		Suggested X coordinate for where to start drawing
*	@param Y		Suggested Y coordinate for where to start drawing
*	@return			Y coordinate of the next line after this output
*/
INT DrawUnitTimes( FViewport* Viewport, FCanvas* Canvas, INT X, INT Y );

/**
*	Render the level stats
*
*	@param Viewport	The viewport to render to
*	@param Canvas	Canvas object to use for rendering
*	@param X		Suggested X coordinate for where to start drawing
*	@param Y		Suggested Y coordinate for where to start drawing
*	@return			Y coordinate of the next line after this output
*/
INT DrawLevelStats( FViewport* Viewport, FCanvas* Canvas, INT X, INT Y );

UBOOL GShouldLogOutAFrameOfSkelCompTick = FALSE;

UGameViewportClient::UGameViewportClient():
	ShowFlags((SHOW_DefaultGame&~SHOW_ViewMode_Mask)|SHOW_ViewMode_Lit)
{
	GUsingBSPDynamicBatch = !GIsEditor;

#if !FINAL_RELEASE
	if(ParseParam(appCmdLine(),TEXT("DynamicBSP")))
		GUsingBSPDynamicBatch = TRUE;

	if(ParseParam(appCmdLine(),TEXT("StaticBSP")))
		GUsingBSPDynamicBatch = FALSE;
#endif
}

void UGameViewportClient::FinishDestroy()
{
	Super::FinishDestroy();
}

/**
 * Called every frame to allow the game viewport to update time based state.
 * @param	DeltaTime - The time since the last call to Tick.
 */
void UGameViewportClient::Tick( FLOAT DeltaTime )
{
	// first call the unrealscript tick
	eventTick(DeltaTime);

	// now tick all interactions
	for ( INT i = 0; i < GlobalInteractions.Num(); i++ )
	{
		UInteraction* Interaction = GlobalInteractions(i);
		Interaction->Tick(DeltaTime);
	}
}

FString UGameViewportClient::ConsoleCommand(const FString& Command)
{
	FString TruncatedCommand = Command.Left(1000);
	FConsoleOutputDevice ConsoleOut(ViewportConsole);
	Exec(*TruncatedCommand,ConsoleOut);
	return *ConsoleOut;
}

/**
 * Routes an input key event received from the viewport to the Interactions array for processing.
 *
 * @param	Viewport		the viewport the input event was received from
 * @param	ControllerId	gamepad/controller that generated this input event
 * @param	Key				the name of the key which an event occured for (KEY_Up, KEY_Down, etc.)
 * @param	EventType		the type of event which occured (pressed, released, etc.)
 * @param	AmountDepressed	(analog keys only) the depression percent.
 * @param	bGamepad - input came from gamepad (ie xbox controller)
 *
 * @return	TRUE to consume the key event, FALSE to pass it on.
 */
UBOOL UGameViewportClient::InputKey(FViewport* Viewport,INT ControllerId,FName Key,EInputEvent EventType,FLOAT AmountDepressed,UBOOL bGamepad)
{

#if !CONSOLE
	// if a movie is playing then handle input key
	if( GFullScreenMovie && 
		GFullScreenMovie->GameThreadIsMoviePlaying(TEXT("")) )
	{
		if (GFullScreenMovie->InputKey(Viewport,ControllerId,Key,EventType,AmountDepressed,bGamepad))		
		{
			return TRUE;
		}
	}
#endif

	UBOOL bResult = FALSE;

	bResult = eventInputKey(ControllerId,Key,EventType,AmountDepressed,bGamepad);

	return bResult;
}

/**
 * Routes an input axis (joystick, thumbstick, or mouse) event received from the viewport to the Interactions array for processing.
 *
 * @param	Viewport		the viewport the input event was received from
 * @param	ControllerId	the controller that generated this input axis event
 * @param	Key				the name of the axis that moved  (KEY_MouseX, KEY_XboxTypeS_LeftX, etc.)
 * @param	Delta			the movement delta for the axis
 * @param	DeltaTime		the time (in seconds) since the last axis update.
 *
 * @return	TRUE to consume the axis event, FALSE to pass it on.
 */
UBOOL UGameViewportClient::InputAxis(FViewport* Viewport,INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime)
{
#if 0
	UBOOL bResult = FALSE;

	// give script the chance to process this input first
	if ( DELEGATE_IS_SET(HandleInputAxis) )
	{
		bResult = delegateHandleInputAxis(ControllerId, Key, Delta, DeltaTime);
	}

	// if it wasn't handled by script, route to the interactions array
	for ( INT InteractionIndex = 0; !bResult && InteractionIndex < GlobalInteractions.Num(); InteractionIndex++ )
	{
		UInteraction* Interaction = GlobalInteractions(InteractionIndex);
	}
	return bResult;
#else
	return eventInputAxis(ControllerId,Key,Delta,DeltaTime);
#endif
}

/**
 * Routes a character input event (typing) received from the viewport to the Interactions array for processing.
 *
 * @param	Viewport		the viewport the input event was received from
 * @param	ControllerId	the controller that generated this character input event
 * @param	Character		the character that was typed
 *
 * @return	TRUE to consume the key event, FALSE to pass it on.
 */
UBOOL UGameViewportClient::InputChar(FViewport* Viewport,INT ControllerId, const FInputCompositionStringData& CompStrData )
{
#if 0
	UBOOL bResult = FALSE;

	if ( DELEGATE_IS_SET(HandleInputChar) )
	{
		// should probably just add a ctor to FString that takes a TCHAR
		FString StringCharacter;
		StringCharacter += Character;

		bResult = delegateHandleInputChar(ControllerId, StringCharacter);
	}

	// if it wasn't handled by script, route to the interactions array
	for ( INT InteractionIndex = 0; !bResult && InteractionIndex < GlobalInteractions.Num(); InteractionIndex++ )
	{
		UInteraction* Interaction = GlobalInteractions(InteractionIndex);
	}

	return bResult;
#else
	// should probably just add a ctor to FString that takes a TCHAR

	return eventInputChar(ControllerId, CompStrData);
#endif
}

//  Process Candidate Window
UBOOL UGameViewportClient::InputCandidate(FViewport* Viewport, UINT ControllerID, const FInputCandidateStringData& CandStrData )
{
	return eventInputCandidate(ControllerID, CandStrData);
}

UBOOL UGameViewportClient::InputReadingString(FViewport *Viewport, UINT ControllerID, const FInputReadingStringData& ReadStrData )
{
	return eventInputReadingString(ControllerID, ReadStrData);
}

/**
 * Returns the forcefeedback manager associated with the PlayerController.
 */
class UForceFeedbackManager* UGameViewportClient::GetForceFeedbackManager(INT ControllerId)
{
	for(INT PlayerIndex = 0;PlayerIndex < GEngine->GamePlayers.Num();PlayerIndex++)
	{
		ULocalPlayer* Player = GEngine->GamePlayers(PlayerIndex);
		if(Player->ViewportClient == this && Player->ControllerId == ControllerId)
		{
			return Player->Actor ? Player->Actor->ForceFeedbackManager : NULL;
		}
	}

	return NULL;
}

/**
 * Determines whether this viewport client should receive calls to InputAxis() if the game's window is not currently capturing the mouse.
 * Used by the UI system to easily receive calls to InputAxis while the viewport's mouse capture is disabled.
 */
UBOOL UGameViewportClient::RequiresUncapturedAxisInput() const
{
	return Viewport != NULL && bDisplayingUIMouseCursor == TRUE;
}


/**
 * Retrieves the cursor that should be displayed by the OS
 *
 * @param	Viewport	the viewport that contains the cursor
 * @param	X			the x position of the cursor
 * @param	Y			the Y position of the cursor
 * 
 * @return	the cursor that the OS should display
 */
EMouseCursor UGameViewportClient::GetCursor( FViewport* Viewport, INT X, INT Y )
{
	UBOOL bIsPlayingMovie = GFullScreenMovie && GFullScreenMovie->GameThreadIsMoviePlaying( TEXT("") );

#if CONSOLE || PLATFORM_UNIX
	UBOOL bIsWithinTitleBar = FALSE;
#else
	POINT CursorPos = { X, Y };
	RECT WindowRect;
	ClientToScreen( (HWND)Viewport->GetWindow(), &CursorPos );
	GetWindowRect( (HWND)Viewport->GetWindow(), &WindowRect );
	UBOOL bIsWithinWindow = ( CursorPos.x >= WindowRect.left && CursorPos.x <= WindowRect.right &&
		CursorPos.y >= WindowRect.top && CursorPos.y <= WindowRect.bottom );

	// The user is mousing over the title bar if Y is less than zero and within the window rect
	UBOOL bIsWithinTitleBar = Y < 0 && bIsWithinWindow;
#endif

	if ( (bDisplayingUIMouseCursor || !bIsPlayingMovie) && (Viewport->IsFullscreen() || !bIsWithinTitleBar) )
	{
		//@todo - returning MC_None causes the OS to not render a mouse...might want to change
		// this so that UI can indicate which mouse cursor should be displayed
		return MC_None;
	}

	return FViewportClient::GetCursor(Viewport, X, Y);
}

/**
 * Set this GameViewportClient's viewport to the viewport specified
 */
void UGameViewportClient::SetViewport( FViewportFrame* InViewportFrame )
{
	FViewport* PreviousViewport = Viewport;
	ViewportFrame = InViewportFrame;
	Viewport = ViewportFrame ? ViewportFrame->GetViewport() : NULL;

	if ( PreviousViewport == NULL && Viewport != NULL )
	{
		// ensure that the player's Origin and Size members are initialized the moment we get a viewport
		eventLayoutPlayers();
	}

	if ( UIController != NULL )
	{
		UIController->SceneClient->SetRenderViewport(Viewport);
	}

}

/**
 * Retrieve the size of the main viewport.
 *
 * @param	out_ViewportSize	[out] will be filled in with the size of the main viewport
 */
void UGameViewportClient::GetViewportSize( FVector2D& out_ViewportSize )
{
	if ( Viewport != NULL )
	{
		out_ViewportSize.X = Viewport->GetSizeX();
		out_ViewportSize.Y = Viewport->GetSizeY();
	}
}

/** @return Whether or not the main viewport is fullscreen or windowed. */
UBOOL UGameViewportClient::IsFullScreenViewport()
{
	return Viewport->IsFullscreen();
}

/** Whether we should precache during the next frame. */
UBOOL GPrecacheNextFrame = FALSE;
/** Whether texture memory has been corrupted because we ran out of memory in the pool. */
UBOOL GIsTextureMemoryCorrupted = FALSE;
extern UBOOL GDoNotApplyCullDistances;

void ApplyTonemapParameters( FSceneView* View )
{
	((FSceneViewState*)(View->State))->ExposureData.ExposureCenterRegionX = View->ExposureCenterRegionX;
	((FSceneViewState*)(View->State))->ExposureData.ExposureCenterRegionY = View->ExposureCenterRegionY;

	if (View->PostProcessSettings)
	{		
		((FSceneViewState*)(View->State))->ExposureData.LuminanceHistogram.MinScale = View->PostProcessSettings->Tonemap_MinExposure;
		((FSceneViewState*)(View->State))->ExposureData.LuminanceHistogram.MaxScale = View->PostProcessSettings->Tonemap_MaxExposure;										
		((FSceneViewState*)(View->State))->ExposureData.bUseDynamicTonemapping = View->PostProcessSettings->bEnableTonemap;
		((FSceneViewState*)(View->State))->ExposureData.LuminanceHistogram.InvLmaxSquare = 1.0f / ( View->PostProcessSettings->Tonemap_Lmax * View->PostProcessSettings->Tonemap_Lmax );
		((FSceneViewState*)(View->State))->ExposureData.LuminanceHistogram.K = View->PostProcessSettings->Tonemap_Darkness;
		((FSceneViewState*)(View->State))->ExposureData.LuminanceHistogram.Beta = View->PostProcessSettings->Tonemap_Beta;
		((FSceneViewState*)(View->State))->ExposureData.LuminanceHistogram.LavgScale = View->PostProcessSettings->Tonemap_LavgScale;
		((FSceneViewState*)(View->State))->ExposureData.LuminanceHistogram.ScaleBias = View->PostProcessSettings->Tonemap_ScaleBias;
		((FSceneViewState*)(View->State))->ExposureData.LuminanceHistogram.KeyValueScale = View->PostProcessSettings->Tonemap_KeyValueScale;
		((FSceneViewState*)(View->State))->ExposureData.LuminanceHistogram.KeyValueOffset = View->PostProcessSettings->Tonemap_KeyValueOffset;
		((FSceneViewState*)(View->State))->ExposureData.LuminanceHistogram.MinKeyValue = View->PostProcessSettings->Tonemap_MinKeyValue;
		((FSceneViewState*)(View->State))->ExposureData.LuminanceHistogram.MaxKeyValue = View->PostProcessSettings->Tonemap_MaxKeyValue;
		((FSceneViewState*)(View->State))->ExposureData.LuminanceHistogram.Shadow = View->PostProcessSettings->Tonemap_Shadow;
		((FSceneViewState*)(View->State))->ExposureData.LuminanceHistogram.Highlight = View->PostProcessSettings->Tonemap_Highlight;
	}
	else
	{
		((FSceneViewState*)(View->State))->ExposureData.bUseDynamicTonemapping = FALSE;
	}	

	if (!(View->Family->ShowFlags & SHOW_Lighting || View->Family->ShowFlags & SHOW_BumpLighting))
	{
		((FSceneViewState*)(View->State))->ExposureData.bUseDynamicTonemapping = FALSE;
	}
}

#if !FINAL_RELEASE	
void DrawTonemapInfo( FCanvas* Canvas, INT& Y, const FSceneView* View )
{
	FSceneViewState* ViewState = (FSceneViewState*)(View->State);	

	if (ViewState)
	{		
		ViewState->ExposureData.DisplayDebugInfo(Canvas, Y);			
	}
}
#endif

// @HACK deif added HUD/UI
//
// avaHUD에서 SceneView를 얻어내야 해서... :) (UI에서 Player의 scene view를 얻기 위함)
// 사실은 UIScene에 있는 LocalPlayer에서 얻어내는 게 가장 아름다운 방법이나... -_-;
// 일단 이렇게 했음. 나중에 수정하게 되면 고치도록!!!
FSceneView* GPlayerSceneView = NULL;

#if !FINAL_RELEASE
extern UBOOL GUpdatingCullDistances;
extern INT GCullDistanceUpdateCount, GCullDistanceUpdateCount2;
#endif

FMatrix GTiledShotTweakMatrix;
FMatrix GOriginalProjectionMatrixForTiledShot;

void TakeScreenshot( FViewport* Viewport )
{
	if( GIsDumpingMovie || GScreenShotRequest )
	{
#if PLATFORM_SUPPORTS_SCREENSHOT_FROM_SCREEN
		struct FScopedHackDIBScreenshot
		{
			FScopedHackDIBScreenshot()
			{
				extern UBOOL GHackDIBScreenshot;
				GHackDIBScreenshot = TRUE;
			}

			~FScopedHackDIBScreenshot()
			{
				extern UBOOL GHackDIBScreenshot;
				GHackDIBScreenshot = FALSE;
			}
		} ScopedHackDIBScreenshot;		
#endif

		// Read the contents of the viewport into an array.
		TArray<FColor>	Bitmap;
		if(Viewport->ReadPixels(Bitmap))
		{
			check(Bitmap.Num() == Viewport->GetSizeX() * Viewport->GetSizeY());

			// Create screenshot folder if not already present.
			GFileManager->MakeDirectory( *GSys->ScreenShotPath, TRUE );

			FString ScreenFileName( GSys->ScreenShotPath * (GIsDumpingMovie ? TEXT("MovieFrame") : TEXT("ScreenShot")) );

			TCHAR File[MAX_PATH];
			for( INT TestBitmapIndex=0; TestBitmapIndex<65536; TestBitmapIndex++ )
			{
				appSprintf( File, TEXT("%s%05i.bmp"), *ScreenFileName, TestBitmapIndex );
				if( GFileManager->FileSize(File) < 0 )
					break;
			}
			FString AbsolutePath = appConvertRelativePathToFull( File );
			for( AController* Controller = GWorld->GetWorldInfo()->ControllerList; Controller != NULL; Controller = Controller->NextController )
			{
				APlayerController* PC = Cast<APlayerController>( Controller );					
				if (PC != NULL)
				{							
					PC->eventNotifyTakeScreenShot( AbsolutePath  );
					break;
				}								
			}

			// Save the contents of the array to a bitmap file.
			appCreateBitmap(File,Viewport->GetSizeX(),Viewport->GetSizeY(),&Bitmap(0),GFileManager);			

		}
		GScreenShotRequest=FALSE;
	}
}

void UGameViewportClient::Draw(FViewport* Viewport,FCanvas* Canvas)
{
	if(GPrecacheNextFrame)
	{

#if !CONSOLE
		ENQUEUE_UNIQUE_RENDER_COMMAND(
			FSetCullDistances,		
		{
			GDoNotApplyCullDistances = TRUE;
		});

		FSceneViewFamily PrecacheViewFamily(Viewport,GWorld->Scene,ShowFlags,GWorld->GetTimeSeconds(),GWorld->GetRealTimeSeconds(),NULL);
		PrecacheViewFamily.Views.AddItem(
			new FSceneView(
				&PrecacheViewFamily,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				0,
				0,
				1,
				1,
				FMatrix(
					FPlane(1.0f / WORLD_MAX,0.0f,0.0f,0.0f),
					FPlane(0.0f,1.0f / WORLD_MAX,0.0f,0.0f),
					FPlane(0.0f,0.0f,1.0f / WORLD_MAX,0.0f),
					FPlane(0.5f,0.5f,0.5f,1.0f)
					),
				FMatrix::Identity,
				/* precache에서도 foreground dpg가 없어야~~~ */
				FMatrix::Identity,
				90,
				/* 없어야~~~ 끝 */
				FLinearColor::Black,
				FLinearColor::Black,
				FLinearColor::White,
				TArray<FPrimitiveSceneInfo*>()
				)
			);
		BeginRenderingViewFamily(Canvas,&PrecacheViewFamily);

		ENQUEUE_UNIQUE_RENDER_COMMAND(
			FResetCullDistances,		
		{
			GDoNotApplyCullDistances = FALSE;
		});
#endif

		GPrecacheNextFrame = FALSE;
	}

	// Create a temporary canvas if there isn't already one.
	UCanvas* CanvasObject = FindObject<UCanvas>(UObject::GetTransientPackage(),TEXT("CanvasObject"));
	if( !CanvasObject )
	{
		CanvasObject = ConstructObject<UCanvas>(UCanvas::StaticClass(),UObject::GetTransientPackage(),TEXT("CanvasObject"));
		CanvasObject->AddToRoot();
	}
	CanvasObject->Canvas = Canvas;

	// Setup a FSceneViewFamily for the player.
	if( GWorld->bGatherDynamicShadowStatsSnapshot )
	{
		// Start with clean slate.
		GWorld->DynamicShadowStats.Empty();
	}
	
#if !FINAL_RELEASE
	if (GUpdatingCullDistances)
	{
		void DrawCullDistanceDebugInfo();
		DrawCullDistanceDebugInfo();
	}
#endif

	FSceneViewFamilyContext ViewFamily(Viewport,GWorld->Scene,ShowFlags,GWorld->GetTimeSeconds(),GWorld->GetRealTimeSeconds(), GWorld->bGatherDynamicShadowStatsSnapshot ? &GWorld->DynamicShadowStats : NULL, TRUE );

	UBOOL bCanApplyCullDistances = TRUE;
	for(INT PlayerIndex = 0;PlayerIndex < GEngine->GamePlayers.Num();PlayerIndex++)
	{
		ULocalPlayer* Player = GEngine->GamePlayers(PlayerIndex);
		if (!Player->Actor || !Player->Actor->CanApplyCullDistance())
		{
			bCanApplyCullDistances = FALSE;
			break;
		}		
	}

	if (!bCanApplyCullDistances)
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND(
			FSetCullDistances,		
		{
			GDoNotApplyCullDistances = TRUE;
		});
	}

	TMap<INT,FSceneView*> PlayerViewMap;
	UBOOL bHaveReverbSettingsBeenSet = FALSE;
	for(INT PlayerIndex = 0;PlayerIndex < GEngine->GamePlayers.Num();PlayerIndex++)
	{
		ULocalPlayer* Player = GEngine->GamePlayers(PlayerIndex);
		if (Player->Actor)
		{
			// Calculate the player's view information.
			FVector		ViewLocation;
			FRotator	ViewRotation;

			FSceneView* View = Player->CalcSceneView( &ViewFamily, ViewLocation, ViewRotation, Viewport );
			
			if(View)
			{
				ApplyTonemapParameters( View );

				//<@ ava specific ; 2007. 9. 10 changmin
				View->Ava_SplitViewFrustum();
				//>@ ava

				// Tiled rendering for high-res screenshots.
				if ( GIsTiledScreenshot )
				{
					// Calculate number of overlapping tiles:
					INT TileWidth	= Viewport->GetSizeX();
					INT TileHeight	= Viewport->GetSizeY();
					INT TotalWidth	= GScreenshotResolutionMultiplier * TileWidth;
					INT TotalHeight	= GScreenshotResolutionMultiplier * TileHeight;
					INT NumColumns	= appCeil( FLOAT(TotalWidth) / FLOAT(TileWidth - 2*GScreenshotMargin) );
					INT NumRows		= appCeil( FLOAT(TotalHeight) / FLOAT(TileHeight - 2*GScreenshotMargin) );
					TileWidth		= appTrunc(View->SizeX);
					TileHeight		= appTrunc(View->SizeY);
					TotalWidth		= GScreenshotResolutionMultiplier * TileWidth;
					TotalHeight		= GScreenshotResolutionMultiplier * TileHeight;

					// Report back to UD3DRenderDevice::TiledScreenshot():
					GScreenshotRect.Min.X = appTrunc(View->X);
					GScreenshotRect.Min.Y = appTrunc(View->Y);
					GScreenshotRect.Max.X = appTrunc(View->X + View->SizeX);
					GScreenshotRect.Max.Y = appTrunc(View->Y + View->SizeY);

					// Calculate tile position (upper-left corner, screen space):
					INT TileRow		= GScreenshotTile / NumColumns;
					INT TileColumn	= GScreenshotTile % NumColumns;
					INT PosX		= TileColumn*TileWidth - (2*TileColumn + 1)*GScreenshotMargin;
					INT PosY		= TileRow*TileHeight - (2*TileRow + 1)*GScreenshotMargin;

					// Calculate offsets to center tile (screen space):
					INT OffsetX		= (TotalWidth - TileWidth) / 2 - PosX;
					INT OffsetY		= (TotalHeight - TileHeight) / 2 - PosY;

					// Convert to projection space:
					FLOAT Scale		= FLOAT(GScreenshotResolutionMultiplier);
					FLOAT OffsetXp	= 2.0f * FLOAT(OffsetX) / FLOAT(TotalWidth);
					FLOAT OffsetYp	= -2.0f * FLOAT(OffsetY) / FLOAT(TotalHeight);

					// Apply offsets and scales:
					FTranslationMatrix OffsetMtx( FVector( OffsetXp, OffsetYp, 0.0f) );
					FScaleMatrix ScaleMtx( FVector(Scale, Scale, 1.0f) );

					GTiledShotTweakMatrix = OffsetMtx * ScaleMtx;
					GOriginalProjectionMatrixForTiledShot = View->ProjectionMatrix;
					View->ViewProjectionMatrix = View->ViewMatrix * View->ProjectionMatrix * GTiledShotTweakMatrix;
					View->SavedProjectionMatrix = View->ProjectionMatrix * GTiledShotTweakMatrix;
					View->ForegroundProjectionMatrix = View->ForegroundProjectionMatrix * GTiledShotTweakMatrix;
					View->InvViewProjectionMatrix = View->ViewProjectionMatrix.Inverse();
				}

				//<@ ava specific ; 2006. 9. 12 changmin
				if( GIsMinimapshot )
				{
					GHasMinimapshot = FALSE;

					AWorldInfo* WorldInfo = GWorld ? GWorld->GetWorldInfo() : NULL;
					FLOAT MeterPerTexel = WorldInfo && WorldInfo->MinimapMeterPerTexel > 0.f ?  WorldInfo->MinimapMeterPerTexel : 
										GEngine->MeterPerTexel > 0.0f ? GEngine->MeterPerTexel : 1.0f;

					// 1 meter = 53 uu
					const FLOAT UUToMeter = 1.0f / 53.0f;
					const FLOAT MeterToUU = 53.0f;

					// find volume
					AMinimapVolume* MinimapVolume = NULL;
					for( TObjectIterator<AMinimapVolume> It; It; ++It )
					{
						MinimapVolume = *It;
					}
					
					if( MinimapVolume )
					{
						// compute view, projection, viewprojection, inverse view projection matrix
						FBox Box = MinimapVolume->BrushComponent->Bounds.GetBox();

						FVector CameraLocation = Box.GetCenter();
						CameraLocation.Z = Box.Max.Z;

						FLOAT XSize = Box.Max.X - Box.Min.X;	// height 
						FLOAT YSize = Box.Max.Y - Box.Min.Y;	// width

						GMinimapshotWidth = (UINT)(YSize * UUToMeter / MeterPerTexel);
						GMinimapshotHeight = (UINT)(XSize * UUToMeter / MeterPerTexel);

						const UINT XSizeInUU = Viewport->GetSizeX() * MeterPerTexel * MeterToUU;
						const UINT YSizeInUU = Viewport->GetSizeY() * MeterPerTexel * MeterToUU;
				

						if( GMinimapshotWidth <= Viewport->GetSizeX() && GMinimapshotHeight <= Viewport->GetSizeY() )
						{
							static FMatrix ViewMatrix;
							
							ViewMatrix = FTranslationMatrix(-CameraLocation);

							//OrthoXY
							ViewMatrix = ViewMatrix * FMatrix(
									FPlane(0,	1,	0,					0),
									FPlane(1,	0,	0,					0),
									FPlane(0,	0,	-1,					0),
									FPlane(0,	0,	0,	1));

							FMatrix	ProjectionMatrix = FOrthoMatrix(
								XSizeInUU / 2.0f,
								YSizeInUU / 2.0f,
								0.5f / HALF_WORLD_MAX,
								HALF_WORLD_MAX
								);

							View->ViewMatrix = ViewMatrix;
							View->ProjectionMatrix = ProjectionMatrix;
							View->ViewProjectionMatrix = ViewMatrix * ProjectionMatrix;
							View->InvViewProjectionMatrix = View->ViewProjectionMatrix.Inverse();

							// Derive the view frustum from the view projection matrix.
							View->ViewFrustum = GetViewFrustumBounds(View->ViewProjectionMatrix,FALSE);

							// Derive the view's near clipping distance and plane.
							View->bHasNearClippingPlane = View->ViewProjectionMatrix.GetFrustumNearPlane(View->NearClippingPlane);
							View->NearClippingDistance = Abs(ProjectionMatrix.M[2][2]) > DELTA ? -ProjectionMatrix.M[3][2] / ProjectionMatrix.M[2][2] : 0.0f;

							// Calculate the view origin from the view/projection matrices.
							if(ProjectionMatrix.M[3][3] < 1.0f)
							{
								View->ViewOrigin = FVector4(ViewMatrix.Inverse().GetOrigin(),1);
							}
							else
							{
								View->ViewOrigin = FVector4(ViewMatrix.Inverse().TransformNormal(FVector(0,0,-1)).SafeNormal(),0);
							}

							GHasMinimapshot = TRUE;
						}
						else
						{
							warnf(NAME_Warning, TEXT("Minimap SIze(%d,%d) is Bigger than ViewportSize(%d,%d)"), GMinimapshotWidth, GMinimapshotHeight, Viewport->GetSizeX(), Viewport->GetSizeY());
							return;
						}
					}
					else
					{
						warnf(NAME_Warning, TEXT("Level has no Minimap Volume"));
						return;

					}
				}
				//>@ ava

				PlayerViewMap.Set(PlayerIndex,View);

				// Update the listener.
				check(GEngine->Client);
				UAudioDevice* AudioDevice = GEngine->Client->GetAudioDevice();
				if( AudioDevice )
				{
					FMatrix CameraToWorld		= View->ViewMatrix.Inverse();

					FVector ProjUp				= CameraToWorld.TransformNormal(FVector(0,1000,0));
					FVector ProjRight			= CameraToWorld.TransformNormal(FVector(1000,0,0));
					FVector ProjFront			= ProjRight ^ ProjUp;

					ProjUp.Z = Abs( ProjUp.Z ); // Don't allow flipping "up".

					ProjUp.Normalize();
					ProjRight.Normalize();
					ProjFront.Normalize();

					AudioDevice->SetListener(PlayerIndex, ViewLocation, ProjUp, ProjRight, ProjFront);

					// Update reverb settings.					
					AReverbVolume* ReverbVolume = GWorld->GetWorldInfo()->GetReverbSettings( ViewLocation, TRUE/*bReverbVolumePrevis*/ );

					UDSPPreset* NewDSPPreset = ReverbVolume ? ReverbVolume->DSPPreset : NULL;					

					AudioDevice->SetReverbSettings( ReverbVolume ? ReverbVolume->Settings : GWorld->GetWorldInfo()->DefaultReverbSettings );

					if (NewDSPPreset != LastDSPPreset)
					{
						if (NewDSPPreset)
						{
							NewDSPPreset->Apply();							
						}
						else
						{
							LastDSPPreset->Stop();
						}

						LastDSPPreset = NewDSPPreset;
					}

					USoundCue* NewAmbientSoundCue = ReverbVolume ? ReverbVolume->AmbientSound : NULL;

					if (NewAmbientSoundCue != LastAmbientSound)
					{
						for( AController* Controller = GWorld->GetWorldInfo()->ControllerList; 
							Controller != NULL; 
							Controller = Controller->NextController
							)
						{
							APlayerController* PC = Cast<APlayerController>( Controller );					

							if (PC != NULL)
							{							
								PC->eventKismet_ClientStopSound( LastAmbientSound, PC, 0.3f );

								if (NewAmbientSoundCue != NULL)
								{
									PC->eventKismet_ClientPlaySound( NewAmbientSoundCue, PC, 1.0f, 1.0f, 0.3f, TRUE, TRUE/*Suppress spatialization for soundscape*/ );
								}													

								break;
							}								
						}

						LastAmbientSound = NewAmbientSoundCue;
					}

					if (ReverbVolume)
					{
						LocationName = ReverbVolume->VolumeName;
					}
					else
					{
						LocationName = TEXT("<nowhere>");
					}
				}

				if (!bDisableWorldRendering)
				{
					INT UnscaledViewX = 0;
					INT UnscaledViewY = 0;
					UINT UnscaledViewSizeX = 0;
					UINT UnscaledViewSizeY = 0;

					// Unscale the view coordinates if needed
					GSystemSettings->UnScaleScreenCoords(
						UnscaledViewX, UnscaledViewY, 
						UnscaledViewSizeX, UnscaledViewSizeY, 
						View->X, View->Y, 
						View->SizeX, View->SizeY);

					// Set the canvas transform for the player's view rectangle.
					CanvasObject->Init();
					CanvasObject->SizeX = UnscaledViewSizeX;
					CanvasObject->SizeY = UnscaledViewSizeY;
					CanvasObject->SceneView = View;
					CanvasObject->Update();
					Canvas->PushAbsoluteTransform(FTranslationMatrix(FVector(UnscaledViewX,UnscaledViewY,0)));					

					// PreRender the player's view.

					APlayerController* PlayerController = Cast<APlayerController>( Player->Actor );
					PlayerController->OnPreRender();


					Player->Actor->eventPreRender(CanvasObject);

					Canvas->PopTransform();
				}

				// Add view information for resource streaming.
				GStreamingManager->AddViewInformation( View->ViewOrigin, View->SizeX, View->SizeX * View->ProjectionMatrix.M[0][0] );
			}
		}
	}

	// Update level streaming.
	GWorld->UpdateLevelStreaming( &ViewFamily );

	// Draw the player views.
	if (!bDisableWorldRendering)
	{
		for (INT ViewIndex = 0; ViewIndex < ViewFamily.Views.Num(); ++ViewIndex)
		{
			if (ViewFamily.Views(ViewIndex)->SeeThroughGroupMask != 0)
			{
				ViewFamily.ShowFlags |= SHOW_SeeThrough;
			}
		}
		BeginRenderingViewFamily(Canvas,&ViewFamily);
	}

	// Remove temporary debug lines.
	if (GWorld->LineBatcher != NULL && GWorld->LineBatcher->BatchedLines.Num())
	{
		GWorld->LineBatcher->BatchedLines.Empty();
		GWorld->LineBatcher->BeginDeferredReattach();
	}

	// End dynamic shadow stats gathering now that snapshot is collected.
	if( GWorld->bGatherDynamicShadowStatsSnapshot )
	{
		GWorld->bGatherDynamicShadowStatsSnapshot = FALSE;
		// Update the dynamic shadow stats browser.
		if( GCallbackEvent )
		{
			GCallbackEvent->Send( CALLBACK_RefreshEditor_DynamicShadowStatsBrowser );
		}
		// Dump stats to the log outside the editor.
		if( !GIsEditor )
		{
			for( INT RowIndex=0; RowIndex<GWorld->DynamicShadowStats.Num(); RowIndex++ )
			{
				const FDynamicShadowStatsRow& Row = GWorld->DynamicShadowStats(RowIndex);
				FString RowStringified;
				for( INT ColumnIndex=0; ColumnIndex<Row.Columns.Num(); ColumnIndex++ )
				{
					RowStringified += FString::Printf(TEXT("%s,"),*Row.Columns(ColumnIndex));
				}
				debugf(TEXT("%s"),*RowStringified);
			}
		}
	}

	GPlayerSceneView = NULL;

	//<@ ava specific ; 2006. 9. 12 changmin
	if( !GIsMinimapshot )
	//>@ ava
	{
		SCOPE_CYCLE_COUNTER(STAT_UIDrawingTime);

		for(INT PlayerIndex = 0;PlayerIndex < GEngine->GamePlayers.Num();PlayerIndex++)
		{
			ULocalPlayer* Player = GEngine->GamePlayers(PlayerIndex);
			if(Player->Actor)
			{
				FSceneView* View = PlayerViewMap.FindRef(PlayerIndex);
				if (View != NULL)
				{
					GPlayerSceneView = View;

					// Set the canvas transform for the player's view rectangle.
					CanvasObject->Init();
					CanvasObject->SizeX = appTrunc(View->SizeX);
					CanvasObject->SizeY = appTrunc(View->SizeY);
					CanvasObject->SceneView = View;
					CanvasObject->Update();
					Canvas->PushAbsoluteTransform(FTranslationMatrix(FVector(View->X,View->Y,0)));

					// Render the player's HUD.
					if( Player->Actor->myHUD )
					{
						SCOPE_CYCLE_COUNTER(STAT_HudTime);				

						Player->Actor->myHUD->Canvas = CanvasObject;
						Player->Actor->myHUD->eventPostRender();
						Player->Actor->myHUD->Canvas = NULL;
					}

					Canvas->PopTransform();

					// draw subtitles
					if (PlayerIndex == 0)
					{
						FVector2D MinPos(0.f, 0.f);
						FVector2D MaxPos(1.f, 1.f);
						eventGetSubtitleRegion(MinPos, MaxPos);

						UINT SizeX = Canvas->GetRenderTarget()->GetSizeX();
						UINT SizeY = Canvas->GetRenderTarget()->GetSizeY();
						FIntRect SubtitleRegion(appTrunc(SizeX * MinPos.X), appTrunc(SizeY * MinPos.Y), appTrunc(SizeX * MaxPos.X), appTrunc(SizeY * MaxPos.Y));
						FSubtitleManager::GetSubtitleManager()->DisplaySubtitles( Canvas, SubtitleRegion );
					}
				}
			}
		}

		// Reset the canvas for rendering to the full viewport.
		CanvasObject->Init();
		CanvasObject->SizeX = Viewport->GetSizeX();
		CanvasObject->SizeY = Viewport->GetSizeY();
		CanvasObject->SceneView = NULL;
		CanvasObject->Update();

		// render the global UIScenes
		UIController->RenderUI(Canvas);

		// Allow the viewport to render additional stuff
		eventPostRender(CanvasObject);
	}

	extern void appRatingInfo_Render( FCanvas* Canvas );
	appRatingInfo_Render( Canvas );

#if STATS
	DWORD DrawStatsBeginTime = appCycles();
#endif

	//@todo joeg: Move this stuff to a function, make safe to use on consoles by
	// respecting the various safe zones, and make it compile out.
#if CONSOLE
		const INT FPSXOffset	= 250;
		const INT StatsXOffset	= 100;
#else
		const INT FPSXOffset	= 70;
		const INT StatsXOffset	= 4;
#endif

#if !FINAL_RELEASE
	UBOOL GIsRedduckInternal = FALSE;
	
	if (GIsRedduckInternal || GIsEditor)
	{	
		if( GIsTextureMemoryCorrupted )
		{
			DrawShadowedString(Canvas,
				100,
				200,
				TEXT("RAN OUT OF TEXTURE MEMORY, EXPECT CORRUPTION AND GPU HANGS!"),
				GEngine->MediumFont,
				FColor(255,0,0)
				);
		}

		if( GWorld->GetWorldInfo()->bMapNeedsLightingFullyRebuilt )
		{
			DrawShadowedString(Canvas,
				10,
				100,
				TEXT("LIGHTING NEEDS TO BE REBUILT"),
				GEngine->SmallFont,
				FColor(128,128,128)
				);
		}

		//<@ ava speicific ; 2007. 1. 12 changmin
		if( GWorld->PersistentLevel && GWorld->PersistentLevel->StructuralModel->LeafBytes == 0 )
		{
			DrawShadowedString(Canvas,
				10,
				120,
				TEXT("WORLD HAS NO PVS INFORMATION"),
				GEngine->SmallFont,
				FColor(128,128,128)
				);
		}
		if( GWorld->GetWorldInfo()->bMapNeedsGeometryFullyRebuilt )
		{
			DrawShadowedString(Canvas,
				10,
				140,
				TEXT("WORLD NEEDS TO BE FULL GEOMETRY REBUILD"),
				GEngine->SmallFont,
				FColor(128,128,128)
				);
		}
		//>@ ava


		if (GWorld->bIsLevelStreamingFrozen)
		{
			DrawShadowedString(Canvas,
				10,
				160,
				TEXT("Level streaming frozen..."),
				GEngine->SmallFont,
				FColor(128,128,128)
				);
		}
		
		if( GUpdatingCullDistances )
		{
			DrawShadowedString(Canvas,
				10,
				220,
				*FString::Printf( TEXT("Updating CullDistances... %d / %d"), GCullDistanceUpdateCount, GCullDistanceUpdateCount2 ),
				GEngine->MediumFont,
				FColor(255,0,0)
				);
		}
	}
#endif

#ifdef _BT_TEST_BY_CRAZY
	if ( GShowBandwidthStat && GWorld->NetDriver )
	{
		INT Y = 80;
		UNetConnection* Connection = GWorld->NetDriver->ServerConnection;
		if (Connection)
		{
			FString Str = FString::Printf(TEXT("NetSpeed = %d, QueuedBytes = %d"), Connection->NewNetSpeed, Connection->QueuedBytes);

			DrawShadowedString(Canvas,
				20,
				Y,
				*Str,
				GEngine->SmallFont,
				FColor(255,255,255)
				);
			Y += 20;
		}
		for( INT i=GWorld->NetDriver->ClientConnections.Num()-1; i>=0; i-- )
		{
			UNetConnection* Connection = GWorld->NetDriver->ClientConnections(i);
			check(Connection);

			FString Str = FString::Printf(TEXT("NetSpeed = %d, QueuedBytes = %d"), Connection->NewNetSpeed, Connection->QueuedBytes);

			DrawShadowedString(Canvas,
				20,
				Y,
				*Str,
				GEngine->SmallFont,
				FColor(255,255,255)
				);

			Y += 20;
		}

		if (UNetConnection::bIgnoreNetReadyCheck)
		{
			DrawShadowedString(Canvas,
				20,
				Y,
				TEXT("NetReady check is ignored."),
				GEngine->SmallFont,
				FColor(255,255,255)
				);
		}
	}
#endif

	{
		INT X = Viewport->GetSizeX() - FPSXOffset;
		INT	Y = 20;

		// Render the FPS counter.
		Y = DrawFPSCounter( Viewport, Canvas, X, Y );
		// Render CPU thread and GPU frame times.
		Y = DrawUnitTimes( Viewport, Canvas, X, Y );

		// Reset Y as above stats are rendered on right hand side.
		Y = 20;

		// Level stats.
		Y = DrawLevelStats( Viewport, Canvas, StatsXOffset, Y );

#if STATS
		GStatChart->AddDataPoint( FString(TEXT("Frame time")), GFPSCounter.GetFrameTime() * 1000.0f );

		// Render the Stats System UI
		GStatManager.Render( Canvas, StatsXOffset, Y );
#endif

#if !FINAL_RELEASE
		for (INT i=0; i<ViewFamily.Views.Num(); ++i)
		{
			const FSceneView* View = ViewFamily.Views(i);
			FSceneViewState* ViewState = (FSceneViewState*)(View->State);	

			if (ViewState)
			{		
				ViewState->ExposureData.DisplayDebugInfo(Canvas, Y);	
			}
		}
#endif
	}

	// Render the stat chart.
	if(GStatChart)
	{
		GStatChart->Render(Viewport, Canvas);
	}

#if PLATFORM_SUPPORTS_SCREENSHOT_FROM_SCREEN
#else
	TakeScreenshot( Viewport );
#endif

	if (!bCanApplyCullDistances)
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND(
			FResetCullDistances,		
		{
			GDoNotApplyCullDistances = FALSE;
		});
	}

#if PS3
	extern void PrintFrameRatePS3();
	PrintFrameRatePS3();
#endif

#if STATS
	DWORD DrawStatsEndTime = appCycles();

	FCycleStat* Stat = (FCycleStat*)GStatManager.GetCycleStat(STAT_DrawStats);
	if (Stat == NULL)
	{
		{
			SCOPE_CYCLE_COUNTER(STAT_DrawStats);
		}
		Stat = (FCycleStat*)GStatManager.GetCycleStat(STAT_DrawStats);
	}
	if( Stat )
	{
		Stat->Cycles = DrawStatsEndTime - DrawStatsBeginTime;
		Stat->NumCallsPerFrame = 1;
	}
#endif
}

void UGameViewportClient::Precache()
{
	if(!GIsEditor)
	{
		// Precache sounds...
		UAudioDevice* AudioDevice = GEngine->Client ? GEngine->Client->GetAudioDevice() : NULL;
		if( AudioDevice )
		{
			debugf(TEXT("Precaching sounds..."));
			for(TObjectIterator<USoundNodeWave> It;It;++It)
			{
				USoundNodeWave* SoundNodeWave = *It;
				AudioDevice->Precache( SoundNodeWave );
			}
			debugf(TEXT("Precaching sounds completed..."));
		}
	}

	// Log time till first precache is finished.
	static UBOOL bIsFirstCallOfFunction = TRUE;
	if( bIsFirstCallOfFunction )
	{
		debugf(TEXT("%5.2f seconds passed since startup."),appSeconds()-GStartTime);
		bIsFirstCallOfFunction = FALSE;
	}
}

void UGameViewportClient::LostFocus(FViewport* Viewport)
{
}

void UGameViewportClient::ReceivedFocus(FViewport* Viewport)
{
}

/*UBOOL UGameViewportClient::IsFocused(FViewport* Viewport)
{
	return Viewport->HasFocus() || Viewport->HasMouseCapture();
}*/

void UGameViewportClient::CloseRequested(FViewport* Viewport)
{
	check(Viewport == this->Viewport);
	if( GFullScreenMovie )
	{
		// force movie playback to stop
		GFullScreenMovie->GameThreadStopMovie(0,FALSE,TRUE);
	}
	GEngine->Client->CloseViewport(this->Viewport);
	SetViewport(NULL);
}


UBOOL UGameViewportClient::Exec(const TCHAR* Cmd,FOutputDevice& Ar)
{
	if( ParseCommand(&Cmd,TEXT("SHOW")) )
	{
		struct { const TCHAR* Name; EShowFlags Flag; }	Flags[] =
		{
			{ TEXT("BOUNDS"),				SHOW_Bounds					},
			{ TEXT("BSP"),					SHOW_BSP					},
			{ TEXT("BSPSPLIT"),				SHOW_BSPSplit				},
			{ TEXT("COLLISION"),			SHOW_Collision				},
			{ TEXT("CONSTRAINTS"),			SHOW_Constraints			},
			{ TEXT("COVER"),				SHOW_Cover					},
			{ TEXT("DECALINFO"),			SHOW_DecalInfo				},
			{ TEXT("DECALS"),				SHOW_Decals					},
			{ TEXT("DYNAMICSHADOWS"),		SHOW_DynamicShadows			},
			{ TEXT("FOG"),					SHOW_Fog					},
			{ TEXT("FOLIAGE"),				SHOW_Foliage				},
			{ TEXT("HITPROXIES"),			SHOW_HitProxies				},
			{ TEXT("LENSFLARES"),			SHOW_LensFlares				},
			{ TEXT("LEVELCOLORATION"),		SHOW_LevelColoration		},
			{ TEXT("MESHEDGES"),			SHOW_MeshEdges				},
			{ TEXT("MISSINGCOLLISION"),		SHOW_MissingCollisionModel	},
			{ TEXT("NAVNODES"),				SHOW_NavigationNodes		},
			{ TEXT("PARTICLES"),			SHOW_Particles				},
			{ TEXT("PATHS"),				SHOW_Paths					},
			{ TEXT("PORTALS"),				SHOW_Portals				},
			{ TEXT("POSTPROCESS"),			SHOW_PostProcess			},
			{ TEXT("SHADOWFRUSTUMS"),		SHOW_ShadowFrustums			},
			{ TEXT("SKELMESHES"),			SHOW_SkeletalMeshes			},
			{ TEXT("SPRITES"),				SHOW_Sprites				},
			{ TEXT("STATICMESHES"),			SHOW_StaticMeshes			},
			{ TEXT("TERRAIN"),				SHOW_Terrain				},
			{ TEXT("TERRAINPATCHES"),		SHOW_TerrainPatches			},
			{ TEXT("UNLITTRANSLUCENCY"),	SHOW_UnlitTranslucency		},
			{ TEXT("ZEROEXTENT"),			SHOW_CollisionZeroExtent	},
			{ TEXT("NONZEROEXTENT"),		SHOW_CollisionNonZeroExtent	},
			{ TEXT("RIGIDBODY"),			SHOW_CollisionRigidBody		},
			{ TEXT("TERRAINCOLLISION"),		SHOW_TerrainCollision		},
			//<@ ava specific ; 2006. 11. 16 changmin
			{ TEXT("LeafColoration"),		SHOW_LeafColoration			},
			//>@ ava
		};

		// Search for a specific show flag and toggle it if found.
		for(UINT FlagIndex = 0;FlagIndex < ARRAY_COUNT(Flags);FlagIndex++)
		{
			if(ParseCommand(&Cmd,Flags[FlagIndex].Name))
			{
				ShowFlags ^= Flags[FlagIndex].Flag;
				// special case: for the SHOW_Collision flag, we need to un-hide any primitive components that collide so their collision geometry gets rendered
				if (Flags[FlagIndex].Flag == SHOW_Collision ||
					Flags[FlagIndex].Flag == SHOW_CollisionNonZeroExtent || 
					Flags[FlagIndex].Flag == SHOW_CollisionZeroExtent || 
					Flags[FlagIndex].Flag == SHOW_CollisionRigidBody )
				{
					// Ensure that all flags, other than the one we just typed in, is off.
					ShowFlags &= ~(Flags[FlagIndex].Flag ^ (SHOW_Collision_Any | SHOW_Collision));

					for (TObjectIterator<UPrimitiveComponent> It; It; ++It)
					{
						if (It->HiddenGame && It->ShouldCollide())
						{
							It->SetHiddenGame(false);
						}
					}
				}
				else
				if (Flags[FlagIndex].Flag == SHOW_Paths)
				{
					UBOOL bShowPaths = (ShowFlags & SHOW_Paths) != 0;
					// make sure all nav points have path rendering components
					for (FActorIterator It; It; ++It)
					{
						ACoverLink *Link = Cast<ACoverLink>(*It);
						if (Link != NULL)
						{
							UBOOL bHasComponent = FALSE;
							for (INT Idx = 0; Idx < Link->Components.Num(); Idx++)
							{
								UCoverMeshComponent *PathRenderer = Cast<UCoverMeshComponent>(Link->Components(Idx));
								if (PathRenderer != NULL)
								{
									PathRenderer->SetHiddenGame(!bShowPaths);
									bHasComponent = TRUE;
									break;
								}
							}
							if (!bHasComponent)
							{
								UCoverMeshComponent *PathRenderer = ConstructObject<UCoverMeshComponent>(UCoverMeshComponent::StaticClass(),Link);
								PathRenderer->SetHiddenGame(!bShowPaths);
								Link->AttachComponent(PathRenderer);
							}
						}
						else
						{
							ANavigationPoint *Nav = Cast<ANavigationPoint>(*It);
							if (Nav != NULL)
							{
								UBOOL bHasComponent = FALSE;
								for (INT Idx = 0; Idx < Nav->Components.Num(); Idx++)
								{
									UPathRenderingComponent *PathRenderer = Cast<UPathRenderingComponent>(Nav->Components(Idx));
									if (PathRenderer != NULL)
									{
										bHasComponent = TRUE;
										PathRenderer->SetHiddenGame(!bShowPaths);
										break;
									}
								}
								if (!bHasComponent)
								{
									UPathRenderingComponent *PathRenderer = ConstructObject<UPathRenderingComponent>(UPathRenderingComponent::StaticClass(),Nav);
									PathRenderer->SetHiddenGame(!bShowPaths);
									Nav->AttachComponent(PathRenderer);
								}
							}
						}
					}
				}
				else
				if (Flags[FlagIndex].Flag == SHOW_TerrainCollision)
				{
					ATerrain::ShowCollisionCallback((ShowFlags & SHOW_TerrainCollision) != 0);
				}
				return TRUE;
			}
		}

		// The specified flag wasn't found -- list all flags and their current value.
		for(UINT FlagIndex = 0;FlagIndex < ARRAY_COUNT(Flags);FlagIndex++)
		{
			Ar.Logf(TEXT("%s : %s"),
				Flags[FlagIndex].Name,
				(ShowFlags & Flags[FlagIndex].Flag) ? TEXT("TRUE") :TEXT("FALSE"));
		}
		return TRUE;
	}
	else if (ParseCommand(&Cmd,TEXT("VIEWMODE")))
	{
#ifndef _DEBUG
		// If there isn't a cheat manager, exit out
		UBOOL bCheatsEnabled = FALSE;
		for (FPlayerIterator It((UEngine*)GetOuter()); It; ++It)
		{
			if (It->Actor != NULL && It->Actor->CheatManager != NULL)
			{
				bCheatsEnabled = TRUE;
				break;
			}
		}
		if (!bCheatsEnabled)
		{
			return TRUE;
		}
#endif

		if( ParseCommand(&Cmd,TEXT("WIREFRAME")) )
		{
			ShowFlags &= ~SHOW_ViewMode_Mask;
			ShowFlags |= SHOW_ViewMode_Wireframe;
		}
		else if( ParseCommand(&Cmd,TEXT("BRUSHWIREFRAME")) )
		{
			ShowFlags &= ~SHOW_ViewMode_Mask;
			ShowFlags |= SHOW_ViewMode_BrushWireframe;
		}
		else if( ParseCommand(&Cmd,TEXT("UNLIT")) )
		{
			ShowFlags &= ~SHOW_ViewMode_Mask;
			ShowFlags |= SHOW_ViewMode_Unlit;
		}
		else if( ParseCommand(&Cmd,TEXT("LIGHTINGONLY")) )
		{
			ShowFlags &= ~SHOW_ViewMode_Mask;
			ShowFlags |= SHOW_ViewMode_LightingOnly;
		}			
		else if( ParseCommand(&Cmd,TEXT("LIGHTCOMPLEXITY")) )
		{
			ShowFlags &= ~SHOW_ViewMode_Mask;
			ShowFlags |= SHOW_ViewMode_LightComplexity;
		}
		else if( ParseCommand(&Cmd,TEXT("TEXTUREDENSITY")))
		{
			ShowFlags &= ~SHOW_ViewMode_Mask;
			ShowFlags |= SHOW_ViewMode_TextureDensity;
		}
		//<@ ava specific ; 2007. 1. 15 changmin
		else if( ParseCommand(&Cmd,TEXT("ViewSpaceNormal")))
		{
			ShowFlags &= ~SHOW_ViewMode_Mask;
			ShowFlags |= SHOW_ViewMode_ViewSpaceNormal;
		}
		//>@ ava
		//<@ ava specific ; 2007. 10 .16 changmin
		else if( ParseCommand(&Cmd, TEXT("BumpLighting")))
		{
			ShowFlags &= ~SHOW_ViewMode_Mask;
			ShowFlags |= SHOW_ViewMode_BumpLighting;
			extern INT GBumpState;
			GBumpState = 0;
			if( ParseCommand(&Cmd, TEXT("Diffuse")) )
			{
				GBumpState = 1;
			}
			else if( ParseCommand(&Cmd, TEXT("Specular")) )
			{
				GBumpState = 2;
			}
			else if( ParseCommand(&Cmd, TEXT("Ambient")) )
			{
				GBumpState = 3;
			}
		}
		//>@ ava
		else
		{
			ShowFlags &= ~SHOW_ViewMode_Mask;
			ShowFlags |= SHOW_ViewMode_Lit;
		}

		return TRUE;
	}
	else if (ParseCommand(&Cmd, TEXT("NEXTVIEWMODE")))
	{
#ifndef _DEBUG
		// If there isn't a cheat manager, exit out
		UBOOL bCheatsEnabled = FALSE;
		for (FPlayerIterator It((UEngine*)GetOuter()); It; ++It)
		{
			if (It->Actor != NULL && It->Actor->CheatManager != NULL)
			{
				bCheatsEnabled = TRUE;
				break;
			}
		}
		if (!bCheatsEnabled)
		{
			return TRUE;
		}
#endif

		QWORD OldShowFlags = ShowFlags;
		ShowFlags &= ~SHOW_ViewMode_Mask;
		switch (OldShowFlags & SHOW_ViewMode_Mask)
		{
			case SHOW_ViewMode_Lit:
				ShowFlags |= SHOW_ViewMode_LightingOnly;
				break;
			case SHOW_ViewMode_LightingOnly:
				ShowFlags |= SHOW_ViewMode_LightComplexity;
				break;
			case SHOW_ViewMode_LightComplexity:
				ShowFlags |= SHOW_ViewMode_Wireframe;
				break;
			case SHOW_ViewMode_Wireframe:
				ShowFlags |= SHOW_ViewMode_BrushWireframe;
				break;
			case SHOW_ViewMode_BrushWireframe:
				ShowFlags |= SHOW_ViewMode_Unlit;
				break;
			case SHOW_ViewMode_Unlit:
				ShowFlags |= SHOW_ViewMode_TextureDensity;
				break;
			case SHOW_ViewMode_TextureDensity:
				ShowFlags |= SHOW_ViewMode_Lit;
				break;
			default:
				ShowFlags |= SHOW_ViewMode_Lit;
				break;
		}

		return TRUE;
	}
	else if (ParseCommand(&Cmd, TEXT("PREVVIEWMODE")))
	{
#ifndef _DEBUG
		// If there isn't a cheat manager, exit out
		UBOOL bCheatsEnabled = FALSE;
		for (FPlayerIterator It((UEngine*)GetOuter()); It; ++It)
		{
			if (It->Actor != NULL && It->Actor->CheatManager != NULL)
			{
				bCheatsEnabled = TRUE;
				break;
			}
		}
		if (!bCheatsEnabled)
		{
			return TRUE;
		}
#endif

		QWORD OldShowFlags = ShowFlags;
		ShowFlags &= ~SHOW_ViewMode_Mask;
		switch (OldShowFlags & SHOW_ViewMode_Mask)
		{
			case SHOW_ViewMode_Lit:
				ShowFlags |= SHOW_ViewMode_TextureDensity;
				break;
			case SHOW_ViewMode_LightingOnly:
				ShowFlags |= SHOW_ViewMode_Lit;
				break;
			case SHOW_ViewMode_LightComplexity:
				ShowFlags |= SHOW_ViewMode_LightingOnly;
				break;
			case SHOW_ViewMode_Wireframe:
				ShowFlags |= SHOW_ViewMode_LightComplexity;
				break;
			case SHOW_ViewMode_BrushWireframe:
				ShowFlags |= SHOW_ViewMode_Wireframe;
				break;
			case SHOW_ViewMode_Unlit:
				ShowFlags |= SHOW_ViewMode_BrushWireframe;
				break;
			case SHOW_ViewMode_TextureDensity:
				ShowFlags |= SHOW_ViewMode_Unlit;
				break;
			default:
				ShowFlags |= SHOW_ViewMode_Lit;
				break;
		}

		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("STAT")) )
	{
		const TCHAR* Temp = Cmd;
		// Check for FPS counter
		if (ParseCommand(&Temp,TEXT("FPS")))
		{
			GShowFpsCounter ^= TRUE;
			return TRUE;
		}
		// Check for Level stats.
		else if (ParseCommand(&Temp,TEXT("LEVELS")))
		{
			GShowLevelStats ^= TRUE;
			return TRUE;
		}
		// Check for idle times.
		else if (ParseCommand(&Temp,TEXT("UNIT")))
		{
			GShowUnitTimes ^= TRUE;
			return TRUE;
		}
#ifdef _BT_TEST_BY_CRAZY
		else if (ParseCommand(&Temp,TEXT("BANDWIDTH")))
		{
			GShowBandwidthStat ^= TRUE;
			return TRUE;
		}
#endif
#if STATS
		// Forward the call to the stat manager
		else
		{
			return GStatManager.Exec(Cmd,Ar);
		}
#else
		return FALSE;
#endif
	}
	else
	if( ParseCommand(&Cmd,TEXT("DUMPDYNAMICSHADOWSTATS")) )
	{
		GWorld->bGatherDynamicShadowStatsSnapshot = TRUE;
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("PRECACHE")) )
	{
		Precache();
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("SETRES")) )
	{
		if(Viewport)
		{
			/* 태식아... -_- 요렇게만 하는 게 낫다.. */
			INT X=appAtoi(Cmd);
			const TCHAR* CmdTemp = appStrchr(Cmd,'x') ? appStrchr(Cmd,'x')+1 : appStrchr(Cmd,'X') ? appStrchr(Cmd,'X')+1 : 
				appStrchr(Cmd,'*') ? appStrchr(Cmd,'*')+1 : TEXT("");
			INT Y=appAtoi(CmdTemp);
			Cmd = CmdTemp;			

			UBOOL	Fullscreen = Viewport->IsFullscreen();
			if(appStrchr(Cmd,'w') || appStrchr(Cmd,'W'))
				Fullscreen = 0;
			else if(appStrchr(Cmd,'f') || appStrchr(Cmd,'F'))
				Fullscreen = 1;
			if( X && Y )
				ViewportFrame->Resize(X,Y,Fullscreen);
		}
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("TILEDSHOT")) )
	{
		GIsTiledScreenshot = TRUE;
		GScreenshotResolutionMultiplier = appAtoi(Cmd);
		GScreenshotResolutionMultiplier = Clamp<INT>( GScreenshotResolutionMultiplier, 2, 128 );
		const TCHAR* CmdTemp = appStrchr(Cmd, ' ');
		GScreenshotMargin = CmdTemp ? Clamp<INT>(appAtoi(CmdTemp), 0, 320) : 64;
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("SHOT")) || ParseCommand(&Cmd,TEXT("SCREENSHOT")) )
	{
		if(Viewport)
		{
			GScreenShotRequest=TRUE;
		}
		return TRUE;
	}
	//<@ ava specific ; 2006. 9. 12 changmin
	else if( ParseCommand(&Cmd,TEXT("MINIMAPSHOT")) )
	{
		GIsMinimapshot = TRUE;
		return TRUE;
	}
	//>@ava
	//<@ AVA 2007. 4. 12
	else if( ParseCommand(&Cmd,TEXT("DUMPHDRSCENE")) )
	{
		GDumpHdrScene = TRUE;
		return TRUE;
	}
	//>@ AVA
	else if( ParseCommand(&Cmd,TEXT("KILLPARTICLES")) )
	{
		// Don't kill in the Editor to avoid potential content clobbering.
		if( !GIsEditor )
		{
			extern UBOOL GIsAllowingParticles;
			// Deactivate system and kill existing particles.
			for( TObjectIterator<UParticleSystemComponent> It; It; ++It )
			{
				UParticleSystemComponent* ParticleSystemComponent = *It;
				ParticleSystemComponent->DeactivateSystem();
				ParticleSystemComponent->KillParticlesForced();
			}
			// No longer initialize particles from here on out.
			GIsAllowingParticles = FALSE;
		}
		return TRUE;
	}
	else if( ParseCommand( &Cmd, TEXT( "CaptureStart" ) ) )
	{
		if( GEngine && GEngine->Client )
		{
			UAudioDevice* AudioDevice = GEngine->Client->GetAudioDevice();	
			if( AudioDevice )
			{
				AudioDevice->CaptureStart();
			}
		}

		return TRUE;
	}
	else if( ParseCommand( &Cmd, TEXT( "CaptureStop" ) ) )
	{
		if( GEngine && GEngine->Client )
		{
			UAudioDevice* AudioDevice = GEngine->Client->GetAudioDevice();	
			if( AudioDevice )
			{
				INT SamplesCaptured = AudioDevice->CaptureStop();
			}
		}

		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("toggleocclusion")) )
	{
		extern UBOOL GIgnoreAllOcclusionQueries;
		GIgnoreAllOcclusionQueries = !GIgnoreAllOcclusionQueries;
		return TRUE;
	}	
	else if ( UIController->Exec(Cmd,Ar) )
	{
		return TRUE;
	}
	else if(ScriptConsoleExec(Cmd,Ar,NULL))
	{
		return TRUE;
	}
	else if( GEngine->Exec(Cmd,Ar) )
	{
		return TRUE;
	}
	//<@ ava specific ; 2006 / 9 / 5 changmin
	else if( ParseCommand(&Cmd, TEXT("DRAWEMISSIVEDYNAMICPRIMITIVES")))
	{
		if( ParseCommand(&Cmd, TEXT("TRUE")) )
		{
			GWorld->bDrawEmissiveDynamicPrimitives = TRUE;
		}
		else if( ParseCommand(&Cmd, TEXT("FALSE")) )
		{
			GWorld->bDrawEmissiveDynamicPrimitives = FALSE;
		}
		return TRUE;
	}
	//>@ ava
	//<@ ava specific ; 2007. 11. 6 changmin
	// add cascaded shadow map
	else if( ParseCommand(&Cmd, TEXT("SplitViewFrustum")))
	{
		extern FLOAT GSliceValues[8];
		GSliceValues[0] = appAtoi(Cmd);
		TCHAR* CmdTemp = appStrchr(Cmd, ' ');
		INT Index = 1;
		while(CmdTemp && Index < 8)
		{
			if( CmdTemp )
			{
				GSliceValues[Index++] = appAtoi(CmdTemp);
				Cmd = CmdTemp + 1;
				CmdTemp = appStrchr(Cmd, ' ');
			}
			else
			{
				break;
			}
		}
		return TRUE;
	}
	else if ( ParseCommand(&Cmd, TEXT("CascadedDepthBias")))
	{
		extern FLOAT GCascadedDepthBias;
		GCascadedDepthBias = appAtof(Cmd);
		return TRUE;
	}
	else if( ParseCommand(&Cmd, TEXT("NumCascadedShadow")) )
	{
		extern INT GNumCascadedShadow;
		GNumCascadedShadow = Clamp<INT>(appAtoi(Cmd), 1, 8);
		return TRUE;
	}
	//>@ ava
	else
	{		
		return FALSE;
	}
}

ULocalPlayer::ULocalPlayer()
{
	if ( !IsTemplate() )
	{
		ViewState = AllocateViewState();

		if( !PlayerPostProcess )
		{
			// initialize to global post process if one is not set
			PlayerPostProcess = GEngine->DefaultPostProcess;
		}

		// Initialize the actor visibility history.
		ActorVisibilityHistory.Init();
	}
}

UBOOL ULocalPlayer::GetActorVisibility(AActor* TestActor)
{
	return ActorVisibilityHistory.GetActorVisibility(TestActor);
}

UBOOL ULocalPlayer::SpawnPlayActor(const FString& URL,FString& OutError)
{
	if(GWorld->IsServer())
	{
		Actor = GWorld->SpawnPlayActor(this,ROLE_SimulatedProxy,FURL(NULL,*URL,TRAVEL_Absolute),OutError);
	}
	else
	{
		// The PlayerController gets replicated from the client though the engine assumes that every Player always has
		// a valid PlayerController so we spawn a dummy one that is going to be replaced later.
		Actor = CastChecked<APlayerController>(GWorld->SpawnActor(APlayerController::StaticClass()));
	}
	return Actor != NULL;
}

void ULocalPlayer::SendSplitJoin()
{
	if (GWorld == NULL || GWorld->NetDriver == NULL || GWorld->NetDriver->ServerConnection == NULL || GWorld->NetDriver->ServerConnection->State != USOCK_Open)
	{
		debugf(NAME_Warning, TEXT("SendSplitJoin(): Not connected to a server"));
	}
	else if (!bSentSplitJoin)
	{
		// make sure we don't already have a connection
		UBOOL bNeedToSendJoin = FALSE;
		if (Actor == NULL)
		{
			bNeedToSendJoin = TRUE;
		}
		else if (GWorld->NetDriver->ServerConnection->Actor != Actor)
		{
			bNeedToSendJoin = TRUE;
			for (INT i = 0; i < GWorld->NetDriver->ServerConnection->Children.Num(); i++)
			{
				if (GWorld->NetDriver->ServerConnection->Children(i)->Actor == Actor)
				{
					bNeedToSendJoin = FALSE;
					break;
				}
			}
		}

		if (bNeedToSendJoin)
		{
			//@todo: send a seperate URL?
			GWorld->NetDriver->ServerConnection->Logf(TEXT("JOINSPLIT"));
			bSentSplitJoin = TRUE;
		}
	}
}

void ULocalPlayer::FinishDestroy()
{
	if ( !IsTemplate() )
	{
		ViewState->Destroy();
		ViewState = NULL;
	}
	Super::FinishDestroy();
}

/**
 * Helper structure for sorting textures by relative cost.
 */
struct FSortedTexture 
{
 	INT		OrigSizeX;
 	INT		OrigSizeY;
	INT		CurSizeX;
	INT		CurSizeY;
 	INT		LODBias;
	INT		MaxSize;
 	INT		CurrentSize;
	FString Name;
 	INT		LODGroup;
 	UBOOL	bIsStreaming;

	/** Constructor, initializing every member variable with passed in values. */
	FSortedTexture(	INT InOrigSizeX, INT InOrigSizeY, INT InCurSizeX, INT InCurSizeY, INT InLODBias, INT InMaxSize, INT InCurrentSize, const FString& InName, INT InLODGroup, UBOOL bInIsStreaming )
	:	OrigSizeX( InOrigSizeX )
	,	OrigSizeY( InOrigSizeY )
	,	CurSizeX( InCurSizeX )
	,	CurSizeY( InCurSizeY )
 	,	LODBias( InLODBias )
	,	MaxSize( InMaxSize )
	,	CurrentSize( InCurrentSize )
	,	Name( InName )
 	,	LODGroup( InLODGroup )
 	,	bIsStreaming( bInIsStreaming )
	{}
};
IMPLEMENT_COMPARE_CONSTREF( FSortedTexture, UnPlayer, { return B.MaxSize - A.MaxSize; } )

/** Helper struct for sorting anim sets by size */
struct FSortedAnimSet
{
	FString Name;
	INT		Size;

	FSortedAnimSet( const FString& InName, INT InSize )
	:	Name(InName)
	,	Size(InSize)
	{}
};
IMPLEMENT_COMPARE_CONSTREF( FSortedAnimSet, UnPlayer, { return B.Size - A.Size; } )



//
void InterpolatePostProcessSettings( FCurrentPostProcessVolumeInfo& CurrentPPInfo, 							   
							   APostProcessVolume* NewVolume, 
							   const FPostProcessSettings& NewSettings,
							   FLOAT PP_DesaturationMultiplier,
								FLOAT PP_HighlightsMultiplier,
								FLOAT PP_MidTonesMultiplier,
								FLOAT PP_ShadowsMultiplier )
{	
	FLOAT CurrentWorldTime = GWorld->GetRealTimeSeconds();

	// Update info for when a new volume goes into use
	if( CurrentPPInfo.LastVolumeUsed != NewVolume )
	{
		CurrentPPInfo.LastVolumeUsed = NewVolume;
		CurrentPPInfo.BlendStartTime = CurrentWorldTime;
	}

	// Calculate the blend factors.
	const FLOAT DeltaTime = Max(CurrentWorldTime - CurrentPPInfo.LastBlendTime,0.f);
	const FLOAT ElapsedBlendTime = Max(CurrentPPInfo.LastBlendTime - CurrentPPInfo.BlendStartTime,0.f);

	// Calculate the blended settings.
	FPostProcessSettings BlendedSettings;
	const FPostProcessSettings& CurrentSettings = CurrentPPInfo.LastSettings;

	// toggles
	BlendedSettings.bEnableBloom = NewSettings.bEnableBloom;
	BlendedSettings.bEnableDOF = NewSettings.bEnableDOF;
	BlendedSettings.bEnableMotionBlur = NewSettings.bEnableMotionBlur;
	BlendedSettings.bEnableSceneEffect = NewSettings.bEnableSceneEffect;
	BlendedSettings.bEnableTonemap = NewSettings.bEnableTonemap;

	// calc bloom lerp amount
	FLOAT BloomFade = 1.f;
	const FLOAT RemainingBloomBlendTime = Max(NewSettings.Bloom_InterpolationDuration - ElapsedBlendTime,0.f);
	if(RemainingBloomBlendTime > DeltaTime)
	{
		BloomFade = Clamp<FLOAT>(DeltaTime / RemainingBloomBlendTime,0.f,1.f);
	}
	// bloom values
	BlendedSettings.Bloom_Scale = Lerp<FLOAT>(CurrentSettings.Bloom_Scale,NewSettings.Bloom_Scale,BloomFade);
	BlendedSettings.Bloom_Threshold = Lerp<FLOAT>(CurrentSettings.Bloom_Threshold,NewSettings.Bloom_Threshold,BloomFade);

	// calc dof lerp amount
	FLOAT DOFFade = 1.f;
	const FLOAT RemainingDOFBlendTime = Max(NewSettings.DOF_InterpolationDuration - ElapsedBlendTime,0.f);
	if(RemainingDOFBlendTime > DeltaTime)
	{
		DOFFade = Clamp<FLOAT>(DeltaTime / RemainingDOFBlendTime,0.f,1.f);
	}
	// dof values		
	BlendedSettings.DOF_FalloffExponent = Lerp<FLOAT>(CurrentSettings.DOF_FalloffExponent,NewSettings.DOF_FalloffExponent,DOFFade);
	BlendedSettings.DOF_BlurKernelSize = Lerp<FLOAT>(CurrentSettings.DOF_BlurKernelSize,NewSettings.DOF_BlurKernelSize,DOFFade);
	BlendedSettings.DOF_MaxNearBlurAmount = Lerp<FLOAT>(CurrentSettings.DOF_MaxNearBlurAmount,NewSettings.DOF_MaxNearBlurAmount,DOFFade);
	BlendedSettings.DOF_MaxFarBlurAmount = Lerp<FLOAT>(CurrentSettings.DOF_MaxFarBlurAmount,NewSettings.DOF_MaxFarBlurAmount,DOFFade);
	BlendedSettings.DOF_ModulateBlurColor = FColor(Lerp<FLinearColor>(CurrentSettings.DOF_ModulateBlurColor,NewSettings.DOF_ModulateBlurColor,DOFFade));
	BlendedSettings.DOF_FocusType = NewSettings.DOF_FocusType;
	BlendedSettings.DOF_FocusInnerRadius = Lerp<FLOAT>(CurrentSettings.DOF_FocusInnerRadius,NewSettings.DOF_FocusInnerRadius,DOFFade);
	BlendedSettings.DOF_FocusDistance = Lerp<FLOAT>(CurrentSettings.DOF_FocusDistance,NewSettings.DOF_FocusDistance,DOFFade);
	BlendedSettings.DOF_FocusPosition = Lerp<FVector>(CurrentSettings.DOF_FocusPosition,NewSettings.DOF_FocusPosition,DOFFade);

	// calc motion blur lerp amount
	FLOAT MotionBlurFade = 1.f;
	const FLOAT RemainingMotionBlurBlendTime = Max(NewSettings.MotionBlur_InterpolationDuration - ElapsedBlendTime,0.f);
	if(RemainingMotionBlurBlendTime > DeltaTime)
	{
		MotionBlurFade = Clamp<FLOAT>(DeltaTime / RemainingMotionBlurBlendTime,0.f,1.f);
	}
	// motion blur values
	BlendedSettings.MotionBlur_MaxVelocity = Lerp<FLOAT>(CurrentSettings.MotionBlur_MaxVelocity,NewSettings.MotionBlur_MaxVelocity,MotionBlurFade);
	BlendedSettings.MotionBlur_Amount = Lerp<FLOAT>(CurrentSettings.MotionBlur_Amount,NewSettings.MotionBlur_Amount,MotionBlurFade);
	BlendedSettings.MotionBlur_CameraRotationThreshold = Lerp<FLOAT>(CurrentSettings.MotionBlur_CameraRotationThreshold,NewSettings.MotionBlur_CameraRotationThreshold,MotionBlurFade);
	BlendedSettings.MotionBlur_CameraTranslationThreshold = Lerp<FLOAT>(CurrentSettings.MotionBlur_CameraTranslationThreshold,NewSettings.MotionBlur_CameraTranslationThreshold,MotionBlurFade);

	// calc scene material lerp amount
	FLOAT SceneMaterialFade = 1.f;
	const FLOAT RemainingSceneBlendTime = Max(NewSettings.Scene_InterpolationDuration - ElapsedBlendTime,0.f);
	if(RemainingSceneBlendTime > DeltaTime)
	{
		SceneMaterialFade = Clamp<FLOAT>(DeltaTime / RemainingSceneBlendTime,0.f,1.f);
	}
	// scene material values
	BlendedSettings.Scene_Desaturation	= Lerp<FLOAT>(CurrentSettings.Scene_Desaturation,NewSettings.Scene_Desaturation*PP_DesaturationMultiplier,SceneMaterialFade);
	BlendedSettings.Scene_HighLights	= Lerp<FVector>(CurrentSettings.Scene_HighLights,NewSettings.Scene_HighLights*PP_HighlightsMultiplier,SceneMaterialFade);
	BlendedSettings.Scene_MidTones		= Lerp<FVector>(CurrentSettings.Scene_MidTones,NewSettings.Scene_MidTones*PP_MidTonesMultiplier,SceneMaterialFade);
	BlendedSettings.Scene_Shadows		= Lerp<FVector>(CurrentSettings.Scene_Shadows,NewSettings.Scene_Shadows*PP_ShadowsMultiplier,SceneMaterialFade);

	// Clamp desaturation to 0..1 range to allow desaturation multipliers > 1 without color shifts at high desaturation.
	BlendedSettings.Scene_Desaturation = Clamp( BlendedSettings.Scene_Desaturation, 0.f, 1.f );

	// the scene material only needs to be enabled if the values don't match the default
	// as it should be setup to not have any affect in the default case
	if( BlendedSettings.bEnableSceneEffect )
	{
		if( BlendedSettings.Scene_Desaturation == 0.0f &&
			BlendedSettings.Scene_HighLights.Equals(FVector(1,1,1)) &&
			BlendedSettings.Scene_MidTones.Equals(FVector(1,1,1)) &&
			BlendedSettings.Scene_Shadows.Equals(FVector(0,0,0)) )
		{
			BlendedSettings.bEnableSceneEffect = FALSE;
		}
	}

	// calc scene material lerp amount
	FLOAT TonemapFade = 1.f;
	const FLOAT RemainingTonemapBlendTime = Max(NewSettings.Tonemap_InterpolationDuration - ElapsedBlendTime,0.f);
	if(RemainingTonemapBlendTime > DeltaTime)
	{
		TonemapFade = Clamp<FLOAT>(DeltaTime / RemainingTonemapBlendTime,0.f,1.f);
	}	
	BlendedSettings.ColorCorrection_InterpolationDuration = NewSettings.ColorCorrection_InterpolationDuration;
	BlendedSettings.Tonemap_InterpolationDuration = NewSettings.Tonemap_InterpolationDuration;
	BlendedSettings.Tonemap_MinExposure	= Lerp<FLOAT>(CurrentSettings.Tonemap_MinExposure,NewSettings.Tonemap_MinExposure,TonemapFade);
	BlendedSettings.Tonemap_MaxExposure	= Lerp<FLOAT>(CurrentSettings.Tonemap_MaxExposure,NewSettings.Tonemap_MaxExposure,TonemapFade);		
	BlendedSettings.Tonemap_MinExposure	= Clamp( BlendedSettings.Tonemap_MinExposure, 0.0f, 1.0f );
	BlendedSettings.Tonemap_MaxExposure	= Max( BlendedSettings.Tonemap_MaxExposure, 1.0f );

	//<@ ava specific ; 2007. 3. 23 changmin
	BlendedSettings.Tonemap_Darkness = Lerp<FLOAT>(CurrentSettings.Tonemap_Darkness,NewSettings.Tonemap_Darkness,TonemapFade);
	BlendedSettings.Tonemap_Beta = Lerp<FLOAT>(CurrentSettings.Tonemap_Beta,NewSettings.Tonemap_Beta,TonemapFade);
	BlendedSettings.Tonemap_Lmax = Lerp<FLOAT>(CurrentSettings.Tonemap_Lmax,NewSettings.Tonemap_Lmax,TonemapFade);
	BlendedSettings.Tonemap_LavgScale = Lerp<FLOAT>(CurrentSettings.Tonemap_LavgScale,NewSettings.Tonemap_LavgScale,TonemapFade);
	BlendedSettings.Tonemap_ScaleBias = Lerp<FLOAT>(CurrentSettings.Tonemap_ScaleBias,NewSettings.Tonemap_ScaleBias,TonemapFade);
	BlendedSettings.Tonemap_MaxKeyValue = Lerp<FLOAT>(CurrentSettings.Tonemap_MaxKeyValue,NewSettings.Tonemap_MaxKeyValue,TonemapFade);
	BlendedSettings.Tonemap_MinKeyValue = Lerp<FLOAT>(CurrentSettings.Tonemap_MinKeyValue,NewSettings.Tonemap_MinKeyValue,TonemapFade);
	BlendedSettings.Tonemap_KeyValueScale = Lerp<FLOAT>(CurrentSettings.Tonemap_KeyValueScale,NewSettings.Tonemap_KeyValueScale,TonemapFade);
	BlendedSettings.Tonemap_KeyValueOffset = Lerp<FLOAT>(CurrentSettings.Tonemap_KeyValueOffset,NewSettings.Tonemap_KeyValueOffset,TonemapFade);
	BlendedSettings.Tonemap_Shadow = Lerp<FLOAT>(CurrentSettings.Tonemap_Shadow,NewSettings.Tonemap_Shadow,TonemapFade);
	BlendedSettings.Tonemap_Highlight = Lerp<FLOAT>(CurrentSettings.Tonemap_Highlight,NewSettings.Tonemap_Highlight,TonemapFade);	
	BlendedSettings.ColorCorrection_Index = NewSettings.ColorCorrection_Index;

	//>@ ava
	if (BlendedSettings.bEnableTonemap)
	{
		if (BlendedSettings.Tonemap_MinExposure == 1.0f &&
			BlendedSettings.Tonemap_MaxExposure == 1.0f)
		{
			BlendedSettings.bEnableTonemap = FALSE;
		}
	}

	// Update the current settings and timer.
	CurrentPPInfo.LastSettings = BlendedSettings;
	CurrentPPInfo.LastBlendTime = CurrentWorldTime;
}


void ULocalPlayer::UpdatePostProcessSettings(const FVector& ViewLocation)
{
	// Find the post-process settings for the view.
	FPostProcessSettings NewSettings;
	APostProcessVolume* NewVolume;

	// Give priority to local PP override flag
	if ( bOverridePostProcessSettings )
	{
		NewVolume = NULL;
		NewSettings = PostProcessSettingsOverride;
		CurrentPPInfo.BlendStartTime = PPSettingsOverrideStartBlend;
	}
	// If not forcing an override on the LocalPlayer, see if we have Camera that wants to override PP settings
	// If so, we just grab those settings straight away and return - no blending.
	else if(Actor && Actor->PlayerCamera && Actor->PlayerCamera->bCamOverridePostProcess)
	{
		NewVolume = NULL;
		CurrentPPInfo.LastSettings = Actor->PlayerCamera->CamPostProcessSettings;
		return;
	}
	else
	{
		NewVolume = GWorld->GetWorldInfo()->GetPostProcessSettings(ViewLocation,TRUE,NewSettings);
	}

	::InterpolatePostProcessSettings( CurrentPPInfo, NewVolume, NewSettings, PP_DesaturationMultiplier, PP_HighlightsMultiplier, PP_MidTonesMultiplier, PP_ShadowsMultiplier );
}
/**
* Calculate the view settings for drawing from this view actor
*
* @param	View - output view struct
* @param	ViewLocation - output actor location
* @param	ViewRotation - output actor rotation
* @param	Viewport - current client viewport
*/
FSceneView* ULocalPlayer::CalcSceneView( FSceneViewFamily* ViewFamily, FVector& ViewLocation, FRotator& ViewRotation, FViewport* Viewport )
{
	if( !Actor )
	{
		return NULL;
	}

	// do nothing if the viewport size is zero - this allows the viewport client the capability to temporarily remove a viewport without actually destroying and recreating it
	if (Size.X <= 0.f || Size.Y <= 0.f)
	{
		return NULL;
	}

	check(Viewport);

	// Compute the view's screen rectangle.
	INT X = appTrunc(Origin.X * Viewport->GetSizeX());
	INT Y = appTrunc(Origin.Y * Viewport->GetSizeY());
	UINT SizeX = appTrunc(Size.X * Viewport->GetSizeX());
	UINT SizeY = appTrunc(Size.Y * Viewport->GetSizeY());

	// Take screen percentage option into account if percentage != 100.
	GSystemSettings->ScaleScreenCoords(X,Y,SizeX,SizeY);

	FLOAT fFOV;

	// if the object propagtor is pushing us new values, use them instead of the player
	if (bOverrideView)
	{
		ViewLocation = OverrideLocation;
		ViewRotation = OverrideRotation;
		fFOV = OverrideFOV;
	}
	else
	{
		Actor->eventGetPlayerViewPoint( ViewLocation, ViewRotation );
		fFOV = Actor->eventGetFOVAngle();
	}	

	// scale distances for cull distance purposes by the ratio of our current FOV to the default FOV
	Actor->LODDistanceFactor = fFOV / Max<FLOAT>(0.01f, (Actor->PlayerCamera != NULL) ? Actor->PlayerCamera->DefaultFOV : Actor->DefaultFOV);

	FMatrix ViewMatrix = FTranslationMatrix(-ViewLocation);
	ViewMatrix = ViewMatrix * FInverseRotationMatrix(ViewRotation);
	ViewMatrix = ViewMatrix * FMatrix(
		FPlane(0,	0,	1,	0),
		FPlane(1,	0,	0,	0),
		FPlane(0,	1,	0,	0),
		FPlane(0,	0,	0,	1));

	UGameUISceneClient* SceneClient = UUIRoot::GetSceneClient();

	FMatrix ProjectionMatrix;
	if( Actor && Actor->PlayerCamera != NULL && Actor->PlayerCamera->bConstrainAspectRatio && !SceneClient->IsUIActive() )
	{
		ProjectionMatrix = FPerspectiveMatrix(
			fFOV * (FLOAT)PI / 360.0f,
			Actor->PlayerCamera->ConstrainedAspectRatio,
			1.0f,
			NEAR_CLIPPING_PLANE
			);

		// Enforce a particular aspect ratio for the render of the scene. 
		// Results in black bars at top/bottom etc.
		Viewport->CalculateViewExtents( 
				Actor->PlayerCamera->ConstrainedAspectRatio, 
				X, Y, SizeX, SizeY );
	}
	else 
	{
		FLOAT CurViewAspectRatio = ((FLOAT)Viewport->GetSizeX()) / ((FLOAT)Viewport->GetSizeY());
		ProjectionMatrix = FPerspectiveMatrix(
			fFOV * (FLOAT)PI / 360.0f,
			SizeX * Viewport->GetDesiredAspectRatio() / CurViewAspectRatio,
			SizeY,
			NEAR_CLIPPING_PLANE
			);
	}

	FLinearColor OverlayColor(0,0,0,0);
	FLinearColor ColorScale(FLinearColor::White);

	if( Actor && Actor->PlayerCamera && !SceneClient->IsUIActive() )
	{
		// Apply screen fade effect to screen.
		if(Actor->PlayerCamera->bEnableFading)
		{
			OverlayColor = Actor->PlayerCamera->FadeColor.ReinterpretAsLinear();
			OverlayColor.A = Clamp(Actor->PlayerCamera->FadeAmount,0.0f,1.0f);
		}

		// Do color scaling if desired.
		if(Actor->PlayerCamera->bEnableColorScaling)
		{
			ColorScale = FLinearColor(
				Actor->PlayerCamera->ColorScale.X,
				Actor->PlayerCamera->ColorScale.Y,
				Actor->PlayerCamera->ColorScale.Z
				);
		}
	}


	//{{ ForegroundDPG용 FOV 설정 : PlayerController->Pawn->Weapon에 설정된다.
	//!{ 2006.3.6	허 창 민
	FMatrix ForegroundProjectionMatrix;
	FLOAT fForegroundFovAngle	= Actor->eventGetForegroundFOVAngle();

	// 0.0 이면 같은 fov 사용
	if( fForegroundFovAngle == 0.0f )
	{
		fForegroundFovAngle = fFOV;
	}

	FLOAT MatrixForegroundFov	= fForegroundFovAngle * (FLOAT)PI / 360.0f;
	if(Actor && Actor->PlayerCamera != NULL && Actor->PlayerCamera->bConstrainAspectRatio )
	{

		ForegroundProjectionMatrix = FPerspectiveMatrix(
			MatrixForegroundFov,
			Actor->PlayerCamera->ConstrainedAspectRatio,
			1.f,
			1.f	//ForeGround 의 Near 값은 좀 더 작아야 하기 때문에 View.NearClipPlane(10.0) 에서 0.1 로 수정
			);
	}
	else
	{
		ForegroundProjectionMatrix = FPerspectiveMatrix(
			MatrixForegroundFov,
			Viewport->GetSizeX() * Size.X,
			Viewport->GetSizeY() * Size.Y,
			1.f	//ForeGround 의 Near 값은 좀 더 작아야 하기 때문에 View.NearClipPlane(10.0) 에서 0.1 로 수정
			);
	}	

	//!} 2006.3.6	허 창 민
	//}}

	// Update the player's post process settings.
	UpdatePostProcessSettings(ViewLocation);

	TArray<FPrimitiveSceneInfo*> HiddenPrimitives;
	if(Actor->PlayerCamera)
	{
		// Translate the camera's hidden actors list to a hidden primitive list.
		const TArray<AActor*>& HiddenActors = Actor->PlayerCamera->HiddenActors;
		for(INT ActorIndex = 0;ActorIndex < HiddenActors.Num();ActorIndex++)
		{
			AActor* HiddenActor = HiddenActors(ActorIndex);
			if(HiddenActor)
			{
				for(INT ComponentIndex = 0;ComponentIndex < HiddenActor->AllComponents.Num();ComponentIndex++)
				{
					UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(HiddenActor->AllComponents(ComponentIndex));
					if(PrimitiveComponent && PrimitiveComponent->SceneInfo && !PrimitiveComponent->bIgnoreHiddenActorsMembership)
					{
						HiddenPrimitives.AddItem(PrimitiveComponent->SceneInfo);
					}
				}
			}
		}
	}

	FSceneView* View = new FSceneView(
		ViewFamily,
		ViewState,
		&ActorVisibilityHistory,
		Actor->GetViewTarget(),
		PlayerPostProcess,
		&CurrentPPInfo.LastSettings,
		NULL,
		X,
		Y,
		SizeX,
		SizeY,
		ViewMatrix,
		ProjectionMatrix,
		ForegroundProjectionMatrix,
		fForegroundFovAngle,		
		FLinearColor::Black,
		OverlayColor,
		ColorScale,
		HiddenPrimitives,
		FALSE,
		Actor->LODDistanceFactor,
		Actor->ExposureCenterRegionX,
		Actor->ExposureCenterRegionY
		);		

	extern void Hack_SetSeeThroughSettings( FSceneView* View );

	Hack_SetSeeThroughSettings( View );
	
	ViewFamily->Views.AddItem(View);

	return View;
}

IMPLEMENT_COMPARE_POINTER(USequenceOp, UnPlayer, { return(B->ActivateCount - A->ActivateCount); } );

//
//	ULocalPlayer::Exec
//

UBOOL ULocalPlayer::Exec(const TCHAR* Cmd,FOutputDevice& Ar)
{
#if !FINAL_RELEASE
	if(ParseCommand(&Cmd,TEXT("TESTDLO")))
	{
		void TestDLOConsistency();
		TestDLOConsistency();
		return TRUE;
	}
	else 
#endif
	/** This will show all of the SkeletalMeshComponents that were ticked for one frame **/
	if( ParseCommand(&Cmd,TEXT("SHOWSKELCOMPTICKTIME")) )
	{
		GShouldLogOutAFrameOfSkelCompTick = TRUE;
		return TRUE;
	}
	else if(ParseCommand(&Cmd,TEXT("LISTTEXTURES")))
	{
		UBOOL bShouldOnlyListStreaming		= ParseCommand(&Cmd, TEXT("STREAMING"));
		UBOOL bShouldOnlyListNonStreaming	= ParseCommand(&Cmd, TEXT("NONSTREAMING"));
		debugf( TEXT("Listing %s textures."), bShouldOnlyListNonStreaming ? TEXT("non streaming") : bShouldOnlyListStreaming ? TEXT("streaming") : TEXT("all")  );

		// Traverse streamable list, creating a map of all streamable textures for fast lookup.
		TMap<UTexture2D*,UBOOL> StreamableTextureMap;
		for( TLinkedList<UTexture2D*>::TIterator It(UTexture2D::GetStreamableList()); It; It.Next() )
		{	
			UTexture2D* Texture	= *It;
			StreamableTextureMap.Set( Texture, TRUE );
		}
		
		// Collect textures.
		TArray<FSortedTexture> SortedTextures;
		for( TObjectIterator<UTexture2D> It; It; ++It )
		{
			UTexture2D*		Texture				= *It;
			INT				OrigSizeX			= Texture->SizeX;
			INT				OrigSizeY			= Texture->SizeY;
			INT				DroppedMips			= Texture->Mips.Num() - Texture->ResidentMips;
			INT				CurSizeX			= Texture->SizeX >> DroppedMips;
			INT				CurSizeY			= Texture->SizeY >> DroppedMips;
			UBOOL			bIsStreamingTexture = StreamableTextureMap.Find( Texture ) != NULL;
			INT				LODGroup			= Texture->LODGroup;
			INT				LODBias				= Texture->LODBias;
			INT				NumMips				= Texture->Mips.Num();	
			INT				MaxMips				= Max( 1, Min( NumMips - Texture->GetCachedLODBias(), GMaxTextureMipCount ) );
			INT				MaxSize				= 0;
			INT				CurrentSize			= 0;

			for( INT MipIndex=NumMips-MaxMips; MipIndex<NumMips; MipIndex++ )
			{
				const FTexture2DMipMap& Mip = Texture->Mips(MipIndex);
				MaxSize += Mip.Data.GetBulkDataSize();
			}
			for( INT MipIndex=NumMips-Texture->ResidentMips; MipIndex<NumMips; MipIndex++ )
			{
				const FTexture2DMipMap& Mip = Texture->Mips(MipIndex);
				CurrentSize += Mip.Data.GetBulkDataSize();
			}

			if( (bShouldOnlyListStreaming && bIsStreamingTexture) 
			||	(bShouldOnlyListNonStreaming && !bIsStreamingTexture) 
			||	(!bShouldOnlyListStreaming && !bShouldOnlyListNonStreaming) )
			{
				new(SortedTextures) FSortedTexture( 
										OrigSizeX, 
										OrigSizeY, 
										CurSizeX,
										CurSizeY,
										LODBias, 
										MaxSize / 1024, 
										CurrentSize / 1024, 
										Texture->GetPathName(), 
										LODGroup, 
										bIsStreamingTexture );
			}
		}

		// Sort textures by cost.
		Sort<USE_COMPARE_CONSTREF(FSortedTexture,UnPlayer)>(SortedTextures.GetTypedData(),SortedTextures.Num());

		// Display.
		INT TotalMaxSize		= 0;
		INT TotalCurrentSize	= 0;
		debugf( TEXT(",Orig Width,Orig Height,Cur Width,Cur Height,Max Size,Cur Size,LODBias,LODGroup,Name,Streaming") );
		for( INT TextureIndex=0; TextureIndex<SortedTextures.Num(); TextureIndex++ )
 		{
 			const FSortedTexture& SortedTexture = SortedTextures(TextureIndex);
			debugf( TEXT(",%i,%i,%i,%i,%i,%i,%i,%s,%s,%s"),
 				SortedTexture.OrigSizeX,
 				SortedTexture.OrigSizeY,
				SortedTexture.CurSizeX,
				SortedTexture.CurSizeY,
				SortedTexture.MaxSize,
 				SortedTexture.CurrentSize,
				SortedTexture.LODBias,
 				*TextureGroupNames[SortedTexture.LODGroup],
				*SortedTexture.Name,
				SortedTexture.bIsStreaming ? TEXT("YES") : TEXT("NO") );
			
			TotalMaxSize		+= SortedTexture.MaxSize;
			TotalCurrentSize	+= SortedTexture.CurrentSize;
 		}

		debugf(TEXT("Total size: Max= %d  Current= %d"), TotalMaxSize, TotalCurrentSize );
		return TRUE;
	}
	else if(ParseCommand(&Cmd,TEXT("LISTANIMSETS")))
	{
		TArray<FSortedAnimSet> SortedSets;
		for( TObjectIterator<UAnimSet> It; It; ++It )
		{
			UAnimSet* Set = *It;
			new(SortedSets) FSortedAnimSet(Set->GetPathName(), Set->GetResourceSize());
		}

		// Sort anim sets by cost
		Sort<USE_COMPARE_CONSTREF(FSortedAnimSet,UnPlayer)>(SortedSets.GetTypedData(),SortedSets.Num());

		// Now print them out.
		debugf(TEXT("Loaded AnimSets:"));
		INT TotalSize = 0;
		for(INT i=0; i<SortedSets.Num(); i++)
		{
			FSortedAnimSet& SetInfo = SortedSets(i);
			TotalSize += SetInfo.Size;
			debugf(TEXT("Size: %d\tName: %s"), SetInfo.Size, *SetInfo.Name);
		}
		debugf(TEXT("Total Size:%d"), TotalSize);
		return TRUE;
	}
	else if(ParseCommand(&Cmd,TEXT("ANIMSEQSTATS")))
	{
		extern void GatherAnimSequenceStats(FOutputDevice& Ar);
		GatherAnimSequenceStats( Ar );
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("SHOWHOTKISMET")) )
	{
		// First make array of all USequenceOps
		TArray<USequenceOp*> AllOps;
		for( TObjectIterator<USequenceOp> It; It; ++It )
		{
			USequenceOp* Op = *It;
			AllOps.AddItem(Op);
		}

		// Then sort them
		Sort<USE_COMPARE_POINTER(USequenceOp, UnPlayer)>(&AllOps(0), AllOps.Num());

		// Then print out the top 10
		INT TopNum = ::Min(10, AllOps.Num());
		Ar.Logf( TEXT("----- TOP 10 KISMET SEQUENCEOPS ------") );
		for(INT i=0; i<TopNum; i++)
		{
			Ar.Logf( TEXT("%6d: %s (%d)"), i, *(AllOps(i)->GetPathName()), AllOps(i)->ActivateCount );
		}
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("EXEC")) )
	{
		TCHAR Filename[512];
		if( ParseToken( Cmd, Filename, ARRAY_COUNT(Filename), 0 ) )
		{
			ExecMacro( Filename, Ar );
		}
		return TRUE;
	}
#if ENABLE_GPU_PROFILING
	else if( ParseCommand(&Cmd,TEXT("TOGGLEDRAWEVENTS")) )
	{
		if( GEmitDrawEvents )
		{
			GEmitDrawEvents = FALSE;
			debugf(TEXT("Draw events are now DISABLED"));
		}
		else
		{
			GEmitDrawEvents = TRUE;
			debugf(TEXT("Draw events are now ENABLED"));
		}
		return TRUE;
	}
#endif
	else if( ParseCommand(&Cmd,TEXT("TOGGLESTREAMINGVOLUMES")) )
	{
		if (ParseCommand(&Cmd, TEXT("ON")))
		{
			GWorld->DelayStreamingVolumeUpdates( 0 );
		}
		else if (ParseCommand(&Cmd, TEXT("OFF")))
		{
			GWorld->DelayStreamingVolumeUpdates( INDEX_NONE );
		}
		else
		{
			if( GWorld->StreamingVolumeUpdateDelay == INDEX_NONE )
			{
				GWorld->DelayStreamingVolumeUpdates( 0 );
			}
			else
			{
				GWorld->DelayStreamingVolumeUpdates( INDEX_NONE );
			}
		}
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("PUSHVIEW")) )
	{
		if (ParseCommand(&Cmd, TEXT("START")))
		{
			bOverrideView = TRUE;
		}
		else if (ParseCommand(&Cmd, TEXT("STOP")))
		{
			bOverrideView = FALSE;
		}
		else if (ParseCommand(&Cmd, TEXT("SYNC")))
		{
			if (bOverrideView)
			{
				// @todo: with PIE, this maybe be the wrong PlayWorld!
				GWorld->FarMoveActor(Actor->Pawn ? (AActor*)Actor->Pawn : Actor, OverrideLocation, FALSE, TRUE, TRUE);
				Actor->SetRotation(OverrideRotation);
			}
		}
		else
		{
			OverrideLocation.X = appAtof(*ParseToken(Cmd, 0));
			OverrideLocation.Y = appAtof(*ParseToken(Cmd, 0));
			OverrideLocation.Z = appAtof(*ParseToken(Cmd, 0));

			OverrideRotation.Pitch = appAtoi(*ParseToken(Cmd, 0));
			OverrideRotation.Yaw   = appAtoi(*ParseToken(Cmd, 0));
			OverrideRotation.Roll  = appAtoi(*ParseToken(Cmd, 0));
		}
		return TRUE;
	}
	// @hack: This is a test matinee skipping function, quick and dirty to see if it's good enough for
	// gameplay. Will fix up better when we have some testing done!
	else if (ParseCommand(&Cmd, TEXT("CANCELMATINEE")))
	{
		UBOOL bMatineeSkipped = FALSE;

		// allow optional parameter for initial time in the matinee that this won't work (ie, 
		// 'cancelmatinee 5' won't do anything in the first 5 seconds of the matinee)
		FLOAT InitialNoSkipTime = appAtof(Cmd);

		// is the player in cinematic mode?
		if (Actor->bCinematicMode)
		{
			// if so, look for all active matinees that has this Player in a director group
			for (TObjectIterator<USeqAct_Interp> It; It; ++It)
			{
				// isit currently playing (and skippable)?
				if (It->bIsPlaying && It->bIsSkippable && (It->bClientSideOnly || GWorld->IsServer()))
				{
					for (INT GroupIndex = 0; GroupIndex < It->GroupInst.Num(); GroupIndex++)
					{
						// is the PC the group actor?
						if (It->GroupInst(GroupIndex)->GetGroupActor() == Actor)
						{
							const FLOAT RightBeforeEndTime = 0.1f;
							// make sure we aren';t already at the end (or before the allowed skip time)
							if ((It->Position < It->InterpData->InterpLength - RightBeforeEndTime) && 
								(It->Position >= InitialNoSkipTime))
							{
								// skip to end
								It->SetPosition(It->InterpData->InterpLength - RightBeforeEndTime, TRUE);

								// send a callback that this was skipped
								GCallbackEvent->Send(CALLBACK_MatineeCanceled, *It);

								bMatineeSkipped = TRUE;
							}
						}
					}
				}
			}

			if(bMatineeSkipped && GWorld && GWorld->GetGameInfo())
			{
				GWorld->GetGameInfo()->eventMatineeCancelled();
			}
		}
		return TRUE;
	}
#if !FINAL_RELEASE
	else if(ParseCommand(&Cmd,TEXT("TOGGLEAUTOMATICBRIGHTNESS")))
	{
		if (ViewState)
			((FSceneViewState*)ViewState)->ExposureData.bDisplayLuminanceHistogram = !((FSceneViewState*)ViewState)->ExposureData.bDisplayLuminanceHistogram;		

		return TRUE;
	}
#endif
	else if(ParseCommand(&Cmd,TEXT("SETAUTOMATICBRIGHTNESS")))
	{
		if (ViewState)
			((FSceneViewState*)ViewState)->ExposureData.bDisplayLuminanceHistogram = TRUE;		

		return TRUE;
	}
	else if(ParseCommand(&Cmd,TEXT("RESETAUTOMATICBRIGHTNESS")))
	{
		if (ViewState)
			((FSceneViewState*)ViewState)->ExposureData.bDisplayLuminanceHistogram = FALSE;		

		return TRUE;
	}
	else if(ParseCommand(&Cmd,TEXT("TEST_INVERTCOLOR")))
	{
		//static BOOL bInverted = FALSE;
		//if (ViewState)
		//{
		//	UINT RowPitch, SlidePitch;
		//	FSceneViewState* SceneViewState = (FSceneViewState*)ViewState;
		//	SceneViewState->ColorCorrectionData.Texture = RHICreateTexture3D(2,2,2, PF_A8R8G8B8,1,TexCreate_ResolveTargetable);
		//	void *Dest = RHILockTexture3D(SceneViewState->ColorCorrectionData.Texture,0,TRUE, RowPitch, SlidePitch);
		//	FColor* DestColor = (FColor*)Dest;

		//	if(!bInverted)
		//	{
		//		for(INT B = 0 ; B < 2 ; B++)
		//		{
		//			for(INT G = 0 ; G < 2 ; G++)
		//			{
		//				for(INT R = 0 ; R < 2 ; R++)
		//				{
		//					*DestColor = FColor(R == 0 ? 255 : 0 , G == 0 ? 255 : 0 , B == 0 ? 255 : 0 );
		//					DestColor++;
		//				}
		//			}
		//		}
		//	}
		//	else
		//	{
		//		for(INT B = 0 ; B < 2 ; B++)
		//		{
		//			for(INT G = 0 ; G < 2 ; G++)
		//			{
		//				for(INT R = 0 ; R < 2 ; R++)
		//				{
		//					*DestColor = FColor(R * 255 , G * 255 , B * 255 );
		//					DestColor++;
		//				}
		//			}
		//		}
		//	}
		//	RHIUnlockTexture3D(SceneViewState->ColorCorrectionData.Texture,0);
		//}

		//bInverted = !bInverted;
		return TRUE;
	}
	else if(ViewportClient && ViewportClient->Exec(Cmd,Ar))
	{
		return TRUE;
	}
	else if(Actor)
	{
		// Since UGameViewportClient calls Exec on UWorld, we only need to explicitly
		// call UWorld::Exec if we either have a null GEngine or a null ViewportClient
		UBOOL bWorldNeedsExec = GEngine == NULL || ViewportClient == NULL;
		if( bWorldNeedsExec && GWorld->Exec(Cmd,Ar) )
		{
			return TRUE;
		}
		else if( GWorld->GetGameInfo() && GWorld->GetGameInfo()->ScriptConsoleExec(Cmd,Ar,Actor->Pawn) )
		{
			return TRUE;
		}
		else if( Actor->myHUD && Actor->myHUD->ScriptConsoleExec(Cmd,Ar,Actor->Pawn) )
		{
			return TRUE;
		}
		else if( Actor->CheatManager && Actor->CheatManager->ScriptConsoleExec(Cmd,Ar,Actor->Pawn) )
		{
			return TRUE;
		}
		else if( Actor->ScriptConsoleExec(Cmd,Ar,Actor->Pawn) )
		{
			return TRUE;
		}
		else if( Actor->PlayerInput && Actor->PlayerInput->ScriptConsoleExec(Cmd,Ar,Actor->Pawn) )
		{
			return TRUE;
		}
		else if( Actor->Pawn )
		{
			if( Actor->Pawn->ScriptConsoleExec(Cmd,Ar,Actor->Pawn) )
			{
				return TRUE;
			}
			else if( Actor->Pawn->InvManager && Actor->Pawn->InvManager->ScriptConsoleExec(Cmd,Ar,Actor->Pawn) )
			{
				return TRUE;
			}
			else if( Actor->Pawn->Weapon && Actor->Pawn->Weapon->ScriptConsoleExec(Cmd,Ar,Actor->Pawn) )
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
}

void ULocalPlayer::ExecMacro( const TCHAR* Filename, FOutputDevice& Ar )
{
	// Create text buffer and prevent garbage collection.
	UTextBuffer* Text = ImportObject<UTextBuffer>( GetTransientPackage(), NAME_None, 0, Filename );
	if( Text )
	{
		Text->AddToRoot();
		debugf( TEXT("Execing %s"), Filename );
		TCHAR Temp[256];
		const TCHAR* Data = *Text->Text;
		while( ParseLine( &Data, Temp, ARRAY_COUNT(Temp) ) )
		{
			Exec( Temp, Ar );
		}
		Text->RemoveFromRoot();
	}
	else
	{
		Ar.Logf( NAME_ExecWarning, *LocalizeError("FileNotFound",TEXT("Core")), Filename );
	}
}

void FConsoleOutputDevice::Serialize(const TCHAR* Text,EName Event)
{
	FStringOutputDevice::Serialize(Text,Event);
	FStringOutputDevice::Serialize(TEXT("\n"),Event);
	GLog->Serialize(Text,Event);

	if( Console != NULL )
	{
		Console->eventOutputText(Text);
	}
}

/**
*	Renders the FPS counter
*
*	@param Viewport	The viewport to render to
*	@param Canvas	Canvas object to use for rendering
*	@param X		Suggested X coordinate for where to start drawing
*	@param Y		Suggested Y coordinate for where to start drawing
*	@return			Y coordinate of the next line after this output
*/
INT DrawFPSCounter( FViewport* Viewport, FCanvas* Canvas, INT X, INT Y )
{
	// Render the FPS counter.
	if( GShowFpsCounter )
	{
#if STATS
		// Calculate the average FPS.
		FLOAT AverageMS = (FLOAT)GFPSCounter.GetAverage() * 1000.0;
		FLOAT AverageFPS = 1000.f / AverageMS;

		// (2007/04/26)
		static FLOAT maxAverageMS = 0;
		static FLOAT minAverageMS = 0;
		static FLOAT tmpMaxAverageMS = 0;
		static FLOAT tmpMinAverageMS = FLT_MAX;
		static FLOAT timer = 0;

		if ( AverageMS > tmpMaxAverageMS )
			tmpMaxAverageMS = AverageMS;
		if ( AverageMS < tmpMinAverageMS )
			tmpMinAverageMS = AverageMS;

		// 2초 마다 갱신.
		timer += GFPSCounter.GetFrameTime();
		if ( timer > 1.0f )
		{
			maxAverageMS = Min(tmpMaxAverageMS, 1000.0f);
			minAverageMS = Max(tmpMinAverageMS, 0.0f);
			tmpMaxAverageMS = 0;
			tmpMinAverageMS = FLT_MAX;
			timer = 0;
		}

		// Choose the counter color based on the average framerate.
		FColor FPSColor = AverageFPS < 20.0f ? FColor(255,0,0) : (AverageFPS < 29.5f ? FColor(255,255,0) : FColor(0,255,0));

		// Draw the FPS counter.
		DrawShadowedString(Canvas,
			X,
			96,
			*FString::Printf(TEXT("%2.2f FPS"),AverageFPS),
#if CONSOLE
			GEngine->MediumFont,
#else
			GEngine->SmallFont,
#endif
			FPSColor
			);

		// Draw the frame time.
		DrawShadowedString(Canvas,
			X,
			120,
			*FString::Printf(TEXT("(%2.2f ms)"), AverageMS),
#if CONSOLE
			GEngine->MediumFont,
#else
			GEngine->SmallFont,
#endif
			FPSColor
			);

		// Draw the min/max frame time.
		DrawShadowedString(Canvas,
			X - 40,
			144,
			*FString::Printf(TEXT("[%2.1f/%2.1f] ms"), minAverageMS, maxAverageMS),
#if PS3
			GEngine->MediumFont,
#else
			GEngine->SmallFont,
#endif
			FPSColor
			);

#else //STATS
		// Calculate rough frame time even if stats are disabled.
		static DOUBLE LastTime	= 0;
		DOUBLE CurrentTime		= appSeconds();
		FLOAT FrameTime			= (CurrentTime - LastTime) * 1000;
		LastTime				= CurrentTime;

		// Draw the frame time in ms.
		DrawShadowedString(Canvas,
			Viewport->GetSizeX() - 220,
			120,
			*FString::Printf(TEXT("%4.2f ms"), FrameTime),
			GEngine->MediumFont,
			FColor( 255, 255, 255 )
			);
#endif
		Y = 144;
	}
	return Y;
}

/**
*	Draws frame times for the overall frame, gamethread, renderthread and GPU.
*	The gamethread time excludes idle time while it's waiting for the render thread.
*	The renderthread time excludes idle time while it's waiting for more commands from the gamethread or waiting for the GPU to swap backbuffers.
*	The GPU time is a measurement on the GPU from the beginning of the first drawcall to the end of the last drawcall. It excludes
*	idle time while waiting for VSYNC. However, it will include any starvation time between drawcalls.
*
*	@param Viewport	The viewport to render to
*	@param Canvas	Canvas object to use for rendering
*	@param X		Suggested X coordinate for where to start drawing
*	@param Y		Suggested Y coordinate for where to start drawing
*	@return			Y coordinate of the next line after this output
*/
INT DrawUnitTimes( FViewport* Viewport, FCanvas* Canvas, INT X, INT Y )
{
	// Render CPU thread and GPU frame times.
	if( GShowUnitTimes )
	{
#if CONSOLE
		UFont* Font = GEngine->MediumFont;
#else
		UFont* Font = GEngine->SmallFont;
#endif
		INT SafeZone			= 0;
#if CONSOLE
		SafeZone				= appTrunc(Viewport->GetSizeX() * 0.05f);
#endif
		FColor Color;

		//void StringSize(UFont* Font,INT& XL,INT& YL,const TCHAR* Format,...);
		INT XL, YL;
		StringSize(Font,XL,YL,TEXT("0.ajkl!"));
		
		INT X2					= Viewport->GetSizeX() - Font->GetStringSize( TEXT(" 000.00 ms ") ) - SafeZone;
		INT X1					= X2 - Font->GetStringSize( TEXT("Frame: ") );
		INT RowHeight			= appTrunc( YL * 1.1f );
		static DOUBLE LastTime	= 0.0;
		static FLOAT FrameTime	= 0.0f;
		DOUBLE CurrentTime		= appSeconds();
		if ( LastTime == 0 )
		{
			LastTime = CurrentTime;
		}
		FrameTime				= 0.9 * FrameTime + 0.1 * (CurrentTime - LastTime) * 1000.0f;
		LastTime				= CurrentTime;

		/** How many cycles the renderthread used (excluding idle time). It's set once per frame in FViewport::Draw. */
		extern DWORD GRenderThreadTime;
		/** How many cycles the gamethread used (excluding idle time). It's set once per frame in FViewport::Draw. */
		extern DWORD GGameThreadTime;

		/** Number of milliseconds the renderthread was used last frame. */
		static FLOAT RenderThreadTime = 0.0f;
		RenderThreadTime = 0.9 * RenderThreadTime + 0.1 * GRenderThreadTime * GSecondsPerCycle * 1000.0;

		/** Number of milliseconds the gamethread was used last frame. */
		static FLOAT GameThreadTime = 0.0f;
		GameThreadTime = 0.9 * GameThreadTime + 0.1 * GGameThreadTime * GSecondsPerCycle * 1000.0;

		// 0-34 ms: Green, 34-50 ms: Yellow, 50+ ms: Red
		Color = FrameTime < 34.0f ? FColor(0,255,0) : (FrameTime < 50.0f ? FColor(255,255,0) : FColor(255,0,0));
		DrawShadowedString(Canvas,X1,Y, *FString::Printf(TEXT("Frame:")),Font,FColor(255,255,255));
		DrawShadowedString(Canvas,X2,Y, *FString::Printf(TEXT("%3.2f ms"), FrameTime),Font,Color);
		Y += RowHeight;

		Color = GameThreadTime < 34.0f ? FColor(0,255,0) : (GameThreadTime < 50.0f ? FColor(255,255,0) : FColor(255,0,0));
		DrawShadowedString(Canvas,X1,Y, *FString::Printf(TEXT("Game:")),Font,FColor(255,255,255));
		DrawShadowedString(Canvas,X2,Y, *FString::Printf(TEXT("%3.2f ms"), GameThreadTime),Font,Color);
		Y += RowHeight;

		Color = RenderThreadTime < 34.0f ? FColor(0,255,0) : (RenderThreadTime < 50.0f ? FColor(255,255,0) : FColor(255,0,0));
		DrawShadowedString(Canvas,X1,Y, *FString::Printf(TEXT("Draw:")),Font,FColor(255,255,255));
		DrawShadowedString(Canvas,X2,Y, *FString::Printf(TEXT("%3.2f ms"), RenderThreadTime),Font,Color);
		Y += RowHeight;

#if CONSOLE
		static FLOAT GPUFrameTime = 0.0f;
		DWORD GPUCycles = RHIGetGPUFrameCycles();
		if ( GPUCycles > 0 )
		{
			GPUFrameTime = 0.9 * GPUFrameTime + 0.1 * GPUCycles * GSecondsPerCycle * 1000.0;
			Color = GPUFrameTime < 34.0f ? FColor(0,255,0) : (GPUFrameTime < 50.0f ? FColor(255,255,0) : FColor(255,0,0));
			DrawShadowedString(Canvas,X1,Y, *FString::Printf(TEXT("GPU:")),Font,FColor(255,255,255));
			DrawShadowedString(Canvas,X2,Y, *FString::Printf(TEXT("%3.2f ms"), GPUFrameTime),Font,Color);
			Y += RowHeight;
		}
#endif
	}
	return Y;
}

/**
*	Render the level stats
*
*	@param Viewport	The viewport to render to
*	@param Canvas	Canvas object to use for rendering
*	@param X		Suggested X coordinate for where to start drawing
*	@param Y		Suggested Y coordinate for where to start drawing
*	@return			Y coordinate of the next line after this output
*/
INT DrawLevelStats( FViewport* Viewport, FCanvas* Canvas, INT X, INT Y )
{
	// Render level stats.
	if( GShowLevelStats )
	{
		enum EStreamingStatus
		{
			LEVEL_Unloaded,
			LEVEL_UnloadedButStillAround,
			LEVEL_Loaded,
			LEVEL_MakingVisible,
			LEVEL_Visible,
			LEVEL_Preloading,
		};
		// Iterate over the world info's level streaming objects to find and see whether levels are loaded, visible or neither.
		TMap<FName,INT> StreamingLevels;	
		AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
		for( INT LevelIndex=0; LevelIndex<WorldInfo->StreamingLevels.Num(); LevelIndex++ )
		{
			ULevelStreaming* LevelStreaming = WorldInfo->StreamingLevels(LevelIndex);
			if( LevelStreaming 
				&&  LevelStreaming->PackageName != NAME_None 
				&&	LevelStreaming->PackageName != GWorld->GetOutermost()->GetFName() )
			{
				if( LevelStreaming->LoadedLevel && !LevelStreaming->bHasUnloadRequestPending )
				{
					if( GWorld->Levels.FindItemIndex( LevelStreaming->LoadedLevel ) != INDEX_NONE )
					{
						if( LevelStreaming->LoadedLevel->bHasVisibilityRequestPending )
						{
							StreamingLevels.Set( LevelStreaming->PackageName, LEVEL_MakingVisible );
						}
						else
						{
							StreamingLevels.Set( LevelStreaming->PackageName, LEVEL_Visible );
						}
					}
					else
					{
						StreamingLevels.Set( LevelStreaming->PackageName, LEVEL_Loaded );
					}
				}
				else
				{
					// See whether the level's world object is still around.
					UPackage* LevelPackage	= Cast<UPackage>(UGameViewportClient::StaticFindObjectFast( UPackage::StaticClass(), NULL, LevelStreaming->PackageName ));
					UWorld*	  LevelWorld	= NULL;
					if( LevelPackage )
					{
						LevelWorld = Cast<UWorld>(UGameViewportClient::StaticFindObjectFast( UWorld::StaticClass(), LevelPackage, NAME_TheWorld ));
					}

					if( LevelWorld )
					{
						StreamingLevels.Set( LevelStreaming->PackageName, LEVEL_UnloadedButStillAround );
					}
					else
					{
						StreamingLevels.Set( LevelStreaming->PackageName, LEVEL_Unloaded );
					}
				}
			}
		}

		UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
		if (GameEngine != NULL)
		{
			// toss in the levels being loaded by PrepareMapChange
			for( INT LevelIndex=0; LevelIndex < GameEngine->LevelsToLoadForPendingMapChange.Num(); LevelIndex++ )
			{
				const FName LevelName = GameEngine->LevelsToLoadForPendingMapChange(LevelIndex);
				StreamingLevels.Set(LevelName, LEVEL_Preloading);
			}
		}

		// Render unloaded levels in red, loaded ones in yellow and visible ones in green. Blue signifies that a level is unloaded but
		// hasn't been garbage collected yet.
		DrawShadowedString(Canvas, X, Y, TEXT("Level streaming"), GEngine->SmallFont, FLinearColor::White );
		Y+=12;


		ULevel *LevelPlayerIsIn = NULL;

		for( AController* Controller = GWorld->GetWorldInfo()->ControllerList; 
			Controller != NULL; 
			Controller = Controller->NextController
			)
		{
			APlayerController* PC = Cast<APlayerController>( Controller );

			if( ( PC != NULL )
				&&( PC->Pawn != NULL )
				)
			{
				// need to do a trace down here
				//TraceActor = Trace( out_HitLocation, out_HitNormal, TraceDest, TraceStart, false, TraceExtent, HitInfo, true );
				FCheckResult Hit(1.0f);
				DWORD TraceFlags;
				TraceFlags = TRACE_World;

				FVector TraceExtent(0,0,0);

				// this will not work for flying around :-(
				GWorld->SingleLineCheck( Hit, PC->Pawn, (PC->Pawn->Location-FVector(0, 0, 256 )), PC->Pawn->Location, TraceFlags, TraceExtent );

				if (Hit.Level != NULL)
				{
					LevelPlayerIsIn = Hit.Level;
				}
				else
					if (Hit.Actor != NULL)
					{
						LevelPlayerIsIn = Hit.Actor->GetLevel();
					}
					else
						if (Hit.Component != NULL)
						{
							LevelPlayerIsIn = Hit.Component->GetOwner()->GetLevel();
						}
			}
		}

		// now draw the "map" name
		FString MapName				= GWorld->CurrentLevel->GetOutermost()->GetName();
		FString LevelPlayerIsInName = LevelPlayerIsIn != NULL ? LevelPlayerIsIn->GetOutermost()->GetName() : TEXT("None");

		if( LevelPlayerIsInName == MapName )
		{
			MapName = *FString::Printf( TEXT("-> %s"), *MapName );
		}
		else
		{
			MapName = *FString::Printf( TEXT("   %s"), *MapName );
		}

		DrawShadowedString(Canvas, X, Y, *MapName, GEngine->SmallFont, FColor(127,127,127) );
		Y+=12;

		// now draw the levels
		for( TMap<FName,INT>::TIterator It(StreamingLevels); It; ++It )
		{
			FString	LevelName	= It.Key().ToString();
			INT		Status		= It.Value();
			FColor	Color		= FColor(255,255,255);
			switch( Status )
			{
			case LEVEL_Visible:
				Color = FColor(255,0,0);	// red  loaded and visible
				break;
			case LEVEL_MakingVisible:
				Color = FColor(255,128,0);	// orange, in process of being made visible
				break;
			case LEVEL_Loaded:
				Color = FColor(255,255,0);	// yellow loaded but not visible
				break;
			case LEVEL_UnloadedButStillAround:
				Color = FColor(0,0,255);	// blue  (GC needs to occur to remove this)
				break;
			case LEVEL_Unloaded:
				Color = FColor(0,255,0);	// green
				break;
			case LEVEL_Preloading:
				Color = FColor(255,0,255);	// purple (preloading)
				break;
			default:
				break;
			};

			UPackage* LevelPackage = FindObject<UPackage>( NULL, *LevelName );

			if( LevelPackage 
				&& (LevelPackage->GetLoadTime() > 0) 
				&& (Status != LEVEL_Unloaded) )
			{
				LevelName += FString::Printf(TEXT(" - %4.1f sec"), LevelPackage->GetLoadTime());
			}
			else if( UObject::GetAsyncLoadPercentage( *LevelName ) >= 0 )
			{
				INT Percentage = appTrunc( UObject::GetAsyncLoadPercentage( *LevelName ) );
				LevelName += FString::Printf(TEXT(" - %3i %%"), Percentage ); 
			}

			if( LevelPlayerIsInName == LevelName )
			{
				LevelName = *FString::Printf( TEXT("->  %s"), *LevelName );
			}
			else
			{
				LevelName = *FString::Printf( TEXT("    %s"), *LevelName );
			}

			DrawShadowedString(Canvas, X + 4, Y, *LevelName, GEngine->SmallFont, Color );
			Y+=12;
		}
	}
	return Y;
}
