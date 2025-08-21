// shadowmap을 vrad에서 지원하기 위해 필요한 코드
// 2006/8/18에 shadowmap 지원이 필요없어, 이곳에 분리, 처리하였다.

#include "vrad.h"
#include "lightmap.h"
//#include "radial.h"
//#include <bumpvects.h>
//#include "utlvector.h"
//#include "vmpi.h"
//
////!{ 2006-03-21	허 창 민
//#include "ivraddll.h"
////!} 2006-03-21	허 창 민
//

// 
// 제거 및 나중에 되살리는 것을 간편하게 하기 때문에, 이 함수는 살려둔다.
/*
=============
PrecompShadowmapOffsets
=============
*/

// face point lighthshadowdata array
// lightshadowdata = light_id(1byte) + shadowdata(byte) * sizeof( numluxels )
// dshadowdata = [num_light(byte) + lightshadowdata_array] per face

void PrecompShadowmapOffsets()
{
	int facenum;

	dface_t *f;

	facelight_t *fl;

	int shadowdatasize = 0;

	for( facenum = 0; facenum < numfaces; facenum++ )
	{
		f = &dfaces[facenum];

		fl = &facelight[facenum];

		if ( texinfo[f->texinfo].flags & TEX_SPECIAL)
			continue;		// non-lit texture

		// set offset
		f->lightshadowofs = shadowdatasize;

		// add this face's shadow size
		const int nLuxels = (f->m_LightmapTextureSizeInLuxels[0]+1) * (f->m_LightmapTextureSizeInLuxels[1]+1);

		shadowdatasize += 1;	// space for num light

		const int shadow_count = fl->shadows.Count();
		for( int shadowIndex = 0; shadowIndex < shadow_count; ++shadowIndex )
		{
			shadowdatasize += 1;			// space for light id
			shadowdatasize += nLuxels;		// space for luxel
		}
	}

	//// The incremental lighting code needs us to preserve the contents of dshadowdata
	//// since it only recomposites lighting for faces that have lights that touch them.
	//if( g_pIncremental && dshadowdata.Count() )
	//	return;

	dshadowdata.SetSize( shadowdatasize );
}

