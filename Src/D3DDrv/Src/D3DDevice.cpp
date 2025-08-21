/*=============================================================================
	D3DViewport.cpp: D3D viewport RHI implementation.
	Copyright 2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "D3DDrvPrivate.h"
#include "SupportedResolution.h"

#if USE_D3D_RHI

//
// RHI constants.
//

INT GMaxLogAnisotropy = 0;

/** The maximum number of mip-maps that a texture can contain. */
INT GMaxTextureMipCount = MAX_TEXTURE_MIP_COUNT;

/** The minimum number of mip-maps that always remain resident.	*/
INT	GMinTextureResidentMipCount = 7;

/** TRUE if PF_DepthStencil textures can be created and sampled. */
UBOOL GSupportsDepthTextures = FALSE;

UBOOL GIsLowEndHW = FALSE; 

/** 
* TRUE if PF_DepthStencil textures can be created and sampled to obtain PCF values. 
* This is different from GSupportsDepthTextures in two ways:
*	-results of sampling are PCF values, not depths
*	-a color target must be bound with the depth stencil texture even if never written to or read from,
*		due to API restrictions
*/
UBOOL GSupportsHardwarePCF = FALSE;

/** TRUE if D24 textures can be created and sampled, retrieving 4 neighboring texels in a single lookup. */
UBOOL GSupportsFetch4 = FALSE;

//<@ ava specific ; 2007. 4. 9 changmin
UBOOL GIsATI = FALSE;
UBOOL GIsNVidia = FALSE;
//>@ ava
//<@ ava specific ; 2007. 5. 11 changmin
// 2007. 5. 11 changmin
// 성능향상을 위해, 현재 render target인 texture를 sampling 하는 것을 이용하고 있다.
// 이를 nvidia graphic card나 ati 고사양( x1xxx) 에서는 지원한다.
// 하지만, 그 하위 버전에서는 지원을 안하므로, 이를 처리하기 위한 flag이다.
UBOOL GRequireResolve	 = FALSE;

// 2007. 5. 11 changmin : shadow projection 시 영역을 표시하기 위해 frustum을 그리는 데,
// 몇몇의 ati graphics card에서는(x850, 9800pro) colorwrite를 false로 했음에도 불구하고 검게 shadow frustum 그려진다.
// 이를 방지하기 위한 setting이다.
// 이게 아니라... FP Blending 문제였음....
//UBOOL GBlackShadowVolume = FALSE;
//>@ ava

/**
* TRUE if floating point blending is supported
*/
UBOOL GSupportsFPBlending = TRUE;

/**
* TRUE if floating point filtering is supported
*/
UBOOL GSupportsFPFiltering = TRUE;

/** TRUE if pixels are clipped to 0<Z<W instead of -W<Z<W. */
const UBOOL GPositiveZRange = TRUE;

/** Can we handle quad primitives? */
const UBOOL GSupportsQuads = FALSE;

/** Are we using an inverted depth buffer? */
const UBOOL GUsesInvertedZ = FALSE;

/** The offset from the upper left corner of a pixel to the position which is used to sample textures for fragments of that pixel. */
const FLOAT GPixelCenterOffset = 0.5f;

/** The shader platform to load from the cache */
EShaderPlatform GRHIShaderPlatform = SP_PCD3D;

/** Shadow buffer size */
INT GShadowDepthBufferSize = 1024;
INT GAvaShadowDepthBufferSize = 2048;	// ava specific ; add cascaded shadow 2008. 1. 2 changmin

/** Bias exponent used to apply to shader color output when rendering to the scene color surface */
INT GCurrentColorExpBias = 0;

/** Enable or disable vsync, overridden by the -novsync command line parameter. */
// USystemSetting로 플래그를 옮김
// UBOOL GEnableVSync = FALSE;

UBOOL GSupportsVertexInstancing = FALSE;

/** If FALSE code needs to patch up vertex declaration. */
UBOOL GVertexElementsCanShareStreamOffset = TRUE;

//<@ ava specific ; 2007. 11. 14 changmin
UBOOL GUseCascadedShadow = FALSE;
//>@ ava


//
// Globals.
//

/** True if the D3D device has been initialized. */
UBOOL GIsRHIInitialized = FALSE;

/** The global D3D interface. */
TD3DRef<IDirect3D9> GDirect3D;

/** The global D3D device. */
TD3DRef<IDirect3DDevice9> GDirect3DDevice;

/** The global D3D device's back buffer. */
FSurfaceRHIRef GD3DBackBuffer;

/** The viewport RHI which is currently fullscreen. */
FD3DViewport* GD3DFullscreenViewport = NULL;

/** The width of the D3D device's back buffer. */
UINT GD3DDeviceSizeX = 0;

/** The height of the D3D device's back buffer. */
UINT GD3DDeviceSizeY = 0;

/** The window handle associated with the D3D device. */
HWND GD3DDeviceWindow = NULL;

/** True if the D3D device is in fullscreen mode. */
UBOOL GD3DIsFullscreenDevice = FALSE;

/** True if the application has lost D3D device access. */
UBOOL GD3DDeviceLost = FALSE;

/** Indicates support for Nvidia's Depth Bounds Test through a driver hack in D3D. */
UBOOL GD3DDepthBoundsHackSupported = FALSE;

//! 바탕화면의 해상도 저장.(2007/04/27 고광록)
DEVMODE GDesktopDevMode;

