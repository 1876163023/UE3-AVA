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

/** 저장된 곡선의 중간값을 계산해서 반환. 곡선이 없으면 0을 반환 */
/** 
 *	Evaluate the output for an arbitary input value. 
 *	For inputs outside the range of the keys, the first/last key value is assumed.
 */
native function float Eval( float InVal, float Default );

/** 저장된 곡선의 중간 비율로 계산해서 반환 */
native function float EvalRatio(Float Ratio);

defaultproperties
{
}
