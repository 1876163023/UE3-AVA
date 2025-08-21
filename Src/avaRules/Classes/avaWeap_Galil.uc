/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaWeap_Galil extends avaWeap_BaseRifle;

defaultproperties
{
	SightInfos(0) = (FOV=90,ChangeTime=0.1)

	AttachmentClass=class'avaAttachment_Galil'

	ClipCnt=35
	AmmoCount=300
	MaxAmmoCount=300	

	ReloadTime=1.8

	Penetration = 2
	RangeModifier=0.81

	AccuracyDivisor=200
	AccuracyOffset=0.35
	MaxInaccuracy=1.25
	TimeToIdleAfterFire=1.28
	IdleInterval=20	

	BulletType=class'avaBullet_556NATO'

	InstantHitDamageTypes(0)	=	class'avaDmgType_Rifle'
}
