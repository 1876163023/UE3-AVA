#pragma once

#include "fmod.hpp"

//		dly_idtype,		dly_idelay,		dly_ifeedback,	dly_igain,		dly_iftype,		dly_icutoff,	dly_iqwidth,	dly_iquality, 
//		rva_isize,		rva_idensity,	rva_idecay,		rva_iftype,		rva_icutoff,	rva_iqwidth,	rva_ifparallel,
//		flt_iftype,		flt_icutoff,	flt_iqwidth,	flt_iquality,			
//		crs_ilfotype,	crs_irate,		crs_idepth,		crs_imix,
//		ptc_ipitch,		ptc_itimeslice,	ptc_ixfade,			
//		env_itype,		env_iamp1,		env_iamp2,		env_iamp3,		env_iattack,	env_idecay,		env_isustain,	env_irelease,	
//		lfo_iwav,		lfo_irate,		lfo_ifoneshot,	
//		mdy_idtype,		mdy_idelay,		mdy_ifeedback,	mdy_igain,		mdy_iftype,		mdy_icutoff,	mdy_iqwidth,	mdy_iquality, mdy_imodrate,	mdy_imoddepth,	mdy_imodglide,
//		dfr_isize,		dfr_idensity,	dfr_idecay,		
//		amp_gain,		amp_vthresh,	amp_distmix,	amp_vfeed,



struct PRCDescriptor
{
	INT											type;
	float										params[16];
};

struct SoftwareDSP
{
	INT											num_blocks;
	PRCDescriptor								blocks[16];
	FLOAT										mix;
	FLOAT										gain;

	void* handle;

	void Activate( BOOL bActivate );

	static FMOD_RESULT F_CALLBACK DSPCallback(FMOD_DSP_STATE *dsp_state, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels);
	static FMOD_RESULT F_CALLBACK DSPSetParameterCallback(FMOD_DSP_STATE *dsp_state, int index, float value);
};

