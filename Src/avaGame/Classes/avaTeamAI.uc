/**
 * Copyright 1998-2007 Epic Games, Inc. All Rights Reserved.
 */
//=============================================================================
// avaTeamAI.
// strategic team AI control for TeamGame
//
//=============================================================================
class avaTeamAI extends Info;

var avaTeamInfo Team;
var avaTeamInfo EnemyTeam;
var	int	NumSupportingPlayer;

var avaGameObjective Objectives; // list of objectives to be defended or attacked by this team
var avaGameObjective PickedObjective;	// objective that was picked from a list of equal priority objectives

var avaSquadAI Squads;
var avaSquadAI AttackSquad, FreelanceSquad;
var class<avaSquadAI> SquadType;
var int OrderOffset;
var name OrderList[8];

/* FIXMESTEVE
var Pickup SuperPickups[16];
var int NumSuperPickups;
*/

function PostBeginPlay()
{
	Super.PostBeginPlay();
	SetTimer(5.0,true);
}

function Timer()
{
	ReAssessStrategy();
}


function CriticalObjectiveWarning(avaGameObjective G, Pawn NewEnemy);

function bool SuperPickupAvailable(avaBot B)
{
	return false;
/* FIXMESTEVE

	local int i;
	local Pickup P;

	if ( NumSuperPickups == 0 )
	{
		ForEach DynamicActors(class'Pickup', P)
			if ( P.IsSuperItem() )
			{
				SuperPickups[NumSuperPickups] = P;
				NumSuperPickups++;
				if ( NumSuperPickups == 16 )
					break;
			}
	}

	if ( NumSuperPickups <= 0 )
	{
		NumSuperPickups = -1;
		return false;
	}

	for ( i=0; i<NumSuperPickups; i++ )
	{
		if ( (SuperPickups[i] != None) && SuperPickups[i].ReadyToPickup(0)
			&& B.SuperPickupNotSpokenFor(SuperPickups[i]) )
			return true;
	}

	if ( NumSuperPickups < 16 )
		return false;

	ForEach DynamicActors(class'Pickup', P)
		if ( P.IsSuperItem() )
		{
			if ( P.ReadyToPickup(2) )
				return true;
		}

	return false;
*/
}

function Reset()
{
	Super.Reset();
	PickedObjective = None;
}

function ClearEnemies()
{
	local avaSquadAI S;
	local int i;
	local avaBot M;

	Reset();
	for ( S=Squads; S!=None; S=S.NextSquad )
	{
		for ( i=0; i<8; i++ )
			S.Reset();
		for	( M=S.SquadMembers; M!=None; M=M.NextSquadMember )
			M.Reset();
	}
}

function avaSquadAI GetSquadLedBy(Controller C)
{
	local avaSquadAI S;

	for ( S=Squads; S!=None; S=S.NextSquad )
		if ( S.SquadLeader == C )
			return S;
	return None;
}

// ReAssessStrategy()
//Look at current strategic situation, and decide whether to update squad objectives

function ReAssessStrategy()
{
	local avaGameObjective O;
	local int PlusDiff, MinusDiff;

	if ( FreelanceSquad == None )
		return;

	// decide whether to play defensively or aggressively
	if ( WorldInfo.Game.TimeLimit > 0 )
	{
		PlusDiff = 0;
		MinusDiff = 2;
		if ( avaGame(WorldInfo.Game).RemainingTime < 180 )
			MinusDiff = 0;
	}
	else
	{
		PlusDiff = 2;
		MinusDiff = 2;
	}

	FreelanceSquad.bFreelanceAttack = false;
	FreelanceSquad.bFreelanceDefend = false;
	if ( Team.Score > EnemyTeam.Score + PlusDiff )
	{
		FreelanceSquad.bFreelanceDefend = true;
		O = GetLeastDefendedObjective(FreelanceSquad.SquadLeader);
	}
	else if ( Team.Score < EnemyTeam.Score - MinusDiff )
	{
		FreelanceSquad.bFreelanceAttack = true;
		O = GetPriorityAttackObjectiveFor(FreelanceSquad, FreelanceSquad.SquadLeader);
	}
	else
		O = GetPriorityFreelanceObjectiveFor(FreelanceSquad);

	if ( (O != None) && (O != FreelanceSquad.SquadObjective) )
		FreelanceSquad.SetObjective(O,true);
}

