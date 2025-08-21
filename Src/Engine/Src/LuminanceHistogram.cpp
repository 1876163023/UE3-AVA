#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "LuminanceHistogram.h"
#include "SceneFilterRendering.h"
#include "ScenePostProcessing.h"
#include "Half.h"

#define EPSILON 1e-4f

IMPLEMENT_SHADER_TYPE(,FMeasureLuminancePixelShader,TEXT("MeasureLuminancePixelShader"),TEXT("Main"),SF_Pixel,VER_MEASURE_LUMINANCE_SHADER_REGISTER_CONFIG,0);

extern TGlobalResource<FFilterVertexDeclaration> GFilterVertexDeclaration;

#define HISTOGRAM_BAR_SIZE 200

static FLOAT AVG( FLOAT x, FLOAT y )
{
	return (x+y) * 0.5f;
}

#define MAX_QUERIES_PER_FRAME	2
#define MIN_MEASURE_VALUE		0.01f
#define MAX_MEASURE_VALUE		16

UBOOL bVisualizeMeasureLuminance = FALSE;

FLOAT FLerp( FLOAT x0, FLOAT x1, FLOAT y0, FLOAT y1, FLOAT u )
{
	return x0 + (x1 - x0) * (u - y0) / (y1 - y0);
}

UBOOL FLuminanceHistogramBucket::ContainsValidData() const
{
	return State == HBSTATE_QueryDone || State == HBSTATE_QueryInFlight;
}	

/** Encapsulates the gamma correction vertex shader. */
class FGammaCorrectionVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FGammaCorrectionVertexShader,Global);

	static UBOOL ShouldCache(EShaderPlatform Platform);
	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment);

	/** Default constructor. */
	FGammaCorrectionVertexShader();

public:

	/** Initialization constructor. */
	FGammaCorrectionVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer);
};

UBOOL Ava_GForceSRGB = FALSE;

