#pragma once

struct SoftwareDSP;

struct PRCDescriptor;

struct DSPFadeOutEntry
{
	DSPFadeOutEntry( FMOD::DSP* DSP, INT Parameter, FLOAT OnValue, FLOAT OffValue );

	FMOD::DSP*									DSP;
	INT											Parameter;
	FLOAT										OnValue, OffValue;		

	void Apply( FLOAT u );
};

struct DSPBase
{
	TArray<FMOD::DSP*>							Filters;
	TArray<DSPFadeOutEntry>						FadeOutEntries;
	TArray<SoftwareDSP*>						SoftwareDSPs;
	FMOD::System*								System;		
	FMOD::ChannelGroup*							ChannelGroup;			
	UBOOL										bActive;
	FLOAT										StartTime, EndTime;	
	FLOAT										FadeTime;
	FLOAT										AutoTurnOff;
	UBOOL										bTurningOn, bTurningOff;
	INT											Slot;

	DSPBase( FMOD::System* System, FMOD::ChannelGroup* InChannelGroup );

	/* 이 DSP를 작동시킨다.
	작동되고 있지 않던 상황이면 Activate까지 불린다 */
	void TurnOn( FLOAT Duration = 0, FLOAT Fade = 1.0f );

	/* 이 DSP를 멈춘다.
	작동되고 있던 상황이면 ProcessFade에서 Deactivate된다 
	주의 : 바로 Deactivate되지 않음 */
	void TurnOff();

	/* 이 DSP의 fade를 관장한다. 자동으로 꺼지기도 함 */
	void ProcessFade();

	/* 이 DSP의 작동을 멈춘다. 작동되고 있던 상황이면 Deactivate도 시킨다 */
	void Teardown();

	/* Activate / Deactivate */
	void Activate( UBOOL bActivate );

	/* Filter 추가 */
	void AddLowPass( FLOAT Cutoff = 5000.0f, FLOAT Resonance = 1.0f );
	void AddHighPass( FLOAT Cutoff = 5000.0f, FLOAT Resonance = 1.0f );
	void AddEcho( FLOAT Delay = 500.0f, FLOAT DecayRatio = 0.5f, FLOAT DryMix = 1.0f, FLOAT WetMix = 1.0f );
	void AddReverb( FLOAT RoomSize = 0.5f, FLOAT Damp = 0.5f, FLOAT WetMix = 0.33f, FLOAT DryMix = 0.66f, FLOAT Width = 1.0f );
	void AddSoftware( FLOAT gain, INT num_blocks, PRCDescriptor* blocks );
};

