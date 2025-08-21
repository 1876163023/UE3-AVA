/*
	Leopard

	2007/07/20 ����
		��ũ GoGoSing������ �� �ִ� ���� ������ ����.
*/
class avaVehicle_Leopard extends avaVehicle
	native;

var protected MaterialInstanceConstant LeftTreadMaterialInstance, RightTreadMaterialInstance;
/** material parameter controlling tread panner speed */
var name TreadSpeedParameterName;

var name LeftBigWheel, LeftSmallWheels[7];
var name RightBigWheel, RightSmallWheels[7];

var float LeftTreadSpeed, RightTreadSpeed;

//! ���׳� �޽�.
var SkeletalMeshComponent AntennaMeshs[2];

//! ����� �޽�.
var SkeletalMeshComponent GunMesh;

/** The Cantilever Beam that is the Antenna itself*/
var avaSkelControl_CantileverBeam AntennaBeamControls[2];

//! �������� ���� �� �ִ� ����.
var array< class<DamageType> >	TakenDamageTypes;

//! MachineGun�� ȸ����.
var repnotify	rotator	GunnerWeaponRotation;
var repnotify	vector	GunnerFlashLocation;

//! ��ũ�� �̵��ӵ� ����.(�̵��߿� �ӵ��� ���ϴ� ���...)
var hmserialize repnotify float	MaxEngineTorque;

//! ��ũ�� ȸ�� �ӵ��� ��ȭ��Ű�� ���.
var repnotify	float	TurnEngineTorqueFactor;

//! Reload Animation�� ���� ����.(avaPawn���� ������)
var repnotify	int		ReloadAnimPlayCount;

// Stop->Go->Died->Rebirth->Go
enum EVehicleState
{
	VHState_None,
	VHState_Died,
	VHState_Stop,
	VHState_Go,
};

//! ���� ���¿� ���缭 Sound�� ����ϱ� ���ؼ� �߰���.
var repnotify EVehicleState	CurrentState;
//! Host/Client ��� ������ State���� ������ �ִ�.
var EVehicleState						LastState;

//!
var avaAnimBlendByEvent	AnimByEvent;

//! ��������� ���� �ð�.
var float				GodModeTime;

//
var AudioComponent		WheelSound;

//
var bool				bActivatedSpawnEvent;

cpptext
{
	virtual void TickSpecial( FLOAT DeltaSeconds );
}

replication
{
	// Variables the server should send ALL clients.
	if ( bNetDirty && Role==ROLE_Authority )
		GunnerWeaponRotation, MaxEngineTorque, TurnEngineTorqueFactor, CurrentState;
	if ( bNetDirty && bNetOwner )
		ReloadAnimPlayCount;
	if ( bNetDirty )
		GunnerFlashLocation;
}

//! clients
simulated function SetMaxEngineTorque(float Torque)
{
	MaxEngineTorque = Torque;
	avaVehicleSimTank(SimObj).MaxEngineTorque = Torque;
}

simulated function SetTurnEngineTorqueFactor(float TorqueFactor)
{
	MaxEngineTorque = TorqueFactor;
	avaVehicleSimTank(SimObj).TurnEngineTorqueFactor = TorqueFactor;
}

