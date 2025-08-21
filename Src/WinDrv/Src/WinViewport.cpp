/*=============================================================================
	WinViewport.cpp: FWindowsViewport code.
	Copyright 1997-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "WinDrv.h"
#include "EngineUserInterfaceClasses.h"
#include "strsafe.h"

#define USE_DIRECTINPUT_MOUSEBUTTON 1

#pragma warning(disable:4995)
#include "../../avaNet/Inc/avaWebInClient.h"
#pragma warning(default:4995)

#define WM_MOUSEWHEEL 0x020A

// From atlwin.h:
#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lParam)	((INT)(short)LOWORD(lParam))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lParam)	((INT)(short)HIWORD(lParam))
#endif

/** If TRUE, FWindowsViewport::UpdateModifierState() will enqueue rather than process immediately. */
UBOOL GShouldEnqueueModifierStateUpdates = FALSE;

/** Settings for the game Window */
HWND GGameWindow = NULL;
UBOOL GGameWindowUsingStartupWindowProc = FALSE;	// Whether the game is using the startup window procedure
UBOOL GUseGameWindowUponResize = TRUE;				// Whether FWindowsViewport::Resize should use GGameWindow rather than create a new window
DWORD GGameWindowStyle = 0;
INT GGameWindowPosX = 0;
INT GGameWindowPosY = 0;
INT GGameWindowWidth = 100;
INT GGameWindowHeight = 100;

/** This variable reflects the latest status sent to us by WM_ACTIVATEAPP */
UBOOL GWindowActive = FALSE;

/*** BEGIN		REDDUCK
*/
extern UBOOL GFakeFullScreen;


// Enumerate Desktop Windows.(2007/07/04 고광록)
BOOL CALLBACK MinimizeDesktopWindowsProc(HWND hwnd, LPARAM lParam)
{
	bool bVisible = ::IsWindowVisible(hwnd) == TRUE;
	HWND hWndParent = HWND(GetWindowLong(hwnd, GWL_HWNDPARENT));
	bool bToolWindow = GetWindowLong(hwnd, GWL_EXSTYLE) == WS_EX_TOOLWINDOW;
	bool bIconic = ::IsIconic(hwnd) == TRUE;
	HWND hwndSelf = (HWND)lParam;
	bool bSelf = (hwndSelf == hwnd);

	if ( lParam == 0 )
		return TRUE;

	// Desktop Window들을 모두 Minimize 상태로 만들어 준다.
	if ( hwnd != NULL && 
		bVisible && 
		(hWndParent == NULL || hWndParent == ::GetDesktopWindow()) && 
		!bToolWindow && 
		!bIconic && 
		!bSelf )
	{
		::ShowWindow(hwnd, SW_MINIMIZE);
	}

	return TRUE;
}

void MinimizeOtherWindows(HWND hwnd)
{
	//---------------------------------------------------------------------------
	//	http://www.programmersheaven.com/mb/windows/207752/207752/ReadMessage.aspx
	//	최소화 될 때 animation기능을 꺼주고 한다.(2007/07/05)
	//---------------------------------------------------------------------------

	// save the minimize window anitation parameter
	ANIMATIONINFO ai;
	ai.cbSize = sizeof(ai);
	SystemParametersInfo(SPI_GETANIMATION, sizeof(ai), &ai, 0);

	INT nMinAnimate = ai.iMinAnimate;

	// no animation
	ai.iMinAnimate = 0;
	SystemParametersInfo(SPI_SETANIMATION, sizeof(ai), &ai, 0);

	// 다른 윈도우들을 모두 최소화 해준다.
	EnumWindows(MinimizeDesktopWindowsProc, (LPARAM)hwnd);

	// restore animation
	ai.iMinAnimate = nMinAnimate;
	SystemParametersInfo(SPI_SETANIMATION, sizeof(ai), &ai, 0);
}

void RestoreDisplaySettings(UINT SizeX, UINT SizeY)
{
	DEVMODE dm;

	ZeroMemory(&dm, sizeof(DEVMODE));
	dm.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);

	if ( dm.dmPelsWidth != SizeX || dm.dmPelsHeight != SizeY )
	{
		dm.dmPelsWidth = SizeX;
		dm.dmPelsHeight = SizeY;
		dm.dmBitsPerPel = 32;
		dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
		// dmDisplayFrequency는 위에서 기존에 얻은 값을 그대로 사용하게 된다.

		LONG lResult;
		if ( (lResult = ChangeDisplaySettings(&dm, CDS_FULLSCREEN)) != DISP_CHANGE_SUCCESSFUL )
		{
			dm.dmDisplayFrequency = 60;
			dm.dmFields &= ~DM_DISPLAYFREQUENCY;

			// 실패하면 60Hz로 다시 시도한다.
			lResult = ChangeDisplaySettings(&dm, CDS_FULLSCREEN);

			// 바탕화면 해상도 자체를 변경한다.(이건 절대로 실패하면 안된다)
			check(lResult == DISP_CHANGE_SUCCESSFUL);
		}
	}
}

/*** END		REDDUCK
*/


/**
* This is client RECT for the window we most recently locked the mouse to (in screen coordinates).  Note that
* it may not exactly match the rect returned by GetClipCursor, since that rect will be constrained to the
* desktop area, while our client rect will not.
*
* The mouse lock rect is a shared system resource, so we should not try to unlock a mouse
* if some other window wants it locked.
*/
RECT GMouseLockClientRect = { 0,0,0,0 };

BYTE GKeyboardInputLanguage = KBDINPUTLANG_Default;	 // Current IME Language

#if WITH_IME
#pragma comment( lib, "Imm32.lib" )

UBOOL CheckIMESupport( HWND Window )
{
	HIMC Imc = ImmGetContext( Window );
	if( !Imc )
	{
		Imc = ImmCreateContext();
		if( Imc )	
		{
			ImmAssociateContext( Window, Imc ); 
		}
	}
	else
	{
		ImmReleaseContext( Window, Imc );
	}

	return( Imc != NULL );
}

void DestroyIME( HWND Window )
{
	HIMC Imc = ImmGetContext( Window );
	if( Imc )
	{
		ImmAssociateContext( Window, NULL );
		ImmDestroyContext( Imc );
	}

	
}

#define LID_TRADITIONAL_CHINESE	0x0404
#define LID_JAPANESE			0x0411
#define LID_KOREAN				0x0412
#define LID_SIMPLIFIED_CHINESE	0x0804

#define LANG_CHT	MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL)
#define LANG_CHS	MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED)

#define MAKEIMEVERSION(major,minor) ( (DWORD)( ( (BYTE)( major ) << 24 ) | ( (BYTE)( minor ) << 16 ) ) )
#define IMEID_VER(dwId)		( ( dwId ) & 0xffff0000 )
#define IMEID_LANG(dwId)	( ( dwId ) & 0x0000ffff )

#define _CHT_HKL_DAYI				( (HKL)0xE0060404 )	// DaYi
#define _CHT_HKL_NEW_PHONETIC		( (HKL)0xE0080404 )	// New Phonetic
#define _CHT_HKL_NEW_CHANG_JIE		( (HKL)0xE0090404 )	// New Chang Jie
#define _CHT_HKL_NEW_QUICK			( (HKL)0xE00A0404 )	// New Quick
#define _CHT_HKL_HK_CANTONESE		( (HKL)0xE00B0404 )	// Hong Kong Cantonese
#define _CHT_IMEFILENAME	TEXT("TINTLGNT.IME")	// New Phonetic
#define _CHT_IMEFILENAME2	TEXT("CINTLGNT.IME")	// New Chang Jie
#define _CHT_IMEFILENAME3	TEXT("MSTCIPHA.IME")	// Phonetic 5.1
#define IMEID_CHT_VER42 ( LANG_CHT | MAKEIMEVERSION( 4, 2 ) )	// New(Phonetic/ChanJie)IME98  : 4.2.x.x // Win98
#define IMEID_CHT_VER43 ( LANG_CHT | MAKEIMEVERSION( 4, 3 ) )	// New(Phonetic/ChanJie)IME98a : 4.3.x.x // Win2k
#define IMEID_CHT_VER44 ( LANG_CHT | MAKEIMEVERSION( 4, 4 ) )	// New ChanJie IME98b          : 4.4.x.x // WinXP
#define IMEID_CHT_VER50 ( LANG_CHT | MAKEIMEVERSION( 5, 0 ) )	// New(Phonetic/ChanJie)IME5.0 : 5.0.x.x // WinME
#define IMEID_CHT_VER51 ( LANG_CHT | MAKEIMEVERSION( 5, 1 ) )	// New(Phonetic/ChanJie)IME5.1 : 5.1.x.x // IME2002(w/OfficeXP)
#define IMEID_CHT_VER52 ( LANG_CHT | MAKEIMEVERSION( 5, 2 ) )	// New(Phonetic/ChanJie)IME5.2 : 5.2.x.x // IME2002a(w/WinXP)
#define IMEID_CHT_VER60 ( LANG_CHT | MAKEIMEVERSION( 6, 0 ) )	// New(Phonetic/ChanJie)IME6.0 : 6.0.x.x // New IME 6.0(web download)
#define IMEID_CHT_VER_VISTA ( LANG_CHT | MAKEIMEVERSION( 7, 0 ) )	// All TSF TIP under Cicero UI-less mode: a hack to make GetImeId() return non-zero value

#define _CHS_HKL		( (HKL)0xE00E0804 )	// MSPY
#define _CHS_IMEFILENAME	TEXT("PINTLGNT.IME")	// MSPY1.5/2/3
#define _CHS_IMEFILENAME2	TEXT("MSSCIPYA.IME")	// MSPY3 for OfficeXP
#define IMEID_CHS_VER41	( LANG_CHS | MAKEIMEVERSION( 4, 1 ) )	// MSPY1.5	// SCIME97 or MSPY1.5 (w/Win98, Office97)
#define IMEID_CHS_VER42	( LANG_CHS | MAKEIMEVERSION( 4, 2 ) )	// MSPY2	// Win2k/WinME
#define IMEID_CHS_VER53	( LANG_CHS | MAKEIMEVERSION( 5, 3 ) )	// MSPY3	// WinXP

UINT (WINAPI * _GetReadingString)( HIMC, UINT, LPWSTR, PINT, BOOL*, PUINT );
BOOL (WINAPI * _ShowReadingWindow)( HIMC, BOOL );

#include <Dimm.h>
INPUTCONTEXT*	(WINAPI * _ImmLockIMC)( HIMC );
BOOL			(WINAPI * _ImmUnlockIMC)( HIMC );
LPVOID			(WINAPI * _ImmLockIMCC)( HIMCC );
BOOL			(WINAPI * _ImmUnlockIMCC)( HIMCC );

HINSTANCE hDllImm32 = NULL;
HINSTANCE hDllIme = NULL;
DWORD ImeId[2] = {0,0};
HKL hkl = NULL;

TArray<HWND> ImePopupWindows;
UBOOL bOpenCandidate = FALSE;
DWORD ImeProperty = 0;

static BYTE GetKeyboardLayoutLanguage( LPARAM lParam )
{
	HKL hKeyboardLayout = (HKL)lParam;
	if( hKeyboardLayout == 0 )
		hKeyboardLayout = GetKeyboardLayout(0);

	if( hkl != hKeyboardLayout )
		hkl = hKeyboardLayout;

	switch( LOWORD( hKeyboardLayout ) )
	{
	case LID_TRADITIONAL_CHINESE:	return KBDINPUTLANG_TraditionalChinese;
	case LID_SIMPLIFIED_CHINESE:	return KBDINPUTLANG_SimplifiedChinese;
	case LID_KOREAN:	return KBDINPUTLANG_Korean;
	case LID_JAPANESE:	return KBDINPUTLANG_Japanese;
	default:	return KBDINPUTLANG_Default;
	}

	return KBDINPUTLANG_Default;
}


static void GetProcIMC()
{
	if( (hDllImm32 = LoadLibrary(TEXT("imm32.dll"))) != NULL )
	{
#pragma warning(push)
#pragma warning(disable:4191)
		_ImmLockIMC =               (INPUTCONTEXT* (WINAPI*)(HIMC hIMC)) GetProcAddress(hDllImm32, "ImmLockIMC");
		_ImmUnlockIMC =             (BOOL (WINAPI*)(HIMC hIMC)) GetProcAddress(hDllImm32, "ImmUnlockIMC");
		_ImmLockIMCC =              (LPVOID (WINAPI*)(HIMCC hIMCC)) GetProcAddress(hDllImm32, "ImmLockIMCC");
		_ImmUnlockIMCC =            (BOOL (WINAPI*)(HIMCC hIMCC)) GetProcAddress(hDllImm32, "ImmUnlockIMCC");
		BOOL (WINAPI* _ImmDisableTextFrameService)(DWORD) = (BOOL (WINAPI*)(DWORD))GetProcAddress(hDllImm32, "ImmDisableTextFrameService");
#pragma warning(pop)

		if ( _ImmDisableTextFrameService )
		{
			_ImmDisableTextFrameService( (DWORD)-1 );
		}
	}

	check( hDllImm32 != NULL && _ImmLockIMC != NULL && _ImmUnlockIMC != NULL && _ImmUnlockIMC != NULL && _ImmUnlockIMCC != NULL );	
}