void FLuminanceHistogramBucket::IssueQuery( FCommandContextRHI* Context, const FViewInfo* View, INT FrameNumber, FLOAT ExposureCenterRegionX, FLOAT ExposureCenterRegionY )
{
	/// create occlusion query :)
	if( !OcclusionQuery )
		OcclusionQuery = RHICreateOcclusionQuery();		

	if (!OcclusionQuery)
		return;

	TShaderMapRef<FGammaCorrectionVertexShader> VertexShader(GetGlobalShaderMap());
	TShaderMapRef<FMeasureLuminancePixelShader> PixelShader(GetGlobalShaderMap());					

	static FBoundShaderStateRHIRef MeasureLuminanceBoundShaderState;

	if (!IsValidRef(MeasureLuminanceBoundShaderState))
	{
		DWORD Strides[MaxVertexElementCount];
		appMemzero(Strides, sizeof(Strides));
		Strides[0] = sizeof(FFilterVertex);

		MeasureLuminanceBoundShaderState = RHICreateBoundShaderState(GFilterVertexDeclaration.VertexDeclarationRHI, Strides, VertexShader->GetVertexShader(), PixelShader->GetPixelShader());
	}

	// Set the gamma correction shaders
	RHISetBoundShaderState(Context,MeasureLuminanceBoundShaderState);
	SetPixelShaderValue(
		Context,
		PixelShader->GetPixelShader(),
		PixelShader->MinMaxLuminanceParameter,
		FLinearColor( MinLuminance, (MaxLuminance >= MAX_MEASURE_VALUE ? 10000.0f : MaxLuminance), 0, 0 )
		);	

	//<@ 2007. 7. 26 changmin
	// hdr integer path 일 경우, scene texture 는 sRGB에 있습니다. 그래서 sRGBRead를 켜야 합니다.
	Ava_GForceSRGB = GIsLowEndHW ? TRUE : FALSE;
	SetTextureParameter(
		Context,
		PixelShader->GetPixelShader(),
		PixelShader->LinearImageTextureParameter,
		TStaticSamplerState<>::GetRHI(),
		GSceneRenderTargets.GetSceneColorTexture()
		);
	Ava_GForceSRGB = FALSE;
	//>@ changmin

	FLOAT WidthScale, HeightScale;

	WidthScale = 0.5f * ( 1.0f - ExposureCenterRegionX );
	HeightScale = 0.5f * ( 1.0f - ExposureCenterRegionY );	

	FLOAT SkipX = (MaxX - MinX) * WidthScale;
	FLOAT SkipY = (MaxY - MinY) * HeightScale;		

	INT FinalMinX = (MinX + SkipX) * View->RenderTargetSizeX, 
		FinalMinY = (MinY + SkipY) * View->RenderTargetSizeY, 
		FinalMaxX = (MaxX - SkipX) * View->RenderTargetSizeX, 
		FinalMaxY = (MaxY - SkipY) * View->RenderTargetSizeY;

	NumPixels = (FinalMaxX - FinalMinX) * (FinalMaxY - FinalMinY);

	UBOOL bUsingStencil = GIsATI;

	// <@ ava specific ; testing for ati card
	// draw pass pixel for debugging
	if( !bVisualizeMeasureLuminance )
	{
		RHISetColorWriteEnable(Context,FALSE);
	}
	
	if (bUsingStencil)
	{
		RHISetStencilState(Context,TStaticStencilState<TRUE,CF_Always,SO_Keep,SO_Keep,SO_Replace,FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,0xff,0xff,1>::GetRHI());
	}
	else
	{
		if( !bVisualizeMeasureLuminance )
		{
			RHIBeginOcclusionQuery(Context,OcclusionQuery);
		}
	}		

	// Set the gamma correction shaders
	RHISetBoundShaderState(Context,MeasureLuminanceBoundShaderState);

	// Draw a quad mapping the blurred pixels in the filter buffer to the scene color buffer.
	DrawDenormalizedQuad(
		Context,
		View->X + FinalMinX, View->Y + FinalMinY,
		FinalMaxX - FinalMinX, FinalMaxY - FinalMinY,
		View->RenderTargetX + FinalMinX, View->RenderTargetY + FinalMinY,
		FinalMaxX - FinalMinX, FinalMaxY - FinalMinY,
		View->Family->RenderTarget->GetSizeX(),View->Family->RenderTarget->GetSizeY(),
		GSceneRenderTargets.GetBufferSizeX(),GSceneRenderTargets.GetBufferSizeY()
		);

	if (bUsingStencil)
	{
		static FBoundShaderStateRHIRef CountingStencilBitBoundShaderState;

		TShaderMapRef<FNULLPixelShader> PixelShader(GetGlobalShaderMap());					

		if (!IsValidRef(CountingStencilBitBoundShaderState))
		{
			DWORD Strides[MaxVertexElementCount];
			appMemzero(Strides, sizeof(Strides));
			Strides[0] = sizeof(FFilterVertex);

			CountingStencilBitBoundShaderState = RHICreateBoundShaderState(GFilterVertexDeclaration.VertexDeclarationRHI, Strides, VertexShader->GetVertexShader(), PixelShader->GetPixelShader());
		}

		// Set the gamma correction shaders
		RHISetBoundShaderState(Context,CountingStencilBitBoundShaderState);

		RHIBeginOcclusionQuery(Context,OcclusionQuery);
		RHISetStencilState(Context,TStaticStencilState<TRUE,CF_Equal,SO_Keep,SO_Keep,SO_Decrement,FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,0xff,0xff,1>::GetRHI());

		// Draw a quad mapping the blurred pixels in the filter buffer to the scene color buffer.
		DrawDenormalizedQuad(
			Context,
			View->X + FinalMinX, View->Y + FinalMinY,
			FinalMaxX - FinalMinX, FinalMaxY - FinalMinY,			
			View->RenderTargetX + FinalMinX, View->RenderTargetY + FinalMinY,
			FinalMaxX - FinalMinX, FinalMaxY - FinalMinY,
			View->Family->RenderTarget->GetSizeX(),View->Family->RenderTarget->GetSizeY(),
			GSceneRenderTargets.GetBufferSizeX(),GSceneRenderTargets.GetBufferSizeY()
			);
	}


	if( !bVisualizeMeasureLuminance )
	{
		RHIEndOcclusionQuery(Context,OcclusionQuery);
		RHISetColorWriteEnable(Context,TRUE);
	}

	if (bUsingStencil)
	{
		RHISetStencilState(Context,TStaticStencilState<>::GetRHI());
	}

	if( State == HBSTATE_Initial )
		State = HBSTATE_FirstQueryInFlight;
	else
		State = HBSTATE_QueryInFlight;

	FrameQueued = FrameNumber;		
}	

void FLuminanceHistogram::ReleaseDynamicRHI()
{
	for (INT i=0; i<ARRAY_COUNT(Buckets); ++i)
	{
		Buckets[i].State = HBSTATE_Initial;
		Buckets[i].OcclusionQuery = NULL;
	}

	TonemapTexture = NULL;
}

