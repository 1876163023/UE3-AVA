/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaWeap_Scout extends avaWeap_BaseSniperRifle;

defaultproperties
{
	AttachmentClass=class'avaAttachment_Scout'

	Spread_WhenFalling = (param1=0.2,param2=0.0)
	Spread_WhenMoving = (param1=0.075,param2=0.0)
	Spread_WhenDucking = (param1=0.0,param2=0.0)
	Spread_WhenSteady= (param1=0.007,param2=0.0)

	ClipCnt=10
	AmmoCount=10
	MaxAmmoCount=100

	ReloadTime=1.8

	Penetration = 3
	RangeModifier=0.98

	AccuracyDivisor=-1
	AccuracyOffset=0
	MaxInaccuracy=0
	TimeToIdleAfterFire=1.8
	IdleInterval=60	

	BulletType=class'avaBullet_762NATO'

	SightInfos(0) = (FOV=90,ChangeTime=0.1)
	SightInfos(1) = (FOV=40)
	SightInfos(2) = (FOV=15)
	
	HitDamage=75
	InstantHitDamageTypes(0)	=	class'avaDmgType_Sniper'
}
