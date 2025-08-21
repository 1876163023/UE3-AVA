/*
	�����ϰ� Vehicle�� �̵��� ó���ϴ� Bot�� ����.

	2007/07/10	����
		avaBot���� ���� �и�.
*/
class avaVehicleBot extends AIController;

var Pawn	LastDriver;
var bool	bFailedToReachTarget;
var bool	bFailedToReachRoute;

//! ��ü Route�� Node���� ����.
var float	ScriptedRouteNavCount;

//! ��ü �Ÿ�.
var float	TotalDistance;

//! ���� �Ÿ�.
var float	CurrentDistance;

//!
var float	SavedWheelSuspensionBias;

//! ������ �ٶ󺸰� �� ���.
var Actor	ForceViewTarget;

//! �̵� ��ġ�� �Ÿ��� ������ �� ����ü.
struct NavCache
{
	var NavigationPoint	NavPoint;		//!< �̵��� ����� ��ġ.
	var float			Distance;		//!< ���� ��ġ���� �̰������� �Ÿ�.
	var Vector			Direction;		//!< �̵��� ����.
};

//! ��ü �̵� ��ο� ���� �� ��ġ�� �Ÿ��� ���� ���� ����.
var array< NavCache > NavCaches;

//! ������ũ �ʱⰪ.
var float	DefaultEngineTorque;

//! ���� ������ũ ���.
var float	EngineTorqueFactor;

//! (from Controller)
event Possess(Pawn inPawn, bool bVehicleTransition)
{
	if (inPawn.Controller != None)
	{
		inPawn.Controller.UnPossess();
	}

	inPawn.PossessedBy(self, bVehicleTransition);
	Pawn = inPawn;
	if (PlayerReplicationInfo != None)
	{
		if (Pawn.IsA('Vehicle') &&
			Vehicle(Pawn).Driver != None)
		{
			PlayerReplicationInfo.bIsFemale = Vehicle(Pawn).Driver.bIsFemale;
		}
		else
		{
			PlayerReplicationInfo.bIsFemale = Pawn.bIsFemale;
		}
	}

	if ( avaVehicle(Pawn) != None )
	{
		// Vehicle�� ��� ù��° ������ ȸ�������� �ٶ󺸵��� ���ش�.
		// (HMó�� �Ŀ� ��ž�� ������ �״�� ������ �ֱ� ���ؼ� �̷��� ��)
		FocalPoint = Pawn.Location + 512*vector(avaVehicle(Pawn).WeaponRotation);
	}
	else
	{
		// preserve Pawn's rotation initially for placed Pawns
		FocalPoint = Pawn.Location + 512*vector(Pawn.Rotation);
	}

	Restart(bVehicleTransition);

	if( Pawn.Weapon == None )
	{
		ClientSwitchToBestWeapon();
	}
}

//! ava���� ����ϴ� GameReplicationInfo�� ��´�.
function avaGameReplicationInfo GetGRI()
{
	if ( WorldInfo != None && WorldInfo.GRI != None )
	{
		if ( avaGameReplicationInfo( WorldInfo.GRI ) != None )
			return avaGameReplicationInfo( WorldInfo.GRI );
	}

	return None;
}

//! avaSeqEvent_VehicleBotEvent�� �̺�Ʈ�� Ȱ��ȭ �����ش�.
function bool ActivateVehicleEvent(int Index)
{
	local avaVehicle V;

	// Vehicle�� ������ �θ� �ִٸ� �װ����� �ѱ��.
	V = avaVehicle(Pawn);
	if ( V != None && V.ParentFactory != None )
		return V.ParentFactory.TriggerEventClass(class'avaSeqEvent_VehicleBotEvent', Pawn, Index);

	return Pawn.TriggerEventClass(class'avaSeqEvent_VehicleBotEvent', Pawn, Index);
}