FLOAT FLuminanceHistogram::GetMinLuminance()
{
	// find minimum luminance in screen
	FLOAT MinLum = 0.0f;
	for( INT i = 0; i < ARRAY_COUNT( Buckets ) - 1; ++i )
	{
		if( !Buckets[i].ContainsValidData() )
		{
			return MinLum;
		}
		if( Buckets[i].NumPixelsInRange > 80 )
		{
			MinLum = AVG( Buckets[i].MinLuminance, Buckets[i].MaxLuminance );
			return MinLum;
		}
	}
	return MinLum;
}

static INT GNumTotalPixels = 0;

FLOAT FLuminanceHistogram::GetAverageLuminance()
{
	GNumTotalPixels = 0;

	FLOAT Total = 0;
	INT NumTotalPixels = 0;
	FLOAT ScaleValue = 1.0f;

	if (Buckets[N_LUMINANCE_RANGES-1].ContainsValidData())
	{
		GNumTotalPixels = Buckets[N_LUMINANCE_RANGES-1].NumPixelsInRange;
	}
	else
		return AVG( Buckets[0].MinLuminance,Buckets[0].MaxLuminance );

	for(INT i=0;i<ARRAY_COUNT( Buckets )-1;i++)
	{
		if( Buckets[i].ContainsValidData() )
		{
			Total += ScaleValue * Buckets[i].NumPixelsInRange * AVG( Buckets[i].MinLuminance,Buckets[i].MaxLuminance );
			NumTotalPixels += Buckets[i].NumPixelsInRange;
		}
		else
			return AVG( Buckets[0].MinLuminance,Buckets[0].MaxLuminance );						 // always return 0.01 until we've queried a whole frame
	}

	if (NumTotalPixels > 0)
	{
		return Total*(1.0/NumTotalPixels);
	}
	else
	{
		return AVG( Buckets[0].MinLuminance,Buckets[0].MaxLuminance );
	}
}



FORCEINLINE FLOAT appLog10( FLOAT x )
{
	static const FLOAT log10 = appLoge(10.0f);

	return appLoge( x ) / log10;
}

static const FLOAT DisplayRange = 256.0f;

void FLuminanceHistogram::Measure( FCommandContextRHI* Context, const FViewInfo* View, FLOAT ExposureCenterRegionX, FLOAT ExposureCenterRegionY )
{
	// find which histogram entries should have something done this frame
	INT NumQueriesIssuedThisFrame=0;
	CurrentQueryFrame++;

	for(INT i=0;i<ARRAY_COUNT( Buckets );++i)
	{
		switch( Buckets[i].State )
		{
		case HBSTATE_Initial:
			if( NumQueriesIssuedThisFrame < MAX_QUERIES_PER_FRAME )
			{
				Buckets[i].IssueQuery(Context,View,CurrentQueryFrame,ExposureCenterRegionX,ExposureCenterRegionY);
				NumQueriesIssuedThisFrame++;
			}
			break;

		case HBSTATE_FirstQueryInFlight:
		case HBSTATE_QueryInFlight:
			if( CurrentQueryFrame>Buckets[i].FrameQueued + 2 )
			{
				DWORD NumPixels = 0;
				if (RHIGetOcclusionQueryResult( Buckets[i].OcclusionQuery, NumPixels, FALSE ))					
				{
					Buckets[i].NumPixelsInRange = NumPixels;						
					Buckets[i].State = HBSTATE_QueryDone;
				}
			}
			break;
		}
	}
	// now, issue queries for the oldest finished queries we have
	while( NumQueriesIssuedThisFrame < MAX_QUERIES_PER_FRAME )
	{
		INT OldestSoFar=-1;
		for( INT i=0; i<ARRAY_COUNT( Buckets ); ++i)
		{
			if (Buckets[i].State == HBSTATE_QueryDone
				&&	(OldestSoFar < 0 || Buckets[i].FrameQueued < Buckets[OldestSoFar].FrameQueued) )
			{
				OldestSoFar = i;
			}
		}

		// nothing to do
		if (OldestSoFar<0)								
			break;

		Buckets[OldestSoFar].IssueQuery(Context,View,CurrentQueryFrame,ExposureCenterRegionX,ExposureCenterRegionY);
		NumQueriesIssuedThisFrame++;
	}
}

