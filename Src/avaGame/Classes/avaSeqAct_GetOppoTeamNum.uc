// 상대팀의 팀 번호를 알아낸다.
class avaSeqAct_GetOppoTeamNum extends SequenceAction;

var int TeamNum;

event Activated()
{
	local Actor A;	

	if (Targets.length == 0)
	{
		ScriptLog("WARNING: Missing Target for Get Team Number");
	}
	else if ( Targets[0] != none )
	{
		A = Actor(Targets[0]);

		if ( A != none )
		{
			TeamNum = int(A.GetTeamNum());
		}
		else if ( SeqVar_Int(Targets[0]) != none )
		{
			TeamNum = SeqVar_Int(Targets[0]).IntValue;
		}


		if ( TeamNum == 0 )
			TeamNum = 1;
		else if ( TeamNum == 1 )
			TeamNum = 0;
		else
			TeamNum = 255;
	}
}


defaultproperties
{
	bCallHandler=false
	ObjCategory="Team"
	ObjName="Get Oppo Team Number"
	VariableLinks(0)=(MaxVars=1)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Int',LinkDesc="OppoTeamNum",PropertyName=TeamNum,MaxVars=1,bWriteable=true)
}