/*! @brief Route�� PathNode���� ��ü �Ÿ��� �����ش�.
	@note
		ScriptedRoute�� ���� �̸� �����Ǿ� �־�� �Ѵ�.
*/
function float CalcTotalDistance()
{
	local int		i;
	local vector	StartPoint;
	local vector	Delta;
	local float		Distance;
	local float		InvDist;
	local NavCache	Cache;

	if ( ScriptedRoute == None )
	{
		`log("avaVehicle.CalcTotalDistance() - (ScriptedRoute == None)");
		return 0.0;
	}

	// ��ΰ� �ϳ��� ���� ���.
	if ( ScriptedRoute.NavList.Length == 0 || Pawn == None )
		return 0.0;

	// ������ġ�� VehicleFactory���� HM�Ŀ��� ������ ���� ��ġ��� �� �� �ִ�.
	// (����ǥ�� �ڲ� �ڷ� ���� �˰��� HM�Ŀ� �߰���ġ���� ������ġ�� ���Ǵ���..�Ф�)
	if ( avaVehicle(Pawn) != None && avaVehicle(Pawn).ParentFactory != None )
		StartPoint = avaVehicle(Pawn).ParentFactory.Location;
	else
		StartPoint = Pawn.Location;

	for ( i = 0; i < ScriptedRoute.NavList.Length; ++i )
	{
		// ���� ��ġ������ ������ ���ؼ� ���̸� �����ش�.
		Delta    = ScriptedRoute.NavList[i].Nav.Location - StartPoint;
		Distance = Sqrt(Delta.x * Delta.x + Delta.y * Delta.y + Delta.z * Delta.z);
		InvDist  = 1 / Distance;

		// �ش� ��ġ�� �Ÿ��� ����.
		Cache.NavPoint  = ScriptedRoute.NavList[i].Nav;
		Cache.Distance  = Distance;
		Cache.Direction = Delta * InvDist;
		NavCaches[NavCaches.Length] = Cache;

		// ������ ��ġ�� ����.
		StartPoint = ScriptedRoute.NavList[i].Nav.Location;

		TotalDistance += Distance;
	}

	return Distance;
}

/*! @brief Route�� PathNode���� ��ü �Ÿ��߿� ���� ��ġ�� ���� �Ÿ��� �����ش�.
	@note
		CalcTotalDistance()�Լ��� ���� ȣ��Ǿ� NavCaches�� ���� ��ȿ�ؾ� �Ѵ�.
	@remark
		ScriptedRouteIndex���� ����� ���, �̵������� ũ�� ���Ҽ��� 
		CurrentDistance���� ũ�� ��ȭ�ϰ� �ȴ�.

		                   C
		                  /
		                 /
		A----x----------B
			 |<--����-->|
			 (Radius=128)

		������ A->B->C�� �̵��ϴ� ���
		�浹����(Radius=128)�� ���ؼ� x��ġ�� ������ �͵� ������ ������ �����Ǹ�, 
		x->C������ �Ÿ��� ���Ǳ� ������ ���� �۾��� ���� ������ Ŀ����.
*/
function float CalcCurrentDistance()
{
	local vector	NextPoint;
	local vector	Delta;
	local float		Distance;
	local int		i;

	// �̵���ΰ� �ִ� ���.
	if ( NavCaches.Length > 0 )
	{
		// ���� �̵��� ��ġ�� �����ȿ� �ִ� ���.
		if ( ScriptedRouteIndex < NavCaches.Length )
		{
			// ������ �̵��� ��ġ���� �Ÿ��� �����ش�.
			NextPoint = NavCaches[ScriptedRouteIndex].NavPoint.Location;
			Delta     = NextPoint - Pawn.Location;
			Distance  = Sqrt(Delta.x * Delta.x + Delta.y * Delta.y + Delta.z * Delta.z);

			// ���� �������� �̵��� ��ġ���� ���� ��ġ�� �� �� ���.
			// (���� ��ġ�� ���� ��ġ���� �� �־��� ���� �ִ�)
//			if ( Distance > NavCaches[ScriptedRouteIndex].Distance )
//				CurrentDistance = 0;
//			else
				CurrentDistance = NavCaches[ScriptedRouteIndex].Distance - Distance;

			// �̵��� ����� �Ÿ����� �����ش�.
			for ( i = 0; i < ScriptedRouteIndex; ++i )
				CurrentDistance += NavCaches[i].Distance;

//			`log("CalcCurrentDistance =" @CurrentDistance @" / " @TotalDistance @"[" @Distance @"/" @NavCaches[ScriptedRouteIndex].Distance @"]");
		}
		else
		{
			// ��ǥ�������� ������ ����̴�.
			CurrentDistance = TotalDistance;
		}
	}
	else
	{
		// �̵������� ����.
		CurrentDistance = 0.0;
	}

	return CurrentDistance;
}

