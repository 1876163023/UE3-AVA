/**
 * UTProjectile
 *
 * This is our base projectile class.
 *
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */

class avaProjectile extends Projectile
	abstract
	native;

`include(avaGame/avaGame.uci)

var() const editconst StaticMeshComponent	StaticMeshComponent;
var				AudioComponent				ImpactSoundComponent;
var				PhysicalMaterial			PhysMat;
var				float						LastImapctTime;

var() const editconst LightEnvironmentComponent LightEnvironment;

/** Additional Sounds */
var SoundCue	AmbientSound;		// The sound that is played looped.
var SoundCue	ExplosionSound;		// The sound that is played when it explodes

/** If true, never cut out ambient sound for perf reasons */
var	bool		bImportantAmbientSound;

/** Effects */

/** This is the effect that is played while in flight */
var ParticleSystemComponent	ProjEffects;

/** The Effects for the explosion */
var Emitter ProjExplosion;

/** Effects Template */
var ParticleSystem	ProjFlightTemplate;
var ParticleSystem	ProjExplosionTemplate;

var	SoundCue		ProjFlightSound;
var AudioComponent	ProjFlightSoundComponent;

/** If true, this explosion effect expects to be orientated differently and have extra data
    passed in via parameters */
var bool bAdvanceExplosionEffect;

/** Fast Effects (if bDropDetail) */
var ParticleSystem FastProjFlightTemplate;
var ParticleSystem FastProjExplosionTemplate;

/** decal for explosion */
var MaterialInstance ExplosionDecal;
var float DecalWidth, DecalHeight;

/** This value sets the cap how far away the explosion effect of this projectile can be seen */
var float MaxEffectDistance;

/** used to prevent effects when projectiles are destroyed (see LimitationVolume) */
var	bool bSuppressExplosionFX;

/** if True, this projectile will remain alive (but hidden) until the flight effect is done */
var bool bWaitForEffects;

/** Acceleration magnitude. By default, acceleration is in the same direction as velocity */
var float AccelRate;

var		float	TossZ;

/** This holds the stat's ID of the firing weapon */
var int FiringOwnerStatsID;
//var int FiringWeaponStatsID;
var int FiringWeaponMode;

/** for console games (wider collision w/ enemy players) */
var float CheckRadius;

/** Make true if want to spawn ProjectileLight.  Set false in TickSpecial() once it's been determined whether Instigator is local player.  Done there to make sure instigator has been replicated */
var bool bCheckProjectileLight;

/** Class of ProjectileLight */
var class<PointLightComponent> ProjectileLightClass;

/** LightComponent for this projectile (spawned only if projectile fired by the local player) */
var PointLightComponent ProjectileLight;

/** Class of ExplosionLight */
var class<avaExplosionLight> ExplosionLightClass;

/** Max distance to create ExplosionLight */
var float	MaxExplosionLightDistance;

/** TerminalVelocity for this projectile when falling */
var float TerminalVelocity;

var			  float       Buoyancy;			// Water buoyancy. A ratio (1.0 = neutral buoyancy, 0.0 = no buoyancy)

var	float	CustomGravityScale;

var float	FullDamageMinRadius;

var float								ShakeRedius;		// Radius 안의 Player 의 Camera 를 흔들어 준다.
var avaPlayerController.ViewShakeInfo	ExplosionShake;		// Shake Data

struct native FlashParameters
{
	var float HoldTime, FadeTime, Alpha;
};

var float				FlashRadius, FlashDamage;
var FlashParameters		FrontFlashParameters, BackFlashParameters;
var	float				ExplosionTime;

var bool				bExplodeWhenHitWall;	// true 이면 땅에 닿았을때 폭발한다...
var bool				bReflectOffWall;		// 벽에 닿았을때 튕긴다...	bExpoldeWhenHitWall 가 true 이면 의미가 없다... 벽에 부딪치면 터지는데....
var float				ReflectOffWallDamping;	// 튕겼을때 Damping 값
var float				ReflectSleepVelocity;	// 튕긴후 이 속도 이하이면 Sleep 상태로 가버려야 한다...

var bool				bCheckAutoMessage;		//	
var int					AutoMessageIndex;		//

var	bool				bSpeedFromOwner;		// Owner 가 avaThrowableWeapon 일때 Owner 로 부터 Speed 를 받아올지 여부...
var float				AdjustInstigatorVelFactor;

// trace direction for explosion decal generation
var vector				DecalTraceDirections[6];

var class<Weapon>		WeaponBy;

var	bool				bIndicating;

cpptext
{
	virtual void TickSpecial( FLOAT DeltaSeconds );
	virtual FLOAT GetGravityZ();
}

native final function PhysicalMaterial GetActorPhysMaterial();

/** CreateProjectileLight() called from TickSpecial() once if Instigator is local player
*/
simulated event CreateProjectileLight()
{
	if ( WorldInfo.bDropDetail )
		return;

	ProjectileLight = new(Outer) ProjectileLightClass;
	AttachComponent(ProjectileLight);
}

simulated function SetCollisionProperties()
{
	PhysMat = GetActorPhysMaterial();
	if(PhysMat.ImpactSound != None)
	{
		ImpactSoundComponent = new(Outer) class'AudioComponent';
		ImpactSoundComponent.bAutoDestroy = TRUE;
		AttachComponent(ImpactSoundComponent);
		ImpactSoundComponent.SoundCue = PhysMat.ImpactSound;
	}
}

/**
 * When this actor begins its life, play any ambient sounds attached to it
 */
simulated function PostBeginPlay()
{
	local AudioComponent	AmbientComponent;

	super.PostBeginPlay();

	if ( bDeleteMe )
		return;

	// Set its Ambient Sound
	if (AmbientSound != None && WorldInfo.NetMode != NM_DedicatedServer)
	{
		if ( bImportantAmbientSound || !WorldInfo.bDropDetail )
		{
			AmbientComponent = CreateAudioComponent(AmbientSound, true, true);
			if ( AmbientComponent != None )
			{
				AmbientComponent.bShouldRemainActiveIfDropped = true;
			}
		}
	}

	WeaponBy = class<Weapon>(Owner.Class);

	// Spawn any effects needed for flight
	SpawnFlightEffects();

	SetCollisionProperties();

	if ( ExplosionTime > 0 )
		SetTimer( ExplosionTime, false, 'ExplosionTimer' );

	if ( bIndicating )
		SetTimer( 1.0, false, 'AddGrenadeForIndicating' );
}

simulated function AddGrenadeForIndicating()
{
	local PlayerController P;
	ForEach LocalPlayerControllers( P )
	{
		// 자신이 던진 것, 다른팀이 던진 것, 같은 팀이라도 FrindlyFireType 이 1이상인 것
		if ( P.Pawn == Instigator || 
			 !avaPlayerController(P).IsSameTeam( Instigator ) || 
			 avaGameReplicationInfo(WorldInfo.GRI).FriendlyFireType > 0 )
			avaHUD( P.myHUD ).AddGrenadeForIndicating( self );
	}
}

simulated event SetInitialState()
{
	bScriptInitialized = true;
	if (Role < ROLE_Authority && AccelRate != 0.f)
	{
		GotoState('WaitingForVelocity');
	}
	else
	{
		GotoState('Auto');
	}
}

/**
 * Initalize the Projectile
 */
function Init(vector Direction)
{
	local vector				X, Y, Z;
	local avaThrowableWeapon	weap;
	local rotator				InitRot;

	`log("avaProjectile.Init" @Direction @Instigator);

	if ( bSpeedFromOwner )
	{
		weap = avaThrowableWeapon( Owner );
		Speed	=	weap.GetProjectileVel();
		TossZ	=	weap.GetProjectileTossz();
	}

	GetAxes( Instigator.GetViewRotation(), X, Y, Z );
	
	InitRot = Rotator( Direction );
	InitRot.Pitch	= 0;
	InitRot.Roll	= 0;
	SetRotation( InitRot );

	Velocity	= Speed * X + Instigator.Velocity * AdjustInstigatorVelFactor;
	Velocity.Z += TossZ;
	Acceleration = AccelRate * Normal(Velocity);

	// allow physics volumes to act on us again (for slowvolumes, etc)
	PhysicsVolume.ActorEnteredVolume(self);

	if ( bCheckAutoMessage == true )
		SetTimer( 0.5, true, 'CheckAutoMessage' );

	`log("avaProjectile.Init - End" @Direction);
}

function CheckAutoMessage()
{
	local avaPawn	P;
	foreach VisibleActors( class'avaPawn', P)
	{
		if ( Instigator.GetTeamNum() == P.GetTeamNum() )	continue;
		if ( VSize( Location - P.Location ) > `AUTORADIOMSG_DISTANCE )		continue;
		avaPlayerController( P.Controller ).RaiseAutoMessage( AutoMessageIndex, false );
		ClearTimer( 'CheckAutoMessage' );
		return;
	}
}

