/*
	Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
*/
class avaCharacter_PointMan extends avaCharacter;

defaultproperties
{
	SpeedPctByChrType			=	1.0
	MaxSpeedByChrType			=	375
	CharacterIcon				=	"p"
	DefaultWeapons.Empty
	DefaultWeapons.Add( class'avaWeap_MP5' )
	// 위에는 바꾸지 말것!!
	DefaultWeapons.Add( class'avaWeap_Glock' )
	DefaultWeapons.Add( class'avaWeap_Knife' )
	DefaultWeapons.Add( class'avaWeap_Grenade' )
	DefaultWeapons.Add( class'avaWeap_SmokeBomb' )
	DefaultWeapons.Add( class'avaWeap_HEGrenade' )
	TypeID						=	0
	Armor_Stomach				=	0
	Armor_Head					=	9999
	ArmorMax					=	0
	Absorb_Stomach				=	0
	Absorb_Head					=	0
	Helmet_DamageThreshold 		=	0
	SoundGroupClass				=	class'avaRules.avaPawnSoundGroupEX'

	BloodEmitterClass = class'avarules.avaEmit_BloodSprayEX'
	BloodEmitterClassTeen = class'avarules.avaEmit_BloodSprayTeenEX'

	DefaultModifier.Add( class'avaMod_CharHeadA' )
	DefaultModifier.Add( class'avaMod_HelmetA' )
	DefaultModifier.Add( class'avaMod_ArmorA' )
	DefaultModifier.Add( class'avaMod_ArmorPadA' )
	DefaultModifier.Add( class'avaMod_PointmanItemA' )
	DefaultModifier.Add( class'avaMod_GloveA' )
	DefaultModifier.Add( class'avaMod_BootsA' )
	DefaultModifier.Add( class'avaMod_Socket_H11A' )

	TakeHitPhysicsBlendOutSpeed = 1.3
	fTakeHitPhysicsStartWeight	= 0.8
	fTakeHitPhysicsMultiflier	= 0.5
	fComboHitPhysicsMultiflier	= 0.5

	//TESTPLAYHITEX 0~8로 확인가능
	DamagedKickBackInfo(SI_Genetic)=( MinAngleX= 5, MaxAngleX= 10, DirectionRandomX= 0.5, DamageAmpX= 0.2, MinAngleZ= 3, MaxAngleZ= 8, DirectionRandomZ= 0.5, DamageAmpZ= 0.2)
	DamagedKickBackInfo(SI_Head)=( MinAngleX= 5, MaxAngleX= 22, DirectionRandomX= 0, DamageAmpX= 0.3, MinAngleZ= 0, MaxAngleZ= 3, DirectionRandomZ= 0.5, DamageAmpZ= 0.15)
	DamagedKickBackInfo(SI_Stomach)=( MinAngleX= 2, MaxAngleX= 12, DirectionRandomX= 1, DamageAmpX= 0.2, MinAngleZ= 0, MaxAngleZ= 3, DirectionRandomZ= 0.5, DamageAmpZ= 0.15)
	DamagedKickBackInfo(SI_Chest)=( MinAngleX= 1, MaxAngleX= 8, DirectionRandomX= 0.2, DamageAmpX= 0.2, MinAngleZ= 0, MaxAngleZ= 5, DirectionRandomZ= 0.5, DamageAmpZ= 0.15)
	DamagedKickBackInfo(SI_LeftArm)=( MinAngleX= 0, MaxAngleX= 4, DirectionRandomX= 0.5, DamageAmpX= 0.08, MinAngleZ= 2, MaxAngleZ= 6, DirectionRandomZ= 0, DamageAmpZ= 0.1)
	DamagedKickBackInfo(SI_RightArm)=( MinAngleX= 0, MaxAngleX= 4, DirectionRandomX= 0.5, DamageAmpX= 0.08, MinAngleZ= 2, MaxAngleZ= 6, DirectionRandomZ= 1, DamageAmpZ= 0.1)
	DamagedKickBackInfo(SI_LeftLeg)=( MinAngleX= 0, MaxAngleX= 4, DirectionRandomX= 1, DamageAmpX= 0.08, MinAngleZ= 2, MaxAngleZ= 6, DirectionRandomZ= 0, DamageAmpZ= 0.1)
	DamagedKickBackInfo(SI_RightLeg)=( MinAngleX= 0, MaxAngleX= 4, DirectionRandomX= 1, DamageAmpX= 0.08, MinAngleZ= 2, MaxAngleZ= 6, DirectionRandomZ= 1, DamageAmpZ= 0.1)

}

