/*=============================================================================
	UnVcWin32.cpp: Visual C++ Windows 32-bit core.
	Copyright 1997-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma pack(push,8)
#include <stdio.h>
#include <float.h>
#include <time.h>
#include <io.h>
#include <direct.h>
#include <errno.h>
#include <sys/stat.h>
#pragma pack(pop)

// Core includes.
#include "CorePrivate.h"
#include "UnThreadingWindows.h"		// For getting thread context in the stack walker.
#include "../../IpDrv/Inc/UnIpDrv.h"		// For GDebugComm
#include "ChartCreation.h"

// Resource includes.
#include "../../avaLaunch/Resources/Resource.h"

#pragma pack(push,8)
#include <string.h>
#include <TlHelp32.h>				// For module info.
#include <DbgHelp.h>				// For stack walker.
#include <psapi.h>
#include <ShellAPI.h>
#include <MMSystem.h>

#include <rpcsal.h>
#include <gameux.h>					// For IGameExplorer
#include <shlobj.h>
#include <intshcut.h>


#include <Softpub.h>				// For authenticode
#include <wincrypt.h>
#include <wintrust.h>

#include "../../WinDrv/Inc/WinDrv.h"

#pragma pack(pop)

// Link with the Wintrust.lib file.
#pragma comment (lib, "wintrust")

/** Resource ID of icon to use for Window */
extern INT GGameIcon;

/** Handle for the initial game Window */
extern HWND GGameWindow;

/** Whether the game is using the startup window procedure */
extern UBOOL GGameWindowUsingStartupWindowProc;

/*----------------------------------------------------------------------------
	Misc functions.
----------------------------------------------------------------------------*/

#include "UnFile.h"

/**
 * Displays extended message box allowing for YesAll/NoAll
 * @return 3 - YesAll, 4 - NoAll, -1 for Fail
 */
int MessageBoxExt( EAppMsgType MsgType, HWND HandleWnd, const TCHAR* Text, const TCHAR* Caption );

/**
 * Callback for MessageBoxExt dialog (allowing for Yes to all / No to all and Cancel )
 * @return		One of ART_Yes, ART_Yesall, ART_No, ART_NoAll, ART_Cancel.
 */
BOOL CALLBACK MessageBoxDlgProc( HWND HandleWnd, UINT Message, WPARAM WParam, LPARAM LParam );

/*-----------------------------------------------------------------------------
	FOutputDeviceWindowsError.
-----------------------------------------------------------------------------*/

//
// Sends the message to the debugging output.
//
void appOutputDebugString( const TCHAR *Message )
{
	OutputDebugString( Message );
}

/** Sends a message to a remote tool. */
void appSendNotificationString( const ANSICHAR *Message )
{
	if ( GDebugComm )
	{
		GDebugComm->SendText( Message );
	}
	OutputDebugString( ANSI_TO_TCHAR(Message) );
}

//
// Immediate exit.
//
void appRequestExit( UBOOL Force )
{
	debugf( TEXT("appRequestExit(%i)"), Force );
	if( Force )
	{
		// Force immediate exit. Dangerous because config code isn't flushed, etc.
		ExitProcess( 1 );
	}
	else
	{
		// Tell the platform specific code we want to exit cleanly from the main loop.
		PostQuitMessage( 0 );
		GIsRequestingExit = 1;
	}
}

//
// Get system error.
//
const TCHAR* appGetSystemErrorMessage( INT Error )
{
	static TCHAR Msg[1024];
	*Msg = 0;
	if( Error==0 )
		Error = GetLastError();
#if UNICODE
	if( !GUnicodeOS )
	{
		ANSICHAR ACh[1024];
		FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM, NULL, Error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), ACh, 1024, NULL );
		appStrcpy( Msg, ANSI_TO_TCHAR(ACh) );
	}
	else
#endif
	{
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, Error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), Msg, 1024, NULL );
	}
	if( appStrchr(Msg,'\r') )
		*appStrchr(Msg,'\r')=0;
	if( appStrchr(Msg,'\n') )
		*appStrchr(Msg,'\n')=0;
	return Msg;
}

/*-----------------------------------------------------------------------------
	Clipboard.
-----------------------------------------------------------------------------*/

//
// Copy text to clipboard.
//
void appClipboardCopy( const TCHAR* Str )
{
	if( OpenClipboard(GetActiveWindow()) )
	{
		verify(EmptyClipboard());
#if UNICODE
		HGLOBAL GlobalMem;
		if( GUnicode && !GUnicodeOS )
		{
			INT Count = WideCharToMultiByte(CP_ACP,0,Str,-1,NULL,0,NULL,NULL);
			GlobalMem = GlobalAlloc( GMEM_DDESHARE | GMEM_MOVEABLE, Count+1 );
			check(GlobalMem);
			ANSICHAR* Data = (ANSICHAR*) GlobalLock( GlobalMem );
			WideCharToMultiByte(CP_ACP,0,Str,-1,Data,Count,NULL,NULL);
			Data[Count] = 0;
			GlobalUnlock( GlobalMem );
			if( SetClipboardData( CF_TEXT, GlobalMem ) == NULL )
				appErrorf(TEXT("SetClipboardData(A) failed with error code %i"), GetLastError() );
		}
		else
#endif
		{
			GlobalMem = GlobalAlloc( GMEM_DDESHARE | GMEM_MOVEABLE, sizeof(TCHAR)*(appStrlen(Str)+1) );
			check(GlobalMem);
			TCHAR* Data = (TCHAR*) GlobalLock( GlobalMem );
			appStrcpy( Data, Str );
			GlobalUnlock( GlobalMem );
			if( SetClipboardData( GUnicode ? CF_UNICODETEXT : CF_TEXT, GlobalMem ) == NULL )
				appErrorf(TEXT("SetClipboardData(%s) failed with error code %i"), GUnicode ? TEXT("W") : TEXT("A"), GetLastError() );
		}
		verify(CloseClipboard());
	}
}

//
// Paste text from clipboard.
//
FString appClipboardPaste()
{
	FString Result;
	if( OpenClipboard(GetActiveWindow()) )
	{
		HGLOBAL GlobalMem = NULL;
		UBOOL Unicode = 0;
		if( GUnicode && GUnicodeOS )
		{
			GlobalMem = GetClipboardData( CF_UNICODETEXT );
			Unicode = 1;
		}
		if( !GlobalMem )
		{
			GlobalMem = GetClipboardData( CF_TEXT );
			Unicode = 0;
		}
		if( !GlobalMem )
			Result = TEXT("");
		else
		{
			void* Data = GlobalLock( GlobalMem );
			check( Data );	
			if( Unicode )
				Result = (TCHAR*) Data;
			else
			{
				ANSICHAR* ACh = (ANSICHAR*) Data;
				INT i;
				for( i=0; ACh[i]; i++ );
				TArray<TCHAR> Ch(i+1);
				for( i=0; i<Ch.Num(); i++ )
					Ch(i)=FromAnsi(ACh[i]);
				Result = &Ch(0);
			}
			GlobalUnlock( GlobalMem );
		}
		verify(CloseClipboard());
	}
	else Result=TEXT("");

	return Result;
}

/*-----------------------------------------------------------------------------
	DLLs.
-----------------------------------------------------------------------------*/

void* appGetDllHandle( const TCHAR* Filename )
{
	check(Filename);	
	return TCHAR_CALL_OS(LoadLibraryW(Filename),LoadLibraryA(TCHAR_TO_ANSI(Filename)));
}

//
// Free a DLL.
//
void appFreeDllHandle( void* DllHandle )
{
	check(DllHandle);
	FreeLibrary( (HMODULE)DllHandle );
}

//
// Lookup the address of a DLL function.
//
void* appGetDllExport( void* DllHandle, const TCHAR* ProcName )
{
	check(DllHandle);
	check(ProcName);
	return (void*)GetProcAddress( (HMODULE)DllHandle, TCHAR_TO_ANSI(ProcName) );
}

/*-----------------------------------------------------------------------------
	Formatted printing and messages.
-----------------------------------------------------------------------------*/

INT appGetVarArgs( TCHAR* Dest, INT Count, const TCHAR*& Fmt, va_list ArgPtr )
{
	INT Result = VSNPRINTF( Dest, Count, Fmt, ArgPtr );
	va_end( ArgPtr );
	return Result;
}
INT appGetVarArgsAnsi( ANSICHAR* Dest, INT Count, const ANSICHAR*& Fmt, va_list ArgPtr)
{
	INT Result = VSNPRINTFA( Dest, Count, Fmt, ArgPtr );
	va_end( ArgPtr );
	return Result;
}

