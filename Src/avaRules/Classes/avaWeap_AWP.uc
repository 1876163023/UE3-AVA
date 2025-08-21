//=============================================================================
//  avaWeap_AWP
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
// 
//	2006/04/05 by OZ
//		AWP 는 현재 TPG1 으로 사용되고 있음
//=============================================================================
class avaWeap_AWP extends avaWeap_BaseSniperRifle;

defaultproperties
{
	AttachmentClass=class'avaAttachment_AWP'

	BaseSkelMeshName	=	"Wp_sn_TPG1.Wp_Sn_TPG1_Rail.MS_TPG1"
	BaseAnimSetName		=	"Wp_sn_TPG1.Ani_TPG_1"

	ClipCnt=10
	AmmoCount=10
	MaxAmmoCount=40

	HitDamage=115

	Penetration = 3
	RangeModifier=0.99

	AccuracyDivisor=-1
	AccuracyOffset=0
	MaxInaccuracy=0
	TimeToIdleAfterFire=2
	IdleInterval=60

	Kickback_WhenMoving = (UpBase=2.0,LateralBase=0.45,UpModifier=0.28,LateralModifier=0.045,UpMax=3.75,LateralMax=3,DirectionChange=7)
	Kickback_WhenFalling = (UpBase=2.2,LateralBase=0.5,UpModifier=0.23,LateralModifier=0.15,UpMax=5.5,LateralMax=3.5,DirectionChange=6)
	Kickback_WhenDucking = (UpBase=1.6,LateralBase=0.3,UpModifier=0.2,LateralModifier=0.0125,UpMax=3.25,LateralMax=2,DirectionChange=7)
	Kickback_WhenSteady = (UpBase=1.65,LateralBase=0.35,UpModifier=0.25,LateralModifier=0.015,UpMax=3.5,LateralMax=2.25,DirectionChange=7)

	Spread_WhenFalling = (param1=0.85,param2=0)
	Spread_WhenMoving = (param1=0.25,param2=0)
	Spread_WhenDucking = (param1=0.0,param2=0)
	Spread_WhenSteady= (param1=0.001,param2=0)	


	BulletType=class'avaBullet_338MAG'

	SightInfos(0) = (FOV=90,ChangeTime=0.1)
	SightInfos(1) = (FOV=40,ChangeTime=0.2)
	SightInfos(2) = (FOV=10,ChangeTime=0.22)

	bReleaseZoomAfterFire = true	//	True 이면 Fire 시 Zoom 이 풀린다.
	fReleaseZoomAfterFireInterval = 0.1
	bRecoverZoomAfterFire = true

	InstantHitDamageTypes(0)	=	class'avaDmgType_Sniper'
}
