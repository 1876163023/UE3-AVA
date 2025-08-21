#include "ALAudioPrivate.h"
#include "UnNet.h"
#include "SoftwareDSP.h"
#include "FMODDSPExtension.h"

DSPFadeOutEntry::DSPFadeOutEntry( FMOD::DSP* DSP, INT Parameter, FLOAT OnValue, FLOAT OffValue ) 
: DSP(DSP), Parameter(Parameter), OffValue(OffValue), OnValue(OnValue)
{
}


void DSPFadeOutEntry::Apply( FLOAT u )
{
	DSP->setParameter( Parameter, (OnValue - OffValue) * u + OffValue );
}

DSPBase::DSPBase( FMOD::System* System, FMOD::ChannelGroup* InChannelGroup )
: System( System ), ChannelGroup( InChannelGroup ), bActive(FALSE), bTurningOn(FALSE), bTurningOff(FALSE), Slot(-1), AutoTurnOff( -1 )
{
}	

void DSPBase::TurnOn( FLOAT Duration /*= 0*/, FLOAT Fade /*= 1.0f */)
{
	FLOAT CurrentTime = appSeconds();

	if (Duration > 0)
	{
		AutoTurnOff = CurrentTime + Duration;
	}
	else
	{
		AutoTurnOff = -1;
	}

	FadeTime = Fade;

	if (Fade < 0)
	{
		bTurningOff = FALSE;
		Activate( TRUE );

		StartTime = EndTime = CurrentTime;
	}
	else
	{
		if (bTurningOn) return;

		if (bTurningOff)
			/// 꺼지고 있던 중?
		{
			FLOAT u = (CurrentTime - StartTime) / (EndTime - StartTime);

			if (u>=1)
			{
				u = 1;
			}

			StartTime = CurrentTime - Fade * (1-u);
			EndTime = StartTime + Fade;			

			bTurningOff = FALSE;
		}
		/// 원래 꺼져있던 상태?
		else
		{
			Activate( TRUE );

			StartTime = CurrentTime;
			EndTime = StartTime + Fade;
		}
	}

	bTurningOn = TRUE;
}

void DSPBase::TurnOff()
{
	if (bTurningOff) return;

	FLOAT CurrentTime = appSeconds();

	if (FadeTime < 0)
	{
		StartTime = CurrentTime;

		bTurningOn = FALSE;
	}
	else
	{
		if (bTurningOn)			
		{
			FLOAT u = (CurrentTime - StartTime) / (EndTime - StartTime);

			if (u>=1)
			{
				u = 1;
			}

			StartTime = CurrentTime - FadeTime * (1-u);
			EndTime = StartTime + FadeTime;			

			bTurningOn = FALSE;
		}
		/// 원래 켜져 있던 상태?
		else
		{
			StartTime = CurrentTime;
			EndTime = StartTime + FadeTime;
		}
	}

	bTurningOff = TRUE;
}

void DSPBase::ProcessFade()
{
	FLOAT CurrentTime = appSeconds();

	if (AutoTurnOff > CurrentTime)
	{
		AutoTurnOff = -1;
		TurnOff();
	}

	if (!bTurningOff && !bTurningOn) return;

	FLOAT u = (CurrentTime - StartTime) / (EndTime - StartTime);

	if (u>1) u=1;

	if (FadeTime < 0 && bTurningOff)
	{
		u = 1 - appExp( (CurrentTime - StartTime) / FadeTime );
	}	

	for (INT i=0; i<FadeOutEntries.Num(); ++i)
	{
		FadeOutEntries(i).Apply( bTurningOff ? (1-u) : u );
	}		

	for (INT i=0; i<SoftwareDSPs.Num(); ++i)
	{
		SoftwareDSPs(i)->mix = bTurningOff ? (1-u) : u;
	}		

	if (u==1 || bTurningOff && FadeTime < 0 && u > 0.995f)
	{
		if (bTurningOff)
		{
			Activate( FALSE );
		}

		bTurningOff = bTurningOn = FALSE;
	}	
}