function NotifyKilled(Controller Killer, Controller Killed, Pawn KilledPawn)
{
	local avaSquadAI S;

	for ( S=Squads; S!=None; S=S.NextSquad )
		S.NotifyKilled(Killer,Killed,KilledPawn);
}

function FindNewObjectives(avaGameObjective DisabledObjective)
{
	local avaSquadAI S;

	for (S = Squads; S != None; S = S.NextSquad)
	{
		if (DisabledObjective == None || S.SquadObjective == DisabledObjective)
		{
			FindNewObjectiveFor(S, true);
		}
	}
}

// FindNewObjectiveFor()
//pick a new objective for a squad that has completed its current objective

function FindNewObjectiveFor(avaSquadAI S, bool bForceUpdate)
{
	local avaGameObjective O, Temp;

	if ( PlayerController(S.SquadLeader) != None )
		return;
	if ( S.bFreelance )
		O = GetPriorityFreelanceObjectiveFor(S);
	else if ( S.GetOrders() == 'Attack' )
		O = GetPriorityAttackObjectiveFor(S, S.SquadLeader);
	if ( O == None )
	{
		O = GetLeastDefendedObjective(S.SquadLeader);
		if ( (O != None) && (O.DefenseSquad != None) )
		{
			if ( S.GetOrders() == 'Attack' )
			{
				S.MergeWith(O.DefenseSquad);
				return;
			}
			else
			{
				Temp = O;
				O = GetPriorityAttackObjectiveFor(S, S.SquadLeader);
				if ( O == None )
				{
					S.MergeWith(Temp.DefenseSquad);
					return;
				}
			}
		}
	}
	if ( (O == None) && (S.bFreelance || (S.GetOrders() == 'Defend')) )
		O = GetPriorityAttackObjectiveFor(S, S.SquadLeader);
	S.SetObjective(O,bForceUpdate);
}

function RemoveSquad(avaSquadAI Squad)
{
	local avaSquadAI S;

	if (Squad == AttackSquad)
	{
		AttackSquad = None;
	}
	if (Squad == FreelanceSquad)
	{
		FreelanceSquad = None;
	}
	if ( Squad == Squads )
	{
		Squads = Squads.NextSquad;
	}
	else
	{
		for (S = Squads; S != None; S = S.NextSquad)
		{
			if (S.NextSquad == Squad)
			{
				S.NextSquad = S.NextSquad.NextSquad;
				return;
			}
		}
	}
}


function avaSquadAI FindSquadOf(Controller C)
{
	local avaSquadAI S;

	if ( avaBot(C) != None )
		return avaBot(C).Squad;

	for ( S=Squads; S!=None; S=S.NextSquad )
		if ( S.SquadLeader == C )
			return S;
	return None;
}

function bool FriendlyToward(Pawn Other)
{
	return WorldInfo.GRI.OnSameTeam(self,Other);
}

simulated function byte GetTeamNum()
{
	return Team.TeamIndex;
}

function SetObjectiveLists()
{
	local avaGameObjective O;

	foreach WorldInfo.AllNavigationPoints(class'avaGameObjective', O)
	{
		if (O.bFirstObjective)
		{
			Objectives = O;
			break;
		}
	}
}

function avaSquadAI FindHumanSquad()
{
	local avaSquadAI S;

	for ( S=Squads; S!=None; S=S.NextSquad )
		if ( S.SquadLeader.IsA('PlayerController') )
			return S;

	return None;
}

function avaSquadAI AddHumanSquad()
{
	local avaSquadAI S;
	local PlayerController P;

	S = FindHumanSquad();
	if ( S != None )
		return S;

	// add human squad
	foreach WorldInfo.AllControllers(class'PlayerController', P)
	{
		if (P.PlayerReplicationInfo.Team == Team && !P.PlayerReplicationInfo.bOnlySpectator)
		{
			return AddSquadWithLeader(P, None);
		}
	}

	return None;
}

function PutBotOnSquadLedBy(Controller C, avaBot B)
{
	local avaSquadAI S;

	for ( S=Squads; S!=None; S=S.NextSquad )
		if ( S.SquadLeader == C )
			break;

	if ( (S == None) && (PlayerController(C) != None) )
		S = AddSquadWithLeader(C,None);

	if ( S != None )
		S.AddBot(B);
}