static void GetIMEId( HWND hWnd )
{
	ImeId[0] = 0;
	ImeId[1] = 0;

	// @TODO : support Vista TSF(TextServiceFramework)
	//if ( g_bUILessMode && GETLANG() == LANG_CHT )
	//{
	//	// In case of Vista, artificial value is returned so that it's not considered as older IME.
	//	dwRet[0] = IMEID_CHT_VER_VISTA;
	//	dwRet[1] = 0;
	//	return dwRet[0];
	//}

	if ( hkl != _CHT_HKL_NEW_PHONETIC && hkl != _CHT_HKL_NEW_CHANG_JIE
		&& hkl != _CHT_HKL_NEW_QUICK && hkl != _CHT_HKL_HK_CANTONESE && hkl != _CHS_HKL )
	{
		return;
	}

	TCHAR szIMEFileName[MAX_PATH + 1];
	if( ! ImmGetIMEFileName(hkl, szIMEFileName, ARRAY_COUNT(szIMEFileName)) )
		return;
	
	if( ! _GetReadingString )
	{
#define LCID_INVARIANT MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)
		if ( ( CompareString( LCID_INVARIANT, NORM_IGNORECASE, szIMEFileName, -1, _CHT_IMEFILENAME, -1 ) != 2 )
			&& ( CompareString( LCID_INVARIANT, NORM_IGNORECASE, szIMEFileName, -1, _CHT_IMEFILENAME2, -1 ) != 2 )
			&& ( CompareString( LCID_INVARIANT, NORM_IGNORECASE, szIMEFileName, -1, _CHT_IMEFILENAME3, -1 ) != 2 )
			&& ( CompareString( LCID_INVARIANT, NORM_IGNORECASE, szIMEFileName, -1, _CHS_IMEFILENAME, -1 ) != 2 )
			&& ( CompareString( LCID_INVARIANT, NORM_IGNORECASE, szIMEFileName, -1, _CHS_IMEFILENAME2, -1 ) != 2 )
			)
		{
			return;
		}
	}

	DWORD dwVerSize;
	DWORD dwVerHandle;
	DWORD dwLang = ((DWORD)hkl & 0xffff);

	dwVerSize = GetFileVersionInfoSize( szIMEFileName, &dwVerHandle );
	if ( dwVerSize )
	{
		// appMalloc alternatives
		TArray<BYTE> VerBuffer;
		VerBuffer.AddZeroed(dwVerSize);
		LPVOID lpVerBuffer = (LPVOID)VerBuffer.GetTypedData();
		if ( lpVerBuffer )
		{
			if ( GetFileVersionInfo( szIMEFileName, dwVerHandle, dwVerSize, lpVerBuffer ) )
			{
				LPVOID lpVerData;
				UINT cbVerData;

				if ( VerQueryValue( lpVerBuffer, TEXT("\\"), &lpVerData, &cbVerData ) )
				{
#define pVerFixedInfo ((VS_FIXEDFILEINFO FAR*)lpVerData)
					DWORD dwVer = pVerFixedInfo->dwFileVersionMS;
					dwVer = ( dwVer & 0x00ff0000 ) << 8 | ( dwVer & 0x000000ff ) << 16;
					if ( _GetReadingString ||
						dwLang == LANG_CHT && (
						dwVer == MAKEIMEVERSION(4, 2) || 
						dwVer == MAKEIMEVERSION(4, 3) || 
						dwVer == MAKEIMEVERSION(4, 4) || 
						dwVer == MAKEIMEVERSION(5, 0) ||
						dwVer == MAKEIMEVERSION(5, 1) ||
						dwVer == MAKEIMEVERSION(5, 2) ||
						dwVer == MAKEIMEVERSION(6, 0) )
						||
						dwLang == LANG_CHS && (
						dwVer == MAKEIMEVERSION(4, 1) ||
						dwVer == MAKEIMEVERSION(4, 2) ||
						dwVer == MAKEIMEVERSION(5, 3) ) )
					{
						ImeId[0] = dwVer | dwLang;
						ImeId[1] = pVerFixedInfo->dwFileVersionLS;
					}
#undef pVerFixedInfo
				}
			}
		}
	}
}

static void SetupIMEApi( HWND hWnd )
{
	TCHAR szImeFile[MAX_PATH + 1];

	// identical to the SetupImeApi function in DXUT::CIMEEditbox
	_GetReadingString = NULL;
	_ShowReadingWindow = NULL;

	if( ImmGetIMEFileName( hkl, szImeFile, sizeof(szImeFile)/sizeof(szImeFile[0]) - 1 ) != 0 )
	{
		if( hDllIme )
			FreeLibrary( hDllIme );

		if( (hDllIme = LoadLibrary( szImeFile )) != NULL)
		{
#pragma warning(push)
#pragma warning( disable : 4191)
			_GetReadingString = (UINT (WINAPI*)(HIMC, UINT, LPWSTR, PINT, BOOL*, PUINT))
				( GetProcAddress( hDllIme, "GetReadingString" ) );
			_ShowReadingWindow =(BOOL (WINAPI*)(HIMC, BOOL))
				( GetProcAddress( hDllIme, "ShowReadingWindow" ) );
#pragma warning(pop)

			DWORD dwErr = GetLastError();

			if( _ShowReadingWindow )
			{
				HIMC hImc = ImmGetContext(hWnd);
				if(hImc) 
				{
					_ShowReadingWindow( hImc, false );
					ImmReleaseContext(hWnd, hImc);
				}
			}
		}
	}
}

inline static UINT GetCurrentCodePage()
{
	TCHAR szCodePage[8];
	INT iRc = GetLocaleInfo( MAKELCID(LOWORD(hkl), SORT_DEFAULT), LOCALE_IDEFAULTANSICODEPAGE, szCodePage, sizeof(szCodePage) );
	
	return iRc == 0 ? 1252 /*Western Alphabet*/ : appAtoi(szCodePage); 
}

static void OnInputLangChanged( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
	hkl = (HKL)lParam;
	GKeyboardInputLanguage = GetKeyboardLayoutLanguage(lParam);

	SetupIMEApi(hWnd);
	GetIMEId(hWnd);
	HIMC hImc = ImmGetContext(hWnd);
	if( hImc )
	{
		ImeProperty = ImmGetProperty(hkl, IGP_PROPERTY);
		ImmReleaseContext(hWnd, hImc);
	}
}

static FString GetPrivateReadingString( HIMC hImc )
{
	FString ReadStr;

	if( hImc )
	{
		if( _GetReadingString )
		{
			DWORD dwErr = 0;
			UINT uMaxUiLen;
			BOOL bVertical;
			// Obtain the reading string size
			INT ReadStrLen = _GetReadingString( hImc, 0, NULL, (PINT)&dwErr, &bVertical, &uMaxUiLen );

			if( ReadStrLen > 0 ) 
			{
				// appMalloc alternatives		
				TArray<BYTE> ReadStrArray;
				ReadStrArray.AddZeroed(sizeof(TCHAR) * ReadStrLen);

				TCHAR *szReadStr = (TCHAR*)ReadStrArray.GetTypedData();
				_GetReadingString( hImc, ReadStrLen, szReadStr, (PINT)&dwErr, &bVertical, &uMaxUiLen );
				ReadStr = szReadStr;
			}
		}
		else  //	IMEs that doesn't implement Reading String API
		{
			INPUTCONTEXT* lpIMC = _ImmLockIMC(hImc);

			// *** hacking code from Michael Yang ***

			LPBYTE p = NULL;
			DWORD dwlen = 0;
			DWORD dwerr = 0;
			WCHAR* wstr = NULL;
			UBOOL unicode = FALSE;

			if( lpIMC )
			{
				switch ( ImeId[0] )
				{
				case IMEID_CHT_VER42: // New(Phonetic/ChanJie)IME98  : 4.2.x.x // Win98
				case IMEID_CHT_VER43: // New(Phonetic/ChanJie)IME98a : 4.3.x.x // WinMe, Win2k
				case IMEID_CHT_VER44: // New ChanJie IME98b          : 4.4.x.x // WinXP

					p = *(LPBYTE *)((LPBYTE)_ImmLockIMCC(lpIMC->hPrivate) + 24);
					if (!p) break;
					dwlen = *(DWORD *)(p + 7*4 + 32*4);	//m_dwInputReadStrLen
					dwerr = *(DWORD *)(p + 8*4 + 32*4);	//m_dwErrorReadStrStart
					wstr = (WCHAR *)(p + 56);
					unicode = TRUE;
					break;

				case IMEID_CHT_VER50: // 5.0.x.x // WinME

					p = *(LPBYTE *)((LPBYTE)_ImmLockIMCC(lpIMC->hPrivate) + 3*4); // PCKeyCtrlManager
					if (!p) break;
					p = *(LPBYTE *)((LPBYTE)p + 1*4 + 5*4 + 4*2 ); // = PCReading = &STypingInfo
					if (!p) break;
					dwlen = *(DWORD *)(p + 1*4 + (16*2+2*4) + 5*4 + 16);		//m_dwDisplayStringLength;
					dwerr = *(DWORD *)(p + 1*4 + (16*2+2*4) + 5*4 + 16 + 1*4);	//m_dwDisplayErrorStart;
					wstr = (WCHAR *)(p + 1*4 + (16*2+2*4) + 5*4);
					unicode = FALSE;
					break;

				case IMEID_CHT_VER51: // 5.1.x.x // IME2002(w/OfficeXP)
				case IMEID_CHT_VER52: // 5.2.x.x // (w/whistler)
				case IMEID_CHS_VER53: // 5.3.x.x // SCIME2k or MSPY3 (w/OfficeXP and Whistler)
					p = *(LPBYTE *)((LPBYTE)_ImmLockIMCC(lpIMC->hPrivate) + 4);   // PCKeyCtrlManager
					if (!p) break;
					p = *(LPBYTE *)((LPBYTE)p + 1*4 + 5*4);                       // = PCReading = &STypingInfo
					if (!p) break;
					dwlen = *(DWORD *)(p + 1*4 + (16*2+2*4) + 5*4 + 16 * 2);		//m_dwDisplayStringLength;
					dwerr = *(DWORD *)(p + 1*4 + (16*2+2*4) + 5*4 + 16 * 2 + 1*4);	//m_dwDisplayErrorStart;
					wstr  = (WCHAR *) (p + 1*4 + (16*2+2*4) + 5*4);
					unicode = TRUE;
					break;

					// the code tested only with Win 98 SE (MSPY 1.5/ ver 4.1.0.21)
				case IMEID_CHS_VER41:
					{
						INT offset;
						offset = ( ImeId[1] >= 0x00000002 ) ? 8 : 7;

						p = *(LPBYTE *)((LPBYTE)_ImmLockIMCC(lpIMC->hPrivate) + offset * 4);
						if (!p) break;
						dwlen = *(DWORD *)(p + 7*4 + 16*2*4);
						dwerr = *(DWORD *)(p + 8*4 + 16*2*4);
						dwerr = Min(dwerr, dwlen);
						wstr = (WCHAR *)(p + 6*4 + 16*2*1);
						unicode = TRUE;
						break;
					}

				case IMEID_CHS_VER42: // 4.2.x.x // SCIME98 or MSPY2 (w/Office2k, Win2k, WinME, etc)
					{
						OSVERSIONINFO osi;
						osi.dwOSVersionInfoSize = sizeof(osi);
						GetVersionEx( &osi );

						UBOOL bIsNT = osi.dwPlatformId == VER_PLATFORM_WIN32_NT;

						INT nTcharSize = bIsNT ? sizeof(WCHAR) : sizeof(CHAR);
						p = *(LPBYTE *)((LPBYTE)_ImmLockIMCC(lpIMC->hPrivate) + 1*4 + 1*4 + 6*4); // = PCReading = &STypintInfo
						if (!p) break;
						dwlen = *(DWORD *)(p + 1*4 + (16*2+2*4) + 5*4 + 16 * nTcharSize);		//m_dwDisplayStringLength;
						dwerr = *(DWORD *)(p + 1*4 + (16*2+2*4) + 5*4 + 16 * nTcharSize + 1*4);	//m_dwDisplayErrorStart;
						wstr  = (WCHAR *) (p + 1*4 + (16*2+2*4) + 5*4);                 //m_tszDisplayString
						unicode = bIsNT ? TRUE : FALSE;
					}
				}	// switch
			}

			if( dwlen > 0 )
			{
				if( unicode )
				{
					// appMalloc alternatives
					TArray<BYTE> ReadStrArray;
					ReadStrArray.AddZeroed(dwlen + 1);
					WCHAR* szReadStr = (WCHAR*)ReadStrArray.GetTypedData();
					appStrncpy(szReadStr, wstr, ReadStrArray.Num());
					ReadStr = szReadStr;
				}
				else
				{
					UINT CodePage = GetCurrentCodePage();
					INT wstrLen = MultiByteToWideChar(CodePage, 0, (CHAR*)wstr, dwlen, NULL, 0); 

					// appMalloc alternatives
					TArray<BYTE> ReadStrArray;
					ReadStrArray.AddZeroed(sizeof(WCHAR) * (wstrLen + 1) );
					WCHAR* pwszReadStr = (WCHAR*)ReadStrArray.GetTypedData();
					MultiByteToWideChar(CodePage, 0, (char*)wstr, dwlen, pwszReadStr, wstrLen + 1);
					ReadStr = pwszReadStr;
				}
			}

			if( p )
			{
				_ImmUnlockIMCC( lpIMC->hPrivate );
			}
			if( lpIMC )
			{
				_ImmUnlockIMC( hImc );
			}
		}
	}
	return ReadStr;
}

UBOOL NotifyIME( BYTE Action, INT Index, INT Value )
{
	HIMC hImc = ImmGetContext( GGameWindow );
	if( !hImc )
		return FALSE;

	DWORD ImeAction = 0;
	switch(Action)
	{
	case IMENOTIFY_OpenCandidate:	ImeAction = NI_OPENCANDIDATE;	break;
	case IMENOTIFY_CloseCandidate:	ImeAction = NI_CLOSECANDIDATE;	break;
	case IMENOTIFY_SelectCandidatestr:	ImeAction = NI_SELECTCANDIDATESTR;	break;
	case IMENOTIFY_ChangeCandidatelist:	ImeAction = NI_CHANGECANDIDATELIST;	break;
	case IMENOTIFY_FinalizeConversionResult:	ImeAction = NI_FINALIZECONVERSIONRESULT;	break;
	case IMENOTIFY_CompositionStr:		ImeAction = NI_COMPOSITIONSTR;	break;
	case IMENOTIFY_SetCandidate_PageStart:	ImeAction = NI_SETCANDIDATE_PAGESTART;	break;
	case IMENOTIFY_SetCandidate_PageSize:	ImeAction = NI_SETCANDIDATE_PAGESIZE;	break;
	case IMENOTIFY_ImeMenuSelected:	ImeAction = NI_IMEMENUSELECTED;	break;
	default: check(FALSE);	break;
	}

	UBOOL bResult = ImmNotifyIME( hImc, ImeAction, Index, Value );
	ImmReleaseContext( GGameWindow, hImc );

	return bResult;
}

