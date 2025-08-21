#include "vrad.h"
#include "lightmap.h"
#include "radial.h"
#include "SHMath.h"
#include <math.h>


void ComputeLuminance( const float* src, const int width, const int height, float* lum )
{
	const float lumWeight[3] = { 0.2135f, 0.7154f, 0.0721f };

	for( int x = 0; x < width; ++x )
	{
		for( int y =0; y < height; ++y )
		{
			const float* pixel = &src[ ( y * width + x ) * 3 ];
			lum[ y * width + x ] = pixel[0] * lumWeight[0] + pixel[1] *lumWeight[1] + pixel[2] * lumWeight[2];
		}
	}
}

float gLightmapMin;
float gLightmapMax;
float gLightmapLum;

float gLightmapLinearAdaption;

int	  gLightmapSize;

void ComputeLuminanceInfo( const float* lum, const int width, const int height, float* minLum, float* maxLum, float* adaptionLum, float* linearAvg )
{
	*minLum = lum[0];
	*maxLum = lum[0];

	float worldLum = 0.0f;
	int lumCount = 0;
	float sumLum = 0.0f;

	for( int x = 0; x < width; ++x)
	{
		for( int y = 0; y < height; ++y )
		{
			float l = lum[ y * width + x ];

			if( l > EQUAL_EPSILON )
			{
				*maxLum = ( *maxLum > l ) ? *maxLum : l;
				*minLum = ( *minLum < l ) ? *minLum : l;

				worldLum += log( l );
				sumLum += l;

				++lumCount;

				ThreadLock();
				gLightmapMin = ( gLightmapMin < l ) ? gLightmapMin : l;
				gLightmapMax = ( gLightmapMax > l ) ? gLightmapMax : l;
				gLightmapLum += log( l );
				gLightmapLinearAdaption += l;
				++gLightmapSize;
				ThreadUnlock();
			}
			
		}
	}

	worldLum = exp( worldLum / lumCount );
    
	*adaptionLum = worldLum;
	*linearAvg = sumLum / lumCount;
}




namespace Tonemap
{
	float gK;
	float gM;
	float gF;
	float gW;

	float gMaxCol;
	float gMinCol;
	
	float gWorldLum;
	float gWorldLinearLum;

	const float maxDisplayLuminance = 300.0f;
	const float displayAdaption = maxDisplayLuminance / 2.0f;
	float gScaleFactor = 0.0f;
	float gMaxOverbright = 2.0f;

	bool bUseLinearAdaption = false;
	bool bApplyTonemap = false;

	static float C( float y )
	{
		if( y < 0.0034f )
		{
			return y / 0.0014f;
		}
		else if ( y < 1 )
		{
			return 2.4483f + log10f( y / 0.0034f) / 0.4027f;
		}
		else if( y < 7.2444f )
		{
			return 16.563f + ( y - 1 ) / 0.4027f;
		}
		else
		{
			return 32.0693f + log10f( y / 7.2444f ) / 0.0556f;
		}
	}

	static float T( float y, float CYmin, float CYmax, float maxDisplay )
	{
		return maxDisplay * ( C(y) - CYmin ) / ( CYmax - CYmin );
	}

	void Initialize( const float* src, const int width, const int height, float f, float w )
	{
		const int imageSize = width * height;

		float *lum = new float[ imageSize ];

		ComputeLuminance( src, width, height, lum );

		float maxLum = lum[0];
		float minLum = lum[0];
		float worldLum = 0.0f;

		for( int i = 1; i < imageSize; ++i )
		{
			float l = lum[i];
			maxLum = ( maxLum > l ) ? maxLum : l;
			minLum = ( minLum < l ) ? minLum : l;
			worldLum += log( 2.3e-5 + l );
		}

		worldLum = exp( worldLum / imageSize );
		maxLum = log( 2.3e-5 + maxLum );
		minLum = log( 2.3e-5 + minLum );

		gK = ( maxLum - log( worldLum ) )/ (maxLum - minLum );
		gM = 0.3f + 0.7f * pow( gK, 1.4f );
		gF = exp( -f );
		gW = w;
		gMaxCol = 0.0f;
		gMinCol = 1.0f;

		delete[] lum;

		gLightmapMin = 1.0e+10;
		gLightmapMax = 0.0f;
		gLightmapSize = 0;
		gLightmapLum = 0.0f;
		gLightmapLinearAdaption = 0.0f;
		gScaleFactor = 0.0f;

		bApplyTonemap = true;
	}