void FLuminanceHistogram::Update( FLOAT CurrentTime, FLOAT TonemapScale, FLOAT BloomStart, UBOOL bDisableScaleRange )
{	
	static const FLOAT DisplayLogRange = appLog10( DisplayRange );
	static const FLOAT Half = appLog10( 1 / 2.0f );

	/* calculate lerp term */	
	FLOAT LerpTerm = 1.0f;		
	FLOAT LerpTermFast = 1.0f;		
	if (LastTime >= 0)
	{
		static const FLOAT AdaptionTime = 0.4f;	
		LerpTerm = 1 - appExp( - Max( CurrentTime - LastTime, 0.0f ) / AdaptionTime );

		static const FLOAT AdaptionTimeFast = 0.1f;	
		LerpTermFast = 1 - appExp( - Max( CurrentTime - LastTime, 0.0f ) / AdaptionTimeFast );
	}
	LastTime = CurrentTime;		

	// debugf( NAME_Log, TEXT("CurrentTime:%.2f,LerpTerm:%.2f/%.2f"), CurrentTime, LerpTerm, LerpTermFast);

	for (INT i=0; i<N_LUMINANCE_RANGES-1; ++i)
	{
		FLuminanceHistogramBucket& e = Buckets[i];

		e.MinLogLuminance = e.UnbiasedMinLogLuminance + 0;
		e.MaxLogLuminance = e.UnbiasedMaxLogLuminance + 0;

		// use a logarithmic ramp for high range in the low range
		e.MinLuminance = appPow(10.0f, e.MinLogLuminance );
		e.MaxLuminance = appPow(10.0f, e.MaxLogLuminance );
	}
	
	AverageLuminance = GetAverageLuminance();

	AverageLuminance *= TonemapScale;

	AverageLogLuminance = appLog10( AverageLuminance );

	// Error handling
	if (!(AverageLogLuminance < 0) && !(AverageLogLuminance > 0))
	{
		AverageLogLuminance = LastAverageLogLuminance;
	}

	if (AverageLogLuminance > LastAverageLogLuminance)
	{
		AverageLogLuminance = (AverageLogLuminance - LastAverageLogLuminance) * LerpTermFast + LastAverageLogLuminance;	
	}
	else
	{
		AverageLogLuminance = (AverageLogLuminance - LastAverageLogLuminance) * LerpTerm + LastAverageLogLuminance;	
	}
	
	LastAverageLogLuminance = AverageLogLuminance;	

	AverageLuminance = appPow( 10.0f, AverageLogLuminance );

	
	//<@ 2007. 7. 25 changmin
	// key값이 averagelogluminance값에 의해 조절되다가, averagelogluminance값이 너무 작아지면, 갑자기 큰 값으로 mapping된다.
	// 이 현상은 night vision을 켜고, 어두운 곳을 볼때 발생하며, scene 이 더 어두워지게 된다.
	// 이를 조절하기 위해 변경.
	//KeyValue = Clamp( (1.03f - 2.0f / ( 2.0f + AverageLogLuminance + LavgScale )) * KeyValueScale + KeyValueOffset, MinKeyValue, MaxKeyValue ) ;
	FLOAT KeyControlValue = ( 2.0f + AverageLogLuminance + LavgScale );
	KeyValue = (KeyControlValue <= 0.0f) ? MinKeyValue : Clamp( (1.03f - 2.0f / KeyControlValue ) * KeyValueScale + KeyValueOffset, MinKeyValue, MaxKeyValue );
	//>@ changmin

	GNumTotalPixels = Max( 1, GNumTotalPixels );

	if( !bDisableScaleRange )
		ScaleFactor = Clamp( KeyValue / AverageLuminance * ScaleBias, MinScale, MaxScale );	
	else
		ScaleFactor = Clamp( KeyValue / AverageLuminance * ScaleBias, 0.0f, 1000.0f );	

	FLOAT GrayLogLuminance = appLog10( KeyValue / ScaleFactor );

	WorldLuminaceScale = 1.0f;

#define MAXOVERBRIGHT 4.0f

	WorldLuminaceMax = MAXOVERBRIGHT / (ScaleFactor + EPSILON);	
	WorldLuminaceLogMax = appLog10( WorldLuminaceMax );

	FLOAT WhitePoint = 1.0f;	
	for(INT l=0; l<N_LUMINANCE_RANGES-1; ++l)
	{
		FLuminanceHistogramBucket&e = Buckets[l];		

		if (e.MinLogLuminance <= WorldLuminaceLogMax && e.MaxLogLuminance >= WorldLuminaceLogMax )
		{
			FLOAT a = e.MinLogLuminance - WorldLuminaceLogMax;
			FLOAT b = e.MaxLogLuminance - WorldLuminaceLogMax;

			FLOAT mu = -a / (b-a+EPSILON);

			WhitePoint = (l + mu);
			break;
		}
	}

	INT GrayPoint_Int = 0;
	FLOAT GrayPoint_Frac = 0;
	FLOAT GrayPoint = 1.0f;	
	for(INT l=0; l<N_LUMINANCE_RANGES-1; ++l)
	{
		FLuminanceHistogramBucket&e = Buckets[l];		

		if (e.MinLogLuminance <= GrayLogLuminance && e.MaxLogLuminance >= GrayLogLuminance)
		{
			FLOAT a = e.MinLogLuminance - GrayLogLuminance;
			FLOAT b = e.MaxLogLuminance - GrayLogLuminance;

			FLOAT mu = -a / (b-a+EPSILON);

			GrayPoint_Int = l;
			GrayPoint_Frac = mu;
			GrayPoint = (l + mu);
			break;
		}
	}

	/*

	gray point까지는 log-log graph상에서 기울기 1을 유지하다가, 그 이후에서 hist-equalization compress를 하도록.
	이렇게 하려면 accumulate 결과를 다시 재정비해야 함. :)

	결과값은 [-DISP_LOG_RANGE ~ 0]에 map

	Gray point는 -HALF에 map

	*/

	// Gray를 0으로 놓음; :) -inf ~ +HALF에 map되어 있음.
	for(INT l=0; l</*appFloor(GrayPoint)*/N_LUMINANCE_RANGES-1; ++l)
	{
		FLuminanceHistogramBucket&e = Buckets[l];		

		Histogram[l] = (e.MinLogLuminance - AverageLogLuminance);
		Histogram[l+1] = (e.MaxLogLuminance - AverageLogLuminance);
	}	

	static const FLOAT tau = 0.05f;	

	INT NumBucketPixels[N_LUMINANCE_RANGES];
	for(INT l=0;l<N_LUMINANCE_RANGES-1;l++)
	{		
		FLuminanceHistogramBucket&e = Buckets[l];

		if (e.ContainsValidData())
		{
			NumBucketPixels[l] = e.NumPixelsInRange;
		}
		else
		{
			NumBucketPixels[l] = 0;
		}
	}

	INT Trimmings = 0;
	INT NumPixels = GNumTotalPixels;
	INT Bound = NumPixels * 0.025f;
	do 
	{
		Trimmings = 0;

		INT Tdb = NumPixels * (WorldLuminaceLogMax - Buckets[0].MinLogLuminance) / N_LUMINANCE_RANGES / DisplayLogRange;

		for(INT l=0;l<N_LUMINANCE_RANGES-1;l++)
		{		
			FLuminanceHistogramBucket&e = Buckets[l];

			if (NumBucketPixels[l] > Tdb)
			{
				Trimmings += NumBucketPixels[l] - Tdb;
				NumBucketPixels[l] = Tdb;
			}
		}

		NumPixels -= Trimmings;

	} while(Trimmings > Bound);

	FLOAT Accumulated = 0.0f;
	for(INT l=0;l<N_LUMINANCE_RANGES-1;l++)
	{		
		FLuminanceHistogramBucket&e = Buckets[l];

		Histogram[l] = Accumulated;

		FLOAT Contribution = 0.0f;

		Contribution = (FLOAT)NumBucketPixels[l] / NumPixels;		

		Contribution /= 1.0f / (N_LUMINANCE_RANGES-1);

		if (Beta > 1)
			Contribution = Clamp( Contribution, 1.0f, Beta );
		else
			Contribution = Clamp( Contribution, Beta, 1.0f );

		Contribution *= (e.MaxLogLuminance - e.MinLogLuminance);				

		Accumulated += Contribution;		
	}	
	Histogram[N_LUMINANCE_RANGES-1] = Accumulated;		
	

	FLOAT A = Histogram[GrayPoint_Int];
	FLOAT B = Histogram[Min(GrayPoint_Int+1,N_LUMINANCE_RANGES-1)];
	FLOAT Offset = (B-A) * GrayPoint_Frac + A - Half;
	
	for(INT l=0; l<N_LUMINANCE_RANGES; ++l)
	{
		Histogram[l] -= Offset;
	}

	/* LERP */
	for (INT l=0; l<N_LUMINANCE_RANGES; ++l)
	{
		// error handling
		if (!appIsFinite(Histogram[l]))
		{
			Histogram[l] = LastHistogram[l];
			continue;
		}

		// error handling
		if (!appIsFinite(LastHistogram[l]))
		{
			LastHistogram[l] = Histogram[l];			
		}

		if (Histogram[l] > LastHistogram[l])
		{
			Histogram[l] = (Histogram[l] - LastHistogram[l]) * LerpTermFast + LastHistogram[l];
		}
		else
		{
			Histogram[l] = (Histogram[l] - LastHistogram[l]) * LerpTerm + LastHistogram[l];
		}
		
		LastHistogram[l] = Histogram[l];
	}

	/* update texture */
	/* prepare tonemap texture */
	UpdateTonemapTexture( BloomStart );
}

