#include "EnginePrivate.h"
#include "EngineDSPClasses.h"

void DSP_Allocate( INT* );
void DSP_Free( INT Handle );
void DSP_Apply( INT Slot, INT Handle, FLOAT Duration, FLOAT Fade );
UBOOL DSP_Reset( INT Handle );
void DSP_LowPass( INT Handle, float CutOff, float Resonance );
void DSP_HighPass( INT Handle, float CutOff, float Resonance );
void DSP_Echo( INT Handle, float Delay, float DecayRatio, float DryMix, float WetMix );
void DSP_Reverb( INT Handle, float Room, float Damp, float DryMix, float WetMix, float Width );
void DSP_Software( INT Handle, float gain, INT num_blocks, void* );

IMPLEMENT_CLASS(UDSPSoftware);
IMPLEMENT_CLASS(UDSPSoftwareBlock);
IMPLEMENT_CLASS(UDSPBlock);
IMPLEMENT_CLASS(UDSPPreset);
IMPLEMENT_CLASS(UDSPEcho);
IMPLEMENT_CLASS(UDSPLowPass);
IMPLEMENT_CLASS(UDSPHighPass);
IMPLEMENT_CLASS(UDSPReverb);

void UDSPBlock::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);	
}

UDSPPreset::UDSPPreset()
: Dirty( TRUE ), Handle(-1)
{		
}

void UDSPPreset::FinishDestroy()
{
	DSP_Free( Handle );

	Super::FinishDestroy();
}

void UDSPPreset::Apply()
{
	if (Dirty)
		Reload();

	DSP_Apply( DSPSlot, Handle, Duration, Fade );
}

void UDSPPreset::Stop()
{
	DSP_Apply( DSPSlot, -1, 0, 0 );
}

void UDSPPreset::Reload()
{
	if (Handle < 0)
		DSP_Allocate( &Handle );

	if (Handle < 0)
		return;

	UBOOL Activated = DSP_Reset( Handle );

	for (INT i=0; i<DSPBlocks.Num(); ++i)
	{
		UDSPBlock* pBlock = DSPBlocks(i);

		if (!pBlock) continue;

		if (pBlock->IsA( UDSPLowPass::StaticClass() ))
		{
			UDSPLowPass* pFilter = Cast<UDSPLowPass>( pBlock );

			DSP_LowPass( Handle, pFilter->CutOff, pFilter->Resonance );
		}
		else if (pBlock->IsA( UDSPHighPass::StaticClass() ))
		{
			UDSPHighPass* pFilter = Cast<UDSPHighPass>( pBlock );

			DSP_LowPass( Handle, pFilter->CutOff, pFilter->Resonance );
		}
		else if (pBlock->IsA( UDSPEcho::StaticClass() ))
		{
			UDSPEcho* pFilter = Cast<UDSPEcho>( pBlock );

			DSP_Echo( Handle, pFilter->delay, pFilter->DecayRatio, pFilter->DryMix, pFilter->WetMix );
		}
		else if (pBlock->IsA( UDSPReverb::StaticClass() ))
		{
			UDSPReverb* pFilter = Cast<UDSPReverb>( pBlock );

			DSP_Reverb( Handle, pFilter->RoomSize, pFilter->Damp, pFilter->DryMix, pFilter->WetMix, pFilter->Width );
		}
		else if (pBlock->IsA( UDSPSoftware::StaticClass() ))
		{
			UDSPSoftware* pFilter = Cast<UDSPSoftware>( pBlock );			

			FDescriptor blocks[64];

			for (INT i=0; i<pFilter->Blocks.Num(); ++i)
			{
				if (pFilter->Blocks(i))
					pFilter->Blocks(i)->eventFillValue( blocks[i] );
			}

			DSP_Software( Handle, pFilter->Gain, pFilter->Blocks.Num(), blocks );
		}
	}

	Dirty = FALSE;

	if (Activated)
		DSP_Apply( DSPSlot, Handle, Duration, Fade );
}