//! �� PathNode������ �Ÿ��� �����.(����׿�)
function LogPathDistance(bool bOnlyCurrentPath=false)
{
/*
	local int i;
	local int last;

	last = NavCaches.Length - 1;

	if ( bOnlyCurrentPath )
	{
		if ( ScriptedRouteIndex < NavCaches.Length )
			`log("[" @ScriptedRouteIndex @"/" @last @"]" @NavCaches[ScriptedRouteIndex].NavPoint 
			@", Distance =" @NavCaches[ScriptedRouteIndex].Distance @"/" @TotalDistance
			@", Current =" @CurrentDistance);
	}
	else
	{
		for ( i = 0; i < NavCaches.Length; ++i )
			`log("[" @i @"/" @last @"]" @NavCaches[i].NavPoint @", Distance =" @NavCaches[i].Distance @"/" @TotalDistance);
	}
*/
}

function UpdateCurrentDistance(bool bClient=false)
{
	local avaGameReplicationInfo GRI;

	GRI = GetGRI();
	if ( GRI != None )
	{
		if( bClient )
			GRI.ClientSetCurrentMissionTime( CurrentDistance );
		else
			GRI.SetCurrentMissionTime( CurrentDistance );
	}
}

/*! @brief avaPathNode.bCheckPoint�� ��� GRI�� üũ ����Ʈ�� ����� �ش�.
	@note
		CalcTotalDistance()�Լ��� ���� ȣ��Ǿ� NavCaches�� ���� ��ȿ�ؾ� �Ѵ�.
*/
function RegisterCheckPoints( )
{
	local int						i;
	local avaPathNode				Node;
	local float						Distance;
	local avaGameReplicationInfo	GRI;

	GRI = GetGRI();
	if ( GRI == None )
	{
		`log("avaVehicleBot.RegisterCheckPoinnts - (WorldInfo.GRI == None)");
		return ;
	}

	Distance = 0;

	for ( i = 0; i < NavCaches.Length; ++i )
	{
		// �����ؼ� ���̸� ��´�.
		Distance += NavCaches[i].Distance;

		Node = avaPathNode( NavCaches[i].NavPoint );
		if ( Node != None )
		{
			// ���� üũ ����Ʈ���, ���� �Ÿ��� �߰��� �ش�.
			if ( Node.bCheckPoint )
				GRI.AddMissionCheckPoint( Distance );
		}
	}
}

/*! @brief
		���� �����̿� �ִ� PathNode��  ã���ش�.
	@return
		���н� -1����.
*/
function int FindTargetIndex()
{
	local int						i;
	local float						DistSq;
	local float						MinDistSq;
	local int						Index;
	local Vector					PawnDir;
	local float						PawnDot;

	Index     = -1;
	DistSq    = 0;
	MinDistSq = VSizeSq2D(NavCaches[0].NavPoint.Location - NavCaches[NavCaches.Length - 1].NavPoint.Location);

	for ( i = 0; i < NavCaches.Length; ++i )
	{
		// ���� ������ �ִ� PathNode�� ã�´�.
		DistSq = VSizeSq2D(Pawn.Location - NavCaches[i].NavPoint.Location);
		if ( DistSq < MinDistSq )
		{
			MinDistSq = DistSq;
			Index = i;
		}
	}

	if ( Index != -1 )
	{
		PawnDir = Normal(Pawn.Location - NavCaches[Index].NavPoint.Location);
		PawnDot = NavCaches[Index].Direction Dot PawnDir;

		/*
				NavPoint
					o Pawn1
					|
			dir --->*   o Pawn2
					|
			Pawn3 o |

		*/
		// ���� �̵��� ��� ��ġ�� �����߰ų� �������� ���.
		// (���� �׸����� Pawn1 or Pawn2�� ��ġ�� �ִ� ���)
		if ( PawnDot >= 0 )
		{
			// ������ ��尡 �ƴ϶�� ���� ���� �����Ѵ�.
			if ( Index + 1 < NavCaches.Length )
				Index++;
		}
	}

	return Index;
}

//! ���� �̵��� ��尡 �ƴ� ������ Ư�� ��ġ�� �ٶ󺸰� �Ѵ�.
function SetForceViewTarget(Actor ViewTarget)
{
	ForceViewTarget = ViewTarget;

	// ViewTarget�� ���� �ٶ󺸵��� �Ѵ�.
	if(ViewTarget != None )
	{
		ScriptedFocus = ViewTarget;
	}
	// ViewTarget�� None�� ��쿡�� ���� �̵��� ����� �ٶ󺸵��� �Ѵ�.
	else if ( ScriptedRouteIndex < ScriptedRoute.NavList.Length )
	{
		ScriptedFocus = ScriptedRoute.NavList[ScriptedRouteIndex].Nav;
	}
}

//! (from Controller)
function bool FireWeaponAt(Actor A)
{
	if ( A == None )
	{
		`log("avaVehicleBot.FireWeaponAt - Target is None.");
		return false;
	}

	Focus = A;

	if ( Pawn.Weapon != None )
	{
//		`log("avaVehicleBot.FireWeaponAt" @A @Pawn.Weapon @Pawn.Weapon.HasAnyAmmo());

		if ( Pawn.Weapon.HasAnyAmmo() )
			return Pawn.BotFire(false);
	}
	else
	{
//		`log("avaVehicleBot.FireWeaponAt - Pawn.BotFire" @A);
		Pawn.BotFire(false);
	}
}

