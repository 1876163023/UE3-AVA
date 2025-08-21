/*=============================================================================
Launch.cpp: Game launcher.
Copyright 1997-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "avaLaunch.h"

#include  <io.h>

// use wxWidgets as a DLL

//
#include "../../External/NProtect/NPGameLib/NPGameLib.h"
#include "nProtect.h"

#if !FINAL_RELEASE
#include <wx/evtloop.h>  // has our base callback class
#endif

FEngineLoop	GEngineLoop;
/** Whether to use wxWindows when running the game */

#if !FINAL_RELEASE
UBOOL		GUsewxWindows;
#else
#define GUsewxWindows 0
#endif

#if STATS
/** Global for tracking FPS */
FFPSCounter GFPSCounter;
#endif

/*-----------------------------------------------------------------------------
WinMain.
-----------------------------------------------------------------------------*/

extern TCHAR MiniDumpFilenameW[64];
extern char  MiniDumpFilenameA[64];
extern INT CreateMiniDump( LPEXCEPTION_POINTERS ExceptionInfo );

// use wxWidgets as a DLL
extern bool IsUnrealWindowHandle( HWND hWnd );

#if !FINAL_RELEASE
class WxUnrealCallbacks : public wxEventLoopBase::wxUnrealCallbacks
{
public:

	virtual bool IsUnrealWindowHandle(HWND hwnd) const
	{
		return ::IsUnrealWindowHandle(hwnd);
	}

	virtual bool IsRequestingExit() const 
	{ 
		return GIsRequestingExit ? true : false; 
	}

	virtual void SetRequestingExit( bool bRequestingExit ) 
	{ 
		GIsRequestingExit = bRequestingExit ? true : false; 
	}

};

static WxUnrealCallbacks s_UnrealCallbacks;
#endif


/**
* Performs any required cleanup in the case of a fatal error.
*/
static void StaticShutdownAfterError()
{
	// Make sure Novodex is correctly torn down.
	DestroyGameRBPhys();

#if WITH_FACEFX
	// Make sure FaceFX is shutdown.
	UnShutdownFaceFX();
#endif // WITH_FACEFX

#if !FINAL_RELEASE
	// Unbind DLLs (e.g. SCC integration)
	WxLaunchApp* LaunchApp = (WxLaunchApp*) wxTheApp;
	if( LaunchApp )
	{
		LaunchApp->ShutdownAfterError();
	}
#endif
}

/**
* Static guarded main function. Rolled into own function so we can have error handling for debug/ release builds depending
* on whether a debugger is attached or not.
*/
static INT GuardedMain( const TCHAR* CmdLine, HINSTANCE hInInstance, HINSTANCE hPrevInstance, INT nCmdShow )
{
	// Set up minidump filename. We cannot do this directly inside main as appItoa returns an FString that requires 
	// destruction and main uses SEH.
	appStrcpy( MiniDumpFilenameW, *FString::Printf( TEXT("unreal-v%i-%s.dmp"), GEngineVersion, *appSystemTimeString() ) );
	strcpy( MiniDumpFilenameA, TCHAR_TO_ANSI( MiniDumpFilenameW ) );	

	INT ErrorLevel	= GEngineLoop.PreInit( CmdLine );

	extern void appHideSplashEx();	

#if !FINAL_RELEASE
	GUsewxWindows	= !(GIsUCC || ParseParam(appCmdLine(),TEXT("nowxwindows")));

	if( GUsewxWindows && ErrorLevel == 0 && !GIsRequestingExit )
	{
		// use wxWidgets as a DLL 
		// set the call back class here
		wxEventLoopBase::SetUnrealCallbacks( &s_UnrealCallbacks );

		// UnrealEd of game with wxWindows.
		ErrorLevel = wxEntry( hInInstance, hPrevInstance, "", nCmdShow);
	}
	else if( GIsUCC )
	{
		appHideSplashEx();

		// UCC.
		UBOOL bInheritConsole = FALSE;

#if !CONSOLE
		if(NULL != GLogConsole)
		{
			// if we're running from a console we inherited, do not sleep indefinitely
			bInheritConsole = GLogConsole->IsInherited();
		}
#endif

		// Either close log window manually or press CTRL-C to exit if not in "silent" or "nopause" mode.
		UBOOL bShouldPause = !bInheritConsole && !GIsSilent && !ParseParam(appCmdLine(),TEXT("NOPAUSE"));
		// if it was specified to not pause if successful, then check that here
		if (ParseParam(appCmdLine(),TEXT("NOPAUSEONSUCCESS")) && ErrorLevel == 0)
		{
			// we succeeded, so don't pause 
			bShouldPause = FALSE;
		}

		// pause if we should
		if (bShouldPause)
		{
			GLog->TearDown();
			GEngineLoop.Exit();
			Sleep(INFINITE);
		}
	}
	else
#endif // FINAL_RELEASE
	{
		if (GIsRequestingExit || ErrorLevel != 0)
		{
			GEngineLoop.Exit();
			return ErrorLevel;
		}		

		// Game without wxWindows.
		ErrorLevel = GEngineLoop.Init();

		appHideSplashEx();

		while( !GIsRequestingExit )
		{
#if STATS
			GFPSCounter.Update(appSeconds());
#endif
			{
				SCOPE_CYCLE_COUNTER(STAT_FrameTime);
				GEngineLoop.Tick();
			}
#if STATS
			// Write all stats for this frame out
			GStatManager.AdvanceFrame();

			if(GIsThreadedRendering)
			{
				ENQUEUE_UNIQUE_RENDER_COMMAND(AdvanceFrame,{GStatManager.AdvanceFrameForThread();});
			}
#endif
		}
	}
	GEngineLoop.Exit();
	return ErrorLevel;
}


