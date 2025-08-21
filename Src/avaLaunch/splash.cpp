#include "avaLaunch.h"
#include <windows.h>

//  ===========================================================================
//  Class   FSplash
//  Desc    Use it for displaying splash screen for applications
//          Works only on Win2000 , WinXP and later versions of Windows
//  ===========================================================================
class FSplash
{
public:
	//  =======================================================================
	//  Func   FSplash
	//  Desc   Default constructor
	//  =======================================================================
	FSplash();

	//  =======================================================================
	//  Func   FSplash
	//  Desc   Constructor
	//  Arg    Path of the Bitmap that will be show on the splash screen
	//  Arg    The color on the bitmap that will be made transparent
	//  =======================================================================
	FSplash(LPCTSTR lpszFileName, COLORREF colTrans);

	//  =======================================================================
	//  Func   ~FSplash
	//  Desc   Desctructor
	//  =======================================================================
	virtual ~FSplash();

	//  =======================================================================
	//  Func   ShowSplash
	//  Desc   Launches the non-modal splash screen
	//  Ret    void 
	//  =======================================================================
	void ShowSplash();

	//  =======================================================================
	//  Func   DoLoop
	//  Desc   Launched the splash screen as a modal window. Not completely 
	//         implemented.
	//  Ret    INT 
	//  =======================================================================
	INT DoLoop();

	//  =======================================================================
	//  Func   CloseSplash
	//  Desc   Closes the splash screen started with ShowSplash
	//  Ret    INT 
	//  =======================================================================
	INT CloseSplash();

	//  =======================================================================
	//  Func   SetBitmap
	//  Desc   Call this with the path of the bitmap. Not required to be used
	//         when the construcutor with the image path has been used.
	//  Ret    1 if succesfull
	//  Arg    Either the file path or the handle to an already loaded bitmap
	//  =======================================================================
	DWORD SetBitmap(LPCTSTR lpszFileName);
	DWORD SetBitmap(HBITMAP hBitmap);

	//  =======================================================================
	//  Func   SetTransparentColor
	//  Desc   This is used to make one of the color transparent
	//  Ret    1 if succesfull
	//  Arg    The colors RGB value. Not required if the color is specified 
	//         using the constructor
	//  =======================================================================
	bool SetTransparentColor(COLORREF col);

	LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	HWND m_hwnd;

private:
	void Init();
	void  OnPaint(HWND hwnd);
	bool MakeTransparent();
	HWND RegAndCreateWindow();
	COLORREF m_colTrans;
	DWORD m_dwWidth;
	DWORD m_dwHeight;
	void FreeResources();
	HBITMAP m_hBitmap;
	LPCTSTR m_lpszClassName;

};



//  ===========================================================================
//  The following is used for layering support which is used in the 
//  splash screen for transparency. In VC 6 these are not defined in the headers
//  for user32.dll and hence we use mechanisms so that it can work in VC 6.
//  We define the flags here and write code so that we can load the function
//  from User32.dll explicitely and use it. This code requires Win2k and above
//  to work.
//  ===========================================================================
typedef BOOL (WINAPI *lpfnSetLayeredWindowAttributes)(HWND hWnd, COLORREF cr, BYTE bAlpha, DWORD dwFlags);

lpfnSetLayeredWindowAttributes g_pSetLayeredWindowAttributes;

/*#define WS_EX_LAYERED 0x00080000 
#define LWA_COLORKEY 1 // Use color as the transparency color.
#define LWA_ALPHA    2 // Use bAlpha to determine the opacity of the layer*/

