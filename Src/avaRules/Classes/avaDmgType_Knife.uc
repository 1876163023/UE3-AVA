//=============================================================================
//  avaDmgType_Knife
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
//
//	2006/03/15 by OZ
//		Class »ý¼º	
//
//=============================================================================

class avaDmgType_Knife extends avaDmgType_Melee;

defaultproperties
{
	KDamageImpulse			=	500
	KDeathImpulse			=	2000
	KDeathUpKick			=	200
	bKRadialImpulse			=	true
	bThrowRagdoll			=	true
	GibPerterbation			=	0.15

	HitSound=SoundCue'avaPlayerSounds.Knife.Knife_on_Body'	
	KevlarHitSound=SoundCue'avaPlayerSounds.Knife.Knife_on_Body'	
	HelmetHitSound=SoundCue'avaPlayerSounds.Knife.Knife_on_Body'	
	HeadshotSound=SoundCue'avaPlayerSounds.Knife.Knife_on_Body'	
}