	bool DoTonemap( float scale )
	{
		if (scale <= 0 || scale >= 20)
			return false;

		gScaleFactor = scale;

		bApplyTonemap = true;

		return bApplyTonemap;
	}

	void UseLinearAdaption( bool bUse )
	{
		bUseLinearAdaption = bUse;
	}

	void ComputeTonemapParameters()
	{	
		float worldLum = exp( gLightmapLum / gLightmapSize );
		gWorldLum = worldLum;

		gWorldLinearLum = gLightmapLinearAdaption / gLightmapSize;

		float a = 1.219f + powf( displayAdaption, 0.4f );
		float b = bUseLinearAdaption ? 1.219f + powf( gWorldLinearLum, 0.4f ) : 1.219f + powf( gWorldLum, 0.4f );
		
		gScaleFactor = ( 1.0f / maxDisplayLuminance ) * powf( a / b, 2.5f );

		Msg( "Tonemap scale factor is %f\n", gScaleFactor );

		/*if (gScaleFactor * gLightmapMax > gMaxOverbright)
		{
			gScaleFactor = gMaxOverbright / gLightmapMax;
		}*/

		//gScaleFactor = gMaxOverbright / gLightmapMax;
	}

	void PrintGlobalInfo()
	{
		Msg( "\nGlobal Luminance Info( Min(%e), Max(%e), DynamicRange(%f), worldLum(%f) )\n", gLightmapMin, gLightmapMax, gLightmapMax / gLightmapMin, gWorldLum );
	}

	void ScaleLocalImage( const float* src, const int width, const int height, float* dest, float localAdaptionY, float localYmin, float localYmax )
	{
		float maxDisplay = 16.0f;

		float s = T( localAdaptionY, C(localYmin), C(localYmax), maxDisplay ) / localAdaptionY;

		for( int x = 0; x < width; ++x )
		{
			for( int y = 0; y < height; ++y )
			{
				const int index = ( y * width + x ) * 3;
				dest[index + 0] = s * src[index + 0];
				dest[index + 1] = s * src[index + 1];
				dest[index + 2] = s * src[index + 2];				
			}
		}
	}

	void AddImage( float* src, const int width, const int height )
	{
		const int imageSize = width * height;

		float* lum = new float[ imageSize ];

		ComputeLuminance( src, width, height, lum );

		float localMinLuminance;
		float localMaxLuminance;
		float localAdaptionLuminance;
		float localLinearAdaption;
		
		ComputeLuminanceInfo( lum, width, height, &localMinLuminance, &localMaxLuminance, &localAdaptionLuminance, &localLinearAdaption );

		//ScaleLocalImage( src, width, height, src, localLinearAdaption, localMinLuminance, localMaxLuminance );

		//Msg( "\nLuminance Info( Min(%e), Max(%e), Avg(%f), DynamicRange(%f) )\n", localMinLuminance, localMaxLuminance, localLinearAdaption, localMaxLuminance / localMinLuminance );

		delete[] lum;
	}

