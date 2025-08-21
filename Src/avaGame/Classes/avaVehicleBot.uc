/*
	간단하게 Vehicle를 이동만 처리하는 Bot을 만듬.

	2007/07/10	고광록
		avaBot에서 따로 분리.
*/
class avaVehicleBot extends AIController;

var Pawn	LastDriver;
var bool	bFailedToReachTarget;
var bool	bFailedToReachRoute;

//! 전체 Route의 Node들의 개수.
var float	ScriptedRouteNavCount;

//! 전체 거리.
var float	TotalDistance;

//! 현재 거리.
var float	CurrentDistance;

//!
var float	SavedWheelSuspensionBias;

//! 강제로 바라보게 할 대상.
var Actor	ForceViewTarget;

//! 이동 위치와 거리를 저장해 둘 구조체.
struct NavCache
{
	var NavigationPoint	NavPoint;		//!< 이동할 대상의 위치.
	var float			Distance;		//!< 이전 위치에서 이곳까지의 거리.
	var Vector			Direction;		//!< 이동할 방향.
};

//! 전체 이동 경로에 대한 각 위치와 거리에 대한 값을 저장.
var array< NavCache > NavCaches;

//! 엔진토크 초기값.
var float	DefaultEngineTorque;

//! 현재 엔진토크 계수.
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
		// Vehicle인 경우 첫번째 무기의 회전방향을 바라보도록 해준다.
		// (HM처리 후에 포탑의 방향을 그대로 유지해 주기 위해서 이렇게 함)
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

//! ava에서 사용하는 GameReplicationInfo를 얻는다.
function avaGameReplicationInfo GetGRI()
{
	if ( WorldInfo != None && WorldInfo.GRI != None )
	{
		if ( avaGameReplicationInfo( WorldInfo.GRI ) != None )
			return avaGameReplicationInfo( WorldInfo.GRI );
	}

	return None;
}

//! avaSeqEvent_VehicleBotEvent의 이벤트를 활성화 시켜준다.
function bool ActivateVehicleEvent(int Index)
{
	local avaVehicle V;

	// Vehicle을 생성한 부모가 있다면 그곳으로 넘긴다.
	V = avaVehicle(Pawn);
	if ( V != None && V.ParentFactory != None )
		return V.ParentFactory.TriggerEventClass(class'avaSeqEvent_VehicleBotEvent', Pawn, Index);

	return Pawn.TriggerEventClass(class'avaSeqEvent_VehicleBotEvent', Pawn, Index);
}

