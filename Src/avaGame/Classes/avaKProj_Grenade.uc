
//=============================================================================
//  avaKProj_Grenade.
// 
//  Copyright 2006 Ntix Soft, Inc. All Rights Reserved.
//
//=============================================================================
class avaKProj_Grenade extends avaKProjectile
	dependson(avaPlayerController);

var float   ExplodeTime;            //  터지는 시간.
var avaPlayerController.ViewShakeInfo ExplosionShake;
var float	ExplosionRadius;

struct FlashParameters
{
	var float HoldTime, FadeTime, Alpha;
};

var float FlashRadius, FlashDamage;
var FlashParameters FrontFlashParameters, BackFlashParameters;

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();
	SetTimer( ExplodeTime, false );
}

simulated function Timer()
{
	local rotator r;
	r = Rotation;
	r.Pitch = 0;
	Explode(Location,vector(r));	
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

simulated function SpawnExplosionEffects(vector HitLocation, vector HitNormal)
{
	local avaPlayerController PC;	
	local vector L;
	local rotator R;
	local avaPlayerController.ViewShakeInfo NewExplosionShake;

	super.SpawnExplosionEffects( HitLocation, HitNormal );

	foreach WorldInfo.AllControllers(class'avaPlayerController', PC)
	{
		PC.GetPlayerViewPoint( L, R );			

		if (VSize(L-HitLocation) < ExplosionRadius)
		{				
			NewExplosionShake = ExplosionShake;

			PC.ShakeView( NewExplosionShake );			
			//PC.ApplyShockMuffleDSP();
			//by killy
		}		
	}

	if (FlashDamage > 0 && FlashRadius > 0)
		Flash( HitLocation, FlashRadius, FlashDamage );
}

defaultproperties
{
//	Components.Remove(StaticMeshComponent0)

	Begin Object Name=StaticMeshComponent0
		StaticMesh=StaticMesh'Wp_M67.MS_M67_3p'
		bNotifyRigidBodyCollision=true
		BlockRigidBody=true
		LightEnvironment=MyLightEnvironment
	End Object
	//CollisionComponent=StaticMeshComponent1
	//StaticMeshComponent=StaticMeshComponent1
	//Components.Add(StaticMeshComponent1)

	//Begin Object Name=StaticMeshComponent0
	//	StaticMesh=StaticMesh'avaChr.grenade'
	//End Object
	//CollisionComponent=StaticMeshComponent0
	
	StartImpulse=10

	ExplodeTime=3.0

	ProjExplosionTemplate=ParticleSystem'avaEffect.Explosion.PS_Grenade_Explosion01'
	//ProjExplosionTemplate	=	ParticleSystem'avaEffect.Explosion.PS_C4_Explosion'

	ExplosionDecal=Material'avaDecal.Crater.M_grenade_Crater'
	Damage				=	140.0
	DamageRadius		=	250
	FullDamageMinRadius	=	100
	MomentumTransfer	=	250000
	//날리는 힘? 속도? by Killy
	MyDamageType=class'avaDmgType_Explosion'
	LifeSpan=0.0
	ExplosionSound=SoundCue'avaWeaponSounds.Grenade_HE.Grenade_HE_Explosion'

	ExplosionShake=(OffsetMag=(X=-5.0,Y=-5.00,Z=-5.00),OffsetRate=(X=-1000.0,Y=-1000.0,Z=-1000.0),OffsetTime=10,RotMag=(X=0.0,Y=0.0,Z=0.0),RotRate=(X=0.0,Y=0.0,Z=0.0),RotTime=2)	
	ExplosionRadius=800

	DecalWidth=400
	DecalHeight=400
	bCheckAutoMessage=true
}