simulated event ReplicatedEvent(name VarName)
{
//	`log("avaVehicle_Leopard.ReplicatedEvent" @VarName);

	if ( VarName == 'MaxEngineTorque' )
	{
		avaVehicleSimTank(SimObj).MaxEngineTorque = MaxEngineTorque;
	}
	else if ( VarName == 'TurnEngineTorqueFactor' )
	{
		avaVehicleSimTank(SimObj).TurnEngineTorqueFactor = TurnEngineTorqueFactor;
	}
	else if ( VarName == 'ReloadAnimPlayCount' )
	{
		// PlayReloadAnimation�Լ��� ���� Client���� ó��.
		OnPlayReloadAnimation();
	}
	else if ( VarName == 'CurrentState' )
	{
		PlayVehicleSounds();

		LastState = CurrentState;
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

simulated function Reset()
{
	// recover from shut down
	if ( !bNoDelete )
	{
		Destroy();
	}
}

/** Attach an actor to another one. Kismet action. */
simulated function OnAttachToActor(SeqAct_AttachToActor Action)
{
	local int			idx;
	local Actor			Attachment;
	local Array<Object> ObjVars;

	Action.GetObjectVars(ObjVars,"Attachment");
	for( idx=0; idx<ObjVars.Length && Attachment == None; idx++ )
	{
		Attachment = Actor(ObjVars[idx]);

//		`log("avaVehicle.OnAttachToActor - " @Self @"Attachment =" @Attachment);

		DoKismetAttachment(Actor(ObjVars[idx]), Action);
	}
}


/** Performs actual attachment. Can be subclassed for class specific behaviors. */
simulated function DoKismetAttachment(Actor Attachment, SeqAct_AttachToActor Action)
{
	local bool	bOldCollideActors, bOldBlockActors;

	Attachment.SetHardAttach( Action.bHardAttach );
	Attachment.SetBase( self );
	Attachment.bIgnoreBaseRotation	= Action.bIgnoreBaseRotation;

	if( Action.bUseRelativeOffset || Action.bUseRelativeRotation )
	{
		// disable collision, so we can successfully move the attachment
		bOldCollideActors	= Attachment.bCollideActors;
		bOldBlockActors		= Attachment.bBlockActors;

		Attachment.SetCollision(FALSE, FALSE);

		if( Action.bUseRelativeRotation )
		{
			Attachment.SetRelativeRotation(Action.RelativeRotation);
		}

		// if we're using the offset, place attachment relatively to the target
		if( Action.bUseRelativeOffset )
		{
			//@AVA epic bug fix :)
			//GetAxes(Rotation,X,Y,Z);
			//Attachment.SetRelativeLocation(Action.RelativeOffset.X * X + Action.RelativeOffset.Y * Y + Action.RelativeOffset.Z * Z);
			Attachment.SetRelativeLocation(Action.RelativeOffset);
		}

		// restore previous collision
		Attachment.SetCollision(bOldCollideActors, bOldBlockActors);
	}
}

simulated function ActivateSpawnEvent()
{
	local array<SequenceObject>			Events;
	local avaSeqEvent_VehicleSpawned	SpawnedEvent;
	local int							Idx;

//	`log("avaVehicle.ActivateSpawnEvent()");

	if ( WorldInfo.GetGameSequence() != None )
	{
		WorldInfo.GetGameSequence().FindSeqObjectsByClass(class'avaSeqEvent_VehicleSpawned',TRUE,Events);
		for (Idx = 0; Idx < Events.Length; Idx++)
		{
			SpawnedEvent = avaSeqEvent_VehicleSpawned(Events[Idx]);
			if (SpawnedEvent != None && SpawnedEvent.CheckActivate(self, self))
			{ 
				SpawnedEvent.SpawnedVehicle = Self;
				SpawnedEvent.PopulateLinkedVariableValues();

				bActivatedSpawnEvent = true;

//				`log("avaVehicle.ActivateSpawnEvent() - Success!!");
			}
			else
			{
//				`log("avaVehicle.ActivateSpawnEvent() - Failed!!" @SpawnedEvent);
			}
		}

		// Kismet�� �̺�Ʈ�� ���ٸ� ���̻� ȣ������ �ʴ´�.
		if ( Events.Length == 0 )
		{
			ClearTimer('ActivateSpawnEvent');

//			`log("avaVehicle.ActivateSpawnEvent() - cannot found 'VehicleSpawned' event!" 
//					@WorldInfo @WorldInfo.GetGameSequence());

			return ;
		}
	}

	// ���� Kismet�� SpawnedVehicle �̺�Ʈ�� �߻����� �ʾҴٸ� �߻����� �ش�.
	if ( bActivatedSpawnEvent )
		ClearTimer('ActivateSpawnEvent');
	else
		SetTimer(0.5, false, 'ActivateSpawnEvent');
}

simulated function InitGunRotation()
{
	local int i;

	// Client������ ����ǵ��� �Ѵ�.
	if ( Role != ROLE_Authority )
	{
		// �θ�� ���� ������ �ٶ󺸵��� 
		for (i=0; i< Seats.Length;i++)
			ForceWeaponRotation(i,Rotation);
	}
}

simulated function PostBeginPlay()
{
	// Client�������� �����Ǿ��ٴ� ���� �� �� �ְ� �ȴ�.
	ActivateSpawnEvent();

//	`log("avaVehicle_Leopard.PostBeginPlay - bDriving=" @bDriving @"Driver=" @Driver);

	// �ٷ� ������ ����ǵ��� ���ش�.
	if ( Mesh != None )
	{
		Mesh.WakeRigidBody();
		Mesh.SkelVisibleTime = 1.0f;
	}

	if ( GunMesh != None )
		GunMesh.SkelVisibleTime = 1.0f;

	if (WorldInfo.NetMode != NM_DedicatedServer && Mesh != None)
	{
		// set up material instance (for overlay effects)
		LeftTreadMaterialInstance = Mesh.CreateMaterialInstance(1);
		RightTreadMaterialInstance = Mesh.CreateMaterialInstance(2);

//		`log("Tread: Left" @LeftTreadMaterialInstance @"Right" @RightTreadMaterialInstance);
	}

	// ���׳��� ���Ͽ� �ٿ��ش�.
	Mesh.AttachComponentToSocket(AntennaMeshs[0],'LAntennaSocket');
	Mesh.AttachComponentToSocket(AntennaMeshs[1],'RAntennaSocket');

	// ������� ���δ�.
	Mesh.AttachComponent(GunMesh, 'Bone01');
//	Mesh.AttachComponentToSocket(GunMesh,'GunRootSocket');

	AntennaMeshs[0].SetShadowParent(Mesh);
	AntennaMeshs[0].SetLightEnvironment(Mesh.LightEnvironment);
	AntennaMeshs[1].SetShadowParent(Mesh);
	AntennaMeshs[1].SetLightEnvironment(Mesh.LightEnvironment);
	GunMesh.SetShadowParent(Mesh);
	GunMesh.SetLightEnvironment(Mesh.LightEnvironment);

	AntennaBeamControls[0] = avaSkelControl_CantileverBeam(AntennaMeshs[0].FindSkelControl('Beam'));
//	if(AntennaBeamControls[0] != none)
//		AntennaBeamControls[0].EntireBeamVelocity = GetVelocity;

	AntennaBeamControls[1] = avaSkelControl_CantileverBeam(AntennaMeshs[1].FindSkelControl('Beam'));
//	if(AntennaBeamControls[1] != none)
//		AntennaBeamControls[1].EntireBeamVelocity = GetVelocity;

	// �޽��� ���� Attachment�� �� �Ŀ��� FindSkelControl�Լ��� ����� �۵���.
	// (�׷��� ���� �������� ȣ���ϰ� ��)
	super.PostBeginPlay();

	Seats[0].TurretControllers[0].LagDegreesPerSecond = 10;
	Seats[0].TurretControllers[1].LagDegreesPerSecond = 10;

	// Client������ ȸ������ �ʱ�ȭ ����.
	InitGunRotation();

	// SpawnEvent�� Ȱ��ȭ ���� ���ߴٸ� 0.5�� �Ŀ� �ٽ� �õ��Ѵ�.
	if ( !bActivatedSpawnEvent )
		SetTimer(0.5, false, 'ActivateSpawnEvent');
}

event bool DriverLeave(bool bForceLeave)
{
	local bool bResult;

//	`log("avaVehicle_Leopard.DriverLeave");

	StopMovement();

	bResult = Super.DriverLeave(bForceLeave);

	// ������ ���� 
	if ( Mesh != None )
		Mesh.PutRigidBodyToSleep();

	return bResult;
}

/** tells our pawn to stop moving */
function StopMovement()
{
	if (Physics != PHYS_Flying)
		Acceleration = vect(0,0,0);

	Steering = 0;
	Throttle = 0;
	Rise = 0;

	// ������ �Ǵ°ǰ�??
	SVehicleSimTank(SimObj).LeftTrackVel  = 0;
	SVehicleSimTank(SimObj).RightTrackVel = 0;
	SVehicleSimTank(SimObj).LeftTrackTorque = 0;
	SVehicleSimTank(SimObj).RightTrackTorque = 0;
}

function SetCurrentState( EVehicleState NewState )
{
	if ( NewState == CurrentState )
	{
//		`log("SameState" @CurrentState @NewState);
		return ;
	}

	LastState    = CurrentState;
	CurrentState = NewState;

	PlayVehicleSounds();
}

//! VehicleBot�� �׼�.
function OnVehicleBotEvent(avaSeqAct_VehicleBotEvent action)
{
	local avaVehicleBot				bot;
	local int						i;
	local int						PlayerPawnCount;
	local avaGameReplicationInfo	GRI;
//	local bool						bProcessed;

	bot = avaVehicleBot(Controller);
	if ( bot == None )
	{
		`log("avaVehicle_Leopard.OnVehicleBotEvent - Controller" @Controller);
		return ;
	}

//	bProcessed = false;

	switch(action.Event)
	{
		case VHBot_None:
			break;

		// �����ִ� Vehicle�� �ٽ� �̵��ϵ��� �Ѵ�.
		case VHBot_Go:
			if ( Health <= 0 )
			{
//				`log("failed VHBot_Go. (Health <= 0)");
				bot.OnStop();
				SetCurrentState(VHState_Died);
			}
			else
			{
				bot.OnGo();
				SetCurrentState(VHState_Go);
			}
//			bProcessed = true;
			break;

		// Vehicle�� ���߰� �Ѵ�.
		case VHBot_Stop:
			if ( Health <= 0 )
				SetCurrentState(VHState_Died);
			else
				SetCurrentState(VHState_Stop);

			bot.OnStop();
//			bProcessed = true;
			break;

		// �ִ� ������ũ�� �������ش�.
		case VHBot_MaxEngineTorque:
			if ( avaVehicleSimTank(SimObj) != None )
			{
				SetMaxEngineTorque(action.MaxEngineTorque);
//				bProcessed = true;
			}
			break;

		case VHBot_TurnEngineTorqueFactor:
			if ( avaVehicleSimTank(SimObj) != None )
			{
				avaVehicleSimTank(SimObj).TurnEngineTorqueFactor = action.TurnEngineTorqueFactor;
//				bProcessed = true;
			}
			break;

		case VHBot_GetHealth_DontUse:
//			bProcessed = true;
//			action.MaxHealth = Health;
			break;

		case VHBot_ResetHealth:
//			bProcessed = true;

			if ( action.MaxHealth > 0 )
				Health = action.MaxHealth;
			else if ( ParentFactory != None )
				Health = ParentFactory.MaxHealth;

			// ���� GodModeTime�� ����ִٸ� ���� ������ �ش�.
			if ( action.GodModeTime > 0 )
			{
				GodModeTime = action.GodModeTime;
				SetTimer(GodModeTime, false, 'StopGodMode');
			}

			if ( Health > 0 )
			{
				// GRI�� ��ũ�� ȸ���� ���¶�� ���� �˷��ش�.
				if ( WorldInfo != None && WorldInfo.GRI != None )
				{
					GRI = avaGameReplicationInfo( WorldInfo.GRI );
					if ( GRI != None )
						GRI.SetMissionIndicatorIdx(0);
				}
			}
			break;

		case VHBot_GetPlayerPawnCount:
			// PlayerController�� �����ϴ� Pawn�� ������ �� �־��ش�.
			PlayerPawnCount = 0;
			for ( i = 0; i < Seats.Length; ++i )
				if ( PlayerController( Seats[i].SeatPawn.Controller ) != None )
					PlayerPawnCount++;

			action.PlayerPawnCount = PlayerPawnCount;
//			bProcessed = true;
			break;

		case VHBot_GodMode:
			GodModeTime = action.GodModeTime;
			SetTimer(GodModeTime, false, 'StopGodMode');
//			bProcessed = true;
			break;

		case VHBot_LookAt:
			bot.SetForceViewTarget(action.ViewTarget);
			break;

		case VHBot_Fire:
			// ���� �����ϴ� ����� �ִٸ� ������ �ش�.
			if ( action.ViewTarget != None )
			{
//				`log("avaVehicle_Leopard.OnVehicleBotEvent" @action.ViewTarget @action.FireTime);

				bot.SetForceViewTarget(action.ViewTarget);
				// �߻��ϱ� ���� �ɸ��� �ð�.(1.0 ~ ?? seconds)
				bot.TimedFireWeaponAtViewTarget(action.FireTime);

//				bProcessed = true;
			}
			break;

		case VHBot_Destroy:
			break;
	}

//	if(bProcessed)
//		`log("avaVehicle_Leopard.OnVehicleBotEvent - " @action.Event @" bot = " @bot @Location);
}

//! ������� ���߱�.
simulated function StopGodMode()
{
	GodModeTime = 0;
}

function AdjustDriverDamage(out int Damage, Controller InstigatedBy, Vector HitLocation, out Vector Momentum, class<DamageType> DamageType)
{
//	`log("avaVehicle_Leopard.AdjustDriverDamage" @Damage @InstigatedBy @HitLocation @Momentum @DamageType);

	Super.AdjustDriverDamage(Damage, InstigatedBy, HitLocation, Momentum, DamageType);
}

//! Ư�� ���⿡ ���ؼ��� ���ظ� �Ե��� ���ش�.
simulated event TakeDamage(int Damage, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional class<Weapon> DamageCauser)
{
	local int i;
	local int TakenDamageIndex;
	local avaGameReplicationInfo	GRI;

//	`log("avaVehicle_Leopard.TakeDamage - Driver" @Driver);

	TakenDamageIndex = -1;

	for ( i = 0; i < TakenDamageTypes.Length; ++i )
	{
		// ��ϵ� ������ Ÿ���� �ִ� ��쿡�� ó���ȴ�.
		if ( TakenDamageTypes[i] == DamageType )
		{
			TakenDamageIndex = i;
			break;
		}
	}

	// ���Ǵ� ������ Ÿ���� ���ٸ� ����.
	if ( TakenDamageIndex == -1 )
	{
		`log("cannot match a DamageType." @DamageType @", Health" @Health);
		return ;
	}

	// before we kill the vehicle and everyone inside, eject anyone who has a shieldbelt
	for (i = 1; i < Seats.Length; i++)
	{
		if (Seats[i].SeatPawn != None || Seats[i].StoragePawn != None)
		{
			if( avaPawn(Seats[i].StoragePawn) != None )
			{
//				`log("avaVehicle_Leopard.Died() - ForceKill" @Seats[i].StoragePawn);

				Seats[i].StoragePawn.TakeDamage(Damage, EventInstigator, HitLocation, Momentum, DamageType, HitInfo, DamageCauser);
			}
		}
	}

	// ��������� �������� ���� �ʰ� �Ѵ�.
	if ( GodModeTime > 0 )
		return ;

	// ��ϵ� ������ Ÿ���� �ִ� ��쿡�� ó���ȴ�.
	Super.TakeDamage(Damage, EventInstigator, HitLocation, Momentum, DamageType, HitInfo, DamageCauser);


	if ( Health < default.Health && Health > 0 )
	{
		if ( WorldInfo != None && WorldInfo.GRI != None )
		{
			GRI = avaGameReplicationInfo( WorldInfo.GRI );
			if ( GRI != None )
				GRI.SetMissionIndicatorIdx(2);
		}
	}

}

