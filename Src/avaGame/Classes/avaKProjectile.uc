//=============================================================================
//  KProjectile.
// 
//  Copyright 2006 Ntix Soft, Inc. All Rights Reserved.
//  Projectile 에 Physics 적용을 위해 KActor 로 부터 상속받은 KProjectile Class
//=============================================================================
class avaKProjectile extends avaKActor native;

`include(avaGame/avaGame.uci)

var	float		StartImpulse;				// 초기 Impulse 값

// Damage attributes.
var		float				Damage;
var		float				DamageRadius;
var		float				FullDamageMinRadius;
var		float				MomentumTransfer;		// Momentum magnitude imparted by impacting projectile.
var		class<DamageType>	MyDamageType;

var		Controller			InstigatorController;
var		Actor				ImpactedActor;			// Actor hit or touched by this projectile.  Gets full damage, even if radius effect projectile, and then ignored in HurtRadius

var		Emitter				ProjExplosion;			// The Effects for the explosion 


/** Additional Sounds */
var SoundCue	AmbientSound;				// The sound that is played looped.
var SoundCue	ExplosionSound;				// The sound that is played when it explodes

var	bool bSuppressExplosionFX;				// used to prevent effects when projectiles are destroyed (see LimitationVolume)

/** This is the effect that is played while in flight */
var ParticleSystemComponent	ProjEffects;

/** Effects Template */
var ParticleSystem ProjFlightTemplate;
var ParticleSystem ProjExplosionTemplate;

/** decal for explosion */
var MaterialInstance ExplosionDecal;
var float DecalWidth, DecalHeight;

/** This value sets the cap how far away the explosion effect of this projectile can be seen */
var float		MaxEffectDistance;

/** This holds the stat's ID of the firing weapon */
var int			FiringOwnerStatsID;
//var int			FiringWeaponStatsID;
var int			FiringWeaponMode;

/** collision **/
var float		LastCollisionSoundTime;
var float		CollisionIntervalSecs;
var SoundCue	CollideSound;
var bool		bCheckAutoMessage;

var	class<Weapon>	WeaponBy;

cpptext
{
	#ifdef EnableHostMigration
	virtual void hmSerialize(FArchive& Ar); // 20061207 dEAthcURe|HM
	#endif
	
	/// HL2 style fake hack aerodynamics. Gravity Scales downto 0.5 * original
	virtual FLOAT GetGravityZ();
	//virtual void OnRigidBodyCollision(const FRigidBodyCollisionInfo& Info0, const FRigidBodyCollisionInfo& Info1, const TArray<FRigidBodyContactInfo>& ContactInfos)
	//{
	//	for (INT i=0; i<ContactInfos.Num(); ++i)
	//	{
	//		FVector RelVel = ContactInfos(i).ContactVelocity[1] - ContactInfos(i).ContactVelocity[0];

	//		// Then project along contact normal, and take magnitude.
	//		FLOAT VelMag = Abs(RelVel | ContactInfos(i).ContactNormal); 

	//		if (VelMag > 20)
	//		{
	//			eventPlayCollisionSound( VelMag );

	//			break;
	//		}
	//	}	
	//	
	//	Super::OnRigidBodyCollision( Info0, Info1, ContactInfos );
	//}
}

simulated function PostBeginPlay()
{	
	local vector X, Y, Z;
	local AudioComponent AmbientComponent;
	local float StartVel;
	local avaThrowableWeapon W;

	Super.PostBeginPlay();

	if ( bDeleteMe )	return;

	// Set it's Ambient Sound
	if (AmbientSound != None && WorldInfo.NetMode != NM_DedicatedServer)
	{
		AmbientComponent = CreateAudioComponent(AmbientSound, true, true,,,, AudioChannel_Item );
		AmbientComponent.bShouldRemainActiveIfDropped = true;
	}

	if ( Role == ROLE_Authority )
	{
		if ( Instigator != None )
		{
			InstigatorController = Instigator.Controller;

			GetAxes( Instigator.GetViewRotation(), X, Y, Z );

			W = avaThrowableWeapon( Owner );

			if ( W != None ) 
			{
				StartVel = W.GetProjectileVel();
				X.Z		 += W.GetProjectileZAng();
			}
			else
			{
				StartVel = 1600;
			}

			CollisionComponent.AddImpulse( X * StartVel + Instigator.Velocity,,,true );

			if ( W != None && W.IsProjectileSpin() )
			{
				X.X = 600;
				X.Y = (FRand() - 0.5) * 2 * 1200;
				X.Z = 0;
				CollisionComponent.SetRBAngularVelocity( X );
			}
		}

		if ( bCheckAutoMessage == true )
			SetTimer( 0.5, true, 'CheckAutoMessage' );
	}

	WeaponBy = class<Weapon>(Owner.Class);

	SpawnFlightEffects();
}

function CheckAutoMessage()
{
	local avaPawn	P;
	local int		nCnt;
	foreach VisibleActors( class'avaPawn', P)
	{
		if ( VSize( Location - P.Location ) > `AUTORADIOMSG_DISTANCE )		continue;
		if ( Instigator.GetTeamNum() == P.GetTeamNum() )	continue;
		++nCnt;
		if ( nCnt >= 2 )
		{
			avaPlayerController( P.Controller ).RaiseAutoMessage( AUTOMESSAGE_Grenade, false );
			ClearTimer( 'CheckAutoMessage' );
			return;
		}
	}
}

