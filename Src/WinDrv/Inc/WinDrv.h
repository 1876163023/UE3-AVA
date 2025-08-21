/*=============================================================================
	WinDrv.h: Unreal Windows viewport and platform driver.
	Copyright 2003-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef _INC_WINDRV
#define _INC_WINDRV

/**
 * The joystick support is hardcoded to either handle an Xbox 1 Type S controller or
 * a PlayStation 2 Dual Shock controller plugged into a PC.
 *
 * See http://www.redcl0ud.com/xbcd.html for Windows drivers for Xbox 1 Type S.
 */

/** This is how many joystick buttons are supported. Must be less than 256 */
#define MAX_JOYSTICK_BUTTONS		16

#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

// Unreal includes.
#include "Engine.h"

// Windows includes.
#pragma pack (push,8)
#define DIRECTINPUT_VERSION 0x800
#include <dinput.h>
#include <xinput.h>
#pragma pack (pop)

// Forward declarations.
class FWindowsViewport;

/** This variable reflects the latest status sent to us by WM_ACTIVATEAPP */
extern UBOOL GWindowActive;

/**
 * Joystick types supported by console controller emulation.	
 */
enum EJoystickType
{
	JOYSTICK_None,
	JOYSTICK_PS2_Old_Converter,
	JOYSTICK_PS2_New_Converter,
	JOYSTICK_X360,
	JOYSTICK_Xbox_Type_S
};

#define JOYSTICK_NUM_XINPUT_CONTROLLERS	4

/**
 * Information about a joystick.
 */
struct FJoystickInfo
{
	/** The DirectInput interface to this Joystick. */
	LPDIRECTINPUTDEVICE8 DirectInput8Joystick;

	/** The joystick's type. */
	EJoystickType JoystickType;

	/** The joystick's controller ID. */
	INT ControllerId;

	/** Last frame's joystick button states, so we only send events on edges */
	UBOOL JoyStates[MAX_JOYSTICK_BUTTONS];

	/** Next time a IE_Repeat event should be generated for each button */
	DOUBLE NextRepeatTime[MAX_JOYSTICK_BUTTONS];

	/** Store previous joy state. Sometimes DInput fails to query joystick, we don't want to have negative impacts such as zero acceleration */
	DIJOYSTATE2 PreviousState;

	/** Device GUID */
	GUID DeviceGUID;

	/** Whether an XInput controller is connected or not */
	UBOOL bIsConnected;
};

/** Contains the state needed to defer processing of a window message. */
struct FDeferredMessage
{
	FWindowsViewport* Viewport;
	UINT Message;
	WPARAM wParam;
	LPARAM lParam;

	struct
	{
		SHORT LeftControl;
		SHORT RightControl;
		SHORT LeftShift;
		SHORT RightShift;
		SHORT Menu;
	}	KeyStates;
};


//
//	UWindowsClient - Windows-specific client code.
//
class UWindowsClient : public UClient
{
	DECLARE_CLASS(UWindowsClient,UClient,CLASS_Transient|CLASS_Config,WinDrv)

	// Static variables.
	static TArray<FWindowsViewport*>	Viewports;
	static LPDIRECTINPUT8				DirectInput8;
	static LPDIRECTINPUTDEVICE8			DirectInput8Mouse;
	static TArray<FJoystickInfo>		Joysticks;
	static HANDLE						KeyboardHookThread;		// Thread that manages the low-level keyboard hook
	static DWORD						KeyboardHookThreadId;	// Thread id for the KeyboardHookThread.

	// Variables.
	UEngine*							Engine;
	FString								WindowClassName;

	TMap<BYTE,FName>					KeyMapVirtualToName;
	TMap<FName,BYTE>					KeyMapNameToVirtual;

	/** A quick lookup to map PS2 controller button indices to Xbox controller button indices */
	BYTE		PS2ToXboxControllerMapping[MAX_JOYSTICK_BUTTONS];
	/** A quick lookup to map X360 XInput controller button indices to Xbox controller button indices */
	BYTE		X360ToXboxControllerMapping[MAX_JOYSTICK_BUTTONS];
	/** An array of FNames that are sent to the input system. Maps Xbox button indices to FNames */
	FName		JoyNames[MAX_JOYSTICK_BUTTONS];