/*! @brief Route의 PathNode들의 전체 거리를 구해준다.
	@note
		ScriptedRoute의 값이 미리 설정되어 있어야 한다.
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

	// 경로가 하나도 없는 경우.
	if ( ScriptedRoute.NavList.Length == 0 || Pawn == None )
		return 0.0;

	// 시작위치는 VehicleFactory여야 HM후에도 고정된 시작 위치라고 할 수 있다.
	// (느낌표가 자꾸 뒤로 가서 알고보니 HM후에 중간위치에서 시작위치로 계산되더라..ㅠㅠ)
	if ( avaVehicle(Pawn) != None && avaVehicle(Pawn).ParentFactory != None )
		StartPoint = avaVehicle(Pawn).ParentFactory.Location;
	else
		StartPoint = Pawn.Location;

	for ( i = 0; i < ScriptedRoute.NavList.Length; ++i )
	{
		// 다음 위치까지의 변위를 구해서 길이를 더해준다.
		Delta    = ScriptedRoute.NavList[i].Nav.Location - StartPoint;
		Distance = Sqrt(Delta.x * Delta.x + Delta.y * Delta.y + Delta.z * Delta.z);
		InvDist  = 1 / Distance;

		// 해당 위치와 거리를 저장.
		Cache.NavPoint  = ScriptedRoute.NavList[i].Nav;
		Cache.Distance  = Distance;
		Cache.Direction = Delta * InvDist;
		NavCaches[NavCaches.Length] = Cache;

		// 마지막 위치를 저장.
		StartPoint = ScriptedRoute.NavList[i].Nav.Location;

		TotalDistance += Distance;
	}

	return Distance;
}

/*! @brief Route의 PathNode들의 전체 거리중에 현재 위치에 대한 거리를 구해준다.
	@note
		CalcTotalDistance()함수가 먼저 호출되어 NavCaches의 값이 유효해야 한다.
	@remark
		ScriptedRouteIndex값이 변경될 경우, 이동방향이 크게 변할수록 
		CurrentDistance값이 크게 변화하게 된다.

		                   C
		                  /
		                 /
		A----x----------B
			 |<--오차-->|
			 (Radius=128)

		위에서 A->B->C로 이동하는 경우
		충돌범위(Radius=128)에 의해서 x위치에 까지만 와도 도착된 것으로 판정되며, 
		x->C까지의 거리로 계산되기 때문에 각이 작아질 수록 오차가 커진다.
*/
function float CalcCurrentDistance()
{
	local vector	NextPoint;
	local vector	Delta;
	local float		Distance;
	local int		i;

	// 이동경로가 있는 경우.
	if ( NavCaches.Length > 0 )
	{
		// 다음 이동할 위치가 범위안에 있는 경우.
		if ( ScriptedRouteIndex < NavCaches.Length )
		{
			// 다음에 이동할 위치와의 거리를 구해준다.
			NextPoint = NavCaches[ScriptedRouteIndex].NavPoint.Location;
			Delta     = NextPoint - Pawn.Location;
			Distance  = Sqrt(Delta.x * Delta.x + Delta.y * Delta.y + Delta.z * Delta.z);

			// 원래 다음으로 이동할 위치보다 현재 위치가 더 먼 경우.
			// (현재 위치가 원래 위치보다 더 멀어질 수도 있다)
//			if ( Distance > NavCaches[ScriptedRouteIndex].Distance )
//				CurrentDistance = 0;
//			else
				CurrentDistance = NavCaches[ScriptedRouteIndex].Distance - Distance;

			// 이동한 경로의 거리들을 더해준다.
			for ( i = 0; i < ScriptedRouteIndex; ++i )
				CurrentDistance += NavCaches[i].Distance;

//			`log("CalcCurrentDistance =" @CurrentDistance @" / " @TotalDistance @"[" @Distance @"/" @NavCaches[ScriptedRouteIndex].Distance @"]");
		}
		else
		{
			// 목표지점까지 도착한 경우이다.
			CurrentDistance = TotalDistance;
		}
	}
	else
	{
		// 이동정보가 없다.
		CurrentDistance = 0.0;
	}

	return CurrentDistance;
}

//! 각 PathNode까지의 거리를 출력함.(디버그용)
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

