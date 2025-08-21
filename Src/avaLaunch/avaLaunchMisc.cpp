/**
* Copyright 2005 Epic Games, Inc. All Rights Reserved.
*/
#include "avaLaunch.h"
#include "Smtp.h"

#if !FINAL_RELEASE
#include "PropertyWindowManager.h"
#endif
//
// Minidump support/ exception handling.
//

#pragma pack(push,8)
#include <DbgHelp.h>
#pragma pack(pop)

TCHAR MiniDumpFilenameW[256] = TEXT("");
char  MiniDumpFilenameA[256] = "";			// We can't use alloca before writing out minidump so we avoid the TCHAR_TO_ANSI macro

void UnHeapCheck()
{
	GMalloc->HeapCheck();
}

INT CreateMiniDump( LPEXCEPTION_POINTERS ExceptionInfo )
{
#if !FINAL_RELEASE
	UnHeapCheck();
#endif

	// Try to create file for minidump.
	HANDLE FileHandle	= TCHAR_CALL_OS( 
		CreateFileW( MiniDumpFilenameW, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ), 
		CreateFileA( MiniDumpFilenameA, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ) 
		);

	// Write a minidump.
	if( FileHandle != INVALID_HANDLE_VALUE )
	{
		MINIDUMP_EXCEPTION_INFORMATION DumpExceptionInfo;

		DumpExceptionInfo.ThreadId			= GetCurrentThreadId();
		DumpExceptionInfo.ExceptionPointers	= ExceptionInfo;
		DumpExceptionInfo.ClientPointers	= true;

		MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), FileHandle, MiniDumpNormal, &DumpExceptionInfo, NULL, NULL );
		CloseHandle( FileHandle );
	}

	ANSICHAR* StackTrace = (ANSICHAR*) appSystemMalloc( 65535 );
	StackTrace[0] = 0;
	// Walk the stack and dump it to the allocated memory.
	appStackWalkAndDump( StackTrace, 0, ExceptionInfo->ContextRecord );
	appStrncat( GErrorHist, ANSI_TO_TCHAR(StackTrace), ARRAY_COUNT(GErrorHist) - 1 );
	appSystemFree( StackTrace );

	return EXCEPTION_EXECUTE_HANDLER;
}

//! 에러 정보를 e-mail로 보낸다.(2007/09/17 고광록)
void SendMailWithDebugInfo()
{
//#if !FINAL_RELEASE

	if ( !GErrorMsgRet )
		return ;

	CSmtp smtp;

	FString Key;
	FString USN;
	FString ID;
	FString	From;
	UBOOL	bSend = FALSE;

	TCHAR	ExeName[MAX_PATH];
	FString	FullName;
	FString	TitleName;

	GetModuleFileName(NULL, ExeName, ARRAY_COUNT(ExeName));
	FullName  = ExeName;
	TitleName = FullName.Right(FullName.Len() - FullName.InStr("\\", TRUE) - 1);

	// 실서버(ava.exe)가 아닌 경우에만 e-mail을 보낸다.
	if( _tcsicmp(*TitleName, TEXT("ava.exe")) == 0 )
		bSend = FALSE;
	else
		bSend = TRUE;

	// email 보내기.
	if ( smtp.Connect(TEXT("rara.redduck.com")) )
	{
		FString MiniDump = appBaseDir();
		MiniDump += MiniDumpFilenameW;

		// read id
		Parse(appCmdLine(),TEXT("-key"), Key);
		if (Key.Len() > 0)
		{
			// defence code for the incompleted key string
			INT Pos = Key.InStr(TEXT("</"));
			if (Pos >= 0)
				Key = Key.Left(Pos);

			FString HashIndex;
			FString StatIndex;
			if ( Key.Split(TEXT(" "), &HashIndex, &StatIndex) )
			{
				TArray<FString> KeyArray;
				INT Num = HashIndex.ParseIntoArray(&KeyArray, TEXT("|"), TRUE);

				if ( Num == 6 )
				{
					USN = *KeyArray(0);
					ID  = *KeyArray(1);
				}
			}

			From  = FString::Printf(TEXT("%s@avaServer"), *ID);
		}
		else if ( Parse(appCmdLine(),TEXT("-usn"), Key) )
		{
			Key.Split(TEXT("|"), &USN, &ID);
			From  = FString::Printf(TEXT("%s@sayclub.com"), *ID);
		}
		else
		{
			// key, usn 모두 찾지 못한 경우. (avaGame.exe인 경우라 판단해도 될까?)
			From  = FString::Printf(TEXT("%s@avaEditor"), *appComputerName());
		}

		TCHAR	To[]      = TEXT("deif@redduck.com");
		TCHAR*	CC[]      = { TEXT("oz99@redduck.com"), TEXT("cmheo@redduck.com"), TEXT("loozend@redduck.com"), TEXT("otterrrr@redduck.com"), NULL };
		TCHAR	Subject[] = TEXT("[A.V.A] Error Messages");

		if ( bSend )
		{
			CSmtpMessage msg;

			msg.Sender = (LPTSTR)*From;
			msg.Recipient = To;
			msg.Subject = Subject;
			msg.Message.AddItem( CSmtpMessageBody(GErrorHist) );

			// 참조...
			for ( int i = 0; CC[i] != NULL; ++i )
				msg.CC.AddItem( CSmtpAddress(CC[i]) );

			smtp.SendMessage(msg);

			// dump파일은 잠시 제거.(분당 30개, 시간당 1800개;;)
//			smtp.SendMessage(, To, Subject, GErrorHist, NULL, 0); //(LPVOID)*MiniDump, 0);
		}
	}
//#endif
}