void appDebugMessagef( const TCHAR* Fmt, ... )
{
	TCHAR TempStr[4096]=TEXT("");
	GET_VARARGS( TempStr, ARRAY_COUNT(TempStr), Fmt, Fmt );
	if( GIsUnattended == TRUE )
	{
		debugf(TempStr);
	}
	else
	{
		MessageBox(NULL, TempStr, TEXT("appDebugMessagef"),MB_OK|MB_SYSTEMMODAL);
	}
}

VARARG_BODY( UBOOL, appMsgf, const TCHAR*, VARARG_EXTRA(EAppMsgType Type) )
{
	TCHAR TempStr[16384]=TEXT("");
	GET_VARARGS( TempStr, ARRAY_COUNT(TempStr), Fmt, Fmt );
	if( GIsUnattended == TRUE )
	{
		if (GWarn)
		{
			warnf(TempStr);
		}

		switch(Type)
		{
		case AMT_YesNo:
			return 0; // No
		case AMT_OKCancel:
			return 1; // Cancel
		case AMT_YesNoCancel:
			return 2; // Cancel
		case AMT_CancelRetryContinue:
			return 0; // Cancel
		case AMT_YesNoYesAllNoAll:
			return ART_No; // No
		default:
			return 1;
		}
	}
	else
	{
		HWND ParentWindow = GWarn ? (HWND)GWarn->hWndEditorFrame : (HWND)NULL;
		switch( Type )
		{
			case AMT_YesNo:
				return MessageBox( ParentWindow, TempStr, TEXT("Message"), MB_YESNO|MB_SYSTEMMODAL ) == IDYES;
				break;
			case AMT_OKCancel:
				return MessageBox( ParentWindow, TempStr, TEXT("Message"), MB_OKCANCEL|MB_SYSTEMMODAL ) == IDOK;
				break;
			case AMT_YesNoCancel:
				{
					INT Return = MessageBox(ParentWindow, TempStr, TEXT("Message"), MB_YESNOCANCEL | MB_ICONQUESTION | MB_SYSTEMMODAL);
					// return 0 for Yes, 1 for No, 2 for Cancel
					return Return == IDYES ? 0 : (Return == IDNO ? 1 : 2);
				}
			case AMT_CancelRetryContinue:
				{
					INT Return = MessageBox(ParentWindow, TempStr, TEXT("Message"), MB_CANCELTRYCONTINUE | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_SYSTEMMODAL);
					// return 0 for Cancel, 1 for Retry, 2 for Continue
					return Return == IDCANCEL ? 0 : (Return == IDTRYAGAIN ? 1 : 2);
				}
				break;
			case AMT_YesNoYesAllNoAll:
				return MessageBoxExt( AMT_YesNoYesAllNoAll, ParentWindow, TempStr, TEXT("Message") );
				// return 0 for No, 1 for Yes, 2 for YesToAll, 3 for NoToAll
				break;
			case AMT_YesNoYesAllNoAllCancel:
				return MessageBoxExt( AMT_YesNoYesAllNoAllCancel, ParentWindow, TempStr, TEXT("Message") );
				// return 0 for No, 1 for Yes, 2 for YesToAll, 3 for NoToAll, 4 for Cancel
				break;
			default:
				MessageBox( ParentWindow, TempStr, TEXT("Message"), MB_OK|MB_SYSTEMMODAL );
				break;
		}
	}
	return 1;
}

void appGetLastError( void )
{
	TCHAR TempStr[4096]=TEXT("");
	appSprintf( TempStr, TEXT("GetLastError : %d\n\n%s"), GetLastError(), appGetSystemErrorMessage() );
	if( GIsUnattended == TRUE )
	{
		appErrorf(TempStr);
	}
	else
	{
		MessageBox( NULL, TempStr, TEXT("System Error"), MB_OK|MB_SYSTEMMODAL );
	}
}

// Interface for recording loading errors in the editor
void EdClearLoadErrors()
{
	GEdLoadErrors.Empty();
}

VARARG_BODY( void VARARGS, EdLoadErrorf, const TCHAR*, VARARG_EXTRA(INT Type) )
{
	TCHAR TempStr[4096]=TEXT("");
	GET_VARARGS( TempStr, ARRAY_COUNT(TempStr), Fmt, Fmt );

	// Check to see if this error already exists ... if so, don't add it.
	// NOTE : for some reason, I can't use AddUniqueItem here or it crashes
	for( INT x = 0 ; x < GEdLoadErrors.Num() ; ++x )
		if( GEdLoadErrors(x) == FEdLoadError( Type, TempStr ) )
			return;

	new( GEdLoadErrors )FEdLoadError( Type, TempStr );
}


/*-----------------------------------------------------------------------------
	Timing.
-----------------------------------------------------------------------------*/

//
// Sleep this thread for Seconds, 0.0 means release the current
// timeslice to let other threads get some attention.
//
void appSleep( FLOAT Seconds )
{
	Sleep( (DWORD)(Seconds * 1000.0) );		
}

/**
 * Sleeps forever. This function does not return!
 */
void appSleepInfinite()
{
	Sleep(INFINITE);
}

//
// Return the system time.
//
void appSystemTime( INT& Year, INT& Month, INT& DayOfWeek, INT& Day, INT& Hour, INT& Min, INT& Sec, INT& MSec )
{
	SYSTEMTIME st;
	GetLocalTime( &st );

	Year		= st.wYear;
	Month		= st.wMonth;
	DayOfWeek	= st.wDayOfWeek;
	Day			= st.wDay;
	Hour		= st.wHour;
	Min			= st.wMinute;
	Sec			= st.wSecond;
	MSec		= st.wMilliseconds;
}

/**
* Returns a string with a unique timestamp (useful for creating log filenames)
*/
FString appSystemTimeString( void )
{
	// Create string with system time to create a unique filename.
	INT Year, Month, DayOfWeek, Day, Hour, Min, Sec, MSec;

	appSystemTime( Year, Month, DayOfWeek, Day, Hour, Min, Sec, MSec );
	FString	CurrentTime = FString::Printf( TEXT( "%i.%02i.%02i-%02i.%02i.%02i" ), Year, Month, Day, Hour, Min, Sec );

	return( CurrentTime );
}

/*-----------------------------------------------------------------------------
	Link functions.
-----------------------------------------------------------------------------*/

//
// Launch a uniform resource locator (i.e. http://www.epicgames.com/unreal).
// This is expected to return immediately as the URL is launched by another
// task.
//
void appLaunchURL( const TCHAR* URL, const TCHAR* Parms, FString* Error )
{
	debugf( NAME_Log, TEXT("LaunchURL %s %s"), URL, Parms?Parms:TEXT("") );
	INT Code = (INT)TCHAR_CALL_OS(ShellExecuteW(NULL,TEXT("open"),URL,Parms?Parms:TEXT(""),TEXT(""),SW_SHOWNORMAL),ShellExecuteA(NULL,"open",TCHAR_TO_ANSI(URL),Parms?TCHAR_TO_ANSI(Parms):"","",SW_SHOWNORMAL));
	if( Error )
		*Error = Code<=32 ? LocalizeError(TEXT("UrlFailed"),TEXT("Core")) : TEXT("");
}

//
// Creates a new process and its primary thread. The new process runs the
// specified executable file in the security context of the calling process.
//
void *appCreateProc( const TCHAR* URL, const TCHAR* Parms )
{
	debugf( NAME_Log, TEXT("CreateProc %s %s"), URL, Parms );

	TCHAR CommandLine[1024];
	appSprintf( CommandLine, TEXT("%s %s"), URL, Parms );

	PROCESS_INFORMATION ProcInfo;
	SECURITY_ATTRIBUTES Attr;
	Attr.nLength = sizeof(SECURITY_ATTRIBUTES);
	Attr.lpSecurityDescriptor = NULL;
	Attr.bInheritHandle = TRUE;

#if UNICODE
	if( GUnicode && !GUnicodeOS )
	{
		STARTUPINFOA StartupInfoA = { sizeof(STARTUPINFO), NULL, NULL, NULL,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			NULL, NULL, NULL, NULL, SW_HIDE, NULL, NULL,
			NULL, NULL, NULL };
		if( !CreateProcessA( NULL, TCHAR_TO_ANSI(CommandLine), &Attr, &Attr, TRUE, DETACHED_PROCESS | REALTIME_PRIORITY_CLASS,
			NULL, NULL, &StartupInfoA, &ProcInfo ) )
			return NULL;
	}
	else
#endif
	{
		STARTUPINFO StartupInfo = { sizeof(STARTUPINFO), NULL, NULL, NULL,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			NULL, NULL, NULL, NULL, SW_HIDE, NULL, NULL,
			NULL, NULL, NULL };
		if( !CreateProcess( NULL, CommandLine, &Attr, &Attr, TRUE, DETACHED_PROCESS | REALTIME_PRIORITY_CLASS,
			NULL, NULL, &StartupInfo, &ProcInfo ) )
			return NULL;
	}
	return (void*)ProcInfo.hProcess;
}

