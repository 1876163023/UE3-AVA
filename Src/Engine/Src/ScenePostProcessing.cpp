/*=============================================================================
	ScenePostProcessing.cpp: Scene post processing implementation.
	Copyright 2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "SceneFilterRendering.h"

static const UINT MAX_CCLAYERS = 3;

/** Encapsulates the gamma correction pixel shader. */
class FGammaCorrectionPixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FGammaCorrectionPixelShader,Global);

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}

	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
	}

	/** Default constructor. */
	FGammaCorrectionPixelShader() {}

public:

	FShaderParameter SceneTextureParameter;
	FShaderParameter InverseGammaParameter;	
	FShaderParameter TonemapTextureParameter;
	FShaderParameter TonemapRangeParameter;
	//FShaderParameter OverlayColorParameter;	// <@ ava specific ; 2007. 3. 23 changmin : 우리는 필요 없어요~ //>@

	/** Initialization constructor. */
	FGammaCorrectionPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FGlobalShader(Initializer)
	{
		SceneTextureParameter.Bind(Initializer.ParameterMap,TEXT("SceneColorTexture"));
		InverseGammaParameter.Bind(Initializer.ParameterMap,TEXT("InverseGamma"));		
	}

	// FGlobalShader interface.
	virtual void Serialize(FArchive& Ar)
	{
		FGlobalShader::Serialize(Ar);
		Ar << SceneTextureParameter << InverseGammaParameter;
	}
};

//<@ ava specific ; 2007. 7. 4 changmin
class FSceneCopyPixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FSceneCopyPixelShader,Global);

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}

	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
	}

	/** Default constructor. */
	FSceneCopyPixelShader() {}

public:

	FShaderParameter SceneTextureParameter;

	/** Initialization constructor. */
	FSceneCopyPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FGlobalShader(Initializer)
	{
		SceneTextureParameter.Bind(Initializer.ParameterMap,TEXT("SceneColorTexture"));
	}

	// FGlobalShader interface.
	virtual void Serialize(FArchive& Ar)
	{
		FGlobalShader::Serialize(Ar);
		Ar << SceneTextureParameter;
	}
};
//>@ ava specific 

//<@ ava specific ; 2007. 7. 4 changmin
class FFillColorPixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FFillColorPixelShader,Global);

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}

	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
	/** Default constructor. */
	}

	FFillColorPixelShader() {}

public:

	FShaderParameter FillColorParameter;

	/** Initialization constructor. */
	FFillColorPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FGlobalShader(Initializer)
	{
		FillColorParameter.Bind(Initializer.ParameterMap,TEXT("FillColor"));
	}

	// FGlobalShader interface.
	virtual void Serialize(FArchive& Ar)
	{
		FGlobalShader::Serialize(Ar);		
		Ar << FillColorParameter;
	}

	void Set( FCommandContextRHI* Context, FShader* PixelShader, const FLinearColor& FillColor )
	{
		SetPixelShaderValue(
			Context,
			PixelShader->GetPixelShader(),
			FillColorParameter,
			FillColor
			);
	}
};
//>@ ava specific 

template <class TonemapPolicy>
class FTonemapShaderParameters
{
public :
	FShaderParameter SceneTextureParameter;
	FShaderParameter InverseGammaParameter;	
	FShaderParameter TonemapTextureParameter;
	FShaderParameter TonemapRangeParameter;
	FShaderParameter ColorCorrectionTextureParameters;
	FShaderParameter ColorCorrectionAlphaParameters;
	//FShaderParameter OverlayColorParameter;	// <@ ava specific ; 2007. 3. 23 changmin : 우리는 필요 없어요~ //>@

	/** Default constructor. */
	FTonemapShaderParameters() {}

	/** Initialization constructor. */
	FTonemapShaderParameters(const FShaderParameterMap& ParameterMap)
	{
		SceneTextureParameter.Bind(ParameterMap,TEXT("SceneColorTexture"));
		InverseGammaParameter.Bind(ParameterMap,TEXT("InverseGamma"));		
		TonemapTextureParameter.Bind(ParameterMap,TEXT("TonemapTexture"));
		TonemapRangeParameter.Bind(ParameterMap,TEXT("TonemapRange"));

		if (TonemapPolicy::NumColorCorrectionLayers > 0)
		{
			ColorCorrectionTextureParameters.Bind(ParameterMap,TEXT("CCTextures"));
			ColorCorrectionAlphaParameters.Bind(ParameterMap,TEXT("CCAlphas"));
		}
	}

