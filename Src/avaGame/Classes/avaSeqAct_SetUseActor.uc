class avaSeqAct_SetUseActor extends SequenceAction;

var() avaUseVolume		useVolume;
var() actor				useActor;

event Activated()
{
	if ( useVolume != None )
	{
		useVolume.UseActor	=	useActor;
	}
}

defaultproperties
{
	ObjCategory		=	"Level"
	ObjName			=	"Set UseVolume Actor"
	bCallHandler	=	false

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Use Volume",PropertyName=useVolume,MinVars=0, MaxVars=1 )
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Use Actor",PropertyName=useActor,MinVars=0, MaxVars=1 )
		
}



