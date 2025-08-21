#pragma once

/* DSP handle을 얻어 낸다 */
void DSP_Allocate( INT* );

void DSP_Free( INT Handle );
void DSP_Apply( INT Slot, INT Handle, FLOAT Duration, FLOAT Fade );
UBOOL DSP_Reset( INT Handle );
void DSP_LowPass( INT Handle, float CutOff, float Resonance );
void DSP_HighPass( INT Handle, float CutOff, float Resonance );
void DSP_Echo( INT Handle, float Delay, float DecayRatio, float DryMix, float WetMix );
void DSP_Reverb( INT Handle, float Room, float Damp, float DryMix, float WetMix, float Width );
void DSP_Software( INT Handle, float gain, INT num_blocks, void* );