	/** Set the material shader parameter values. */
	void Set(
		FCommandContextRHI* Context, 
		FShader* PixelShader, 
		FLOAT DisplayGamma, 
		const FExposureData& ExposureData, 
		FLOAT Opacity
		)
	{
		SetPixelShaderValue(
			Context,
			PixelShader->GetPixelShader(),
			InverseGammaParameter,
			DisplayGamma
			);

		SetPixelShaderValue(
			Context,
			PixelShader->GetPixelShader(),
			TonemapRangeParameter, 
			FLinearColor( 
				ExposureData.LuminanceHistogram.WorldLuminaceScale / (ExposureData.LuminanceHistogram.WorldLuminaceMax + 1e-6f),
				0,
				0,
				Opacity ) );

		SetTextureParameter(
			Context,
			PixelShader->GetPixelShader(),
			TonemapTextureParameter, 
			TStaticSamplerState<SF_Linear>::GetRHI(),
			ExposureData.LuminanceHistogram.TonemapTexture );


		if (GSystemSettings->NeedsUpscale())
		{
			SetTextureParameter(
				Context,
				PixelShader->GetPixelShader(),
				SceneTextureParameter,
				TStaticSamplerState<SF_Linear>::GetRHI(),
				GSceneRenderTargets.GetSceneColorTexture()
				);		
		}
		else
		{
			SetTextureParameter(
				Context,
				PixelShader->GetPixelShader(),
				SceneTextureParameter,
				TStaticSamplerState<>::GetRHI(),
				GSceneRenderTargets.GetSceneColorTexture()
				);		
		}
	}

	virtual void SetColorCorrectionLayer( FCommandContextRHI* Context, FShader* PixelShader, UINT Layer, FTextureRHIParamRef Texture, FLOAT BlendAlpha )
	{
		check( Layer < TonemapPolicy::NumColorCorrectionLayers );

		SetTextureParameter( 
			Context,
			PixelShader->GetPixelShader(),
			ColorCorrectionTextureParameters,
			TStaticSamplerState<SF_Linear>::GetRHI(),
			Texture,
			Layer
			);		

		SetPixelShaderValue( 
			Context,
			PixelShader->GetPixelShader(),
			ColorCorrectionAlphaParameters,			
			BlendAlpha,
			Layer
			);	
	}

	/** Serializer. */
	friend FArchive& operator<<(FArchive& Ar,FTonemapShaderParameters& P)
	{
		Ar << P.SceneTextureParameter << P.InverseGammaParameter << P.TonemapTextureParameter << P.TonemapRangeParameter;

		if (TonemapPolicy::NumColorCorrectionLayers)
		{
			Ar << P.ColorCorrectionTextureParameters << P.ColorCorrectionAlphaParameters;
		}
		
		return Ar;
	}
};

struct FNoColorCorrection
{
	static const UINT NumColorCorrectionLayers = 0;

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}
};

struct FOneLayeredColorCorrection
{
	static const UINT NumColorCorrectionLayers = 1;

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}
};

struct FTwoLayeredColorCorrection
{
	static const UINT NumColorCorrectionLayers = 2;

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}
};

struct FThreeLayeredColorCorrection
{
	static const UINT NumColorCorrectionLayers = 3;

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return Platform == SP_PCD3D;
	}
};

class FTonemapPixelShaderInterface : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FTonemapPixelShaderInterface,Global);

public:
	FTonemapPixelShaderInterface() {}

	FTonemapPixelShaderInterface(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FGlobalShader(Initializer)
	{} 

	virtual void Set(FCommandContextRHI* Context, FShader* PixelShader, FLOAT DisplayGamma, const FExposureData& ExposureData, FLOAT Opacity ) 
	{
	}

	virtual void SetColorCorrectionLayer( FCommandContextRHI* Context, FShader* PixelShader, UINT Layer, FTextureRHIParamRef Texture, FLOAT BlendAlpha )
	{
	}

	virtual UINT GetNumColorCorrectionLayers() const
	{
		return 0;
	}
};

void FColorCorrectionData::ImportData()
{
	FArchive* Ar = GFileManager->CreateFileReader( *SourcePath );

	if (!Ar)
		return;

	TArray<BYTE> Buffer;	
	Buffer.AddZeroed( Resolution * Resolution * Resolution );

	/*Data.Lock(LOCK_READ_WRITE);
	FColor* ColorData = (FColor*)Data.Realloc( Resolution * Resolution * Resolution * sizeof(FColor) );*/

	Data.Empty();
	Data.AddZeroed( Resolution * Resolution * Resolution  );

	FColor* ColorData = &Data(0);

	for (INT Channel=0; Channel<3; ++Channel)
	{
		Ar->Serialize( &Buffer(0), Buffer.Num() );

		const BYTE* Src = &Buffer(0);

		BYTE* Dest = ((BYTE*)ColorData) + (2-Channel);

		for(INT B=0; B<Resolution; B++)
		{
			for (INT G=0; G<Resolution; G++)
			{
				for (INT R=0; R<Resolution; R++)
				{
					*Dest = *Src++;

					Dest += 4;
				}
			}
		}
	}		

	//Data.Unlock();

	if (Resource)
	{
		UpdateResource();
	}

	delete Ar;
}

