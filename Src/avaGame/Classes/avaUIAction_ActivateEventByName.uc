class avaUIAction_ActivateEventByName extends UIAction;

/** Name of the event to activate */
var() Name		EventName;
var() string	EventNameString;
var() string	StrParam;
var() int		IntParam;
var() float		FloatParam;
var() bool		BoolParam;
var() object	ObjParam;

event Activated()
{
	Local Name inEventName;

	inEventName = EventName;
	if( Len(EventNameString) != 0 )
		inEventName = name(EventNameString);
	
	class'avaEventTrigger'.static.ActivateEventByName(inEventName, StrParam, IntParam, BoolParam, FloatParam, ObjParam );
}

DefaultProperties
{
	ObjName="Activate UIEvent (Name)"
	ObjCategory="UI"

	bAutoActivateOutputLinks=true
	EventName=DefaultEvent

	// remove the "Targets" variable link, as it's unnecessary for this action
	VariableLinks.Empty

	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Event Name",PropertyName=EventNameString,bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Str",PropertyName=StrParam,bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Int",PropertyName=IntParam,bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Float",PropertyName=FloatParam,bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Bool",PropertyName=BoolParam,bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="Obj",PropertyName=ObjParam,bHidden=true))
}