BOOL CALLBACK CollectImePopupProc( HWND hWnd, LPARAM lParam )
{
	TCHAR szClassName[MAX_PATH+1];
	DWORD Id = ImeId[0];
	INT ClassNameLen = GetClassName( hWnd, szClassName, ARRAY_COUNT(szClassName) );
	if( ClassNameLen > 0 )
	{
		// MSPY
		if( (Id == IMEID_CHS_VER41 || Id == IMEID_CHS_VER42 || Id == IMEID_CHS_VER53) &&
			appStristr( szClassName, TEXT("MSPY")) == szClassName )
		{
			ImePopupWindows.AddItem( hWnd );
		}
	}
	return TRUE;
}

void CollectImePopupWindows()
{
	::EnumWindows( (WNDENUMPROC)CollectImePopupProc, 0 );
}

void HideImePopupWindows( HWND hWnd )
{
	switch( GKeyboardInputLanguage )
	{
	case KBDINPUTLANG_Default:
	case KBDINPUTLANG_Korean:
	case KBDINPUTLANG_Japanese:
		if( hWnd )
		{
			HIMC hImc = ImmGetContext(hWnd);
			if( hImc )
			{
				CANDIDATEFORM CandForm;
				CandForm.dwIndex = 0;
				CandForm.dwStyle = CFS_FORCE_POSITION;
				CandForm.ptCurrentPos.x = CandForm.ptCurrentPos.y = -100000;
				ImmSetCandidateWindow(hImc, &CandForm);
				ImmReleaseContext(hWnd, hImc);
			}
		}
		break;
	case KBDINPUTLANG_TraditionalChinese:
	case KBDINPUTLANG_SimplifiedChinese:
		{
			if( bOpenCandidate )
			{
				if( ImePopupWindows.Num() == 0 )
					CollectImePopupWindows();

				for( INT i = 0 ; i < ImePopupWindows.Num() ; i++ )
					::MoveWindow(ImePopupWindows(i),0,0,0,0,TRUE);
			}
		}
		break;
	}
}

#endif

//
//	FWindowsViewport::FWindowsViewport
//
FWindowsViewport::FWindowsViewport(UWindowsClient* InClient,FViewportClient* InViewportClient,const TCHAR* InName,UINT InSizeX,UINT InSizeY,UBOOL InFullscreen,HWND InParentWindow,INT InPosX, INT InPosY)
	:	FViewport(InViewportClient)
	,	bUpdateModifierStateEnqueued( FALSE )	
	,   bShouldResetMouseButtons( TRUE )
	,	PreventCapture(FALSE)
{
	Client					= InClient;

	Name					= InName;
	Window					= NULL;
	ParentWindow			= InParentWindow;

	// New MouseCapture/MouseLock API
	SystemMouseCursor		= MC_Arrow;
	bMouseLockRequested		= FALSE;
	bCapturingMouseInput	= FALSE;
	MouseLockClientRect.top = MouseLockClientRect.left = MouseLockClientRect.bottom = MouseLockClientRect.right = 0;

	// Old MouseCapture/MouseLock API
	bCapturingJoystickInput = 1;
	bLockingMouseToWindow	= 0;

	Minimized				= 0;
	Maximized				= 0;
	Resizing				= 0;
	bPerformingSize			= FALSE;
	
	LastMouseXEventTime		= 0;
	LastMouseYEventTime		= 0;

	LastLockedTime			= 0;

	Client->Viewports.AddItem(this);

	// if win positions are default, attempt to read from cmdline
	if (InPosX == -1)
	{
		Parse(appCmdLine(),TEXT("PosX="),InPosX);
	}
	if (InPosY == -1)
	{
		Parse(appCmdLine(),TEXT("PosY="),InPosY);
	}

	// Creates the viewport window.
	Resize(InSizeX,InSizeY,InFullscreen,InPosX,InPosY);

#if WITH_IME
	bSupportsIME			= TRUE;
	GetProcIMC();
	OnInputLangChanged(Window, 0,0);
#endif

	appMemzero(KeyStates,sizeof(KeyStates));

	// Set as active window.
	::SetActiveWindow(Window);

	// Set initial key state.
	for(UINT KeyIndex = 0;KeyIndex < 256;KeyIndex++)
	{
		FName*	KeyName = Client->KeyMapVirtualToName.Find(KeyIndex);

		if(KeyName && *KeyName != KEY_LeftMouseButton && *KeyName != KEY_RightMouseButton && *KeyName != KEY_MiddleMouseButton && *KeyName != KEY_XMouseButton && *KeyName != KEY_YMouseButton)
			KeyStates[KeyIndex] = ::GetKeyState(KeyIndex) & 0x8000;
	}

#ifdef _WEB_IN_CLIENT
	_WebInClient().SetReceiveHandle(Window);
#endif
}

FWindowsViewport::~FWindowsViewport()
{
	check(!Window);

	RECT ClipRect;
	UBOOL bClipRectValid = (::GetClipCursor( &ClipRect ) != 0);
	if( bClipRectValid )
	{
		// Release lock on mouse cursor rect if we had that
		if( GMouseLockClientRect.top == MouseLockClientRect.top &&
			GMouseLockClientRect.left == MouseLockClientRect.left &&
			GMouseLockClientRect.bottom == MouseLockClientRect.bottom &&
			GMouseLockClientRect.right == MouseLockClientRect.right )
		{
			MouseLockClientRect.top = MouseLockClientRect.left = MouseLockClientRect.bottom = MouseLockClientRect.right = 0;
			GMouseLockClientRect.top = GMouseLockClientRect.left = GMouseLockClientRect.bottom = GMouseLockClientRect.top = 0;
			::ClipCursor(NULL);
		}
	}
}

//
//	FWindowsViewport::Destroy
//

void FWindowsViewport::Destroy()
{
	ViewportClient = NULL;

#if WITH_IME
	DestroyIME( Window );
#endif

	// Release the viewport RHI.
	UpdateViewportRHI(TRUE,0,0,FALSE);

	// Destroy the viewport's window.
	DestroyWindow(Window);
}

//
//	FWindowsViewport::Resize
//
void FWindowsViewport::Resize(UINT NewSizeX,UINT NewSizeY,UBOOL NewFullscreen,INT InPosX, INT InPosY)
{
	if( bPerformingSize )
	{
		debugf(NAME_Error,TEXT("bPerformingSize == 1, FWindowsViewport::Resize"));
		appDebugBreak();
		return;
	}

	// This makes sure that the Play From Here window in the editor can't go fullscreen
	if (NewFullscreen && appStricmp(*Name, PLAYWORLD_VIEWPORT_NAME) == 0)
	{
		return;
	}

	bPerformingSize			= TRUE;
	UBOOL bWasFullscreen	= IsFullscreen();
	UBOOL bUpdateWindow		= FALSE;

	// Make sure we're using a supported fullscreen resolution.
	// @ WARN : MAKE SURE THAT USING A SUPPORTED RESOLUTION AT FULLSCREEN MODE !
	if ( NewFullscreen && !ParentWindow )
	{
		FD3DViewport::GetSupportedResolution(NewSizeX,NewSizeY);
	}

	DWORD	WindowStyle;
	DWORD	WindowStyleEx	= WS_EX_APPWINDOW;
	UBOOL	bShowWindow		= TRUE;
	INT		WindowWidth		= NewSizeX;
	INT		WindowHeight	= NewSizeY;
	INT		WindowPosX		= InPosX;
	INT		WindowPosY		= InPosY;

	// Figure out window style
	if( ParentWindow )
	{
		WindowStyle			= WS_CHILD | WS_CLIPSIBLINGS;
		NewFullscreen		= 0;
	}
	else
	{
		if (NewFullscreen)
		{
			WindowStyle		= WS_POPUP | WS_SYSMENU;
			WindowStyleEx	|= WS_EX_TOPMOST;
		}
		else if (GIsGame)
		{
			WindowStyle		= WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_BORDER | WS_CAPTION;
		}
		else
		{
			WindowStyle		= WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION;
		}
	}

	// Adjust window size based on the styles
	{
		RECT  WindowRect;
		WindowRect.left		= 0;
		WindowRect.top		= 0;
		WindowRect.right	= NewSizeX;
		WindowRect.bottom	= NewSizeY;
		::AdjustWindowRect(&WindowRect,WindowStyle,0);
		WindowWidth			= WindowRect.right - WindowRect.left;
		WindowHeight		= WindowRect.bottom - WindowRect.top;
	}

	// Center the window, unless we're setting the position explicitly
	if( NewFullscreen )
	{
		WindowPosX		= 0;
		WindowPosY		= 0;
		WindowWidth		= NewSizeX;
		WindowHeight	= NewSizeY;
	}
	else if( Window == NULL )
	{
		// Obtain width and height of primary monitor.
		INT ScreenWidth		= ::GetSystemMetrics( SM_CXSCREEN );
		INT ScreenHeight	= ::GetSystemMetrics( SM_CYSCREEN );

		if (WindowPosX == -1)
		{
			if (!Parse(appCmdLine(), TEXT("WindowPosX="), WindowPosX))
			{
				WindowPosX = (ScreenWidth - WindowWidth) / 2;
			}
		}
		if (WindowPosY == -1)
		{
			if (!Parse(appCmdLine(), TEXT("WindowPosY="), WindowPosY))
			{
				WindowPosY = (ScreenHeight - WindowHeight) / 2;
			}
		}
	}
	else
	{
		RECT WindowRect;
		::GetWindowRect( Window, &WindowRect );
		WindowPosX = WindowRect.left;
		WindowPosY = WindowRect.top;
	}

	// If not a child window or fullscreen, clamp the position against the virtual screen.
	if( !ParentWindow && !NewFullscreen )
	{
		RECT ScreenRect;
		ScreenRect.left		= ::GetSystemMetrics( SM_XVIRTUALSCREEN );
		ScreenRect.top		= ::GetSystemMetrics( SM_YVIRTUALSCREEN );
		ScreenRect.right	= ::GetSystemMetrics( SM_CXVIRTUALSCREEN );
		ScreenRect.bottom	= ::GetSystemMetrics( SM_CYVIRTUALSCREEN );

		if ( (WindowPosX + WindowWidth) > ScreenRect.right )
		{
			WindowPosX = ScreenRect.right - WindowWidth;
		}
		if ( WindowPosX < ScreenRect.left )
		{
			WindowPosX = ScreenRect.left;
		}
		if ( (WindowPosY + WindowHeight) > ScreenRect.bottom )
		{
			WindowPosY = ScreenRect.bottom - WindowHeight;
		}
		if ( WindowPosY < ScreenRect.top )
		{
			WindowPosY = ScreenRect.top;
		}
	}

	if( Window == NULL )
	{
		// Reuse the pre-created game window?
		if ( GGameWindow && GUseGameWindowUponResize )
		{
			Window						= GGameWindow;
			GUseGameWindowUponResize	= FALSE;
			SetWindowTextX(Window, *Name);
			bShowWindow = FALSE;
		}
		else
		{
			// Create the window
			Window = CreateWindowExX( WindowStyleEx, *Client->WindowClassName, *Name, WindowStyle, WindowPosX, WindowPosY, WindowWidth, WindowHeight, ParentWindow, NULL, hInstance, this );
		}
		verify( Window );
	}
	else
	{
		bUpdateWindow = TRUE;
	}

	// Is this the game window and it hasn't been displayed yet?
	if ( GGameWindow && Window == GGameWindow && GGameWindowUsingStartupWindowProc )
	{
		GGameWindowStyle	= WindowStyle;
		GGameWindowPosX		= WindowPosX;
		GGameWindowPosY		= WindowPosY;
		GGameWindowWidth	= WindowWidth;
		GGameWindowHeight	= WindowHeight;

		// D3D window proc hook doesn't work if we change it after the device has been created,
		// _IF_ we created D3D in fullscreen mode at startup.
		// This problem manifested itself by not being able to Alt-Tab properly (D3D didn't turn off WS_EX_TOPMOST).
		if ( NewFullscreen )
		{
			extern void appShowGameWindow();
			appShowGameWindow();
		}
	}

	// Fullscreen window?
	if ( NewFullscreen )
	{
		// Hide system cursor when we are in full screen mode.
		while ( ::ShowCursor(FALSE)>=0 );
	}

	// Switching to windowed?
	if ( bWasFullscreen && !NewFullscreen )
	{
		// Show system cursor when we switch to windowed mode.
		while ( ::ShowCursor(TRUE)<0 );

		// Update the render device FIRST (so that the window won't be clamped by the previous resolution).
		UpdateRenderDevice( NewSizeX, NewSizeY, NewFullscreen );

		// Update the window SECOND.
		if ( bUpdateWindow )
		{
			UpdateWindow( NewFullscreen, WindowStyle, WindowPosX, WindowPosY, WindowWidth, WindowHeight );
		}
	}
	else
	{
		// Update the window FIRST.
		if ( bUpdateWindow )
		{
			UpdateWindow( NewFullscreen, WindowStyle, WindowPosX, WindowPosY, WindowWidth, WindowHeight );
		}

		// Update the render device SECOND.
		UpdateRenderDevice( NewSizeX, NewSizeY, NewFullscreen );
	}

	// Show the viewport.
	if ( bShowWindow )
	{
		::ShowWindow( Window, SW_SHOW );
		::UpdateWindow( Window );

		GWindowActive = TRUE;
	}

	UpdateMouseLock();
	bPerformingSize = FALSE;
}