void FColorCorrectionData::UpdateResource()
{
	if (Resource)
	{
		ReleaseResource();
	}

	if (!GIsUCC)
	{
		Resource = CreateResource();
		if (Resource)
		{
			BeginInitResource(Resource);
		}
	}
}

void FColorCorrectionData::ReleaseResource()
{
	check(Resource);

	// Free the resource.
	ReleaseResourceAndFlush(Resource);
	delete Resource;
	Resource = NULL;
}

FColorCorrectionResource* FColorCorrectionData::CreateResource()
{	
	return new FColorCorrectionResource( this );
}

void FColorCorrectionResource::UploadData()
{
	INT Resolution = Source->Resolution;

	if (Resolution == 0)
		return;

	UINT DestRowPitch, DestSlidePitch;
	BYTE* Buffer = (BYTE*)RHILockTexture3D(Texture3DRHI,0,TRUE,DestRowPitch,DestSlidePitch);	
	
	//check(Source->Data.GetBulkDataSize() == Resolution * Resolution * Resolution * sizeof(FColor));	
	if (Source->Data.Num() != Resolution * Resolution * Resolution)
		return;
	
	//const FColor* Src = (const FColor*)Source->Data.Lock(LOCK_READ_ONLY);
	const FColor* Src = &Source->Data(0);

	for(INT B=0; B<Resolution; B++)
	{
		for (INT G=0; G<Resolution; G++)
		{
			FColor* Dest = (FColor*)(Buffer + DestRowPitch * G + DestSlidePitch * B);
			const FColor* SrcRow = Src + Resolution * ( G + Resolution * B );
			appMemcpy( Dest, SrcRow, sizeof(FColor) * Resolution );	
		}
	}		

	//Source->Data.Unlock();

	RHIUnlockTexture3D(Texture3DRHI,0);
}

void FColorCorrectionResource::InitRHI()
{
	if (Source->Resolution <= 0 || Source->Resolution > 64)
		return;

	// Create the texture RHI.  		
	Texture3DRHI = RHICreateTexture3D(Source->Resolution,Source->Resolution,Source->Resolution,PF_A8R8G8B8,1,0);
	TextureRHI = Texture3DRHI;

	UploadData();
}	

void FColorCorrectionResource::ReleaseRHI()
{
	TextureRHI.Release();
	Texture3DRHI.Release();
}

static FColorCorrectionResource* GCCTextures[MAX_CCLAYERS];
static FLOAT GCCAlphas[MAX_CCLAYERS];

/** Encapsulates the gamma correction pixel shader. */
template <class TonemapPolicy>
class FTonemapPixelShader : public FTonemapPixelShaderInterface 
{
	DECLARE_SHADER_TYPE(FTonemapPixelShader,Global);

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TonemapPolicy::ShouldCache(Platform);
	}

	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("NUM_CC_LAYERS"), *FString::Printf(TEXT("%u"), TonemapPolicy::NumColorCorrectionLayers ));
	}

	/** Default constructor. */
	FTonemapPixelShader() {}

public:
	virtual UINT GetNumColorCorrectionLayers() const
	{
		return TonemapPolicy::NumColorCorrectionLayers;
	}

	virtual void Set(FCommandContextRHI* Context, FShader* PixelShader, FLOAT DisplayGamma, const FExposureData& ExposureData, FLOAT Opacity ) 
	{
		TonemapParameters.Set( Context, PixelShader, DisplayGamma, ExposureData, Opacity );
	}	

	virtual void SetColorCorrectionLayer( FCommandContextRHI* Context, FShader* PixelShader, UINT Layer, FTextureRHIParamRef Texture, FLOAT BlendAlpha )
	{
		check( Layer < TonemapPolicy::NumColorCorrectionLayers );

		TonemapParameters.SetColorCorrectionLayer( Context, PixelShader, Layer, Texture, BlendAlpha );
	}

	/** Initialization constructor. */
	FTonemapPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FTonemapPixelShaderInterface(Initializer), TonemapParameters( Initializer.ParameterMap )
	{		
	}

	// FGlobalShader interface.
	virtual void Serialize(FArchive& Ar)
	{
		FGlobalShader::Serialize(Ar);

		Ar << TonemapParameters;
	}

private :
	FTonemapShaderParameters<TonemapPolicy> TonemapParameters;	
};