	void ScaleImage( const float* src, const int width, const int height, float* dest )
	{
		for( int x = 0; x < width; ++x )
		{
			for( int y = 0; y < height; ++y )
			{
				const int index = ( y * width + x ) * 3;
				dest[index + 0] = gScaleFactor * src[index + 0];
				dest[index + 1] = gScaleFactor * src[index + 1];
				dest[index + 2] = gScaleFactor * src[index + 2];

				//ThreadLock();
				//gMaxCol = gMaxCol > dest[index+0] ? gMaxCol : dest[index+0];
				//gMaxCol = gMaxCol > dest[index+1] ? gMaxCol : dest[index+1];
				//gMaxCol = gMaxCol > dest[index+2] ? gMaxCol : dest[index+2];

				//gMinCol = gMinCol < dest[index+0] ? gMinCol : dest[index+0];
				//gMinCol = gMinCol < dest[index+1] ? gMinCol : dest[index+1];
				//gMinCol = gMinCol < dest[index+2] ? gMinCol : dest[index+2];
				//ThreadUnlock();
			}
		}
	}

	void ScaleSHRGB( SHVectorRGB& rgb )
	{
		rgb = rgb * gScaleFactor;
	}

	void ScaleRGBE( BYTE* rgbe )
	{
		// read rgbe and make float lightmap
		colorRGBExp32* rgbeSrc = (colorRGBExp32*)rgbe;

		float r = TexLightToLinear( rgbeSrc->r, rgbeSrc->exponent );
		float g = TexLightToLinear( rgbeSrc->g, rgbeSrc->exponent );
		float b = TexLightToLinear( rgbeSrc->b, rgbeSrc->exponent );

		Vector vec( r, g, b );
		VectorScale( vec, gScaleFactor, vec );
		Vec3toColorRGBExp32( vec, (colorRGBExp32*)rgbe );

	}

	void NormalizeImage( float* dest, const int width, const int height )
	{
		//--- normalize intensities
		for( int x = 0 ; x< width ; x++ )
		{
			for( int y = 0 ; y<height ; y++ )
			{
				float* pixel = &dest[ (y*width + x)*3 ];
				pixel[0] = ( pixel[0] - gMinCol)/(gMaxCol-gMinCol);
				pixel[1] = ( pixel[1] - gMinCol)/(gMaxCol-gMinCol);
				pixel[2] = ( pixel[2] - gMinCol)/(gMaxCol-gMinCol);
			}
		}
	}
};


void TonemapAmbientCubes( int iThread, int ambientcube_chunk_number )
{
	int start_ambientcube = ambientcube_chunk_number * ambientcube_chunk_size;
	int end_ambientcube = min(s_pVRadDll2->NumAmbientCubes_, start_ambientcube + ambientcube_chunk_size);

	for( int v = start_ambientcube; v < end_ambientcube; ++v )
	{
		Tonemap::ScaleSHRGB( (SHVectorRGB&)s_pVRadDll2->AmbientCubes_[ v ].SHValues );
	}

	for( int v = start_ambientcube; v < end_ambientcube; ++v )
	{
		Tonemap::ScaleRGBE( s_pVRadDll2->AmbientCubes_[ v ].Irradiance );
	}
}

void TonemapLightVertices( int iThread, int vertex_chunk_number )
{
	int start_vertex = vertex_chunk_number * vertex_chunk_size;
	int end_vertex = min(num_blackmesh_vertexes, start_vertex + vertex_chunk_size);

	const int vertex_light_size = 16;
	const int vertexcount = end_vertex - start_vertex;

	const int bumpCount = NUM_BUMP_VECTS + 1;

	float* src_vertexlight = new float[vertexcount * bumpCount * 3];
	float* tonemap_vertexlight = new float[vertexcount * bumpCount * 3];

	for( int pass = 0; pass < 2; ++pass )
	{
		// read rgbe and make float lightmap
		colorRGBExp32* rgbeSrc = (pass == 0) ? (colorRGBExp32*)&blackmesh_vertexlightdata[ start_vertex * vertex_light_size ]
											:(colorRGBExp32*)&blackmesh_worldvertexlightdata[ start_vertex * vertex_light_size ];

		for( int v = start_vertex; v < end_vertex; ++v )
		{
			for( int b = 0; b < bumpCount; ++b)
			{
				float* floatSrc = &src_vertexlight[ ( ( v - start_vertex ) * bumpCount + b ) * 3 ];

				floatSrc[0] = TexLightToLinear( rgbeSrc->r, rgbeSrc->exponent );
				floatSrc[1] = TexLightToLinear( rgbeSrc->g, rgbeSrc->exponent );
				floatSrc[2] = TexLightToLinear( rgbeSrc->b, rgbeSrc->exponent );

				++rgbeSrc;
			}

		}

		// scale float lightmap
		Tonemap::ScaleImage( src_vertexlight, vertexcount * bumpCount, 1, tonemap_vertexlight );

		// write scale float lightmap to rgbe
		colorRGBExp32* rgbeDest = (pass == 0) ? (colorRGBExp32*)&blackmesh_vertexlightdata[ start_vertex * vertex_light_size ]
												: (colorRGBExp32*)&blackmesh_worldvertexlightdata[start_vertex * vertex_light_size];

		for( int v = start_vertex; v < end_vertex; ++v )
		{
			for( int b = 0; b < bumpCount; ++b )
			{
				Vector* tonemapSrc = (Vector*) &tonemap_vertexlight[ ( (v - start_vertex) * bumpCount + b) * 3 ];

				Vec3toColorRGBExp32( *tonemapSrc, rgbeDest );

				++rgbeDest;
			}
		}
	}

	delete[] src_vertexlight;
	delete[] tonemap_vertexlight;
}

