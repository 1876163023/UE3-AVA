#include "avaLaunch.h"

//HHOOK g_hKeyboardHook;
//UBOOL g_bWindowActive = TRUE;
//
//static LRESULT CALLBACK LowLevelKeyboardProc( int nCode, WPARAM wParam, LPARAM lParam )
//{
//	if (nCode < 0 || nCode != HC_ACTION )  // do not process message 
//		return CallNextHookEx( g_hKeyboardHook, nCode, wParam, lParam); 
//
//	KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
//	switch (wParam) 
//	{
//	case WM_KEYDOWN:  
//	case WM_KEYUP:    
//#if !FINAL_RELEASE
//		if (g_bWindowActive && GEngine)
//		{
//			UWindowsClient* Client = Cast<UWindowsClient>( GEngine->Client );
//
//			if (Client)
//			{
//				for (INT i=0; i<Client->Viewports.Num(); ++i)
//				{
//					if (Client->Viewports(i)->IsFullscreen())
//					{
//#endif
//						if ((p->vkCode == VK_LWIN) || (p->vkCode == VK_RWIN))
//							return 1;
//
//						// CTRL+ESC
//						if ((p->vkCode == VK_ESCAPE) && (::GetAsyncKeyState(VK_CONTROL) & 0x8000))
//						{
//							return 1;
//						}
//
//						// ALT+ESC
//						if ((p->vkCode == VK_ESCAPE) && (p->flags & LLKHF_ALTDOWN))
//						{
//							return 1;
//						}
//#if !FINAL_RELEASE
//					}
//				}
//			}						
//		}
//#endif
//		break;
//	}
//
//	return CallNextHookEx( g_hKeyboardHook, nCode, wParam, lParam );
//}
//static STICKYKEYS g_StartupStickyKeys = {sizeof(STICKYKEYS), 0};
//static TOGGLEKEYS g_StartupToggleKeys = {sizeof(TOGGLEKEYS), 0};
//static FILTERKEYS g_StartupFilterKeys = {sizeof(FILTERKEYS), 0};    

void AllowAccessibilityShortcutKeys( UBOOL bAllowKeys )
{	
	/*if( bAllowKeys )
	{
		// Restore StickyKeys/etc to original state and enable Windows key
		SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &g_StartupStickyKeys, 0);
		SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &g_StartupToggleKeys, 0);
		SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &g_StartupFilterKeys, 0);
	}
	else
	{
		// Disable StickyKeys/etc shortcuts but if the accessibility feature is on, 
		// then leave the settings alone as its probably being usefully used

		STICKYKEYS skOff = g_StartupStickyKeys;
		if( (skOff.dwFlags & SKF_STICKYKEYSON) == 0 )
		{
			// Disable the hotkey and the confirmation
			skOff.dwFlags &= ~SKF_HOTKEYACTIVE;
			skOff.dwFlags &= ~SKF_CONFIRMHOTKEY;

			SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &skOff, 0);
		}

		TOGGLEKEYS tkOff = g_StartupToggleKeys;
		if( (tkOff.dwFlags & TKF_TOGGLEKEYSON) == 0 )
		{
			// Disable the hotkey and the confirmation
			tkOff.dwFlags &= ~TKF_HOTKEYACTIVE;
			tkOff.dwFlags &= ~TKF_CONFIRMHOTKEY;

			SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &tkOff, 0);
		}

		FILTERKEYS fkOff = g_StartupFilterKeys;
		if( (fkOff.dwFlags & FKF_FILTERKEYSON) == 0 )
		{
			// Disable the hotkey and the confirmation
			fkOff.dwFlags &= ~FKF_HOTKEYACTIVE;
			fkOff.dwFlags &= ~FKF_CONFIRMHOTKEY;

			SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &fkOff, 0);
		}
	}*/
}

void PlatformPostInit()
{
	/*// Initialization
	g_hKeyboardHook = SetWindowsHookEx( WH_KEYBOARD_LL,  LowLevelKeyboardProc, GetModuleHandle(NULL), 0 );

	// Save the current sticky/toggle/filter key settings so they can be restored them later
	SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &g_StartupStickyKeys, 0);
	SystemParametersInfo(SPI_GETTOGGLEKEYS, sizeof(TOGGLEKEYS), &g_StartupToggleKeys, 0);
	SystemParametersInfo(SPI_GETFILTERKEYS, sizeof(FILTERKEYS), &g_StartupFilterKeys, 0);

	// Disable when full screen
	AllowAccessibilityShortcutKeys( TRUE );*/
}

void PlatformPreExit()
{
	/*// Restore back when going to windowed or shutting down
	AllowAccessibilityShortcutKeys( TRUE );

	// Cleanup before shutdown
	UnhookWindowsHookEx( g_hKeyboardHook );*/
}