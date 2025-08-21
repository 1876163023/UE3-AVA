/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaPawnSoundGroup extends Object
	abstract
	dependson(avaPhysicalMaterialProperty);

var SoundCue JumpSound;
var SoundCue LandSound;
var SoundCue DyingSound, KevlarHitSound, HelmetHitSound, HeadshotSound, HitSound;
var SoundCue KevlarHitSoundSelf, HelmetHitSoundSelf, HeadshotSoundSelf, HitSoundSelf;	// 쏜사람이 자신일때 추가적으로 Play 되는 Sound
var SoundCue NVGOnSound, NVGOffSound, FlashlightSound;
var SoundCue WeapZoomSound;
var SoundCue FellDamageSound;
var SoundCue SpawnSound;
var SoundCue TargettedSound;


struct BodyPart2
{
	var	name Id;
	var string SkeletalMeshName;
	var SkeletalMeshComponent BodyMesh;
	var SkeletalMeshComponent OverlayMesh;
	var int	MeshMaterialIndex;
	var SoundCue bRequired;
};

var array<BodyPart2>	BodyParts;





//static function PlayFlashlightSound(Pawn P)
//{
//	P.PlaySound(Default.FlashlightSound);
//}

//=================================================================================================
// Pawn.PlaySound 를 호출하지 않는다. 이미 Replication 되었음...
//=================================================================================================
static function PlayWeaponZoomSound(Pawn P )
{
	P.PlaySound( Default.WeapZoomSound, true );
}

static function PlayDyingSound(Pawn P,class<avaDamageType> DamageType)
{
	if ( DamageType != None && DamageType.default.DyingSound != None )		P.PlaySound( DamageType.default.DyingSound, true );
	else																	P.PlaySound( Default.DyingSound, true );
}

static function PlayPainSound( Pawn P, EHitEffectType EffectType, class<avaDamageType> DamageType, optional bool bSelf = false )
{
	if ( DamageType == class'DmgType_Fell' )
	{
		PlayFellDamageSound( P );
	}
	else
	{
		switch ( EffectType )
		{
		case HET_Default :		PlayHitSound( P, DamageType, bSelf );		break;
		case HET_HelmetHit :	PlayHelmetHitSound( P, DamageType, bSelf );	break;
		case HET_HeadShot :		PlayHeadshotSound( P, DamageType, bSelf );	break;
		case HET_KevlarHit :	PlayKevlarHitSound( P, DamageType, bSelf );	break;
		}
	}
}

static function PlayFellDamageSound(Pawn P )
{
	P.PlaySound( Default.FellDamageSound, true );
}

static function PlayHeadshotSound(Pawn P,class<avaDamageType> DamageType, bool bSelf )
{
	if ( !bSelf )
	{
		if ( DamageType != None && DamageType.default.HeadShotSound != None )	P.PlaySound( DamageType.default.HeadShotSound, true );
		else																	P.PlaySound( Default.HeadshotSound, true );
	}
	else
	{
		P.PlaySound( Default.HeadShotSoundSelf, true );
	}
}

static function PlayHitSound(Pawn P,class<avaDamageType> DamageType, bool bSelf )
{
	if ( !bSelf )
	{
		if ( DamageType != None && DamageType.default.HitSound != None )		P.PlaySound( DamageType.default.HitSound, true );
		else																	P.PlaySound( Default.HitSound, true );
	}
	else
	{
		P.PlaySound( Default.HitSoundSelf, true );
	}
}

static function PlayHelmetHitSound(Pawn P,class<avaDamageType> DamageType, bool bSelf )
{
	if ( !bSelf )
	{
		if ( DamageType != None && DamageType.default.HelmetHitSound != None )	P.PlaySound( DamageType.default.HelmetHitSound, true );	
		else																	P.PlaySound( Default.HelmetHitSound, true );	
	}
	else
	{
		P.PlaySound( Default.HelmetHitSoundSelf, true );
	}
}

static function PlayKevlarHitSound(Pawn P,class<avaDamageType> DamageType, bool bSelf )
{
	if ( !bSelf )
	{
		if ( DamageType != None && DamageType.default.KevlarHitSound != None )	P.PlaySound( DamageType.default.KevlarHitSound, true );
		else																	P.PlaySound( Default.KevlarHitSound, true );
	}
	else
	{
		P.PlaySound( Default.KevlarHitSoundSelf, true );
	}
}

static function PlayNightvisionSound( Pawn P )
{
	if (avaPawn(P).NightvisionActivated)	P.PlaySound( Default.NVGOnSound, true );
	else									P.PlaySound( Default.NVGOffSound, true );
}

static function PlaySpawnSound( Pawn P )
{
	if ( Default.SpawnSound != None )
		P.PlaySound( Default.SpawnSound, true );
}

static function PlayTargettedSound( Pawn P )
{
	if ( Default.TargettedSound != None )
		P.PlaySound( Default.TargettedSound, true );
}



