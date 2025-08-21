/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaWeap_G3SG1 extends avaWeap_BaseSniperRifle;

defaultproperties
{
	AttachmentClass=class'avaAttachment_G3SG1'

	ClipCnt=20
	AmmoCount=20
	MaxAmmoCount=110

	ReloadTime=1.8

	// g3sg1
	Penetration = 3
	RangeModifier=0.98

	AccuracyDivisor=-1
	AccuracyOffset=0
	MaxInaccuracy=0
	TimeToIdleAfterFire=1.8
	IdleInterval=60

	Kickback_WhenMoving = (UpBase=0.0,LateralBase=0.0,UpModifier=0.0,LateralModifier=0.0,UpMax=0.0,LateralMax=0.0,DirectionChange=0)
	Kickback_WhenFalling = (UpBase=0.0,LateralBase=0.0,UpModifier=0.0,LateralModifier=0.0,UpMax=0.0,LateralMax=0.0,DirectionChange=0)
	Kickback_WhenDucking = (UpBase=0.0,LateralBase=0.0,UpModifier=0.0,LateralModifier=0.0,UpMax=0.0,LateralMax=0.0,DirectionChange=0)
	Kickback_WhenSteady = (UpBase=0.0,LateralBase=0.0,UpModifier=0.0,LateralModifier=0.0,UpMax=0.0,LateralMax=0.0,DirectionChange=0)

	Spread_WhenFalling = (param1=0.45,param2=-0.45)
	Spread_WhenMoving = (param1=0.15,param2=0)
	Spread_WhenDucking = (param1=0.035,param2=-0.035)
	Spread_WhenSteady= (param1=0.055,param2=-0.055)	

	SightInfos(0) = (FOV=90,ChangeTime=0.1)
	SightInfos(1) = (FOV=40)
	SightInfos(2) = (FOV=15)

	BulletType=class'avaBullet_762NATO'

	HitDamage=80
	InstantHitDamageTypes(0)	=	class'avaDmgType_Sniper'
}