void FLuminanceHistogram::UpdateTonemapTexture( FLOAT BloomStart )
{
	if (!TonemapTexture)
	{
		TonemapTexture = RHICreateTexture2D( TONEMAP_TEXTURE_SIZE, 1, PF_G16R16F, 1, 0 );
	}

	UINT DestStride;
	WORD* Data = (WORD*)RHILockTexture2D( TonemapTexture, 0, TRUE, DestStride );

	FLOAT* SystemData = SystemTonemapData;

	if (Data)
	{
		FLOAT Raw[TONEMAP_TEXTURE_SIZE];

		Graph( Raw, TONEMAP_TEXTURE_SIZE );				

		Raw[0] = 0;

		/*GrayPoint = TONEMAP_TEXTURE_SIZE / MAXOVERBRIGHT * KeyValue;
		GrayPoint_Int = appFloor( GrayPoint );
		GrayPoint_Frac = GrayPoint - GrayPoint_Int;

		FLOAT a = Raw[GrayPoint_Int];
		FLOAT b = Raw[GrayPoint_Int+1];

		FLOAT x = (b-a) * GrayPoint_Frac + a;

		for (INT i=0; i<TONEMAP_TEXTURE_SIZE; ++i)
		{			
		Raw[i] *= 0.5f/x;
		}*/

		const FLOAT LumDelta = 1.0f / (TONEMAP_TEXTURE_SIZE);
		const FLOAT TonemapScale = WorldLuminaceScale / (WorldLuminaceMax + 1e-6f);

		SystemTonemapScale = TonemapScale;

		for (INT i=0; i<TONEMAP_TEXTURE_SIZE-1; ++i)
		{	
			const FLOAT x = Raw[i];
			const FLOAT y = Raw[i+1];
			FLOAT a = (y-x) / LumDelta;
			FLOAT b = x - a * i * LumDelta;

			if (x<BloomStart && y>=BloomStart)
			{
				FLOAT alpha = (BloomStart-x);
				FLOAT beta = y-BloomStart;

				FLOAT u = alpha / (alpha + beta);

				BloomThreshold = (i + u) / ((TONEMAP_TEXTURE_SIZE-1) * TonemapScale);
			}

			Data[i*2+0] = half_from_float( *((UINT*)&a) );
			Data[i*2+1] = half_from_float( *((UINT*)&b) );

			SystemData[i*2+0] = a;
			SystemData[i*2+1] = b;
		}

		Data[(TONEMAP_TEXTURE_SIZE-1)*2+0] = Data[(TONEMAP_TEXTURE_SIZE-2)*2+0];
		Data[(TONEMAP_TEXTURE_SIZE-1)*2+1] = Data[(TONEMAP_TEXTURE_SIZE-2)*2+1];

		SystemData[(TONEMAP_TEXTURE_SIZE-1)*2+0] = SystemData[(TONEMAP_TEXTURE_SIZE-2)*2+0];
		SystemData[(TONEMAP_TEXTURE_SIZE-1)*2+1] = SystemData[(TONEMAP_TEXTURE_SIZE-2)*2+1];


		RHIUnlockTexture2D( TonemapTexture, 0 );
	}
}

