class avaSeqAct_GetPlayerClass extends SequenceAction;

var int ClassID;

event Activated()
{
	local avaPawn P;
	local Controller C;

	if (Targets.length == 0)
	{
		ScriptLog("WARNING: Missing Target for Get Player Class.");
	}
	else
	{
		P = avaPawn(Targets[0]);
		if (P == None)
		{
			C = Controller(Targets[0]);
			if (C != None)
			{
				P = avaPawn(C.Pawn);
			}
		}

		//C = Controller(Targets[0]);
		//if (C == None)
		//{
		//	P = Pawn(Targets[0]);
		//	if (P != None)
		//	{
		//		if (P.Controller != None)
		//		{
		//			C = P.Controller;
		//		}					
		//	}
		//}
	}

	if (P != None)
	{
		ClassID = P.GetTypeID();
	}
	//if (C != None && C.PlayerReplicationInfo != None)
	//{
	//	ClassID = avaPlayerReplicationInfo(C.PlayerReplicationInfo).GetPlayerClassID();
	//}
	else
	{
		`warn(self @ ": Cannot find Pawn object");
		ClassID = 0;
	}
}

defaultproperties
{
	bCallHandler=false
	ObjCategory="Team"
	ObjName="Get Player Class"
	VariableLinks(0)=(MaxVars=1)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Int',LinkDesc="Class ID",PropertyName=ClassID,MaxVars=1,bWriteable=true)

	ObjClassVersion=2
}