/*! @brief avaPathNode.bCheckPoint의 경우 GRI에 체크 포인트를 등록해 준다.
	@note
		CalcTotalDistance()함수가 먼저 호출되어 NavCaches의 값이 유효해야 한다.
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
		// 누적해서 길이를 얻는다.
		Distance += NavCaches[i].Distance;

		Node = avaPathNode( NavCaches[i].NavPoint );
		if ( Node != None )
		{
			// 만약 체크 포인트라면, 현재 거리를 추가해 준다.
			if ( Node.bCheckPoint )
				GRI.AddMissionCheckPoint( Distance );
		}
	}
}

/*! @brief
		가장 가까이에 있는 PathNode를  찾아준다.
	@return
		실패시 -1리턴.
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
		// 가장 가까이 있는 PathNode를 찾는다.
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
		// 다음 이동할 대상 위치에 도착했거나 지나쳤을 경우.
		// (위의 그림에서 Pawn1 or Pawn2의 위치에 있는 경우)
		if ( PawnDot >= 0 )
		{
			// 마지막 노드가 아니라면 다음 노드로 설정한다.
			if ( Index + 1 < NavCaches.Length )
				Index++;
		}
	}

	return Index;
}

//! 다음 이동할 노드가 아닌 강제로 특정 위치를 바라보게 한다.
function SetForceViewTarget(Actor ViewTarget)
{
	ForceViewTarget = ViewTarget;

	// ViewTarget을 향해 바라보도록 한다.
	if(ViewTarget != None )
	{
		ScriptedFocus = ViewTarget;
	}
	// ViewTarget이 None인 경우에는 현재 이동할 대상을 바라보도록 한다.
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

//! 일정 시간만큼 지연시킨 후에 FireWeaponAtViewTarget함수를 호출한다.
function TimedFireWeaponAtViewTarget(float delay=1.0)
{
//	`log("avaVehicleBot.TimedFireWeaponAtViewTarget" @delay);

	// 포가 돌아갈 시간만큼 기다렸다가 호출되도록 한다.
	SetTimer(delay, false, 'FireWeaponAtViewTarget');
}

//! ViewTarget을 향해서 발포한다.
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
				// 총 거리를 얻어서.
				CalcTotalDistance();

				// GRI에 전체 거리를 설정한다.
				GRI = GetGRI();
				if ( GRI != None )
				{
					GRI.SetMissionMaxTime( TotalDistance );
					// HM후에 호출될 수 있어서 이렇게 설정.
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

				// Host의 경우 체크 포인트를 등록해 준다.
				if ( Role == ROLE_Authority )
					RegisterCheckPoints();

				// avaRoute인 경우 시작 인덱스를 얻어올 수 있다.
				if ( avaVehicle(Pawn) != None )
				{
					ScriptedRouteIndex = avaVehicle(Pawn).AIScriptedRouteIndex;
//					`log("avaVehicleBot.OnMoveToActor - AIScriptedRouteIndex=" @ScriptedRouteIndex);

					// 기본값을 저장.
					DefaultEngineTorque = avaVehicleSimTank(avaVehicle(Pawn).SimObj).MaxEngineTorque;

					// 만약 0인(HostMigration이 아닌) 경우...
					if ( ScriptedRouteIndex == 0 )
					{
						// avaRoute에 시작 인덱스가 있다면 그 값으로 설정해 준다.
						if ( avaRoute(ScriptedRoute) != None )
							ScriptedRouteIndex = avaRoute(ScriptedRoute).StartIndex;
					}
					else
					{
						// HostMigration인 경우 1가지 체크 추가.
						// HM 중 혹시라도 다음 이동할 위치(Pawn.Location, RBState.Position)를 넘어간 경우
						// HM 후에 탱크가 뒤로 돌아가려는 현상이 발생할 수 있어서 추가.

/*
						NextIndex = FindTargetIndex();
						// 캐릭터의 위치가 이미 ScriptedRouteIndex를 넘었을 경우
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

				// 매번 Loop를 돌 때마다 현재 위치를 갱신해 준다.(HM후 갱신을 위해서)
				CalcCurrentDistance();
				// GRI에 현재 이동한 거리를 넣어준다.(HM후 갱신을 위해서)
				UpdateCurrentDistance();

				// 전체 경로의 거리를 로그로 남긴다.
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
	// 만약 Health가 0이 된 경우에는 다시 시작해야 한다.
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

		// 서스펜션 덜컹 거리는 부분 빼기.
		SimTank = avaVehicleSimTank(avaVehicle(Pawn).SimObj);
		SavedWheelSuspensionBias = SimTank.WheelSuspensionBias;
		SimTank.WheelSuspensionBias=0;

		avaVehicle(Pawn).SetDriving( false );
		StopMovement();

//		bPreparingMove = false;
		RouteCache.Length = 0;

		// ScriptedMove에서 execMoveToward가 처음 호출 후 PrepareForMove에서 Anchor가 설정되는데
		// execPollMoveToward가 호출되기 전에 Stop되었다가 다시 Go가 될 경우,
		// 이전의 Anchor값이 설정되어 있어서 PrepareForMove함수 내부에서 Side쪽에 DynamicAnchor를 생성해서
		// 엄한 방향으로 이동하게 되더라.
//		Pawn.SetAnchor( NavigationPoint(FindPathToward( Pawn )) );
	}

Begin:
	// Latent 정지.
	StopLatentExecution();

	// 2초 후에 물리적인 기능 정지!
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

		// 다음 목적지를 도착하고 다시 Pop될 경우에는 Vehicle을 멈춰준다.
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

	//! 엔진의 토크를 초기값으로 설정해 준다.
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

	//! 엔진의 토크를 증가시켜 준다.
	function IncEngineTorque()
	{
		local avaVehicle	V;

		if ( avaVehicle(Pawn) == None )
			return ;

		V = avaVehicle(Pawn);

		if ( avaVehicleSimTank(V.SimObj) != None )
		{
			// 최대 10배까지 빠르게 해준다.
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
		// 매번 Loop를 돌 때마다 현재 위치를 갱신해 준다.
		CalcCurrentDistance();

		// GRI에 현재 이동한 거리를 넣어준다.
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

				// 이동을 못하면 토크를 n배로 계속 추가해 준다.
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

		// Reset에 의해서 ScriptedRoute가 None인 경우에는
		// 이벤트를 생성하지 않는다.
		if ( ScriptedRoute != None )
		{
			// [1] = End Route, [2] = Failed to End Route
			if ( bFailedToReachRoute || ScriptedRouteIndex < ScriptedRouteNavCount )
				ActivateVehicleEvent(2);
			else
				ActivateVehicleEvent(1);
		}

		//  실패 하지 않은 경우에만 결과를 반영하도록 하자.
		if ( !bFailedToReachRoute )
		{
			// if we still have the move target, then finish the latent move
			// otherwise consider it aborted
			ClearLatentAction(class'SeqAct_AIMoveToActor', (ScriptedRoute == None));
			ScriptedRoute = None;
		}

		// 0.1초마다 CurrentDistance 갱신 타이머 함수 종료.
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

		// 0.1초마다 CurrentDistance 갱신 타이머 함수 시작.
		SetTimer(0.1, true, 'UpdateCurrentDistanceForServer');
	}

Begin:
//	`log("avaBot.ScriptedRouteMove:Begin");

	while ( Pawn != None && 
			!bFailedToReachRoute &&
			//ScriptedRoute != None && 
			ScriptedRouteIndex < ScriptedRouteNavCount && ScriptedRouteIndex >= 0)
	{
		// Vehicle인 경우 현재 RouteIndex값을 저장해 준다.(HostMigration에서 사용)
		if ( avaVehicle(Pawn) != None )
		{
//			`log("avaVehicleBot - avaVehicle.AIScriptedRouteIndex = " @ScriptedRouteIndex);
			avaVehicle(Pawn).AIScriptedRouteIndex = ScriptedRouteIndex;
		}

		ScriptedMoveTarget = ScriptedRoute.NavList[ScriptedRouteIndex].Nav;
		if (ScriptedMoveTarget != None)
		{
			// 대상을 처다보게 해준다.
			if ( ForceViewTarget != None )
				ScriptedFocus = ForceViewTarget;
			else
				ScriptedFocus = ScriptedMoveTarget;

//			`log("avaBot.ScriptedRouteMove:Begin" @ScriptedMoveTarget 
//				@" Index=" @ScriptedRouteIndex
//				@" Focus=" @ScriptedFocus);
			PushState('ScriptedMove');
		}

		// 현재 PathNode의 거리에 대한 로그.
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
	// OnAIMoveToActor함수가 호출되어 state ScriptedRouteMove가 실행중인 경우에만 갱신하도록 해준다.
	// 또한 이동 중인 경우에만 갱신한다.
	if ( IsInState('ScriptedMove') && Pawn.Physics != PHYS_None )
	{
//		`log("eventTick");

		// 매번 Loop를 돌 때마다 현재 위치를 갱신해 준다.
		CalcCurrentDistance();

		// GRI에 현재 이동한 거리를 넣어준다.(Local용 갱신)
		UpdateCurrentDistance(true);
	}
}

//! 0.1초 간격으로 호출될 함수.(Server만 호출)
function UpdateCurrentDistanceForServer()
{
	// 매번 Loop를 돌 때마다 현재 위치를 갱신해 준다.
	CalcCurrentDistance();

	// GRI에 현재 이동한 거리를 넣어준다.(Server:Replication용 갱신)
	UpdateCurrentDistance(false);

//	`log("UpdateCurrentDistanceForServer");
}

defaultproperties
{
	// PlayerReplicationInfo를 생성하려면 true여야 한다.
	bIsPlayer=true

	EngineTorqueFactor=1.0
}