//! 전체화면에서 아이템을 사기위해 Alt+Tab이 되도록 한다.(2007/04/30 고광록)
//:fixed by deif ( 2007/12/7 )
// Windowed mode로 하면 average frame time은 비슷하나, 주기적으로 hitch가 생겨서 전반적인 integrator quality에 영향을 심대하게... -_- 준다.
// 이에 따라 모든 것을 full screen으로 하고 fake full screen이 필요할 경우에만 일시적으로 on하기로 한다.
UBOOL GFakeFullScreen = FALSE;


/**
 * Cleanup the D3D device.
 * This function must be called from the main game thread.
 */
void CleanupD3DDevice()
{
	check( IsInGameThread() );
	if(GIsRHIInitialized)
	{
		// Ask all initialized FRenderResources to release their RHI resources.
		for(TLinkedList<FRenderResource*>::TIterator ResourceIt(FRenderResource::GetResourceList());ResourceIt;ResourceIt.Next())
		{
			ResourceIt->ReleaseRHI();
		}
		for(TLinkedList<FRenderResource*>::TIterator ResourceIt(FRenderResource::GetResourceList());ResourceIt;ResourceIt.Next())
		{
			ResourceIt->ReleaseDynamicRHI();
		}
		GIsRHIInitialized = FALSE;
		// release dynamic resources from global movie player
		if( GFullScreenMovie )
		{
			GFullScreenMovie->ReleaseDynamicResources();
		}
	}

	// (만약 전체화면인 경우) 바탕화면 해상도 복구.(2007/04/27 고광록)
	if ( GFakeFullScreen && GD3DIsFullscreenDevice )
		ChangeDisplaySettings(&GDesktopDevMode, CDS_RESET);
}

//<@ ava specific ;2007. 5. 21 changmin
INT DeviceShaderVersion = 0;
INT GetDeviceShaderVersion()
{
	return DeviceShaderVersion;
}
//>@ ava

/**
* Initializes GRHIShaderPlatform based on the system's Caps
*/
void InitializeD3DShaderPlatform()
{
	check( IsInGameThread() );

	// Wait for the rendering thread to go idle.
	FlushRenderingCommands();

	if ( !GDirect3D )
	{
		*GDirect3D.GetInitReference() = Direct3DCreate9(D3D_SDK_VERSION);
	}
	if ( !GDirect3D )
	{
		appErrorf(
			NAME_FriendlyError,
			TEXT("Please install DirectX 9.0c or later (see Release Notes for instructions on how to obtain it)")
			);
	}

	D3DCAPS9 DeviceCaps;
	UINT AdapterIndex = D3DADAPTER_DEFAULT;
	D3DDEVTYPE DeviceType = D3DDEVTYPE_HAL;	
	
	if (SUCCEEDED(GDirect3D->GetDeviceCaps(AdapterIndex,DeviceType,&DeviceCaps)))
	{
		// Determine whether ps_3_0 is supported.
		UBOOL bSupportsShaderModel3 = (DeviceCaps.PixelShaderVersion & 0xff00) >= 0x0300;

		//<@ ava specific ; 2007. 5. 21 changmin
		DeviceShaderVersion = (DeviceCaps.PixelShaderVersion & 0xff00) >> 8;
		//>@ ava

		//only overwrite if not supported, since this may have been specified on the command line
		if (!bSupportsShaderModel3)
		{
			if (GRHIShaderPlatform == SP_PCD3D)
			{
				GRHIShaderPlatform = SP_PCD3D_SM2;
			}

			D3DDISPLAYMODE	CurrentDisplayMode;
			VERIFYD3DRESULT(GDirect3D->GetAdapterDisplayMode(AdapterIndex,&CurrentDisplayMode));

			// Determine the supported lighting buffer formats.
			UBOOL bSupports16bitRT = FALSE;
			if(FAILED(GDirect3D->CheckDeviceFormat(
				AdapterIndex,
				DeviceType,
				CurrentDisplayMode.Format,
				D3DUSAGE_RENDERTARGET,
				D3DRTYPE_TEXTURE,
				D3DFMT_A16B16G16R16F
				)))
				bSupports16bitRT = FALSE;
			else
				bSupports16bitRT = TRUE;

			// Determine whether alpha blending with a floating point framebuffer is supported.
			if (FAILED(
				GDirect3D->CheckDeviceFormat(
				AdapterIndex,
				DeviceType,
				CurrentDisplayMode.Format,
				D3DUSAGE_RENDERTARGET | D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
				D3DRTYPE_TEXTURE,
				bSupports16bitRT ? D3DFMT_A16B16G16R16F : D3DFMT_A32B32G32R32F
				)))
			{
				//only overwrite if not supported, since the previous setting may have been specified on the command line
				GRHIShaderPlatform = SP_PCD3D_SM2_POOR;
				GIsLowEndHW = TRUE; // integer hdr로 가는 열쇠...
			}
		}
	}
	
	if (GRHIShaderPlatform == SP_PCD3D_SM2_POOR)
	{
		//GSupportsFPBlending = FALSE;
		GSupportsFPBlending = TRUE;	// 다음번 device check에서 무효화된다...의미없긴 하지만.. 일단 놔두자.. changmin
		GIsLowEndHW = TRUE;	// integer hdr 로 가는 열쇠
	}

	// 현재 바탕화면 해상도 저장.(2007/04/27 고광록)
	ZeroMemory(&GDesktopDevMode, sizeof(DEVMODE));
	GDesktopDevMode.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &GDesktopDevMode);
}

UBOOL GForceRecreateDevice = FALSE;

/**
 * Initializes the D3D device for the current viewport state.
 * This function must be called from the main game thread.
 */
