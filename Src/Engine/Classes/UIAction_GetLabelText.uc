/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: AVA

	Name: UIAction_GetLabelText.uc

	Description: Acquires label text of some UI components.

***/
class UIAction_GetLabelText extends UIAction_GetValue
	native(inherit);

cpptext
{
	/**
	 * Activated event handler.
	 */
	virtual void Activated(void);
}


var string	OutText;

DefaultProperties
{
	ObjName="Get Label Text"
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Value",PropertyName=OutText,bWriteable=true))
}
