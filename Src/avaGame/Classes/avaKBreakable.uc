class avaKBreakable extends avaKActor native;

struct native BreakPart
{	
	var() StaticMesh Mesh;	
	var() bool bInteractive;
	var() float Burst;
	var() vector Velocity;
	var() float Lifetime;		
};

var()	int						Health;
var		int						CurrentHealth;
var()	array<BreakPart>		BreakParts;
var()	array<avaKBreakableDynamic>	BreakArcheType;

var()	SoundCue				BreakSound;
var		float					BreakTime;

var hmserialize bool	bDynamicSpawned;	// [!] 20070323 dEAthcURe|HM 'hmserialize'

var		array<avaKActor_Debris>	DebrisActors;

var hmserialize repnotify bool	bBroken;	// [!] 20070323 dEAthcURe|HM 'hmserialize'
var()	bool					bBreakByManual;

var bool						bUsePooling;

cpptext 
{
	#ifdef EnableHostMigration
	virtual void hmSerialize(FArchive& Ar); // 20061213 dEAthcURe|HM
	#endif
}

replication
{
	if (bNetDirty && Role == ROLE_Authority)
		bBroken;
}

native function CreateBreakArcheType();

simulated event ReplicatedEvent(name VarName)
{
	if ( VarName == 'bBroken' && bBroken == true )
	{
		OnBreakAPart();
	}
	else Super.ReplicatedEvent( VarName );
}


simulated event PostBeginPlay()
{
	Super.PostBeginPlay();

	CurrentHealth = Health;
}

simulated function Destroyed()
{
	
	Super.Destroyed();
}

simulated function ClientReset()
{
	local avaKActor_Debris	debris;

	Super.ClientReset();
	if ( bUsePooling )
	{
		foreach DebrisActors( debris )
			debris.Recycle();
	}
	else
	{
		foreach DebrisActors( debris )
			debris.Destroy();
	}
	DebrisActors.length = 0;
}

function BreakAll()
{	
	bBroken = true;

	if (BreakSound != None)
		PlaySound( BreakSound, false, true );

	OnBreakApart();

	// 다시 avaKBreakable 을 만든다..
	CreateBreakArcheType();

	Shutdown();

	bNetDirty = true;

	ActivateEvent( 'BreakAll' );
}

simulated function Client_ShutDown()
{
	Super.Client_ShutDown();
	WakeCollidingActors();
}

simulated function WakeCollidingActors()
{
	local avaKActor	kactor;
	foreach CollidingActors(class'avaKActor',kactor, 256 )
	{
		kactor.StaticMeshComponent.WakeRigidBody();
	}
}

simulated function ShutDown()
{
	Super.ShutDown();
	TriggerEventClass( class'SeqEvent_Destroyed' , self );
	WakeCollidingActors();
}

// {{ 20061213 dEAthcURe|HM
event function HmShutdown()
{
	ShutDown();	
}
// }} 20061213 dEAthcURe|HM

simulated function OnBreakApart()
{
	local int				i;
	local avaKActor_Debris	NewPart;
	local StaticMesh		NewMesh;			
	local float				MaxLifeSpan;


	for (i=0; i<BreakParts.Length; ++i)
	{
		
		if ( bUsePooling == true )
		{
			NewPart = avaGameReplicationInfo(WorldInfo.GRI).ObjectPool.GetDebris(Location,Rotation);
		}
		else
		{
			NewPart = spawn(class'avaKActor_Debris',,,Location,Rotation);		
		}
		NewMesh = BreakParts[i].Mesh;
		NewPart.StaticMeshComponent.SetStaticMesh( NewMesh );
		NewPart.ReplicatedMesh = NewMesh;
		NewPart.StaticMeshComponent.CastShadow				= false;
		NewPart.StaticMeshComponent.bCastDynamicShadow		= false;
		//NewPart.StaticMeshComponent.bAcceptsLights			= StaticMeshComponent.bAcceptsLights;
		//NewPart.StaticMeshComponent.bAcceptsDynamicLights	= StaticMeshComponent.bAcceptsDynamicLights	;
		NewPart.StaticMeshComponent.SetTraceBlocking( false, false );

		NewPart.Initialize( BreakParts[i].bInteractive, BreakParts[i].Velocity, BreakParts[i].Burst, self );		

		// Debris 는 동기화가 안되기 때문에 이렇게 처리하면 안된다...
		if ( bUsePooling == true )
		{
			if (BreakParts[i].Lifetime <= 0)
				NewPart.SetTimer( 1.0, false, 'Recycle' );
			else
				NewPart.SetTimer( BreakParts[i].Lifetime, false, 'Recycle' );
		}
		else
		{
			if (BreakParts[i].Lifetime <= 0)
				NewPart.Lifespan = 1.0;
			else
				NewPart.Lifespan = BreakParts[i].Lifetime;
		}

		if ( MaxLifeSpan < NewPart.LifeSpan )
			MaxLifeSpan = NewPart.LifeSpan;

		DebrisActors[ DebrisActors.length ] = NewPart;

	}		

	

	if ( bDynamicSpawned == true && Role == ROLE_Authority )
	{
		Lifespan = MaxLifeSpan;
	}
}

event TakeDamage( int Damage, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional class<Weapon> weapon )
{
	if (bBroken) return;

	super.TakeDamage( Damage, EventInstigator, HitLocation, Momentum, DamageType, HitInfo, weapon );	

	if ( bBreakByManual == false )
	{
		CurrentHealth -= Damage;

		if (CurrentHealth < 0)
		{
			BreakAll();
		}
	}
}

simulated function Reset()
{
	ClientReset();
	CurrentHealth = Health;

	bBroken = false;
	bNetDirty = true;

	// recover from shut down
	if ( bDynamicSpawned == false )
	{
		RecoverShutDown();
	}
	else
	{
		Destroy();
	}
}

// {{ 20070108 dEAthcURe|HM
event HmReinit()
{
	super.HmReinit();
	//reset(); // 깨진건 깨진거임 복구할필요없3
}
// }} 20070108 dEAthcURe|HM

function ActivateEvent( name EventName )
{
	local	avaSeqEvent_KBreakable	eventKBreakable;
	local	int i;
	for ( i=0 ; i<GeneratedEvents.Length; ++i )
	{
		eventKBreakable = avaSeqEvent_KBreakable( GeneratedEvents[i] );
		if ( eventKBreakable != none )	eventKBreakable.ActivateEvent( EventName );
	}
}

defaultproperties
{
	bBroken = false	
	bDynamicSpawned = false	// bDynamicSpawned 로 맵에서 배치했는지를 판단한다.

	Begin Object Name=StaticMeshComponent0
		bUseAsOccluder		=	FALSE
		CollideActors		=	true
		CastShadow			=	true
		bAcceptsLights		=	true
		BlockRigidBody		=	true
		BlockNonZeroExtent	=	true
		BlockZeroExtent		=	true
		BlockActors			=	true
		bCastDynamicShadow	=	true
		CullDistanceEx=(Value[0]=7500,Value[1]=7500,Value[2]=7500,Value[3]=7500,Value[4]=7500,Value[5]=7500,Value[6]=7500,Value[7]=7500)
		RBChannel			=	RBCC_GameplayPhysics
		RBCollideWithChannels=(Default=TRUE,GameplayPhysics=TRUE,EffectPhysics=FALSE)
	End Object
	Physics	= PHYS_RigidBody
	bCollideActors	=	true
	bBlockActors	=	true

	bUsePooling		=	true

	SupportedEvents.Add(class'avaSeqEvent_KBreakable')
}