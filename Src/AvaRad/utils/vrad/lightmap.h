//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================

#ifndef LIGHTMAP_H
#define LIGHTMAP_H
#pragma once

#include "bumpvects.h"
#include "bsplib.h"

typedef struct
{
	dface_t		*faces[2];
	Vector		interface_normal;
	qboolean	coplanar;
} edgeshare_t;

//!{ 2006-05-03	Çã Ã¢ ¹Î
//extern edgeshare_t	edgeshare[MAX_MAP_EDGES];
extern CUtlVector<edgeshare_t>	edgeshare;
//!} 2006-05-03	Çã Ã¢ ¹Î


//==============================================

// This is incremented each time BuildFaceLights and FinalLightFace
// are called. It's used for a status bar in WorldCraft.
extern int g_iCurFace;

//!{ 2006-05-03	Çã Ã¢ ¹Î
//extern int vertexref[MAX_MAP_VERTS];
//extern int *vertexface[MAX_MAP_VERTS];
extern CUtlVector<int> vertexref;
extern CUtlVector<int*>	vertexface;
//!} 2006-05-03	Çã Ã¢ ¹Î


struct faceneighbor_t
{
	int		numneighbors;			// neighboring faces that share vertices
	int		*neighbor;				// neighboring face list (max of 64)

	Vector	*normal;				// adjusted normal per vertex
	Vector	facenormal;				// face normal

	bool	bHasDisp;				// is this surface a displacement surface???
};
//!{ 2006-05-03	Çã Ã¢ ¹Î
//extern faceneighbor_t faceneighbor[MAX_MAP_FACES];
extern CUtlVector<faceneighbor_t> faceneighbor;
//!} 2006-05-03	Çã Ã¢ ¹Î


//==============================================


struct sample_t
{
	// in local luxel space
	winding_t	*w;
	int			s, t;
	Vector2D	coord;	
	Vector2D	mins;
	Vector2D	maxs;
	// in world units
	Vector		pos;
	Vector		normal;
	float		area;	
};

//!{ 2006-06-29	Çã Ã¢ ¹Î
struct lightshadow_t
{
	int		light_id;
	float*	shadow;
};
//!} 2006-06-29	Çã Ã¢ ¹Î

struct facelight_t
{
	// irregularly shaped light sample data, clipped by face and luxel grid
	int			numsamples;
	sample_t	*sample;			
	Vector		*light[MAXLIGHTMAPS][NUM_BUMP_VECTS+1];	// result of direct illumination, indexed by sample

	//!{ 2006-06-29	Çã Ã¢ ¹Î
	CUtlVector<lightshadow_t>	shadows;
	//!} 2006-06-29	Çã Ã¢ ¹Î

	// regularly spaced lightmap grid
	int			numluxels;			
	Vector		*luxel;				// world space position of luxel
	Vector		*luxelNormals;		// world space normal of luxel
	float		worldAreaPerLuxel;
};

extern directlight_t	*activelights;
extern directlight_t	*freelights;

//!{ 2006-05-03	Çã Ã¢ ¹Î
//extern facelight_t		facelight[MAX_MAP_FACES];
extern CUtlVector<facelight_t>	facelight;
//!} 2006-05-03	Çã Ã¢ ¹Î

extern int				numdlights;


//==============================================

struct lightinfo_t
{
	vec_t	facedist;
	Vector	facenormal;

	Vector	facemid;		// world coordinates of center

	Vector	modelorg;		// for origined bmodels

	Vector	textureOrigin;
	Vector	worldToTextureSpace[2];	// s = (world - texorg) . worldtotex[0]
	Vector	textureToWorldSpace[2];	// world = texorg + s * textoworld[0]

	Vector	luxelOrigin;
	Vector	worldToLuxelSpace[2]; // s = (world - texorg) . worldtotex[0]
	Vector	luxelToWorldSpace[2]; // world = texorg + s * textoworld[0]
	
