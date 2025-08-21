/*
	Vehicle spawner.

	Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved.

	2007/05/04	����
		UTVehicleFactory�� �״�� ������.
*/
class avaVehicleFactory extends NavigationPoint
	abstract
	native
	nativereplication
	placeable;

var()	editconst int		hmId; // 20070723 dEAthcURe|HM

var()	class<avaVehicle>	VehicleClass;		// FIXMESTEVE - this should get changed depending on vehicle set of controlling team
var		avaVehicle			ChildVehicle;

var		float			RespawnProgress;		/** Timer for determining when to spawn vehicles */
var		float			RespawnRateModifier;
var()   bool            bMayReverseSpawnDirection;
var()	bool			bStartNeutral;			/** Not applicable to Onslaught */
var		bool			bHasLockedVehicle;		/** Whether vehicles spawned at this factory are initially team locked */
/** vehicle factory can't be activated while this is set */
var()	bool			bDisabled;
/** if set, replicate ChildVehicle reference */
var bool bReplicateChildVehicle;

var		avaGameObjective	ReverseObjective;		/** Reverse spawn dir if controlled by same team controlling this objective */
var     int             TeamNum;

var		vector			HUDLocation;
var private transient   MaterialInstanceConstant HUDMaterialInstance;

/** This array holds the initial gun rotations for a spawned vehicle. */
var() array<Rotator>	InitialGunRotations;

//! ������ �ı��� �� �ٷ� Respawn
var() bool	bForceRespawnWhenDestoryed;

//! �ִ� ü��.
var() int	MaxHealth;

/** allows setting this vehicle factory to only spawn when one team controls this factory */
var() ETeamType TeamSpawningControl;

//! ���߿뿡���� ����ϴ� �ɼ�.
var() bool bUseCustomSetting;

cpptext
{
	// {{ 20070731 dEAthcURe|HM	
	virtual void PreSave() {
		Super::PreSave();
		hmId = ++GhmId;
	}
	// }} 20070731 dEAthcURe|HM
	
	virtual void CheckForErrors();
	virtual void TickSpecial( FLOAT DeltaSeconds );
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );
}

replication
{
	if (bNetDirty && Role == ROLE_Authority)
		bHasLockedVehicle;
	if (bNetDirty && Role == ROLE_Authority && bReplicateChildVehicle)
		ChildVehicle;
}

function ActivateVehicleFactory()
{
	local avaGameObjective O, Best;
	local float BestDist, NewDist;
//	local avaGame Game;

	if ( avaGame(WorldInfo.Game) != None && !avaGame(WorldInfo.Game).bTeamGame )
		bStartNeutral = true;

	if ( bStartNeutral )
	{
		Activate(255);
	}
	else
	{
		ForEach WorldInfo.AllNavigationPoints(class'avaGameObjective',O)
		{
			NewDist = VSize(Location - O.Location);
			if ( (Best == None) || (NewDist < BestDist) )
			{
				Best = O;
				BestDist = NewDist;
			}
		}

		if ( Best != None )
			Activate(Best.DefenderTeamIndex);
		else
			Activate(255);
	}
}

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	if ( Role == ROLE_Authority )
	{
		if ( avaGame(WorldInfo.Game) != None )
		{
			ActivateVehicleFactory();
		}
		else
		{
			bStartNeutral = true;
			Activate(255);
		}
	}
	else
	{
		AddToClosestObjective();
	}
}

simulated function AddToClosestObjective()
{
	local avaGameObjective O, Best;
	local float Distance, BestDistance;
/*
	if (ONSObjectiveOverride != None)
	{
		Best = ONSObjectiveOverride;
	}
	else if ( UTOnslaughtGame(WorldInfo.Game) != None )
	{
		Best = UTOnslaughtGame(WorldInfo.Game).ClosestObjectiveTo(self);
	}
	else
	{
*/		foreach WorldInfo.AllNavigationPoints(class'avaGameObjective', O)
		{
			Distance = VSize(Location - O.Location);
			if ( (Best == None) || (Distance < BestDistance) )
			{
				BestDistance = Distance;
				Best = O;
			}
		}
//	}

	if ( Best != None )
	{
		Best.VehicleFactories[Best.VehicleFactories.Length] = self;
	}
}

// Called after PostBeginPlay.
//
simulated event SetInitialState()
{
	bScriptInitialized = true;
}

/** function used to update where icon for this actor should be rendered on the HUD
 *  @param NewHUDLocation is a vector whose X and Y components are the X and Y components of this actor's icon's 2D position on the HUD
 */