void FLuminanceHistogram::Graph( FLOAT* Data, INT Resolution )
{
	static const FLOAT DisplayLogRange = appLog10( DisplayRange );

	INT l=0;
	for (INT x=0; x<Resolution; ++x)
	{
		FLOAT Luminance = (WorldLuminaceMax) * x / (Resolution-1);
		FLOAT LogLuminance = appLog10(Max(Luminance, EPSILON));		

		for(; l<N_LUMINANCE_RANGES-1;l++)
		{
			FLuminanceHistogramBucket&e = Buckets[l];

			// 이 range?
			if (e.MaxLogLuminance > LogLuminance)
			{
				break;
			}
		}

		const FLuminanceHistogramBucket& 
			a = Buckets[Max(l-1,0)], 
			b = Buckets[l], 
			c = Buckets[Min(l+1,N_LUMINANCE_RANGES-2)];

		const FLOAT mu = (LogLuminance - b.MinLogLuminance) / (b.MaxLogLuminance - b.MinLogLuminance);
		const FLOAT mu2 = mu * mu;
		const FLOAT 
			y0 = Histogram[Max(l-1,0)],
			y1 = Histogram[l],
			y2 = Histogram[Min(l+1,N_LUMINANCE_RANGES-1)],
			y3 = Histogram[Min(l+2,N_LUMINANCE_RANGES-1)];

		const FLOAT a0 = y3 - y2 - y0 + y1;
		const FLOAT a1 = y0 - y1 - a0;
		const FLOAT a2 = y2 - y0;
		const FLOAT a3 = y1;

		// cubic
		// Data[x] = a0 * mu * mu2 + a1 * mu2 + a2 * mu + a3;

		// linear
		Data[x] = (y2 - y1) * mu + a3;

		Data[x] = ( appPow( 10.0f, DisplayLogRange + Data[x] ) - 1.0f ) / ( DisplayRange );				
	}	

	FLOAT d2 = (Data[Resolution-2] - Data[Resolution-3]);
	FLOAT d1 = (Data[Resolution-1] - Data[Resolution-2]);
	FLOAT dd = d2 - d1;
	FLOAT d = d1 * 0.5f + dd * 0.5f;
	Data[Resolution-1] = d + Data[Resolution-2];

	FLOAT Min = Data[0], Max = Data[Resolution/(INT)MAXOVERBRIGHT-1];

	// make linear! :$
	/*static const FLOAT B = 0.7f;

	for (INT x=0; x<Resolution; ++x)
	{
		FLOAT HistEqual = (Data[x] - Min) / (Max - Min);
		FLOAT Linear = (FLOAT)x / (Resolution/(INT)MAXOVERBRIGHT-1);		

		Data[x] = (HistEqual - Linear) * B + Linear;
	}	*/	

	/*static const FLOAT Shadows = 0.005f;
	static const FLOAT Highlights = 0.8f;
	static const FLOAT MidTones = 1.3f;

	for (INT x=0; x<Resolution; ++x)
	{
		FLOAT L = Data[x];		
		L = appPow( Clamp( L - Shadows, 0.0f, 1.0f ) / Highlights, MidTones );
		Data[x] = L;
	}*/
	
	for (INT x=0; x<Resolution; ++x)
	{
		FLOAT L = Data[x];		
		L = L * ( 1 + L * InvLmaxSquare ) / ( K + L );
		L = L * ( 1 + L * InvLmaxSquare * 0.01f ) / ( K + L );

		L = (L - Shadow) * Highlight;

		Data[x] = L;
	}
}

