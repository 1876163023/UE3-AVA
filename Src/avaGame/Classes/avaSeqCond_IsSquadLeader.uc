class avaSeqCond_IsSquadLeader extends SequenceCondition;

var Actor	CheckPlayer;

event Activated()
{
	local Controller C;
	local avaPlayerReplicationInfo R;

	if (CheckPlayer == None)
	{
		ScriptLog("WARNING: Missing CheckPlayer for avaCond_IsSquadLeader.");
	}
	else
	{
		C = Controller(CheckPlayer);

		if (C != None && C.PlayerReplicationInfo != None)
		{
			R = avaPlayerReplicationInfo(C.PlayerReplicationInfo);

			if (R != None && R.IsSquadLeader())
			{
				OutputLinks[0].bHasImpulse = true;
				return;
			}
		}

		OutputLinks[1].bHasImpulse = true;
	}
}

defaultproperties
{
	ObjCategory="Team"
	ObjName="Is Squad Leader"

	OutputLinks(0)=(LinkDesc="Yes")
	OutputLinks(1)=(LinkDesc="No")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object', LinkDesc="Player",PropertyName=CheckPlayer,MaxVars=1)
}

