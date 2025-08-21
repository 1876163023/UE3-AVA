
//=============================================================================
//  avaKProj_Grenade.
// 
//  Copyright 2006 Ntix Soft, Inc. All Rights Reserved.
//
//=============================================================================
class avaKProj_SmokeBomb extends avaKProjectile native;

var float   ExplodeTime;            //  터지는 시간.
var float	SmokeDuration;			//	Smoke 지속 시간.

var ParticleSystemComponent	SmokeEmitter;				
var ParticleSystem			SmokeTemplate;

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();
	SetTimer( ExplodeTime, false, 'ExplodeTimer' );
}

simulated function ExplodeTimer()
{
	SmokeEmitter = new(Outer) class'avaParticleSystemComponent';
	AttachComponent(SmokeEmitter);
	SmokeEmitter.SetTemplate(SmokeTemplate);
	/// 2006/3/14 ; deif
	/// 연기는 rotation이 적용되지 않도록 수정
	SmokeEmitter.SetAbsolute(,true);
	if (ExplosionSound != None)
	{			
		PlaySound(ExplosionSound, true,,,Location);
	}
	SetTimer( SmokeDuration, false, 'SmokeEndTimer' );

	SetCollision( false, false );
}

/**
 * Spawn Explosion Effects
 */
simulated function SpawnExplosionEffects(vector HitLocation, vector HitNormal);

simulated function SmokeEndTimer()
{
	local rotator r;
	r = Rotation;
	r.Pitch = 0;	
	SmokeEmitter.DeactivateSystem();
	//Explode(Location, Vector(Rotation) );
	Explode(Location,vector(r));
}

simulated event Landed( vector HitNormal, actor FloorActor )
{
//	`log( "Landed" );
}

defaultproperties
{
//	Components.Remove(StaticMeshComponent0)

	Begin Object Name=StaticMeshComponent0
		StaticMesh=StaticMesh'Wp_M83.MS_M83_3p'
		bNotifyRigidBodyCollision=true
		BlockRigidBody=true
		LightEnvironment=MyLightEnvironment
	End Object
	//CollisionComponent=StaticMeshComponent1
	//StaticMeshComponent=StaticMeshComponent1
	//Components.Add(StaticMeshComponent1)


	//Begin Object Name=StaticMeshComponent0
	//	StaticMesh=StaticMesh'avaChr.grenade'
	//	bNotifyRigidBodyCollision=true
	//End Object
	//CollisionComponent=StaticMeshComponent0
	//bNoDelete=false


	StartImpulse=10

	ExplodeTime=3.0
	SmokeDuration=12.0

	SmokeTemplate=ParticleSystem'avaEffect.Smoke.Smoke_Screen01_PS'	
	ProjExplosionTemplate=None	
	Damage=0
	DamageRadius=0
	MomentumTransfer=50000
	MyDamageType=class'avaDmgType_Explosion'
	LifeSpan=0.0
	ExplosionSound=SoundCue'avaWeaponSounds.Grenade_Smoke.Grenade_Smoke_Explosion'
}