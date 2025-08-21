/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class SeqVar_Vector extends SequenceVariable
	native(Sequence);

cpptext
{
	virtual FVector* GetVectorRef()
	{
		return &VectValue;
	}

	virtual FString GetValueStr()
	{
		return VectValue.ToString();
	}

	virtual UBOOL SupportsProperty(UProperty *Property);
	virtual void PublishValue(USequenceOp *Op, UProperty *Property, FSeqVarLink &VarLink);
	virtual void PopulateValue(USequenceOp *Op, UProperty *Property, FSeqVarLink &VarLink);
}

var() vector VectValue;

defaultproperties
{
	ObjName="Vector"
	ObjColor=(R=128,G=128,B=0,A=255)
}