void FLuminanceHistogram::DisplayHistogram( FCanvas* Canvas, INT& Y )
{		
	FLOAT NeutralLuminance = ScaleFactor;
	INT XPos=10;

	for(INT l=0;l<N_LUMINANCE_RANGES-1;l++)
	{
		INT NumPixels=0;
		FLuminanceHistogramBucket&e = Buckets[l];

		if (e.ContainsValidData())
			NumPixels += e.NumPixelsInRange;

		INT Width = 200 * (e.MaxLuminance-e.MinLuminance);

		UBOOL bInRange = (NeutralLuminance >= e.MinLuminance && NeutralLuminance < e.MaxLuminance);

		if (NumPixels)
		{
			INT Height=Clamp(HISTOGRAM_BAR_SIZE,1,HISTOGRAM_BAR_SIZE * NumPixels / Max( 1, GNumTotalPixels ) );			

			DrawTile( 
				Canvas, 
				XPos, HISTOGRAM_BAR_SIZE-Height + Y, Width, Height, 
				0, 0, 1, 1, 
				bInRange ? FLinearColor( 1, 0, 0, 1 ) : FLinearColor::White	);
		}		

		XPos += Width + 2;
	}		

	Y += HISTOGRAM_BAR_SIZE + 10;

	DrawString( Canvas, 10, Y, *FString::Printf( TEXT("Average luminance : %.4f"), GetAverageLuminance() ), GEngine->TinyFont, FColor(255,255,255) );	

	Y += 20;
	
	XPos=10;

	FLOAT Data[512];
	Graph( Data, 512 );

	for(INT l=0;l<512;l++)
	{		
		INT Height = Clamp( (INT)(HISTOGRAM_BAR_SIZE * Data[l]), 1, HISTOGRAM_BAR_SIZE );
		DrawTile( 
			Canvas, 
			XPos + l, HISTOGRAM_BAR_SIZE- Height + Y, 1, Height, 
			0, 0, 1, 1, 
			FLinearColor::White	);
	}		

	Y += HISTOGRAM_BAR_SIZE + 10;
}