simulated function bool IsProjectileSpin()
{
	return false;
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
		ProjEffects.SetTemplate(ProjFlightTemplate);
		ProjEffects.OnSystemFinished = MyOnParticleSystemFinished;
	}
}

simulated function MyOnParticleSystemFinished(ParticleSystemComponent PSC)
{
	//if (bWaitForEffects && PSC == ProjEffects)
	//{
	//	// it is not safe to destroy the actor here because other threads are doing stuff, so do it next tick
	//	LifeSpan = 0.01;
	//}
}

/**
 * Explode this Projectile
 */
simulated function Explode(vector HitLocation, vector HitNormal)
{
	if (Damage>0 && DamageRadius>0)
		HurtRadius(Damage,DamageRadius, MyDamageType, MomentumTransfer, HitLocation );

	SpawnExplosionEffects(HitLocation, HitNormal);

	if ( Role == ROLE_Authority )
		MakeNoise(1.0);

	HitNormal = normal(Velocity * -1);
	Trace(HitLocation,HitNormal,(Location + (HitNormal*-32)), Location + (HitNormal*32),true,vect(0,0,0));

	SetPhysics(PHYS_None);

	if (ProjEffects!=None)
	{
		ProjEffects.DeactivateSystem();
	}

	if ( ProjExplosion==none && WorldInfo.NetMode != NM_DedicatedServer && (!bSuppressExplosionFX) )
		SpawnExplosionEffects(Location,HitNormal);

	HideProjectile();
	SetCollision(false,false);

	//Destroy();
	LifeSpan = 1.0;

	// Final Failsafe check for explosion effect
	if (ProjExplosion == None && WorldInfo.NetMode != NM_DedicatedServer && !bSuppressExplosionFX)
	{
		SpawnExplosionEffects(Location, vector(Rotation) * -1);
	}
}


/**
 * Spawn Explosion Effects
 */
simulated function SpawnExplosionEffects(vector HitLocation, vector HitNormal)
{
	local avaDecalLifetimeDataRound LifetimeData;
	local vector NewHitLoc;
	local TraceHitInfo HitInfo;

	// Client Side 에서 TearOff 되어 있는 KActor 들에 Impulse 를 주기 위해서..
	if ( Role < ROLE_Authority && Damage>0 && DamageRadius>0 )
	{
		HurtRadius(Damage,DamageRadius, MyDamageType, MomentumTransfer, HitLocation );
	}

	if (WorldInfo.NetMode != NM_DedicatedServer)
	{
		if (ProjExplosionTemplate != None && EffectIsRelevant(Location, false, MaxEffectDistance))
		{
			ProjExplosion = Spawn(class'avaSpawnedEmitter', self,, HitLocation, rotator(HitNormal));
			if (ProjExplosion != None)
			{
				ProjExplosion.SetTemplate(ProjExplosionTemplate,true);
			}
		}

		if (ExplosionSound != None)
		{			
			PlaySound(ExplosionSound, true,,,HitLocation);
		}

		// Projectile 에서는 ImpactedActor 가 HitWall 된 Actor 이거나 Touch 된 Actor 이다.
		// avaKProjectile 에서는 HitWall 이나 Touch 된 Actor 를 알 수 없기때문에 Trace 를 다시한다.
		if ( ImpactedActor == None )
		{
			ImpactedActor = Trace(NewHitLoc, HitNormal, (HitLocation - (Vect(0,0,1) * 64)), HitLocation + (Vect(0,0,1) * 64), true,, HitInfo);
		}
//		`log( "avaKProjectile.SpawnExplosionEffects" @ExplosionDecal @ImpactedActor @HitLocation @HitNormal );

		/*if (ExplosionDecal != None)
		{
			foreach VisibleCollidingActors( class'Actor', Victim, DecalWidth / 4, HitLocation )
			{
				if (Victim.IsA( 'WorldInfo' ) || Victim.IsA( 'Terrain' ) || Victim.IsA( 'StaticMeshActor' ))
				{
					LifetimeData = new(ImpactedActor.Outer) class'avaDecalLifetimeDataRound';
					LifetimeData.Round = avaGameReplicationInfo(WorldInfo.GRI).CurrentRound;

					Victim.AddDecal(
						ExplosionDecal, HitLocation, rotator(-HitNormal), 
						0, DecalWidth, DecalHeight, 10.0, false, 
						none, LifetimeData, true, false, '' );
				}
				else
				{
					`log( "Visible but not processed... "@Victim );
				}
			}
		}*/

		if (ExplosionDecal != None && ImpactedActor != None)
		{			
			LifetimeData = new(ImpactedActor.Outer) class'avaDecalLifetimeDataRound';
			LifetimeData.Round = avaGameReplicationInfo(WorldInfo.GRI).CurrentRound;

			ImpactedActor.AddDecal(
				ExplosionDecal, HitLocation, rotator(-HitNormal), 
				0, DecalWidth, DecalHeight, 10.0, false, 
				none, LifetimeData, false, false, '', -1, -1 );
		}
	}
}

