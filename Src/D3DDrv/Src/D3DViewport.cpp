/*=============================================================================
	D3DViewport.cpp: D3D viewport RHI implementation.
	Copyright 2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "D3DDrvPrivate.h"
#include "D3DPerfSaver.h"

#if USE_D3D_RHI

/**
 * A D3D event query resource.
 */
class FD3DEventQuery : public FRenderResource
{
public:

	/** Issues an event for the query to poll. */
	void IssueEvent()
	{
		Query->Issue(D3DISSUE_END);
	}

	/** Waits for the event query to finish. */
	void WaitForCompletion()
	{
		DOUBLE StartTime = appSeconds();
		while(1)
		{
			UBOOL bRenderingFinished = FALSE;
			HRESULT Result = Query->GetData(&bRenderingFinished,sizeof(bRenderingFinished),D3DGETDATA_FLUSH);
			if( FAILED(Result) )
			{
				if( Result == D3DERR_DEVICELOST )
				{
					GD3DDeviceLost = 1;
					break;
				}
				VERIFYD3DRESULT(Result);
			}
			else if( Result == S_OK && bRenderingFinished )
			{
				break;
			}
			else
			{
				appSleep(0);
			}

			if((appSeconds() - StartTime) > 0.5)
			{
				debugf(TEXT("Timed out while waiting for GPU to catch up. (500 ms)"));
				break;
			}
		};
	}

	// FRenderResource interface.
	virtual void InitDynamicRHI()
	{
		VERIFYD3DRESULT(GDirect3DDevice->CreateQuery(D3DQUERYTYPE_EVENT,Query.GetInitReference()));

		// Initialize the query by issuing an initial event.
		IssueEvent();
	}
	virtual void ReleaseDynamicRHI()
	{
		Query.Release();
	}

private:	
	TD3DRef<IDirect3DQuery9> Query;
};

//
// Globals.
//

/** A list of all viewport RHIs that have been created. */
TArray<FD3DViewport*> GD3DViewports;

/** The viewport which is currently being drawn. */
FD3DViewport* GD3DDrawingViewport = NULL;

/** A global event query used to check the GPU's progress on previously rendered frames. */
TGlobalResource<FD3DEventQuery> GD3DFrameSyncEvent;

FD3DViewport::FD3DViewport(void* InWindowHandle,UINT InSizeX,UINT InSizeY,UBOOL bInIsFullscreen):
	WindowHandle(InWindowHandle),
	SizeX(InSizeX),
	SizeY(InSizeY),
	bIsFullscreen(bInIsFullscreen)
{
	GD3DViewports.AddItem(this);
	UpdateD3DDeviceFromViewports();
}

FD3DViewport::~FD3DViewport()
{
	GD3DViewports.RemoveItem(this);
	UpdateD3DDeviceFromViewports();
}

void FD3DViewport::Resize(UINT InSizeX,UINT InSizeY,UBOOL bInIsFullscreen)
{
	SizeX = InSizeX;
	SizeY = InSizeY;
	bIsFullscreen = bInIsFullscreen;
	UpdateD3DDeviceFromViewports();
}

/**
 * Returns a supported screen resolution that most closely matches input.
 * @param Width - Input: Desired resolution width in pixels. Output: A width that the platform supports.
 * @param Height - Input: Desired resolution height in pixels. Output: A height that the platform supports.
 */
void FD3DViewport::GetSupportedResolution( UINT &Width, UINT &Height )
{
	::GetSupportedResolution( Width, Height );
}

/*=============================================================================
 *	The following RHI functions must be called from the main thread.
 *=============================================================================*/
FViewportRHIRef RHICreateViewport(void* WindowHandle,UINT SizeX,UINT SizeY,UBOOL bIsFullscreen)
{
	check( IsInGameThread() );
	return new FD3DViewport(WindowHandle,SizeX,SizeY,bIsFullscreen);
}

void RHIResizeViewport(FViewportRHIParamRef Viewport,UINT SizeX,UINT SizeY,UBOOL bIsFullscreen)
{
	check( IsInGameThread() );
	Viewport->Resize(SizeX,SizeY,bIsFullscreen);
}

UBOOL GPendingRecreateDevice = FALSE;
extern UBOOL GForceRecreateDevice;

void RHITick( FLOAT DeltaTime )
{
	check( IsInGameThread() );

	// Check to see if the device has been lost.
	if ( GD3DDeviceLost )
	{
		UpdateD3DDeviceFromViewports();
	}

	// 그냥 요렇게만 하는 게 정상인 듯;
	// 기억으로는 GD3DDeviceLost를 건드렸다가 낭패를 봤던 것 같아서 따로 두고, 별도로 구현한 것 같은데
	// 그게 문제!
	if ( GPendingRecreateDevice )
	{
		GPendingRecreateDevice = FALSE;

		GForceRecreateDevice = TRUE;

		UpdateD3DDeviceFromViewports();

		GForceRecreateDevice = FALSE;
	}
}