simulated function SetHUDLocation(vector NewHUDLocation)
{
	HUDLocation = NewHUDLocation;
}
/*
simulated function RenderMapIcon(avaDrawMapPanel MP, UTPlayerController PlayerOwner, LinearColor FinalColor)
{
	if ( !bHasLockedVehicle )
		return;

	if ( HUDMaterialInstance == None )
	{
		HUDMaterialInstance = new(Outer) class'MaterialInstanceConstant';
		HUDMaterialInstance.SetParent(MP.HUDIcons);
	}
	HUDMaterialInstance.SetVectorParameterValue('HUDColor', FinalColor);
	MP.DrawRotatedTile(HUDMaterialInstance, HUDLocation, Rotation.Yaw, VehicleClass.Default.MapSize, VehicleClass.Default.MapSize*MP.Canvas.ClipY/MP.Canvas.ClipX,
					VehicleClass.Default.IconXStart, VehicleClass.Default.IconYStart, VehicleClass.Default.IconXWidth, VehicleClass.Default.IconYWidth);
}
*/

function bool CanActivateForTeam(byte T)
{
	return ( TeamSpawningControl == TEAM_Unknown || TeamSpawningControl == T );
}

function Activate(byte T)
{
	if ( !bDisabled )//&& CanActivateForTeam(T) )
	{
		TeamNum = T;
		GotoState('Active');
	}
}

function Deactivate()
{
	local vector HitLocation, HitNormal;

	GotoState('');
	TeamNum = 255;
	if (ChildVehicle != None && !ChildVehicle.bDeleteMe && ChildVehicle.bTeamLocked)
	{
		if (avaGame(WorldInfo.Game).MatchIsInProgress())
		{
			HitLocation = Location;
			ChildVehicle.Health = -2 * ChildVehicle.Default.Health;
			ChildVehicle.TearOffMomentum = vect(0,0,1);
			TraceComponent(HitLocation, HitNormal, ChildVehicle.Mesh, ChildVehicle.Location, Location);
//			ChildVehicle.Died(None, class'UTDmgType_NodeDestruction', HitLocation);
		}
		else
		{
			ChildVehicle.Destroy();
		}
	}
}

function TarydiumBoost(float Quantity);

/** called when someone starts driving our child vehicle */
function VehicleTaken()
{
	TriggerEventClass(class'avaSeqEvent_VehicleFactory', None, 1);
	bHasLockedVehicle = false;
	// it's possible that someone could enter and immediately exit the vehicle, but if that happens we mark the
	// vehicle as a navigation obstruction and the AI will use that codepath to avoid it, so this extra cost isn't necessary
	ExtraCost = 0;
}

function VehicleDestroyed( avaVehicle V )
{
	TriggerEventClass(class'avaSeqEvent_VehicleFactory', None, 2);

	ChildVehicle = None;
	bHasLockedVehicle = false;
	ExtraCost = 0;
}

simulated function byte GetTeamNum()
{
	return TeamNum;
}

event SpawnVehicle();

//! �����ð����� ������Ų �Ŀ� �Լ��� ȣ���Ѵ�.(2007/09/04)
function TriggerSpawnedEventEx(optional float delay=0.1)
{
	if ( delay <= 0.0 )
		TriggerSpawnedEvent();
	else
		SetTimer(delay, false, 'TriggerSpawnedEvent');
}

function TriggerSpawnedEvent()
{
	TriggerEventClass(class'avaSeqEvent_VehicleFactory', None, 0);
}

function OnToggle(SeqAct_Toggle Action)
{
	local avaGameObjective Objective;

	if (Action.InputLinks[0].bHasImpulse)
	{
		bDisabled = false;
	}
	else if (Action.InputLinks[1].bHasImpulse)
	{
		bDisabled = true;
	}
	else
	{
		bDisabled = !bDisabled;
	}

	if (bDisabled)
	{
		Deactivate();
	}
	else
	{
		// find the objective that owns us and use it to activate us
		foreach WorldInfo.AllNavigationPoints(class'avaGameObjective', Objective)
		{
			if (Objective.VehicleFactories.Find(self) != INDEX_NONE)
			{
				Activate(Objective.GetTeamNum());
				break;
			}
		}

	}
}