//
// Retrieves the termination status of the specified process.
//
UBOOL appGetProcReturnCode( void* ProcHandle, INT* ReturnCode )
{
	return GetExitCodeProcess( (HANDLE)ProcHandle, (DWORD*)ReturnCode ) && *((DWORD*)ReturnCode) != STILL_ACTIVE;
}

/*-----------------------------------------------------------------------------
	File finding.
-----------------------------------------------------------------------------*/

//
// Deletes 1) all temporary files; 2) all cache files that are no longer wanted.
//
void appCleanFileCache()
{
	// do standard cache cleanup
	GSys->PerformPeriodicCacheCleanup();

	// perform any other platform-specific cleanup here
}

/*-----------------------------------------------------------------------------
	Guids.
-----------------------------------------------------------------------------*/

//
// Create a new globally unique identifier.
//
FGuid appCreateGuid()
{
	FGuid Result(0,0,0,0);
	verify( CoCreateGuid( (GUID*)&Result )==S_OK );
	return Result;
}

/*-----------------------------------------------------------------------------
	Command line.
-----------------------------------------------------------------------------*/

// Get startup directory.  NOTE: Only one return value is valid at a time!
const TCHAR* appBaseDir()
{
	static TCHAR Result[512]=TEXT("");
	if( !Result[0] )
	{
		// Get directory this executable was launched from.
#if UNICODE
		if( GUnicode && !GUnicodeOS )
		{
			ANSICHAR ACh[256];
			GetModuleFileNameA( hInstance, ACh, ARRAY_COUNT(ACh) );
			MultiByteToWideChar( CP_ACP, 0, ACh, -1, Result, ARRAY_COUNT(Result) );
		}
		else
#endif
		{
			GetModuleFileName( hInstance, Result, ARRAY_COUNT(Result) );
		}
		INT i;
		for( i=appStrlen(Result)-1; i>0; i-- )
			if( Result[i-1]==PATH_SEPARATOR[0] || Result[i-1]=='/' )
				break;
		Result[i]=0;
	}
	return Result;
}

// Get computer name.  NOTE: Only one return value is valid at a time!
const TCHAR* appComputerName()
{
	static TCHAR Result[256]=TEXT("");
	if( !Result[0] )
	{
		DWORD Size=ARRAY_COUNT(Result);
#if UNICODE
		if( GUnicode && !GUnicodeOS )
		{
			ANSICHAR ACh[ARRAY_COUNT(Result)];
			GetComputerNameA( ACh, &Size );
			MultiByteToWideChar( CP_ACP, 0, ACh, -1, Result, ARRAY_COUNT(Result) );
		}
		else
#endif
		{
			GetComputerName( Result, &Size );
		}
		TCHAR *c, *d;
		for( c=Result, d=Result; *c!=0; c++ )
			if( appIsAlnum(*c) )
				*d++ = *c;
		*d++ = 0;
	}
	return Result;
}

// Get user name.  NOTE: Only one return value is valid at a time!
const TCHAR* appUserName()
{
	static TCHAR Result[256]=TEXT("");
	if( !Result[0] )
	{
		DWORD Size=ARRAY_COUNT(Result);
#if UNICODE
		if( GUnicode && !GUnicodeOS )
		{
			ANSICHAR ACh[ARRAY_COUNT(Result)];
			GetUserNameA( ACh, &Size );
			MultiByteToWideChar( CP_ACP, 0, ACh, -1, Result, ARRAY_COUNT(Result) );
		}
		else
#endif
		{
			GetUserName( Result, &Size );
		}
		TCHAR *c, *d;
		for( c=Result, d=Result; *c!=0; c++ )
			if( appIsAlnum(*c) )
				*d++ = *c;
		*d++ = 0;
	}
	return Result;
}

// shader dir relative to appBaseDir
const TCHAR* appShaderDir()
{
	static TCHAR Result[256] = TEXT("..") PATH_SEPARATOR TEXT("Engine") PATH_SEPARATOR TEXT("Shaders");
	return Result;
}

/**
 * Retrieve a environment variable from the system
 *
 * @param VariableName The name of the variable (ie "Path")
 * @param Result The string to copy the value of the variable into
 * @param ResultLength The size of the Result string
 */
void appGetEnvironmentVariable(const TCHAR* VariableName, TCHAR* Result, INT ResultLength)
{
	DWORD Error = GetEnvironmentVariable(VariableName, Result, ResultLength * sizeof(TCHAR));
	if (Error <= 0)
	{
		// on error, just return an empty string
		*Result = 0;
	}
}

/*-----------------------------------------------------------------------------
	App init/exit.
-----------------------------------------------------------------------------*/

//
// Platform specific initialization.
//
static void DoCPUID( int i, DWORD *A, DWORD *B, DWORD *C, DWORD *D )
{
#if ASM_X86
 	__asm
	{			
		mov eax,[i]
		_emit 0x0f
		_emit 0xa2

		mov edi,[A]
		mov [edi],eax

		mov edi,[B]
		mov [edi],ebx

		mov edi,[C]
		mov [edi],ecx

		mov edi,[D]
		mov [edi],edx

		mov eax,0
		mov ebx,0
		mov ecx,0
		mov edx,0
		mov esi,0
		mov edi,0
	}
#else
	*A=*B=*C=*D=0;
#endif
}

/** Original C- Runtime pure virtual call handler that is being called in the (highly likely) case of a double fault */
_purecall_handler DefaultPureCallHandler;

/**
 * Our own pure virtual function call handler, set by appPlatformPreInit. Falls back
 * to using the default C- Runtime handler in case of double faulting.
 */
static void PureCallHandler()
{
	static INT AlreadyCalled = 0;
	appDebugBreak();
	if( AlreadyCalled )
	{
		// Call system handler if we're double faulting.
		if( DefaultPureCallHandler )
			DefaultPureCallHandler();
	}
	else
	{
		AlreadyCalled = 1;
		if( GIsRunning )
		{
			appMsgf( AMT_OK, TEXT("Pure virtual function being called while application was running (GIsRunning == 1).") );
		}
		appErrorf(TEXT("Pure virtual function being called") );
	}
}

/** Does PC specific initialization of timing information */
void appInitTiming(void)
{
	LARGE_INTEGER Frequency;
	verify( QueryPerformanceFrequency(&Frequency) );
	GSecondsPerCycle = 1.0 / Frequency.QuadPart;
	GStartTime = appSeconds();
}


/**
 * Verifies the certificate in a single file
 *
 * @param Filename Name of file to verify (.exe, .dll, .cab, etc)
 * @param TrustData Verification settings
 * @param Policy Verification policy
 * 
 * @return FALSE if the verification failed
 */
UBOOL VerifyAuthenticodeCertificateInFile(const TCHAR* Filename, WINTRUST_DATA* TrustData, GUID* Policy)
{
	// set the filename to use
	TrustData->pFile->pcwszFilePath = Filename;

	// return FALSE on ANY verification error (user never needs to know what failed)
	// for debugging problems, you may want to save the return value and print out info
	return WinVerifyTrust(NULL, Policy, TrustData) == ERROR_SUCCESS;
}

/**
 * Verifies all needed authenticode certificates.
 *
 * @param AppPath Pathname to the executable which should always be checked
 * @param FailedFile This will hold the of the file that failed (only set on a failure)
 *
 * @returns FALSE if verification failed for any file and the game should not continue
 */
