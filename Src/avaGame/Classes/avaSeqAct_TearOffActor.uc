class avaSeqAct_TearOffActor extends SequenceAction;

event Activated()
{
	local SeqVar_Object ObjVar;
	local Actor			actor;

	foreach LinkedVariables(class'SeqVar_Object', ObjVar, "Target")
	{
		actor = Actor(ObjVar.GetObjectValue());
		if (actor != None)
		{
			actor.bTearOff = true;
		}
	}
}

defaultproperties
{
	ObjCategory="Actor"
	ObjName="TearOff"
	bCallHandler=false
}