	/** Last time in ms (arbitrarily based) we received an input event for mouse x axis */
	DWORD								LastMouseXEventTime;
	/** Last time in ms (arbitrarily based) we received an input event for mouse y axis */
	DWORD								LastMouseYEventTime;

	/** Whether attached viewports process windows messages or not */
	UBOOL								ProcessWindowsMessages;

	/** Time until we check for newly connected XInput Controllers again */
	FLOAT								ControllerScanTime;

	/** Next controller to check for connection (Index into Joysticks array) */
	INT									NextControllerToCheck;

	// Audio device.
	UClass*								AudioDeviceClass;
	UAudioDevice*						AudioDevice;

	// Accessibility shortcut keys
	STICKYKEYS							StartupStickyKeys;
	TOGGLEKEYS							StartupToggleKeys;
	FILTERKEYS							StartupFilterKeys;

	/** Whether to allow joystick input. */
	UBOOL								bAllowJoystickInput;

	/** Messages that have been deferred for later processing at a predetermined point. */
	TArray<FDeferredMessage>	DeferredMessages;

	/**
	* Defers a message received by a viewport until a predetermined point in the frame.
	*/
	void DeferMessage(FWindowsViewport* Viewport,UINT Message,WPARAM wParam,LPARAM lParam);

	/** Processes window messages that have been deferred. */
	void ProcessDeferredMessages();

	// Constructors.
	UWindowsClient();
	void StaticConstructor();

	// UObject interface.
	virtual void Serialize(FArchive& Ar);
	virtual void FinishDestroy();
	virtual void ShutdownAfterError();

	// UClient interface.
	virtual void Init(UEngine* InEngine);
	virtual void Tick( FLOAT DeltaTime );
	virtual UBOOL Exec(const TCHAR* Cmd,FOutputDevice& Ar);

	/**
	 * PC only, debugging only function to prevent the engine from reacting to OS messages. Used by e.g. the script
	 * debugger to avoid script being called from within message loop (input).
	 *
	 * @param InValue	If FALSE disallows OS message processing, otherwise allows OS message processing (default)
	 */
	virtual void AllowMessageProcessing( UBOOL InValue ) { ProcessWindowsMessages = InValue; }

	virtual FViewportFrame* CreateViewportFrame(FViewportClient* ViewportClient,const TCHAR* Name,UINT SizeX,UINT SizeY,UBOOL Fullscreen = 0);
	virtual FViewport* CreateWindowChildViewport(FViewportClient* ViewportClient,void* ParentWindow,UINT SizeX=0,UINT SizeY=0,INT InPosX = -1, INT InPosY = -1);
	virtual void CloseViewport(FViewport* Viewport);

	virtual class UAudioDevice* GetAudioDevice() { return AudioDevice; }
		
	/** Function to immediately stop any force feedback vibration that might be going on this frame. */
	virtual void ForceClearForceFeedback();

	/**
	* Toggles accessibility shortcut keys on and off
	* 
	* @param State - TRUE to enable shortcuts, FALSE to disable
	*/
	void AllowAccessibilityShortcutKeys( UBOOL AllowKeys );
	
	/**
	* Retrieves the name of the key mapped to the specified character code.
	*
	* @param	KeyCode	the key code to get the name for; should be the key's ANSI value
	*/
	virtual FName GetVirtualKeyName( INT KeyCode ) const;

	// Window message handling.
	static LONG APIENTRY StaticWndProc( HWND hWnd, UINT Message, UINT wParam, LONG lParam );

	/**
	* Retrieves the name of the key mapped to the specified character code.
	*
	* @param	VirtualKeyName - one, two, f1, f2, left bracket 등등
	* @return	not found - 0 , otherwise the byte-code found
	*/
	virtual BYTE GetKeyCode( FName VirtualKeyName ) const { const BYTE* NewCode = KeyMapNameToVirtual.Find(VirtualKeyName); return NewCode ? *NewCode : NULL; }

	/**
	*	Reads input from shared resources, such as buffered D3D mouse input.
	*/
	void ProcessInput( FLOAT DeltaTime );

	/** Poll mouse input */
	static UBOOL PollMouseInput( );

	/** Flush all mouse input */
	static void FlushMouseInput( );
};

/**
 * A Windows implementation of FViewport and FViewportFrame.
 */
class FWindowsViewport : public FViewportFrame, public FViewport
{
public:

	enum EForceCapture		{EC_ForceCapture};