//static inline int AllocateShadowSamples( facelight_t* fl, int light_id )
//{	
//	const int lightshadowIndex = fl->shadows.AddToTail();
//	lightshadow_t& lightshadow = fl->shadows[lightshadowIndex];
//
//	lightshadow.light_id = light_id;
//	lightshadow.shadow = (float*) calloc(fl->numsamples, sizeof(float));
//
//	return lightshadowIndex;
//}
//
//static int FindOrAllocateShadows( facelight_t* fl, int light_id )
//{
//	const int num_shadows = fl->shadows.Count();
//	for( int shadowIndex =0 ; shadowIndex < num_shadows; ++shadowIndex )
//	{
//		const lightshadow_t* lightshadow = &fl->shadows[shadowIndex];
//		if( lightshadow->light_id == light_id )
//		{
//			return shadowIndex;
//		}
//	}
//	return AllocateShadowSamples( fl, light_id );
//}
//
////-----------------------------------------------------------------------------
//// Iterates over all lights and computes shadow at a sample point
////-----------------------------------------------------------------------------
//static void ResampleShadowAtPoint( SampleInfo_t& info, int light_id, float* pShadow )
//{
//	Vector delta;
//	float falloff, dot;
//
//	// Iterate over all direct lights and add them to the particular sample
//	for (directlight_t *dl = activelights; dl != NULL; dl = dl->next)
//	{
//		if ((dl->light.type == emit_skyambient))
//			continue;
//
//		if( dl->light_id != light_id )
//			continue;
//
//		if( dl->light.style != 1 )
//			continue;
//
//		dot = GatherSampleLight( dl, info.m_FaceNum, info.m_Point, info.m_PointNormal[0], delta, &falloff, info.m_iThread );
//
//		float visibility = 1.0f;
//
//		if( dot == 0.0f )
//		{
//			visibility = 0.0f;
//		}
//
//		*pShadow += visibility;
//	}
//}
//
////-----------------------------------------------------------------------------
//// Perform supersampling at a particular point
////-----------------------------------------------------------------------------
//static int SupersampleShadowAtPoint( lightinfo_t& l, SampleInfo_t& info, int sampleIndex, int light_id, float* pShadow )
//{
//	Vector superSamplePosition;
//	Vector2D sampleLightOrigin;
//	Vector2D superSampleLightCoord;
//
//	// Check out the sample we're currently dealing with...
//	sample_t& sample = info.m_pFaceLight->sample[sampleIndex];
//
//	// Get the position of the original sample in lightmapspace
//	WorldToLuxelSpace( &l, sample.pos, sampleLightOrigin );
//	// Msg("coord %f %f\n", coord[0], coord[1] );	
//
//	// Some parameters related to supersampling
//	int subsample = 4;	// FIXME: make a parameter
//	float cscale = 1.0 / subsample;
//	float csshift = -((subsample - 1) * cscale) / 2.0;
//
//	// Clear out the shadow value
//	*pShadow = 0.0f;
//
//	int subsampleCount = 0;
//	for (int s = 0; s < subsample; ++s)
//	{
//		for (int t = 0; t < subsample; ++t)
//		{
//			// make sure the coordinate is inside of the sample's winding and when normalizing
//			// below use the number of samples used, not just numsamples and some of them
//			// will be skipped if they are not inside of the winding
//			superSampleLightCoord[0] = sampleLightOrigin[0] + s * cscale + csshift;
//			superSampleLightCoord[1] = sampleLightOrigin[1] + t * cscale + csshift;
//			// Msg("subsample %f %f\n", superSampleLightCoord[0], superSampleLightCoord[1] );
//
//			// Figure out where the supersample exists in the world, and make sure
//			// it lies within the sample winding
//			LuxelSpaceToWorld( &l, superSampleLightCoord[0], superSampleLightCoord[1], superSamplePosition );
//
//			// A winding should exist only if the sample wasn't a uniform luxel, or if dumppatches is true.
//			if ( sample.w )
//			{
//				if( !PointInWinding( superSamplePosition, sample.w ) )
//					continue;
//			}
//
//			// Compute the super-sample illumination point and normal
//			// We're assuming the flat normal is the same for all supersamples
//			ComputeIlluminationPointAndNormals( l, superSamplePosition, sample.normal, &info );
//
//			// Resample the non-ambient light at this point...
//			ResampleShadowAtPoint( info, light_id, pShadow );
//
//			// Got another subsample
//			++subsampleCount;
//		}
//	}
//
//	return subsampleCount;
//}
//
//// converting to shadow map
////-----------------------------------------------------------------------------
//// Compute the maximum intensity based on all bumped lighting
////-----------------------------------------------------------------------------
//static void ComputeShadowSampleIntensities( SampleInfo_t& info, float *pShadowSamples, float* pSampleIntensity )
//{
//	for(int sampleIdx=0; sampleIdx<info.m_pFaceLight->numsamples; sampleIdx++)
//	{
//		// Compute a separate intensity for each
//		sample_t& sample = info.m_pFaceLight->sample[sampleIdx];
//
//		int destIdx = sample.s + sample.t * info.m_LightmapWidth;
//
//		pSampleIntensity[destIdx] = pShadowSamples[sampleIdx];
//	}
//}
//
//static void ComputeShadowSampleIntensities2( SampleInfo_t& info, int sampleIdx, float *pShadowSamples, float* pSampleIntensity )
//{
//	// Compute a separate intensity for each
//	sample_t& sample = info.m_pFaceLight->sample[sampleIdx];
//
//	int destIdx = sample.s + sample.t * info.m_LightmapWidth;
//
//	pSampleIntensity[destIdx] = pShadowSamples[sampleIdx];
//}
//
//
////-----------------------------------------------------------------------------
//// Perform supersampling on a particular lightstyle
////-----------------------------------------------------------------------------
//static void BuildSupersampleFaceLightShadows( lightinfo_t& l, SampleInfo_t& info, int shadowIndex )
//{
//	// This is used to make sure we don't supersample a light sample more than once
//	int processedSampleSize = info.m_LightmapSize * sizeof(bool);
//	bool* pHasProcessedSample = (bool*)( tls_realloc( tls_hasProcessedSamples, tlsz_hasProcessedSamples, processedSampleSize ) );
//
//	memset( pHasProcessedSample, 0, processedSampleSize );
//
//	// This is used to compute a simple gradient computation of the shadow samples
//	// We're going to store the maximum intensity at each sample location
//	float* pGradient = (float*)(tls_realloc( tls_gradient, tlsz_gradient, info.m_pFaceLight->numsamples * sizeof(float) ) );
//	float* pSampleIntensity = (float*)(tls_realloc( tls_sampleIntensity, tlsz_sampleIntensity, info.m_LightmapSize * sizeof(float) ));
//
//	// Compute the maximum intensity of all lighting associated with this lightstyle
//	float* pShadowSamples = info.m_pFaceLight->shadows[shadowIndex].shadow;
//	ComputeShadowSampleIntensities( info, pShadowSamples, pSampleIntensity );
//
//	float ShadowValue;
//
//	float *pVisualizePass = NULL;
//	if (debug_extra)
//	{
//		int visualizationSize = info.m_pFaceLight->numsamples * sizeof(float);
//		pVisualizePass = (float*)stackalloc( visualizationSize );
//		memset( pVisualizePass, 0, visualizationSize ); 
//	}
//
//	// What's going on here is that we're looking for large lighting discontinuities
//	// (large light intensity gradients) as a clue that we should probably be supersampling
//	// in that area. Because the supersampling operation will cause lighting changes,
//	// we've found that it's good to re-check the gradients again and see if any other
//	// areas should be supersampled as a result of the previous pass. Keep going
//	// until all the gradients are reasonable or until we hit a max number of passes
//	bool do_anotherpass = true;
//	int pass = 1;
//	while (do_anotherpass && pass <= extrapasses)
//	{
//		// Look for lighting discontinuities to see what we should be supersampling
//		ComputeLightmapGradients( info, pHasProcessedSample, pSampleIntensity, pGradient );
//
//		do_anotherpass = false;
//
//		const int light_id = info.m_pFaceLight->shadows[shadowIndex].light_id;
//
//		// Now check all of the samples and supersample those which we have
//		// marked as having high gradients
//		for (int i=0 ; i<info.m_pFaceLight->numsamples; ++i)
//		{
//			// Don't supersample the same sample twice
//			if (pHasProcessedSample[i])
//				continue;
//
//			// Don't supersample if the lighting is pretty uniform near the sample
//			// 1.0f / ( suepersample ^2 )
//			if (pGradient[i] < 0.0625)
//				continue;
//
//
//			// Joy! We're supersampling now, and we therefore must do another pass
//			// Also, we need never bother with this sample again
//			pHasProcessedSample[i] = true;
//			do_anotherpass = true;
//
//			if (debug_extra)
//			{
//				// Mark the little visualization bitmap with a color indicating
//				// which pass it was updated on.
//				pVisualizePass[i] = (pass & 1) * 255;
//				pVisualizePass[i] = (pass & 2) * 128;
//				pVisualizePass[i] = (pass & 4) * 64;
//			}
//
//			// Figure out the position + normal direction of the sample under consideration
//			sample_t& sample = info.m_pFaceLight->sample[i];
//			ComputeIlluminationPointAndNormals( l, sample.pos, sample.normal, &info );
//
//			// Supersample the light for shadow
//			int supersampleCount = SupersampleShadowAtPoint( l, info, i, light_id, &ShadowValue );
//
//			// Because of sampling problems, small area triangles may have no samples.
//			// In this case, just use what we already have
//			if (supersampleCount > 0)
//			{
//				pShadowSamples[i] = ShadowValue / (float)supersampleCount;
//				// Recompute the shadow intensity based on the supersampling
//				ComputeShadowSampleIntensities2( info, i, pShadowSamples, pSampleIntensity );
//			}
//
//		}
//
//		// We've finished another pass
//		pass++;
//	}
//
//	if (debug_extra)
//	{
//		// Copy colors representing which supersample pass the sample was messed with
//		// into the actual lighting values so we can visualize it
//		for (int i=0 ; i<info.m_pFaceLight->numsamples ; ++i)
//		{
//			pVisualizePass[i] = pShadowSamples[i]; 
//
//		}
//	}
//}