void FWindowsViewport::UpdateWindow( UBOOL NewFullscreen, DWORD WindowStyle, INT WindowPosX, INT WindowPosY, INT WindowWidth, INT WindowHeight )
{
	LONG CurrentWindowStyle = GetWindowLongX(Window, GWL_STYLE);
	LONG CurrentWindowStyleEx = GetWindowLongX(Window, GWL_EXSTYLE);

	// Don't change window style if we don't have to. Calling it will prevent proper refresh behind the window, which
	// looks bad if we're shrinking the window. Note also that we can't use exact equality, since CurrentWindowStyle may
	// contain extra bits, such as WS_CLIPSIBLINGS. At least this check will catch the four cases above.
	// Also, SWP_NOSENDCHANGING must not be set.
	UINT Flags = /*SWP_NOSENDCHANGING |*/ SWP_NOZORDER;
	if ( (CurrentWindowStyle & WindowStyle) != WindowStyle )
	{
		SetWindowLongX(Window, GWL_STYLE, WindowStyle);
		Flags |= SWP_FRAMECHANGED;
	}

	if ( (!ParentWindow && !NewFullscreen) || GFakeFullScreen )
	{
		HWND hWndInsertAfter = HWND_TOP;
		if ( CurrentWindowStyleEx & WS_EX_TOPMOST )
		{
			// Turn off WS_EX_TOPMOST in window mode.
			hWndInsertAfter = HWND_NOTOPMOST;
			Flags &= ~SWP_NOZORDER;
		}
		::SetWindowPos(Window, hWndInsertAfter, WindowPosX, WindowPosY, WindowWidth, WindowHeight, Flags);
	}
	else if (NewFullscreen)
	{

		HWND hWndInsertAfter = HWND_TOP;
		
		::SetWindowPos(Window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|Flags);		

		::SetForegroundWindow(Window);
	}
}

void FWindowsViewport::UpdateRenderDevice( UINT NewSizeX, UINT NewSizeY, UBOOL NewFullscreen )
{
	// Initialize the viewport's render device.
	if( NewSizeX && NewSizeY )
	{
		UpdateViewportRHI(FALSE,NewSizeX,NewSizeY,NewFullscreen);
		// Update system settings which actually saves the resolution
		//GSystemSettings->SetResolution(NewSizeX, NewSizeY, NewFullscreen);
	}
	// #19088: Based on certain startup patterns, there can be a case when all viewports are destroyed, which in turn frees up the D3D device (which results in badness).
	// There are plans to fix the initialization code, but until then hack the known case when a viewport is deleted due to being resized to zero width or height.
	// (D3D does not handle the zero width or zero height case)
	else if( NewSizeX && !NewSizeY )
	{
		NewSizeY = 1;
		UpdateViewportRHI(FALSE,NewSizeX,NewSizeY,NewFullscreen);
	}
	else if( !NewSizeX && NewSizeY )
	{
		NewSizeX = 1;
		UpdateViewportRHI(FALSE,NewSizeX,NewSizeY,NewFullscreen);
	}
	// End hack
	else
	{
		UpdateViewportRHI(TRUE,NewSizeX,NewSizeY,NewFullscreen);
	}
}

//
//	FWindowsViewport::ShutdownAfterError - Minimalist shutdown.
//
void FWindowsViewport::ShutdownAfterError()
{
	if(Window)
	{
		::DestroyWindow(Window);
		Window = NULL;
	}
}

UBOOL FWindowsViewport::HasFocus() const
{
	HWND FocusWindow = ::GetFocus();
	return (FocusWindow == Window);
}

UBOOL FWindowsViewport::IsCursorVisible( ) const
{
	CURSORINFO CursorInfo;
	CursorInfo.cbSize = sizeof(CURSORINFO);
	UBOOL bIsVisible = (::GetCursorInfo( &CursorInfo ) != 0);
	bIsVisible = bIsVisible && UBOOL(CursorInfo.flags & CURSOR_SHOWING) && UBOOL(CursorInfo.hCursor != NULL);
	return bIsVisible;
}

void FWindowsViewport::ShowCursor( UBOOL bVisible )
{
	UBOOL bIsCursorVisible = IsCursorVisible();
	if ( bVisible && !bIsCursorVisible )
	{
		// Restore the old mouse position when we show the cursor.
		if ( PreCaptureMousePos.x >= 0 && PreCaptureMousePos.y >= 0 )
		{
			::SetCursorPos( PreCaptureMousePos.x, PreCaptureMousePos.y );
		}
		while ( ::ShowCursor(TRUE)<0 );
		PreCaptureMousePos.x = -1;
		PreCaptureMousePos.y = -1;
	}
	else if ( !bVisible && bIsCursorVisible )
	{
		while ( ::ShowCursor(FALSE)>=0 );

		// Remember the current mouse position when we hide the cursor.
		PreCaptureMousePos.x = -1;
		PreCaptureMousePos.y = -1;
		::GetCursorPos(&PreCaptureMousePos);
	}
}

void FWindowsViewport::CaptureMouse( UBOOL bCapture )
{
#if USE_DIRECTINPUT_MOUSEBUTTON
	if (!GIsEditor)
		return;
#endif

	HWND CaptureWindow = ::GetCapture();
	UBOOL bIsMouseCaptured = (CaptureWindow == Window);
	bCapturingMouseInput = bCapture;
	if ( bCapture && !bIsMouseCaptured )
	{
		::SetCapture( Window );
		UWindowsClient::FlushMouseInput();
	}
	else if ( !bCapture && bIsMouseCaptured )
	{
		::ReleaseCapture();
	}
}

void FWindowsViewport::UpdateMouseLock()
{
	// If we're the foreground window, let us decide whether the system cursor should be visible or not.
	UBOOL bIsForeground = (::GetForegroundWindow() == Window);
	UBOOL bIsSystemCursorVisible;
	if ( bIsForeground )
	{
		bIsSystemCursorVisible = (SystemMouseCursor != MC_None);
	}
	else
	{
		bIsSystemCursorVisible = IsCursorVisible();
	}

	RECT ClipRect;
	UBOOL bIsGameCursorVisible = (GEngine && GEngine->GameViewport && GEngine->GameViewport->bDisplayingUIMouseCursor);
	UBOOL bIsAnyCursorVisible = bIsSystemCursorVisible || bIsGameCursorVisible;
	UBOOL bClipRectValid = (::GetClipCursor( &ClipRect ) != 0);
	UBOOL bShouldMouseLock = HasFocus() && (IsFullscreen() || !bIsAnyCursorVisible || bMouseLockRequested);	

	//@HACK : deif
	bClipRectValid = FALSE;

	// @todo: The following doesn't handle a window with a mouse lock changing position or size.  Not a big deal
	// @todo: Doesn't handle a window being destroyed that had a mouse lock.  Uncommon.

	UBOOL bNewPreventTaskSwitching = FALSE;

	// Update mouse lock
	if ( bShouldMouseLock )
	{
		RECT ClientRect;
		::GetClientRect( Window, &ClientRect );
		::MapWindowPoints( Window, NULL, (POINT*)&ClientRect, 2 );

		if ( !bClipRectValid ||
			ClientRect.top != GMouseLockClientRect.top ||
			ClientRect.left != GMouseLockClientRect.left ||
			ClientRect.bottom != GMouseLockClientRect.bottom ||
			ClientRect.right != GMouseLockClientRect.right )
		{
			::ClipCursor( &ClientRect );

			// The rect we just set may have been clipped by the screen rect.
			UBOOL bClipRectValid = (::GetClipCursor( &ClipRect ) != 0);
			if ( bClipRectValid )
			{
				// NOTE: We store the ClientRect, not the ClipRect, since that's what we'll be testing against later!
				GMouseLockClientRect = ClientRect;
				MouseLockClientRect = ClientRect;

				bNewPreventTaskSwitching = IsFullscreen() && !bIsGameCursorVisible;
			}
			else
			{
				GMouseLockClientRect.top = GMouseLockClientRect.left = GMouseLockClientRect.bottom = GMouseLockClientRect.top = 0;
				MouseLockClientRect.top = MouseLockClientRect.left = MouseLockClientRect.bottom = MouseLockClientRect.right = 0;
				::ClipCursor(NULL);
			}
		}
	}
	else
	{
		// Is the current system lock rect the one _our_ viewport set? Only unlock if it is.
		if ( !bClipRectValid ||
			(GMouseLockClientRect.top == MouseLockClientRect.top &&
			GMouseLockClientRect.left == MouseLockClientRect.left &&
			GMouseLockClientRect.bottom == MouseLockClientRect.bottom &&
			GMouseLockClientRect.right == MouseLockClientRect.right) )
		{
			GMouseLockClientRect.top = GMouseLockClientRect.left = GMouseLockClientRect.bottom = GMouseLockClientRect.top = 0;
			MouseLockClientRect.top = MouseLockClientRect.left = MouseLockClientRect.bottom = MouseLockClientRect.right = 0;
			::ClipCursor( NULL );
		}
	}

	extern UBOOL GPreventTaskSwitching;
	GPreventTaskSwitching = bNewPreventTaskSwitching;
}

UBOOL FWindowsViewport::UpdateMouseCursor( UBOOL bSetCursor )
{
	UBOOL bHandled = FALSE;
	HCURSOR NewCursor = NULL;
	if( IsFullscreen() )
	{
		bHandled = bSetCursor;
		SystemMouseCursor = MC_None;
	}
	else
	{
		INT		MouseX		= GetMouseX();
		INT		MouseY		= GetMouseY();
		SystemMouseCursor	= MC_Arrow;
		if(ViewportClient)
		{
			SystemMouseCursor = ViewportClient->GetCursor(this,MouseX,MouseY);
		}
		if( MouseX < 0 || MouseY < 0 || MouseX >= (INT)GetSizeX() || MouseY >= (INT)GetSizeY())
		{
			// Don't set the cursor if the mouse isn't in the client area.
			bHandled = FALSE;
		}
		else if ( SystemMouseCursor == MC_None && HasFocus() )
		{
			bHandled = bSetCursor;
		}
		else if(SystemMouseCursor == MC_NoChange)
		{
			// Intentionally do not set the cursor.
			return TRUE;
		}
		else
		{
			LPCTSTR	CursorResource = IDC_ARROW;
			switch(SystemMouseCursor)
			{
			case MC_Arrow:				CursorResource = IDC_ARROW; break;
			case MC_Cross:				CursorResource = IDC_CROSS; break;
			case MC_SizeAll:			CursorResource = IDC_SIZEALL; break;
			case MC_SizeUpRightDownLeft:CursorResource = IDC_SIZENESW; break;
			case MC_SizeUpLeftDownRight:CursorResource = IDC_SIZENWSE; break;
			case MC_SizeLeftRight:		CursorResource = IDC_SIZEWE; break;
			case MC_SizeUpDown:			CursorResource = IDC_SIZENS; break;
			case MC_Hand:				CursorResource = IDC_HAND; break;
			};
			NewCursor = ::LoadCursor(NULL, CursorResource);
			bHandled = bSetCursor;
		}
	}
	if ( bHandled )
	{
		::SetCursor( NewCursor );
	}
	return bHandled;
}

void FWindowsViewport::LockMouseToWindow(UBOOL bLock)
{
	bMouseLockRequested = bLock;
	UpdateMouseLock();
}

//
//	FWindowsViewport::CaptureJoystickInput
//
UBOOL FWindowsViewport::CaptureJoystickInput(UBOOL Capture)
{
	bCapturingJoystickInput	= Capture;

	return bCapturingJoystickInput;
}

//
//	FWindowsViewport::KeyState
//
UBOOL FWindowsViewport::KeyState(FName Key) const
{
	BYTE* VirtualKey = Client ? Client->KeyMapNameToVirtual.Find(Key) : NULL;
	if( VirtualKey )
	{
		return KeyStates[*VirtualKey];
	}
	else
	{
		return FALSE;
	}
}

//
//	FWindowsViewport::GetMouseX
//
INT FWindowsViewport::GetMouseX()
{
	POINT P;
	::GetCursorPos( &P );
	::ScreenToClient( Window, &P );
	return P.x;
}

//
//	FWindowsViewport::GetMouseY
//
INT FWindowsViewport::GetMouseY()
{
	POINT P;
	::GetCursorPos( &P );
	::ScreenToClient( Window, &P );
	return P.y;
}

void FWindowsViewport::GetMousePos( FIntPoint& MousePosition )
{
	POINT P;
	::GetCursorPos( &P );
	::ScreenToClient( Window, &P );
	MousePosition.X = P.x;
	MousePosition.Y = P.y;
}

void FWindowsViewport::SetMouse(INT x, INT y)
{
	PreCaptureMousePos.x = x;
	PreCaptureMousePos.y = y;
	::ClientToScreen(Window, &PreCaptureMousePos);
	::SetCursorPos(PreCaptureMousePos.x, PreCaptureMousePos.y);
}

//
//	FWindowsViewport::InvalidateDisplay
//

void FWindowsViewport::InvalidateDisplay()
{
	::InvalidateRect(Window,NULL,0);
}

FViewportFrame* FWindowsViewport::GetViewportFrame()
{
	return ParentWindow ? NULL : this;
}