function avaSquadAI AddSquadWithLeader(Controller C, avaGameObjective O)
{
	local avaSquadAI S;

	S = spawn(SquadType);
	S.Initialize(Team,O,C);
	S.NextSquad = Squads;
	Squads = S;
	return S;
}

function avaGameObjective GetLeastDefendedObjective(Controller InController)
{
	local avaGameObjective O, Best;
	local bool bCheckDistance;
	local float BestDistSq, NewDistSq;

	bCheckDistance = (InController != None) && (InController.Pawn != None);
	for ( O=Objectives; O!=None; O=O.NextObjective )
	{
		if ( (O.DefenderTeamIndex == Team.TeamIndex) && !O.IsDisabled() )
		{
			if ( (Best == None) || (Best.DefensePriority < O.DefensePriority) )
			{
				Best = O;
				BestDistSq = VSizeSq(Best.Location - InController.Pawn.Location);
			}
			else if ( Best.DefensePriority == O.DefensePriority )
			{
				// prioritize less defended or closer nodes
				NewDistSq = VSizeSq(O.Location - InController.Pawn.Location);
				if ( (Best.GetNumDefenders() > O.GetNumDefenders()) || (bCheckDistance && (NewDistSq < BestDistSq)) )
				{
					Best = O;
					BestDistSq = NewDistSq;
				}
			}
		}
	}
	return Best;
}

function avaGameObjective GetPriorityAttackObjectiveFor(avaSquadAI InAttackSquad, Controller InController)
{
	local avaGameObjective O;
	local bool bCheckDistance;
	local float BestDistSq, NewDistSq;

	bCheckDistance = (InController != None) && (InController.Pawn != None);

	if ( (PickedObjective != None) && PickedObjective.IsDisabled() )
		PickedObjective = None;
	if ( PickedObjective == None )
	{
		for ( O=Objectives; O!=None; O=O.NextObjective )
		{
			if ( (O.DefenderTeamIndex != Team.TeamIndex) && !O.IsDisabled() )
			{
				if ( (PickedObjective == None) || (PickedObjective.DefensePriority < O.DefensePriority) )
				{
					PickedObjective = O;
					BestDistSq = VSizeSq(PickedObjective.Location - InController.Pawn.Location);
				}
				else if ( bCheckDistance && (PickedObjective.DefensePriority == O.DefensePriority) )
				{
					// prioritize closer nodes
					NewDistSq = VSizeSq(O.Location - InController.Pawn.Location);
					if ( NewDistSq < BestDistSq )
					{
						PickedObjective = O;
						BestDistSq = NewDistSq;
					}
				}
			}
		}
	}
	return PickedObjective;
}

function avaGameObjective GetPriorityFreelanceObjectiveFor(avaSquadAI InFreelanceSquad)
{
	return GetPriorityAttackObjectiveFor(InFreelanceSquad, InFreelanceSquad.SquadLeader);
}

function bool PutOnDefense(avaBot B)
{
	local avaGameObjective O;

	O = GetLeastDefendedObjective(B);
	if ( O != None )
	{
		if ( O.DefenseSquad == None )
			O.DefenseSquad = AddSquadWithLeader(B, O);
		else
			O.DefenseSquad.AddBot(B);
		return true;
	}
	return false;
}

function PutOnOffense(avaBot B)
{
	if ( (AttackSquad == None) || (AttackSquad.Size >= AttackSquad.MaxSquadSize) )
		AttackSquad = AddSquadWithLeader(B, GetPriorityAttackObjectiveFor(None, B));
	else
		AttackSquad.AddBot(B);
}

function PutOnFreelance(avaBot B)
{
	if ( (FreelanceSquad == None) || (FreelanceSquad.Size >= FreelanceSquad.MaxSquadSize) )
		FreelanceSquad = AddSquadWithLeader(B, GetPriorityFreelanceObjectiveFor(None));
	else
		FreelanceSquad.AddBot(B);
	if ( !FreelanceSquad.bFreelance )
	{
	FreelanceSquad.bFreelance = true;
		NetUpdateTime = WorldInfo.TimeSeconds - 1;
	}
}


//SetBotOrders - sets bot's initial orders

//FIXME - need assault type pick leader when leader dies for attacking
//freelance squad - backs up defenders under attack, or joins in attacks

