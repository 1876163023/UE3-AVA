#include "ALAudioPrivate.h"
#include "UnNet.h"
#include "SoftwareDSP.h"
#include "FMODDSPExtension.h"

static UALAudioDevice* GetAudioDevice()
{
	if (!GEngine || !GEngine->Client) return NULL;

	return Cast<UALAudioDevice>( GEngine->Client->GetAudioDevice() );	
}

static DSPBase* HandleToDSP( INT Handle )
{
	UALAudioDevice* AudioDevice = GetAudioDevice();

	return (AudioDevice && Handle >= 0 && Handle < AudioDevice->DSPs.Num()) ? AudioDevice->DSPs(Handle) : NULL;
}


void DSP_Allocate( INT* Handle )
{
	UALAudioDevice* AudioDevice = GetAudioDevice();

	if (AudioDevice) 
	{		
		INT Index;
		for (Index=0; Index<AudioDevice->DSPs.Num(); ++Index)
		{
			if (AudioDevice->DSPs(Index) == NULL)
			{
				break;
			}			
		}

		if (Index < AudioDevice->DSPs.Num())
			AudioDevice->DSPs(Index) = new DSPBase( AudioDevice->System, AudioDevice->FXChannelGroup );
		else			
			AudioDevice->DSPs.AddItem( new DSPBase( AudioDevice->System, AudioDevice->FXChannelGroup ) );

		*Handle = Index;

		return;			
	}

	*Handle = -1;	
}

void DSP_Free( INT Handle )
{
	UALAudioDevice* AudioDevice = GetAudioDevice();

	if (!AudioDevice) return;

	DSPBase* DSP = HandleToDSP( Handle );;

	if (!DSP)
		return;		

	INT Slot = DSP->Slot;	

	/* 작동중이었으면 작동을 멈춰야지! */
	if (Slot >= 0 && Slot < MAX_DSP_SLOT) 
	{
		checkSlow(AudioDevice->ActiveDSP[Slot] == Handle);
		
		/* Active slot이 있으면 free시킨다 */
		AudioDevice->ActiveDSP[Slot] = -1;		
	}	

	/* DSP를 정지시킨다 */
	DSP->Teardown();

	/* DSP를 제거한다 */
	delete DSP;

	/* Avail list에 등록한다 */		
	AudioDevice->DSPs(Handle) = NULL;				
}

void DSP_Apply( INT Slot, INT Handle, FLOAT Duration, FLOAT Fade )
{
	UALAudioDevice* AudioDevice = GetAudioDevice();

	if (!AudioDevice) return;	

	/* Slot index가 invalid */
	if (Slot < 0 || Slot >= MAX_DSP_SLOT) return;

	/* 같은 것이면 의미 없다 */
	if (Handle == AudioDevice->ActiveDSP[Slot]) return;	
	
	DSPBase* PrevDSP = HandleToDSP( AudioDevice->ActiveDSP[Slot] );
	DSPBase* NewDSP = HandleToDSP( Handle );

	/// DSP에 Slot을 역추적할 수 있도록 등록한다.
	if (NewDSP)
	{		
		NewDSP->Slot = Slot;
	}	

	/* 기존에 설정되어 있는 것을 끈다 */	
	if (PrevDSP)
		PrevDSP->TurnOff();

	/* 멀쩡한 Handle이면 켠다 */
	if (NewDSP)
	{
		AudioDevice->ActiveDSP[Slot] = Handle;
		
		NewDSP->TurnOn( Duration, Fade );
	}
	else
	{
		AudioDevice->ActiveDSP[Slot] = -1;
	}	
}

UBOOL DSP_Reset( INT Handle )
{
	UALAudioDevice* AudioDevice = GetAudioDevice();

	if (!AudioDevice) return FALSE;

	DSPBase* DSP = HandleToDSP( Handle );;

	/* 있지도 않은 것이면 쉽게 넘어가면 됨 */
	if (!DSP)
		return FALSE;

	INT Slot = DSP->Slot;			

	DSP->Teardown();

	/* 작동중이었으면 작동을 멈춰야지! */
	if (Slot >= 0 && Slot < MAX_DSP_SLOT)
	{
		checkSlow(AudioDevice->ActiveDSP[Slot] == Handle);

		AudioDevice->ActiveDSP[Slot] = -1;
		DSP->Slot = -1;

		return TRUE;
	}
	else
		return FALSE;	
}

void DSP_LowPass( INT Handle, float CutOff, float Resonance )
{
	UALAudioDevice* AudioDevice = GetAudioDevice();

	if (!AudioDevice) return;

	DSPBase* DSP = HandleToDSP( Handle );;

	if (DSP)
	{
		DSP->AddLowPass( CutOff, Resonance );
	}	
}

void DSP_HighPass( INT Handle, float CutOff, float Resonance )
{
	UALAudioDevice* AudioDevice = GetAudioDevice();

	if (!AudioDevice) return;

	DSPBase* DSP = HandleToDSP( Handle );;

	if (DSP)
	{
		DSP->AddHighPass( CutOff, Resonance );
	}	
}

void DSP_Echo( INT Handle, float Delay, float DecayRatio, float DryMix, float WetMix )
{
	UALAudioDevice* AudioDevice = GetAudioDevice();

	if (!AudioDevice) return;

	DSPBase* DSP = HandleToDSP( Handle );;

	if (DSP)
	{
		DSP->AddEcho( Delay, DecayRatio, DryMix, WetMix );
	}	
}

void DSP_Reverb( INT Handle, float Room, float Damp, float DryMix, float WetMix, float Width )
{
	UALAudioDevice* AudioDevice = GetAudioDevice();

	if (!AudioDevice) return;

	DSPBase* DSP = HandleToDSP( Handle );;

	if (DSP)
	{
		DSP->AddReverb( Room, Damp, WetMix, DryMix, Width );
	}	
}

void DSP_Software( INT Handle, FLOAT gain, INT num_blocks, void* blocks )
{
	UALAudioDevice* AudioDevice = GetAudioDevice();

	if (!AudioDevice) return;

	DSPBase* DSP = HandleToDSP( Handle );;

	if (DSP)
	{
		DSP->AddSoftware( gain, num_blocks, (PRCDescriptor*)blocks );
	}	
}