void UpdateD3DDeviceFromViewports()
{
	check( IsInGameThread() );

	const UBOOL bForceRecreateDevice = GForceRecreateDevice;
	GForceRecreateDevice = FALSE;

	// Wait for the rendering thread to go idle.
	FlushRenderingCommands();

	if(GD3DViewports.Num())
	{
		// Create the Direct3D object if necessary.
		if ( !GDirect3D )
		{
			*GDirect3D.GetInitReference() = Direct3DCreate9(D3D_SDK_VERSION);
		}
		if ( !GDirect3D )
		{
			appErrorf(
				NAME_FriendlyError,
				TEXT("Please install DirectX 9.0c or later (see Release Notes for instructions on how to obtain it)")
				);
		}

		// Find the maximum viewport dimensions and any fullscreen viewports.
		FD3DViewport*	NewFullscreenViewport = NULL;
		UINT			MaxViewportSizeX = 0,
						MaxViewportSizeY = 0,
						MaxHitViewportSizeX = 0,
						MaxHitViewportSizeY = 0;

		for(INT ViewportIndex = 0;ViewportIndex < GD3DViewports.Num();ViewportIndex++)
		{
			FD3DViewport* D3DViewport = GD3DViewports(ViewportIndex);

			MaxViewportSizeX = Max(MaxViewportSizeX,D3DViewport->GetSizeX());
			MaxViewportSizeY = Max(MaxViewportSizeY,D3DViewport->GetSizeY());

			if(D3DViewport->IsFullscreen())
			{
				check(!NewFullscreenViewport);
				NewFullscreenViewport = D3DViewport;

				// Check that all fullscreen viewports use supported resolutions.
				UINT Width = D3DViewport->GetSizeX();
				UINT Height = D3DViewport->GetSizeY();
				GetSupportedResolution( Width, Height );
				check( Width == D3DViewport->GetSizeX() && Height == D3DViewport->GetSizeY() );
			}
		}
		
		// Determine the adapter and device type to use.
		UINT AdapterIndex = D3DADAPTER_DEFAULT;
		D3DDEVTYPE DeviceType = DEBUG_SHADERS ? D3DDEVTYPE_REF : D3DDEVTYPE_HAL;

		// Setup the needed device parameters.
		UINT NewDeviceSizeX = MaxViewportSizeX;
		UINT NewDeviceSizeY = MaxViewportSizeY;
		HWND NewDeviceWindow = NewFullscreenViewport ? (HWND)NewFullscreenViewport->GetWindowHandle() : (HWND)GD3DViewports(0)->GetWindowHandle();
		UBOOL NewDeviceFullscreen = NewFullscreenViewport ? TRUE : FALSE;

		// Check to see if GDirect3DDevice needs to be recreated.
		UBOOL bRecreateDevice = bForceRecreateDevice;
		if(!GDirect3DDevice)
		{
			bRecreateDevice = TRUE;
		}
		else
		{
			if(GD3DDeviceLost)
			{
				// NOTE: We can't use GetFocus() or GetActiveWindow() since NewDeviceWindow
				// may be owned by the game thread and not by us.
				HWND FocusWindow = ::GetForegroundWindow();
				UBOOL bIsMinimized = FocusWindow ? ::IsIconic( FocusWindow ) : TRUE;
				if ( FocusWindow != NewDeviceWindow || bIsMinimized )
				{
					// Abort and try again next time Present() is called again from RHIEndDrawingViewport().
					return;
				}
				FocusWindow = ::GetFocus();
				bRecreateDevice = TRUE;
			}

			if(NewDeviceFullscreen != GD3DIsFullscreenDevice)
			{
				bRecreateDevice = TRUE;
			}

			if(NewDeviceFullscreen)
			{
				if(GD3DDeviceSizeX != NewDeviceSizeX || GD3DDeviceSizeY != NewDeviceSizeY)
				{
					bRecreateDevice = TRUE;
				}
			}
			else
			{
				if(GD3DDeviceSizeX < NewDeviceSizeX || GD3DDeviceSizeY < NewDeviceSizeY)
				{
					bRecreateDevice = TRUE;
				}
			}

			if(GD3DDeviceWindow != NewDeviceWindow)
			{
				bRecreateDevice	= TRUE;
			}
		}

		if(bRecreateDevice)
		{
#if ENABLE_GPU_PROFILING
			// PerfHUD등
			GGPUPerfAnalyzerLoaded = FALSE;
#endif

			HRESULT Result;			

			// Retrieve VSYNC ini setting.
			//GConfig->GetBool( TEXT("WinDrv.WindowsClient"), TEXT("UseVSYNC"), GEnableVSync, GEngineIni );

			// Disable VSYNC if -novsync is on the command line.
			//GEnableVSync = GEnableVSync && ParseParam(appCmdLine(), TEXT("novsync"));
			GSystemSettings->bEnableVSync = GSystemSettings->bEnableVSync && !ParseParam(appCmdLine(), TEXT("novsync"));

			// Enable VSYNC if -vsync is on the command line.
			//GEnableVSync = GEnableVSync || ParseParam(appCmdLine(), TEXT("vsync"));
			GSystemSettings->bEnableVSync = GSystemSettings->bEnableVSync || ParseParam(appCmdLine(), TEXT("vsync"));

			// Setup the present parameters.
			D3DPRESENT_PARAMETERS PresentParameters;
			appMemzero(&PresentParameters,sizeof(PresentParameters));
			PresentParameters.BackBufferCount			= 1;
			PresentParameters.BackBufferFormat			= D3DFMT_A8R8G8B8;
			PresentParameters.BackBufferWidth			= NewDeviceSizeX;
			PresentParameters.BackBufferHeight			= NewDeviceSizeY;
			PresentParameters.SwapEffect				= NewDeviceFullscreen ? D3DSWAPEFFECT_DISCARD : D3DSWAPEFFECT_COPY;
			PresentParameters.Flags						= GIsEditor ? D3DPRESENTFLAG_LOCKABLE_BACKBUFFER : 0;
			PresentParameters.EnableAutoDepthStencil	= FALSE;
			PresentParameters.Windowed					= !NewDeviceFullscreen;
			PresentParameters.PresentationInterval		= GSystemSettings->bEnableVSync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
			PresentParameters.hDeviceWindow				= NewDeviceWindow;

			//{ 바탕화면 해상도 변환.(2007/04/27 고광록)
			// fixed by deif (2007/12/7) 
			// ; ATI의 overdrive를 지원하기 위해 fake full screen을 임시로 껐었는데, 위의 이유에서 모든 것에 대해 처리.
			if ( GFakeFullScreen )
			{
				if ( PresentParameters.Windowed )
				{
					// 단, 이전 상태가 full-screen인 경우에만 다시 설정.
					if ( GD3DIsFullscreenDevice )
						ChangeDisplaySettings(&GDesktopDevMode, CDS_RESET);
				}
				else
				{
					if ( GD3DDeviceSizeX != NewDeviceSizeX || GD3DDeviceSizeY != NewDeviceSizeY )
					{
						// 화면 해상도 자체를 바꿔준다.
						DEVMODE dm;
						ZeroMemory(&dm, sizeof(DEVMODE));
						dm.dmSize = sizeof(DEVMODE);
						dm.dmPelsWidth = NewDeviceSizeX;
						dm.dmPelsHeight = NewDeviceSizeY;
						dm.dmBitsPerPel = 32;
						dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
						dm.dmDisplayFrequency = GDesktopDevMode.dmDisplayFrequency;		//!< 바탕화면 Refresh Rate설정.(2007/06/29)

						// 좌표를 (0, 0)으로 맞춰줘야 한다.
						::MoveWindow(NewDeviceWindow, 0, 0, NewDeviceSizeX, NewDeviceSizeY, TRUE);
						::SetForegroundWindow(NewDeviceWindow);

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

					// 그리고 윈도우 모드로 무조건 생성되게 한다.
					PresentParameters.Windowed   = TRUE;
					PresentParameters.SwapEffect = D3DSWAPEFFECT_COPY;
				}
			}
			//}

			if(GDirect3DDevice)
			{
				TLinkedList<FRenderResource*> *&ResourceList = FRenderResource::GetResourceList();

				// Release dynamic resources and render targets.
				for(TLinkedList<FRenderResource*>::TIterator ResourceIt(ResourceList);ResourceIt;ResourceIt.Next())
				{
					ResourceIt->ReleaseDynamicRHI();
				}
				// release dynamic resources from global movie player
				if( GFullScreenMovie )
				{
					GFullScreenMovie->ReleaseDynamicResources();
				}

				// release all state blocks
				FD3DBaseState::ResetStateBlocks();

				extern void D3DPerfSaver_Reset();
				D3DPerfSaver_Reset();

				// Release the back-buffer and depth-buffer references to allow resetting the device.
				GD3DBackBuffer.Release();

				// Simple reset the device with the new present parameters.
				do 
				{
					if( FAILED(Result=GDirect3DDevice->Reset(&PresentParameters) ) )
					{
						// Sleep for a second before trying again if we couldn't reset the device as the most likely
						// cause is the device not being ready/lost which can e.g. occur if a screen saver with "lock"
						// kicks in.
						appSleep(1.0);
					}
				} 
				while( FAILED(Result) );

				// Get pointers to the device's back buffer and depth buffer.
				TD3DRef<IDirect3DSurface9> BackBuffer;
				TD3DRef<IDirect3DSurface9> DepthBuffer;
				VERIFYD3DRESULT(GDirect3DDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,BackBuffer.GetInitReference()));
				GD3DBackBuffer = FSurfaceRHIRef(FTexture2DRHIRef(),NULL,BackBuffer,TRUE);

				// Reinitialize dynamic resources and render targets.
				for(TLinkedList<FRenderResource*>::TIterator ResourceIt(ResourceList);ResourceIt;ResourceIt.Next())
				{
					ResourceIt->InitDynamicRHI();
				}
			}
			else
			{
				// Get information about the device being used.

				D3DCAPS9 DeviceCaps;
				VERIFYD3DRESULT(GDirect3D->GetDeviceCaps(AdapterIndex,DeviceType,&DeviceCaps));

				D3DADAPTER_IDENTIFIER9	AdapterIdentifier;

				VERIFYD3DRESULT(GDirect3D->GetAdapterIdentifier(AdapterIndex,0,&AdapterIdentifier));

				//<@ ava specific ; 2007. 4. 9 changmin
				const DWORD ATI_VENDOR_ID = 0x1002;
				const DWORD NVIDIA_VENDOR_ID = 0x10DE;

				if( AdapterIdentifier.VendorId == ATI_VENDOR_ID )
				{
					extern UBOOL GDoNotUseHarewareDynamicBuffers;
					GIsATI = TRUE;					
					//GDoNotUseHarewareDynamicBuffers = TRUE;	// 실험완료.. 속도가 좀 느림..
					debugf(NAME_Log, TEXT("ATI Card is detected") );
					//debugf(NAME_Log, TEXT("ATI Device Id : %x"), AdapterIdentifier.DeviceId );
				}
				else if( AdapterIdentifier.VendorId == NVIDIA_VENDOR_ID )
				{
					GIsNVidia = TRUE;
					debugf(NAME_Log, TEXT("NVidia Card is detected") );
				}
				//>@ ava

				// Query max texture size supported by the device.
				GMaxTextureMipCount = appCeilLogTwo( Min<INT>( DeviceCaps.MaxTextureHeight, DeviceCaps.MaxTextureWidth ) ) + 1;
				GMaxTextureMipCount = Min<INT>( MAX_TEXTURE_MIP_COUNT, GMaxTextureMipCount );
				GMaxLogAnisotropy = appCeilLogTwo( DeviceCaps.MaxAnisotropy );

				// Check for ps_2_0 support.
				if((DeviceCaps.PixelShaderVersion & 0xff00) < 0x0200)
				{
					appErrorf(TEXT("Device does not support pixel shaders 2.0 or greater.  PixelShaderVersion=%08x"),DeviceCaps.PixelShaderVersion);
				}

				// Check for hardware vertex instancing support.
				GSupportsVertexInstancing = (DeviceCaps.PixelShaderVersion & 0xff00) >= 0x0300;

				// Check whether card supports HW TnL
				const UBOOL bSupportsHardwareTnL = ( DeviceCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) ? TRUE : FALSE;
				
				// Check whether card supports vertex elements sharing offsets. But check it only if there's no HW TnL.
				if( bSupportsHardwareTnL )
				{
					GVertexElementsCanShareStreamOffset = (DeviceCaps.DevCaps2 & D3DDEVCAPS2_VERTEXELEMENTSCANSHARESTREAMOFFSET) ? TRUE : FALSE; 
				}

				// Check for required format support.

				D3DDISPLAYMODE	CurrentDisplayMode;
				VERIFYD3DRESULT(GDirect3D->GetAdapterDisplayMode(AdapterIndex,&CurrentDisplayMode));

				// Determine the supported lighting buffer formats.
				UBOOL bSupports16bitRT = FALSE;
				if(FAILED(GDirect3D->CheckDeviceFormat(
							AdapterIndex,
							DeviceType,
							CurrentDisplayMode.Format,
							D3DUSAGE_RENDERTARGET,
							D3DRTYPE_TEXTURE,
							D3DFMT_A16B16G16R16F
							)))
					bSupports16bitRT = FALSE;
				else
					bSupports16bitRT = TRUE;

				if(FAILED(GDirect3D->CheckDeviceFormat(
							AdapterIndex,
							DeviceType,
							CurrentDisplayMode.Format,
							D3DUSAGE_RENDERTARGET,
							D3DRTYPE_TEXTURE,
							D3DFMT_A32B32G32R32F
							)))
					appErrorf(TEXT("Device does not support 4 component FP render target format."));

				//<@ ava specific : 2007. 7. 3 changmin
				//if (GIsLowEndHW)
				//{
				//	//// Setup the FP RGB pixel formats based on 16-bit FP support.
				//	//GPixelFormats[ PF_FloatRGB	].PlatformFormat	= D3DFMT_X8R8G8B8;
				//	//GPixelFormats[ PF_FloatRGB	].BlockBytes		= 4;
				//	//GPixelFormats[ PF_FloatRGBA	].PlatformFormat	= D3DFMT_A8R8G8B8;
				//	//GPixelFormats[ PF_FloatRGBA	].BlockBytes		= 4;
				//	//GPixelFormats[ PF_FloatRGBA_Full ].PlatformFormat	= D3DFMT_A8R8G8B8;
				//	//GPixelFormats[ PF_FloatRGBA_Full ].BlockBytes		= 4;
				//}
				//else
				//>@ ava
				{
					// Setup the FP RGB pixel formats based on 16-bit FP support.
					GPixelFormats[ PF_FloatRGB	].PlatformFormat	= bSupports16bitRT ? D3DFMT_A16B16G16R16F : D3DFMT_A32B32G32R32F;
					GPixelFormats[ PF_FloatRGB	].BlockBytes		= bSupports16bitRT ? 8 : 16;
					GPixelFormats[ PF_FloatRGBA	].PlatformFormat	= bSupports16bitRT ? D3DFMT_A16B16G16R16F : D3DFMT_A32B32G32R32F;
					GPixelFormats[ PF_FloatRGBA	].BlockBytes		= bSupports16bitRT ? 8 : 16;
					GPixelFormats[ PF_FloatRGBA_Full ].PlatformFormat	= bSupports16bitRT ? D3DFMT_A16B16G16R16F : D3DFMT_A32B32G32R32F;
					GPixelFormats[ PF_FloatRGBA_Full ].BlockBytes		= bSupports16bitRT ? 8 : 16;
				}				

				// Determine whether alpha blending with a floating point framebuffer is supported.
				if (FAILED(
					GDirect3D->CheckDeviceFormat(
						AdapterIndex,
						DeviceType,
						CurrentDisplayMode.Format,
						D3DUSAGE_RENDERTARGET | D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
						D3DRTYPE_TEXTURE,
						bSupports16bitRT ? D3DFMT_A16B16G16R16F : D3DFMT_A32B32G32R32F
					)))
				{
					//only overwrite if not supported, since the previous setting may have been specified on the command line
					GSupportsFPBlending = FALSE;
				}				

				// can't read from depth textures on pc (although certain ATI cards can, but DST's are not optimal for use as depth buffers)
				GSupportsDepthTextures = FALSE;

				// Determine whether hardware shadow mapping is supported.
				GSupportsHardwarePCF = SUCCEEDED(
					GDirect3D->CheckDeviceFormat(
						AdapterIndex, 
						DeviceType, 
						CurrentDisplayMode.Format, 
						D3DUSAGE_DEPTHSTENCIL,
						D3DRTYPE_TEXTURE,
						D3DFMT_D24X8 
					)
				);

				if (!IsSM2Platform( GRHIShaderPlatform ))
				{
					// Check for D24 support, which indicates that ATI's Fetch4 is also supported
					GSupportsFetch4 = SUCCEEDED(
						GDirect3D->CheckDeviceFormat(
						AdapterIndex, 
						DeviceType, 
						CurrentDisplayMode.Format, 
						D3DUSAGE_DEPTHSTENCIL,
						D3DRTYPE_TEXTURE,
						(D3DFORMAT)(MAKEFOURCC('D','F','2','4'))
						)
						);
				}

				// Check for support of Nvidia's depth bounds test in D3D9
				GD3DDepthBoundsHackSupported = SUCCEEDED(
					GDirect3D->CheckDeviceFormat(
					AdapterIndex, 
					DeviceType, 
					CurrentDisplayMode.Format, 
					0,
					D3DRTYPE_SURFACE,
					(D3DFORMAT)(MAKEFOURCC('N','V','D','B'))
					)
					);				

				// Determine whether filtering of floating point textures is supported.
				if (FAILED(
					GDirect3D->CheckDeviceFormat(
					AdapterIndex,
					DeviceType,
					CurrentDisplayMode.Format,
					D3DUSAGE_RENDERTARGET | D3DUSAGE_QUERY_FILTER,
					D3DRTYPE_TEXTURE,
					bSupports16bitRT ? D3DFMT_A16B16G16R16F : D3DFMT_A32B32G32R32F
					)))
				{
					//only overwrite if not supported, since the previous setting may have been specified on the command line
					GSupportsFPFiltering = FALSE;
				}

				// Verify that the device supports the required single component 32-bit floating pointer render target format.

				if(FAILED(GDirect3D->CheckDeviceFormat(
							AdapterIndex,
							DeviceType,
							CurrentDisplayMode.Format,
							D3DUSAGE_RENDERTARGET,
							D3DRTYPE_TEXTURE,
							D3DFMT_R32F
							)))
					appErrorf(TEXT("Device does not support 1x32 FP render target format."));

				// Query for YUV texture format support.

				if( SUCCEEDED(GDirect3D->CheckDeviceFormat( AdapterIndex, DeviceType, CurrentDisplayMode.Format, D3DUSAGE_DYNAMIC, D3DRTYPE_TEXTURE, D3DFMT_UYVY	) ) )
				{
					// Query for SRGB read support (gamma correction on texture sampling) for YUV texture format. E.g. not supported on Radeon 9800.
					if( FAILED(GDirect3D->CheckDeviceFormat( AdapterIndex, DeviceType, CurrentDisplayMode.Format, D3DUSAGE_QUERY_SRGBREAD, D3DRTYPE_TEXTURE, D3DFMT_UYVY	) ) )
					{
						GPixelFormats[PF_UYVY].Flags |= PF_REQUIRES_GAMMA_CORRECTION;
					}
					else
					{
						GPixelFormats[PF_UYVY].Flags &= ~ PF_REQUIRES_GAMMA_CORRECTION;
					}

					GPixelFormats[PF_UYVY].Supported = 1;
				}
				else
				{
					GPixelFormats[PF_UYVY].Supported = 0;
				}

				// Try creating a new Direct3D device till it either succeeds or fails with an error code != D3DERR_DEVICELOST.
				
				while( 1 )
				{
					// Games for Windows Live requires the multithreaded flag or it will BSOD
					DWORD CreationFlags = D3DCREATE_FPU_PRESERVE /*| D3DCREATE_MULTITHREADED*/;
					
					// use software vertex shader if no HW T&L support
					CreationFlags |= TRUE == bSupportsHardwareTnL ? D3DCREATE_HARDWARE_VERTEXPROCESSING : D3DCREATE_SOFTWARE_VERTEXPROCESSING;
					
					// Driver management of video memory currently is FAR from being optimal for streaming!
					CreationFlags |= D3DCREATE_DISABLE_DRIVER_MANAGEMENT;

					UINT AdapterNumber = D3DADAPTER_DEFAULT;

#if ENABLE_GPU_PROFILING
					// Automatically detect nvperfhud device
					for( UINT Adapter=0; Adapter < GDirect3D->GetAdapterCount(); Adapter++ )
					{
						D3DADAPTER_IDENTIFIER9 Identifier;
						HRESULT Res = GDirect3D->GetAdapterIdentifier( Adapter, 0, &Identifier );
						if( appStrcmp( *FString(Identifier.Description), TEXT("NVIDIA NVPerfHUD") ) == 0 ||
							appStrcmp( *FString(Identifier.Description), TEXT("NVIDIA\tNVPerfHUD") ) == 0 ||
							appStrstr( *FString(Identifier.Description), TEXT("PerfHUD") ) != 0 )
						{
							AdapterNumber = Adapter;
							DeviceType = D3DDEVTYPE_REF;

							GGPUPerfAnalyzerLoaded = TRUE;
							debugf(NAME_Log, TEXT("Found NVIDIA PerfHUD(%s)"), *FString(Identifier.Description));
							break;
						}
					}
#endif

					Result = GDirect3D->CreateDevice(
						AdapterNumber == INDEX_NONE ? D3DADAPTER_DEFAULT : AdapterNumber,
						DeviceType,
						NewDeviceWindow,
						CreationFlags,
						&PresentParameters,
						GDirect3DDevice.GetInitReference()
						);

					if( Result != D3DERR_DEVICELOST )
					{
						break;
					}

					appSleep( 0.5f );
				}				

				VERIFYD3DRESULT(Result);

				// 혹시나 스플래쉬가 있다면 꺼준다.
				extern void appHideSplashEx();
				appHideSplashEx();

				// 한번 까맣게 처리
				GDirect3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0f, 0 );
				GDirect3DDevice->Present( NULL, NULL, NULL, NULL );

				// Initialize the platform pixel format map.
				GPixelFormats[ PF_Unknown		].PlatformFormat	= D3DFMT_UNKNOWN;
				GPixelFormats[ PF_A32B32G32R32F	].PlatformFormat	= D3DFMT_A32B32G32R32F;
				GPixelFormats[ PF_A8R8G8B8		].PlatformFormat	= D3DFMT_A8R8G8B8;
				GPixelFormats[ PF_G8			].PlatformFormat	= D3DFMT_L8;
				GPixelFormats[ PF_G16			].PlatformFormat	= D3DFMT_UNKNOWN;	// Not supported for rendering.
				GPixelFormats[ PF_DXT1			].PlatformFormat	= D3DFMT_DXT1;
				GPixelFormats[ PF_DXT3			].PlatformFormat	= D3DFMT_DXT3;
				GPixelFormats[ PF_DXT5			].PlatformFormat	= D3DFMT_DXT5;
				GPixelFormats[ PF_UYVY			].PlatformFormat	= D3DFMT_UYVY;
				GPixelFormats[ PF_DepthStencil	].PlatformFormat	= D3DFMT_D24S8;
				GPixelFormats[ PF_ShadowDepth	].PlatformFormat	= D3DFMT_D24X8;
				GPixelFormats[ PF_FilteredShadowDepth ].PlatformFormat = D3DFMT_D24X8;
				GPixelFormats[ PF_R32F			].PlatformFormat	= D3DFMT_R32F;
				GPixelFormats[ PF_G16R16		].PlatformFormat	= D3DFMT_G16R16;
				GPixelFormats[ PF_G16R16F		].PlatformFormat	= D3DFMT_G16R16F;
				GPixelFormats[ PF_G32R32F		].PlatformFormat	= D3DFMT_G32R32F;				
				GPixelFormats[ PF_D24 ].PlatformFormat    = (D3DFORMAT)(MAKEFOURCC('D','F','2','4'));				

				// Fall back to D3DFMT_A8R8G8B8 if D3DFMT_G16R16 is not a supported render target format. In theory
				// we could try D3DFMT_A16B16G16R16 but cards not supporting G16R16 are usually too slow to use 'bigger' format.
				if( FAILED( GDirect3D->CheckDeviceFormat(
					AdapterIndex,
					DeviceType,
					CurrentDisplayMode.Format,
					D3DUSAGE_RENDERTARGET,
					D3DRTYPE_TEXTURE,
					D3DFMT_G16R16 ) ) )
				{
					GPixelFormats[ PF_G16R16 ].PlatformFormat = D3DFMT_A8R8G8B8;
				}
				else
				{
					GPixelFormats[ PF_G16R16 ].PlatformFormat = D3DFMT_G16R16;
				}

				// Fall back to D3DFMT_A8R8G8B8 if D3DFMT_A2B10G10R10 is not a supported render target format.
				if( FAILED( GDirect3D->CheckDeviceFormat(
					AdapterIndex,
					DeviceType,
					CurrentDisplayMode.Format,
					D3DUSAGE_RENDERTARGET,
					D3DRTYPE_TEXTURE,
					D3DFMT_A2B10G10R10 ) ) )
				{
					GPixelFormats[ PF_A2B10G10R10 ].PlatformFormat = D3DFMT_A8R8G8B8;
				}
				else
				{
					GPixelFormats[ PF_A2B10G10R10 ].PlatformFormat = D3DFMT_A2B10G10R10;
				}

				// Fall back to D3DFMT_A8R8G8B8 if D3DFMT_A16B16G16R16 is not a supported render target format. In theory
				// we could try D3DFMT_A32B32G32R32 but cards not supporting the 16 bit variant are usually too slow to use
				// the full 32 bit format.
				if( FAILED( GDirect3D->CheckDeviceFormat(
					AdapterIndex,
					DeviceType,
					CurrentDisplayMode.Format,
					D3DUSAGE_RENDERTARGET,
					D3DRTYPE_TEXTURE,
					D3DFMT_A16B16G16R16 ) ) )
				{
					GPixelFormats[ PF_A16B16G16R16 ].PlatformFormat = D3DFMT_A8R8G8B8;
				}
				else
				{
					GPixelFormats[ PF_A16B16G16R16 ].PlatformFormat = D3DFMT_A16B16G16R16;
				}

				// Get pointers to the device's back buffer and depth buffer.
				TD3DRef<IDirect3DSurface9> BackBuffer;
				VERIFYD3DRESULT(GDirect3DDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,BackBuffer.GetInitReference()));
				GD3DBackBuffer = FSurfaceRHIRef(FTexture2DRHIRef(),NULL,BackBuffer,TRUE);

				// Notify all initialized FRenderResources that there's a valid RHI device to create their RHI resources for now.
				check(!GIsRHIInitialized);
				GIsRHIInitialized = TRUE;
				for(TLinkedList<FRenderResource*>::TIterator ResourceIt(FRenderResource::GetResourceList());ResourceIt;ResourceIt.Next())
				{
					ResourceIt->InitDynamicRHI();
				}
				for(TLinkedList<FRenderResource*>::TIterator ResourceIt(FRenderResource::GetResourceList());ResourceIt;ResourceIt.Next())
				{
					ResourceIt->InitRHI();
				}
			}

			// Update saved device settings.
			GD3DFullscreenViewport = NewFullscreenViewport;
			GD3DDeviceSizeX = NewDeviceSizeX;
			GD3DDeviceSizeY = NewDeviceSizeY;
			GD3DDeviceWindow = NewDeviceWindow;
			GD3DIsFullscreenDevice = NewDeviceFullscreen;
			GD3DDeviceLost = FALSE;
		}