//  ===========================================================================
//  Func    ExtWndProc
//  Desc    The windows procedure that is used to forward messages to the 
//          FSplash class. FSplash sends the "this" pointer through the
//          CreateWindowEx call and the pointer reaches here in the 
//          WM_CREATE message. We store it here and use it for message 
//          forwarding.
//  ===========================================================================
static LRESULT CALLBACK ExtWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static FSplash * spl = NULL;
	if(uMsg == WM_CREATE)
	{
		spl = (FSplash*)((LPCREATESTRUCT)lParam)->lpCreateParams;
	}
	if(spl)
		return spl->WindowProc(hwnd, uMsg, wParam, lParam);
	else
		return DefWindowProc (hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK FSplash::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//  We need to handle on the WM_PAINT message
	switch(uMsg)
	{
		case WM_PAINT : OnPaint( hwnd );
			break;
	}

	return DefWindowProc (hwnd, uMsg, wParam, lParam) ;
}

void FSplash:: OnPaint(HWND hwnd)
{
	if (!m_hBitmap)
		return;

	//  =======================================================================
	//  Paint the background by BitBlting the bitmap
	//  =======================================================================
	PAINTSTRUCT ps ;
	HDC hDC = BeginPaint (hwnd, &ps) ;

	RECT   rect;
	::GetClientRect(m_hwnd, &rect);

	HDC hMemDC      = ::CreateCompatibleDC(hDC);
	HBITMAP hOldBmp = (HBITMAP)::SelectObject(hMemDC, m_hBitmap);

	BitBlt(hDC, 0, 0, m_dwWidth, m_dwHeight, hMemDC, 0, 0, SRCCOPY);

	::SelectObject(hMemDC, hOldBmp);

	::DeleteDC(hMemDC);

	EndPaint (hwnd, &ps) ;
}

void FSplash::Init()
{
	//  =======================================================================
	//  Initialize the variables
	//  =======================================================================
	m_hwnd = NULL;
	m_lpszClassName = TEXT("SPLASH");
	m_colTrans = 0;

	//  =======================================================================
	//  Keep the function pointer for the SetLayeredWindowAttributes function
	//  in User32.dll ready
	//  =======================================================================
	HMODULE hUser32 = GetModuleHandle(TEXT("USER32.DLL"));

#pragma warning(disable:4191)
	g_pSetLayeredWindowAttributes = (lpfnSetLayeredWindowAttributes)GetProcAddress(hUser32, "SetLayeredWindowAttributes");
}

FSplash::FSplash()
{
	Init();
}

FSplash::FSplash(LPCTSTR lpszFileName, COLORREF colTrans)
{
	Init();

	SetBitmap(lpszFileName);
	SetTransparentColor(colTrans);
}

FSplash::~FSplash()
{
	FreeResources();
}

HWND FSplash::RegAndCreateWindow()
{
	//  =======================================================================
	//  Register the window with ExtWndProc as the window procedure
	//  =======================================================================
	WNDCLASSEX wndclass;
	wndclass.cbSize         = sizeof (wndclass);
	wndclass.style          = CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW;
	wndclass.lpfnWndProc    = ExtWndProc;
	wndclass.cbClsExtra     = 0;
	wndclass.cbWndExtra     = DLGWINDOWEXTRA;
	wndclass.hInstance      = ::GetModuleHandle(NULL);
	wndclass.hIcon          = NULL;
	wndclass.hCursor        = ::LoadCursor( NULL, IDC_WAIT );
	wndclass.hbrBackground  = (HBRUSH)::GetStockObject(LTGRAY_BRUSH);
	wndclass.lpszMenuName   = NULL;
	wndclass.lpszClassName  = m_lpszClassName;
	wndclass.hIconSm        = NULL;

	if(!RegisterClassEx (&wndclass))
		return NULL;

	//  =======================================================================
	//  Create the window of the application, passing the this pointer so that
	//  ExtWndProc can use that for message forwarding
	//  =======================================================================
	DWORD nScrWidth  = ::GetSystemMetrics(SM_CXFULLSCREEN);
	DWORD nScrHeight = ::GetSystemMetrics(SM_CYFULLSCREEN);

	INT x = (nScrWidth  - m_dwWidth) / 2;
	INT y = (nScrHeight - m_dwHeight) / 2;
	m_hwnd = ::CreateWindowEx(WS_EX_TOPMOST|WS_EX_TOOLWINDOW, m_lpszClassName, 
		TEXT("Alliance of Valiant Arms (Loading)"), WS_POPUP, x, y, 
		m_dwWidth, m_dwHeight, NULL, NULL, NULL, this);

	//  =======================================================================
	//  Display the window
	//  =======================================================================
	if(m_hwnd)
	{
		MakeTransparent();
		ShowWindow   (m_hwnd, SW_SHOW) ;
		UpdateWindow (m_hwnd);
	}
	return m_hwnd;
}