UBOOL VerifyAuthenticodeCertificates(const TCHAR* AppPath, FString& FailedFile)
{
	// pretty standard verification policy
	GUID WVTPolicyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
	
	// initialize a trust data structure
	WINTRUST_DATA WinTrustData;
	appMemzero(&WinTrustData, sizeof(WinTrustData));
	WinTrustData.cbStruct = sizeof(WinTrustData);

	// disable UI (set to other values to show "Do you want trust this publisher?" standard UI)
	WinTrustData.dwUIChoice = WTD_UI_NONE;

	// no revocation checking
	WinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE; 

	// cause an error if the certificate is missing from the file
	WinTrustData.dwProvFlags = WTD_SAFER_FLAG;

	// if we were showing a UI, this sets the UI mode (execute message instead of install message)
	WinTrustData.dwUIContext = WTD_UICONTEXT_EXECUTE;

	// we are checking a file
	WinTrustData.dwUnionChoice = WTD_CHOICE_FILE;

	// make an empty file info structure (used to tell WinVerifyTrust about the file with the 
	// embedded certificate)
	WINTRUST_FILE_INFO FileData;
	appMemzero(&FileData, sizeof(FileData));
	FileData.cbStruct = sizeof(WINTRUST_FILE_INFO);

	// hook up the file data
	WinTrustData.pFile = &FileData;

	// verify all DLLs and the running exe
	TArray<FString> FilesToVerify;
	FilesToVerify.AddItem(AppPath);
	GFileManager->FindFiles(FilesToVerify, TEXT("*.dll"), TRUE, FALSE);

	// verify all desired files
	for (INT FileIndex = 0; FileIndex < FilesToVerify.Num(); FileIndex++)
	{
		if (VerifyAuthenticodeCertificateInFile(*FilesToVerify(FileIndex), &WinTrustData, &WVTPolicyGUID) == FALSE)
		{
			FailedFile = FilesToVerify(FileIndex);
			return FALSE;
		}
	}

	return TRUE;
}

/**
 * Handles Game Explorer operations (installing/uninstalling for testing, checking parental controls, etc)
 *
 * @returns FALSE if the game cannot continue.
 */
UBOOL HandleGameExplorerIntegration()
{
	TCHAR AppPath[MAX_PATH];
	GetModuleFileName(NULL, AppPath, MAX_PATH - 1);

	// for shipping PC game, always use verification, for development, base it on a commandline param
#if !SHIPPING_PC_GAME
	if (ParseParam( appCmdLine(), TEXT("auth")))
#endif
	{
		FString FailedFile;
		if (VerifyAuthenticodeCertificates(AppPath, FailedFile) == FALSE)
		{
			appMsgf(AMT_OK, TEXT("Modified executable code [%s] is not allowed. Goodbye!\n(Handle this better, as this string makes it easy to find code to hack around :) )"), *FailedFile);

			// @todo: Exit more cleanly then this :)
			exit(1);
			return FALSE;
		}
	}

	// check to make sure we are able to run, based on parental rights
	CoInitialize(NULL);

	IGameExplorer* GameExp;
	HRESULT hr = CoCreateInstance(__uuidof(GameExplorer), NULL, CLSCTX_INPROC_SERVER, __uuidof(IGameExplorer), (void**) &GameExp);

	BOOL bHasAccess = 1;
	BSTR AppPathBSTR = SysAllocString(AppPath);

	// @todo: This will allow access if the CoCreateInstance fails, but it should probaly disallow 
	// access if OS is Vista and it fails, succeed for XP
	if (SUCCEEDED(hr) && GameExp)
	{
		GameExp->VerifyAccess(AppPathBSTR, &bHasAccess);
	}

	// Guid for testing GE (un)installation
	static const GUID TestGEGuid = 
	{ 0x7851c25b, 0x8a90, 0x4771, { 0xa0, 0x8d, 0x24, 0xe5, 0x61, 0x2c, 0x5a, 0xc3 } };

	// add the game to the game explorer if desired
	if (ParseParam( appCmdLine(), TEXT("installge")))
	{
		if (bHasAccess && GameExp)
		{
			BSTR AppDirBSTR = SysAllocString(appBaseDir());
			GUID Guid = TestGEGuid;
			hr = GameExp->AddGame(AppPathBSTR, AppDirBSTR, GIS_CURRENT_USER, &Guid);

			UBOOL bWasSuccessful = FALSE;
			// if successful
			if (SUCCEEDED(hr))
			{
				// get location of app local dir
				TCHAR UserPath[MAX_PATH];
				SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, UserPath);

				// convert guid to a string
				TCHAR GuidDir[MAX_PATH];
				StringFromGUID2(TestGEGuid, GuidDir, MAX_PATH - 1);

				// make the base path for all tasks
				FString BaseTaskDirectory = FString(UserPath) + TEXT("\\Microsoft\\Windows\\GameExplorer\\") + GuidDir;

				// make full paths for play and support tasks
				FString PlayTaskDirectory = BaseTaskDirectory + TEXT("\\PlayTasks");
				FString SupportTaskDirectory = BaseTaskDirectory + TEXT("\\SupportTasks");
				
				// make sure they exist
				GFileManager->MakeDirectory(*PlayTaskDirectory, TRUE);
				GFileManager->MakeDirectory(*SupportTaskDirectory, TRUE);

				// interface for creating a shortcut
				IShellLink* Link;
				hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,	IID_IShellLink, (void**)&Link);

				// get the persistent file interface of the link
				IPersistFile* LinkFile;
				Link->QueryInterface(IID_IPersistFile, (void**)&LinkFile);

				Link->SetPath(AppPath);

				// create all of our tasks

				// first is just the game
				Link->SetArguments(TEXT("-installed"));
				Link->SetDescription(TEXT("Play"));
				GFileManager->MakeDirectory(*(PlayTaskDirectory + TEXT("\\0")), TRUE);
				LinkFile->Save(*(PlayTaskDirectory + TEXT("\\0\\Play.lnk")), TRUE);

				Link->SetArguments(TEXT("-installed -seekfreeloading"));
				Link->SetDescription(TEXT("Play SeekFree"));
				GFileManager->MakeDirectory(*(PlayTaskDirectory + TEXT("\\1")), TRUE);
				LinkFile->Save(*(PlayTaskDirectory + TEXT("\\1\\PlaySeekFree.lnk")), TRUE);

				Link->SetArguments(TEXT("make -installed"));
				Link->SetDescription(TEXT("Compile Script Code"));
				GFileManager->MakeDirectory(*(PlayTaskDirectory + TEXT("\\2")), TRUE);
				LinkFile->Save(*(PlayTaskDirectory + TEXT("\\2\\Compile.lnk")), TRUE);

				Link->SetArguments(TEXT("editor -installed"));
				Link->SetDescription(TEXT("Editor"));
				GFileManager->MakeDirectory(*(PlayTaskDirectory + TEXT("\\3")), TRUE);
				LinkFile->Save(*(PlayTaskDirectory + TEXT("\\3\\Editor.lnk")), TRUE);

				Link->SetArguments(TEXT("-uninstallge -installed"));
				Link->SetDescription(TEXT("Uninstall from Game Explorer"));
				GFileManager->MakeDirectory(*(PlayTaskDirectory + TEXT("\\4")), TRUE);
				LinkFile->Save(*(PlayTaskDirectory + TEXT("\\4\\Uninstall.lnk")), TRUE);

				LinkFile->Release();
				Link->Release();

				IUniformResourceLocator* InternetLink;
				CoCreateInstance (CLSID_InternetShortcut, NULL, 
					CLSCTX_INPROC_SERVER, IID_IUniformResourceLocator, (LPVOID*) &InternetLink);

				InternetLink->QueryInterface(IID_IPersistFile, (void**)&LinkFile);

				// make an internet shortcut
				InternetLink->SetURL(TEXT("http://www.unrealtournament3.com/"), 0);
				GFileManager->MakeDirectory(*(SupportTaskDirectory + TEXT("\\0")), TRUE);
				LinkFile->Save(*(SupportTaskDirectory + TEXT("\\0\\UT3.url")), TRUE);

				LinkFile->Release();
				InternetLink->Release();
			}
			appMsgf(AMT_OK, TEXT("GameExplorer installation was %s, quitting now."), SUCCEEDED(hr) ? TEXT("successful") : TEXT("a failure"));

			SysFreeString(AppDirBSTR);
		}
		else
		{
			appMsgf(AMT_OK, TEXT("GameExplorer installation failed because you don't have access (check parental control levels and that you are running XP). You should not need Admin access"));
		}

		// @todo: Exit more cleanly then this :)
		exit(1);
	}
	else if (ParseParam( appCmdLine(), TEXT("uninstallge")))
	{
		if (GameExp)
		{
			hr = GameExp->RemoveGame(TestGEGuid);
			appMsgf(AMT_OK, TEXT("GameExplorer uninstallation was %s, quitting now."), SUCCEEDED(hr) ? TEXT("successful") : TEXT("a failure"));
		}
		else
		{
			appMsgf(AMT_OK, TEXT("GameExplorer uninstallation failed because you are probably not running Vista."));
		}

		// @todo: Exit more cleanly then this :)
		exit(1);
	}

	// free the string and shutdown COM
	SysFreeString(AppPathBSTR);
	CoUninitialize();

	// if we don't have access, we must quit ASAP after showing a message
	if (!bHasAccess)
	{
		appMsgf(AMT_OK, *LocalizeError(TEXT("Error_ParentalControls"), TEXT("Launch")));
		// @todo: Exit more cleanly then this :)
		exit(1);

		return FALSE;
	}

	return TRUE;
}