#if ENABLE_GPU_PROFILING
		if (GGPUPerfAnalyzerLoaded)
		{
			GEmitDrawEvents = TRUE;
		}
#endif
	}
	else
	{
		// If no viewports are open, clean up the existing device.
		CleanupD3DDevice();
		GDirect3DDevice = NULL;
		GD3DBackBuffer.Release();		
	}
}

/**
 * Returns a supported screen resolution that most closely matches the input.
 * @param Width - Input: Desired resolution width in pixels. Output: A width that the platform supports.
 * @param Height - Input: Desired resolution height in pixels. Output: A height that the platform supports.
 */
void GetSupportedResolution( UINT &Width, UINT &Height )
{
	// Create the Direct3D object if necessary.
	if ( !GDirect3D )
	{
		*GDirect3D.GetInitReference() = Direct3DCreate9(D3D_SDK_VERSION);
	}
	if ( !GDirect3D )
	{
		appErrorf(
			NAME_FriendlyError,
			TEXT("Please install DirectX 9.0c or later (see Release Notes for instructions on how to obtain it)")
			);
	}

	// Determine the adapter and device type to use.
	UINT AdapterIndex = D3DADAPTER_DEFAULT;
	D3DDEVTYPE DeviceType = DEBUG_SHADERS ? D3DDEVTYPE_REF : D3DDEVTYPE_HAL;

	// Find the screen size that most closely matches the desired resolution.
	D3DDISPLAYMODE BestMode = { 0, 0, 0, D3DFMT_A8R8G8B8 };
	UINT InitializedMode = FALSE;
	UINT NumAdapterModes = GDirect3D->GetAdapterModeCount(AdapterIndex,D3DFMT_X8R8G8B8);
	for(UINT ModeIndex = 0;ModeIndex < NumAdapterModes;ModeIndex++)
	{
		D3DDISPLAYMODE DisplayMode;
		GDirect3D->EnumAdapterModes(AdapterIndex,D3DFMT_X8R8G8B8,ModeIndex,&DisplayMode);

		UBOOL IsEqualOrBetterWidth = Abs((INT)DisplayMode.Width - (INT)Width) <= Abs((INT)BestMode.Width - (INT)Width);
		UBOOL IsEqualOrBetterHeight = Abs((INT)DisplayMode.Height - (INT)Height) <= Abs((INT)BestMode.Height - (INT)Height);
		if(!InitializedMode || (IsEqualOrBetterWidth && IsEqualOrBetterHeight))
		{
			BestMode = DisplayMode;
			InitializedMode = TRUE;
		}
	}
	check(InitializedMode);
	Width = BestMode.Width;
	Height = BestMode.Height;
}

