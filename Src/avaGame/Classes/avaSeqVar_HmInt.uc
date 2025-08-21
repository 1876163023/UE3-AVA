/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaSeqVar_HmInt extends SequenceVariable
	native(Sequence);

cpptext
{	
	void retrieveValue(void);
	virtual INT* GetIntRef();
	/*
	{
		return &IntValue;
	}
	*/

	virtual FString GetValueStr();
	/*
	{
		return FString::Printf(TEXT("%d"),IntValue);
	}
	*/

	virtual UBOOL SupportsProperty(UProperty *Property)
	{
		return (Property->IsA(UIntProperty::StaticClass()));
	}

	virtual void PublishValue(USequenceOp *Op, UProperty *Property, FSeqVarLink &VarLink);
	virtual void PopulateValue(USequenceOp *Op, UProperty *Property, FSeqVarLink &VarLink);
}

var() int				IntValue;

defaultproperties
{
	ObjName="HmInt"
	ObjColor=(R=192,G=255,B=255,A=255)
}
