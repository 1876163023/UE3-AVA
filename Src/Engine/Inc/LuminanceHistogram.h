#ifndef __LUMINANCEHISTOGRAM_H__
#define __LUMINANCEHISTOGRAM_H__

#define N_LUMINANCE_RANGES 64

enum EHistogramBucketState
{
	HBSTATE_Initial,
	HBSTATE_FirstQueryInFlight,
	HBSTATE_QueryInFlight,
	HBSTATE_QueryDone
};

class FLuminanceHistogramBucket
{
public :
	EHistogramBucketState						State;
	FOcclusionQueryRHIRef						OcclusionQuery;
	INT											FrameQueued;
	INT											NumPixels, NumPixelsInRange;
	FLOAT										MinLuminance, MaxLuminance;
	FLOAT										MinLogLuminance, MaxLogLuminance;
	FLOAT										UnbiasedMinLogLuminance, UnbiasedMaxLogLuminance;
	FLOAT										MinX, MinY, MaxX, MaxY;	

	UBOOL ContainsValidData() const;
	/*
	ViewState->ExposureData.InvMinExposure
	ViewState->ExposureData.InvMaxExposure
	*/
	void IssueQuery( FCommandContextRHI* Context, const FViewInfo* View, INT FrameNumber, FLOAT ExposureCenterRegionX, FLOAT ExposureCenterRegionY);

	void Copy( const FLuminanceHistogramBucket& Src );
};

class FLuminanceHistogram 
{
	enum {TONEMAP_TEXTURE_SIZE = 256};

	FLuminanceHistogramBucket Buckets[N_LUMINANCE_RANGES];
	FLOAT LastHistogram[N_LUMINANCE_RANGES];
	FLOAT Histogram[N_LUMINANCE_RANGES];
	FLOAT LastTime;		
	INT CurrentQueryFrame;
	FLOAT LastAverageLogLuminance;
	FLOAT SystemTonemapData[TONEMAP_TEXTURE_SIZE*2];
	FLOAT SystemTonemapScale;

public:
	FLuminanceHistogram();
	~FLuminanceHistogram();

	// FRenderResource interface.	
	void ReleaseDynamicRHI();

	FLOAT GetAverageLuminance();	
	FLOAT GetMinLuminance();
	void Measure( FCommandContextRHI* Context, const FViewInfo* View, FLOAT ExposureCenterRegionX, FLOAT ExposureCenterRegionY );
	void Update( FLOAT CurrentTime, FLOAT TonemapScale, FLOAT BloomStart, UBOOL bDisableScaleRange = FALSE );
	void DisplayHistogram( FCanvas* Canvas, INT& Y );	
	void Graph( FLOAT* Data, INT Resolution );
	void UpdateTonemapTexture( FLOAT BloomStart );
	void Tonemap(FVector& Color)
	{
		FLOAT Luminosity = Color.Y + 0.00001f;
		FLOAT Lw = Luminosity * SystemTonemapScale;
		INT Index = Clamp<INT>((INT)(Lw * TONEMAP_TEXTURE_SIZE), 0, TONEMAP_TEXTURE_SIZE-1);
		FLOAT ScaledLum = SystemTonemapData[Index*2] * Lw + SystemTonemapData[Index*2+1];
		FLOAT ScaleFactor = ScaledLum / Luminosity;
		Color *= ScaleFactor;
	}

	void Copy( const FLuminanceHistogram& Src );
	
	FLOAT ScaleFactor;
	FLOAT WorldLuminaceScale;	
	FLOAT WorldLuminaceLogMax;	
	FLOAT WorldLuminaceMax;
	FLOAT AverageLuminance;
	FLOAT AverageLogLuminance;		
	FLOAT KeyValue;
	FLOAT ScaleBias;
	FLOAT KeyValueScale, KeyValueOffset, MinKeyValue, MaxKeyValue, Shadow, Highlight;
	FTexture2DRHIRef TonemapTexture;

	FLOAT LavgScale, MinScale, MaxScale, K, InvLmaxSquare, Beta;
	FLOAT BloomThreshold;
};

/** Encapsulates the gamma correction pixel shader. */
class FMeasureLuminancePixelShader : public FShader
{
	DECLARE_SHADER_TYPE(FMeasureLuminancePixelShader,Global);

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}

	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment)
	{
	}

	/** Default constructor. */
	FMeasureLuminancePixelShader() {}

public:

	FShaderParameter LinearImageTextureParameter;		
	FShaderParameter MinMaxLuminanceParameter;	

	/** Initialization constructor. */
	FMeasureLuminancePixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FShader(Initializer)
	{
		LinearImageTextureParameter.Bind(Initializer.ParameterMap,TEXT("LinearImageTexture"), TRUE);		
		MinMaxLuminanceParameter.Bind(Initializer.ParameterMap,TEXT("MinMaxLuminance"),TRUE);		
	}

	// FShader interface.
	virtual void Serialize(FArchive& Ar)
	{
		FShader::Serialize(Ar);
		Ar << LinearImageTextureParameter << MinMaxLuminanceParameter;
	}
};

#endif