	/**
	 * Minimal initialization constructor.
	 */
	FWindowsViewport(
		UWindowsClient* InClient,
		FViewportClient* InViewportClient,
		const TCHAR* InName,
		UINT InSizeX,
		UINT InSizeY,
		UBOOL InFullscreen,
		HWND InParentWindow,
		INT InPosX = -1,
		INT InPosY = -1
		);

	/** Destructor. */
	~FWindowsViewport();

	// FViewport interface.
	virtual void*	GetWindow()					{ return Window; }
	virtual UBOOL	KeyState(FName Key) const;
	virtual UBOOL	CaptureJoystickInput(UBOOL Capture);

	// New MouseCapture/MouseLock API
	virtual UBOOL	HasMouseCapture() const		{ return bCapturingMouseInput; }
	virtual UBOOL	HasFocus() const;
	virtual void	ShowCursor( UBOOL bVisible );
	virtual void	CaptureMouse( UBOOL bCapture );
	virtual void	LockMouseToWindow( UBOOL bLock );
	virtual void	UpdateMouseLock();

	virtual INT		GetMouseX();
	virtual INT		GetMouseY();
	virtual void	GetMousePos( FIntPoint& MousePosition );
	virtual void	SetMouse(INT x, INT y);

	virtual void	InvalidateDisplay();

	virtual FViewportFrame* GetViewportFrame();
	void			ProcessInput( FLOAT DeltaTime );

	// FViewportFrame interface.
	virtual FViewport* GetViewport() { return this; }
	virtual void Resize(UINT NewSizeX,UINT NewSizeY,UBOOL NewFullscreen,INT InPosX = -1, INT InPosY = -1);

	// FWindowsViewport interface.
	void			Destroy();
	void			ShutdownAfterError();	
	LONG			ViewportWndProc(UINT Message,UINT wParam,LONG lParam);
	void			ProcessDeferredMessage(const FDeferredMessage& Message);
	void			Tick(FLOAT DeltaTime);

	// Helper functions.
	UBOOL			IsCursorVisible() const;
	UBOOL			UpdateMouseCursor( UBOOL bSetCursor );	

	virtual UBOOL ReadPixels(TArray<FColor>& OutputBuffer,ECubeFace CubeFace=CubeFace_PosX);

private:

	// Viewport state.
	UWindowsClient*			Client;
	HWND					Window,
							ParentWindow;

	FString					Name;

	UBOOL					Minimized;
	UBOOL					Maximized;
	UBOOL					Resizing;			// TRUE when the user is resizing by dragging
	UBOOL					bPerformingSize;	// TRUE when the game is resizing insde Resize()

	UBOOL					KeyStates[256];
	
	// New MouseCapture/MouseLock API
	EMouseCursor			SystemMouseCursor;
	UBOOL					bMouseLockRequested;
	UBOOL					bCapturingMouseInput;
	UBOOL					PreventCapture;
	RECT                    MouseLockClientRect;

	// Old MouseCapture/MouseLock API (Info saved during captures and fullscreen sessions).
	POINT					PreCaptureMousePos;
	RECT					PreCaptureCursorClipRect;
	UBOOL					bCapturingJoystickInput;
	UBOOL					bLockingMouseToWindow;

	/** Last time in ms (arbitrarily based) we received an input event for mouse x axis */
	DWORD					LastMouseXEventTime;
	/** Last time in ms (arbitrarily based) we received an input event for mouse y axis */
	DWORD					LastMouseYEventTime;

	/** If TRUE, an UpdateModifierState() is enqueued. */
	UBOOL					bUpdateModifierStateEnqueued;

	/** True if ResetMouseButtons should be called after receiving focus */
	UBOOL					bShouldResetMouseButtons;
	
	//! 마지막으로 포커스를 잠근 시간을 저장.
	DOUBLE					LastLockedTime;

#if WITH_IME
	/** Whether or not the OS supports IME */
	UBOOL					bSupportsIME;
#endif

	void	HandlePossibleSizeChange( );
	void	UpdateRenderDevice( UINT NewSizeX, UINT NewSizeY, UBOOL bNewIsFullscreen );
	void	UpdateWindow( UBOOL NewFullscreen, DWORD WindowStyle, INT WindowPosX, INT WindowPosY, INT WindowWidth, INT WindowHeight );
	void	UpdateModifierState( );
	void	UpdateModifierState( INT VirtKey );

