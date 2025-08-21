class avaShatterGlassActor extends Actor
	native
	nativereplication
	placeable;

var() const editconst avaShatterGlassComponent ShatterGlassComponent;
//var() transient repnotify bool AllBroken;
var() hmserialize repnotify int PanelBitmap[8]; 
var int OldPanelBitmap[8];
var() SoundCue BreakSound;
var transient float LastTimeSoundPlayed;
var() float BreakSound_RetriggerTime;
var ParticleSystem Effect;

cpptext
{
	virtual void CheckForErrors();	
	virtual void OnRigidBodyCollision(const FRigidBodyCollisionInfo& Info0, const FRigidBodyCollisionInfo& Info1, const FCollisionImpactData& RigidCollisionData);
	virtual INT* GetOptimizedRepList(BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel);;
}

replication
{
	if (Role == ROLE_Authority)
		PanelBitmap;
}

event PreBeginPlay() {}

event TakeDamage( int Damage, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional class<Weapon> weapon)
{
	ShatterGlassComponent.TakeDamage( Damage, EventInstigator, HitLocation, Momentum, DamageType, HitInfo );

	UpdateEdges();	

	super.TakeDamage( Damage, EventInstigator, HitLocation, Momentum, DamageType, HitInfo, weapon );
}

simulated event ReplicatedEvent(name VarName)
{
	local int	i, diff;		
	local bool	NeedReset;

	//if ( VarName == 'AllBroken' )
	//{
	//	ShatterGlassComponent.BreakAll();
	//}
	//else 
	if (VarName == 'PanelBitmap')
	{
		`log( "avaShatterGlassActor.ReplicatedEvent PanelBitmap" );

		NeedReset = true;
		for ( i=0; i<8 ; ++ i )
		{
			if ( PanelBitmap[i] !=0 )
			{
				NeedReset = false;
			}
		}

		if ( NeedReset == true )
		{
			Reset();
		}
		else
		{
			for (i=0; i<8; ++i)
			{
				diff = PanelBitmap[i] ^ OldPanelBitmap[i];
				if (diff != 0)
				{
					ShatterGlassComponent.BreakBitmap( i, diff );				

					OldPanelBitmap[i] = PanelBitmap[i];
				}
			}		
		}

		ShatterGlassComponent.UpdateEdges();
	}
	//else if (VarName == 'NumEdgeUpdates')
	//{
	//	ShatterGlassComponent.UpdateEdges();
	//}
	else
		super.ReplicatedEvent( VarName );
}

event function Break( int x, int z )
{
	PanelBitmap[ z/2 ] = PanelBitmap[ z/2 ] | (1 << (x + (z&1)*16)) ;				

	NetUpdateTime = WorldInfo.TimeSeconds - 1;

	ShatterGlassComponent.Break( x, z );
}

simulated event function CreateShards( vector HitLocation, vector force, vector forcePos )
{
	local Emitter AnEmitter;

	// if we can show gore then we can show what ever effect is on the physicalMaterial
	AnEmitter = avaGameReplicationInfo(WorldInfo.GRI).ObjectPool.GetImpactEmitter( Effect, HitLocation, rotator(forcePos) );					
	AnEmitter.ParticleSystemComponent.SetLODLevel(avaGameReplicationInfo(WorldInfo.GRI).GetLODLevelToUse(AnEmitter.ParticleSystemComponent.Template, HitLocation) );
	AnEmitter.ParticleSystemComponent.ActivateSystem();
	AnEmitter.ParticleSystemComponent.SetOcclusionGroup(ShatterGlassComponent);
	AnEmitter.SetTimer(3, FALSE, 'HideSelf');	
}

event function BreakAll()
{
	`log( "avaShatterGlassActor.BreakAll" );
	//AllBroken = true;

	NetUpdateTime = WorldInfo.TimeSeconds - 1;

	ShatterGlassComponent.BreakAll();
}

event function UpdateEdges()
{
	//NumEdgeUpdates++;

	//NetUpdateTime = WorldInfo.TimeSeconds - 1;

	ShatterGlassComponent.UpdateEdges();
}

simulated function PostBeginPlay()
{
	super.PostBeginPlay();	
	
	//if (NumEdgeUpdates > 0)
	//{
	//	ShatterGlassComponent.UpdateEdges();
	//}
}

simulated function Tick( float DeltaTime )
{	
	//`log( "Scale "@DrawScale@", Scale3D "@DrawScale3D );

	if (ShatterGlassComponent.bIsBroken)
	{			
		ShatterGlassComponent.GlassTick();
	}
}

simulated function Reset()
{
	local int i;

	super.Reset();

	//`log( "ShatterGlassActor reset" );

	for (i=0; i<8; ++i)
	{
		PanelBitmap[i] = 0;
		OldPanelBitmap[i] = 0;
	}
	
	//AllBroken = false;

	ShatterGlassComponent.Reset();
}

simulated event ClientPlaySound( vector SourceLocation )
{
	if (WorldInfo.TimeSeconds - LastTimeSoundPlayed > BreakSound_RetriggerTime)
	{
		LastTimeSoundPlayed = WorldInfo.TimeSeconds;

		PlaySound( BreakSound, false, true,,, true);		
	}
}

// {{ 20070516 dEAthcURe|HM
event function onHmRestore()
{
	local int i;	
	`log("[dEAthcURe|avaShatterGlassActor|onHmRestore]");
	for (i=0; i<8; ++i) {
		`log("[dEAthcURe|avaShatterGlassActor|onHmRestore] PanelBitmap" @ i @ PanelBitmap[i]);
		if(PanelBitmap[i]!=0) {
			`log("[dEAthcURe|avaShatterGlassActor|onHmRestore] ShatterGlassComponent.BreakBitmap" @ i);
			ShatterGlassComponent.BreakBitmap( i, PanelBitmap[i]);
			OldPanelBitmap[i] = PanelBitmap[i];			
		}
	}
	UpdateEdges();
}
// }} 20070516 dEAthcURe|HM

defaultproperties
{
	Begin Object Class=avaShatterGlassComponent Name=ShatterGlassComponent0		
	End Object
	CollisionComponent=ShatterGlassComponent0
	ShatterGlassComponent=ShatterGlassComponent0
	Components.Add(ShatterGlassComponent0)
	
	Physics=PHYS_Interpolating	

	bEdShouldSnap=true
	bStatic=false
	bCollideActors=true
	bBlockActors=true
	bProjTarget=true
	
	bGameRelevant=true
	bAlwaysRelevant=true
	RemoteRole=ROLE_SimulatedProxy
	bHidden=FALSE	
	bNoDelete=True

	BreakSound=SoundCue'avaGlass.glass_sheet_break1Cue'	
	BreakSound_RetriggerTime=0.1
	Effect=ParticleSystem'avaEffect.Gun_Effect.PS_Glass_Splatter';
}
