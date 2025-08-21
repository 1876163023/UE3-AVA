class avaUIAction_ChangeTeam extends UIAction;

var() int NewTeamNum;
var() int NewClassNum;

event Activated()
{
	local avaPlayerReplicationInfo	avaPRI;
	local PlayerController			PC;
	local WorldInfo					WorldInfo;
	if ( GetOwnerScene() == None )						return;
	WorldInfo = GetOwnerScene().GetWorldInfo();
	`log( "avaUIAction_ChangeClass" @NewClassNum @NewTeamNum );
	foreach WorldInfo.LocalPlayerControllers(PC)
	{
		avaPRI = avaPlayerReplicationInfo( PC.PlayerReplicationInfo );
		avaPRI.SetPlayerClassID( NewClassNum );
		if ( avaPRI.Team != None && avaPRI.Team.TeamIndex != NewTeamNum )
			avaPlayerController(PC).ServerChangeTeam( NewTeamNum );
	}
}

defaultproperties
{
	ObjName="ava ChangeTeam"
	bCallHandler=false
	VariableLinks.Add( (ExpectedType=class'SeqVar_Int',LinkDesc="Team",PropertyName=NewTeamNum, MinVars=0, MaxVars=1 ) )
	VariableLinks.Add( (ExpectedType=class'SeqVar_Int',LinkDesc="Class",PropertyName=NewClassNum, MinVars=0, MaxVars=1) )
}

