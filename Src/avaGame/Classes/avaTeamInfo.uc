/**
 * Copyright 1998-2007 Epic Games, Inc. All Rights Reserved.
 */
//=============================================================================
// avaTeamInfo.
//
//=============================================================================

class avaTeamInfo extends TeamInfo 
	native;

`include(avaGame/avaGame.uci)

var() class<avaPawn> AllowedTeamMembers[32];
var int DesiredTeamSize;
var avaTeamAI AI;
var string TeamSymbolName;						// 클랜전일때 클랜명으로 사용하도록 하자...		
var RepNotify Material TeamIcon;	
var avaGameObjective HomeBase;					// key objective associated with this team

var color BaseTeamColor[4];
var color TextColor[4];
var localized string TeamColorNames[4];
var string RealTeamName;	// TeamInfo에 있는 TeamName을 사용하지 마세요

var hmserialize	byte	AttackPoint;
var hmserialize byte	DefencePoint;
var hmserialize byte	LeaderPoint;
var hmserialize byte	TacticsPoint;

replication
{
	// Variables the server should send to the client.
	if ( bNetInitial && (Role==ROLE_Authority) )
		TeamIcon, RealTeamName, TeamSymbolName, AttackPoint, DefencePoint, LeaderPoint, TacticsPoint;
}

simulated function string GetHumanReadableName()
{
	if ( TeamName == Default.TeamName )
	{
		if ( TeamIndex < 4 )
			return TeamColorNames[TeamIndex];
		return TeamName@TeamIndex;
	}
	return TeamName;
}

simulated function color GetHUDColor()
{
	return BaseTeamColor[TeamIndex];
}

function color GetTextColor()
{
	return TextColor[TeamIndex];
}

/* Reset()
reset actor to initial state - used when restarting level without reloading.
*/
function Reset()
{
	Super.Reset();
}

/* FIXMESTEVE
event ReplicatedEvent(name VarName)
{
	if ( VarName == 'TeamIcon' )
	{
		TeamSymbolNotify();
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

simulated function TeamSymbolNotify()
{
	local Actor A;

	ForEach AllActors(class'Actor', A)
		A.SetTeamSymbol(self);
}
*/

function bool AllBotsSpawned()
{
	return false;
}

function Initialize(int TeamBots);

function bool NeedsBotMoreThan(avaTeamInfo T)
{
	return ( (DesiredTeamSize - Size) > (T.DesiredTeamSize - T.Size) );
}

function SetBotOrders(avaBot NewBot)
{
	if (AI != None)
	{
		AI.SetBotOrders(NewBot);
	}
}

function RemoveFromTeam(Controller Other)
{
/*? UTCustomChar_Data...
	local int i;

	Super.RemoveFromTeam(Other);
	if (AI != None)
	{
		AI.RemoveFromTeam(Other);
	}

	// clear in use flag
	if (avaBot(Other) != None && Other.PlayerReplicationInfo != None)
	{
		for (i = 0; i < CharactersInUse.Length; i++)
		{
			if (CharactersInUse[i] && Other.PlayerReplicationInfo.PlayerName ~= class'avaCustomChar_Data'.default.Characters[i].CharName)
			{
				CharactersInUse[i] = false;
				break;
			}
		}
	}
*/
}

simulated event Destroyed()
{
	Super.Destroyed();

	if (AI != None)
	{
		AI.Destroy();
	}
}

simulated function class<Pawn> NextLoadOut(class<Pawn> CurrentLoadout)
{
	local int i;
	local class<Pawn> Result;

	Result = AllowedTeamMembers[0];

	for ( i=0; i<ArrayCount(AllowedTeamMembers) - 1; i++ )
	{
		if ( AllowedTeamMembers[i] == CurrentLoadout )
		{
			if ( AllowedTeamMembers[i+1] != None )
				Result = AllowedTeamMembers[i+1];
			break;
		}
		else if ( AllowedTeamMembers[i] == None )
			break;
	}

	return Result;
}


function bool AlreadyExistsEntry(string CharacterName, bool bNoRecursion)
{
	return false;
}

function int GetAvailableScore( EPointType Type, int nScore )
{
	local float	CapRate;
	CapRate = avaGame(WorldInfo.Game).GetScoreCapRate();		// Game Rule 에 의한 Score Capacity 변화...
	switch ( Type )
	{
	case PointType_Attack	:	return GetAvailablePoint( AttackPoint,  `MAX_TEAMATTACKSCORE * CapRate,  nScore );	break;
	case PointType_Defence	:	return GetAvailablePoint( DefencePoint, `MAX_TEAMDEFENCESCORE * CapRate, nScore );	break;
	case PointType_Leader	:	return GetAvailablePoint( LeaderPoint,	`MAX_TEAMLEADERSCORE * CapRate,  nScore );	break;
	case PointType_Tactics	:	return GetAvailablePoint( TacticsPoint, `MAX_TEAMTACTICSSCORE * CapRate, nScore );	break;
	}
	return 0;
}

function int GetAvailablePoint( int Cur, int Max, int nScore )
{
	local int nRemain;
	nRemain = Cur + nScore - Max;
	if ( nRemain > 0 )
		nScore -= nRemain;
	return nScore;
}

function AddPoint( EPointType Type, out int nScore )
{
	local float	CapRate;
	CapRate = avaGame(WorldInfo.Game).GetScoreCapRate();		// Game Rule 에 의한 Score Capacity 변화...
	nScore = GetAvailableScore( Type, nScore );
	switch ( Type )
	{
	case PointType_Attack	:	AttackPoint		= Clamp( AttackPoint + nScore, 0, `MAX_TEAMATTACKSCORE * CapRate );		break;
	case PointType_Defence	:	DefencePoint	= Clamp( DefencePoint + nScore, 0, `MAX_TEAMDEFENCESCORE * CapRate );	break;
	case PointType_Leader	:	LeaderPoint		= Clamp( LeaderPoint + nScore, 0, `MAX_TEAMLEADERSCORE * CapRate );		break;
	case PointType_Tactics	:	TacticsPoint	= Clamp( TacticsPoint + nScore, 0, `MAX_TEAMTACTICSSCORE * CapRate );	break;
	}
}

defaultproperties
{
	DesiredTeamSize=8
	BaseTeamColor(0)=(r=97,g=255,b=64,a=255)
	BaseTeamColor(1)=(r=255,g=163,b=31,a=255)
	BaseTeamColor(2)=(r=65,g=255,b=64,a=255)
	BaseTeamColor(3)=(r=255,g=255,b=0,a=255)

	TextColor(0)=(r=255,g=96,b=96,a=255)
	TextColor(1)=(r=128,g=128,b=255,a=255)
	TextColor(2)=(r=96,g=255,b=96,a=255)
	TextColor(3)=(r=255,g=255,b=96,a=255)
}