FLuminanceHistogram::FLuminanceHistogram()
: LastTime(-1)
{
	BloomThreshold = 1.0f;

	KeyValueScale = 1.0f;
	KeyValueOffset = 0.0f;
	MinKeyValue = 0.18f;
	MaxKeyValue = 0.90f;

	Shadow = 0.0f;
	Highlight = 1.0f;

	MinScale = 0.25f;
	MaxScale = 4.0f;

	InvLmaxSquare = 1.0f / Square( 0.5f );
	K = 3.0f;
	LavgScale = 1.0f;

	CurrentQueryFrame = 0;

	ScaleBias = 1.0f;

	for( INT Bucket=0;Bucket<N_LUMINANCE_RANGES;Bucket++ )
	{
		INT idx=Bucket;
		FLuminanceHistogramBucket &e=Buckets[idx];
		e.State=HBSTATE_Initial;
		e.MinX=0;
		e.MaxX=1;
		e.MinY=0;
		e.MaxY=1;
		if (Bucket!=N_LUMINANCE_RANGES-1)				// last Bucket is special
		{
			//e.MaxLuminance=-.01f+appExp(FLerp(appLoge(.01f),appLoge(.01f+MAX_MEASURE_VALUE),0,N_LUMINANCE_RANGES-1,Bucket+1));

			e.UnbiasedMinLogLuminance = FLerp( appLog10( MIN_MEASURE_VALUE ), appLog10( MIN_MEASURE_VALUE + MAX_MEASURE_VALUE ), 0, N_LUMINANCE_RANGES-1, Bucket );
			e.UnbiasedMaxLogLuminance = FLerp( appLog10( MIN_MEASURE_VALUE ), appLog10( MIN_MEASURE_VALUE + MAX_MEASURE_VALUE ), 0, N_LUMINANCE_RANGES-1, Bucket+1 );			

			e.MinLogLuminance = e.UnbiasedMinLogLuminance;
			e.MaxLogLuminance = e.UnbiasedMaxLogLuminance;

			// use a logarithmic ramp for high range in the low range
			e.MinLuminance = appPow(10.0f, e.MinLogLuminance );
			e.MaxLuminance = appPow(10.0f, e.MaxLogLuminance );
		}
		else
		{
			// the last Bucket is used as a test to determine the return range for occlusion
			// queries to use as a scale factor. some boards (nvidia) have their occlusion
			// query return values larger when using AA.
			e.MinLuminance = -1;
			e.MaxLuminance = 100000.0f;

			e.MinLogLuminance = appLog10( EPSILON );
			e.MaxLogLuminance = appLog10( e.MaxLuminance );
		}
		
		// 구간 테스트...
		//debugf(NAME_Log, TEXT("Bucket[%d] : (%f,%f : deta %f)"), Bucket, e.MinLuminance, e.MaxLuminance, e.MaxLuminance - e.MinLuminance );
	}
}

FLuminanceHistogram::~FLuminanceHistogram()
{	
}

void FLuminanceHistogram::Copy( const FLuminanceHistogram& Src )
{
	for (INT i=0; i<N_LUMINANCE_RANGES; ++i)
	{
		Buckets[i].Copy( Src.Buckets[i] );
		LastHistogram[i] = Src.LastHistogram[i];
		Histogram[i] = Src.Histogram[i];		
	}

#define CopySimple(x) x = Src.x;

	CopySimple(LastTime);		
	CopySimple(CurrentQueryFrame);
	CopySimple(LastAverageLogLuminance);
	
	CopySimple(ScaleFactor);
	CopySimple(WorldLuminaceScale);	
	CopySimple(WorldLuminaceLogMax);	
	CopySimple(WorldLuminaceMax);
	CopySimple(AverageLuminance);
	CopySimple(AverageLogLuminance);		
	CopySimple(KeyValue);
	CopySimple(ScaleBias);
	CopySimple(KeyValueScale); CopySimple(KeyValueOffset); CopySimple(MinKeyValue); CopySimple(MaxKeyValue); CopySimple(Shadow); CopySimple(Highlight);	

	CopySimple(LavgScale); CopySimple(MinScale); CopySimple(MaxScale); CopySimple(K); CopySimple(InvLmaxSquare); CopySimple(Beta);
}

void FLuminanceHistogramBucket::Copy( const FLuminanceHistogramBucket& Src )
{	
	if (State == HBSTATE_Initial && (Src.State == HBSTATE_QueryInFlight || Src.State == HBSTATE_QueryDone))
	{
		State = HBSTATE_QueryDone;
	}
	
	CopySimple(NumPixels); CopySimple(NumPixelsInRange);
	CopySimple(MinLuminance); CopySimple(MaxLuminance);
	CopySimple(MinLogLuminance); CopySimple(MaxLogLuminance);
	CopySimple(UnbiasedMinLogLuminance); CopySimple(UnbiasedMaxLogLuminance);
	CopySimple(MinX); CopySimple(MinY); CopySimple(MaxX); CopySimple(MaxY);	
}

#undef CopySimple