void GetSupportedResolutions( TMultiMap< FScreenResolution, FScreenResolution >& Result )
{
	// Create the Direct3D object if necessary.
	if ( !GDirect3D )
	{
		*GDirect3D.GetInitReference() = Direct3DCreate9(D3D_SDK_VERSION);
	}
	if ( !GDirect3D )
	{
		appErrorf(
			NAME_FriendlyError,
			TEXT("Please install DirectX 9.0c or later (see Release Notes for instructions on how to obtain it)")
			);
	}

	// Determine the adapter and device type to use.
	UINT AdapterIndex = D3DADAPTER_DEFAULT;
	D3DDEVTYPE DeviceType = DEBUG_SHADERS ? D3DDEVTYPE_REF : D3DDEVTYPE_HAL;

	// Find the screen size that most closely matches the desired resolution.
	D3DDISPLAYMODE BestMode = { 0, 0, 0, D3DFMT_A8R8G8B8 };
	UINT InitializedMode = FALSE;
	UINT NumAdapterModes = GDirect3D->GetAdapterModeCount(AdapterIndex,D3DFMT_X8R8G8B8);
	for(UINT ModeIndex = 0;ModeIndex < NumAdapterModes;ModeIndex++)
	{
		D3DDISPLAYMODE DisplayMode;
		if (FAILED(GDirect3D->EnumAdapterModes(AdapterIndex,D3DFMT_X8R8G8B8,ModeIndex,&DisplayMode)))
		{
			continue;
		}

		FScreenResolution NewResolution;
		NewResolution.Width = DisplayMode.Width;
		NewResolution.Height = DisplayMode.Height;

		if (DisplayMode.Width < 800 || DisplayMode.Height < 600)
		{
			continue;
		}

		FScreenResolution AspectRatio;
		AspectRatio.Factor( NewResolution );

		// 16:10 때문에 -_-; 8:5로 나옴
		if (AspectRatio.Width == 8)
		{
			AspectRatio.Width *= 2;
			AspectRatio.Height *= 2;
		}

		Result.AddUnique( AspectRatio, NewResolution );
	}	
}

FCommandContextRHI* RHIGetGlobalContext()
{
	return NULL;
}

#endif
