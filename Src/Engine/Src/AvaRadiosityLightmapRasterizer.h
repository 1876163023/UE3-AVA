#ifndef _AVARADIOSITYLIGHTMAPRASTERIZER_H_
#define _AVARADIOSITYLIGHTMAPRASTERIZER_H_

//!{ 2006-04-14	허 창 민
/**
* The interpolated vectors for the static-mesh lighting rasterizer.
*/
struct AvaRadiosityLightMapRasterInterpolant
{
	FVector2D			RadiosityLightmapCoord;


	AvaRadiosityLightMapRasterInterpolant(const FVector2D& InRadiosityLightmapCoord):
	RadiosityLightmapCoord(InRadiosityLightmapCoord)
	{}

	friend AvaRadiosityLightMapRasterInterpolant operator+(const AvaRadiosityLightMapRasterInterpolant& A,const AvaRadiosityLightMapRasterInterpolant& B)
	{
		return AvaRadiosityLightMapRasterInterpolant( A.RadiosityLightmapCoord + B.RadiosityLightmapCoord );
	}

	friend AvaRadiosityLightMapRasterInterpolant operator-(const AvaRadiosityLightMapRasterInterpolant& A,const AvaRadiosityLightMapRasterInterpolant& B)
	{
		return AvaRadiosityLightMapRasterInterpolant( A.RadiosityLightmapCoord - B.RadiosityLightmapCoord );
	}

	friend AvaRadiosityLightMapRasterInterpolant operator*(const AvaRadiosityLightMapRasterInterpolant& A,FLOAT B)
	{
		return AvaRadiosityLightMapRasterInterpolant( A.RadiosityLightmapCoord * B );
	}

	friend AvaRadiosityLightMapRasterInterpolant operator/(const AvaRadiosityLightMapRasterInterpolant& A,FLOAT B)
	{
		FLOAT InvB = 1.0f / B;
		return AvaRadiosityLightMapRasterInterpolant( A.RadiosityLightmapCoord * InvB );
	}
};

//
//	FStaticMeshLightingRasterizer - Rasterizes static mesh lighting into a lightmap.
//

// 매우 주의 할 것.. 이것을 켜면 속도가 10배 느려진다.
//#define DEBUG_AVA_RASTER

class AvaRadiosityLightingRasterPolicy
{
public:
	typedef AvaRadiosityLightMapRasterInterpolant InterpolantType;

	struct FLightSample
	{
		UINT Coverage[NUM_LIGHTMAP_COEFFICIENTS];
		FLinearColor Color[NUM_LIGHTMAP_COEFFICIENTS];
		FLinearColor NormalColor;

		FLightSample()
		{
			for( INT i = 0; i < NUM_LIGHTMAP_COEFFICIENTS; ++i)
			{
				Coverage[i] = 0;
				Color[i] = FLinearColor( 0.0f, 0.0f, 0.0f );
			}

			NormalColor = FLinearColor( 0.0f, 0.0f, 0.0f );
		}
	};

	unsigned char*			RadiosityLightmap;
	UINT					RadiosityLightmapSizeX;
	UINT					RadiosityLightmapSizeY;
	UINT					SizeX;
	UINT					SizeY;

	TArray<FLightSample>	LightSamples;

	AvaRadiosityLightingRasterPolicy( UINT InSizeX, UINT InSizeY)
		: RadiosityLightmap( NULL ),
		RadiosityLightmapSizeX( 0 ),
		RadiosityLightmapSizeY( 0 ),
		SizeX( InSizeX ),
		SizeY( InSizeY )
	{
		LightSamples.AddZeroed(SizeX * SizeY);
	}

	void SetRadiosityLightmap( unsigned char* InRadiosityLightmap, UINT InSizeX, UINT InSizeY )
	{
		RadiosityLightmap = InRadiosityLightmap;
		RadiosityLightmapSizeX = InSizeX;
		RadiosityLightmapSizeY = InSizeY;
	}