void TonemapLightFace( int iThread, int facenum )
{
	dface_t	        *f;
	facelight_t	    *fl;
	int			    lightstyles;
	Vector		    lb[NUM_BUMP_VECTS + 1], v[NUM_BUMP_VECTS + 1];

	int				bumpSample;


	f = &dfaces[facenum];

	// test for non-lit texture
	if ( texinfo[f->texinfo].flags & TEX_SPECIAL)
		return;		

	fl = &facelight[facenum];


	for (lightstyles=0; lightstyles < MAXLIGHTMAPS; lightstyles++ )
	{
		if ( f->styles[lightstyles] == 255 )
			break;
	}

	if ( !lightstyles )
		return;

	bool needsBumpmap = ( texinfo[f->texinfo].flags & SURF_BUMPLIGHT ) ? true : false;
	int bumpSampleCount = needsBumpmap ? NUM_BUMP_VECTS + 1 : 1;



	const int lightmapWidth = f->m_LightmapTextureSizeInLuxels[0] + 1;
	const int lightmapHeight = f->m_LightmapTextureSizeInLuxels[1] + 1;
	float* floatLightmap = new float[fl->numluxels * 3];
	float* scaledLightmap = new float[fl->numluxels * 3];

	// lightmap
	for (int k=0 ; k < lightstyles; k++ )
	{
		for( bumpSample = 0; bumpSample < bumpSampleCount; ++bumpSample )
		{
			// read rgbe and make float lightmap
			colorRGBExp32* rgbeSrc = (colorRGBExp32*)&dlightdata[f->lightofs + (k * bumpSampleCount + bumpSample) * fl->numluxels*4]; 

			for( int x = 0; x < lightmapWidth; ++x )
			{
				for( int y = 0; y < lightmapHeight; ++y )
				{
					float* floatSrc = &floatLightmap[ (y * lightmapWidth + x) * 3 ];

					floatSrc[0] = TexLightToLinear( rgbeSrc->r, rgbeSrc->exponent );
					floatSrc[1] = TexLightToLinear( rgbeSrc->g, rgbeSrc->exponent );
					floatSrc[2] = TexLightToLinear( rgbeSrc->b, rgbeSrc->exponent );

					++rgbeSrc;
				}
			}

			// scale float lightmap
			Tonemap::ScaleImage( floatLightmap, lightmapWidth, lightmapHeight, scaledLightmap );

			// write scale float lightmap to rgbe
			colorRGBExp32* rgbeDest = (colorRGBExp32*)&dlightdata[f->lightofs + (k * bumpSampleCount + bumpSample) * fl->numluxels*4]; 

			for( int x = 0; x < lightmapWidth; ++x )
			{
				for( int y = 0; y < lightmapHeight; ++y )
				{
					Vector* scaledSrc = (Vector*) &scaledLightmap[ (y * lightmapWidth + x) * 3 ];

					Vec3toColorRGBExp32( *scaledSrc, rgbeDest );

					++rgbeDest;
				}
			}
		}
	}

	// light map without sun
	for (int k=0 ; k < lightstyles; k++ )
	{
		for( bumpSample = 0; bumpSample < bumpSampleCount; ++bumpSample )
		{
			// read rgbe and make float lightmap
			colorRGBExp32* rgbeSrc = (colorRGBExp32*)&dlightdata2[f->lightofs + (k * bumpSampleCount + bumpSample) * fl->numluxels*4]; 

			for( int x = 0; x < lightmapWidth; ++x )
			{
				for( int y = 0; y < lightmapHeight; ++y )
				{
					float* floatSrc = &floatLightmap[ (y * lightmapWidth + x) * 3 ];

					floatSrc[0] = TexLightToLinear( rgbeSrc->r, rgbeSrc->exponent );
					floatSrc[1] = TexLightToLinear( rgbeSrc->g, rgbeSrc->exponent );
					floatSrc[2] = TexLightToLinear( rgbeSrc->b, rgbeSrc->exponent );

					++rgbeSrc;
				}
			}

			// scale float lightmap
			Tonemap::ScaleImage( floatLightmap, lightmapWidth, lightmapHeight, scaledLightmap );

			// write scale float lightmap to rgbe
			colorRGBExp32* rgbeDest = (colorRGBExp32*)&dlightdata2[f->lightofs + (k * bumpSampleCount + bumpSample) * fl->numluxels*4]; 

			for( int x = 0; x < lightmapWidth; ++x )
			{
				for( int y = 0; y < lightmapHeight; ++y )
				{
					Vector* scaledSrc = (Vector*) &scaledLightmap[ (y * lightmapWidth + x) * 3 ];

					Vec3toColorRGBExp32( *scaledSrc, rgbeDest );

					++rgbeDest;
				}
			}
		}
	}

	delete[] floatLightmap;
	delete[] scaledLightmap;
}

