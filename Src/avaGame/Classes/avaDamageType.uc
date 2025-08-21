/**
 * avaDamageType
 *
 *
 *
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */

class avaDamageType extends DamageType
	abstract;

var	LinearColor		DamageBodyMatColor;
var float           DamageOverlayTime;
var float           DeathOverlayTime;

var		bool			bDirectDamage;
var		bool            bSeversHead;
var		bool			bCauseConvulsions;
var		bool			bUseTearOffMomentum;	// For ragdoll death. Add entirety of killing hit's momentum to ragdoll's initial velocity.
var		bool			bThrowRagdoll;
var		bool			bLeaveBodyEffect;
var		bool            bBulletHit;
var		bool			bForceTeamDamage;		// Map Option 과 상관없이 Team 원에게 Damage 를 준다.

var()	float			BackStabModifier;


var() float             GibPerterbation;    // When gibbing, the chunks will fly off in random directions.


var SoundCue			DyingSound, KevlarHitSound, HelmetHitSound, HeadshotSound, HitSound;


/**
 * Possibly spawn a custom hit effect
 */
static function SpawnHitEffect(Pawn P, bool bDamage, vector Momentum, name BoneName, vector HitLocation);

/** @return duration of hit effect, primarily used for replication timeout to avoid replicating out of date hits to clients when pawns become relevant */
static function float GetHitEffectDuration(Pawn P, float Damage)
{
	return 0.5;
}

static function PawnTornOff(avaPawn DeadPawn);

/** allows DamageType to spawn additional effects on gibs (such as flame trails) */
static function SpawnGibEffects(avaGib Gib);

defaultproperties
{
	DamageBodyMatColor=(R=10)
	DamageOverlayTime=0.1
	DeathOverlayTime=0.1
	bDirectDamage=true
    GibPerterbation=0.06
	BackStabModifier=0.0
}