//! ���� �ʰ� ���ڸ��� ���ߵ��� ���ش�.
function bool Died(Controller Killer, class<DamageType> damageType, vector HitLocation, optional bool bScoreKill = true, optional class<Weapon> weaponBy)
{
	local avaVehicleBot				bot;
	local avaGameReplicationInfo	GRI;
	local Pawn						P;

	if ( Seats[0].SeatPawn != None )
	{
		bot = avaVehicleBot(Seats[0].SeatPawn.Controller);
		if ( bot != None )
		{
/*
			// before we kill the vehicle and everyone inside, eject anyone who has a shieldbelt
			for (i = 1; i < Seats.Length; i++)
			{
				if (Seats[i].SeatPawn != None || Seats[i].StoragePawn != None)
				{
					if( avaPawn(Seats[i].StoragePawn) != None )
					{
						// ��ũ ������ �������
						EjectSeat(i);
					}
				}
			}
*/

/*
			for (i = 1; i < Seats.Length; i++)
			{
				if (Seats[i].SeatPawn != None)
				{
					// kill the WeaponPawn with the appropriate killer, etc for kill credit and death messages
					Seats[i].SeatPawn.Died(Killer, DamageType, HitLocation);
				}
			}
*/
			// bot ó��.
			bot.OnStop();
			SetCurrentState(VHState_Died);

			P = Killer.Pawn;
			if ( P == None )
				P = self;

			// Vehicle�� Died�ߴٴ� �̺�Ʈ�� ������.(�θ� ���丮�� �ִٸ� �θ𿡰�)
			if ( ParentFactory != None )
				ParentFactory.TriggerEventClass(class'avaSeqEvent_VehicleBotEvent', P, 5);
			else
				TriggerEventClass(class'avaSeqEvent_VehicleBotEvent', P, 5);

			// GRI�� ��ũ�� �μ��� ���¶�� ���� �˷��ش�.
			if ( WorldInfo != None && WorldInfo.GRI != None )
			{
				GRI = avaGameReplicationInfo( WorldInfo.GRI );
				if ( GRI != None )
					GRI.SetMissionIndicatorIdx(1);
			}
			return true;
		}
	}

	return Super.Died(Killer, damageType, HitLocation, bScoreKill, weaponBy);
}