void NormalizeLightFace( int iThread, int facenum )
{
	dface_t	        *f;
	facelight_t	    *fl;
	int			    lightstyles;
	Vector		    lb[NUM_BUMP_VECTS + 1], v[NUM_BUMP_VECTS + 1];
	int				bumpSample;


	f = &dfaces[facenum];

	// test for non-lit texture
	if ( texinfo[f->texinfo].flags & TEX_SPECIAL)
		return;		

	fl = &facelight[facenum];


	for (lightstyles=0; lightstyles < MAXLIGHTMAPS; lightstyles++ )
	{
		if ( f->styles[lightstyles] == 255 )
			break;
	}

	if ( !lightstyles )
		return;

	bool needsBumpmap = ( texinfo[f->texinfo].flags & SURF_BUMPLIGHT ) ? true : false;
	int bumpSampleCount = needsBumpmap ? NUM_BUMP_VECTS + 1 : 1;



	const int lightmapWidth = f->m_LightmapTextureSizeInLuxels[0] + 1;
	const int lightmapHeight = f->m_LightmapTextureSizeInLuxels[1] + 1;
	float* floatLightmap = new float[fl->numluxels * 3];


	for (int k=0 ; k < lightstyles; k++ )
	{
		for( bumpSample = 0; bumpSample < bumpSampleCount; ++bumpSample )
		{
			// read rgbe and make float lightmap
			colorRGBExp32* rgbeSrc = (colorRGBExp32*)&dlightdata[f->lightofs + (k * bumpSampleCount + bumpSample) * fl->numluxels*4]; 

			for( int x = 0; x < lightmapWidth; ++x )
			{
				for( int y = 0; y < lightmapHeight; ++y )
				{
					float* floatSrc = &floatLightmap[ (y * lightmapWidth + x) * 3 ];

					floatSrc[0] = TexLightToLinear( rgbeSrc->r, rgbeSrc->exponent );
					floatSrc[1] = TexLightToLinear( rgbeSrc->g, rgbeSrc->exponent );
					floatSrc[2] = TexLightToLinear( rgbeSrc->b, rgbeSrc->exponent );

					++rgbeSrc;
				}
			}

			// normalize float lightmap
			Tonemap::NormalizeImage( floatLightmap, lightmapWidth, lightmapHeight );

			// write scale float lightmap to rgbe
			colorRGBExp32* rgbeDest = (colorRGBExp32*)&dlightdata[f->lightofs + (k * bumpSampleCount + bumpSample) * fl->numluxels*4]; 

			for( int x = 0; x < lightmapWidth; ++x )
			{
				for( int y = 0; y < lightmapHeight; ++y )
				{
					Vector* floatSrc = (Vector*) &floatLightmap[ (y * lightmapWidth + x) * 3 ];

					Vec3toColorRGBExp32( *floatSrc, rgbeDest );

					++rgbeDest;
				}
			}
		}
	}

	delete[] floatLightmap;
}