function SetBotOrders(avaBot NewBot)
{
	local avaSquadAI HumanSquad;
	local name NewOrders;

	if ( Objectives == None )
		SetObjectiveLists();

//	if ( avaTeamGame(WorldInfo.Game).bForceAllRed )
//		NewOrders = 'DEFEND';
	/* @FIXME: get singleplayer orders from somewhere
	else if ( (R==None) || R.NoRecommendation() )
	{
		// pick orders
		if ( Team.Size == 0 )
			OrderOffset = 0;
		NewOrders = OrderList[OrderOffset % 8];
		OrderOffset++;
	}
	else if ( R.RecommendDefense() )
		NewOrders = 'DEFEND';
	else if ( R.RecommendAttack() )
		NewOrders = 'ATTACK';
	else if ( R.RecommendSupport() )
		NewOrders = 'FOLLOW';
	else
		NewOrders = 'FREELANCE';
	*/
//	else
//	{
		// resset orders list if only bot on team
		//@FIXME: this still doesn't handle players joining/leaving in e.g. Players vs Bots screwing up orders over time
		if (Team.Size == 1)
		{
			OrderOffset = 0;
		}
		NewOrders = OrderList[OrderOffset % 8];
		OrderOffset++;
//	}

	// `log(NewBot$" set Initial orders "$NewOrders);
	if ( (NewOrders == 'DEFEND') && PutOnDefense(NewBot) )
		return;

	if ( NewOrders == 'FREELANCE' )
	{
		PutOnFreelance(NewBot);
		return;
	}

	if ( NewOrders == 'ATTACK' )
	{
		PutOnOffense(NewBot);
		return;
	}

	if ( NewOrders == 'FOLLOW' )
	{
		// Follow any human player
		HumanSquad = AddHumanSquad();
		if ( HumanSquad != None )
		{
			HumanSquad.AddBot(NewBot);
			return;
		}
	}
	PutOnOffense(NewBot);
}

// SetOrders()
// Called when player gives orders to bot
function SetOrders(avaBot B, name NewOrders, Controller OrderGiver)
{
//	local avaPlayerReplicationInfo PRI;

//	PRI = avaPlayerReplicationInfo(B.PlayerReplicationInfo);
/*? avaHoldSpot not implemented.
	if ( avaHoldSpot(B.DefensePoint) != None )
	{
		PRI.bHolding = false;
		B.FreePoint();
	}
*/
	//`log("Team New orders "$NewOrders@OrderGiver);
	if ( NewOrders == 'Hold' )
	{
/*? avaHoldSpot not implemented.
		PRI.bHolding = true;
		PutBotOnSquadLedBy(OrderGiver,B);
		B.DefensePoint = PlayerController(OrderGiver).ViewTarget.Spawn(class'avaHoldSpot');
		if ( Vehicle(PlayerController(OrderGiver).ViewTarget) != None )
			avaHoldSpot(B.DefensePoint).HoldVehicle = avaVehicle(PlayerController(OrderGiver).ViewTarget);
*/
		if ( PlayerController(OrderGiver).ViewTarget.Physics == PHYS_Ladder )
			B.DefensePoint.SetPhysics(PHYS_Ladder);
	}
	else if ( NewOrders == 'Defend' )
		PutOnDefense(B);
	else if ( NewOrders == 'Attack' )
		PutOnOffense(B);
	else if ( NewOrders == 'Follow' )
	{
		B.FreePoint();
		PutBotOnSquadLedBy(OrderGiver,B);
	}
	else if ( NewOrders == 'Freelance' )
	{
		PutOnFreelance(B);
		return;
	}
}

function CallForHelp(avaBot B)
{
}

function RemoveFromTeam(Controller Other)
{
	local avaSquadAI S;

	if ( PlayerController(Other) != None )
	{
		for ( S=Squads; S!=None; S=S.NextSquad )
			S.RemovePlayer(PlayerController(Other));
	}
	else if ( avaBot(Other) != None )
	{
		for ( S=Squads; S!=None; S=S.NextSquad )
			S.RemoveBot(avaBot(Other));
	}
}

defaultproperties
{
	RemoteRole=ROLE_None
	SquadType=class'avaGame.avaSquadAI'

	Components.Remove(Sprite)

	OrderList(0)=FOLLOW
	OrderList(1)=ATTACK
	OrderList(2)=DEFEND
	OrderList(3)=FREELANCE
	OrderList(4)=FOLLOW
	OrderList(5)=ATTACK
	OrderList(6)=DEFEND
	OrderList(7)=FREELANCE
}