/** For Antenna delegate purposes (let's turret motion be more dramatic)*/
function vector GetVelocity()
{
	return velocity;
}

//! �̺�Ʈ�� �ش��ϴ� �ִϸ��̼��� �ϱ� ���� �̸� ã�� ���´�. (from Actor)
simulated event PostInitAnimTree(SkeletalMeshComponent SkelComp)
{
	// ����� �޽��� ���.
	if ( GunMesh == SkelComp )
	{
		AnimByEvent = avaAnimBlendByEvent( GunMesh.Animations.FindAnimNode('AnimByEvent') );
		if ( AnimByEvent == None )
			`warn( "cannot found 'AnimByEvent' in AnimTree" @GunMesh.AnimTreeTemplate );
	}
}

//! �߻��� �̺�Ʈ�� �ش��ϴ� �ִϸ��̼��� �Ѵ�.(avaPawn���� �״�� ������)
simulated function PlayAnimByEvent( EBlendEvent eventType, optional bool bLoop = false )
{
	if ( AnimByEvent != None )
	{
//		`log("avaVehicle_Leopard.PlayAnimByEvent" @eventType @bLoop);
		AnimByEvent.PlayAnimByEvent( eventType, bLoop );
	}
}

//! �߻��� �̺�Ʈ�� �ش��ϴ� �ִϸ��̼��� �����.(avaPawn���� �״�� ������)
simulated function StopAnimByEvent( EBlendEvent eventType )
{
	if ( AnimByEvent != None )
	{
//		`log("avaVehicle_Leopard.StopEvent" @eventType );
		AnimByEvent.StopEvent( eventType );
	}
}

