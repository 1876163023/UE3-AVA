class avaUIAction_GetTeam extends UIAction_GetValue;

var int TeamNum;
var int ClassNum;

event Activated()
{
	local PlayerController			PC;
	local WorldInfo					WorldInfo;
	if ( GetOwnerScene() == None )						return;
	WorldInfo = GetOwnerScene().GetWorldInfo();
	foreach WorldInfo.LocalPlayercontrollers(PC)
	{
		TeamNum	= avaPlayerController(PC).GetTeamNum();
		ClassNum = avaPlayerReplicationInfo( PC.PlayerReplicationInfo ).GetPlayerClassID();
		`log( "avaUIAction_GetTeam.Activated" @TeamNum @ClassNum );
		break;
	}
}

defaultproperties
{
	bCallHandler=false
	ObjCategory="Team"
	ObjName="ava GetTeamNum"
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Team",PropertyName=TeamNum,bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Class",PropertyName=ClassNum,bWriteable=true))
}