//
//	FWindowsViewport::ProcessInput
//
void FWindowsViewport::ProcessInput( FLOAT DeltaTime )
{
	if( !ViewportClient )
	{
		return;
	}

#if WITH_PANORAMA
	extern UBOOL appIsPanoramaGuideOpen(void);
	/** If the Live guide is open, don't consume it's input */
	if (appIsPanoramaGuideOpen())
	{
		// Also pause any waveforms being played
		if ((bCapturingJoystickInput || ViewportClient->RequiresUncapturedAxisInput()) && HasFocus() )
		{
			XINPUT_VIBRATION Feedback = {0};
			for (INT JoystickIndex = 0; JoystickIndex < UWindowsClient::Joysticks.Num(); JoystickIndex++)
			{
				// This will turn off the vibration
				XInputSetState(JoystickIndex,&Feedback);
			}
		}
		return;
	}
#endif

	if( Client->bAllowJoystickInput && HasFocus() && (bCapturingJoystickInput || ViewportClient->RequiresUncapturedAxisInput()) )
	{
		for(INT JoystickIndex = 0;JoystickIndex < UWindowsClient::Joysticks.Num();JoystickIndex++)
		{
			DIJOYSTATE2 State;
			XINPUT_STATE XIstate;
			FJoystickInfo& JoystickInfo = UWindowsClient::Joysticks(JoystickIndex);
			UBOOL bIsConnected = FALSE;

			ZeroMemory( &XIstate, sizeof(XIstate) );
			ZeroMemory( &State, sizeof(State) );

			if ( JoystickInfo.DirectInput8Joystick )
			{
				// Focus issues with Viewports: Force gamepad input
				JoystickInfo.DirectInput8Joystick->Unacquire();
				JoystickInfo.DirectInput8Joystick->SetCooperativeLevel(GetForegroundWindow(),DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
				bIsConnected = ( SUCCEEDED(JoystickInfo.DirectInput8Joystick->Acquire()) && SUCCEEDED(JoystickInfo.DirectInput8Joystick->Poll()) );
				if( bIsConnected && SUCCEEDED(JoystickInfo.DirectInput8Joystick->GetDeviceState(sizeof(DIJOYSTATE2), &State)) )
				{
					// Store state
					JoystickInfo.PreviousState = State;
				}
				else
				{
					// if failed to get device state, use previous one.
					// this seems to happen quite frequently unfortunately :(
					State = JoystickInfo.PreviousState;
				}
			}
			else if ( JoystickIndex < JOYSTICK_NUM_XINPUT_CONTROLLERS )
			{
				// Simply get the state of the controller from XInput.
				if ( JoystickInfo.bIsConnected )
				{
					JoystickInfo.bIsConnected = ( XInputGetState( JoystickIndex, &XIstate ) == ERROR_SUCCESS ) ? TRUE : FALSE;
				}
				bIsConnected = JoystickInfo.bIsConnected;
			}

			if ( bIsConnected )
			{
				// see the UWindowsClient::UWindowsClient calls below for which slots in this array map to which names
				// 1 means pressed, 0 means not pressed
				UBOOL CurrentStates[MAX_JOYSTICK_BUTTONS] = {0};

				if( JoystickInfo.JoystickType == JOYSTICK_Xbox_Type_S )
				{
					// record our current button pressed state
					for (INT ButtonIndex = 0; ButtonIndex < 12; ButtonIndex++)
					{
						CurrentStates[ButtonIndex] = State.rgbButtons[ButtonIndex];
					}
					CurrentStates[12] = State.rgdwPOV[0] == 0;
					CurrentStates[13] = State.rgdwPOV[0] == 18000;
					CurrentStates[14] = State.rgdwPOV[0] == 27000;
					CurrentStates[15] = State.rgdwPOV[0] == 9000;

					// Axis, convert range 0..65536 set up in EnumAxesCallback to +/- 1.
					ViewportClient->InputAxis( this, JoystickInfo.ControllerId, KEY_XboxTypeS_LeftX	, (State.lX  - 32768.f) / 32768.f, DeltaTime/*, TRUE*/ );
					ViewportClient->InputAxis( this, JoystickInfo.ControllerId, KEY_XboxTypeS_LeftY	, -(State.lY  - 32768.f) / 32768.f, DeltaTime/*, TRUE*/ );
					ViewportClient->InputAxis( this, JoystickInfo.ControllerId, KEY_XboxTypeS_RightX	, (State.lRx - 32768.f) / 32768.f, DeltaTime/*, TRUE*/ );
					ViewportClient->InputAxis( this, JoystickInfo.ControllerId, KEY_XboxTypeS_RightY	, (State.lRy - 32768.f) / 32768.f, DeltaTime/*, TRUE*/ );
				}
				// Support X360 controller
				else if( JoystickInfo.JoystickType == JOYSTICK_X360 )
				{
					CurrentStates[Client->X360ToXboxControllerMapping[0]] = XIstate.Gamepad.wButtons & XINPUT_GAMEPAD_A;
					CurrentStates[Client->X360ToXboxControllerMapping[1]] = XIstate.Gamepad.wButtons & XINPUT_GAMEPAD_B;
					CurrentStates[Client->X360ToXboxControllerMapping[2]] = XIstate.Gamepad.wButtons & XINPUT_GAMEPAD_X;
					CurrentStates[Client->X360ToXboxControllerMapping[3]] = XIstate.Gamepad.wButtons & XINPUT_GAMEPAD_Y;
					CurrentStates[Client->X360ToXboxControllerMapping[4]] = XIstate.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
					CurrentStates[Client->X360ToXboxControllerMapping[5]] = XIstate.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
					CurrentStates[Client->X360ToXboxControllerMapping[6]] = XIstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK;
					CurrentStates[Client->X360ToXboxControllerMapping[7]] = XIstate.Gamepad.wButtons & XINPUT_GAMEPAD_START;
					CurrentStates[Client->X360ToXboxControllerMapping[8]] = XIstate.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB;
					CurrentStates[Client->X360ToXboxControllerMapping[9]] = XIstate.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB;

					CurrentStates[Client->X360ToXboxControllerMapping[10]] = XIstate.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
					CurrentStates[Client->X360ToXboxControllerMapping[11]] = XIstate.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD;

					// record our current button pressed state
					CurrentStates[Client->X360ToXboxControllerMapping[12]] = XIstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP;
					CurrentStates[Client->X360ToXboxControllerMapping[13]] = XIstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
					CurrentStates[Client->X360ToXboxControllerMapping[14]] = XIstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
					CurrentStates[Client->X360ToXboxControllerMapping[15]] = XIstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;

					// Axis, convert range -32768..+32767 set up in EnumAxesCallback to +/- 1.
					ViewportClient->InputAxis( this, JoystickInfo.ControllerId, KEY_XboxTypeS_LeftX	, XIstate.Gamepad.sThumbLX / 32768.f, DeltaTime/*, TRUE*/ );
					ViewportClient->InputAxis( this, JoystickInfo.ControllerId, KEY_XboxTypeS_LeftY	, XIstate.Gamepad.sThumbLY / 32768.f, DeltaTime/*, TRUE*/ );
					ViewportClient->InputAxis( this, JoystickInfo.ControllerId, KEY_XboxTypeS_RightX, XIstate.Gamepad.sThumbRX / 32768.f, DeltaTime/*, TRUE*/ );
					// this needs to be inverted as the XboxTypeS axis are flipped from the "norm"
					ViewportClient->InputAxis( this, JoystickInfo.ControllerId, KEY_XboxTypeS_RightY, -XIstate.Gamepad.sThumbRY / 32768.f, DeltaTime/*, TRUE*/ );

					if( GWorld->IsPaused() )
					{
						// Stop vibration when game is paused
						for(INT i=0; i<GEngine->GamePlayers.Num(); i++)
						{
							ULocalPlayer* LP = GEngine->GamePlayers(i);
							if(LP && LP->Actor && LP->Actor->ForceFeedbackManager)
							{
								LP->Actor->ForceFeedbackManager->bIsPaused = TRUE;
								if( LP->Actor->ForceFeedbackManager )
								{
									LP->Actor->ForceFeedbackManager->ApplyForceFeedback(JoystickIndex,DeltaTime);
								}
							}
						}
					}
					else
					{
						// Now update any waveform data
						UForceFeedbackManager* Manager = ViewportClient->GetForceFeedbackManager(JoystickIndex);
						if (Manager != NULL)
						{
							Manager->ApplyForceFeedback(JoystickIndex,DeltaTime);
						}
					}
				}
				else if( JoystickInfo.JoystickType == JOYSTICK_PS2_Old_Converter || JoystickInfo.JoystickType == JOYSTICK_PS2_New_Converter )
				{
					// PS2 controller has to be mapped funny, since we use Xbox button mapping below
					for (INT ButtonIndex = 0; ButtonIndex < 12; ButtonIndex++)
					{
						CurrentStates[Client->PS2ToXboxControllerMapping[ButtonIndex]] = State.rgbButtons[ButtonIndex];
					}

					CurrentStates[12] = State.rgdwPOV[0] == 0;
					CurrentStates[13] = State.rgdwPOV[0] == 18000;
					CurrentStates[14] = State.rgdwPOV[0] == 27000;
					CurrentStates[15] = State.rgdwPOV[0] == 9000;

					// Axis, convert range 0..65536 set up in EnumAxesCallback to +/- 1.
					ViewportClient->InputAxis( this, JoystickInfo.ControllerId, KEY_XboxTypeS_LeftX			, (State.lX  - 32768.f) / 32768.f, DeltaTime/*, TRUE*/ );
					ViewportClient->InputAxis( this, JoystickInfo.ControllerId, KEY_XboxTypeS_LeftY			, (State.lY  - 32768.f) / 32768.f, DeltaTime/*, TRUE*/ );
					if( JoystickInfo.JoystickType == JOYSTICK_PS2_Old_Converter )
					{
						ViewportClient->InputAxis( this, JoystickInfo.ControllerId, KEY_XboxTypeS_RightX, (State.lRz - 32768.f) / 32768.f, DeltaTime/*, TRUE*/ );
						ViewportClient->InputAxis( this, JoystickInfo.ControllerId, KEY_XboxTypeS_RightY, (State.lZ  - 32768.f) / 32768.f, DeltaTime/*, TRUE*/ );
					}
					else
					{
						ViewportClient->InputAxis( this, JoystickInfo.ControllerId, KEY_XboxTypeS_RightX, (State.lZ  - 32768.f) / 32768.f, DeltaTime/*, TRUE*/ );
						ViewportClient->InputAxis( this, JoystickInfo.ControllerId, KEY_XboxTypeS_RightY, (State.lRz - 32768.f) / 32768.f, DeltaTime/*, TRUE*/ );
					}
				}

				const DOUBLE CurrentTime = appSeconds();

				// Buttons (treated as digital buttons).
				for (INT ButtonIndex = 0; ButtonIndex < MAX_JOYSTICK_BUTTONS; ButtonIndex++)
				{
					if (CurrentStates[ButtonIndex] != JoystickInfo.JoyStates[ButtonIndex])
					{
						ViewportClient->InputKey(this, JoystickInfo.ControllerId, Client->JoyNames[ButtonIndex], CurrentStates[ButtonIndex] ? IE_Pressed : IE_Released, 1.f/*, TRUE*/ );
						if ( CurrentStates[ButtonIndex] != 0 )
						{
							// this button was pressed - set the button's NextRepeatTime to the InitialButtonRepeatDelay
							JoystickInfo.NextRepeatTime[ButtonIndex] = CurrentTime + Client->InitialButtonRepeatDelay;
						}
					}
					else if ( CurrentStates[ButtonIndex] != 0 && JoystickInfo.NextRepeatTime[ButtonIndex] <= CurrentTime )
					{
						// it is time to generate an IE_Repeat event for this joystick button
						ViewportClient->InputKey(this, JoystickInfo.ControllerId, Client->JoyNames[ButtonIndex], IE_Repeat, 1.f, TRUE );

						// set the button's NextRepeatTime to the ButtonRepeatDelay
						JoystickInfo.NextRepeatTime[ButtonIndex] = CurrentTime + Client->ButtonRepeatDelay;
					}
				}

				// update last frames state
				appMemcpy(JoystickInfo.JoyStates, CurrentStates, sizeof(JoystickInfo.JoyStates));
			}
		}
	}

	// Process keyboard input.

	for( UINT KeyIndex = 0;KeyIndex < 256;KeyIndex++ )
	{
		FName* KeyName = Client->KeyMapVirtualToName.Find(KeyIndex);

		if(  KeyName )
		{
			UBOOL NewKeyState = ::GetKeyState(KeyIndex) & 0x8000;

			// wxWindows does not let you tell the left and right shift apart, so the problem is pressing the right shift
			// results in a left shift event. Then this code looks to see if left shift is down, and it isn't, so it calls
			// a Released event straight away. This code means that both shift have to be up to generate a Release event.
			// This isn't perfect, but means you can use the right shift as a modifier in the editor etc.
			if(KeyIndex == VK_RSHIFT)
			{
				NewKeyState = NewKeyState || ::GetKeyState(VK_LSHIFT) & 0x8000;
			}
			else if(KeyIndex == VK_LSHIFT)
			{
				NewKeyState = NewKeyState || ::GetKeyState(VK_RSHIFT) & 0x8000;
			}


			if (*KeyName != KEY_LeftMouseButton 
				&&  *KeyName != KEY_RightMouseButton 
				&&  *KeyName != KEY_MiddleMouseButton 
				&&	*KeyName != KEY_XMouseButton
				&&	*KeyName != KEY_YMouseButton)
			{
				if( !NewKeyState && KeyStates[KeyIndex] )
				{
					KeyStates[KeyIndex] = FALSE;
					ViewportClient->InputKey(this,0,*KeyName,IE_Released);
				}
			}
			else if ( GIsGame && KeyStates[KeyIndex] )
			{
				//@todo. The Repeat event should really be sent in both editor and game,
				// but the editor viewport input handlers need to be updated accordingly.
				// This would involve changing the lock mouse to windows functionality 
				// (which does not check the button for a press event, but simply 
				// captures if the mouse button state is true - which is the case for
				// repeats). It would also require verifying each function makes no 
				// assumptions about what event is passed in.
				ViewportClient->InputKey(this, 0, *KeyName, IE_Repeat);
			}
		}
	}

#if WITH_IME
	// Tick/init the input method editor
	bSupportsIME = CheckIMESupport( Window );
#endif
}

//
//	FWindowsViewport::HandlePossibleSizeChange
//
void FWindowsViewport::HandlePossibleSizeChange()
{
	// If ViewportClient has been cleared, the window is being destroyed, and size changes don't matter.
	if(ViewportClient)
	{
		RECT WindowClientRect;
		::GetClientRect( Window, &WindowClientRect );

		UINT NewSizeX = WindowClientRect.right - WindowClientRect.left;
		UINT NewSizeY = WindowClientRect.bottom - WindowClientRect.top;

		if(!IsFullscreen() && (NewSizeX != GetSizeX() || NewSizeY != GetSizeY()))
		{
			Resize( NewSizeX, NewSizeY, 0 );

			if(ViewportClient)
				ViewportClient->ReceivedFocus(this);
		}
	}
}

void UWindowsClient::DeferMessage(FWindowsViewport* Viewport,UINT Message,WPARAM wParam,LPARAM lParam)
{
	// Add the message to the queue.
	FDeferredMessage DeferredMessage;
	DeferredMessage.Viewport = Viewport;
	DeferredMessage.Message = Message;
	DeferredMessage.wParam = wParam;
	DeferredMessage.lParam = lParam;
	DeferredMessage.KeyStates.LeftControl = ::GetKeyState(VK_LCONTROL);
	DeferredMessage.KeyStates.RightControl = ::GetKeyState(VK_RCONTROL);
	DeferredMessage.KeyStates.LeftShift = ::GetKeyState(VK_LSHIFT);
	DeferredMessage.KeyStates.RightShift = ::GetKeyState(VK_RSHIFT);
	DeferredMessage.KeyStates.Menu = ::GetKeyState(VK_MENU);
	DeferredMessages.AddItem(DeferredMessage);
}

void UWindowsClient::ProcessDeferredMessages()
{
	for(INT MessageIndex = 0;MessageIndex < DeferredMessages.Num();MessageIndex++)
	{
		// Make a copy of the deferred message before we process it.  We need to do this because new messages
		// may be added to the DiferredMessages array while we're processing them!
		FDeferredMessage DeferredMessageCopy = DeferredMessages( MessageIndex );
		DeferredMessages(MessageIndex).Viewport->ProcessDeferredMessage( DeferredMessageCopy );
	}
	DeferredMessages.Empty();
}

//
//	FWindowsViewport::ViewportWndProc - Main viewport window function.
//
LONG FWindowsViewport::ViewportWndProc( UINT Message, WPARAM wParam, LPARAM lParam )
{
	// Process any enqueued UpdateModifierState requests.
	if ( bUpdateModifierStateEnqueued )
	{
		UpdateModifierState();
	}

	if( !Client->ProcessWindowsMessages || Client->Viewports.FindItemIndex(this) == INDEX_NONE )
	{
		return DefWindowProcX(Window,Message,wParam,lParam);
	}

	// Message handler.
	switch(Message)
	{
	case WM_CLOSE:
	case WM_CHAR:
	case WM_SYSCHAR:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_XBUTTONDBLCLK:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_XBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_XBUTTONUP:
	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		Client->DeferMessage(this,Message,wParam,lParam);
		return 0;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:		
		Client->DeferMessage(this,Message,wParam,lParam);
		return 1;

	case WM_PAINT:
	case WM_SETCURSOR:
		// Painting and cursor updates only need to happen once per frame, so they replace previously deferred messages of the same type.
		Client->DeferMessage(this,Message,wParam,lParam);
		return DefWindowProcX(Window,Message,wParam,lParam);

	case WM_MOUSEMOVE:
	case WM_CAPTURECHANGED:
	case WM_ACTIVATE:
	case WM_ENTERSIZEMOVE:
	case WM_EXITSIZEMOVE:
	case WM_SIZE:
	case WM_SIZING:
		Client->DeferMessage(this,Message,wParam,lParam);
		return DefWindowProcX(Window,Message,wParam,lParam);

	case WM_ACTIVATEAPP:
		if( wParam == TRUE )
		{
			GWindowActive = TRUE;

			// Fake full screen에서는 minimize하지 않는다.
			if (IsFullscreen() && !GFakeFullScreen)
			{
				MinimizeOtherWindows(Window);
			}
		}
		else
		{
			GWindowActive = FALSE;
		}
		return DefWindowProcX( Window, Message, wParam, lParam);

	case WM_DESTROY:
		Window = NULL;
#ifdef WITH_IME
		if( hDllIme )
			FreeLibrary( hDllIme );
		if( hDllImm32 )
			FreeLibrary( hDllImm32 );
		hDllIme = NULL;
		hDllImm32 = NULL;
#endif
		return 0;

	case WM_MOUSEACTIVATE:
		if(LOWORD(lParam) != HTCLIENT)
		{
			// The activation was the result of a mouse click outside of the client area of the window.
			// Prevent the mouse from being captured to allow the user to drag the window around.
			PreventCapture = 1;
		}
		Client->DeferMessage(this,Message,wParam,lParam);
		return MA_ACTIVATE;

		// Focus issues with Viewports: Viewports are owned by a wxPanel window (to wrap this plain Windows code into wxWidgets I guess),
		// which makes all these messages become processed by IsDialogMessage(). As part of that, we need to tell IsDialogMessage that
		// we want all WM_CHARS and don't let Windows beep for unprocessed chars in WM_KEYDOWN.
		// I never figured out why bound keys don't beep in WM_KEYDOWN (like spacebar) and un-bound keys beep... :(   -Smedis
	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;

	case WM_DISPLAYCHANGE:
		return 0;

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 160;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 120; 
		return DefWindowProcX( Window, Message, wParam, lParam );

#if WITH_IME
	case WM_INPUTLANGCHANGE:
		{
			OnInputLangChanged( Window, wParam, lParam);
			if( ViewportClient )
			{
				FInputCompositionStringData CompStrData(IMECOMPOSITION_InputLangChanged);
				CompStrData.CompCursorPos = ImeProperty & IME_PROP_AT_CARET;
				ViewportClient->InputChar(this, 0, CompStrData);
			}
			return DefWindowProcX( Window, Message, wParam, lParam );
		}
	case WM_IME_NOTIFY :
		{
			Client->DeferMessage(this,Message,wParam,lParam);
			// WM_IME_NOTIFY의 DefaultProc를 하지 않으면 중국어 입력시 부자연스러운 입력 전환이 생긴다.
			// 예) 大寒(dahan)을 입력한다고 하면 MSPY에서 dahan을 입력하면
			// 大汗이 입력된다. 여기서 스페이스바를 누르고 커서위치 1번에서 편집에 들어가면 ( 커서위치 0大1汗2 )
			// 大寒으로 변경이 가능하다. 그런데 이글자가 원하지 않는 글자라고 생각하고 좌우키를 눌러 커서를 0 또는 2로 바꾸면
			// 원래 글자인 大汗으로 변경되어야하지만 DefaultProc의 지원을 받지 않으면 大汗으로 '당장' 바뀌지 않는다.
			// 그러나 IME 내부에서는 바뀌어있다. IME와 현재 사용중인 컨트럴간에 불일치가 생기므로 혼란스럽다.
			//
			// 결론적으로 WM_IME_NOTIFY의 DefaultProc를 사용하되 IME Window창을 숨겨주는것이 바람직해보인다.

			// ReadingString Window는 글자 조합과 관계가 없으므로 숨긴다.
			if( wParam == IMN_PRIVATE )
				return 0;

			if( wParam == IMN_OPENCANDIDATE || wParam == IMN_CHANGECANDIDATE )
			{
				bOpenCandidate = TRUE;
			}
			else if ( wParam == IMN_CLOSECANDIDATE )
			{
				ImePopupWindows.Empty();
				bOpenCandidate = FALSE;
			}

			return DefWindowProcX( Window, Message, wParam, lParam );
		}
	case WM_IME_COMPOSITION:
		{
			Client->DeferMessage(this,Message,wParam,lParam);
			
			HideImePopupWindows( Window );

			if( lParam & GCS_RESULTSTR )
			{
				return 0;	
			}
			else if( lParam & GCS_COMPSTR ) 
			{
				return 0;
			}
			else
			{
				return DefWindowProcX( Window, Message, wParam, lParam );
			}
		}

	case WM_IME_STARTCOMPOSITION:
		Client->DeferMessage(this,Message,wParam,lParam);
		// 왼쪽위에 뜨는 키입력창을 막으려면 DefWndProc없이 리턴해야한다.
		return 0;

	case WM_IME_ENDCOMPOSITION:
		Client->DeferMessage(this,Message,wParam,lParam);
		return 0;

	case WM_IME_CHAR:
		// WM_IME_CHAR 처리 불필요 : WM_IME_CHAR는 완성된 한글자가 들어올때만 이벤트 발생 ( 조합문자 처리불가 )
		// WM_IME_CHAR 리턴 이유 : DefWndProc처리하면 WM_CHAR를 다시한번 부른다.
		return 0;
#endif

	case WM_ERASEBKGND:
		return 1;

	case WM_POWERBROADCAST:
		//Prevent power management
		switch( wParam )
		{
		case PBT_APMQUERYSUSPEND:
		case PBT_APMQUERYSTANDBY:
			return BROADCAST_QUERY_DENY;
		}
		return DefWindowProcX( Window, Message, wParam, lParam );

	case WM_SYSCOMMAND:
		// Prevent moving/sizing and power loss in fullscreen mode.
		switch( wParam & 0xfff0 )
		{
		case SC_SCREENSAVE:
			return 1;

		case SC_MOVE:
		case SC_SIZE:
		case SC_MAXIMIZE:
		case SC_KEYMENU:
		case SC_MONITORPOWER:
			if( IsFullscreen() )
				return 1;
			break;
		case SC_RESTORE:
			if( IsIconic(Window) )
			{
				::ShowWindow(Window,SW_RESTORE);
				return 0;
			}
		}
		return DefWindowProcX( Window, Message, wParam, lParam );

	case WM_NCHITTEST:
		// Prevent the user from selecting the menu in fullscreen mode.
		if( IsFullscreen() )
			return HTCLIENT;
		return DefWindowProcX( Window, Message, wParam, lParam );

	case WM_POWER:
		if( PWR_SUSPENDREQUEST == wParam )
		{
			debugf( NAME_Log, TEXT("Received WM_POWER suspend") );
			return PWR_FAIL;
		}
		else
			debugf( NAME_Log, TEXT("Received WM_POWER") );
		return DefWindowProcX( Window, Message, wParam, lParam );

	case WM_QUERYENDSESSION:
		debugf( NAME_Log, TEXT("Received WM_QUERYENDSESSION") );
		return DefWindowProcX( Window, Message, wParam, lParam );

	case WM_ENDSESSION:
		if ( wParam )
		{
			debugf( NAME_Log, TEXT("Received WM_ENDSESSION") );
			appRequestExit( FALSE );
			return TRUE;
		}
		return DefWindowProcX( Window, Message, wParam, lParam );

		// The WM_SYNCPAINT message is used to synchronize painting while avoiding linking independent GUI threads.
	case WM_SYNCPAINT:
		// 전체 화면에서 스크린 세이버에서 돌아올 경우 해상도를 복구해 준다.(2007/07/06)
		if ( IsFullscreen() )
			RestoreDisplaySettings(GetSizeX(), GetSizeY());
		return DefWindowProcX( Window, Message, wParam, lParam );	
	
		/////////////////////////////////////////////////////////////////////////
		// WebInClient support
#ifdef _WEB_IN_CLIENT
	case WM_COPYDATA:
		if ((HWND)wParam == _WebInClient().GetWICHandle())
		{
			COPYDATASTRUCT *pData = (COPYDATASTRUCT*)lParam;
			FString Param = ANSI_TO_TCHAR((LPCSTR)pData->lpData);
			_WebInClient().ProcMessage(Param);
		}
		return DefWindowProcX( Window, Message, wParam, lParam );
#endif
		/////////////////////////////////////////////////////////////////////////


	default:
		return DefWindowProcX( Window, Message, wParam, lParam );
	}
}

void FWindowsViewport::ProcessDeferredMessage(const FDeferredMessage& DeferredMessage)
{
	// Helper class to aid in sending callbacks, but still let each message return in their case statements
	class FCallbackHelper
	{
		FViewport* Viewport;
	public:
		FCallbackHelper(FViewport* InViewport, UINT InMessage) 
			:	Viewport( InViewport )
		{ 
			GCallbackEvent->Send( CALLBACK_PreWindowsMessage, Viewport, InMessage ); 
		}
		~FCallbackHelper() 
		{ 
			GCallbackEvent->Send( CALLBACK_PostWindowsMessage, Viewport, 0 ); 
		}
	};

	const WPARAM wParam = DeferredMessage.wParam;
	const LPARAM lParam = DeferredMessage.lParam;

	// Message handler.
	switch(DeferredMessage.Message)
	{
	case WM_CLOSE:
		if( ViewportClient )
			ViewportClient->CloseRequested(this);
		break;

	case WM_PAINT:
		if(ViewportClient)
			ViewportClient->RedrawRequested(this);
		break;

	case WM_ACTIVATE:
		/*switch (LOWORD(wParam))
		{
		case WA_ACTIVE:
		case WA_CLICKACTIVE:
			if( GEngine != NULL )
			{
				GEngine->OnLostFocusPause(FALSE);
			}
			break;
		case WA_INACTIVE:
			if( GEngine != NULL )
			{
				GEngine->OnLostFocusPause(TRUE);
			}
			break;
		}*/
		break;

	case WM_MOUSEMOVE:
		if( !bCapturingMouseInput )
		{
			// the destruction of this will send the PostWindowsMessage callback
			FCallbackHelper CallbackHelper(this, DeferredMessage.Message);

			INT	X = GET_X_LPARAM(lParam);
			INT Y = GET_Y_LPARAM(lParam);

			if(ViewportClient)
			{
				ViewportClient->MouseMove(this,X,Y);
			}
		}
		break;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		{
			// the destruction of this will send the PostWindowsMessage callback
			FCallbackHelper CallbackHelper(this, DeferredMessage.Message);

			// Get key code.
			UINT KeyCode = 0;
			FName* Key = NULL;
			if(GetUnKeyName( DeferredMessage.Message, wParam, lParam, Key, KeyCode ))
			{
				// Update the cached key state.
				UBOOL OldKeyState = KeyStates[KeyCode];
				KeyStates[KeyCode] = TRUE;

				UBOOL bEnableAltEnterResize = FALSE;
#if !FINAL_RELEASE
				bEnableAltEnterResize = TRUE;
#endif				

				// Send key to input system.
				if( *Key==KEY_Enter && (DeferredMessage.KeyStates.Menu & 0x8000) )
				{
					if (bEnableAltEnterResize)
					{
						Resize(GetSizeX(),GetSizeY(),!IsFullscreen());
					}					
				}

				if( ViewportClient )
				{
					ViewportClient->InputKey(this,0,*Key,OldKeyState ? IE_Repeat : IE_Pressed);
				}
			}
		}
		break;

	case WM_KEYUP:
	case WM_SYSKEYUP:
		{
			// the destruction of this will send the PostWindowsMessage callback
			FCallbackHelper CallbackHelper(this, DeferredMessage.Message);

			// Get key code.
			UINT KeyCode = 0;
			FName* Key = NULL;
			if(GetUnKeyName( DeferredMessage.Message, wParam, lParam, Key, KeyCode ))
			{			
				//  [10/25/2006 YTS] 
				//	KeyCode == KEY_PrintScreen is not handled by WM_SYSKEYDOWN
				//	but Handled by WM_SYSKEYUP. UInput::InputKey processes it as a abnormal state and ignores KEY_PrintScreen at once.
				// So I push the WM_SYSKEYDOWN message with Keycode == KEY_PrintScreen.
				if( Key != NULL && (*Key == KEY_PrintScreen) && KeyStates[KeyCode] == FALSE)
					ViewportWndProc(WM_SYSKEYDOWN, wParam, lParam);

				// Update the cached key state.
				KeyStates[KeyCode] = FALSE;

				// Send key to input system.
				if( ViewportClient )
				{
					ViewportClient->InputKey(this,0,*Key,IE_Released);
				}
			}
		}
		break;

	case WM_CHAR:
	case WM_SYSCHAR:
		{
			// the destruction of this will send the PostWindowsMessage callback
			FCallbackHelper CallbackHelper(this, DeferredMessage.Message);

			TCHAR Character = GUnicodeOS ? wParam : TCHAR(wParam);
			if(ViewportClient)
			{
				TCHAR sz[] = {0,0};
				sz[0] = Character;
				FString ResultStr(sz);
				FInputCompositionStringData CompStrData(IMECOMPOSITION_None, ResultStr);
				ViewportClient->InputChar(this,0, CompStrData);
			}
		}
		break;

	case WM_SETCURSOR:
		{
			UBOOL bHandled = UpdateMouseCursor( TRUE );
			if ( bHandled )
			{
				UpdateMouseLock();
			}
		}
		break;

	case WM_SETFOCUS:
		UWindowsClient::DirectInput8Mouse->Unacquire();
		UWindowsClient::DirectInput8Mouse->SetCooperativeLevel(Window,DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
		for(INT JoystickIndex = 0;JoystickIndex < UWindowsClient::Joysticks.Num();JoystickIndex++)
		{
			FJoystickInfo& JoystickInfo = UWindowsClient::Joysticks(JoystickIndex);
			if ( JoystickInfo.DirectInput8Joystick )
			{
				JoystickInfo.DirectInput8Joystick->Unacquire();
				JoystickInfo.DirectInput8Joystick->SetCooperativeLevel(Window,DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
			}
		}
		if(ViewportClient && !PreventCapture)
		{
			ViewportClient->ReceivedFocus(this);
		}
		/*if ( GFullScreenMovie )
		{
			GFullScreenMovie->Mute( FALSE );
		}*/
		UpdateMouseCursor( TRUE );
		UpdateModifierState();
		UpdateMouseLock();
		break;

	case WM_KILLFOCUS:
		if(ViewportClient)
		{
			ViewportClient->LostFocus(this);
		}
		/*if ( GFullScreenMovie )
		{
			GFullScreenMovie->Mute( TRUE );
		}*/
		LockMouseToWindow( FALSE );
		if ( bCapturingMouseInput )
		{
			CaptureMouse( FALSE );
			OnMouseButtonUp( WM_LBUTTONUP, 0 );
			OnMouseButtonUp( WM_MBUTTONUP, 0 );
			OnMouseButtonUp( WM_RBUTTONUP, 0 );
			OnMouseButtonUp( WM_XBUTTONUP, MAKEWPARAM(0,MK_XBUTTON1) );
			OnMouseButtonUp( WM_XBUTTONUP, MAKEWPARAM(0,MK_XBUTTON2) ); 
		}

		// Make sure our mouse button bits get reset to the 'up' state after losing focus
		bShouldResetMouseButtons = TRUE;

		break;

	case WM_CAPTURECHANGED:
		if ( bCapturingMouseInput )
		{
			LockMouseToWindow( FALSE );
			CaptureMouse( FALSE );
			OnMouseButtonUp( WM_LBUTTONUP, 0 );
			OnMouseButtonUp( WM_MBUTTONUP, 0 );
			OnMouseButtonUp( WM_RBUTTONUP, 0 );
			OnMouseButtonUp( WM_XBUTTONUP, MAKEWPARAM(0,MK_XBUTTON1) );
			OnMouseButtonUp( WM_XBUTTONUP, MAKEWPARAM(0,MK_XBUTTON2) ); 
		}
		break;

	case WM_MOUSEACTIVATE:
		{
			// Reset mouse buttons back to 'up' if we need to
			ConditionallyResetMouseButtons();
		}
		break;

#if WITH_IME
	case WM_IME_NOTIFY:
		{
			HIMC Imc = ImmGetContext( Window );
			if( !Imc )
			{
				break;
				//appErrorf( TEXT( "No IME context" ) );
			}

			DWORD dwSize;
			FString CandStr;

			switch(wParam)
			{
			case IMN_OPENCANDIDATE:
			case IMN_CHANGECANDIDATE:
				{

				// Get size before Allocation memory.
				if( (dwSize = ImmGetCandidateList(Imc, 0x0, NULL, 0)) == 0)
					break;

				// appMalloc alternatives
				TArray<BYTE> CandList;
				CandList.AddZeroed(dwSize);
				LPCANDIDATELIST lpC = (LPCANDIDATELIST)CandList.GetTypedData();

				TArray<FString> CandStrList;
				// Get an info structure of candidate window ( LPCANDIDATELIST)
				if( ImmGetCandidateList(Imc, 0, lpC, CandList.Num()) )
				{
					for( UINT i = 0 ; i < lpC->dwCount ; i++)
					{
						FString CandStr = (TCHAR*)((BYTE*)lpC + lpC->dwOffset[i]);
						CandStrList.AddItem( CandStr );
					}

					FInputCandidateStringData CandStrData(CandStrList, lpC->dwCount, lpC->dwSelection, lpC->dwPageStart, lpC->dwPageSize);
					if(ViewportClient)
						ViewportClient->InputCandidate(this, 0, CandStrData);
				}
				break;

				}
			case IMN_CLOSECANDIDATE:
				if(ViewportClient)
				{
					FInputCandidateStringData CandStrData;
					ViewportClient->InputCandidate(this, 0, CandStrData);
				}
				break;
			case IMN_PRIVATE:
				{
					FString ReadStr = GetPrivateReadingString( Imc );
					FInputReadingStringData ReadStrData(ReadStr);
					ViewportClient->InputReadingString(this, 0, ReadStrData);
				}
				break;
			}
			ImmReleaseContext( Window, Imc );
		}
		// Block the default candidate window ( which supported by MS )
		break;

	case WM_IME_STARTCOMPOSITION:
		{
			FInputCompositionStringData CompStrData(IMECOMPOSITION_Start);
			ViewportClient->InputChar(this,0,CompStrData);
		}
		break;
	case WM_IME_ENDCOMPOSITION:
		{
			FInputCompositionStringData CompStrData(IMECOMPOSITION_End);
			ViewportClient->InputChar(this,0,CompStrData);
		}
		break;
	case WM_IME_COMPOSITION:
		{
			HIMC Imc = ImmGetContext( Window );
			checkf( Imc, TEXT( "No IME context" ) );

			static const INT CompStrBufSize = 30;

			TCHAR szCompStr[CompStrBufSize + 1] = { TEXT('\0'), TEXT('\0') };

			if( lParam & GCS_RESULTSTR )
			{
				INT ResStrSize = ImmGetCompositionString( Imc, GCS_RESULTSTR, szCompStr, sizeof(szCompStr) );
				FString ResStr(szCompStr);

				FInputCompositionStringData InputResStrData( IMECOMPOSITION_Result, ResStr );
				ViewportClient->InputChar( this, 0, InputResStrData );
			}
			else if ( lParam & GCS_COMPSTR )
			{
				INT CompStrSize = ImmGetCompositionString( Imc, GCS_COMPSTR, szCompStr, sizeof(szCompStr) );
				FString CompStr(szCompStr);

				TArray<BYTE> CompAttrs;
				TArray<INT> CompClss;
				INT CompCursorPos = 0;

				if( GKeyboardInputLanguage != KBDINPUTLANG_Korean )
				{
					BYTE CompAttr[30 + 1];
					DWORD CompCls[30 + 1];

					INT CompAttrSize = lParam & GCS_COMPATTR ? ImmGetCompositionString( Imc, GCS_COMPATTR, CompAttr, sizeof(CompAttr) ) : 0;
					CompCursorPos = lParam & GCS_CURSORPOS ? ImmGetCompositionString( Imc, GCS_CURSORPOS, NULL, 0 ) : 0;
					INT CompClsSize = lParam & GCS_COMPCLAUSE ? ImmGetCompositionString( Imc, GCS_COMPCLAUSE, CompCls, sizeof(CompCls) ) : 0;

					CompAttrs.AddZeroed(CompAttrSize);
					CompClss.AddZeroed(CompClsSize);

					for( INT i = 0 ; i < CompAttrSize ; i++ )
						CompAttrs(i) = CompAttr[i];
					for( INT i = 0 ; i < CompClsSize ; i++ )
						CompClss(i) = (INT)CompCls[i];
				}

				FInputCompositionStringData InputCompStrData( IMECOMPOSITION_Composition, CompStr, CompAttrs, CompCursorPos, CompClss );
				ViewportClient->InputChar( this, 0, InputCompStrData );
				//debugf( TEXT("OnImeComposition (%s), %s, %s, %s\n"),
				//	*CompStr,
				//	lParam & GCS_RESULTSTR ? TEXT("Res") : lParam & GCS_COMPSTR ? TEXT("Comp") : TEXT("Unknown"),
				//	lParam & GCS_COMPSTR ? TEXT("CompAttr") : TEXT("NoCompAttr"),
				//	lParam & GCS_COMPCLAUSE ? TEXT("CompClause") : TEXT("NoCompClause"));
			}
			ImmReleaseContext( Window, Imc );

		}
		break;
#endif

	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_XBUTTONDBLCLK:
		{		
			// the destruction of this will send the PostWindowsMessage callback
			FCallbackHelper CallbackHelper(this, DeferredMessage.Message);
			OnMouseButtonDoubleClick( DeferredMessage.Message, wParam );
		}			
		break;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_XBUTTONDOWN:
		{
			// the destruction of this will send the PostWindowsMessage callback
			FCallbackHelper CallbackHelper(this, DeferredMessage.Message);
			CaptureMouse( TRUE );
			OnMouseButtonDown( DeferredMessage.Message, wParam );
		}
		break;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_XBUTTONUP:
		{
			// the destruction of this will send the PostWindowsMessage callback
			FCallbackHelper CallbackHelper(this, DeferredMessage.Message);
			UBOOL bAnyButtonDown = FALSE;
			switch (DeferredMessage.Message)
			{
			case WM_LBUTTONUP: bAnyButtonDown = KeyState(KEY_RightMouseButton) || KeyState(KEY_MiddleMouseButton) || KeyState(KEY_XMouseButton); break;
			case WM_RBUTTONUP: bAnyButtonDown = KeyState(KEY_LeftMouseButton) || KeyState(KEY_MiddleMouseButton) || KeyState(KEY_XMouseButton); break;
			case WM_MBUTTONUP: bAnyButtonDown = KeyState(KEY_LeftMouseButton) || KeyState(KEY_RightMouseButton) || KeyState(KEY_XMouseButton); break;
			case WM_XBUTTONUP:
				{
					UBOOL bXButton1 = (GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON1) ? TRUE : FALSE;	// Is XBUTTON1 currently pressed?
					UBOOL bXButton2 = (GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON2) ? TRUE : FALSE;	// Is XBUTTON2 currently pressed?
					if ( KeyState(KEY_XMouseButton) && !bXButton1 )								// Did XBUTTON1 get released?
					{
						bAnyButtonDown = KeyState(KEY_LeftMouseButton) || KeyState(KEY_RightMouseButton) || KeyState(KEY_MiddleMouseButton) || KeyState(KEY_YMouseButton);
					}
					else
					{
						bAnyButtonDown = KeyState(KEY_LeftMouseButton) || KeyState(KEY_RightMouseButton) || KeyState(KEY_MiddleMouseButton) || KeyState(KEY_XMouseButton);
					}
				}
				break;
			}
			if ( !bAnyButtonDown )
			{
				CaptureMouse( FALSE );
			}
			OnMouseButtonUp( DeferredMessage.Message, wParam );
		}
		break;

	case WM_ENTERSIZEMOVE:
		Resizing = TRUE;
		break;

	case WM_EXITSIZEMOVE:
		Resizing = FALSE;
		HandlePossibleSizeChange();
		break;

	case WM_SIZE:
		if ( !bPerformingSize )
		{
			// Show mouse cursor if we're being minimized.
			if( SIZE_MINIMIZED == wParam )
			{
				Minimized = TRUE;
				Maximized = FALSE;
			}
			else if( SIZE_MAXIMIZED == wParam )
			{
				Minimized = FALSE;
				Maximized = TRUE;
				HandlePossibleSizeChange();
			}
			else if( SIZE_RESTORED == wParam )
			{
				if( Maximized )
				{
					Maximized = FALSE;
					HandlePossibleSizeChange();
				}
				else if( Minimized)
				{
					Minimized = FALSE;
					HandlePossibleSizeChange();
				}
				else
				{
					// Game:
					// If we're neither maximized nor minimized, the window size 
					// is changing by the user dragging the window edges.  In this 
					// case, we don't reset the device yet -- we wait until the 
					// user stops dragging, and a WM_EXITSIZEMOVE message comes.
					if(!Resizing)
						HandlePossibleSizeChange();
				}
			}
		}
		break;

	case WM_SIZING:
		// Flush the rendering command queue to ensure that there aren't pending viewport draw commands for the old viewport size.
		FlushRenderingCommands();
		break;
	};
}

void FWindowsViewport::Tick(FLOAT DeltaTime)
{
	// Update the mouse lock every frame.
	UpdateMouseLock();
}


/**
* Resets mouse buttons if we were asked to by a message handler
*/
void FWindowsViewport::ConditionallyResetMouseButtons()
{
	if( bShouldResetMouseButtons )
	{
		KeyStates[ VK_LBUTTON ] = FALSE;
		KeyStates[ VK_RBUTTON ] = FALSE;
		KeyStates[ VK_MBUTTON ] = FALSE;
	}

	bShouldResetMouseButtons = false;
}


/*
* Resends the state of the modifier keys (Ctrl, Shift, Alt).
* Used when receiving focus, otherwise these keypresses may
* be lost to some other process in the system.
*/
void FWindowsViewport::UpdateModifierState()
{
	if ( GShouldEnqueueModifierStateUpdates )
	{
		bUpdateModifierStateEnqueued = TRUE;
	}
	else
	{
		// Clear any enqueued UpdateModifierState requests.
		bUpdateModifierStateEnqueued = FALSE;
		UpdateModifierState( VK_LCONTROL );
		UpdateModifierState( VK_RCONTROL );
		UpdateModifierState( VK_LSHIFT );
		UpdateModifierState( VK_RSHIFT );
		UpdateModifierState( VK_LMENU );
		UpdateModifierState( VK_RMENU );

		// Reset the mouse button states if we need to.  In the case of receiving focus
		// via a mouse click, this will have already happened through the MOUSEACTIVATE pathway.
		ConditionallyResetMouseButtons();
	}
}

/*
* Resends the state of the specified key to the viewport client.
* It would've been nice to call InputKey only if the current state differs from what 
* FEditorLevelViewportClient::Input thinks, but I can't access that here... :/
*/
void FWindowsViewport::UpdateModifierState( INT VirtKey )
{
	if ( !ViewportClient || !Client )
	{
		return;
	}

	FName* Key = Client->KeyMapVirtualToName.Find( VirtKey );
	if (!Key)
	{
		return;
	}

	UBOOL bDown = (::GetKeyState(VirtKey) & 0x8000) ? TRUE : FALSE;
	UBOOL bChangedState = (KeyStates[VirtKey] != bDown);
	KeyStates[VirtKey] = bDown;
	if ( bChangedState )
	{
		ViewportClient->InputKey(this, 0, *Key, bDown ? IE_Pressed : IE_Released );
	}
}

void FWindowsViewport::OnMouseButtonDoubleClick( UINT Message, WPARAM wParam )
{
	// Note: When double-clicking, the following message sequence is sent:
	//	WM_*BUTTONDOWN
	//	WM_*BUTTONUP
	//	WM_*BUTTONDBLCLK	(Needs to set the KeyStates[*] to TRUE)
	//	WM_*BUTTONUP

	FName Key;
	if ( Message == WM_LBUTTONDBLCLK )
		Key = KEY_LeftMouseButton;
	else if ( Message == WM_MBUTTONDBLCLK )
		Key = KEY_MiddleMouseButton;
	else if ( Message == WM_RBUTTONDBLCLK )
		Key = KEY_RightMouseButton;
	else if( Message == WM_XBUTTONDBLCLK  && GET_XBUTTON_WPARAM(wParam) == XBUTTON1)									
		Key = KEY_XMouseButton;
	else if( Message == WM_XBUTTONDBLCLK && GET_XBUTTON_WPARAM(wParam) == XBUTTON2)									
		Key = KEY_YMouseButton;	

	if ( Client )
	{
		BYTE* KeyIndex = Client->KeyMapNameToVirtual.Find(Key);
		if ( KeyIndex )
		{
			KeyStates[*KeyIndex] = TRUE;
		}
	}

	if(ViewportClient)
	{
		ViewportClient->InputKey(this,0,Key,IE_DoubleClick);
	}
}

void FWindowsViewport::OnMouseButtonDown( UINT Message, WPARAM wParam )
{
	FName Key;
	if( Message == WM_LBUTTONDOWN )
		Key = KEY_LeftMouseButton;
	else if( Message == WM_MBUTTONDOWN )
		Key = KEY_MiddleMouseButton;
	else if( Message == WM_RBUTTONDOWN )
		Key = KEY_RightMouseButton;
	else if( Message == WM_XBUTTONDOWN && GET_XBUTTON_WPARAM(wParam) == XBUTTON1)									
		Key = KEY_XMouseButton;
	else if( Message == WM_XBUTTONDOWN && GET_XBUTTON_WPARAM(wParam) == XBUTTON2)									
		Key = KEY_YMouseButton;
	else
		return;

	if ( Client )
	{
		BYTE* KeyIndex = Client->KeyMapNameToVirtual.Find(Key);
		if ( KeyIndex )
		{
			KeyStates[*KeyIndex] = TRUE;
		}
	}

	if(ViewportClient)
	{
		::SetFocus( Window );			// Focus issues with Viewports: Force focus on Window

#if USE_DIRECTINPUT_MOUSEBUTTON
		if (GIsEditor)
#endif
			ViewportClient->InputKey(this,0,Key,IE_Pressed);
	}
}

void FWindowsViewport::OnMouseButtonUp( UINT Message, WPARAM wParam )
{
	// allow mouse capture to resume after resizing
	PreventCapture = 0;

	FName Key;
	if( Message == WM_LBUTTONUP )
		Key = KEY_LeftMouseButton;
	else if( Message == WM_MBUTTONUP )
		Key = KEY_MiddleMouseButton;
	else if( Message == WM_RBUTTONUP )
		Key = KEY_RightMouseButton;
	else if( Message == WM_XBUTTONUP && GET_XBUTTON_WPARAM(wParam) == XBUTTON1)									
		Key = KEY_XMouseButton;
	else if( Message == WM_XBUTTONUP && GET_XBUTTON_WPARAM(wParam) == XBUTTON2)									
		Key = KEY_YMouseButton;		
	else
		return;

	UBOOL bChangedState = TRUE;
	if ( Client )
	{
		BYTE* KeyIndex = Client->KeyMapNameToVirtual.Find(Key);
		if ( KeyIndex )
		{
			bChangedState = KeyStates[*KeyIndex];
			KeyStates[*KeyIndex] = FALSE;
		}
	}

	if ( ViewportClient && bChangedState )
	{
#if USE_DIRECTINPUT_MOUSEBUTTON
		if (GIsEditor)
#endif
			ViewportClient->InputKey(this,0,Key,IE_Released);
	}
}

UBOOL FWindowsViewport::GetUnKeyName( UINT Message, WPARAM wParam, LPARAM lParam, FName*& Key, UINT& KeyCode, UBOOL bSetKeyboardState /*= TRUE*/ )
{
	check(Client);

	KeyCode = wParam;
	Key = Client->KeyMapVirtualToName.Find(KeyCode);

	TCHAR szKeyName[128] = {0,0};
	GetKeyNameText(lParam, szKeyName, ARRAY_COUNT(szKeyName));

	if( appStricmp( szKeyName, TEXT("Right Control") ) == 0 )
		Key = Client->KeyMapVirtualToName.Find(KeyCode = VK_RCONTROL);
	else if( appStricmp( szKeyName, TEXT("Left Control") ) == 0  || appStricmp( szKeyName, TEXT("Control") ) == 0 || appStricmp( szKeyName, TEXT("Ctrl")) == 0)
		Key = Client->KeyMapVirtualToName.Find(KeyCode = VK_LCONTROL);
	else if( appStricmp( szKeyName, TEXT("Right Alt") ) == 0 )
		Key = Client->KeyMapVirtualToName.Find(KeyCode = VK_RMENU);
	else if( appStricmp( szKeyName, TEXT("Left Alt") ) == 0 || appStricmp( szKeyName, TEXT("Alt") ) == 0 )
		Key = Client->KeyMapVirtualToName.Find(KeyCode = VK_LMENU);
	else if( appStricmp( szKeyName, TEXT("Right Shift") ) == 0 )
		Key = Client->KeyMapVirtualToName.Find(KeyCode = VK_RSHIFT);
	else if( appStricmp( szKeyName, TEXT("Left Shift") ) == 0 || appStricmp( szKeyName, TEXT("Shift") ) == 0 )
		Key = Client->KeyMapVirtualToName.Find(KeyCode = VK_LSHIFT);
	else if( appStricmp( szKeyName, TEXT("Num Enter") ) == 0 )
		Key = Client->KeyMapVirtualToName.Find(KeyCode = VK_CLEAR);
	// "Num Enter"는 Keypad Enter로 일반 Enter와 같은 키코드를 쓰기때문에 게임내에서 두 키를 구별할 수 없다. 
	// 따라서 "Num Enter"는 VK_Return 대신에 VK_Clear를 쓴다.

	if( ! Key )
	{
		if( lstrlen( szKeyName ) == 1 )
			Key = Client->KeyMapVirtualToName.Find(KeyCode = LOBYTE(VkKeyScan(szKeyName[0])));
		else if( lstrcmpi(szKeyName, TEXT("space")) == 0 )
			Key = Client->KeyMapVirtualToName.Find(KeyCode = L' ');
		// Alternative Code For Space Input , but I think there's a better way than this.
	}

	if( ! Key )
		return FALSE;


	if( bSetKeyboardState && (KeyCode == VK_RCONTROL || KeyCode == VK_RMENU || KeyCode == VK_RSHIFT) )
	{
		BYTE KeyboardState[256];
		GetKeyboardState( KeyboardState );

		BYTE* ModifierKeys[2]; 
		switch( KeyCode )
		{
		case VK_RCONTROL:
			ModifierKeys[0] = &KeyboardState[VK_RCONTROL];
			ModifierKeys[1] = &KeyboardState[VK_CONTROL];
			break;
		case VK_RMENU:
			ModifierKeys[0] = &KeyboardState[VK_RMENU];
			ModifierKeys[1] = &KeyboardState[VK_MENU];
			break;
		case VK_RSHIFT:
			ModifierKeys[0] = &KeyboardState[VK_RSHIFT];
			ModifierKeys[1] = &KeyboardState[VK_SHIFT];
			break;
		}

		UBOOL bKeyDown = (Message == WM_KEYDOWN || Message == WM_SYSKEYDOWN);
		for( INT i = 0 ; i < ARRAY_COUNT(ModifierKeys) ; i++ )
			*ModifierKeys[i] = bKeyDown ? (*ModifierKeys[i] | 0x80) : (*ModifierKeys[i] & 0x7f);

		SetKeyboardState( KeyboardState );
	}

	return TRUE;
}

void Get24BitImage ( INT nWidth, INT nHeight, HBITMAP hBitmap , TArray<FColor>& OutputBuffer )
{
	OutputBuffer.AddZeroed( nWidth * nHeight );

	HDC hDC = ::GetDC( 0 );

	HDC  memDC1 = ::CreateCompatibleDC ( hDC );
	HDC memDC2 = ::CreateCompatibleDC ( hDC );

	BYTE *lpBits = NULL;

	BITMAPINFO bmi;
	::ZeroMemory( &bmi, sizeof(BITMAPINFO) );
	bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth       = nWidth;
	bmi.bmiHeader.biHeight      = nHeight;
	bmi.bmiHeader.biPlanes      = 1;
	bmi.bmiHeader.biBitCount    = 24;
	bmi.bmiHeader.biCompression = BI_RGB;

	HBITMAP hDIBMemBM  = ::CreateDIBSection( 0, &bmi, DIB_RGB_COLORS, (void**)&lpBits, NULL, NULL );
	HBITMAP hOldBmp1  = (HBITMAP)::SelectObject(memDC1, hDIBMemBM );

	HBITMAP hOldBmp2  = (HBITMAP) ::SelectObject ( memDC2,hBitmap);

	::BitBlt( memDC1, 0, 0, nWidth, nHeight, memDC2, 0, 0, SRCCOPY );

	FColor* DestColor = (FColor*)OutputBuffer.GetData();

	for ( INT i = 0 ; i < nHeight ; i++)
	{
		const BYTE* Src = lpBits + nWidth*3*(nHeight-1-i);
		for (INT j = 0; j < nWidth; j++)
		{			
			BYTE B = *Src++;
			BYTE G = *Src++;
			BYTE R = *Src++;
			*DestColor++ = FColor( R, G, B );
		}		
	}

	// clean up
	::SelectObject  ( memDC1, hOldBmp1  );
	::SelectObject  ( memDC2,hOldBmp2  );
	::ReleaseDC    ( 0, hDC      );
	::DeleteObject  ( hDIBMemBM  );
	::DeleteObject  ( hOldBmp1  );
	::DeleteObject  ( hOldBmp2  );
	::DeleteDC  ( memDC1  );
	::DeleteDC  ( memDC2  );
}

UBOOL FWindowsViewport::ReadPixels(TArray<FColor>& OutputBuffer,ECubeFace CubeFace/*=CubeFace_PosX*/)
{
	UBOOL bRet = FALSE;

	extern UBOOL GHackDIBScreenshot;

	if (GHackDIBScreenshot)
	{
		FlushRenderingCommands();

		RECT R;
		::GetClientRect( Window, &R );
		::MapWindowPoints( Window, NULL, (POINT*)&R, 2 );

		INT Width = R.right - R.left;
		INT Height = R.bottom - R.top;

		HDC hdcScreen = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL); 
		HDC hdcCompatible = CreateCompatibleDC(hdcScreen); 

		// Create a compatible bitmap for hdcScreen. 

		HBITMAP hbmScreen = CreateCompatibleBitmap(hdcScreen, Width, Height );
		if (hbmScreen != NULL)
		{
			if (SelectObject(hdcCompatible, hbmScreen))
			{
				if (BitBlt(hdcCompatible, 
					0,0, 
					Width, Height, 
					hdcScreen, 
					R.left, R.top, 
					SRCCOPY)) 
				{	
					LPBITMAPINFO pbi;

					if ((pbi = (LPBITMAPINFO)(new char[sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD)])) != NULL) 
					{
						memset(&pbi->bmiHeader,0,sizeof(BITMAPINFOHEADER));
						pbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

						if (GetDIBits(hdcCompatible,hbmScreen,0,Height,NULL,pbi,DIB_RGB_COLORS))
						{
							if (pbi->bmiHeader.biBitCount == 32)
							{
								static TArray<BYTE> BitmapArray;

								BitmapArray.Empty( pbi->bmiHeader.biSizeImage );

								BYTE* Buffer = &BitmapArray(0);

								if (GetDIBits(hdcCompatible,hbmScreen,0,Height,Buffer,pbi,DIB_RGB_COLORS))
								{
									OutputBuffer.AddZeroed( Width * Height );

									const BYTE* Src = Buffer;

									for (INT i=0; i<Height; ++i)
									{
										for (INT j=0; j<Width; ++j)
										{
											OutputBuffer((Height-i-1)*Width+j) = FColor( Src[2],Src[1], Src[0] );

											Src += 4;
										}									
									}

									bRet = TRUE;
								}
							}
						}

						delete[] pbi;
					}
				}
			}

			DeleteObject(hbmScreen);
		}

		DeleteDC(hdcScreen); 
		DeleteDC(hdcCompatible);

		return bRet;
	}
	else	
		return FRenderTarget::ReadPixels( OutputBuffer, CubeFace );	
}