// Server
simulated function PlayReloadAnimation()
{
	// �ڱ��ڽ��� ���� ȣ���ϰ�
	OnPlayReloadAnimation();

	// Client�� BroadCasting�ȴ�.
	ReloadAnimPlayCount++;
}

// Reload Animation�� ���� ó���ϴ� �Լ�.
simulated function OnPlayReloadAnimation()
{
	PlayAnimByEvent( EBT_Reload );
}

//! ���� �ش� �̺�Ʈ�� ���� �ִϸ��̼��� �÷��� ���ΰ�?
simulated function bool IsPlayingAnimByEvent(EBlendEvent eventType)
{
	if( AnimByEvent == None )
		return false;

	return (AnimByEvent.PrvEventType == eventType);
}

//-----------------------------------------------------------------------------
//	PlayAnimByEvent�� StopAnimByEvent�Լ��� ���ؼ� ����ѿ� ���� �ִϸ��̼���
//	ó���ϴ� ��ƾ�̴�.
//-----------------------------------------------------------------------------

simulated function PlayFiring(float Rate, name WeaponFiringMode)
{
//	`log("avaVehicle_Leopard.PlayFiring" @Rate @WeaponFiringMode);

	PlayAnimByEvent( EBT_Fire );

	Super.PlayFiring( Rate, WeaponFiringMode );
}

