/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaWeap_USP extends avaWeap_BasePistol;

defaultproperties
{
	BulletType=class'avaBullet_45ACP'

	BaseSpeed		= 268	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치

	HitDamage = 1

	FireInterval(0)=0.1
	ClipCnt=500
	AmmoCount=500
	MaxAmmoCount=500

	Penetration = 2
	RangeModifier=0.75

	AccuracyDivisor=500
	AccuracyOffset=0.18
	MaxInaccuracy=1.5

	SpreadDecayTime = 0.5

	Kickback_WhenMoving = (UpBase=2.64,LateralBase=0.6,UpModifier=0.54,LateralModifier=0.36,UpMax=14.4,LateralMax=4.8,DirectionChange=3)
	Kickback_WhenFalling = (UpBase=3.3,LateralBase=0.75,UpModifier=0.675,LateralModifier=0.45,UpMax=18,LateralMax=6,DirectionChange=3)
	Kickback_WhenDucking = (UpBase=1.76,LateralBase=0.4,UpModifier=0.36,LateralModifier=0.24,UpMax=9.6,LateralMax=3.2,DirectionChange=3)
	Kickback_WhenSteady = (UpBase=2.2,LateralBase=0.5,UpModifier=0.45,LateralModifier=0.3,UpMax=12,LateralMax=4,DirectionChange=3)
	
	Spread_WhenFalling = (param1=0.021,param2=0.015)
	Spread_WhenMoving = (param1=0.0175,param2=0.013)
	Spread_WhenDucking = (param1=0.0126,param2=0.009)
	Spread_WhenSteady = (param1=0.014,param2=0.01)

	Kickback_WhenMovingA = (UpBase=2.64,LateralBase=0.6,UpModifier=0.54,LateralModifier=0.36,UpMax=14.4,LateralMax=4.8,DirectionChange=3)
	Kickback_WhenFallingA = (UpBase=3.3,LateralBase=0.75,UpModifier=0.675,LateralModifier=0.45,UpMax=18,LateralMax=6,DirectionChange=3)
	Kickback_WhenDuckingA = (UpBase=1.76,LateralBase=0.4,UpModifier=0.36,LateralModifier=0.24,UpMax=9.6,LateralMax=3.2,DirectionChange=3)
	Kickback_WhenSteadyA = (UpBase=2.2,LateralBase=0.5,UpModifier=0.45,LateralModifier=0.3,UpMax=12,LateralMax=4,DirectionChange=3)
	
	Spread_WhenFallingA = (param1=0.021,param2=0.015)
	Spread_WhenMovingA = (param1=0.0175,param2=0.013)
	Spread_WhenDuckingA = (param1=0.0126,param2=0.009)
	Spread_WhenSteadyA = (param1=0.014,param2=0.01)
	
	AccuracyDivisorA  =  500
	AccuracyOffsetA  =  0.18
	MaxInaccuracyA  =  1.5
	
	DirectionHold = 1
	FireIntervalMultiplierA = 1.8

	KickbackLimiter(0) = (Min=0.6,Max=1)
	KickbackLimiter(1) = (Min=0.7,Max=1)
	KickbackLimiter(2) = (Min=0.7,Max=1)

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.6
	BobDampingInDash	=	0.4

	EquipTime			=0.70
	PutDownTime			=0.3333
	ReloadTime			=2.2

	SightInfos(0) = (FOV=90,ChangeTime=0.1)

	AttachmentClass=class'avaAttachment_Glock'

	BaseSkelMeshName	=	"Wp_Pis_Glock21C.Wp_Glock21C_Basic.MS_Glock21C_Basic"
	BaseAnimSetName		=	"Wp_Pis_Glock21C.Ani_Glock21C"

	WeaponFireSnd=SoundCue'avaWeaponSounds.Pistol_Glock21c.Pistol_Glock21c_Fire'	
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'

	InstantHitDamageTypes(0)	=	class'avaDmgType_Pistol'
}