void appPlatformPreInit()
{
	// Check Windows version.
	OSVERSIONINFO Version;
	Version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&Version);
	GUnicodeOS = Version.dwPlatformId==VER_PLATFORM_WIN32_NT;

	DefaultPureCallHandler = _set_purecall_handler( PureCallHandler );
}

/**
* Temporary window procedure for the game window during startup.
* It gets replaced later on with SetWindowLong().
*/
LRESULT CALLBACK StartupWindowProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
		// Prevent power management
	case WM_POWERBROADCAST:
		{
			switch( wParam )
			{
			case PBT_APMQUERYSUSPEND:
			case PBT_APMQUERYSTANDBY:
				return BROADCAST_QUERY_DENY;
			}
		}
	}

	return DefWindowProc(hWnd, Message, wParam, lParam);
}

void appPlatformInit()
{
	// Randomize.
    if( GIsBenchmarking )
	{
		srand( 0 );
	}
    else
	{
		srand( (unsigned)time( NULL ) );
	}

	// Set granularity of sleep and such to 1 ms.
	timeBeginPeriod( 1 );

	// Identity.
	debugf( NAME_Init, TEXT("Computer: %s"), appComputerName() );
	debugf( NAME_Init, TEXT("User: %s"), appUserName() );

	// Get CPU info.
	SYSTEM_INFO SI;
	GetSystemInfo(&SI);
	debugf( NAME_Init, TEXT("CPU Page size=%i, Processors=%i"), SI.dwPageSize, SI.dwNumberOfProcessors );
	GNumHardwareThreads = SI.dwNumberOfProcessors;

#if ASM_X86
	// Check processor version with CPUID.
	DWORD A=0, B=0, C=0, D=0;
	DoCPUID(0,&A,&B,&C,&D);
	TCHAR Brand[13];
	Brand[ 0] = (ANSICHAR)(B);
	Brand[ 1] = (ANSICHAR)(B>>8);
	Brand[ 2] = (ANSICHAR)(B>>16);
	Brand[ 3] = (ANSICHAR)(B>>24);
	Brand[ 4] = (ANSICHAR)(D);
	Brand[ 5] = (ANSICHAR)(D>>8);
	Brand[ 6] = (ANSICHAR)(D>>16);
	Brand[ 7] = (ANSICHAR)(D>>24);
	Brand[ 8] = (ANSICHAR)(C);
	Brand[ 9] = (ANSICHAR)(C>>8);
	Brand[10] = (ANSICHAR)(C>>16);
	Brand[11] = (ANSICHAR)(C>>24);
	Brand[12] = (ANSICHAR)(0);
	DoCPUID( 1, &A, &B, &C, &D );
	if( !(D & 0x02000000) )
	{
		appErrorf(TEXT("Engine requires processor with SSE support"));
	}
	// Print features.
	debugf( NAME_Init, TEXT("CPU Detected: %s"), Brand );
#endif

	// Timer resolution.
	debugf( NAME_Init, TEXT("High frequency timer resolution =%f MHz"), 0.000001 / GSecondsPerCycle );

	// Get memory.
	MEMORYSTATUS M;
	GlobalMemoryStatus(&M);
	debugf( NAME_Init, TEXT("Memory total: Phys=%iK Pagef=%iK Virt=%iK"), M.dwTotalPhys/1024, M.dwTotalPageFile/1024, M.dwTotalVirtual/1024 );

#if STATS
	// set our max physical memory available
	GStatManager.SetAvailableMemory(MCR_Physical, M.dwTotalPhys);
#endif

	// allow for game explorer processing (including parental controls)
	HandleGameExplorerIntegration();
}

void appPlatformPostInit()
{
	// 윈도우 키 등을 막는 함수.(from _AVA Launch/WindowsKeyBlocker.cpp)
	extern void PlatformPostInit();
	PlatformPostInit();

	if ( GIsGame )
	{
		// Register the window class
		FString WindowClassName = FString(GPackage) + TEXT("Unreal") + TEXT("UWindowsClient");
#if UNICODE
		if( GUnicodeOS )
		{
			WNDCLASSEXW Cls;
			appMemzero( &Cls, sizeof(Cls) );
			Cls.cbSize			= sizeof(Cls);
			// disable dbl-click messages in the game as the dbl-click event is sent instead of the key pressed event, which causes issues with e.g. rapid firing
			Cls.style			= GIsGame ? (CS_OWNDC) : (CS_DBLCLKS|CS_OWNDC);
			Cls.lpfnWndProc		= StartupWindowProc;
			Cls.hInstance		= hInstance;
			Cls.hIcon			= LoadIconIdX(hInstance,GGameIcon);
			Cls.lpszClassName	= *WindowClassName;
			Cls.hIconSm			= LoadIconIdX(hInstance,GGameIcon);
			verify(RegisterClassExW( &Cls ));
		}
		else
#endif
		{
			extern ANSICHAR* appStrncpyANSI( ANSICHAR* Dest, const ANSICHAR* Src, INT MaxLen );

			WNDCLASSEXA Cls;
			// dupe the memory for the string since we can't hold a pointer to the result of a TCHAR_TO_ANSI
			INT WindowClassNameLen = appStrlen(*WindowClassName);
			ANSICHAR* WindowClassNameAnsi = appStrncpyANSI( 
				(ANSICHAR*)appMalloc((WindowClassNameLen+1)*sizeof(ANSICHAR)), 
				TCHAR_TO_ANSI(*WindowClassName), 
				WindowClassNameLen+1 );
			appMemzero( &Cls, sizeof(Cls) );
			Cls.cbSize			= sizeof(Cls);
			// disable dbl-click messages in the game as the dbl-click event is sent instead of the key pressed event, which causes issues with e.g. rapid firing
			Cls.style			= GIsGame ? (CS_OWNDC) : (CS_DBLCLKS|CS_OWNDC);
			Cls.lpfnWndProc		= StartupWindowProc;
			Cls.hInstance		= hInstance;
			Cls.hIcon			= LoadIconIdX(hInstance,GGameIcon);
			Cls.lpszClassName	= WindowClassNameAnsi;
			Cls.hIconSm			= LoadIconIdX(hInstance,GGameIcon);
			verify(RegisterClassExA( &Cls ));
			free(WindowClassNameAnsi);
		}

		// Create a minimized game window
		DWORD WindowStyle;
		WindowStyle = WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_BORDER | WS_CAPTION;
		// Obtain width and height of primary monitor.
		INT ScreenWidth  = ::GetSystemMetrics( SM_CXSCREEN );
		INT ScreenHeight = ::GetSystemMetrics( SM_CYSCREEN );
		INT WindowWidth  = ScreenWidth / 2;
		INT WindowHeight = ScreenHeight / 2;
		INT WindowPosX = (ScreenWidth - WindowWidth ) / 2;
		INT WindowPosY = (ScreenHeight - WindowHeight ) / 2;

		FString Name = *LocalizeGeneral("Product",GPackage);	//TEXT("UnrealEngine3 Startup");
		GGameWindowUsingStartupWindowProc = TRUE;

		// Create the window
		GGameWindow = CreateWindowExX( WS_EX_APPWINDOW, *WindowClassName, *Name, WindowStyle, WindowPosX, WindowPosY, WindowWidth, WindowHeight, NULL, NULL, hInstance, NULL );
		verify( GGameWindow );
		ShowWindow( GGameWindow, SW_SHOWMINIMIZED );
	}
}