	UBOOL GetBilinearRadiositySample(const FVector2D& Coordinate, UINT iBump, FLinearColor* OutColor ) const
	{
#ifdef DEBUG_AVA_RASTER
		if( iBump == 0 )
		{
			debugf( TEXT("radiosity lightmap texcoord : %f, %f"), Coordinate.X, Coordinate.Y );
		}
#endif

		FLOAT FloatX = Coordinate.X - 0.5f;
		FLOAT FloatY = Coordinate.Y - 0.5f;

		INT IntX = appTrunc(FloatX);
		INT IntY = appTrunc(FloatY);

		FLOAT FracX = FloatX - IntX;
		FLOAT FracY = FloatY - IntY;

		if( IntX < (INT)RadiosityLightmapSizeX - 1 && IntY < (INT)RadiosityLightmapSizeY - 1 )
		{
			FLinearColor Color0;
			FLinearColor Color1;
			FLinearColor Color2;
			FLinearColor Color3;

			GetRadiositySample( IntX,	IntY,	iBump,	&Color0 );
			GetRadiositySample( IntX+1,	IntY,	iBump,	&Color1 );
			GetRadiositySample( IntX,	IntY+1, iBump,	&Color2 );
			GetRadiositySample( IntX+1,	IntY+1, iBump,	&Color3 );

			//debugf( TEXT("4 sample <%d, %d> <%d, %d> <%d, %d> <%d, %d>"),
			//	IntX, IntY,
			//	IntX+1, IntY,
			//	IntX, IntY + 1,
			//	IntX + 1, IntY + 1 );

			*OutColor = BiLerp(
				Color0,
				Color1,
				Color2,
				Color3,
				FracX,
				FracY
				);
		}
		else if(IntX < (INT)RadiosityLightmapSizeX - 1)
		{
			FLinearColor Color0;
			FLinearColor Color1;

			GetRadiositySample( IntX, IntY, iBump, &Color0 );
			GetRadiositySample( IntX + 1, IntY, iBump, &Color1 );

			//debugf( TEXT("2 sample <%d, %d> <%d, %d>"),
			//	IntX, IntY,
			//	IntX+1, IntY);

			*OutColor = Lerp(
				Color0,
				Color1,
				FracX
				);
		}
		else if(IntY < (INT)RadiosityLightmapSizeY - 1)
		{
			FLinearColor Color0;
			FLinearColor Color1;

			GetRadiositySample( IntX, IntY, iBump, &Color0 );
			GetRadiositySample( IntX, IntY+1, iBump, &Color1 );

			//debugf( TEXT("2 sample <%d, %d> <%d, %d>"),
			//	IntX, IntY,
			//	IntX, IntY + 1);

			*OutColor = Lerp(
				Color0,
				Color1,
				FracY
				);
		}
		else
		{
			GetRadiositySample( IntX, IntY, iBump, OutColor );

			//debugf( TEXT("1 sample <%d, %d>"),	IntX, IntY );
		}

		return TRUE;
	}

	UBOOL GetRadiositySample( UINT X, UINT Y, UINT iBump, FLinearColor* OutColor ) const
	{
		INT U = Clamp( (INT)X, 0, (INT)RadiosityLightmapSizeX-1 );
		INT V = Clamp( (INT)Y, 0, (INT)RadiosityLightmapSizeY-1 );

		UINT NumLuxels	= RadiosityLightmapSizeX * RadiosityLightmapSizeY;

		unsigned char* RadiositySample = &RadiosityLightmap[ ( iBump ) * NumLuxels * 4 ];
		RadiositySample += ( V * RadiosityLightmapSizeX + U ) * 4;

		signed char Exponent = ((signed char)RadiositySample[3]);

		FLOAT R = (FLOAT)RadiositySample[0] * pow( 2.0f, Exponent);
		FLOAT G = (FLOAT)RadiositySample[1] * pow( 2.0f, Exponent);
		FLOAT B = (FLOAT)RadiositySample[2] * pow( 2.0f, Exponent);

		if (R > 255 || G > 255 || B > 255) 
		{
			R = 1;
			G = B = 0;
		}		

		*OutColor = FLinearColor( R, G, B );

		return TRUE;
	}