/**
 *
 */
simulated function ProcessTouch(Actor Other, Vector HitLocation, Vector HitNormal)
{
	HitWall( HitNormal, Other, None );
}

simulated function ExplosionTimer()
{
	Explode(Location,Vector(Rotation));
}

/**
 * Explode this Projectile
 */
simulated function Explode(vector HitLocation, vector HitNormal)
{
	ClearTimer( 'ExplosionTimer' );

	if (Damage>0 && DamageRadius>0)
		HurtRadius(Damage,DamageRadius, MyDamageType, MomentumTransfer, HitLocation );

	SpawnExplosionEffects(HitLocation, HitNormal);

	if ( Role == ROLE_Authority )
		MakeNoise(1.0);

	ShutDown();
}

/**
 * Spawns any effects needed for the flight of this projectile
 */
simulated function SpawnFlightEffects()
{
	if ( WorldInfo.NetMode != NM_DedicatedServer )
	{
		ProjEffects = new(Outer) class'avaParticleSystemComponent';
		AttachComponent(ProjEffects);
		if ( WorldInfo.bDropDetail && (FastProjFlightTemplate != None) )
			ProjEffects.SetTemplate(FastProjFlightTemplate);
		else
			ProjEffects.SetTemplate(ProjFlightTemplate);
		//ProjEffects.OnSystemFinished = MyOnParticleSystemFinished;

		if ( ProjFlightSound  != None )
		{
			ProjFlightSoundComponent = CreateAudioComponent( ProjFlightSound, TRUE, TRUE ); 
		}

		if ( bCheckProjectileLight )
			CreateProjectileLight();
	}
}