/**
*	Pumps Windows messages.
*/
void appWinPumpMessages()
{
	MSG Msg;
	while( PeekMessage(&Msg,NULL,0,0,PM_REMOVE) )
	{
		TranslateMessage( &Msg );
		DispatchMessage( &Msg );
	}
}

/*
*	Shows the intial game window in the proper position and size.
*	It also changes the window proc from StartupWindowProc to
*	UWindowsClient::StaticWndProc.
*	This function doesn't have any effect if called a second time.
*/
void appShowGameWindow()
{
	if ( GGameWindow && GGameWindowUsingStartupWindowProc )
	{
		extern DWORD GGameWindowStyle;
		extern INT GGameWindowPosX;
		extern INT GGameWindowPosY;
		extern INT GGameWindowWidth;
		extern INT GGameWindowHeight;

		// Convert position from screen coordinates to workspace coordinates.
		HMONITOR Monitor = MonitorFromWindow(GGameWindow, MONITOR_DEFAULTTOPRIMARY);
		MONITORINFO MonitorInfo;
		MonitorInfo.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo( Monitor, &MonitorInfo );
		INT PosX = GGameWindowPosX - MonitorInfo.rcWork.left;
		INT PosY = GGameWindowPosY - MonitorInfo.rcWork.top;

		// Clear out old messages using the old StartupWindowProc
		appWinPumpMessages();

		SetWindowLongX(GGameWindow, GWL_STYLE, GGameWindowStyle);
		SetWindowLongX(GGameWindow, GWL_WNDPROC, (LONG) UWindowsClient::StaticWndProc);
		//		SetClassLongX(GGameWindow, GWL_WNDPROC, (LONG) UWindowsClient::StaticWndProc);
		GGameWindowUsingStartupWindowProc	= FALSE;

		// Restore the minimized window to the correct position, size and styles.
		WINDOWPLACEMENT Placement;
		appMemzero(&Placement, sizeof(WINDOWPLACEMENT));
		Placement.length					= sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(GGameWindow, &Placement);
		Placement.flags						= 0;
		Placement.showCmd					= SW_SHOWNORMAL;	// Restores the minimized window (SW_SHOW won't do that)
		Placement.rcNormalPosition.left		= PosX;
		Placement.rcNormalPosition.right	= PosX + GGameWindowWidth;
		Placement.rcNormalPosition.top		= PosY;
		Placement.rcNormalPosition.bottom	= PosY + GGameWindowHeight;
		SetWindowPlacement(GGameWindow, &Placement);
		UpdateWindow(GGameWindow);

		// Pump the messages using the new (correct) WindowProc
		appWinPumpMessages();
	}
}

/*-----------------------------------------------------------------------------
	Stack walking.
-----------------------------------------------------------------------------*/

typedef BOOL  (WINAPI *TFEnumProcesses)( DWORD * lpidProcess, DWORD cb, DWORD * cbNeeded);
typedef BOOL  (WINAPI *TFEnumProcessModules)(HANDLE hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded);
typedef DWORD (WINAPI *TFGetModuleBaseName)(HANDLE hProcess, HMODULE hModule, LPSTR lpBaseName, DWORD nSize);
typedef DWORD (WINAPI *TFGetModuleFileNameEx)(HANDLE hProcess, HMODULE hModule, LPSTR lpFilename, DWORD nSize);
typedef BOOL  (WINAPI *TFGetModuleInformation)(HANDLE hProcess, HMODULE hModule, LPMODULEINFO lpmodinfo, DWORD cb);

static TFEnumProcesses			FEnumProcesses;
static TFEnumProcessModules		FEnumProcessModules;
static TFGetModuleBaseName		FGetModuleBaseName;
static TFGetModuleFileNameEx	FGetModuleFileNameEx;
static TFGetModuleInformation	FGetModuleInformation;

/**
 * Helper function performing the actual stack walk. This code relies on the symbols being loaded for best results
 * walking the stack albeit at a significant performance penalty.
 *
 * This helper function is designed to be called within a structured exception handler.
 *
 * @param	BackTrace			Array to write backtrace to
 * @param	MaxDepth			Maxium depth to walk - needs to be less than or equal to array size
 * @param	Context				Thread context information
 * @return	EXCEPTION_EXECUTE_HANDLER
 */