/*=============================================================================
 *	Viewport functions.
 *=============================================================================*/

void RHIBeginDrawingViewport(FViewportRHIParamRef Viewport)
{
	SCOPE_CYCLE_COUNTER(STAT_PresentTime);

	check(!GD3DDrawingViewport);
	GD3DDrawingViewport = Viewport;

	// Wait for the device to begin rendering the previous frame before starting a new frame.
	//GD3DFrameSyncEvent.WaitForCompletion();
	//GD3DFrameSyncEvent.IssueEvent();

	// D3D Perf saver :)
	GD3DPerfSaver->BeginFrame();

	// Tell D3D we're going to start rendering.
	GDirect3DDevice->BeginScene();

	// update any resources that needed a deferred update
	FDeferredUpdateResource::UpdateResources();

	// Set the configured D3D render state.
	for(INT SamplerIndex = 0;SamplerIndex < 16;SamplerIndex++)
	{
		extern INT GMaxLogAnisotropy;
		GDirect3DDevice->SetSamplerState(SamplerIndex,D3DSAMP_MAXANISOTROPY,1 << Clamp(GSystemSettings->Anisotropy,0,GMaxLogAnisotropy));
	}

	// Set the render target and viewport.
	RHISetRenderTarget(NULL, GD3DBackBuffer, FSurfaceRHIRef());

	extern UBOOL GIsNVidia;
	// Allow the user to force minimum driver shader optimizations on legacy NVIDIA hardware to reduce hitching.
	if (GIsNVidia)
	{
		UBOOL bUseMinimalNVIDIADriverShaderOptimization = FALSE;
		GConfig->GetBool(TEXT("Engine.ISVHacks"), TEXT("UseMinimalNVIDIADriverShaderOptimization"), bUseMinimalNVIDIADriverShaderOptimization, GEngineIni);
		if(bUseMinimalNVIDIADriverShaderOptimization)
		{
			GDirect3DDevice->SetRenderState(D3DRS_ADAPTIVETESS_Y,MAKEFOURCC('C','O','P','M'));
		}
	}
}

void RHIEndDrawingViewport(FViewportRHIParamRef Viewport,UBOOL bPresent,UBOOL bLockToVsync)
{
	SCOPE_CYCLE_COUNTER(STAT_PresentTime);

	check(GD3DDrawingViewport == Viewport);
	GD3DDrawingViewport = NULL;

	extern const FSceneView* GCurrentView;	// Cached current view for RHISetViewParameters.
	GCurrentView = NULL;

	// Clear references the device might have to resources.
	GDirect3DDevice->SetRenderTarget(0,GD3DBackBuffer);
	GDirect3DDevice->SetDepthStencilSurface(NULL);

	for(UINT TextureIndex = 0;TextureIndex < 16;TextureIndex++)
	{
		GDirect3DDevice->SetTexture(TextureIndex,NULL);
	}

	GD3DPerfSaver->SetVertexShader(NULL);

	for(UINT StreamIndex = 0;StreamIndex < 16;StreamIndex++)
	{
		GDirect3DDevice->SetStreamSource(StreamIndex,NULL,0,0);
	}

	GDirect3DDevice->SetIndices(NULL);
	GD3DPerfSaver->SetPixelShader(NULL);

	// Tell D3D we're done rendering.
	GDirect3DDevice->EndScene();

	if(bPresent)
	{
		// Present the back buffer to the viewport window.
		HRESULT Result;
		if(Viewport->IsFullscreen())
		{
			Result = GDirect3DDevice->Present(NULL,NULL,NULL,NULL);
		}
		else
		{
			RECT SourceRect;
			SourceRect.left		= SourceRect.top = 0;
			SourceRect.right	= Viewport->GetSizeX();
			SourceRect.bottom	= Viewport->GetSizeY();
			Result				= GDirect3DDevice->Present(&SourceRect,NULL,(HWND)Viewport->GetWindowHandle(),NULL);
		}

		// Detect a lost device.
		if(Result == D3DERR_DEVICELOST)
		{
			// This variable is checked periodically by the main thread.
			GD3DDeviceLost = TRUE;
		}
		else
		{
			VERIFYD3DRESULT(Result);
		}
	}

	GD3DFrameSyncEvent.WaitForCompletion();
	GD3DFrameSyncEvent.IssueEvent();

	// If the input latency timer has been triggered, block until the GPU is completely
	// finished displaying this frame and calculate the delta time.
	if ( GInputLatencyTimer.RenderThreadTrigger )
	{		
		GD3DFrameSyncEvent.WaitForCompletion();
		DWORD EndTime = appCycles();
		GInputLatencyTimer.DeltaTime = EndTime - GInputLatencyTimer.StartTime;
		GInputLatencyTimer.RenderThreadTrigger = FALSE;
	}
}

FSurfaceRHIRef RHIGetViewportBackBuffer(FViewportRHIParamRef Viewport)
{
	return GD3DBackBuffer;
}

#endif
