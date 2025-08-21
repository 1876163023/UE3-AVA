#include "../avaLaunch.h"
#include "Engine.h"
#include "FullScreenMovieBink.h"

#if SUPPORTS_PRAGMA_PACK
#pragma pack (push,8)
#endif
#define RAD_NO_LOWERCASE_TYPES

#include "bink/bink.h"
#undef D3D_OVERLOADS
#include "bink/dx9rad3d.cpp"
#include "bink/binktextures.cpp"


#if SUPPORTS_PRAGMA_PACK
#pragma pack (pop)
#endif


#pragma comment(lib,"binkw32.lib")

static BINKTEXTURESET GBinkTextureSet = { 0 };
static HBINK GBink = 0;

void appPumpMessage()
{
	MSG msg;

	if ( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) )
	{
		if ( msg.message == WM_QUIT )
			return;

		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}	
}


static void Decompress_frame( BINKTEXTURESET * GBinkTextureSet )
{
	Lock_Bink_textures( GBinkTextureSet );
	BinkRegisterFrameBuffers( GBink, &GBinkTextureSet->bink_buffers );
	BinkDoFrame( GBink );
	
	while ( BinkShouldSkip( GBink ) )
	{
		BinkNextFrame( GBink );
		BinkDoFrame( GBink );
	}

	Unlock_Bink_textures( GDirect3DDevice, GBinkTextureSet, GBink );
	BinkNextFrame( GBink );
}

static void ShowFrame()
{
	extern UINT GD3DDeviceSizeX, GD3DDeviceSizeY;

	INT width = GD3DDeviceSizeX;
	INT height = GD3DDeviceSizeY;

	D3DVIEWPORT9 vp = { 0, 0, width, height, 0.0f, 1.0f };		

	FLOAT x_scale = (F32)width / (F32)GBink->Width;
	FLOAT y_scale = (F32)height / (F32)GBink->Height;

	y_scale = x_scale;

	GDirect3DDevice->SetViewport( &vp );
	GDirect3DDevice->BeginScene();		

	GDirect3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0f, 0 );

	Draw_Bink_textures( GDirect3DDevice, &GBinkTextureSet, GBink->Width, GBink->Height, 0, (height - y_scale * GBink->Height) / 2, x_scale, y_scale, 1.0f );

	GDirect3DDevice->EndScene();
	GDirect3DDevice->Present( 0, 0, 0, 0 );
}


/*-----------------------------------------------------------------------------
FFullScreenMovieBink
-----------------------------------------------------------------------------*/

/** 
* Constructor
*/
FFullScreenMovieBink::FFullScreenMovieBink()
: bPlaying(FALSE)
{
	BinkSoundUseDirectSound( 0 );

	CriticalSection = GSynchronizeFactory->CreateCriticalSection();
}

FFullScreenMovieBink::~FFullScreenMovieBink()
{
	GSynchronizeFactory->Destroy( CriticalSection );
}

/** 
* Perform one-time initialization and create instance
*
* @return new instance if successful
*/
FFullScreenMovieSupport* FFullScreenMovieBink::StaticInitialize()
{
	static FFullScreenMovieBink* StaticInstance = NULL;
	if( !StaticInstance )
	{
		StaticInstance = new FFullScreenMovieBink();
	}
	return StaticInstance;
}

/**
* Pure virtual that must be overloaded by the inheriting class. It will
* be called from within UnLevTick.cpp after ticking all actors.
*
* @param DeltaTime	Game time passed since the last call.
*/
void FFullScreenMovieBink::Tick(FLOAT DeltaTime)
{		
	if (GBink == NULL)
		return;

	if ( !BinkWait( GBink ) )
	{
		Decompress_frame( &GBinkTextureSet );

		CurrentFrame = GBink->FrameNum;

		ShowFrame();
	}

	// 끝에 도달했다.
	if (GBink->FrameNum == GBink->Frames)
	{
		SkipMovie();
	}	
}