static INT CaptureStackTraceHelper( DWORD64 *BackTrace, DWORD MaxDepth, CONTEXT* Context )
{
	STACKFRAME64		StackFrame64;
	HANDLE				ProcessHandle;
	HANDLE				ThreadHandle;
	unsigned long		LastError;
	UBOOL				bStackWalkSucceeded	= TRUE;
	DWORD				CurrentDepth		= 0;

	__try
	{
		// Get context, process and thread information.
		ProcessHandle	= GetCurrentProcess();
		ThreadHandle	= GetCurrentThread();

		// Zero out stack frame.
		memset( &StackFrame64, 0, sizeof(StackFrame64) );

		// Initialize the STACKFRAME structure.
		StackFrame64.AddrPC.Offset       = Context->Eip;
		StackFrame64.AddrPC.Mode         = AddrModeFlat;
		StackFrame64.AddrStack.Offset    = Context->Esp;
		StackFrame64.AddrStack.Mode      = AddrModeFlat;
		StackFrame64.AddrFrame.Offset    = Context->Ebp;
		StackFrame64.AddrFrame.Mode      = AddrModeFlat;

		// Walk the stack one frame at a time.
		while( bStackWalkSucceeded && (CurrentDepth < MaxDepth) )
		{
			bStackWalkSucceeded = StackWalk64(  IMAGE_FILE_MACHINE_I386, 
												ProcessHandle, 
												ThreadHandle, 
												&StackFrame64,
												Context,
												NULL,
												SymFunctionTableAccess64,
												SymGetModuleBase64,
												NULL );

			BackTrace[CurrentDepth++] = StackFrame64.AddrPC.Offset;

			if( !bStackWalkSucceeded  )
			{
				// StackWalk failed! give up.
				LastError = GetLastError( );
				break;
			}

			if( StackFrame64.AddrFrame.Offset == 0 )
			{
				// This frame offset is not valid.
				break;
			}
		}
		
	} 
	__except ( EXCEPTION_EXECUTE_HANDLER )
	{
		// We need to catch any execptions within this function so they dont get sent to 
		// the engine's error handler, hence causing an infinite loop.
		return EXCEPTION_EXECUTE_HANDLER;
	} 

	// NULL out remaining entries.
	for ( ; CurrentDepth<MaxDepth; CurrentDepth++ )
	{
		BackTrace[CurrentDepth] = NULL;
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

#pragma DISABLE_OPTIMIZATION // Work around "flow in or out of inline asm code suppresses global optimization" warning C4740.

/**
 * Capture a stack backtrace and optionally use the passed in exception pointers.
 *
 * @param	BackTrace			[out] Pointer to array to take backtrace
 * @param	MaxDepth			Entries in BackTrace array
 * @param	Context				Optional thread context information
 */
void appCaptureStackBackTrace( DWORD64* BackTrace, DWORD MaxDepth, CONTEXT* Context )
{
	// Make sure we have place to store the information before we go through the process of raising
	// an exception and handling it.
	if( BackTrace == NULL || MaxDepth == 0 )
	{
		return;
	}

	if( Context )
	{
		CaptureStackTraceHelper( BackTrace, MaxDepth, Context );
	}
	else
	{
#ifdef _WIN64
		// Raise an exception so CaptureStackBackTraceHelper has access to context record.
		__try
		{
			RaiseException(	0,			// Application-defined exception code.
							0,			// Zero indicates continuable exception.
							0,			// Number of arguments in args array (ignored if args is NULL)
							NULL );		// Array of arguments
			}
		// Capture the back trace.
		__except( CaptureStackTraceHelper( BackTrace, MaxDepth, GetExceptionInformation()->ContextRecord ) )
		{
		}
#else
		// Use a bit of inline assembly to capture the information relevant to stack walking which is
		// basically EIP and EBP.
		CONTEXT HelperContext;
		memset( &HelperContext, 0, sizeof(CONTEXT) );
		HelperContext.ContextFlags = CONTEXT_FULL;

		// Use a fake function call to pop the return address and retrieve EIP.
		__asm
		{
			call FakeFunctionCall
		FakeFunctionCall: 
			pop eax
			mov HelperContext.Eip, eax
			mov HelperContext.Ebp, ebp
		}

		// Capture the back trace.
		CaptureStackTraceHelper( BackTrace, MaxDepth, &HelperContext );
#endif
	}
}

#pragma ENABLE_OPTIMIZATION

extern void appScriptStackWalkAndDump( ANSICHAR* HumanReadableString );

/**
 * Walks the stack and appends the human readable string to the passed in one.
 * @warning: The code assumes that HumanReadableString is large enough to contain the information.
 *
 * @param	HumanReadableString	String to concatenate information with
 * @param	IgnoreCount			Number of stack entries to ignore (some are guaranteed to be in the stack walking code)
 * @param	Context				Optional thread context information
 */ 
void appStackWalkAndDump( ANSICHAR* HumanReadableString, INT IgnoreCount, CONTEXT* Context )
{	
	// Initialize stack walking... loads up symbol information.
	appInitStackWalking();

	// Temporary memory holding the stack trace.
	#define MAX_DEPTH 100
	DWORD64 StackTrace[MAX_DEPTH];
	memset( StackTrace, 0, sizeof(StackTrace) );

	// Capture stack backtrace.
	appCaptureStackBackTrace( StackTrace, MAX_DEPTH, Context );

	// Capture script backtrace.
	appScriptStackWalkAndDump( HumanReadableString );

	// Skip the first two entries as they are inside the stack walking code.
	INT CurrentDepth = IgnoreCount;
	while( StackTrace[CurrentDepth] )
	{
		appProgramCounterToHumanReadableString( StackTrace[CurrentDepth], HumanReadableString );
		strcat( HumanReadableString, "\r\n" );
		CurrentDepth++;
	}
}

/**
 * Converts the passed in program counter address to a human readable string and appends it to the passed in one.
 * @warning: The code assumes that HumanReadableString is large enough to contain the information.
 *
 * @param	ProgramCounter			Address to look symbol information up for
 * @param	HumanReadableString		String to concatenate information with
 * @param	VerbosityFlags			Bit field of requested data for output.
 */ 
void appProgramCounterToHumanReadableString( DWORD64 ProgramCounter, ANSICHAR* HumanReadableString, EVerbosityFlags VerbosityFlags )
{
	ANSICHAR			SymbolBuffer[sizeof(IMAGEHLP_SYMBOL64) + 512];
	PIMAGEHLP_SYMBOL64	Symbol;
	DWORD				SymbolDisplacement		= 0;
	DWORD64				SymbolDisplacement64	= 0;
	DWORD				LastError;
	
	HANDLE				ProcessHandle = GetCurrentProcess();

	// Initialize stack walking as it loads up symbol information which we require.
	appInitStackWalking();

	// Initialize symbol.
	Symbol					= (PIMAGEHLP_SYMBOL64) SymbolBuffer;
	Symbol->SizeOfStruct	= sizeof(SymbolBuffer);
	Symbol->MaxNameLength	= 512;

	// Get symbol from address.
	if( SymGetSymFromAddr64( ProcessHandle, ProgramCounter, &SymbolDisplacement64, Symbol ) )
	{
		ANSICHAR			FunctionName[1024];

		// Skip any funky chars in the beginning of a function name.
		INT Offset = 0;
		while( Symbol->Name[Offset] < 32 || Symbol->Name[Offset] > 127 )
		{
			Offset++;
		}

		// Write out function name if there is sufficient space.
		sprintf( FunctionName,  ("%s() "), Symbol->Name + Offset );
		strcat( HumanReadableString, FunctionName );
	}
	else
	{
		// No symbol found for this address.
		LastError = GetLastError( );
	}

	if( VerbosityFlags & VF_DISPLAY_FILENAME )
	{
		IMAGEHLP_LINE64		ImageHelpLine;
		ANSICHAR			FileNameLine[1024];

		// Get Line from address
		ImageHelpLine.SizeOfStruct = sizeof( ImageHelpLine );
		if( SymGetLineFromAddr64( ProcessHandle, ProgramCounter, &SymbolDisplacement, &ImageHelpLine) )
		{
			sprintf( FileNameLine, ("0x%-8x + %d bytes [File=%s line=%d] "), (DWORD) ProgramCounter, SymbolDisplacement, ImageHelpLine.FileName, ImageHelpLine.LineNumber );
		}
		else    
		{
			// No line number found.  Print out the logical address instead.
			sprintf( FileNameLine, "Address = 0x%-8x (filename not found) ", (DWORD) ProgramCounter );
		}
		strcat( HumanReadableString, FileNameLine );
	}

	if( VerbosityFlags & VF_DISPLAY_MODULE )
	{
		IMAGEHLP_MODULE64	ImageHelpModule;
		ANSICHAR			ModuleName[1024];

		// Get module information from address.
		ImageHelpModule.SizeOfStruct = sizeof( ImageHelpModule );
		if( SymGetModuleInfo64( ProcessHandle, ProgramCounter, &ImageHelpModule) )
		{
			// Write out Module information if there is sufficient space.
			sprintf( ModuleName, "[in %s]", ImageHelpModule.ImageName );
			strcat( HumanReadableString, ModuleName );
		}
		else
		{
			LastError = GetLastError( );
		}
	}
}

#pragma warning(disable:4191) //@todo: figure out why this is needed

/**
 * Loads modules for current process.
 */ 
static void LoadProcessModules()
{
	INT			ErrorCode = 0;	
	HANDLE		ProcessHandle = GetCurrentProcess(); 
	const INT	MAX_MOD_HANDLES = 1024;
	HMODULE		ModuleHandleArray[MAX_MOD_HANDLES];
	HMODULE*	ModuleHandlePointer = ModuleHandleArray;
	DWORD		BytesRequired;
	MODULEINFO	ModuleInfo;

	// Enumerate process modules.
	UBOOL bEnumProcessModulesSucceeded = FEnumProcessModules( ProcessHandle, ModuleHandleArray, sizeof(ModuleHandleArray), &BytesRequired );
	if( !bEnumProcessModulesSucceeded )
	{
		ErrorCode = GetLastError();
		return;
	}

	// Static array isn't sufficient so we dynamically allocate one.
	UBOOL bNeedToFreeModuleHandlePointer = FALSE;
	if( BytesRequired > sizeof( ModuleHandleArray ) )
	{
		// Keep track of the fact that we need to free it again.
		bNeedToFreeModuleHandlePointer = TRUE;
		ModuleHandlePointer = (HMODULE*) appMalloc( BytesRequired );
		FEnumProcessModules( ProcessHandle, ModuleHandlePointer, sizeof(ModuleHandleArray), &BytesRequired );
	}

	// Find out how many modules we need to load modules for.
	INT	ModuleCount = BytesRequired / sizeof( HMODULE );

	// Load the modules.
	for( INT ModuleIndex=0; ModuleIndex<ModuleCount; ModuleIndex++ )
	{
		ANSICHAR ModuleName[1024];
		ANSICHAR ImageName[1024];
		FGetModuleInformation( ProcessHandle, ModuleHandleArray[ModuleIndex], &ModuleInfo,sizeof( ModuleInfo ) );
		FGetModuleFileNameEx( ProcessHandle, ModuleHandleArray[ModuleIndex], ImageName, 1024 );
		FGetModuleBaseName( ProcessHandle, ModuleHandleArray[ModuleIndex], ModuleName, 1024 );

		// Load module.
		if( !SymLoadModule64( ProcessHandle, ModuleHandleArray[ModuleIndex], ImageName, ModuleName, (DWORD64) ModuleInfo.lpBaseOfDll, (DWORD) ModuleInfo.SizeOfImage ) )
		{
			ErrorCode = GetLastError();
		}
	} 

	// Free the module handle pointer allocated in case the static array was insufficient.
	if( bNeedToFreeModuleHandlePointer )
	{
		appFree( ModuleHandlePointer );
	}
}

UBOOL GUsingRemoteSymbolServers = FALSE;

/**
 * Initializes the symbol engine if needed.
 */ 
UBOOL appInitStackWalking()
{
	static UBOOL SymEngInitialized = FALSE;
	
	// Only initialize once.
	if( !SymEngInitialized )
	{
		void* DllHandle = appGetDllHandle( TEXT("PSAPI.DLL") );
		if( DllHandle == NULL )
		{
			return FALSE;
		}

		// Load dynamically linked PSAPI routines.
		FEnumProcesses			= (TFEnumProcesses)			appGetDllExport( DllHandle,TEXT("EnumProcesses"));
		FEnumProcessModules		= (TFEnumProcessModules)	appGetDllExport( DllHandle,TEXT("EnumProcessModules"));
		FGetModuleFileNameEx	= (TFGetModuleFileNameEx)	appGetDllExport( DllHandle,TEXT("GetModuleFileNameExA"));
		FGetModuleBaseName		= (TFGetModuleBaseName)		appGetDllExport( DllHandle,TEXT("GetModuleBaseNameA"));
		FGetModuleInformation	= (TFGetModuleInformation)	appGetDllExport( DllHandle,TEXT("GetModuleInformation"));

		// Abort if we can't look up the functions.
		if( !FEnumProcesses || !FEnumProcessModules || !FGetModuleFileNameEx || !FGetModuleBaseName || !FGetModuleInformation )
		{
			return FALSE;
		}

		// Set up the symbol engine.
		DWORD SymOpts = SymGetOptions();
		SymOpts |= SYMOPT_LOAD_LINES ;
		SymOpts |= SYMOPT_DEBUG;
		SymSetOptions ( SymOpts );

		// Initialize the symbol engine.
#ifdef _DEBUG
		SymInitialize ( GetCurrentProcess(), "Lib/Debug", TRUE );
#elif FINAL_RELEASE
		SymInitialize ( GetCurrentProcess(), GUsingRemoteSymbolServers ? "\\\\avacomfile\\LatestSymbols\\ReleaseLTCG" : "Lib/ReleaseLTCG", TRUE );
#else
		SymInitialize ( GetCurrentProcess(), GUsingRemoteSymbolServers ? "\\\\avacomfile\\LatestSymbols\\Release" : "Lib/Release", TRUE );
#endif
		LoadProcessModules();

		SymEngInitialized = TRUE;
	}
	return SymEngInitialized;
}

/** Returns the language setting that is configured for the platform */
FString appGetLanguageExt(void)
{
	FString Temp;
	GConfig->GetString( TEXT("Engine.Engine"), TEXT("Language"), Temp, GEngineIni );
	if (Temp.Len() == 0)
	{
		Temp = TEXT("INT");
	}
	return Temp;
}


/*-----------------------------------------------------------------------------
	SHA-1 functions.
-----------------------------------------------------------------------------*/

/**
 * Gets the stored SHA hash from the platform, if it exists. This function
 * must be able to be called from any thread.
 *
 * @param Pathname Pathname to the file to get the SHA for
 * @param Hash 20 byte array that receives the hash
 *
 * @return TRUE if the hash was found, FALSE otherwise
 */
UBOOL appGetFileSHAHash(const TCHAR* Pathname, BYTE Hash[20])
{
	return FALSE;
}

/**
 * Callback that is called if the asynchronous SHA verification fails
 * This will be called from a pooled thread.
 *
 * @param FailedPathname Pathname of file that failed to verify
 * @param bFailedDueToMissingHash TRUE if the reason for the failure was that the hash was missing, and that was set as being an error condition
 */
void appOnFailSHAVerification(const TCHAR* FailedPathname, UBOOL bFailedDueToMissingHash)
{
	appErrorf(TEXT("SHA Verification failed for '%s'. Reason: %s"), 
		FailedPathname ? FailedPathname : TEXT("Unknown file"),
		bFailedDueToMissingHash ? TEXT("Missing hash") : TEXT("Bad hash"));
}

/*----------------------------------------------------------------------------
	Misc functions.
----------------------------------------------------------------------------*/

/**
 * Helper global variables, used in MessageBoxDlgProc for set message text.
 */
static TCHAR* GMessageBoxText = NULL;
static TCHAR* GMessageBoxCaption = NULL;

/**
 * Displays extended message box allowing for YesAll/NoAll
 * @return 3 - YesAll, 4 - NoAll, -1 for Fail
 */
int MessageBoxExt( EAppMsgType MsgType, HWND HandleWnd, const TCHAR* Text, const TCHAR* Caption )
{
	GMessageBoxText = (TCHAR *) Text;
	GMessageBoxCaption = (TCHAR *) Caption;

	if( MsgType == AMT_YesNoYesAllNoAll )
	{
		return DialogBox( GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_YESNO2ALL), HandleWnd, MessageBoxDlgProc );
	}
	else if( MsgType == AMT_YesNoYesAllNoAllCancel )
	{
		return DialogBox( GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_YESNO2ALLCANCEL), HandleWnd, MessageBoxDlgProc );
	}

	return -1;
}