////!{ 2006-06-30	허 창 민
//void AddShadowToRadial( radial_t *rad, 
//					   Vector const &pnt, 
//					   Vector2D const &coordmins, Vector2D const &coordmaxs, 
//					   float shadow,
//					   int shadowIndex )
//{
//	int     s_min, s_max, t_min, t_max;
//	Vector2D  coord;
//	int	    s, t;
//	float   ds, dt;
//	float   r;
//	float	area;
//	int		bumpSample;
//
//	// convert world pos into local lightmap texture coord
//	WorldToLuxelSpace( &rad->l, pnt, coord );
//
//	s_min = ( int )( coordmins[0] );
//	t_min = ( int )( coordmins[1] );
//	s_max = ( int )( coordmaxs[0] + 0.9999f ) + 1; // ????
//	t_max = ( int )( coordmaxs[1] + 0.9999f ) + 1;
//
//	s_min = max( s_min, 0 );
//	t_min = max( t_min, 0 );
//	s_max = min( s_max, rad->w );
//	t_max = min( t_max, rad->h );
//
//	for( s = s_min; s < s_max; s++ )
//	{
//		for( t = t_min; t < t_max; t++ )
//		{
//			float s0 = max( coordmins[0] - s, -1.0 );
//			float t0 = max( coordmins[1] - t, -1.0 );
//			float s1 = min( coordmaxs[0] - s, 1.0 );
//			float t1 = min( coordmaxs[1] - t, 1.0 );
//
//			area = (s1 - s0) * (t1 - t0);
//
//			if (area > EQUAL_EPSILON)
//			{
//				ds = fabs( coord[0] - s );
//				dt = fabs( coord[1] - t );
//
//				r = max( ds, dt );
//
//				if (r < 0.1)
//				{
//					r = area / 0.1;
//				}
//				else
//				{
//					r = area / r;
//				}
//
//				int i = s+t*rad->w;
//
//				rad->shadow[shadowIndex][i] += r * shadow;
//			}
//		}
//	}
//}
//
////!} 2006-06-30	허 창 민
//
////!{ 2006-06-30	허 창 민
//radial_t *BuildShadowRadial( radial_t* rad, int facenum, int shadowIndex )
//{
//	int		        j, k;
//	facelight_t	    *fl;
//	faceneighbor_t  *fn;
//
//	Vector2D		coordmins, coordmaxs;
//
//	fl = &facelight[facenum];
//	fn = &faceneighbor[facenum];
//
//	const int light_id = fl->shadows[shadowIndex].light_id;
//
//	const int my_shadowIndex = shadowIndex;
//	for( k = 0; k < fl->numsamples; k++)
//	{
//		float shadow = fl->shadows[my_shadowIndex].shadow[k];
//		AddShadowToRadial( rad, fl->sample[k].pos, fl->sample[k].mins, fl->sample[k].maxs, shadow, my_shadowIndex);
//	}
//
//	// debug
//	//return rad;
//
//	for (j = 0; j < fn->numneighbors; j++)
//	{
//		fl = &facelight[fn->neighbor[j]];
//
//		// look same lightshadow
//		const int neighbor_shadow_count = fl->shadows.Count();
//
//		int neighbor_shadow_index = -1;
//		for( int nei_shadowIndex =0; nei_shadowIndex < neighbor_shadow_count; ++nei_shadowIndex )
//		{
//			lightshadow_t* lightshadow = &fl->shadows[nei_shadowIndex];
//			if( lightshadow->light_id == light_id )
//			{
//				neighbor_shadow_index = nei_shadowIndex;
//				break;
//			}
//		}
//
//		// skip neighbor
//		if( neighbor_shadow_index == -1 )
//		{
//			continue;
//		}
//
//		lightinfo_t l;
//
//		InitLightinfo( &l, fn->neighbor[j] );
//
//		for (k=0 ; k<fl->numsamples ; k++)
//		{
//			Vector tmp;
//			Vector2D mins, maxs;
//
//			LuxelSpaceToWorld( &l, fl->sample[k].mins[0], fl->sample[k].mins[1], tmp );
//			WorldToLuxelSpace( &rad->l, tmp, mins );
//
//			LuxelSpaceToWorld( &l, fl->sample[k].maxs[0], fl->sample[k].maxs[1], tmp );
//			WorldToLuxelSpace( &rad->l, tmp, maxs );
//
//			const float shadow = fl->shadows[neighbor_shadow_index].shadow[k];
//
//			AddShadowToRadial( rad, fl->sample[k].pos, mins, maxs, shadow, my_shadowIndex );	// update할 shadowmap은 face의 shadowIndex이다.
//		}
//	}
//
//	return rad;
//}
////!} 2006-06-30	허 창 민
//
////!{ 2006-06-30	허 창 민
////-----------------------------------------------------------------------------
//// Purpose: returns the closest shadow value for a given point on the surface
////			this is normally a 1:1 mapping
////-----------------------------------------------------------------------------
//bool SampleShadowRadial( radial_t *rad, Vector& pnt, float* shadow, int shadowIndex )
//{
//	Vector2D coord;
//
//	WorldToLuxelSpace( &rad->l, pnt, coord );
//	int i = ( int )( coord[0] + 0.5f ) + ( int )( coord[1] + 0.5f ) * rad->w;
//
//	bool baseSampleOk = true;
//
//	*shadow = 0.0f;
//
//	if( rad->weight[i] > WEIGHT_EPS )
//	{
//		*shadow = rad->shadow[shadowIndex][i] / rad->weight[i];
//	}
//	else
//	{
//		// error, luxel has no samples
//		// shadow가 드리워지지 않아야 하므로
//		if( bRed2Black )
//		{
//			*shadow = 1.0f;
//		}
//		else
//		{
//			*shadow = 1.0f;	// error를 표시할 방법이 없네..
//		}
//
//		baseSampleOk = false;
//	}
//
//	return baseSampleOk;
//}
////!} 2006-06-30	허 창 민