void AddLightFaceToTonemapImage( int iThread, int facenum )
{
	dface_t	        *f;
	facelight_t	    *fl;
	int			    lightstyles;
	Vector		    lb[NUM_BUMP_VECTS + 1], v[NUM_BUMP_VECTS + 1];
	int				bumpSample;


	f = &dfaces[facenum];

	// test for non-lit texture
	if ( texinfo[f->texinfo].flags & TEX_SPECIAL)
		return;		

	fl = &facelight[facenum];


	for (lightstyles=0; lightstyles < MAXLIGHTMAPS; lightstyles++ )
	{
		if ( f->styles[lightstyles] == 255 )
			break;
	}

	if ( !lightstyles )
		return;

	bool needsBumpmap = ( texinfo[f->texinfo].flags & SURF_BUMPLIGHT ) ? true : false;
	int bumpSampleCount = needsBumpmap ? NUM_BUMP_VECTS + 1 : 1;

	const int lightmapWidth = f->m_LightmapTextureSizeInLuxels[0] + 1;
	const int lightmapHeight = f->m_LightmapTextureSizeInLuxels[1] + 1;
	float* floatLightmap = new float[fl->numluxels * 3];


	for (int k=0 ; k < lightstyles; k++ )
	{
		for( bumpSample = 0; bumpSample < bumpSampleCount; ++bumpSample )
		{
			// read rgbe and make float lightmap
			colorRGBExp32* rgbeSrc = (colorRGBExp32*)&dlightdata[f->lightofs + (k * bumpSampleCount + bumpSample) * fl->numluxels*4]; 

			for( int x = 0; x < lightmapWidth; ++x )
			{
				for( int y = 0; y < lightmapHeight; ++y )
				{
					float* floatSrc = &floatLightmap[ (y * lightmapWidth + x) * 3 ];

					floatSrc[0] = TexLightToLinear( rgbeSrc->r, rgbeSrc->exponent );
					floatSrc[1] = TexLightToLinear( rgbeSrc->g, rgbeSrc->exponent );
					floatSrc[2] = TexLightToLinear( rgbeSrc->b, rgbeSrc->exponent );

					++rgbeSrc;
				}
			}

			Tonemap::AddImage( floatLightmap, lightmapWidth, lightmapHeight );

			// write scale float lightmap to rgbe
			colorRGBExp32* rgbeDest = (colorRGBExp32*)&dlightdata[f->lightofs + (k * bumpSampleCount + bumpSample) * fl->numluxels*4]; 

			for( int x = 0; x < lightmapWidth; ++x )
			{
				for( int y = 0; y < lightmapHeight; ++y )
				{
					Vector* scaledSrc = (Vector*) &floatLightmap[ (y * lightmapWidth + x) * 3 ];

					Vec3toColorRGBExp32( *scaledSrc, rgbeDest );

					++rgbeDest;
				}
			}
		}
	}

	delete[] floatLightmap;
}