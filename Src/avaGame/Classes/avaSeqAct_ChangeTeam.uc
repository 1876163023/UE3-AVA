/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaSeqAct_ChangeTeam extends UIAction;

var() byte NewTeamNum;
var() byte NewClassNum;

event Activated()
{
	local SeqVar_Object ObjVar;
	local Pawn P;
	local Controller C;
	local avaTeamGame Game;

	Game = avaTeamGame(GetWorldInfo().Game);
	if (Game != None)
	{
		foreach LinkedVariables(class'SeqVar_Object', ObjVar, "Target")
		{
			// find the object to change the team of
			C = Controller(ObjVar.GetObjectValue());
			if (C == None)
			{
				P = Pawn(ObjVar.GetObjectValue());
				if (P != None)
				{
					if (P.Controller != None)
					{
						C = P.Controller;
					}					
				}
			}
			// if we got a player, change its team
			if (C != None && C.PlayerReplicationInfo != None)
			{
				if (C.PlayerReplicationInfo.Team != None)
				{
					C.PlayerReplicationInfo.Team.RemoveFromTeam(C);
					C.PlayerReplicationInfo.Team = None;
				}
				if (NewTeamNum != 255)
				{
					Game.Teams[NewTeamNum].AddToTeam(C);
				}
				// update the pawn (teamskins, etc)
				if (C.Pawn != None)
				{
					C.Pawn.NotifyTeamChanged();
				}
			}
		}
	}
}

event bool IsValidLevelSequenceObject()
{
	return true;
}

defaultproperties
{
	ObjCategory="Team"
	ObjName="Change Team"
	bCallHandler=false
}