//! WinMain호출 전에 실행되도록 한다.
class GlobalStatic
{
public:
	GlobalStatic()
	{
		// nProtect관련 함수 호출.
		FixVC80DEP();
	}
} GGlobalStatic;

UBOOL CheckForNPGE( TCHAR* pCmdLine )
{
	UBOOL bOptionSpecified = FALSE;
	
	if (appStristr(pCmdLine,TEXT("NPGE")) != NULL)
	{
		bOptionSpecified = TRUE;
	}

	if (_access("NPGE.txt",0) != -1)
	{
		bOptionSpecified = TRUE;
	}	
	
	// nProtect Game Encrypt처리후에는 바로 종료시켜 준다.
	if ( bOptionSpecified )
	{
		if ( !NPGameInit() )
			debugf(TEXT("Failed - NPGameInit."));

		NPGameExit();
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

INT WINAPI WinMain( HINSTANCE hInInstance, HINSTANCE hPrevInstance, char* /*pCmdLine_NotUsed*/, INT nCmdShow )
{	
#if FINAL_RELEASE
	// "NPGEClient.exe /ava:ava!@#:5:ava.exe;npge"형태로 입력받으면
	// CreateProcess("ava.exe", "npge", ... );로 처리되어서
	// main/WinMain계열의 CmdLine에서는 npge이후의 인자부터 CmdLine으로 처리되기 때문에
	// GetCommandLine()함수로 받아야 제대로 처리된다고 한다.
	if (CheckForNPGE( GetCommandLine() ))
		return 1;
#endif

	// Initialize all timing info
	appInitTiming();

	// default to no game
	appStrcpy(GGameName, TEXT("None"));

	INT ErrorLevel			= 0;
	GIsStarted				= 1;
	hInstance				= hInInstance;
	const TCHAR* CmdLine	= GetCommandLine();

#ifdef _DEBUG
	if ( true )
#else
	if( IsDebuggerPresent() )
#endif
	{
		// Don't use exception handling when a debugger is attached to exactly trap the crash.
		ErrorLevel = GuardedMain( CmdLine, hInInstance, hPrevInstance, nCmdShow );
	}
	else
	{
		// Use structured exception handling to trap any crashs, walk the the stack and display a crash dialog box.
		__try
		{
			GIsGuarded = 1;
			ErrorLevel = GuardedMain( CmdLine, hInInstance, hPrevInstance, nCmdShow );
			GIsGuarded = 0;
		}
		__except( CreateMiniDump( GetExceptionInformation() ) )
		{
			extern void appHideSplashEx();
			appHideSplashEx();

			// Crashed.
			ErrorLevel = 1;
			GError->HandleError();

			// 에러정보 E-Mail을 보내는 루틴.
			extern void SendMailWithDebugInfo();
			SendMailWithDebugInfo();
			StaticShutdownAfterError();

			// Shutdown sockets layer
			appSocketExit();
		}
	}

	// Final shut down.
	appExit();
	GIsStarted = 0;
	return ErrorLevel;
}

// Work around for VS.NET 2005 bug with 'link library dependencies' option.
//
// http://connect.microsoft.com/VisualStudio/feedback/ViewFeedback.aspx?FeedbackID=112962

#if _MSC_VER >= 1400 && !(defined XBOX)

#pragma comment(lib, "ALAudio.lib")
#pragma comment(lib, "Core.lib")
#pragma comment(lib, "D3DDrv.lib")
#pragma comment(lib, "Engine.lib")
#pragma comment(lib, "FXStudio.lib")
#pragma comment(lib, "GameFramework.lib")
#pragma comment(lib, "IpDrv.lib")
#pragma comment(lib, "WinDrv.lib")
#pragma comment(lib, "_AVAGame.lib")
#pragma comment(lib, "_AVANetDrv.lib")


#if !FINAL_RELEASE
#pragma comment(lib, "Editor.lib")
#pragma comment(lib, "UnrealEd.lib")
#endif

#endif // _MSC_VER >= 1400

/*-----------------------------------------------------------------------------
The End.
-----------------------------------------------------------------------------*/