/**
* Pure virtual that must be overloaded by the inheriting class. It is
* used to determine whether an object is ready to be ticked. This is 
* required for example for all UObject derived classes as they might be
* loaded async and therefore won't be ready immediately.
*
* @return	TRUE if class is ready to be ticked, FALSE otherwise.
*/
UBOOL FFullScreenMovieBink::IsTickable()
{	
	return bPlaying;
}

void FFullScreenMovieBink::PlayMovie( EMovieMode InMovieMode, const TCHAR* MovieFilename, INT StartFrame )
{
	PlayingMovieFilename = MovieFilename;	

	GBink = BinkOpen( TCHAR_TO_ANSI(MovieFilename), BINKSNDTRACK | BINKNOFRAMEBUFFERS );	

	if (GBink != NULL)
	{
		if ( Create_Bink_shaders( GDirect3DDevice ) )
		{
			BinkGetFrameBuffersInfo( GBink, &GBinkTextureSet.bink_buffers );

			if ( Create_Bink_textures( GDirect3DDevice, &GBinkTextureSet ) )
			{
				bPlaying = TRUE;

				CurrentFrame = 0;

				return;
			}

			Free_Bink_shaders();
		}
	}
	
	BinkClose(GBink);
	GBink = NULL;
}

/**
* Stops the currently playing movie
*
* @param DelayInSeconds Will delay the stopping of the movie for this many seconds. If zero, this function will wait until the movie stops before returning.
* @param bWaitForMovie if TRUE then wait until the movie finish event triggers
* @param bForceStop if TRUE then non-skippable movies and startup movies are forced to stop
*/
void FFullScreenMovieBink::GameThreadStopMovie(FLOAT DelayInSeconds,UBOOL bWaitForMovie,UBOOL bForceStop)
{	
	if (!bForceStop)
	{
		FLOAT WaitUntil = appSeconds() + DelayInSeconds;	

		while (IsTickable() && (bWaitForMovie || appSeconds() < WaitUntil))
		{
			ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
				WaitForMovie, 
				FFullScreenMovieBink*, Instance, this,
			{
				Instance->Tick(0.0f);
			});	

			appSleep(0.01f);			

			appPumpMessage();			

			// Windows message는 즉시 처리되지 않고 defer된다 
			extern void appUpdateTimaAndHandleMaxTickRate();
			appUpdateTimaAndHandleMaxTickRate();

			GEngine->Client->Tick( GDeltaTime );
		}
	}

	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
		WaitForMovie, 
		FFullScreenMovieBink*, Instance, this,
	{
		Instance->StopMovie( TRUE );
	});	
}

/**
* Block game thread until movie is complete (must have been started
* with GameThreadPlayMovie or it may never return)
*/
void FFullScreenMovieBink::GameThreadWaitForMovie()
{	
	GameThreadStopMovie();
}

void FFullScreenMovieBink::WaitForMovie()
{	
}

/**
* Checks to see if the movie has finished playing. Will return immediately
*
* @param MovieFilename MovieFilename to check against (should match the name passed to GameThreadPlayMovie). Empty string will match any movie
* 
* @return TRUE if the named movie has finished playing
*/
UBOOL FFullScreenMovieBink::GameThreadIsMovieFinished(const TCHAR* MovieFilename)
{
	return !bPlaying;
}

/**
* Checks to see if the movie is playing. Will return immediately
*
* @param MovieFilename MovieFilename to check against (should match the name passed to GameThreadPlayMovie). Empty string will match any movie
* 
* @return TRUE if the named movie is playing
*/
UBOOL FFullScreenMovieBink::GameThreadIsMoviePlaying(const TCHAR* MovieFilename)
{
	return bPlaying;
}

/**
* Get the name of the most recent movie played
*
* @return Name of the movie that was most recently played, or empty string if a movie hadn't been played
*/
FString FFullScreenMovieBink::GameThreadGetLastMovieName()
{
	FScopeLock ScopeLock( CriticalSection );

	return PlayingMovieFilename;
}