//! ���� �ð���ŭ ������Ų �Ŀ� FireWeaponAtViewTarget�Լ��� ȣ���Ѵ�.
function TimedFireWeaponAtViewTarget(float delay=1.0)
{
//	`log("avaVehicleBot.TimedFireWeaponAtViewTarget" @delay);

	// ���� ���ư� �ð���ŭ ��ٷȴٰ� ȣ��ǵ��� �Ѵ�.
	SetTimer(delay, false, 'FireWeaponAtViewTarget');
}

//! ViewTarget�� ���ؼ� �����Ѵ�.
function FireWeaponAtViewTarget()
{
//	`log("avaVehicleBot.FireWeaponAtViewTarget" @ForceViewTarget);

	FireWeaponAt(ForceViewTarget);
}

/**
 * Scripting hook to move this AI to a specific actor.
 */
function OnAIMoveToActor(SeqAct_AIMoveToActor Action)
{
	local Actor						DestActor;
	local SeqVar_Object				ObjVar;
	local avaGameReplicationInfo	GRI;
//	local int						NextIndex;

//	`log("avaBot.OnAIMoveToActor:" @Action);

	// abort any previous latent moves
	ClearLatentAction(class'SeqAct_AIMoveToActor',true,Action);

	// set AI focus, if one was specified
	ScriptedFocus = None;
	foreach Action.LinkedVariables(class'SeqVar_Object', ObjVar, "Look At")
	{
		ScriptedFocus = Actor(ObjVar.GetObjectValue());
		if (ScriptedFocus != None)
		{
			break;
		}
	}

	// pick a destination
	foreach Action.LinkedVariables(class'SeqVar_Object', ObjVar, "Destination")
	{
		// use the first valid actor
		DestActor = Actor(ObjVar.GetObjectValue());
		if (DestActor != None)
		{
			break;
		}
	}
	// if we found a valid destination
	if (DestActor != None)
	{
		// set the target and push our movement state
		ScriptedRoute = Route(DestActor);
		if (ScriptedRoute != None)
		{
//			`log("Found ScriptedRoute!");

			if (ScriptedRoute.NavList.length == 0)
			{
				`warn("Invalid route with empty MoveList for scripted move");
			}
			else
			{
				// �� �Ÿ��� ��.
				CalcTotalDistance();

				// GRI�� ��ü �Ÿ��� �����Ѵ�.
				GRI = GetGRI();
				if ( GRI != None )
				{
					GRI.SetMissionMaxTime( TotalDistance );
					// HM�Ŀ� ȣ��� �� �־ �̷��� ����.
					if ( Pawn != None && Pawn.Health > 0 )
					{
						if ( Pawn.Health < Pawn.default.Health )
							GRI.SetMissionIndicatorIdx(2);
						else
							GRI.SetMissionIndicatorIdx(0);
					}
					else
						GRI.SetMissionIndicatorIdx(1);
				}

				// Host�� ��� üũ ����Ʈ�� ����� �ش�.
				if ( Role == ROLE_Authority )
					RegisterCheckPoints();

				// avaRoute�� ��� ���� �ε����� ���� �� �ִ�.
				if ( avaVehicle(Pawn) != None )
				{
					ScriptedRouteIndex = avaVehicle(Pawn).AIScriptedRouteIndex;
//					`log("avaVehicleBot.OnMoveToActor - AIScriptedRouteIndex=" @ScriptedRouteIndex);

					// �⺻���� ����.
					DefaultEngineTorque = avaVehicleSimTank(avaVehicle(Pawn).SimObj).MaxEngineTorque;

					// ���� 0��(HostMigration�� �ƴ�) ���...
					if ( ScriptedRouteIndex == 0 )
					{
						// avaRoute�� ���� �ε����� �ִٸ� �� ������ ������ �ش�.
						if ( avaRoute(ScriptedRoute) != None )
							ScriptedRouteIndex = avaRoute(ScriptedRoute).StartIndex;
					}
					else
					{
						// HostMigration�� ��� 1���� üũ �߰�.
						// HM �� Ȥ�ö� ���� �̵��� ��ġ(Pawn.Location, RBState.Position)�� �Ѿ ���
						// HM �Ŀ� ��ũ�� �ڷ� ���ư����� ������ �߻��� �� �־ �߰�.

/*
						NextIndex = FindTargetIndex();
						// ĳ������ ��ġ�� �̹� ScriptedRouteIndex�� �Ѿ��� ���
						if ( ScriptedRouteIndex < NextIndex && NextIndex != -1 )
						{
							if ( NextIndex - ScriptedRouteIndex > 1 )
								ScriptedRouteIndex = ScriptedRouteIndex + 1;
						}
*/
					}
				}
				else if ( avaRoute(ScriptedRoute) != None )
					ScriptedRouteIndex = avaRoute(ScriptedRoute).StartIndex;
				else
					ScriptedRouteIndex = 0;

				// �Ź� Loop�� �� ������ ���� ��ġ�� ������ �ش�.(HM�� ������ ���ؼ�)
				CalcCurrentDistance();
				// GRI�� ���� �̵��� �Ÿ��� �־��ش�.(HM�� ������ ���ؼ�)
				UpdateCurrentDistance();

				// ��ü ����� �Ÿ��� �α׷� �����.
				LogPathDistance();

				if (!IsInState('ScriptedRouteMove'))
				{
					PushState('ScriptedRouteMove');
				}
			}
		}
		else
		{
//			`log("Found ScriptedMove!");

			ScriptedMoveTarget = DestActor;
			if (!IsInState('ScriptedMove'))
			{
				PushState('ScriptedMove');
			}
		}
	}
	else
	{
		`warn("Invalid destination for scripted move");
	}
}

/** tells our pawn to stop moving */
function StopMovement()
{
	local Vehicle V;

	if (Pawn.Physics != PHYS_Flying)
	{
		Pawn.Acceleration = vect(0,0,0);
	}
	V = Vehicle(Pawn);
	if (V != None)
	{
		V.Steering = 0;
		V.Throttle = 0;
		V.Rise = 0;

		V.Driver.Acceleration = vect(0,0,0);
	}
}

function OnStop()
{
	if ( !IsInState('Stop') )
	{
		PushState('Stop');
	}
	// ���� Health�� 0�� �� ��쿡�� �ٽ� �����ؾ� �Ѵ�.
//	else if ( !bot.IsInState('ScriptedRouteMove') )
//		bot.PushState('ScriptedRouteMove');
}

function OnGo()
{
	if ( IsInState('Stop') )
	{
		PopState();
	}
}

state Stop
{
	event PoppedState()
	{
		local avaVehicleSimTank SimTank;

//		`log("avaVehicleBot.Stop.PoppedState" @Pawn);

//		if ( Vehicle(Pawn) != None )
//			Vehicle(Pawn).Driver.StartDriving( Vehicle(Pawn) );

//		avaVehicle(Pawn).bNoZDamping=false;
//		Pawn.CustomGravityScaling =1.0f;
//		avaVehicle(Pawn).Mesh.WakeRigidBody();

		//avaVehicle(Pawn).bIgnoreForces = false;
//		avaVehicle(Pawn).Driver = LastDriver;

		SimTank = avaVehicleSimTank(avaVehicle(Pawn).SimObj);
		SimTank.WheelSuspensionBias=SavedWheelSuspensionBias;

		avaVehicle(Pawn).SetDriving( true );
		avaVehicle(Pawn).SetPhysics(PHYS_RigidBody);
	}

	event PushedState()
	{
		local avaVehicleSimTank SimTank;

//		`log("avaVehicleBot.Stop.PushedState" @Pawn);

//		if ( Vehicle(Pawn) != None )
//			Vehicle(Pawn).Driver.StopDriving( Vehicle(Pawn) );

//		avaVehicle(Pawn).bNoZDamping=true;
//		Pawn.CustomGravityScaling =0.1f;
//		avaVehicle(Pawn).Mesh.PutRigidBodyToSleep();

		//avaVehicle(Pawn).bIgnoreForces = true;
//		LastDriver = avaVehicle(Pawn).Driver;
//		avaVehicle(Pawn).Driver = None;

		// ������� ���� �Ÿ��� �κ� ����.
		SimTank = avaVehicleSimTank(avaVehicle(Pawn).SimObj);
		SavedWheelSuspensionBias = SimTank.WheelSuspensionBias;
		SimTank.WheelSuspensionBias=0;

		avaVehicle(Pawn).SetDriving( false );
		StopMovement();

//		bPreparingMove = false;
		RouteCache.Length = 0;

		// ScriptedMove���� execMoveToward�� ó�� ȣ�� �� PrepareForMove���� Anchor�� �����Ǵµ�
		// execPollMoveToward�� ȣ��Ǳ� ���� Stop�Ǿ��ٰ� �ٽ� Go�� �� ���,
		// ������ Anchor���� �����Ǿ� �־ PrepareForMove�Լ� ���ο��� Side�ʿ� DynamicAnchor�� �����ؼ�
		// ���� �������� �̵��ϰ� �Ǵ���.
//		Pawn.SetAnchor( NavigationPoint(FindPathToward( Pawn )) );
	}