	//	vec_t	exactmins[2], exactmaxs[2]; not used anymore.
	
	int		lightmapTextureMinsInLuxels[2];
	int		lightmapTextureSizeInLuxels[2];

	int		facenum;
	dface_t	*face;

	int		isflat;
	int		hasbumpmap;
};

struct SampleInfo_t
{
	int		m_FaceNum;
	int		m_WarnFace;
	dface_t	*m_pFace;
	facelight_t	*m_pFaceLight;
	int		m_LightmapWidth;
	int		m_LightmapHeight;
	int		m_LightmapSize;
	int		m_NormalCount;
	int		m_Cluster;
	int		m_iThread;
	texinfo_t	*m_pTexInfo;
	bool	m_IsDispFace;
	Vector	m_Point;
	NVector	m_PointNormal[ NUM_BUMP_VECTS + 1 ];
};


extern void InitLightinfo( lightinfo_t *l, int facenum );

void FreeDLights();

namespace Pbrt
{
	//enum { MAX_SAMPLES = 512, MAX_SUN_SAMPLES = 512 };
	enum { MAX_SAMPLES = 256, MAX_SUN_SAMPLES = 256 };

	struct Sample_t
	{
		Vector	wi;			// 12 bytes
		float	pdf;		// 4 bytes
		Vector	radiance;	// 12 bytes
		int		dummy;
	};

	struct EnvmapSample_t : public Sample_t
	{		
		Vector  scaledRadiance;
	};
	
	void Initialize();

	// cosine weighted hemisphere samples
	Sample_t* GetCachedSample( const Vector& normal );
	void CosineWeightedSphereSample( const Vector& normal, Vector* wi, float* pdf );
	void CosineWeightedHemiSphereSample( const Vector& normal, int sampleCount, Sample_t* samples );
	void GetUniformSamplesHemisphere( const Vector &normal, int sampleCount, Sample_t *samples );

	// environment map brightness based samples
	bool LoadEnvironmentMap( const char* fileName, int exposure, float sunScale, float skyMax, float skyScale, float Yaw );

	const Vector& GetSunDirection();
	const Vector& GetMaxSunRadiance();
	void SampleEnvironmentMap( const Vector& wi, Vector* radiance );
	void SampleSkyFromEnvironmentMap( const Vector& wi, Vector* radiance );
	void SampleSunFromEnvironmentMap( const Vector& wi, Vector* radiance );

	void GetEnvironmentMapSamples( EnvmapSample_t** samples );
	float GetEnvrionmentPdf( const Vector& wi );
	
	inline float PowerHeuristic(int nf, float fPdf, int ng,	float gPdf)
	{
		float f = nf * fPdf, g = ng * gPdf;
		return (f*f) / (f*f + g*g);
	}

	inline float CosineWeightedHemiSpherePDF( const Vector& normal, const Vector& wi )
	{
		float dot = DotProduct( normal, wi );
		const float inv_pi = 0.31830988618379067154f;

		if( dot < EQUAL_EPSILON )
		{
			return 0.0f;
		}
		else
		{
			return dot * inv_pi;
		}
	}

	extern float phi, theta;

	extern Sample_t	uniform_sky_samples[MAX_SAMPLES];
	extern Sample_t	uniform_sphere_samples[MAX_SAMPLES];
};

namespace Tonemap
{
	void Initialize( const float* src, const int width, const int height, float f, float w );

	void AddImage( float* src, const int width, const int height);

	bool DoTonemap( float scale );
	void UseLinearAdaption( bool bUse );
	void ComputeTonemapParameters();
	void ScaleImage( const float* src, const int width, const int height, float* dest );

	void PrintGlobalInfo();

	void NormalizeImage( float* image, const int width, const int height );
};

directlight_t *AllocDLight( Vector& origin );

extern bool gSunVisible;

#endif // LIGHTMAP_H
