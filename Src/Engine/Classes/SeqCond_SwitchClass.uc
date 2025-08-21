/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class SeqCond_SwitchClass extends SequenceCondition
	native(Sequence);

cpptext
{
	void Activated();
	virtual void	UpdateDynamicLinks();
	virtual FColor	GetConnectionColor( INT ConnType, INT ConnIndex, INT MouseOverConnType, INT MouseOverConnIndex );
}

/** Stores class name to compare for each output link and whether it should fall through to next node */
struct native SwitchClassInfo
{
	var() Name ClassName;
	var() Byte bFallThru;
};
var() array<SwitchClassInfo> ClassArray;

defaultproperties
{
	ObjName="Switch Class"

	InputLinks(0)=(LinkDesc="In")
	OutputLinks(0)=(LinkDesc="Default")
	ClassArray(0)=(ClassName=Default)

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Object")
}