void DSPBase::Teardown()
{
	/* 먼저 Deactivate시켜서 filter를 system graph에서 제거한다 */
	Activate( FALSE );

	/* 등록 되어 있는 모든 filter를 제거한다 */
	for (INT i=0; i<Filters.Num(); ++i)
	{			
		Filters(i)->release();			
	}

	/* SW filter 객체를 지운다 */
	for (INT i=0; i<SoftwareDSPs.Num(); ++i)
	{
		delete SoftwareDSPs(i);
	}

	SoftwareDSPs.Empty();
	Filters.Empty();
	FadeOutEntries.Empty();
}

void DSPBase::Activate( UBOOL bActivate )
{
	if (bActive ^ bActivate)
	{
		/* 먼저 System의 DSP 작업이 multithread에서 작동하고 있지 않도록 확인 */
		System->lockDSP();

		FMOD::DSP* head;
		System->getDSPHead( &head );

		FMOD::DSP* next;
		head->getInput( 0, &next );

		/* 등록되어 있는 모든 filter에 대해 system dsp graph에 등록하거나 등록 해제 한다 */
		for (INT i=0; i<Filters.Num(); ++i)
		{
			if (bActivate)
				ChannelGroup->addDSP( Filters(i) );
			else
				Filters(i)->remove();
		}		

		/* SW DSP의 작동을 지시한다 */
		for (INT i=0; i<SoftwareDSPs.Num(); ++i)
		{
			SoftwareDSPs(i)->Activate(bActivate);				
		}		

		/* unlock */
		System->unlockDSP();

		bActive = bActivate;

		bTurningOn = bTurningOff = FALSE;
	}		
}

void DSPBase::AddLowPass( FLOAT Cutoff /*= 5000.0f*/, FLOAT Resonance /*= 1.0f*/ )
{
	FMOD::DSP* dsp;

	FMOD_RESULT result;

	result = System->createDSPByType( FMOD_DSP_TYPE_LOWPASS, &dsp );		

	if (result != FMOD_OK)
		return;

	dsp->setParameter(FMOD_DSP_LOWPASS_CUTOFF, Cutoff);
	dsp->setParameter(FMOD_DSP_LOWPASS_RESONANCE, Resonance);		

	FadeOutEntries.AddItem( DSPFadeOutEntry( dsp, FMOD_DSP_LOWPASS_CUTOFF, Cutoff, 20000 ) );

	Filters.AddItem( dsp );
}

void DSPBase::AddHighPass( FLOAT Cutoff /*= 5000.0f*/, FLOAT Resonance /*= 1.0f*/ )
{
	FMOD::DSP* dsp;

	FMOD_RESULT result;

	result = System->createDSPByType( FMOD_DSP_TYPE_HIGHPASS, &dsp );		

	if (result != FMOD_OK)
		return;

	dsp->setParameter(FMOD_DSP_HIGHPASS_CUTOFF, Cutoff);
	dsp->setParameter(FMOD_DSP_HIGHPASS_RESONANCE, Resonance);		

	FadeOutEntries.AddItem( DSPFadeOutEntry( dsp, FMOD_DSP_HIGHPASS_CUTOFF, Cutoff, 0 ) );

	Filters.AddItem( dsp );
}