simulated function StopPlayFiring()
{
//	`log("avaVehicle_Leopard.StopPlayFiring");

	StopAnimByEvent( EBT_Fire );

	Super.StopFiring();
}

/**
 * This function is called when the driver's status has changed.
 */
simulated function DrivingStatusChanged()
{
	// turn parking friction on or off
	bUpdateWheelShapes = true;

	// possibly use different physical material while being driven (to allow properties like friction to change).
	if ( bDriving )
	{
		if ( DrivingPhysicalMaterial != None )
		{
			Mesh.SetPhysMaterialOverride(DrivingPhysicalMaterial);
		}
	}
	else if ( DefaultPhysicalMaterial != None )
	{
		Mesh.SetPhysMaterialOverride(DefaultPhysicalMaterial);
	}

/*
	if ( bDriving && !bIsDisabled )
	{
		VehiclePlayEnterSound();
	}
	else if ( Health > 0 )
	{
		VehiclePlayExitSound();
	}
*/

//	`log("avaVehicle_Leoopard.DrivingStatusChanged " @bDriving @Health);

	bBlocksNavigation = !bDriving;

//	VehicleEvent(bDriving ? 'EngineStart' : 'EngineStop');

	// if the vehicle is being driven and this vehicle should cause view shaking, turn on the timer to check for nearby local players
	if ( bDriving && WorldInfo.NetMode != NM_DedicatedServer && 
		 ProximityShakeRadius > 0.0 && (ProximityShake.OffsetTime > 1.0 || ProximityShake.RotTime > 1.0) )
	{
		SetTimer(0.5, true, 'ProximityShakeTimer');
	}
	else
	{
		ClearTimer('ProximityShakeTimer');
	}
}

simulated function StartWheelSound()
{
	if ( WheelSound != None )
		WheelSound.Play();
}

simulated function StopWheelSound()
{
	if ( WheelSound != None )
		WheelSound.Stop();
}

simulated function PlayVehicleSounds()
{
	// Engine Sound.(with Enter/Exit Sound)
	switch(CurrentState)
	{
		case VHState_Go:
		case VHState_Stop:
			// ����ٰ� �ٽ� ��Ƴ��ٸ� EngineStart/Enter Sound���.
			if ( LastState == VHState_Died )
			{
				VehiclePlayEnterSound();
				VehicleEvent('EngineStart');
			}
			else
			{
				StartEngineSoundTimed();
			}
			break;

		case VHState_Died:
			// ����ִ� ���¿��� �״� �������� EngineStop/Exit Sound���.
			if ( LastState == VHState_Go || LastState == VHState_Stop )
			{
				VehiclePlayExitSound();
				VehicleEvent('EngineStop');
			}
			else
			{
				StopEngineSoundTimed();
			}
			break;
	}

	// Wheel Sound
	switch( CurrentState )
	{
		case VHState_Go:
			StartWheelSound();
			break;

		case VHState_Stop:
		case VHState_Died:
			StopWheelSound();
			break;
	}
}

// {{ 20070801 dEAthcURe|HM
/*
event function OnHmRestore()
{
	Super.OnHmRestore();

	// hmó�� �Ŀ� ���� ���¿� ���� Sound�� �ٽ� ��½��� ��� �Ѵ�.
}*/

// }} 20070801 dEAthcURe|HM

event function OnHmRestore()
{
	// HM�Ŀ� �̵��ӵ� ����.
	SetMaxEngineTorque(MaxEngineTorque);

	Super.OnHmRestore();
}

