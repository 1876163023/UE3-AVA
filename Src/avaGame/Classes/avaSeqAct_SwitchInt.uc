/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaSeqAct_SwitchInt extends SequenceAction
	native;

cpptext
{
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	void Activated();
	void DeActivated() { }
};

var() int			ValueA;
var() array<int>	Values;

/**
 * Determines whether this class should be displayed in the list of available ops in the UI's kismet editor.
 *
 * @param	TargetObject	the widget that this SequenceObject would be attached to.
 *
 * @return	TRUE if this sequence object should be available for use in the UI kismet editor
 */
event bool IsValidUISequenceObject( optional UIScreenObject TargetObject )
{
	return true;
}

defaultproperties
{
	ObjName="Switch Int"
	ObjCategory="Switch"

	ValueA=0
	Values(0)=0

	InputLinks(0)=(LinkDesc="In")
	OutputLinks(0)=(LinkDesc="A == 0")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="A",PropertyName=ValueA)
}
