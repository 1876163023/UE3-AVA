/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaSeqCond_CompareInt extends SeqCond_CompareInt
	native;

cpptext
{
	void Activated()
	{
		// compare the values and set appropriate output impulse
		if (ValueA <= ValueB)
		{
			OutputLinks(0).bHasImpulse = TRUE;
		}
		if (ValueA > ValueB)
		{
			OutputLinks(1).bHasImpulse = TRUE;
		}
		if (ValueA == ValueB)
		{
			OutputLinks(2).bHasImpulse = TRUE;
		}
		if (ValueA < ValueB)
		{
			OutputLinks(3).bHasImpulse = TRUE;
		}
		if (ValueA >= ValueB)
		{
			OutputLinks(4).bHasImpulse = TRUE;
		}
		if (ValueA != ValueB)
		{
			OutputLinks(5).bHasImpulse = TRUE;
		}
	}
};

defaultproperties
{
	ObjName="Compare Int (ava)"
	ObjCategory="Comparison"

	OutputLinks(5)=(LinkDesc="A != B")
}