void DSPBase::AddEcho( FLOAT Delay /*= 500.0f*/, FLOAT DecayRatio /*= 0.5f*/, FLOAT DryMix /*= 1.0f*/, FLOAT WetMix /*= 1.0f*/ )
{
	FMOD::DSP* dsp;

	FMOD_RESULT result;

	result = System->createDSPByType( FMOD_DSP_TYPE_ECHO, &dsp );		

	if (result != FMOD_OK)
		return;

	dsp->setParameter(FMOD_DSP_ECHO_DELAY, Delay);/* Roomsize. 0.0 to 1.0.  Default = 0.5 */
	dsp->setParameter(FMOD_DSP_ECHO_DECAYRATIO, DecayRatio);     /* Damp.     0.0 to 1.0.  Default = 0.5 */
	dsp->setParameter(FMOD_DSP_ECHO_WETMIX, WetMix);   /* Wet mix.  0.0 to 1.0.  Default = 0.33 */
	dsp->setParameter(FMOD_DSP_ECHO_DRYMIX, DryMix);   /* Dry mix.  0.0 to 1.0.  Default = 0.66 */		

	FadeOutEntries.AddItem( DSPFadeOutEntry( dsp, FMOD_DSP_ECHO_WETMIX, WetMix, 0 ) );
	FadeOutEntries.AddItem( DSPFadeOutEntry( dsp, FMOD_DSP_ECHO_DRYMIX, DryMix, 1 ) );

	Filters.AddItem( dsp );
}

void DSPBase::AddReverb( FLOAT RoomSize /*= 0.5f*/, FLOAT Damp /*= 0.5f*/, FLOAT WetMix /*= 0.33f*/, FLOAT DryMix /*= 0.66f*/, FLOAT Width /*= 1.0f */)
{
	FMOD::DSP* dsp;

	FMOD_RESULT result;

	result = System->createDSPByType( FMOD_DSP_TYPE_REVERB, &dsp );		

	if (result != FMOD_OK)
		return;

	dsp->setParameter(FMOD_DSP_REVERB_ROOMSIZE, RoomSize);/* Roomsize. 0.0 to 1.0.  Default = 0.5 */
	dsp->setParameter(FMOD_DSP_REVERB_DAMP, Damp);     /* Damp.     0.0 to 1.0.  Default = 0.5 */
	dsp->setParameter(FMOD_DSP_REVERB_WETMIX, WetMix);   /* Wet mix.  0.0 to 1.0.  Default = 0.33 */
	dsp->setParameter(FMOD_DSP_REVERB_DRYMIX, DryMix);   /* Dry mix.  0.0 to 1.0.  Default = 0.66 */
	dsp->setParameter(FMOD_DSP_REVERB_WIDTH, Width);    /* Stereo width. 0.0 to 1.0.  Default = 1.0 */		

	FadeOutEntries.AddItem( DSPFadeOutEntry( dsp, FMOD_DSP_REVERB_WETMIX, WetMix, 0 ) );
	FadeOutEntries.AddItem( DSPFadeOutEntry( dsp, FMOD_DSP_REVERB_DRYMIX, DryMix, 1 ) );

	Filters.AddItem( dsp );
}		

void DSPBase::AddSoftware( FLOAT gain, INT num_blocks, PRCDescriptor* blocks )
{
	FMOD::DSP* dsp;

	FMOD_RESULT result;

	SoftwareDSP* swdsp = new SoftwareDSP;
	swdsp->handle = NULL;
	swdsp->num_blocks = num_blocks;		
	swdsp->gain = gain;
	memcpy( swdsp->blocks, blocks, sizeof(PRCDescriptor) * num_blocks );		

	FMOD_DSP_DESCRIPTION  dspdesc; 

	memset(&dspdesc, 0, sizeof(FMOD_DSP_DESCRIPTION)); 

	strcpy(dspdesc.name, "SoftwareDSP"); 
	dspdesc.channels     = 0;                   // 0 = whatever comes in, else specify. 
	dspdesc.read         = SoftwareDSP::DSPCallback; 		
	dspdesc.userdata     = (void *)swdsp; 

	swdsp->mix = 1.0f;

	result = System->createDSP(&dspdesc, &dsp); 
	if (result != FMOD_OK)
	{
		delete swdsp;
		return;
	}

	SoftwareDSPs.AddItem( swdsp );
	Filters.AddItem( dsp );		
}