	UBOOL GetNormalSample( UINT X, UINT Y, FLinearColor* OutColor ) const
	{
		INT iBump = 0;

		INT U = Clamp( (INT)X, 0, (INT)SizeX-1 );
		INT V = Clamp( (INT)Y, 0, (INT)SizeY-1 );

		UINT NumLuxels	= RadiosityLightmapSizeX * RadiosityLightmapSizeY;

		*OutColor = LightSamples(V * SizeX + U).NormalColor;

		return TRUE;
	}


	/**
	* Returns Radiosity Lightmap Sample
	* @param	X - The X coordinate to read from.
	* @param	Y - The Y coordinate to read from.
	* @param	OutColor - On return, Radiosity Sample Value.
	* @return	True if the sample mapped to a point on the surface.
	*/
	UBOOL GetSample(UINT X,UINT Y, UINT iBump, FLinearColor* OutColor) const
	{
		static FLinearColor Black( 0.0f, 0.0f, 0.0f, 1.0f );
		if( X < SizeX && Y < SizeY && iBump < NUM_LIGHTMAP_COEFFICIENTS )
		{
			const FLinearColor& Color = LightSamples( Y * SizeX + X ).Color[iBump];
			UINT Coverage = LightSamples( Y * SizeX + X ).Coverage[iBump];

			if( Coverage != 0 )
			{
				*OutColor = Color / Coverage;
			}
			else
			{
				*OutColor = Black;
			}

			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	UINT GetCoverage( UINT X, UINT Y, UINT iBump ) const
	{
		return LightSamples( Y * SizeX + X ).Coverage[iBump];
	}

protected:

	INT GetMinX() const { return 0; }
	INT GetMaxX() const { return SizeX; }
	INT GetMinY() const { return 0; }
	INT GetMaxY() const { return SizeY; }

	void ProcessPixel(INT X,INT Y,const AvaRadiosityLightMapRasterInterpolant& Interpolant,UBOOL BackFacing)
	{
		check( RadiosityLightmap );

		if( !RadiosityLightmap )
		{
			return;
		}

#ifdef DEBUG_AVA_RASTER
		debugf( TEXT("Process Pixel : (%d, %d)"), X, Y );
#endif

		// bump sample
		for( UINT iBump = 0; iBump < NUM_LIGHTMAP_COEFFICIENTS; ++iBump )
		{
			FLinearColor RadiositySample;

			GetBilinearRadiositySample( Interpolant.RadiosityLightmapCoord, iBump + 1, &RadiositySample );

			if( LightSamples(Y * SizeX + X).Coverage[iBump] )
			{
				FLinearColor& PreviousColor = LightSamples( Y * SizeX + X).Color[iBump];
				FLOAT PreviousSum = PreviousColor.R + PreviousColor.G + PreviousColor.B;
				FLOAT SampleSum = RadiositySample.R + RadiositySample.G + RadiositySample.B;

				if( PreviousSum < SampleSum )
				{
					LightSamples( Y * SizeX + X).Color[iBump] = RadiositySample;
				}
			}
			else
			{
				LightSamples( Y * SizeX + X).Color[iBump] = RadiositySample;
				LightSamples(Y * SizeX + X).Coverage[iBump] = 1;
			}
		}

		// normal sample
		FLinearColor NormalSample;
		GetBilinearRadiositySample( Interpolant.RadiosityLightmapCoord, 0, &NormalSample );
		LightSamples( Y * SizeX + X ).NormalColor = NormalSample;
	}
};

void ConvertRGBEToLinear( const unsigned char* RGBE, FVector* Result )
{
	check(Result);

	signed char Exponent = ((signed char)RGBE[3]);

	Result->X = (FLOAT)(RGBE[0] * pow( 2.0f, Exponent));
	Result->Y = (FLOAT)(RGBE[1] * pow( 2.0f, Exponent));
	Result->Z = (FLOAT)(RGBE[2] * pow( 2.0f, Exponent));
}

//!} 2006-04-14	허 창 민

#endif