state Active
{
	function TarydiumBoost(float Quantity)
	{
		RespawnProgress -= Quantity;
	}

	function VehicleDestroyed( avaVehicle V )
	{
		Global.VehicleDestroyed(V);
		RespawnProgress = VehicleClass.Default.RespawnTime - VehicleClass.Default.SpawnInTime;

		// 0���� Ŀ�� TickSpecial���� ó���ȴ�.
		if ( bForceRespawnWhenDestoryed )
			RespawnProgress = 0.1;

	}

	event SpawnVehicle()
	{
		local Pawn P;
		local bool bIsBlocked;
		local Rotator SpawnRot, TurretRot;
		local int i;

		// �̷��� ��ũ�� ������ �����Ǹ�.... ���� ����°ɱ�...?
		if ( ChildVehicle != None )
			return;

		if ( (ChildVehicle != None) && !ChildVehicle.bDeleteMe )
		{
			return;
		}

		// tell AI to avoid navigating through factories with a vehicle on top of them
		ExtraCost = FMax(ExtraCost,5000);

		foreach CollidingActors(class'Pawn', P, VehicleClass.default.SpawnRadius)
		{
			bIsBlocked = true;
//			if (PlayerController(P.Controller) != None)
//				PlayerController(P.Controller).ReceiveLocalizedMessage(class'avaOnslaughtMessage', 11);
		}

		if (bIsBlocked)
		{
			SetTimer(1.0, false, 'SpawnVehicle'); //try again later
		}
		else
		{
			SpawnRot = Rotation;
			if ( bMayReverseSpawnDirection && (ReverseObjective != None) && (ReverseObjective.DefenderTeamIndex == TeamNum) )
			{
				SpawnRot.Yaw += 32768;
			}
			ChildVehicle = spawn(VehicleClass,,,, SpawnRot);
			if (ChildVehicle != None )
			{				
				ChildVehicle.hmId = hmId;
				// ����� ���� Factory���� �ʱ�ȭ�� ���� �� ������ ���ؼ� ȣ���Ѵ�.
				if ( bUseCustomSetting )
					OnSpawnChildVehicle();

				ChildVehicle.SetTeamNum(TeamNum);
				ChildVehicle.ParentFactory = Self;
				if ( bStartNeutral )
					ChildVehicle.bTeamLocked = false;
				else if ( ChildVehicle.bTeamLocked )
					bHasLockedVehicle = true;
				ChildVehicle.Mesh.WakeRigidBody();

				for (i=0; i<ChildVehicle.Seats.Length;i++)
				{
					if (i < InitialGunRotations.Length)
					{
						TurretRot = InitialGunRotations[i];
						if ( bMayReverseSpawnDirection && (ReverseObjective != None) && (ReverseObjective.DefenderTeamIndex == TeamNum) )
						{
							TurretRot.Yaw += 32768;
						}
					}
					else
					{
						TurretRot = SpawnRot;
					}

					ChildVehicle.ForceWeaponRotation(i,TurretRot);
				}
				if (avaGame(WorldInfo.Game).MatchIsInProgress())
				{
					ChildVehicle.PlaySpawnEffect();
				}

				// HM����� ������ ���� Tick���� ȣ������ �ʵ��� �Ѵ�.
				// (���Ŀ� ������ avaVehicle.OnHmRestore()���� ȣ���ϴ� ��??)
//				SetTimer(0.1, false, 'TriggerSpawnedEvent');

				// HM�� ��쿡�� avaVehicle.OnHmRestore()���� �̺�Ʈ�� �߻����� �ش�.
				// (��ġ ���� ������ ������ ���� ó���ϱ� ���ؼ� �ű�. 2007/09/04)
				if ( WorldInfo.Game.bMigratedHost )
				{
					`log("avaVehicleFactory - WorldInfo.Game.bMigratedHost =" @WorldInfo.Game.bMigratedHost);
				}
				else
				{
					// if gameplay hasn't started yet, we need to wait a bit for everything to be initialized
					if (WorldInfo.bStartup)
					{
						SetTimer(0.1, false, 'TriggerSpawnedEvent');
					}
					else
					{
						TriggerSpawnedEvent();
					}
				}
			}
		}
	}

	function Activate(byte T)
	{
		if (!CanActivateForTeam(T))
		{
			Deactivate();
		}
		else
		{
			TeamNum = T;
			if (ChildVehicle != None)
			{
				// if we have an unused vehicle available, just change its team
				if (ChildVehicle.bTeamLocked)
				{
					ChildVehicle.SetTeamNum(T);
				}
			}
			else
			{
				// force a new vehicle to be spawned
				RespawnProgress = 0.0;
				ClearTimer('SpawnVehicle');
				SpawnVehicle();
			}
		}
	}

	function BeginState(name PreviousStateName)
	{
		if ( avaGame(WorldInfo.Game).MatchIsInProgress() )
		{
			RespawnProgress = VehicleClass.Default.InitialSpawnDelay - VehicleClass.Default.SpawnInTime;
			if (RespawnProgress <= 0.0)
			{
				SpawnVehicle();
			}
		}
		else
		{
			RespawnProgress = 0.0;
			SpawnVehicle();
		}
	}

	function EndState(name NextStateName)
	{
		RespawnProgress = 0.0;
		ClearTimer('SpawnVehicle');
	}
}

// ChildVehicle�� ���� ������ �ش�.
function OnSpawnChildVehicle();

defaultproperties
{
	hmId = -1; // 20070731 dEAthcURe|HM
	Components.Remove(Sprite2)
	GoodSprite=None
	BadSprite=None

	bHidden=true
	bBlockable=true
	bAlwaysRelevant=true
	bSkipActorPropertyReplication=false
	RemoteRole=ROLE_SimulatedProxy
	bStatic=False
	bNoDelete=True
	TeamNum=255
	RespawnRateModifier=1.0
	NetUpdateFrequency=1.0
	bForceRespawnWhenDestoryed=true

	SupportedEvents.Add(class'avaSeqEvent_VehicleFactory')
}