DefaultProperties
{
	Health=400

	MaxDesireability=0.8
	MomentumMult=2//0.3
	bCanFlip=false
	bTurnInPlace=true
	bCanStrafe=true
	bSeparateTurretFocus=true
	GroundSpeed=520
	MaxSpeed=900
	MaxAngularVelocity=7500
	bUseLookSteer=true
	LeftStickDirDeadZone=0.1

	TeamBeaconOffset=(z=130.0)

	COMOffset=(x=-20.0,y=0.0,z=-30.0)
	InertiaTensorMultiplier=(x=1.0,y=1.0,z=1.0)

	Begin Object Name=MyLightEnvironment
		NumVolumeVisibilitySamples = 8
	End Object

	Begin Object Name=SVehicleMesh
		SkeletalMesh=SkeletalMesh'Avaprop_Trans01.Leopard.SK_Leopard_Renewal'
		AnimTreeTemplate=AnimTree'Avaprop_Trans01.Leopard.SK_Leopard2A6_01_AnimTree'
		PhysicsAsset=PhysicsAsset'Avaprop_Trans01.Leopard.SK_Leopard_Renewal_Physics'
//		RBCollideWithChannels=(Default=TRUE,GameplayPhysics=TRUE,EffectPhysics=TRUE,Vehicle=TRUE,Untitled1=TRUE)
		SeeThroughGroup=1
	End Object

	Begin Object class=SkeletalMeshComponent Name=SkelGunMesh
		SkeletalMesh=SkeletalMesh'Avaprop_Trans01.Leopard_MG3.MS_MG3_Leopard'
		AnimTreeTemplate=AnimTree'Avaprop_Trans01.Leopard.SK_MG3_AnimTree'
		AnimSets(0)=AnimSet'Avaprop_Trans01.Leopard.MG3_Ani'
		PhysicsAsset=PhysicsAsset'Avaprop_Trans01.Leopard.SK_MG3_Physics'
		bForceDiscardRootMotion=true
//		ShadowParent = SVehicleMesh
		SeeThroughGroup=1
	End Object
	GunMesh = SkelGunMesh
//	Components.Add(SkelGunMesh)

	Begin Object class=SkeletalMeshComponent name=SkelAntennaMesh0
		SkeletalMesh=SkeletalMesh'Avaprop_Trans01.Leopard.SK_Leopard2A6_Antenna_Final'
		AnimTreeTemplate=AnimTree'Avaprop_Trans01.Leopard.SK_Leopard2A6_Antenna_AnimTree'
		ShadowParent = SVehicleMesh
		BlockRigidBody=false
		LightEnvironment = MyLightEnvironment
		PhysicsWeight=0.0
		TickGroup=TG_PostASyncWork
		bUseAsOccluder=FALSE
//		CullDistance=1300.0
		CollideActors=false
		bUpdateSkelWhenNotRendered=true
//		bIgnoreControllersWhenNotRendered=true
		SeeThroughGroup=1
	End Object
	AntennaMeshs[0] = SkelAntennaMesh0

	Begin Object class=SkeletalMeshComponent name=SkelAntennaMesh1
		SkeletalMesh=SkeletalMesh'Avaprop_Trans01.Leopard.SK_Leopard2A6_Antenna_Final'
		AnimTreeTemplate=AnimTree'Avaprop_Trans01.Leopard.SK_Leopard2A6_Antenna_AnimTree'
		ShadowParent = SVehicleMesh
		BlockRigidBody=false
		LightEnvironment = MyLightEnvironment		
		PhysicsWeight=0.0
		TickGroup=TG_PostASyncWork
		bUseAsOccluder=FALSE
//		CullDistance=1300.0
		CollideActors=false
		bUpdateSkelWhenNotRendered=true
//		bIgnoreControllersWhenNotRendered=true
		SeeThroughGroup=1
	End Object
	AntennaMeshs[1] = SkelAntennaMesh1

//	DrivingPhysicalMaterial=PhysicalMaterial'Avaprop_Trans01.Leopard.SK_Leopard_Final_PhysMat_Driving'
//	DefaultPhysicalMaterial=PhysicalMaterial'Avaprop_Trans01.Leopard.SK_Leopard_Final_PhysMat'

	TreadSpeedParameterName = "Leopard_Tread_Speed"

	// Wheel
	Begin Object Class=avaVehicleLeopardWheel Name=LWheel1
		BoneName="Wheel_L_01"
		SuspensionTravel=0
		SkelControlName="Wheel_L_01_Ctrl"
		Side=SIDE_Left
	End Object

	Begin Object Class=avaVehicleLeopardWheel Name=LWheel2
		BoneName="Wheel_L_02"
		SkelControlName="Wheel_L_02_Ctrl"
		Side=SIDE_Left
		SteerFactor=1.0
	End Object

	Begin Object Class=avaVehicleLeopardWheel Name=LWheel3
		BoneName="Wheel_L_03"
		SkelControlName="Wheel_L_03_Ctrl"
		Side=SIDE_Left
	End Object

	Begin Object Class=avaVehicleLeopardWheel Name=LWheel4
		BoneName="Wheel_L_04"
		SkelControlName="Wheel_L_04_Ctrl"
		Side=SIDE_Left
	End Object

	Begin Object Class=avaVehicleLeopardWheel Name=LWheel5
		BoneName="Wheel_L_05"
		SkelControlName="Wheel_L_05_Ctrl"
		Side=SIDE_Left
	End Object

	Begin Object Class=avaVehicleLeopardWheel Name=LWheel6
		BoneName="Wheel_L_06"
		SkelControlName="Wheel_L_06_Ctrl"
		Side=SIDE_Left
	End Object

	Begin Object Class=avaVehicleLeopardWheel Name=LWheel7
		BoneName="Wheel_L_07"
		SkelControlName="Wheel_L_07_Ctrl"
		Side=SIDE_Left
	End Object

	Begin Object Class=avaVehicleLeopardWheel Name=LWheel8
		BoneName="Wheel_L_08"
		SkelControlName="Wheel_L_08_Ctrl"
		Side=SIDE_Left
		SteerFactor=1.0
	End Object

	Begin Object Class=avaVehicleLeopardWheel Name=LWheel9
		BoneName="Wheel_L_09"
		SuspensionTravel=0
		SkelControlName="Wheel_L_09_Ctrl"
		Side=SIDE_Left
	End Object

	Begin Object Class=avaVehicleLeopardWheel Name=RWheel1
		BoneName="Wheel_R_01"
		SuspensionTravel=0
		SkelControlName="Wheel_R_01_Ctrl"
		Side=SIDE_Right
	End Object

	Begin Object Class=avaVehicleLeopardWheel Name=RWheel2
		BoneName="Wheel_R_02"
		SkelControlName="Wheel_R_02_Ctrl"
		Side=SIDE_Right
		SteerFactor=1.0
	End Object

	Begin Object Class=avaVehicleLeopardWheel Name=RWheel3
		BoneName="Wheel_R_03"
		SkelControlName="Wheel_R_03_Ctrl"
		Side=SIDE_Right
	End Object

	Begin Object Class=avaVehicleLeopardWheel Name=RWheel4
		BoneName="Wheel_R_04"
		SkelControlName="Wheel_R_04_Ctrl"
		Side=SIDE_Right
	End Object

	Begin Object Class=avaVehicleLeopardWheel Name=RWheel5
		BoneName="Wheel_R_05"
		SkelControlName="Wheel_R_05_Ctrl"
		Side=SIDE_Right
	End Object

	Begin Object Class=avaVehicleLeopardWheel Name=RWheel6
		BoneName="Wheel_R_06"
		SkelControlName="Wheel_R_06_Ctrl"
		Side=SIDE_Right
	End Object

	Begin Object Class=avaVehicleLeopardWheel Name=RWheel7
		BoneName="Wheel_R_07"
		SkelControlName="Wheel_R_07_Ctrl"
		Side=SIDE_Right
	End Object

	Begin Object Class=avaVehicleLeopardWheel Name=RWheel8
		BoneName="Wheel_R_08"
		SkelControlName="Wheel_R_08_Ctrl"
		Side=SIDE_Right
		SteerFactor=1.0
	End Object

	Begin Object Class=avaVehicleLeopardWheel Name=RWheel9
		BoneName="Wheel_R_09"
		SuspensionTravel=0
		SkelControlName="Wheel_R_09_Ctrl"
		Side=SIDE_Right
	End Object

	// Left Wheel
	Wheels(0)=LWheel1
	Wheels(1)=LWheel2
	Wheels(2)=LWheel3
	Wheels(3)=LWheel4
	Wheels(4)=LWheel5
	Wheels(5)=LWheel6
	Wheels(6)=LWheel7
	Wheels(7)=LWheel8
	Wheels(8)=LWheel9

	// Right Wheel
	Wheels(9) =RWheel1
	Wheels(10)=RWheel2
	Wheels(11)=RWheel3
	Wheels(12)=RWheel4
	Wheels(13)=RWheel5
	Wheels(14)=RWheel6
	Wheels(15)=RWheel7
	Wheels(16)=RWheel8
	Wheels(17)=RWheel9

	LeftBigWheel="Wheel_L_01_Ctrl"
	RightBigWheel="Wheel_R_01_Ctrl"

	LeftSmallWheels[0]="Wheel_L_02_Ctrl"
	LeftSmallWheels[1]="Wheel_L_03_Ctrl"
	LeftSmallWheels[2]="Wheel_L_04_Ctrl"
	LeftSmallWheels[3]="Wheel_L_05_Ctrl"
	LeftSmallWheels[4]="Wheel_L_06_Ctrl"
	LeftSmallWheels[5]="Wheel_L_07_Ctrl"
	LeftSmallWheels[6]="Wheel_L_08_Ctrl"

	RightSmallWheels[0]="Wheel_R_02_Ctrl"
	RightSmallWheels[1]="Wheel_R_03_Ctrl"
	RightSmallWheels[2]="Wheel_R_04_Ctrl"
	RightSmallWheels[3]="Wheel_R_05_Ctrl"
	RightSmallWheels[4]="Wheel_R_06_Ctrl"
	RightSmallWheels[5]="Wheel_R_07_Ctrl"
	RightSmallWheels[6]="Wheel_R_08_Ctrl"

/*
	ProximityShakeRadius=600.0
	ProximityShake=(OffsetMag=(X=2.5,Y=0.0,Z=10.0),OffsetRate=(X=35.0,Y=35.0,Z=35.0),OffsetTime=4.0)
*/
	TurnTime=2.0

	LookSteerDeadZone=0.05
	LookSteerSensitivity=3.0

	RespawnTime=2.0

	ViewPitchMin=-13000

	bEjectKilledBodies=false

//	bReducedFallingCollisionDamage=true

//	bFrontalCollision=true
	bNoZDamping=true

	// VehicleBot�� ���� �̺�Ʈ�� ������ �� �ֵ��� �Ѵ�.

	IconCode	=	1
	SupportedEvents.Add(class'avaSeqEvent_VehicleBotEvent')
}