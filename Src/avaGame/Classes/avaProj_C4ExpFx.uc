//=============================================================================
//  avaProj_C4ExpFx
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
//
//	2006/04/17
//		1. c4 의 폭파 Effect 를 Play 하기 위한 Class
//=============================================================================
class avaProj_C4ExpFx extends Actor;

var SoundCue			ExplosionSound;		// The sound that is played when it explodes
var Emitter				ProjExplosion;
var ParticleSystem		ProjExplosionTemplate;
var MaterialInstance	ExplosionDecal;
var float				DecalWidth, DecalHeight;

var avaPlayerController.ViewShakeInfo ExplosionShake;
var float				ExplosionRadius;

simulated event PostBeginPlay()
{
	local vector NewHitLoc, HitNormal;
	local TraceHitInfo		HitInfo;
	local Actor				ImpactedActor;
	local rotator			r;
	if ( ProjExplosion == None )
	{
		ImpactedActor = Trace(NewHitLoc, HitNormal, (Location - (Vect(0,0,1) * 64)), Location + (Vect(0,0,1) * 64), true,, HitInfo);
		r = Rotation;
		r.Pitch = 0;
		SpawnExplosionEffects(NewHitLoc, vector(r), ImpactedActor );
	}
	if ( Role == ROLE_Authority )
	{
		SetTimer(3.0,false);
	}
	`log( "SpawnExplosionEffects PostBeginPlay" );
}

simulated event Timer()
{
	Destroy();
}

simulated function Destroyed()
{
	local vector NewHitLoc, HitNormal;
	local TraceHitInfo		HitInfo;
	local Actor				ImpactedActor;
	if ( ProjExplosion == None )
	{
		ImpactedActor = Trace(NewHitLoc, HitNormal, (Location - (Vect(0,0,1) * 64)), Location + (Vect(0,0,1) * 64), true,, HitInfo);
		SpawnExplosionEffects(NewHitLoc, HitNormal, ImpactedActor );
	}
}

simulated function ShakeCamera()
{
	local PlayerController PC;	
	local vector L;
	local rotator R;
	local avaPlayerController.ViewShakeInfo NewExplosionShake;

	foreach LocalPlayerControllers( PC )
	{
//		`log( "ShakeCamera" );
		avaPlayerController(PC).GetPlayerViewPoint( L, R );			
		if (VSize(L-Location) < ExplosionRadius)
		{				
			NewExplosionShake = ExplosionShake;
			avaPlayerController(PC).ShakeView( NewExplosionShake );			
		}		
	}
}

simulated function SpawnExplosionEffects(vector HitLocation, vector HitNormal, Actor ImpactedActor )
{
	local avaDecalLifetimeDataRound LifetimeData;
	if (WorldInfo.NetMode != NM_DedicatedServer)
	{
		ShakeCamera();
		if (ProjExplosionTemplate != None )
		{
			ProjExplosion = Spawn(class'avaSpawnedEmitter', self,, HitLocation, rotator(HitNormal));
			if (ProjExplosion != None)
				ProjExplosion.SetTemplate(ProjExplosionTemplate,true);
		}
		if (ExplosionSound != None)		PlaySound(ExplosionSound, true);
		if (ExplosionDecal != None )
		{
			LifetimeData = new(ImpactedActor.Outer) class'avaDecalLifetimeDataRound';
			LifetimeData.Round = avaGameReplicationInfo(WorldInfo.GRI).CurrentRound;
			ImpactedActor.AddDecal(
				ExplosionDecal, HitLocation, rotator(-HitNormal),
				0, DecalWidth, DecalHeight, 10.0, false, 
				none, LifetimeData, true, false, '', -1, -1 );
		}
	}
}

defaultproperties
{
	ExplosionSound			=	SoundCue'avaWeaponSounds.Mission_C4.Mission_C4_explosion'
	ProjExplosionTemplate	=	ParticleSystem'avaEffect.Explosion.PS_C4_Explosion'
	ExplosionDecal			=	Material'avaDecal.Crater.M_C4_Crater'
	DecalWidth				=	800
	DecalHeight				=	800
	RemoteRole				=	ROLE_SimulatedProxy
	ExplosionShake			=	(OffsetMag=(X=-15.0,Y=-15.00,Z=-15.00),OffsetRate=(X=-1000.0,Y=-1000.0,Z=-1000.0),OffsetTime=10,RotMag=(X=0.0,Y=0.0,Z=0.0),RotRate=(X=0.0,Y=0.0,Z=0.0),RotTime=2)	
	ExplosionRadius			=	2560
}
