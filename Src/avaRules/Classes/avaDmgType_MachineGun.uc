/*=============================================================================
  avaDmgType_MachineGun
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2006/08/16 by OZ
		DamageType �з�
=============================================================================*/
class avaDmgType_MachineGun extends avaDmgType_Gun;

defaultproperties
{
	KDamageImpulse	=	500
	KDeathImpulse	=	4500
	HitSound=SoundCue'avaPlayerSounds.Damage.Damage1Cue'	
	KevlarHitSound=SoundCue'avaPlayerSounds.Kevlar.Kevlar1Cue'	
	HelmetHitSound=SoundCue'avaPlayerSounds.HelmetHit.HelmetHitCue'	
	HeadshotSound=SoundCue'avaPlayerSounds.HeadShot.HeadShot1Cue'
}