simulated function bool CheckMaxEffectDistance(PlayerController P, vector SpawnLocation, optional float CullDistance)
{
	local float Dist;

	if ( P.ViewTarget == None )
		return true;

	if ( (Vector(P.Rotation) Dot (SpawnLocation - P.ViewTarget.Location)) < 0.0 )
	{
		return (VSize(P.ViewTarget.Location - SpawnLocation) < 1000);
	}

	Dist = VSize(SpawnLocation - P.ViewTarget.Location);

	if (CullDistance > 0 && CullDistance < Dist * P.LODDistanceFactor)
	{
		return false;
	}

	return true;
}

/**
 * Spawn Explosion Effects
 */
simulated function SpawnExplosionEffects(vector HitLocation, vector HitNormal)
{
	local avaDecalLifetimeDataRound LifetimeData;
	local avaExplosionLight L;
	local Actor LightHitActor;
	local vector LightHitLocation, LightHitNormal;
	local vector Direction;
	local vector NewHitLoc;
	local TraceHitInfo HitInfo;
	local int TraceIndex;
	local bool bResetImpactActor;
	local float DistanceFromExplosion;
	local float	VolumeScaleFactor;

	if (WorldInfo.NetMode != NM_DedicatedServer)
	{
		bSuppressExplosionFX = true;
		if (ProjExplosionTemplate != None && EffectIsRelevant(Location, false, MaxEffectDistance))
		{
			if (!bAdvanceExplosionEffect)
			{
				ProjExplosion = Spawn(class'avaEmitter', self,, HitLocation, rotator(HitNormal));
			}
			else
			{
				Direction = normal(Velocity - 2.0 * HitNormal * (Velocity dot HitNormal)) * Vect(1,1,0);
				ProjExplosion = Spawn(class'avaEmitter', self,, HitLocation, rotator(Direction));
			}
				
			if (ProjExplosion != None)
			{
				if ( ImpactedActor != None )
					ProjExplosion.SetBase(ImpactedActor);
				if ( WorldInfo.bDropDetail && (FastProjExplosionTemplate != None) )
					ProjExplosion.SetTemplate(FastProjExplosionTemplate,true);
				else
					ProjExplosion.SetTemplate(ProjExplosionTemplate,true);
				if ( (ExplosionLightClass != None) && !WorldInfo.bDropDetail && ShouldSpawnExplosionLight(HitLocation, HitNormal) )
				{
					L = new(Outer) ExplosionLightClass;
					ProjExplosion.AttachComponent(L);

					LightHitActor = Trace( LightHitLocation, LightHitNormal, HitLocation + 0.25*L.Radius*HitNormal, HitLocation, false );
					if ( LightHitActor == None )
						L.SetTranslation(0.25*L.Radius*vect(1,0,0));
					else
						L.SetTranslation(0.5*VSize(HitLocation - LightHitLocation)*vect(1,0,0));
				}
				
				if (bAdvanceExplosionEffect)
				{
					ProjExplosion.SetVectorParameter('Velocity',Direction);
					ProjExplosion.SetVectorParameter('HitNormal',HitNormal);
				}
			}
		}
		
		if ( ShakeRedius > 0.0 )
			ShakeView( HitLocation );

		if (FlashDamage > 0 && FlashRadius > 0)
			Flash( HitLocation, FlashRadius, FlashDamage );

		if (ExplosionSound != None)
		{
			PlaySound(ExplosionSound, true);
		}
		
		`log( "avaProjectile.AddDecal" @self @ImpactedActor );

		if( ImpactedActor == None )
		{
			bResetImpactActor = true;
		}

		for(TraceIndex = 0; TraceIndex < 6; ++TraceIndex )
		{
			if( TraceIndex != 2 && TraceIndex != 5 )
			{
				VolumeScaleFactor = 0.5f;
			}
			else
			{
				VolumeScaleFactor = 1.0f;
			}
			
			NewHitLoc = HitLocation;
			if ( ImpactedActor == None )
			{
				//ImpactedActor = Trace(NewHitLoc, HitNormal, (HitLocation + (DecalTraceDirections[TraceIndex] * 64)), HitLocation - (DecalTraceDirections[TraceIndex] * 16), true,, HitInfo);
				ImpactedActor = Trace(NewHitLoc, HitNormal, (HitLocation + (DecalTraceDirections[TraceIndex] * 64)), HitLocation, true,, HitInfo);
			}

			if (ExplosionDecal != None && ImpactedActor != None )
			{
				// FIXME: decal lifespan based on detail level
				LifetimeData = new(ImpactedActor.Outer) class'avaDecalLifetimeDataRound';
				LifetimeData.Round = avaGameReplicationInfo(WorldInfo.GRI).CurrentRound;
				
				if( HitInfo.HitComponent != None )
				{
					`log( "avaProjectile.AddDecal HitComponent" @self @HitInfo.HitComponent @ImpactedActor );
					// changmin
					// static mesh actor에도 volume을 지정하면, 너무 많은 vertex 를 가진 decal이 만들어져서
					// dynamic vertex buffer overflow가 일어납니다.. so.. static mesh는 이렇게 갑니다.. 일단..
					ImpactedActor.AddDecal(
						ExplosionDecal, NewHitLoc, rotator(-HitNormal),
						0, DecalWidth, DecalHeight, 10, false,
						HitInfo.HitComponent, LifetimeData, false, false, '', -1, -1 );
					
					ImpactedActor = ImpactedActor.Trace(NewHitLoc, HitNormal, (NewHitLoc + (DecalTraceDirections[TraceIndex] * 64)), NewHitLoc - (DecalTraceDirections[TraceIndex]*16), true, , HitInfo );
					
					if ( ImpactedActor != None )
					{
						DistanceFromExplosion = ( NewHitLoc - HitLocation ) Dot DecalTraceDirections[TraceIndex];
						
						ImpactedActor.Ava_AddDecal(
							ExplosionDecal, NewHitLoc, rotator(-HitNormal),
							0, DecalWidth, DecalHeight,
							-DistanceFromExplosion, (DecalWidth/2 - DistanceFromExplosion)*VolumeScaleFactor, 
							false, 
							none, LifetimeData, false, false, '', -1, -1 );
					}
				}
				else
				{
					`log( "avaProjectile.AddDecal None HitComponent" @self @ImpactedActor @TraceIndex );

					DistanceFromExplosion = ( NewHitLoc - HitLocation ) Dot DecalTraceDirections[TraceIndex];
					
					ImpactedActor.Ava_AddDecal(
						ExplosionDecal, NewHitLoc, rotator(-HitNormal),
						0, DecalWidth, DecalHeight,
						-DistanceFromExplosion, (DecalWidth/2 - DistanceFromExplosion)*VolumeScaleFactor, 
						false, 
						none, LifetimeData, false, false, '', -1, -1 );
				}
			}
			if( bResetImpactActor )
			{
				ImpactedActor = None;
			}
		}
	}
}

simulated function ShakeView( vector HitLocation )
{
	local avaPlayerController PC;	
	local vector L;
	local rotator R;
	local avaPlayerController.ViewShakeInfo NewExplosionShake;
	foreach WorldInfo.AllControllers(class'avaPlayerController', PC)
	{
		PC.GetPlayerViewPoint( L, R );			
		if (VSize(L-HitLocation) < ShakeRedius )
		{				
			NewExplosionShake = ExplosionShake;
			PC.ShakeView( NewExplosionShake );			
		}		
	}
}

simulated function Flash( vector ExplosionSite, float Radius, float FD )
{
	local avaPlayerController PC;	
	local vector L;
	local rotator R;
	local float dist;
	local Actor HitActor;
	local vector HitLocation, HitNormal;
	local TraceHitInfo HitInfo;	

	local float AdjustedDamage;
	local float FadeTime, HoldTime, Alpha;	

	foreach WorldInfo.AllControllers(class'avaPlayerController', PC)
	{
		PC.GetPlayerViewPoint( L, R );			

		dist = VSize(L-ExplosionSite);
		
		if (dist < Radius)
		{			
			HitActor = Trace(HitLocation, HitNormal, L, Location, false,, HitInfo);			
			
			if (HitActor == None || HitActor == PC.Pawn)
			{
				AdjustedDamage = FClamp( (1 - (dist - FD) / ( Radius - FD ) ), 0, 1 );

				if ((vector(R) dot (L-HitLocation)) < 0)
				{
					FadeTime = AdjustedDamage * FrontFlashParameters.FadeTime;
					HoldTime = AdjustedDamage * FrontFlashParameters.HoldTime;
					Alpha = FrontFlashParameters.Alpha;
				}
				else
				{
					FadeTime = AdjustedDamage * BackFlashParameters.FadeTime;
					HoldTime = AdjustedDamage * BackFlashParameters.HoldTime;
					Alpha = BackFlashParameters.Alpha;
				}
				
				PC.ApplyFlashEffect( HoldTime, FadeTime, Alpha );
			}			
		}		
	}
}

/** ShouldSpawnExplosionLight()
Decide whether or not to create an explosion light for this explosion
*/
simulated function bool ShouldSpawnExplosionLight(vector HitLocation, vector HitNormal)
{
	local PlayerController P;
	local float Dist;

	// decide whether to spawn explosion light
	ForEach LocalPlayerControllers(P)
	{
		Dist = VSize(P.ViewTarget.Location - Location);
		if ( (P.Pawn == Instigator) || (Dist < ExplosionLightClass.Default.Radius) || ((Dist < MaxExplosionLightDistance) && ((vector(P.Rotation) dot (Location - P.ViewTarget.Location)) > 0)) )
		{
			return true;
		}
	}
	return false;
}

/**
 * Clean up
 */
simulated function Shutdown()
{
	local vector HitLocation, HitNormal;

	HitNormal = normal(Velocity * -1);
	Trace(HitLocation,HitNormal,(Location + (HitNormal*-32)), Location + (HitNormal*32),true,vect(0,0,0));

	SetPhysics(PHYS_None);

	if (ProjEffects!=None)
	{	
		ProjEffects.DeactivateSystem();
	}

	if ( ProjExplosion==none && WorldInfo.NetMode != NM_DedicatedServer && (!bSuppressExplosionFX) )
		SpawnExplosionEffects(Location,HitNormal);

	if ( ProjFlightSoundComponent != None )
	{
		ProjFlightSoundComponent.Stop();
		ProjFlightSoundComponent = None;
	}


	HideProjectile();
	SetCollision(false,false);

	// If we have to wait for effects, tweak the death conditions

	if (bWaitForEffects)
	{
		if (bNetTemporary)
		{
			if ( WorldInfo.NetMode == NM_DedicatedServer )
			{
				// We are on a dedicated server and not replicating anything nor do we have effects so destroy right away
				Destroy();
			}
			else
			{
				// We can't die right away but make sure we don't replicate to anyone
				RemoteRole = ROLE_None;
			}
		}
		else
		{
			bTearOff = true;
			if (WorldInfo.NetMode == NM_DedicatedServer)
			{
				LifeSpan=2.0;
			}
		}
	}
	else
	{
		Destroy();
	}
}

// If this actor

event TornOff()
{
	ShutDown();
	Super.TornOff();
}

/**
 * Hide any meshes/etc.
 */
simulated function HideProjectile()
{
	local MeshComponent ComponentIt;
	foreach ComponentList(class'MeshComponent',ComponentIt)
	{
		ComponentIt.SetHidden(true);
	}
}

simulated function Destroyed()
{
	local PlayerController P;
	// Final Failsafe check for explosion effect
	if (ProjExplosion == None && WorldInfo.NetMode != NM_DedicatedServer && !bSuppressExplosionFX)
	{
		SpawnExplosionEffects(Location, vector(Rotation) * -1);
	}

	if ( bIndicating )
	{
		foreach LocalPlayerControllers( P )
		{
			avaHUD( P.myHUD ).RemoveGrenadeForIndicating( self );
		}
	}

	super.Destroyed();
}

simulated function MyOnParticleSystemFinished(ParticleSystemComponent PSC)
{
	//`log( "MyOnParticleSystemFinished" );
	//if (bWaitForEffects && PSC == ProjEffects)
	//{
	//	// it is not safe to destroy the actor here because other threads are doing stuff, so do it next tick
	//	LifeSpan = 0.01;
	//}
}

/** state used only on the client for projectiles with AccelRate > 0 to wait for Velocity to be replicated so we can use it to set Acceleration
 *	the alternative would be to make Velocity repnotify in Actor.uc, but since many Actors (such as Pawns) change their
 *	velocity very frequently, that would have a greater performance hit
 */
state WaitingForVelocity
{
	simulated function Tick(float DeltaTime)
	{
		if (!IsZero(Velocity))
		{
			Acceleration = AccelRate * Normal(Velocity);
			GotoState('Auto');
		}
	}
}

/**
 * Track Stats
 */
function UpdateHitStats(bool bDirectHit,Pawn iActor)
{
	local avaGame GI;
	if (Instigator != None )
	{
		GI = avaGame(WorldInfo.Game);
		if (GI != none && GI.GameStats != None)
		{
			GI.GameStats.WeaponEvent(FiringOwnerStatsID,class<avaWeapon>(weaponBy), FiringWeaponMode, Instigator.PlayerReplicationInfo,'hit',VSize( Instigator.Location - iActor.Location )/16*0.3);
		}
	}
}

function InitStats(avaWeapon InstigatorWeapon)
{
	FiringOwnerStatsID = InstigatorWeapon.OwnerStatsID;
	//FiringWeaponStatsID = InstigatorWeapon.WeaponStatsID;
	FiringWeaponMode = InstigatorWeapon.CurrentFireMode;
}

simulated function bool CalcCamera( float fDeltaTime, out vector out_CamLoc, out rotator out_CamRot, out float out_FOV )
{
	out_CamLoc = Location + (CylinderComponent.CollisionHeight * Vect(0,0,1));
	return true;
}

function Actor GetHomingTarget(avaProjectile Seeker, Controller InstigatedBy) { return self; }

/** called when this Projectile is the ViewTarget of a local player
 * @return the Pawn to use for rendering HUD displays
 */
simulated function Pawn GetPawnOwner();

simulated static function float GetRange()
{
	local float AccelTime;

	if (default.LifeSpan == 0.0)
	{
		return 15000.0;
	}
	else if (default.AccelRate == 0.0)
	{
		return (default.Speed * default.LifeSpan);
	}
	else
	{
		AccelTime = (default.MaxSpeed - default.Speed) / default.AccelRate;
		if (AccelTime < default.LifeSpan)
		{
			return ((0.5 * default.AccelRate * AccelTime * AccelTime) + (default.Speed * AccelTime) + (default.MaxSpeed * (default.LifeSpan - AccelTime)));
		}
		else
		{
			return (0.5 * default.AccelRate * default.LifeSpan * default.LifeSpan) + (default.Speed * default.LifeSpan);
		}
	}
}

simulated function Bool HurtRadiusEx
(
	float				BaseDamage,
	float				InDamageRadius,
	class<DamageType>	DamageType,
	float				Momentum,
	vector				HurtOrigin,
	optional Actor		IgnoredActor,
	optional Controller InstigatedByController = Instigator != None ? Instigator.Controller : None,
	optional bool       bDoFullDamage
)
{
	local Actor	Victim;
	local bool bCausedDamage;

	local actor							HitActor;
	local vector						HitNormal, HitLocation;
	local TraceHitInfo					HitInfo;
	local vector						DestLocation;

	// Prevent HurtRadius() from being reentrant.
	if ( bHurtEntry )
		return false;

	bHurtEntry = true;
	bCausedDamage = false;

	//foreach CollidingActors( class'Actor', Victim, InDamageRadius, Location )
	foreach VisibleCollidingActors( class'Actor', Victim, InDamageRadius, HurtOrigin )
	{
		if ( (Victim == IgnoredActor) || ( Victim == self ) )
			continue;
		
		if ( !TouchingActor( Victim ) )
		{
			if ( Victim != Instigator || bExplodeWhenHitWall == false )
			{
				DestLocation = Victim.Location;
				HitActor =  Trace( HitLocation, HitNormal, DestLocation, location, true, vect(0,0,0), HitInfo, TRACEFLAG_Bullet );

				if ( Victim != HitActor )
				{
					if ( avaPawn( Victim ) != None )
					{
						HitActor =  Trace( HitLocation, HitNormal, DestLocation + class'avaPawn'.Default.MaxStepHeight * vect(0,0,0.5), location, true, vect(0,0,0), HitInfo, TRACEFLAG_Bullet );
						if ( Victim != HitActor )
						{
							HitActor =  Trace( HitLocation, HitNormal, DestLocation + class'avaPawn'.Default.MaxStepHeight * vect(0,0,1), location, true, vect(0,0,0), HitInfo, TRACEFLAG_Bullet );
							if ( Victim != HitActor )
								continue;
						}
					}
					else
					{
						continue;
					}
				}
			}
		}
		Victim.TakeRadiusDamageEx( InstigatedByController, BaseDamage, InDamageRadius, FullDamageMinRadius, DamageType, Momentum, HurtOrigin, bDoFullDamage, weaponBy );
		bCausedDamage = bCausedDamage || Victim.bProjTarget;
	}
	bHurtEntry = false;
	return bCausedDamage;
}

/* HurtRadius()
 Hurt locally authoritative actors within the radius.
*/
simulated function bool HurtRadius( float DamageAmount, 
								    float InDamageRadius, 
                                    class<DamageType> DamageType, 
									float Momentum, 
									vector HurtOrigin,
									optional actor IgnoredActor, 
									optional Controller InstigatedByController = Instigator != None ? Instigator.Controller : None,
									optional bool bDoFullDamage
									)
{
	local bool bCausedDamage, bResult;

	if ( bHurtEntry )	return false;

	bCausedDamage = false;
	if (InstigatedByController == None)
		InstigatedByController = InstigatorController;

	// if ImpactedActor is set, we actually want to give it full damage, and then let him be ignored by super.HurtRadius()
	if ( (ImpactedActor != None) && (ImpactedActor != self)  )
	{
		ImpactedActor.TakeRadiusDamageEx( InstigatedByController,
										  DamageAmount,
										  InDamageRadius,
										  FullDamageMinRadius,
										  DamageType,
										  Momentum,
										  HurtOrigin,
										  true,
										  weaponBy );
		bCausedDamage = ImpactedActor.bProjTarget;
	}


	bResult = HurtRadiusEx(DamageAmount, InDamageRadius, DamageType, Momentum, HurtOrigin, ImpactedActor, InstigatedByController, bDoFullDamage);
	if ( bResult || bCausedDamage )
	{
		if ( ImpactedActor != None && Pawn(ImpactedActor) != None )
		{
			UpdateHitStats(bCausedDamage,Pawn(ImpactedActor));
		}
		return true;
	}
	return false;
}

/**
 * Explode when the projectile comes to rest on the floor.  It's called from the native physics processing functions.  By default,
 * when we hit the floor, we just explode.
 */
simulated event Landed( vector HitNormal, actor FloorActor )
{
	super.Landed(HitNormal, FloorActor);
	HitWallEx( HitNormal, FloorActor, None );
}

// 공중에 떠있으면 이상하잖아....
simulated event HitWallEx(vector HitNormal, Actor Wall, PrimitiveComponent WallComp)
{
	HitWall( HitNormal,  Wall, WallComp);
	if ( Speed < ReflectSleepVelocity )
		SetPhysics(PHYS_None);
}

simulated function ExplodeEx(Actor Wall,vector HitLocation,vector HitNormal)
{
	if (DamageRadius > 0.0)
	{
		Explode( HitLocation, HitNormal );
	}
	else
	{
		Wall.TakeDamage(Damage,InstigatorController,HitLocation,MomentumTransfer * Normal(Velocity), MyDamageType,, weaponBy);
		Shutdown();
	}	
}

simulated event HitWall(vector HitNormal, Actor Wall, PrimitiveComponent WallComp)
{
	local float	TimeSinceLastImpact;

	if ( Wall.IsA( 'avaShatterGlassActor' ) == true )
	{
		Wall.TakeDamage(Damage,InstigatorController,Location,MomentumTransfer * Normal(Velocity), MyDamageType,, weaponBy);
		return;
	}
	else if ( BlockingVolume(Wall) != None )
	{
		if ( BlockingVolume(Wall).bIgnoreProjectile == true )
			return;
	}

	TimeSinceLastImpact = WorldInfo.TimeSeconds - LastImapctTime;
	if ( bExplodeWhenHitWall )
	{
		ExplodeEx(Wall,Location,HitNormal);
	}
	else
	{
		if ( bReflectOffWall == true )
		{
			Velocity = ReflectOffWallDamping * (( Velocity dot HitNormal ) * HitNormal * -2.0 + Velocity);   // Reflect off Wall w/damping
			Speed = VSize(Velocity);


			if( ImpactSoundComponent != None && (Speed > PhysMat.ImpactThreshold) && (TimeSinceLastImpact > PhysMat.ImpactReFireDelay) )
			{
				ImpactSoundComponent.Play();
				LastImapctTime	= WorldInfo.TimeSeconds;
			}
		}
	}
}

defaultproperties
{
	//bRotationFollowsVelocity=true
	bAlwaysRelevant=true
	DamageRadius=+0.0
	TossZ=0.0
	bWaitForEffects=false
	MaxEffectDistance=+10000.0
	MaxExplosionLightDistance=+4000.0
	CheckRadius=0.0
	bBlockedByInstigator=false
	TerminalVelocity=3500.0
	bCollideComplex=FALSE	// Ignore simple collision on StaticMeshes, and collide per poly

	Components.Remove(Sprite)

	Begin Object Name=CollisionCylinder
		CollisionRadius=2
		CollisionHeight=2
	End Object

	Begin Object Class=DynamicLightEnvironmentComponent Name=MyLightEnvironment
		bEnabled=True
	End Object
	Components.Add(MyLightEnvironment)
	LightEnvironment=MyLightEnvironment

	// Add the Mesh
	Begin Object Class=StaticMeshComponent Name=ProjectileMesh
		CollideActors=true
		CastShadow=false
		CullDistanceEx=(Value[0]=7500,Value[1]=7500,Value[2]=7500,Value[3]=7500,Value[4]=7500,Value[5]=7500,Value[6]=7500,Value[7]=7500)
		BlockRigidBody=false
		BlockActors=true
		Scale=1.0
		bUseAsOccluder=false
		bAcceptsDecals=false
		bAcceptsLights=true
		LightEnvironment=MyLightEnvironment
	End Object
	StaticMeshComponent = ProjectileMesh
	Components.Add(ProjectileMesh)
	
	// decal
	DecalTraceDirections[0]=(X=1,Y=0,Z=0)
	DecalTraceDirections[1]=(X=0,Y=1,Z=0)
	DecalTraceDirections[2]=(X=0,Y=0,Z=1)
	DecalTraceDirections[3]=(X=-1,Y=0,Z=0)
	DecalTraceDirections[4]=(X=0,Y=-1,Z=0)
	DecalTraceDirections[5]=(X=0,Y=0,Z=-1)

	bUpdateSimulatedPosition = true
	bSkipActorPropertyReplication = false
	bReplicateMovement				= true
	AdjustInstigatorVelFactor		= 0.3
}
