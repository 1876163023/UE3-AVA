/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaWeap_P228 extends avaWeap_BasePistol;

defaultproperties
{
	AttachmentClass=class'avaAttachment_P228'

	Kickback_WhenMoving = (UpBase=2,LateralBase=0,UpModifier=0,LateralModifier=0,UpMax=90,LateralMax=0,DirectionChange=1)
	Kickback_WhenFalling = (UpBase=2,LateralBase=0,UpModifier=0,LateralModifier=0,UpMax=90,LateralMax=0,DirectionChange=1)
	Kickback_WhenDucking = (UpBase=2,LateralBase=0,UpModifier=0,LateralModifier=0,UpMax=90,LateralMax=0,DirectionChange=1)
	Kickback_WhenSteady = (UpBase=2,LateralBase=0,UpModifier=0,LateralModifier=0,UpMax=90,LateralMax=0,DirectionChange=1)


	BaseSkelMeshName	=	"Wp_Pis_P226.Wp_Pis_P226_Base.MS_P226_Basic_1p"
	BaseAnimSetName		=	"Wp_Pis_P226.Wp_Pis_P226_Base.Ani_P226_Dup"

	Spread_WhenFalling = (param1=1.5,param2=-1.5)
	Spread_WhenMoving = (param1=0.255,param2=-0.255)
	Spread_WhenDucking = (param1=0.75,param2=-0.75)
	Spread_WhenSteady= (param1=0.15,param2=-0.15)

	FireInterval(0)=+0.225
	ClipCnt=13
	AmmoCount=13
	MaxAmmoCount=65

	HitDamage = 52

	ReloadTime=2.2

	Penetration = 2
	RangeModifier=0.81

	SpreadDecayTime = 2

	BulletType=class'avaBullet_9MM'

	SightInfos(0) = (FOV=90,ChangeTime=0.1)

	InstantHitDamageTypes(0)	=	class'avaDmgType_Pistol'	
}
