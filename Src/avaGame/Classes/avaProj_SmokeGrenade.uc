class avaProj_SmokeGrenade extends avaProjectile;

var ParticleSystemComponent	SmokeEmitter;				
var ParticleSystem			SmokeTemplate;
var float					SmokeDuration;			//	Smoke 지속 시간.
var SoundCue				SmokeSound;

simulated function ExplosionTimer()
{
	SmokeEmitter = new(Outer) class'avaParticleSystemComponent';
	AttachComponent(SmokeEmitter);
	SmokeEmitter.SetTemplate(SmokeTemplate);
	SmokeEmitter.SetAbsolute(,true);
	if (SmokeSound != None)
	{			
		PlaySound( SmokeSound, true );
		//	PlaySoundAt(SmokeSound, Location);
	}
	SetTimer( SmokeDuration, false, 'SmokeEndTimer' );
	SetCollision( false, false );
}

simulated function SmokeEndTimer()
{
	local rotator r;
	r = Rotation;
	r.Pitch = 0;	
	SmokeEmitter.DeactivateSystem();
	//Explode(Location, Vector(Rotation) );
	Explode(Location,vector(r));
}

defaultproperties
{
	// Add the Meshk
	Begin Object Name=ProjectileMesh		
		StaticMesh=StaticMesh'Wp_M83.MS_M83_3p'
		Rotation=(Yaw=-8192)
	End Object

	Physics					= PHYS_Falling
	ExplosionTime			= 3.0
	
	speed					= 400
	MaxSpeed				= 1000.0
	TossZ					= 245.0
	CustomGravityScale		= 0.5;

	bCollideWorld			= true
	bBounce					= true
	bNetTemporary			= false
	bWaitForEffects			= false
	
	bCheckProjectileLight	= false

	bExplodeWhenHitWall		= false
	bReflectOffWall			= true
	ReflectOffWallDamping	= 0.5
	ReflectSleepVelocity	= 20
	bSpeedFromOwner			= true

	SmokeTemplate			=	ParticleSystem'avaEffect.Smoke.Smoke_Screen01_PS'	
	ProjExplosionTemplate	=	None	
	MomentumTransfer		=	250000
	SmokeSound				=	SoundCue'avaWeaponSounds.Grenade_Smoke.Grenade_Smoke_Explosion'
	LifeSpan				=	0.0
	Damage					=	0
	DamageRadius			=	0
	SmokeDuration			=	12.0
}
