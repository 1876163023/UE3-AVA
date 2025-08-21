/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaWeap_Elites extends avaWeap_BasePistol;

defaultproperties
{
	AttachmentClass=class'avaAttachment_Elites'


	BaseSkelMeshName	=	"Wp_Pis_P226.Wp_Pis_P226_Base.MS_P226_Basic_1p"
	BaseAnimSetName		=	"Wp_Pis_P226.Wp_Pis_P226_Base.Ani_P226_Dup"

	WeaponFireSnd=SoundCue'avaWeaponSounds.AR_AK47.AR_AK47_Fire'	

	Spread_WhenFalling = (param1=1.5,param2=-1.5)
	Spread_WhenMoving = (param1=0.255,param2=-0.255)
	Spread_WhenDucking = (param1=0.075,param2=-0.075)
	Spread_WhenSteady= (param1=0.15,param2=-0.15)

	FireInterval(0)=0.3
	ClipCnt=18
	AmmoCount=18
	MaxAmmoCount=90
	HitDamage = 36

	ReloadTime=2.2

	Penetration = 1
	RangeModifier=0.75

	SpreadDecayTime = 2

	BulletType=class'avaBullet_9MM'

	Kickback_WhenSteady = (UpBase=2,LateralBase=0,UpModifier=0,LateralModifier=0,UpMax=90,LateralMax=0,DirectionChange=1)	

	SightInfos(0) = (FOV=90,ChangeTime=0.1)
	InstantHitDamageTypes(0)	=	class'avaDmgType_Pistol'
}