//simulated event PlayCollisionSound( float VelocityMagnitude )
//{
//	local float volume;
//	local AudioComponent AC;
//	if (WorldInfo.TimeSeconds - LastCollisionSoundTime > CollisionIntervalSecs)
//	{
//		if (VelocityMagnitude > 100)
//			volume = 1;
//		else
//			volume = (VelocityMagnitude-20)/80.0;
//
//		if (CollideSound != None)
//		{
//			AC = WorldInfo.CreateAudioComponent( CollideSound, false, true );
//			AC.bUseOwnerLocation = false;
//			AC.Location = Location;
//			AC.VolumeMultiplier = volume;
//			AC.bAutoDestroy = true;
//			AC.Play();
//		}
//
//		LastCollisionSoundTime = WorldInfo.TimeSeconds;
//	}
//}

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

	// Prevent HurtRadius() from being reentrant.
	if ( bHurtEntry )
		return false;

	bHurtEntry = true;
	bCausedDamage = false;
	foreach VisibleCollidingActors( class'Actor', Victim, InDamageRadius, HurtOrigin )
	{
		if ( (Victim != self) && (Victim != IgnoredActor) )
		{
			Victim.TakeRadiusDamageEx( InstigatedByController, BaseDamage, InDamageRadius, FullDamageMinRadius, DamageType, Momentum, HurtOrigin, bDoFullDamage, WeaponBy );
			bCausedDamage = bCausedDamage || Victim.bProjTarget;
		}
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
										  true, WeaponBy );
		bCausedDamage = ImpactedActor.bProjTarget;
	}


	bResult = HurtRadiusEx(DamageAmount, InDamageRadius, DamageType, Momentum, HurtOrigin, ImpactedActor, InstigatedByController, bDoFullDamage);
	if ( bResult || bCausedDamage )
	{
		if ( ImpactedActor != None && Pawn(ImpactedActor) != None )
			UpdateHitStats(bCausedDamage, Pawn( ImpactedActor ) );
		return true;
	}
	return false;
}

/**
 * Track Stats
 */
function UpdateHitStats(bool bDirectHit, Pawn iActor)
{
	local avaGame GI;

	if (Instigator != None )
	{
		GI = avaGame(WorldInfo.Game);
		if (GI != none && GI.GameStats != None)
		{
			//if (bDirectHit)
			//{
			//	GI.GameStats.WeaponEvent(FiringOwnerStatsID,FiringWeaponStatsID, FiringWeaponMode, Instigator.PlayerReplicationInfo,'directhit', VSize( Instigator.Location - ImpactedActor.Location )/16*0.3  );
			//}
			//else
			//{
			GI.GameStats.WeaponEvent(FiringOwnerStatsID,class<avaWeapon>(weaponBy), FiringWeaponMode, Instigator.PlayerReplicationInfo,'hit',VSize( Instigator.Location - iActor.Location )/16*0.3 );
			//}
		}
	}
}

function InitStats(avaWeapon InstigatorWeapon)
{
	FiringOwnerStatsID = InstigatorWeapon.OwnerStatsID;
	//FiringWeaponStatsID = InstigatorWeapon.WeaponStatsID;
	FiringWeaponMode = InstigatorWeapon.CurrentFireMode;
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
	// Final Failsafe check for explosion effect
	if (ProjExplosion == None && WorldInfo.NetMode != NM_DedicatedServer && !bSuppressExplosionFX)
	{
		SpawnExplosionEffects(Location, vector(Rotation) * -1);
	}

	super.Destroyed();
}

defaultproperties
{
	MaxEffectDistance=+10000.0
	CollisionIntervalSecs=0.3
	bNoDelete=false

	Begin Object Name=StaticMeshComponent0
		bNotifyRigidBodyCollision = TRUE
		CullDistanceEx=(Value[0]=7500,Value[1]=7500,Value[2]=7500,Value[3]=7500,Value[4]=7500,Value[5]=7500,Value[6]=7500,Value[7]=7500)
		bAcceptsDecals=false
	End Object
}


