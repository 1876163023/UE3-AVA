/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaSeqVar_Hm extends SequenceVariable
	native(Sequence);

cpptext
{	
	void retrieveValue(void);
	UBOOL* GetBoolRef();
	/*
	{
		return (UBOOL*)&bValue;
	}
	*/

	FString GetValueStr();
	/*
	{
		return bValue == TRUE ? GTrue : GFalse;
	}
	*/

	virtual UBOOL SupportsProperty(UProperty *Property)
	{
		return (Property->IsA(UBoolProperty::StaticClass()));
	}

	virtual void PublishValue(USequenceOp *Op, UProperty *Property, FSeqVarLink &VarLink);
	virtual void PopulateValue(USequenceOp *Op, UProperty *Property, FSeqVarLink &VarLink);
}

var() int			bValue;

// Red bool - gives you wings!
defaultproperties
{
	ObjName="Hm"
	ObjColor=(R=255,G=128,B=128,A=255)
}