/** Encapsulates the gamma correction vertex shader. */
class FGammaCorrectionVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FGammaCorrectionVertexShader,Global);

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}

	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
	}

	/** Default constructor. */
	FGammaCorrectionVertexShader() {}

public:

	/** Initialization constructor. */
	FGammaCorrectionVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FGlobalShader(Initializer)
	{
	}
};

/** Encapsulates the gamma correction pixel shader. */

class FEncodeSceneColorPixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FEncodeSceneColorPixelShader,Global);

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}

	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
	}

	/** Default constructor. */
	FEncodeSceneColorPixelShader() {}

public:

	FShaderParameter SceneTextureParameter;

	/** Initialization constructor. */
	FEncodeSceneColorPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FGlobalShader(Initializer)
	{
		SceneTextureParameter.Bind(Initializer.ParameterMap,TEXT("SceneColorTexture"));		
	}

	// FGlobalShader interface.
	virtual void Serialize(FArchive& Ar)
	{
		FGlobalShader::Serialize(Ar);
		Ar << SceneTextureParameter;
	}
};


/**
* A macro for declaring a new instance of the FBranchingPCFProjectionPixelShader template
*/
#define IMPLEMENT_TONEMAP_SHADER_TYPE(TonemapPolicy,ShaderName,SourceFilename,FunctionName,Frequency,MinVersion,MinLicenseeVersion) \
	template<> \
	FTonemapPixelShader<TonemapPolicy>::ShaderMetaType FTonemapPixelShader<TonemapPolicy>::StaticType( \
	TEXT(#ShaderName), \
	SourceFilename, \
	FunctionName, \
	Frequency, \
	MinVersion, \
	MinLicenseeVersion , \
	FTonemapPixelShader<TonemapPolicy>::ConstructSerializedInstance, \
	FTonemapPixelShader<TonemapPolicy>::ConstructCompiledInstance, \
	FTonemapPixelShader<TonemapPolicy>::ModifyCompilationEnvironment, \
	FTonemapPixelShader<TonemapPolicy>::ShouldCache \
	);

IMPLEMENT_TONEMAP_SHADER_TYPE(FNoColorCorrection,NoColorCorrectionShaderName,TEXT("TonemapPixelShader"),TEXT("Main"),SF_Pixel,VER_REMOVE_COOKED_PHYS_TERRAIN,VER_AVA_COLORCORRECTION);
IMPLEMENT_TONEMAP_SHADER_TYPE(FOneLayeredColorCorrection,OneLayeredColorCorrectionShaderName,TEXT("TonemapPixelShader"),TEXT("Main"),SF_Pixel,VER_REMOVE_COOKED_PHYS_TERRAIN,VER_AVA_COLORCORRECTION);
IMPLEMENT_TONEMAP_SHADER_TYPE(FTwoLayeredColorCorrection,TwoLayeredColorCorrectionShaderName,TEXT("TonemapPixelShader"),TEXT("Main"),SF_Pixel,VER_REMOVE_COOKED_PHYS_TERRAIN,VER_AVA_COLORCORRECTION);
IMPLEMENT_TONEMAP_SHADER_TYPE(FThreeLayeredColorCorrection,ThreeLayeredColorCorrectionShaderName,TEXT("TonemapPixelShader"),TEXT("Main"),SF_Pixel,VER_REMOVE_COOKED_PHYS_TERRAIN,VER_AVA_COLORCORRECTION);

IMPLEMENT_SHADER_TYPE(,FGammaCorrectionPixelShader,TEXT("GammaCorrectionPixelShader"),TEXT("Main"),SF_Pixel,VER_MODULATESHADOWPROJECTION_SHADER_RECOMPILE,VER_AVA_ADD_MINLUMINANCE_SHADER_RECOMPILE);
IMPLEMENT_SHADER_TYPE(,FGammaCorrectionVertexShader,TEXT("GammaCorrectionVertexShader"),TEXT("Main"),SF_Vertex,0,0);
IMPLEMENT_SHADER_TYPE(,FEncodeSceneColorPixelShader,TEXT("EncodeSceneColorPixelShader"),TEXT("Main"),SF_Pixel,VER_MODULATESHADOWPROJECTION_SHADER_RECOMPILE,0);
IMPLEMENT_SHADER_TYPE(,FSceneCopyPixelShader,TEXT("SceneCopyPixelShader"),TEXT("Main"),SF_Pixel,VER_MODULATESHADOWPROJECTION_SHADER_RECOMPILE, VER_AVA_ADD_MINLUMINANCE_SHADER_RECOMPILE);
IMPLEMENT_SHADER_TYPE(,FFillColorPixelShader,TEXT("FillColorPixelShader"),TEXT("Main"),SF_Pixel,VER_MODULATESHADOWPROJECTION_SHADER_RECOMPILE, VER_AVA_FILLCOLOR_RECOMPILE);

extern TGlobalResource<FFilterVertexDeclaration> GFilterVertexDeclaration;

FGlobalBoundShaderStateRHIRef FSceneRenderer::PostProcessBoundShaderState;
static FGlobalBoundShaderStateRHIRef TonemapPostProcessBoundShaderState[MAX_CCLAYERS+1];
static FGlobalBoundShaderStateRHIRef ScreenCopyBoundShaderState;
FGlobalBoundShaderStateRHIRef FSceneRenderer::AutomaticExposureBoundShaderState;
FGlobalBoundShaderStateRHIRef FSceneRenderer::EncodeSceneColorBoundShaderState;

void RHICopyToResolveTarget(FSurfaceRHIParamRef SourceSurface, INT SizeX, INT SizeY, ECubeFace CubeFace=CubeFace_PosX );

FTonemapPixelShaderInterface* GetTonemapPixelShader( UINT NumColorCorrectionLayers, FGlobalBoundShaderStateRHIRef& BoundShaderState )
{
	// Color correction 설정에 따른 쉐이더 branch
	switch (NumColorCorrectionLayers)
	{
	case 0 :
		{
			TShaderMapRef< FTonemapPixelShader<FNoColorCorrection> > PixelShader(GetGlobalShaderMap());
			BoundShaderState = TonemapPostProcessBoundShaderState[0];
			return *PixelShader;
		}		

	case 1 :		
		{
			TShaderMapRef< FTonemapPixelShader<FOneLayeredColorCorrection> > PixelShader(GetGlobalShaderMap());
			BoundShaderState = TonemapPostProcessBoundShaderState[1];
			return *PixelShader;		
		}		
		
	case 2 :		
		{
			TShaderMapRef< FTonemapPixelShader<FTwoLayeredColorCorrection> > PixelShader(GetGlobalShaderMap());
			BoundShaderState = TonemapPostProcessBoundShaderState[2];
			return *PixelShader;
		}
		
	default :
		{
			TShaderMapRef< FTonemapPixelShader<FThreeLayeredColorCorrection> > PixelShader(GetGlobalShaderMap());
			BoundShaderState = TonemapPostProcessBoundShaderState[3];
			return *PixelShader;		
		}
	}
}

FTonemapPixelShaderInterface* SetTonemapShader( 
	FCommandContextRHI* Context, UINT NumColorCorrectionLayers, FVertexDeclarationRHIParamRef VertexDeclaration, FShader* VertexShader, UINT Stride)
{
	FGlobalBoundShaderStateRHIRef BoundShaderState;

	FTonemapPixelShaderInterface* PixelShader = GetTonemapPixelShader( NumColorCorrectionLayers, BoundShaderState );

	SetGlobalBoundShaderState( Context, BoundShaderState, VertexDeclaration, VertexShader, PixelShader, Stride );

	return PixelShader;
}

void FSceneRenderer::MeasureLuminance(const FViewInfo* View)
{
	FSceneViewState* ViewState = (FSceneViewState*)(View->State);	

	if (ViewState == NULL)		
	{
		return;
	}

	// Set the view family's render target/viewport.
	if( GIsATI )
	{
		// ati 는 depth surface가 binding 되어 있어야, occlusion query가 작동을 합니다.
		// 아래의 UpdateExposure call에서 occlusion query를 사용합니다.
		RHISetRenderTarget(GlobalContext,ViewFamily.RenderTarget->RenderTargetSurfaceRHI,GSceneRenderTargets.GetSceneDepthSurface());	
	}
	else
	{
		RHISetRenderTarget(GlobalContext,ViewFamily.RenderTarget->RenderTargetSurfaceRHI,FSurfaceRHIRef());
	}

	// turn off culling and blending
	RHISetRasterizerState(GlobalContext,TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
	RHISetBlendState(GlobalContext,TStaticBlendState<>::GetRHI());	

	// turn off depth reads/writes
	RHISetDepthState(GlobalContext,TStaticDepthState<FALSE,CF_Always>::GetRHI());

	// update exposure :)
	extern UBOOL bVisualizeMeasureLuminance;
	if( bVisualizeMeasureLuminance )
	{
		if( ViewFamily.bClear )
		{
			RHIClear( GlobalContext, TRUE, FLinearColor::Black, FALSE, 0.0f, FALSE, 0 );
			ViewFamily.bClear = FALSE;
		}
	}
	
	ViewState->ExposureData.Measure( GlobalContext, View );	
}

void FSceneRenderer::PostProcessView(const FViewInfo* View)
{
	FSceneViewState* ViewState = (FSceneViewState*)(View->State);		

	// Update Exposure
	if (ViewState != NULL)		
	{
		ViewState->ExposureData.Update( GlobalContext, View );	
	}	

	RHISetRenderTarget(GlobalContext,ViewFamily.RenderTarget->RenderTargetSurfaceRHI,FSurfaceRHIRef());
	
	// turn off culling and blending
	RHISetRasterizerState(GlobalContext,TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
	RHISetBlendState(GlobalContext,TStaticBlendState<>::GetRHI());	

	// turn off depth reads/writes
	RHISetDepthState(GlobalContext,TStaticDepthState<FALSE,CF_Always>::GetRHI());	

	const UBOOL bUseDynamicTonemapping = ViewState && (View->Family->ShowFlags & SHOW_Lighting || View->Family->ShowFlags & SHOW_BumpLighting ) && ViewState->ExposureData.bUseDynamicTonemapping;		

	// Deferred the clear until here so the garbage left in the non rendered regions by the post process effects do not show up
	if( ViewFamily.bClear )
	{
		RHIClear( GlobalContext, TRUE, FLinearColor::Black, FALSE, 0.0f, FALSE, 0 );
		ViewFamily.bClear = FALSE;
	}	

	// EnvCube? :)
	if (ViewFamily.bEncodingOffline)
	{
		TShaderMapRef<FGammaCorrectionVertexShader> VertexShader(GetGlobalShaderMap());
		TShaderMapRef<FEncodeSceneColorPixelShader> PixelShader(GetGlobalShaderMap());		

		SetGlobalBoundShaderState(GlobalContext, EncodeSceneColorBoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader, sizeof(FFilterVertex));

		SetTextureParameter(
			GlobalContext,
			PixelShader->GetPixelShader(),
			PixelShader->SceneTextureParameter,
			TStaticSamplerState<>::GetRHI(),
			GSceneRenderTargets.GetSceneColorTexture()
			);			

		// Draw a quad mapping the blurred pixels in the filter buffer to the scene color buffer.
		DrawDenormalizedQuad(
			GlobalContext,
			View->X,View->Y,
			View->SizeX,View->SizeY,
			View->RenderTargetX,View->RenderTargetY,
			View->RenderTargetSizeX,View->RenderTargetSizeY,
			ViewFamily.RenderTarget->GetSizeX(),ViewFamily.RenderTarget->GetSizeY(),
			GSceneRenderTargets.GetBufferSizeX(),GSceneRenderTargets.GetBufferSizeY()
			);
	}
	else
	{
		TShaderMapRef<FGammaCorrectionVertexShader> VertexShader(GetGlobalShaderMap());
		TShaderMapRef<FGammaCorrectionPixelShader> PixelShader(GetGlobalShaderMap());
		TShaderMapRef<FSceneCopyPixelShader> SceneCopyPixelShader(GetGlobalShaderMap());
		
		//<@ ava specific ; 2007. 1. 16 changmin
		if( GIsLowEndHW )
		{
			SetGlobalBoundShaderState(GlobalContext, ScreenCopyBoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *SceneCopyPixelShader, sizeof(FFilterVertex));

			if (GSystemSettings->NeedsUpscale())
			{
				SetTextureParameter(
					GlobalContext,
					SceneCopyPixelShader->GetPixelShader(),
					SceneCopyPixelShader->SceneTextureParameter,
					TStaticSamplerState<SF_Linear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),
					GSceneRenderTargets.GetSceneColorTexture()
					);
			}
			else
			{
				SetTextureParameter(
					GlobalContext,
					SceneCopyPixelShader->GetPixelShader(),
					SceneCopyPixelShader->SceneTextureParameter,
					TStaticSamplerState<>::GetRHI(),
					GSceneRenderTargets.GetSceneColorTexture()
					);
			}
		}
		else if (bUseDynamicTonemapping && !( ViewFamily.ShowFlags & SHOW_ViewSpaceNormal ))
		{
			FTonemapPixelShaderInterface* TonemapPixelShader = NULL;
			
			// pair가 맞아야 함!
			UINT NumCCLayers = ViewState->ColorCorrectionTickets.Num();

			// SM3에서만 3개까지 compile가능하더라..
			if (GRHIShaderPlatform == SP_PCD3D && NumCCLayers > 3)
			{
				NumCCLayers = 3;
			}
			else if (GRHIShaderPlatform != SP_PCD3D && NumCCLayers > 2)
			{
				NumCCLayers = 2;
			}
			
			TonemapPixelShader = SetTonemapShader( GlobalContext, NumCCLayers, GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, sizeof(FFilterVertex));			

			if (View->Opacity < 1.0f)
			{
				RHISetBlendState(GlobalContext,TStaticBlendState<BO_Add,BF_SourceAlpha,BF_InverseSourceAlpha,BO_Add,BF_Zero,BF_One>::GetRHI());
			}			
			
			const FLOAT InverseGamma = 1.0f / ViewFamily.RenderTarget->GetDisplayGamma();
			TonemapPixelShader->Set( GlobalContext, TonemapPixelShader, InverseGamma, ViewState->ExposureData, View->Opacity );												

			UINT MaxLayers = TonemapPixelShader->GetNumColorCorrectionLayers();

			UINT TargetLayer = 0;
			for (UINT Layer = 0; TargetLayer < NumCCLayers; ++Layer)
			{
				const FColorCorrectionTicket& Ticket = ViewState->ColorCorrectionTickets(Layer + ViewState->ColorCorrectionTickets.Num() - NumCCLayers);

				TonemapPixelShader->SetColorCorrectionLayer(GlobalContext, TonemapPixelShader, TargetLayer++, Ticket.Resource ? Ticket.Resource->TextureRHI : NULL, Ticket.CurrentWeight );									

				//debugf( TEXT("%d %p %.2f"), Layer, Ticket.Resource, Ticket.CurrentWeight );
			}

			check( TargetLayer == NumCCLayers );
		}
		else
		//>@ ava
		{
			SetGlobalBoundShaderState(GlobalContext, PostProcessBoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader, sizeof(FFilterVertex));

			//<@ ava specific ; 2007. 1. 16 changmin
			if( ViewFamily.ShowFlags & SHOW_ViewSpaceNormal )
			{
				SetPixelShaderValue(
					GlobalContext,
					PixelShader->GetPixelShader(),
					PixelShader->InverseGammaParameter,
					1.0f	// gamma correction을 끈다.
					);
			}
			else
			//>@ ava
			{
				SetPixelShaderValue(
					GlobalContext,
					PixelShader->GetPixelShader(),
					PixelShader->InverseGammaParameter,
					1.0f / ViewFamily.RenderTarget->GetDisplayGamma()
					);
			}			

			if (GSystemSettings->NeedsUpscale())
			{
				SetTextureParameter(
					GlobalContext,
					PixelShader->GetPixelShader(),
					PixelShader->SceneTextureParameter,
					TStaticSamplerState<SF_Linear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),
					GSceneRenderTargets.GetSceneColorTexture()
					);
			}
			else
			{
				SetTextureParameter(
					GlobalContext,
					PixelShader->GetPixelShader(),
					PixelShader->SceneTextureParameter,
					TStaticSamplerState<>::GetRHI(),
					GSceneRenderTargets.GetSceneColorTexture()
					);
			}
		}

		//<@ ava specific ; 2007. 4. 6 changmin test.... occlusion query
		extern UBOOL bVisualizeMeasureLuminance;
		if( !bVisualizeMeasureLuminance )
		{
			if (GSystemSettings->NeedsUpscale())
			{
				INT UnscaledViewX = 0;
				INT UnscaledViewY = 0;
				UINT UnscaledViewSizeX = 0;
				UINT UnscaledViewSizeY = 0;

				//convert view constraints to their unscaled versions
				GSystemSettings->UnScaleScreenCoords(
					UnscaledViewX, UnscaledViewY, 
					UnscaledViewSizeX, UnscaledViewSizeY, 
					View->X, View->Y, 
					View->SizeX, View->SizeY);

				//RHISetViewport( GlobalContext, UnscaledViewX, UnscaledViewY, 0.0f, UnscaledViewSizeX, UnscaledViewSizeY, 1.0f );

				// Draw a quad mapping scene color to the view's render target
				DrawDenormalizedQuad(
					GlobalContext,
					UnscaledViewX,UnscaledViewY,
					UnscaledViewSizeX,UnscaledViewSizeY,
					View->RenderTargetX,View->RenderTargetY,
					View->RenderTargetSizeX,View->RenderTargetSizeY,
					ViewFamily.RenderTarget->GetSizeX(),ViewFamily.RenderTarget->GetSizeY(),
					GSceneRenderTargets.GetBufferSizeX(),GSceneRenderTargets.GetBufferSizeY()
					);
			}
			else
			{
				// Draw a quad mapping the blurred pixels in the filter buffer to the scene color buffer.
				DrawDenormalizedQuad(
					GlobalContext,
					View->X,View->Y,
					View->SizeX,View->SizeY,
					View->RenderTargetX,View->RenderTargetY,
					View->RenderTargetSizeX,View->RenderTargetSizeY,
					ViewFamily.RenderTarget->GetSizeX(),ViewFamily.RenderTarget->GetSizeY(),
					GSceneRenderTargets.GetBufferSizeX(),GSceneRenderTargets.GetBufferSizeY()
					);
			}
		}

		RHISetBlendState(GlobalContext,TStaticBlendState<>::GetRHI());	
	}	
}

/**
 * Finish rendering a view, writing the contents to ViewFamily.RenderTarget.
 * @param View - The view to process.
 */
void FSceneRenderer::FinishRenderViewTarget(const FViewInfo* View)
{
	// If the bUseLDRSceneColor flag is set then that means the final post-processing shader has already renderered to
	// the view's render target and that one of the post-processing shaders has performed the gamma correction.
	if( View->bUseLDRSceneColor /*|| 
		// Also skip the final copy to View.RenderTarget if disabled by the view family
		!View->Family->bResolveScene*/ )
	{
		return;
	}

	////if the shader complexity viewmode is enabled, use that to render to the view's rendertarget
	//if (View->Family->ShowFlags & SHOW_ShaderComplexity)
	//{
	//	RenderShaderComplexity(View);
	//	return;
	//}

	// Set the view family's render target/viewport.
	RHISetRenderTarget(GlobalContext,ViewFamily.RenderTarget->GetRenderTargetSurface(),FSurfaceRHIRef());	

	// Deferred the clear until here so the garbage left in the non rendered regions by the post process effects do not show up
	//if( ViewFamily.bDeferClear )
	{
		RHIClear( GlobalContext, TRUE, FLinearColor::Black, FALSE, 0.0f, FALSE, 0 );
//		ViewFamily.bDeferClear = FALSE;
	}

	// turn off culling and blending
	RHISetRasterizerState(GlobalContext,TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
	RHISetBlendState(GlobalContext,TStaticBlendState<>::GetRHI());

	// turn off depth reads/writes
	RHISetDepthState(GlobalContext,TStaticDepthState<FALSE,CF_Always>::GetRHI());

	TShaderMapRef<FGammaCorrectionVertexShader> VertexShader(GetGlobalShaderMap());
	TShaderMapRef<FGammaCorrectionPixelShader> PixelShader(GetGlobalShaderMap());

	SetGlobalBoundShaderState(GlobalContext, PostProcessBoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader, sizeof(FFilterVertex));

	SetPixelShaderValue(
		GlobalContext,
		PixelShader->GetPixelShader(),
		PixelShader->InverseGammaParameter,
		1.0f / ViewFamily.RenderTarget->GetDisplayGamma()
		);
	//SetPixelShaderValue(GlobalContext,PixelShader->GetPixelShader(),PixelShader->ColorScaleParameter,View->ColorScale);
	//SetPixelShaderValue(GlobalContext,PixelShader->GetPixelShader(),PixelShader->OverlayColorParameter,View->OverlayColor);

	SetTextureParameter(
		GlobalContext,
		PixelShader->GetPixelShader(),
		PixelShader->SceneTextureParameter,
		TStaticSamplerState<>::GetRHI(),
		GSceneRenderTargets.GetSceneColorTexture()
		);

	// Draw a quad mapping the blurred pixels in the filter buffer to the scene color buffer.
	DrawDenormalizedQuad(
		GlobalContext,
		View->X,View->Y,
		View->SizeX,View->SizeY,
		View->RenderTargetX,View->RenderTargetY,
		View->RenderTargetSizeX,View->RenderTargetSizeY,
		ViewFamily.RenderTarget->GetSizeX(),ViewFamily.RenderTarget->GetSizeY(),
		GSceneRenderTargets.GetBufferSizeX(),GSceneRenderTargets.GetBufferSizeY()
		);
}


void DrawSilhouette( FCommandContextRHI* Context, const FSceneView& View, const FLinearColor& DrawColor )
{
	static FGlobalBoundShaderStateRHIRef BoundShaderState;

	TShaderMapRef<FGammaCorrectionVertexShader> VertexShader(GetGlobalShaderMap());
	TShaderMapRef<FFillColorPixelShader> PixelShader(GetGlobalShaderMap());

	// turn off depth reads/writes		
	SetGlobalBoundShaderState(Context, BoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader, sizeof(FFilterVertex));

	PixelShader->Set(Context,*PixelShader,DrawColor);

	// Draw a quad mapping the blurred pixels in the filter buffer to the scene color buffer.
	DrawDenormalizedQuad(
		Context,
		View.X,View.Y,
		View.SizeX,View.SizeY,
		View.RenderTargetX,View.RenderTargetY,
		View.RenderTargetSizeX,View.RenderTargetSizeY,		
		View.Family->RenderTarget->GetSizeX(),View.Family->RenderTarget->GetSizeY(),
		GSceneRenderTargets.GetBufferSizeX(),GSceneRenderTargets.GetBufferSizeY()
		);					
}