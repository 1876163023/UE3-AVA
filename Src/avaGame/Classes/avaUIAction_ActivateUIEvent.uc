class avaUIAction_ActivateUIEvent extends UIAction
	native;


cpptext
{
	void Activated();

protected:
	void GetEventsOfClassRecursive( class UClass* EventClassToFind, UUIScreenObject* TargetObject, TArray<UUIEvent*>& outEventFound );
}

/** Name of the event to activate */
var() Name EventName;
var() bool bFindAllActiveScenes<ToolTip=it can be tiresome. otherwise find event of ancestors>;
var() string EventNameString;

DefaultProperties
{
	ObjName="Activate UI Event"
	ObjCategory="UI"

	bAutoActivateOutputLinks=false
	EventName=DefaultEvent

	// remove the "Targets" variable link, as it's unnecessary for this action
	VariableLinks.Empty

	OutputLinks.Empty
	OutputLinks(0)=(LinkDesc="Failed")
	OutputLinks(1)=(LinkDesc="Success")

	bFindAllActiveScenes=false;

	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Event Name",PropertyName=EventNameString,bHidden=true))
}