	void	OnMouseButtonDoubleClick( UINT Message, WPARAM wParam );
	void	OnMouseButtonDown( UINT Message, WPARAM wParam );
	void	OnMouseButtonUp( UINT Message, WPARAM wParam );

	/** 해당 키값을 얻어온다.
	 * 영문(미국) 자판이 아닐경우 RightControl,RightAlt, RightShift를 따로 구한다. (wParam이 제대로 안들어옴)
	 **/
	UBOOL	GetUnKeyName( UINT Message, WPARAM wParam, LPARAM lParam, FName*& Key, UINT& KeyCode, UBOOL bSetKeyboardState = TRUE );

	/**
	* Resets mouse buttons if we were asked to by a message handler
	*/
	void ConditionallyResetMouseButtons();
};

#include "XnaForceFeedbackManager.h"

/*-----------------------------------------------------------------------------
	Unicode support.
-----------------------------------------------------------------------------*/

#define SetClassLongX(a,b,c)						TCHAR_CALL_OS(SetClassLongW(a,b,c),SetClassLongA(a,b,c))
#define GetClassLongX(a,b)							TCHAR_CALL_OS(GetClassLongW(a,b),GetClassLongA(a,b))
#define RemovePropX(a,b)							TCHAR_CALL_OS(RemovePropW(a,b),RemovePropA(a,TCHAR_TO_ANSI(b)))
#define DispatchMessageX(a)							TCHAR_CALL_OS(DispatchMessageW(a),DispatchMessageA(a))
#define PeekMessageX(a,b,c,d,e)						TCHAR_CALL_OS(PeekMessageW(a,b,c,d,e),PeekMessageA(a,b,c,d,e))
#define PostMessageX(a,b,c,d)						TCHAR_CALL_OS(PostMessageW(a,b,c,d),PostMessageA(a,b,c,d))
#define SendMessageX(a,b,c,d)						TCHAR_CALL_OS(SendMessageW(a,b,c,d),SendMessageA(a,b,c,d))
#define SendMessageLX(a,b,c,d)						TCHAR_CALL_OS(SendMessageW(a,b,c,(LPARAM)d),SendMessageA(a,b,c,(LPARAM)(TCHAR_TO_ANSI((TCHAR *)d))))
#define SendMessageWX(a,b,c,d)						TCHAR_CALL_OS(SendMessageW(a,b,(WPARAM)c,d),SendMessageA(a,b,(WPARAM)(TCHAR_TO_ANSI((TCHAR *)c)),d))
#define DefWindowProcX(a,b,c,d)						TCHAR_CALL_OS(DefWindowProcW(a,b,c,d),DefWindowProcA(a,b,c,d))
#define CallWindowProcX(a,b,c,d,e)					TCHAR_CALL_OS(CallWindowProcW(a,b,c,d,e),CallWindowProcA(a,b,c,d,e))
#define GetWindowLongX(a,b)							TCHAR_CALL_OS(GetWindowLongW(a,b),GetWindowLongA(a,b))
#define SetWindowLongX(a,b,c)						TCHAR_CALL_OS(SetWindowLongW(a,b,c),SetWindowLongA(a,b,c))
#define LoadMenuIdX(i,n)							TCHAR_CALL_OS(LoadMenuW(i,MAKEINTRESOURCEW(n)),LoadMenuA(i,MAKEINTRESOURCEA(n)))
#define LoadCursorIdX(i,n)							TCHAR_CALL_OS(LoadCursorW(i,MAKEINTRESOURCEW(n)),LoadCursorA(i,MAKEINTRESOURCEA(n)))
#ifndef LoadIconIdX
#define LoadIconIdX(i,n)							TCHAR_CALL_OS(LoadIconW(i,MAKEINTRESOURCEW(n)),LoadIconA(i,MAKEINTRESOURCEA(n)))
#endif
#define CreateWindowExX(a,b,c,d,e,f,g,h,i,j,k,l)	TCHAR_CALL_OS(CreateWindowEx(a,b,c,d,e,f,g,h,i,j,k,l),CreateWindowExA(a,TCHAR_TO_ANSI(b),TCHAR_TO_ANSI(c),d,e,f,g,h,i,j,k,l))
#define SetWindowTextX(a,b)							TCHAR_CALL_OS(SetWindowTextW(a,b),SetWindowTextA(a,TCHAR_TO_ANSI(b)))

#endif //_INC_WINDRV
