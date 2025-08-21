//=============================================================================
// CurveEdPresetCurve
// A preset curve data object
// Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class CurveEdPresetCurve extends object
	native
	hidecategories(Object)
	editinlinenew
	;

/** Preset Generated Point							*/
struct native PresetGeneratedPoint
{
	var		float				KeyIn;
	var		float				KeyOut;
	var		bool				TangentsValid;
	var		float				TangentIn;
	var		float				TangentOut;
	var		EInterpCurveMode	IntepMode;
};

/** Name of the curve								*/
var()	localized string     			CurveName;

/** The points of the curve							*/
var()	editconst array<PresetGeneratedPoint>		Points;

cpptext
{
	UBOOL	StoreCurvePoints(INT CurveIndex, FCurveEdInterface* Distribution);
}

/** ����� ��� �߰����� ����ؼ� ��ȯ. ��� ������ 0�� ��ȯ */
/** 
 *	Evaluate the output for an arbitary input value. 
 *	For inputs outside the range of the keys, the first/last key value is assumed.
 */
native function float Eval( float InVal, float Default );

/** ����� ��� �߰� ������ ����ؼ� ��ȯ */
native function float EvalRatio(Float Ratio);

defaultproperties
{
}
