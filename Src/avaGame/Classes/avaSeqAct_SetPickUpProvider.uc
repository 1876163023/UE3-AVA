class avaSeqAct_SetPickUpProvider extends SequenceAction;

var() avaPickUpProvider		PickUpProvider;
var() actor					ProviderActor;

event Activated()
{
	if ( PickUpProvider != None )
	{
		PickUpProvider.ProviderActor	=	ProviderActor;
	}
}

defaultproperties
{
	ObjCategory		=	"Level"
	ObjName			=	"Set PickUp Provider"
	bCallHandler	=	false

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="PickUp Provider",PropertyName=PickUpProvider,MinVars=0, MaxVars=1 )
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Provider Actor",PropertyName=ProviderActor,MinVars=0, MaxVars=1 )
		
}



