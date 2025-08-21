/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */

class avaEmit_GlassShards extends avaEmit_HitEffect;

defaultproperties
{
	EmitterTemplate=ParticleSystem'avaEffect.Gun_Effect.PS_Glass_Splatter'

	Begin Object Name=ParticleSystemComponent0
		bOwnerNoSee=true
	End Object
	ParticleSystemComponent=ParticleSystemComponent0
}