/**
 * Calculates button position and size, localize button text.
 * @param HandleWnd handle to dialog window
 * @param Text button text to localize
 * @param DlgItemId dialog item id
 * @param PositionX current button position (x coord)
 * @param PositionY current button position (y coord)
 * @return TRUE if succeeded
 */
static UBOOL SetDlgItem( HWND HandleWnd, const TCHAR* Text, INT DlgItemId, INT* PositionX, INT* PositionY )
{
	SIZE SizeButton;
		
	HDC DC = CreateCompatibleDC( NULL );
	GetTextExtentPoint32( DC, Text, wcslen(Text), &SizeButton );
	ReleaseDC( NULL, DC );

	SizeButton.cx += 14;
	SizeButton.cy += 8;

	HWND Handle = GetDlgItem( HandleWnd, DlgItemId );
	if( Handle )
	{
		*PositionX -= ( SizeButton.cx + 5 );
		SetWindowPos( Handle, HWND_TOP, *PositionX, *PositionY - SizeButton.cy, SizeButton.cx, SizeButton.cy, 0 );
		SetDlgItemText( HandleWnd, DlgItemId, Text );
		
		return TRUE;
	}

	return FALSE;
}

/**
 * Callback for MessageBoxExt dialog (allowing for Yes to all / No to all )
 * @return		One of ART_Yes, ART_Yesall, ART_No, ART_NoAll, ART_Cancel.
 */
BOOL CALLBACK MessageBoxDlgProc( HWND HandleWnd, UINT Message, WPARAM WParam, LPARAM LParam )
{
    switch(Message)
    {
        case WM_INITDIALOG:
			{
				// Sets most bottom and most right position to begin button placement
				RECT Rect;
				POINT Point;
				
				GetWindowRect( HandleWnd, &Rect );
				Point.x = Rect.right;
				Point.y = Rect.bottom;
				ScreenToClient( HandleWnd, &Point );
				
				INT PositionX = Point.x - 8;
				INT PositionY = Point.y - 10;

				// Localize dialog buttons, sets position and size
				SetDlgItem( HandleWnd, *LocalizeUnrealEd("Cancel"), IDC_CANCEL, &PositionX, &PositionY );
				SetDlgItem( HandleWnd, *LocalizeUnrealEd("NoToAll"), IDC_NOTOALL, &PositionX, &PositionY );
				SetDlgItem( HandleWnd, *LocalizeUnrealEd("No"), IDC_NO_B, &PositionX, &PositionY );
				SetDlgItem( HandleWnd, *LocalizeUnrealEd("YesToAll"), IDC_YESTOALL, &PositionX, &PositionY );
				SetDlgItem( HandleWnd, *LocalizeUnrealEd("Yes"), IDC_YES, &PositionX, &PositionY );

				SetDlgItemText( HandleWnd, IDC_MESSAGE, GMessageBoxText );
				SetWindowText( HandleWnd, GMessageBoxCaption );

				SetForegroundWindow( HandleWnd );

				return TRUE;
			}
		case WM_COMMAND:
            switch( LOWORD( WParam ) )
            {
                case IDC_YES:
                    EndDialog( HandleWnd, ART_Yes );
					break;
                case IDC_YESTOALL:
                    EndDialog( HandleWnd, ART_YesAll );
					break;
				case IDC_NO_B:
                    EndDialog( HandleWnd, ART_No );
					break;
				case IDC_NOTOALL:
                    EndDialog( HandleWnd, ART_NoAll );
					break;
				case IDC_CANCEL:
					EndDialog( HandleWnd, ART_Cancel );
					break;
            }
			break;
        default:
            return FALSE;
    }
    return TRUE;
}

/** 
* This will update the passed in FMemoryChartEntry with the platform specific data
*
* @param FMemoryChartEntry the struct to fill in
**/
void appUpdateMemoryChartStats( struct FMemoryChartEntry& MemoryEntry )
{
	//@todo fill in these values

	// set the memory chart data
	MemoryEntry.GPUMemUsed = 0;

	MemoryEntry.NumAllocations = 0;
	MemoryEntry.AllocationOverhead = 0;
	MemoryEntry.AllignmentWaste = 0;
}