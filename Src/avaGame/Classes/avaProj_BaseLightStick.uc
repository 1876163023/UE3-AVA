class avaProj_BaseLightStick extends avaProjectile;

var float		ExplodeTime;
var float		RealExplodeTime;
var hmserialize repnotify	float	UsedTime; // [!] 20070624 dEAthcURe 'hmserialize'

replication
{
	if (Role == ROLE_Authority && bNetDirty )
		UsedTime;
}

simulated event ReplicatedEvent(name VarName)
{
	if ( VarName == 'UsedTime' )
	{
		SetUsedTime( UsedTime );
	}
	else Super.ReplicatedEvent( VarName );
}

simulated function SetUsedTime( float Time )
{
	UsedTime = Time;
	RealExplodeTime = ExplodeTime - UsedTime;
	if ( RealExplodeTime < 0.0 )		SetTimer( 3.0 ,false);
	else								SetTimer( RealExplodeTime + 3.0, false );
	CreateProjectileLight();
}

simulated event CreateProjectileLight()
{
	if ( RealExplodeTime > 0.0 )
	{
		avaAttachment_BaseLightStick( avaWeapon( Owner ).WeaponAttachment ).DisableLight();
		ProjectileLight = new(Outer) ProjectileLightClass;
		avaLightStickComponent( ProjectileLight ).ElapsedTime = UsedTime;
		AttachComponent(ProjectileLight);
	}
}

simulated function Timer()
{
	Explode(Location,Normal(Velocity));
}

// {{ [!] 20070624 dEAthcURe
event function onHmRestore()
{
	SetUsedTime( UsedTime );
	`log("[dEAthcURe|avaProj_BaseLightStick::onHmRestore] SetUsedTime" @ UsedTime);
}
// }} [!] 20070624 dEAthcURe

defaultproperties
{
	// Add the Meshk
	Begin Object Name=ProjectileMesh		
		StaticMesh=StaticMesh'Wp_LightStick.MS_LightStick_EU_3p'
		Rotation=(Yaw=-8192)
	End Object

	Physics				= PHYS_Falling
	ExplodeTime			= 15.0
	
	speed				= 400
	MaxSpeed			= 1000.0
	TossZ				= 245.0
	CustomGravityScale	= 0.5;

	bCollideWorld		= true
	bBounce				= true
	bNetTemporary		= false
	bWaitForEffects		= false
	
	MomentumTransfer	= 50000

	//ProjFlightTemplate		=	ParticleSystem'ava_StageEffect.Sparks.LightStick'
	ProjectileLightClass	= class'avaLightStickComponent'
	bCheckProjectileLight	= false


	bExplodeWhenHitWall		= false
	bReflectOffWall			= true
	ReflectOffWallDamping	= 0.3
	ReflectSleepVelocity	= 30
	bSpeedFromOwner			= true
}