void appShowSplashEx( const TCHAR* SplashName );
void appHideSplashEx();

//
//	WxLaunchApp implementation.
//

#if !FINAL_RELEASE

/**
* Gets called on initialization from within wxEntry.	
*/
bool WxLaunchApp::OnInit()
{
	wxApp::OnInit();

	FString SplashName;
	const TCHAR *IniKey = GIsEditor ? TEXT("SplashScreenEditor") : TEXT("SplashScreen");
//	if ( !GConfig || !GConfig->GetString( TEXT("Engine.Engine"), IniKey, SplashName, GEngineIni ) )
//	{
//		SplashName = GIsEditor ? TEXT("Splash\\EdSplash.bmp") : TEXT("Splash\\Splash.bmp");
//	}

	if ( GIsEditor )
	{
		if ( !GConfig || !GConfig->GetString( TEXT("Engine.Engine"), IniKey, SplashName, GEngineIni ) )
			SplashName = TEXT("Splash\\EdSplash.bmp");
	}
	else
	{
		FConfigFile *cfgFile = NULL;

		if ( !GConfig || (cfgFile = GConfig->FindConfigFile(GEngineIni)) != NULL )
		{
			const FConfigSection* cfgSection = cfgFile->Find( FString(TEXT("Engine.Engine")) );

			TArray<FString> SplashNames;
			cfgSection->MultiFind( FString(IniKey), SplashNames );

			int maxSplashs = SplashNames.Num() == 0 ? 1 : SplashNames.Num();
			int random = INT(timeGetTime()) % maxSplashs;

			SplashName = SplashNames( random );
		}

		if ( SplashName.Len() == 0 )
			SplashName = TEXT("Splash\\Splash.bmp");
	}	

	// Initialize XML resources
	wxXmlResource::Get()->InitAllHandlers();
	verify( wxXmlResource::Get()->Load( TEXT("wxRC/UnrealEd*.xrc") ) );

	INT ErrorLevel = GEngineLoop.Init();

	appHideSplashEx();

	if ( ErrorLevel )
	{		
		return 0;
	}

	// Init subsystems
	InitPropertySubSystem();
	

	return 1;
}

/** 
* Gets called after leaving main loop before wxWindows is shut down.
*/
int WxLaunchApp::OnExit()
{
	return wxApp::OnExit();
}

/**
* Performs any required cleanup in the case of a fatal error.
*/
void WxLaunchApp::ShutdownAfterError()
{
}

/**
* Callback from wxWindows main loop to signal that we should tick the engine.
*/
void WxLaunchApp::TickUnreal()
{
#if STATS
	GFPSCounter.Update(appSeconds());
#endif
	if( !GIsRequestingExit )
	{
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

/**
* The below is a manual expansion of wxWindows's IMPLEMENT_APP to allow multiple wxApps.
*
* @warning: when upgrading wxWindows, make sure that the below is how IMPLEMENT_APP looks like
*/
wxApp *wxCreateApp()
{
	wxApp::CheckBuildOptions(WX_BUILD_OPTIONS_SIGNATURE, "UnrealEngine3");
	//return GIsEditor ? new WxUnrealEdApp : new WxLaunchApp;
	return GIsEditor ? new WxUnrealEdApp : new WxLaunchApp;
}
wxAppInitializer wxTheAppInitializer((wxAppInitializerFunction) wxCreateApp);
WxLaunchApp& wxGetApp() { return *(WxLaunchApp *)wxTheApp; }

/*-----------------------------------------------------------------------------
The End.
-----------------------------------------------------------------------------*/

#endif