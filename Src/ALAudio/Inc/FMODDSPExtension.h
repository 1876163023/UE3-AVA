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

	/* �� DSP�� �۵���Ų��.
	�۵��ǰ� ���� �ʴ� ��Ȳ�̸� Activate���� �Ҹ��� */
	void TurnOn( FLOAT Duration = 0, FLOAT Fade = 1.0f );

	/* �� DSP�� �����.
	�۵��ǰ� �ִ� ��Ȳ�̸� ProcessFade���� Deactivate�ȴ� 
	���� : �ٷ� Deactivate���� ���� */
	void TurnOff();

	/* �� DSP�� fade�� �����Ѵ�. �ڵ����� �����⵵ �� */
	void ProcessFade();

	/* �� DSP�� �۵��� �����. �۵��ǰ� �ִ� ��Ȳ�̸� Deactivate�� ��Ų�� */
	void Teardown();

	/* Activate / Deactivate */
	void Activate( UBOOL bActivate );

	/* Filter �߰� */
	void AddLowPass( FLOAT Cutoff = 5000.0f, FLOAT Resonance = 1.0f );
	void AddHighPass( FLOAT Cutoff = 5000.0f, FLOAT Resonance = 1.0f );
	void AddEcho( FLOAT Delay = 500.0f, FLOAT DecayRatio = 0.5f, FLOAT DryMix = 1.0f, FLOAT WetMix = 1.0f );
	void AddReverb( FLOAT RoomSize = 0.5f, FLOAT Damp = 0.5f, FLOAT WetMix = 0.33f, FLOAT DryMix = 0.66f, FLOAT Width = 1.0f );
	void AddSoftware( FLOAT gain, INT num_blocks, PRCDescriptor* blocks );
};