Begin:
	// Latent ����.
	StopLatentExecution();

	// 2�� �Ŀ� �������� ��� ����!
	Sleep(2);
	avaVehicle(Pawn).SetPhysics(PHYS_None);

//	`log("avaVehicleBot.Completed! waiting...");
}

/**
 * Simple scripted movement state, attempts to pathfind to ScriptedMoveTarget and
 * returns execution to previous state upon either success/failure.
 */
state ScriptedMove
{
	event PoppedState()
	{
//		`log("avaBot.ScriptedMove.PoppedState(), bFailedToReachTarget =" @bFailedToReachTarget);

		// [3] = Reached Target, [4] = Failed to Reach Target
		if (ScriptedRoute != None)
		{
			if ( bFailedToReachTarget )
				ActivateVehicleEvent(4);
			else
				ActivateVehicleEvent(3);
		}

		if (ScriptedRoute == None)
		{
			// if we still have the move target, then finish the latent move
			// otherwise consider it aborted
			ClearLatentAction(class'SeqAct_AIMoveToActor', (ScriptedMoveTarget == None));
		}

		// and clear the scripted move target
		ScriptedMoveTarget = None;

		// ���� �������� �����ϰ� �ٽ� Pop�� ��쿡�� Vehicle�� �����ش�.
		StopMovement();
	}

	event PushedState()
	{
//		`log("avaBot.ScriptedMove.PushedState()");

		if (Pawn != None)
		{
			// make sure the pawn physics are initialized
			Pawn.SetMovementPhysics();
		}

		bFailedToReachTarget = false;
	}

	//! ������ ��ũ�� �ʱⰪ���� ������ �ش�.
	function ResetEngineTorque()
	{
		local avaVehicle	V;

		if ( EngineTorqueFactor == 1.0 )
			return ;

		if ( avaVehicle(Pawn) != None )
		{
			V = avaVehicle(Pawn);
			if ( avaVehicleSimTank(V.SimObj) != None )
				avaVehicleSimTank(V.SimObj).MaxEngineTorque = DefaultEngineTorque;
		}

		EngineTorqueFactor = 1.0;
	}

	//! ������ ��ũ�� �������� �ش�.
	function IncEngineTorque()
	{
		local avaVehicle	V;

		if ( avaVehicle(Pawn) == None )
			return ;

		V = avaVehicle(Pawn);

		if ( avaVehicleSimTank(V.SimObj) != None )
		{
			// �ִ� 10����� ������ ���ش�.
			if ( EngineTorqueFactor < 10 )
				EngineTorqueFactor += 1.0f;

			avaVehicleSimTank(V.SimObj).MaxEngineTorque = DefaultEngineTorque * EngineTorqueFactor;
		}
	}

Begin:
	// while we have a valid pawn and move target, and
	// we haven't reached the target yet
	while (Pawn != None &&
		   !bFailedToReachTarget &&
		   !Pawn.ReachedDestination(ScriptedMoveTarget))
	{
		// �Ź� Loop�� �� ������ ���� ��ġ�� ������ �ش�.
		CalcCurrentDistance();

		// GRI�� ���� �̵��� �Ÿ��� �־��ش�.
		UpdateCurrentDistance();

		// check to see if it is directly reachable
		if ( ActorReachable(ScriptedMoveTarget) )
		{
//			`log("ActorReachable("@ScriptedMoveTarget @")");

//			ResetEngineTorque();

			// then move directly to the actor
			MoveToward(ScriptedMoveTarget, ScriptedFocus, 1);
		}
		else
		{
			// attempt to find a path to the target
			MoveTarget = FindPathToward(ScriptedMoveTarget);
			if (MoveTarget != None)
			{
//				`log("MoveTarget = " @MoveTarget @"ScriptedMoveTarget = " @ScriptedMoveTarget );

//				ResetEngineTorque();

				// move to the first node on the path
				MoveToward(MoveTarget, ScriptedFocus, 0.0);
			}
			else
			{
				// abort the move
				`warn("Failed to find path to"@ScriptedMoveTarget);

				// �̵��� ���ϸ� ��ũ�� n��� ��� �߰��� �ش�.
//				IncEngineTorque();

				MoveToward(ScriptedMoveTarget, ScriptedFocus);

//				ScriptedMoveTarget = None;
//				bFailedToReachTarget = true;
			}
		}
	}

	// return to the previous state
	PopState();
}

/** scripted route movement state, pushes ScriptedMove for each point along the route */
state ScriptedRouteMove
{
	event PoppedState()
	{
//		`log("avaBot.ScriptedRouteMove.PoppedState() = bFailedToReachRoute =" @bFailedToReachRoute
//			@"Route(Current/Total) =" @ScriptedRouteIndex @"/" @ScriptedRouteNavCount);

		// Reset�� ���ؼ� ScriptedRoute�� None�� ��쿡��
		// �̺�Ʈ�� �������� �ʴ´�.
		if ( ScriptedRoute != None )
		{
			// [1] = End Route, [2] = Failed to End Route
			if ( bFailedToReachRoute || ScriptedRouteIndex < ScriptedRouteNavCount )
				ActivateVehicleEvent(2);
			else
				ActivateVehicleEvent(1);
		}

		//  ���� ���� ���� ��쿡�� ����� �ݿ��ϵ��� ����.
		if ( !bFailedToReachRoute )
		{
			// if we still have the move target, then finish the latent move
			// otherwise consider it aborted
			ClearLatentAction(class'SeqAct_AIMoveToActor', (ScriptedRoute == None));
			ScriptedRoute = None;
		}

		// 0.1�ʸ��� CurrentDistance ���� Ÿ�̸� �Լ� ����.
		ClearTimer('UpdateCurrentDistanceForServer');
	}

	event PushedState()
	{
		if ( ScriptedRouteIndex == 0 )
		{
			// [0] = Start Route
			ActivateVehicleEvent(0);
		}

		bFailedToReachRoute = false;

		ScriptedRouteNavCount = ScriptedRoute.NavList.length;

		// 0.1�ʸ��� CurrentDistance ���� Ÿ�̸� �Լ� ����.
		SetTimer(0.1, true, 'UpdateCurrentDistanceForServer');
	}

Begin:
//	`log("avaBot.ScriptedRouteMove:Begin");

	while ( Pawn != None && 
			!bFailedToReachRoute &&
			//ScriptedRoute != None && 
			ScriptedRouteIndex < ScriptedRouteNavCount && ScriptedRouteIndex >= 0)
	{
		// Vehicle�� ��� ���� RouteIndex���� ������ �ش�.(HostMigration���� ���)
		if ( avaVehicle(Pawn) != None )
		{
//			`log("avaVehicleBot - avaVehicle.AIScriptedRouteIndex = " @ScriptedRouteIndex);
			avaVehicle(Pawn).AIScriptedRouteIndex = ScriptedRouteIndex;
		}

		ScriptedMoveTarget = ScriptedRoute.NavList[ScriptedRouteIndex].Nav;
		if (ScriptedMoveTarget != None)
		{
			// ����� ó�ٺ��� ���ش�.
			if ( ForceViewTarget != None )
				ScriptedFocus = ForceViewTarget;
			else
				ScriptedFocus = ScriptedMoveTarget;

//			`log("avaBot.ScriptedRouteMove:Begin" @ScriptedMoveTarget 
//				@" Index=" @ScriptedRouteIndex
//				@" Focus=" @ScriptedFocus);
			PushState('ScriptedMove');
		}

		// ���� PathNode�� �Ÿ��� ���� �α�.
		LogPathDistance(true);

		if (Pawn != None && Pawn.ReachedDestination(ScriptedRoute.NavList[ScriptedRouteIndex].Nav))
		{
			if (bReverseScriptedRoute)
			{
				ScriptedRouteIndex--;
			}
			else
			{
				ScriptedRouteIndex++;
			}
		}
		else
		{
			`warn("Aborting scripted route");
//			ScriptedRoute = None;
			bFailedToReachRoute = true;
			PopState();
		}
	}

	if (Pawn != None && ScriptedRoute != None && ScriptedRouteNavCount > 0)
	{
		switch (ScriptedRoute.RouteType)
		{
			case ERT_Linear:
				PopState();
				break;
			case ERT_Loop:
				bReverseScriptedRoute = !bReverseScriptedRoute;
				// advance index by one to get back into valid range
				if (bReverseScriptedRoute)
				{
					ScriptedRouteIndex--;
				}
				else
				{
					ScriptedRouteIndex++;
				}
				Goto('Begin');
				break;
			case ERT_Circle:
				ScriptedRouteIndex = 0;
				Goto('Begin');
				break;
			default:
				`warn("Unknown route type");
//				ScriptedRoute = None;
				PopState();
				break;
		}
	}
	else
	{
//		ScriptedRoute = None;
		PopState();
	}

	// should never get here
	`warn("Reached end of state execution");
//	ScriptedRoute = None;
	PopState();
}

//! CurrentDistance will get called every tick.
event Tick(float DeltaTime)
{
	// OnAIMoveToActor�Լ��� ȣ��Ǿ� state ScriptedRouteMove�� �������� ��쿡�� �����ϵ��� ���ش�.
	// ���� �̵� ���� ��쿡�� �����Ѵ�.
	if ( IsInState('ScriptedMove') && Pawn.Physics != PHYS_None )
	{
//		`log("eventTick");

		// �Ź� Loop�� �� ������ ���� ��ġ�� ������ �ش�.
		CalcCurrentDistance();

		// GRI�� ���� �̵��� �Ÿ��� �־��ش�.(Local�� ����)
		UpdateCurrentDistance(true);
	}
}

//! 0.1�� �������� ȣ��� �Լ�.(Server�� ȣ��)
function UpdateCurrentDistanceForServer()
{
	// �Ź� Loop�� �� ������ ���� ��ġ�� ������ �ش�.
	CalcCurrentDistance();

	// GRI�� ���� �̵��� �Ÿ��� �־��ش�.(Server:Replication�� ����)
	UpdateCurrentDistance(false);

//	`log("UpdateCurrentDistanceForServer");
}

defaultproperties
{
	// PlayerReplicationInfo�� �����Ϸ��� true���� �Ѵ�.
	bIsPlayer=true

	EngineTorqueFactor=1.0
}