INT FSplash::DoLoop()
{
	//  =======================================================================
	//  Show the window
	//  =======================================================================
	if(!m_hwnd)
		ShowSplash();

	//  =======================================================================
	//  Get into the modal loop
	//  =======================================================================
	MSG msg ;
	while (GetMessage (&msg, NULL, 0, 0))
	{
		TranslateMessage (&msg) ;
		DispatchMessage  (&msg) ;
	}

	return msg.wParam ;

}

void FSplash::ShowSplash()
{
	CloseSplash();
	RegAndCreateWindow();
}


DWORD FSplash::SetBitmap(LPCTSTR lpszFileName)
{
	//  =======================================================================
	//  load the bitmap
	//  =======================================================================
	HBITMAP    hBitmap       = NULL;
	hBitmap = (HBITMAP)::LoadImage(0, lpszFileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	return SetBitmap(hBitmap);
}

DWORD FSplash::SetBitmap(HBITMAP hBitmap)
{
	INT nRetValue;
	BITMAP  csBitmapSize;

	//  =======================================================================
	//  Free loaded resource
	//  =======================================================================
	FreeResources();

	if (hBitmap)
	{
		m_hBitmap = hBitmap;

		//  ===================================================================
		//  Get bitmap size
		//  ===================================================================
		nRetValue = ::GetObject(hBitmap, sizeof(csBitmapSize), &csBitmapSize);
		if (nRetValue == 0)
		{
			FreeResources();
			return 0;
		}
		m_dwWidth = (DWORD)csBitmapSize.bmWidth;
		m_dwHeight = (DWORD)csBitmapSize.bmHeight;
	}

	return 1;
}

void FSplash::FreeResources()
{
	if (m_hBitmap)
		::DeleteObject (m_hBitmap);
	m_hBitmap = NULL;
}

INT FSplash::CloseSplash()
{

	if(m_hwnd)
	{
		DestroyWindow(m_hwnd);
		m_hwnd = 0;
		UnregisterClass(m_lpszClassName, ::GetModuleHandle(NULL));
		return 1;
	}
	return 0;
}

bool FSplash::SetTransparentColor(COLORREF col)
{
	m_colTrans = col;

	return MakeTransparent();
}

bool FSplash::MakeTransparent()
{
	//  =======================================================================
	//  Set the layered window style and make the required color transparent
	//  =======================================================================
	if(m_hwnd && g_pSetLayeredWindowAttributes && m_colTrans )
	{
		//  set layered style for the window
		SetWindowLong(m_hwnd, GWL_EXSTYLE, GetWindowLong(m_hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
		//  call it with 0 alpha for the given color
		g_pSetLayeredWindowAttributes(m_hwnd, m_colTrans, 0, LWA_COLORKEY);
	}    
	return TRUE;
}

static FSplash* GSplashEx = NULL;

void appShowSplashEx( const TCHAR* SplashName )
{	
	if (GSplashEx != NULL)
		return;

	FString Filepath = appGameDir();
	Filepath += SplashName;

	//  Launch splash screen
	GSplashEx = new FSplash(*Filepath, RGB(128, 128, 128));
	GSplashEx->ShowSplash();
}

void appHideSplashEx()
{
	if (GSplashEx == NULL)
		return;

	GSplashEx->CloseSplash();
	delete GSplashEx;
	GSplashEx = NULL;
}