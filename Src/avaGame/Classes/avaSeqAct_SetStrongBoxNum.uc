class avaSeqAct_SetStrongBoxNum extends SequenceAction;

var()	avaStrongBoxActor	actor;
var()	int					Progress;

event Activated()
{
	if ( InputLinks[0].bHasImpulse )			// start
	{
		actor.SetProgress( 1 );
	}
	else if ( InputLinks[1].bHasImpulse )		// Porgress
	{
		actor.SetProgress( Progress + 1 );	
	}
	else if ( InputLinks[2].bHasImpulse )		// end
	{
		actor.SetProgress( 8 );
	}
	else if ( InputLinks[3].bHasImpulse )		// reset
	{
		actor.SetProgress( 0 );
	}	
}

defaultproperties
{
	ObjCategory		=	"Level"
	ObjName			=	"Set StrongBox Num"
	bCallHandler	=	false

	InputLinks(0)=(LinkDesc="Start")
	InputLinks(1)=(LinkDesc="Progress")
	InputLinks(2)=(LinkDesc="End")
	InputLinks(3)=(LinkDesc="Reset")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="StrongBox",PropertyName=actor,MinVars=0, MaxVars=1 )
	VariableLinks(1)=(ExpectedType=class'SeqVar_Int',LinkDesc="Progress",PropertyName=Progress,MinVars=0, MaxVars=1 )
}