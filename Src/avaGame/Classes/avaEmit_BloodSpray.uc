/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */

class avaEmit_BloodSpray extends avaEmit_HitEffect;

var ParticleSystem BloodEmitterTemplate[3];
var ParticleSystem BloodEmitterTemplateSelf[3];

defaultproperties
{
	BloodEmitterTemplate(0)	= ParticleSystem'avaEffect.Particles.PS_BloodSpurt_Head'
	BloodEmitterTemplate(1)	= ParticleSystem'avaEffect.Particles.PS_BloodSpurt'
	BloodEmitterTemplate(2)	= ParticleSystem'avaEffect.Particles.PS_BloodSpurt_Small'

	EmitterTemplate=ParticleSystem'avaEffect.Particles.PS_BloodSpurt'
	//Begin Object Name=ParticleSystemComponent0
	//	bOwnerNoSee=true
	//End Object
	ParticleSystemComponent=ParticleSystemComponent0
}
