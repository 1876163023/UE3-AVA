/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaWeap_Mac10 extends avaWeap_BaseSMG;

defaultproperties
{
	AttachmentClass=class'avaAttachment_Mac10'
	
	ClipCnt=30
	AmmoCount=30
	MaxAmmoCount=130

	ReloadTime=1.8

	Penetration = 1
	RangeModifier=0.82

	AccuracyDivisor=200
	AccuracyOffset=0.6
	MaxInaccuracy=1.65
	TimeToIdleAfterFire=2
	IdleInterval=20

	Kickback_WhenMoving = (UpBase=0.9,LateralBase=0.45,UpModifier=0.25,LateralModifier=0.035,UpMax=3.5,LateralMax=2.75,DirectionChange=7)
	Kickback_WhenFalling = (UpBase=1.3,LateralBase=0.55,UpModifier=0.4,LateralModifier=0.05,UpMax=4.75,LateralMax=3.75,DirectionChange=5)
	Kickback_WhenDucking = (UpBase=0.75,LateralBase=0.4,UpModifier=0.175,LateralModifier=0.03,UpMax=2.75,LateralMax=2.5,DirectionChange=10)
	Kickback_WhenSteady = (UpBase=0.775,LateralBase=0.425,UpModifier=0.2,LateralModifier=0.03,UpMax=3,LateralMax=2.75,DirectionChange=9)

	Spread_WhenFalling = (param1=0,param2=0.375)
	Spread_WhenMoving = (param1=0,param2=0.03)
	Spread_WhenDucking = (param1=0,param2=0.03)
	Spread_WhenSteady= (param1=0,param2=0.03)	
	

	BulletType=class'avaBullet_9MM'

	SightInfos(0) = (FOV=90,ChangeTime=0.1)
	BaseSkelMeshName		= "Wp_Smg_MP5K.Wp_Smg_MP5K_Basic.MS_MP5K"
	BaseAnimSetName			= "Wp_Smg_MP5K.Ani_MP5K_PDW_1P"
	InstantHitDamageTypes(0)	=	class'avaDmgType_SMG'
}
