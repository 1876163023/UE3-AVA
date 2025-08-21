class avaSeqAct_BreakKBreakable extends SequenceAction;

var avaKBreakable	KBreakable;

event Activated()
{
	if ( KBreakable != None )
		KBreakable.BreakAll();
}

defaultproperties
{
	ObjCategory		=	"Actor"
	ObjName			=	"Break KBreakable"
	bCallHandler	=	false

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="KBreakable",PropertyName=KBreakable,MinVars=0, MaxVars=1 )
}