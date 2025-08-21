class avaSeqAct_ActivateKOTH extends SequenceAction;

var() avaVolume_KOTH	KOTHVolume;
var() int				Team;
var() bool				bEnable;

event Activated()
{
	if ( KOTHVolume != None )
		KOTHVolume.ActivateTeam( Team, bEnable );
}

defaultproperties
{
	ObjCategory="Objective"
	ObjName="Activate KOTH"
	bCallHandler=false

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="KOTHVolume",PropertyName=KOTHVolume,MinVars=0, MaxVars=1 )
	VariableLinks(1)=(ExpectedType=class'SeqVar_Int',LinkDesc="Team",PropertyName=Team,MinVars=0, MaxVars=1 )
	VariableLinks(2)=(ExpectedType=class'SeqVar_Bool',LinkDesc="bActivate",PropertyName=bEnable,MinVars=0, MaxVars=1 )
}
