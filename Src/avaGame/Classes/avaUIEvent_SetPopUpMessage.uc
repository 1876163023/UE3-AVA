/**
 * This event is activated when an object that responds to mouse clicks is clicked on.
 *
 * Copyright Redduck Games, Inc. All Rights Reserved.
 *
 * @note: native because C++ code activates this event
 */
class avaUIEvent_SetPopUpMessage extends UIEvent;

DefaultProperties
{
	ObjName="Set PopUpMessage"
	ObjCategory="avaNet"

	MaxTriggerCount=1

	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="LabelText",bWriteable=true)
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="IntParam",bWriteable=true)
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="NextScene",bHidden=true,bWriteable=true)
}
