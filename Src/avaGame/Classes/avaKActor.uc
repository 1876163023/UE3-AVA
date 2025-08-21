//=============================================================================
//  avaKActor
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
// 
//	2006/03/22 by OZ
//		1. reset 과 shutdown 구현을 위해 KActor 로 부터 상속
//=============================================================================
class avaKActor extends KActor
	native;

var vector			init_Location;				//
var rotator			init_Rotation;				//
var RigidBodyState	init_RBState;				//

var float			ClientSleepTime;			//  Server 에서 Data 를 받지 못하면 이 시간 이후에 Sleep 상태로 간다.
var float			ClientSleepRemainingTime;
var bool			bIsLastSleep;				//	이전 Tick 에서 Sleep 이었나 Check

var hmserialize bool	bShutDown;	// [!] 20070323 dEAthcURe|HM 'hmserialize'	//	ShutDown 중임

var hmserialize repnotify BYTE	ShutDownCnt;		// [!] 20070323 dEAthcURe|HM 'hmserialize'	

native final function ResetRBState(RigidBodyState newState, float newAngErrorAccumulator);
native final function RigidBodyState GetRBState();

replication
{
	if ( bNetDirty && Role == ROLE_Authority )
		ShutDownCnt;
}

cpptext
{
	#ifdef EnableHostMigration
	virtual void hmSerialize(FArchive& Ar); // 20061129 dEAthcURe|HM
	#endif
	
	// AActor interface
	virtual void PostLoad();
	virtual void physRigidBody(FLOAT DeltaTime);
}

simulated event PostBeginPlay()
{
	Super.PostBeginPlay();
	init_Location = Location;
	init_Rotation = Rotation;
	init_RBState  = GetRBState();
}

simulated function Client_ShutDown()
{
	bShutDown = true;
	CollisionComponent.SetBlockRigidBody( false );
	if (!bWakeOnLevelStart)	StaticMeshComponent.PutRigidBodyToSleep();
	else					StaticMeshComponent.WakeRigidBody();
}

simulated function ClientReset()
{
	bShutDown = false;
	CollisionComponent.SetBlockRigidBody( true );
	if (!bWakeOnLevelStart)
	{
		StaticMeshComponent.PutRigidBodyToSleep();
	}
	else
	{
		StaticMeshComponent.WakeRigidBody();
	}
}

simulated event ReplicatedEvent(name VarName)
{
	if ( VarName == 'ShutDownCnt' )
	{
		Client_ShutDown();
	}
	else Super.ReplicatedEvent( VarName );	
}

function SetBlockRigidBody( bool bFlag )
{
	CollisionComponent.SetBlockRigidBody( bFlag );
}

simulated function ShutDown()
{
	SetPhysics( PHYS_None );		// Shut down physics
	SetCollision( false, false );	// shut down collision
	SetBlockRigidBody( false );
	SetHidden(True);
	++ShutDownCnt;
	bShutDown = true;
	//bStasis		= true;				// ignore if in a non rendered zone
}

// {{ [+] 20070228 dEAthcURe
event function HmShutdown()
{	
	ShutDown();
}
// }} [+] 20070228 dEAthcURe

simulated function Reset()
{
	// recover from shut down
	if ( bNoDelete )
	{
		RecoverShutDown();
	}
	else
	{
		Destroy();
	}
}

function RecoverShutDown()
{
	SetPhysics( PHYS_None );
	SetLocation( InitialLocation );
	SetRotation( InitialRotation );
	SetCollision( default.bCollideActors, default.bBlockActors );
	SetBlockRigidBody( CollisionComponent.default.BlockRigidBody );
	SetHidden(False);
	bStasis		= default.bStasis;
	
	SetPhysics( default.Physics );
	StaticMeshComponent.SetRBLinearVelocity( Vect(0,0,0) );
	StaticMeshComponent.SetRBAngularVelocity( Vect(0,0,0) );
	StaticMeshComponent.SetRBPosition( InitialLocation );
	StaticMeshComponent.SetRBRotation( InitialRotation );
	// Force replication
	NetUpdateTime = WorldInfo.TimeSeconds - 1;
	//super.Reset();
	//SetLocation( init_Location );
	//SetRotation( init_Rotation );
	ResetRBState( init_RBState, Default.AngErrorAccumulator );
	bShutDown = false;
	if (!bWakeOnLevelStart)
	{
		StaticMeshComponent.PutRigidBodyToSleep();
	}
	else
	{
		StaticMeshComponent.WakeRigidBody();
	}

	// Resolve the RBState and get all of the needed flags set
	ResolveRBState();
}

defaultproperties
{
	Begin Object Name=StaticMeshComponent0
		bAcceptsLights=True		
		bCastHiddenShadow=FALSE
	End Object
	
	ClientSleepTime = 1.0
	bBlocksNavigation = false // AI-Vehicle이 장애물로 인식하지 않도록 하기 위해 Off.(2007/07/27 고광록)
}