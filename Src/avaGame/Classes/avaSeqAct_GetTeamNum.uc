class avaSeqAct_GetTeamNum extends SequenceAction;

var int TeamNum;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 죽은 경우에는 Controller 가 none 이고 PlayerReplicationInfo 가 none 이기 때문에 GetTeamNum 를 가지고 올 수 없다....
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
event Activated()
{
	local Actor A;

	if (Targets.length == 0)
	{
		ScriptLog("WARNING: Missing Target for Get Team Number");
	}
	else
	{
		A = Actor(Targets[0]);
	}

	//`log( "avaSeqAct_GetTeamNum" @A @A.GetTeamNum() @avaPawn(A).Controller @avaPawn(A).PlayerReplicationInfo @avaPawn(A).PlayerReplicationInfo.Team @avaPawn(A).PlayerReplicationInfo.Team.TeamIndex );
	TeamNum = (A != None) ? int(A.GetTeamNum()) : 255;
}

defaultproperties
{
	bCallHandler=false
	ObjCategory="Team"
	ObjName="Get Team Number"
	VariableLinks(0)=(MaxVars=1)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Int',LinkDesc="Team Number",PropertyName=TeamNum,MaxVars=1,bWriteable=true)
}