/**
* Kicks off a thread to control the startup movie sequence
*/
void FFullScreenMovieBink::GameThreadInitiateStartupSequence()
{
	// 첫 번째 Tick에서 Game window를 초기화 한다.
	GEngine->Tick(0);

#define VIDEO_ROOT TEXT("..\\avaGame\\Video\\")

	GameThreadPlayMovie(MM_PlayOnceFromStream, VIDEO_ROOT TEXT("neowiz.bik"));
	GameThreadPlayMovie(MM_PlayOnceFromStream, VIDEO_ROOT TEXT("redduck.bik"));
	GameThreadPlayMovie(MM_PlayOnceFromStream, VIDEO_ROOT TEXT("nvidia.bik"));	
	GameThreadPlayMovie(MM_PlayOnceFromStream, VIDEO_ROOT TEXT("eni.bik"));	
	GameThreadPlayMovie(MM_PlayOnceFromStream, VIDEO_ROOT TEXT("ava.bik"));	
}

/**
* Returns the current frame number of the movie (not thred synchronized in anyway, but it's okay 
* if it's a little off
*/
INT FFullScreenMovieBink::GameThreadGetCurrentFrame()
{
	return CurrentFrame;
}

/**
* Releases any dynamic resources. This is needed for flushing resources during device reset on d3d.
*/
void FFullScreenMovieBink::ReleaseDynamicResources()
{
	Free_Bink_textures( GDirect3DDevice, &GBinkTextureSet );
	Free_Bink_shaders();
}

void FFullScreenMovieBink::StopMovie( UBOOL bResetFlag )
{
	if (bPlaying)
	{
		Free_Bink_textures( GDirect3DDevice, &GBinkTextureSet );
		Free_Bink_shaders();
		BinkClose(GBink);
		GBink = NULL;		

		if (bResetFlag)
		{
			bPlaying = FALSE;
		}
	}	
}

UBOOL FFullScreenMovieBink::InputKey(FViewport* Viewport,INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed,UBOOL bGamepad)
{
	if (Event == IE_Pressed)
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			EnsureMoviePlaying,
			FFullScreenMovieBink*, Instance, this,		
		{
			Instance->SkipMovie();
		});	

		FlushRenderingCommands();
	}	

	return FViewportClient::InputKey(Viewport,ControllerId,Key,Event,AmountDepressed,bGamepad);
}

void FFullScreenMovieBink::SkipMovie()
{
	if (!bPlaying)
		return;

	// 일단 멈추고
	StopMovie( FALSE );

	// 다음 목록으로 넘어감
	// 없으면 그냥 끝
	if (!PlayNextMovie())
	{
		bPlaying = FALSE;
	}
	else
	{
		check( bPlaying );
	}
}

/**
* Kick off a movie play from the game thread
*
* @param InMovieMode How to play the movie (usually MM_PlayOnceFromStream or MM_LoopFromMemory).
* @param MovieFilename Path of the movie to play in its entirety
* @param StartFrame Optional frame number to start on
*/

void FFullScreenMovieBink::GameThreadPlayMovie( EMovieMode InMovieMode, const TCHAR* MovieFilename, INT StartFrame )
{
	FMoviePlayEntry Entry = { InMovieMode, MovieFilename, StartFrame };

	// Entry를 추가한다
	{
		FScopeLock ScopeLock( CriticalSection );
		QueuedMovie.AddItem( Entry );
	}	

	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
		EnsureMoviePlaying,
		FFullScreenMovieBink*, Instance, this,		
	{
		Instance->EnsureMoviePlaying();
	});	

	// 이 지점이 끝나야 Play가 확인될 수 있습니다.
	FlushRenderingCommands();
}

void FFullScreenMovieBink::EnsureMoviePlaying()
{
	if (!bPlaying)
	{
		PlayNextMovie();
	}
}

UBOOL FFullScreenMovieBink::PlayNextMovie()
{
	FMoviePlayEntry Entry;
	{
		FScopeLock ScopeLock( CriticalSection );

		// 없으면 나가쇼~
		if (QueuedMovie.Num() == 0)
		{			
			return FALSE;
		}

		Entry = QueuedMovie(0);
		QueuedMovie.Remove(0,1);
	}


	PlayMovie( Entry.InMovieMode, *Entry.MovieFilename, Entry.StartFrame );

	return TRUE;
}