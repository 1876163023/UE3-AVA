/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class SeqCond_CompareString extends SequenceCondition;



var() string ValueA;
var() string ValueB;


event Activated()
{
	// compare the values and set appropriate output impulse
	if (ValueA <= ValueB)
	{
		OutputLinks[0].bHasImpulse = true;
	}
	if (ValueA > ValueB)
	{
		OutputLinks[1].bHasImpulse = true;
	}
	if (ValueA == ValueB)
	{
		OutputLinks[2].bHasImpulse = true;
	}
	if (ValueA != ValueB)
	{
		OutputLinks[3].bHasImpulse = true;
	}
	if (ValueA < ValueB)
	{
		OutputLinks[4].bHasImpulse = true;
	}
	if (ValueA >= ValueB)
	{
		OutputLinks[5].bHasImpulse = true;
	}
}

defaultproperties
{
	ObjName="Compare String"
	ObjCategory="Comparison"

	InputLinks(0)=(LinkDesc="In")
	OutputLinks(0)=(LinkDesc="A <= B")
	OutputLinks(1)=(LinkDesc="A > B")
	OutputLinks(2)=(LinkDesc="A == B")
	OutputLinks(3)=(LinkDesc="A != B")
	OutputLinks(4)=(LinkDesc="A < B")
	OutputLinks(5)=(LinkDesc="A >= B")

	VariableLinks(0)=(ExpectedType=class'SeqVar_String',LinkDesc="A",PropertyName=ValueA)
	VariableLinks(1)=(ExpectedType=class'SeqVar_String',LinkDesc="B",PropertyName=ValueB)
}
