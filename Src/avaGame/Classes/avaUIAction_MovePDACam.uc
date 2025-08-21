// PDA is Expired...
class avaUIAction_MovePDACam extends UIAction;

event Activated()
{
	//Local WorldInfo WorldInfo;
	//Local PlayerController PC;
	//Local avaCameraActor PDAViewTarget;

	//if ( GetOwnerScene() == None )
	//	return;

	//WorldInfo = GetOwnerScene().GetWorldInfo();
	//foreach WorldInfo.LocalPlayerControllers(PC)
	//	PDAViewTarget = avaCameraActor(avaPlayerController(PC).PDAViewTarget);
	//
	//if( PDAViewTarget == None )
	//	return;

	//if ( InputLinks[0].bHasImpulse )
	//	PDAViewTarget.PullDown();
	//else if ( InputLinks[1].bHasImpulse )
	//	PDAviewTarget.PushUp();
}

defaultproperties
{
	ObjName="ava MovePDACam"
	bCallHandler=false

	InputLinks.Empty
	InputLinks(0)=(LinkDesc="ZoomIn")
	InputLinks(1)=(LinkDesc="ZoomOut")
}