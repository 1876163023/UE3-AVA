/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaWeap_SG552 extends avaWeap_BaseRifle;

defaultproperties
{
	AttachmentClass=class'avaAttachment_SG552'

	ClipCnt=30
	AmmoCount=30
	MaxAmmoCount=90

	ReloadTime=1.8

	Penetration = 2
	RangeModifier=0.955

	AccuracyDivisor=220
	AccuracyOffset=0.3
	MaxInaccuracy=1
	TimeToIdleAfterFire=2
	IdleInterval=20

	Kickback_WhenMoving = (UpBase=1,LateralBase=0.45,UpModifier=0.28,LateralModifier=0.04,UpMax=4.25,LateralMax=2.5,DirectionChange=7)
	Kickback_WhenFalling = (UpBase=1.25,LateralBase=0.45,UpModifier=0.22,LateralModifier=0.18,UpMax=6,LateralMax=4,DirectionChange=5)
	Kickback_WhenDucking = (UpBase=0.6,LateralBase=0.35,UpModifier=0.2,LateralModifier=0.0125,UpMax=3.7,LateralMax=2,DirectionChange=10)
	Kickback_WhenSteady = (UpBase=0.625,LateralBase=0.375,UpModifier=0.25,LateralModifier=0.0125,UpMax=4,LateralMax=2.25,DirectionChange=9)

	Spread_WhenFalling = (param1=0.035,param2=0.45)
	Spread_WhenMoving = (param1=0.035,param2=0.075)
	Spread_WhenDucking = (param1=0,param2=0.02)
	Spread_WhenSteady= (param1=0,param2=0.02)	

	SightInfos(1) = (FOV=55)

	

	BulletType=class'avaBullet_556NATO'

	SightInfos(0) = (FOV=90,ChangeTime=0.1)
	HitDamage=33
	InstantHitDamageTypes(0)	=	class'avaDmgType_Rifle'
}
