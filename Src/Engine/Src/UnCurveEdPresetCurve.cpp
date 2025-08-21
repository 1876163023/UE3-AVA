/*=============================================================================
	UnCurveEdPresetCurve.cpp: Shader implementation.
	Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

/**
 *	UCurveEdPresetCurve
 */
IMPLEMENT_CLASS(UCurveEdPresetCurve);

UBOOL UCurveEdPresetCurve::StoreCurvePoints(INT CurveIndex, FCurveEdInterface* Distribution)
{
	if (CurveIndex >= Distribution->GetNumSubCurves())
	{
		return FALSE;
	}

	Points.Empty();
	for (INT PointIndex = 0; PointIndex < Distribution->GetNumKeys(); PointIndex++)
	{
		INT NewIndex = Points.AddZeroed();
		FPresetGeneratedPoint* NewPoint = &Points(NewIndex);

		NewPoint->KeyIn		= Distribution->GetKeyIn(PointIndex);
		NewPoint->KeyOut	= Distribution->GetKeyOut(CurveIndex, PointIndex);
		NewPoint->IntepMode	= Distribution->GetKeyInterpMode(PointIndex);
		Distribution->GetTangents(CurveIndex, PointIndex, NewPoint->TangentIn, NewPoint->TangentOut);
	}

	return TRUE;
}

FLOAT UCurveEdPresetCurve::EvalRatio( FLOAT Ratio )
{
	return Points.Num() >= 2 ? Eval( (Points(Points.Num() - 1).KeyIn - Points(0).KeyIn) * Clamp( Ratio, 0.f, 1.f ) + Points(0).KeyIn, 1.f)  : 1.f ;
}

//
// UnMath.h
// FInterpCurveFloat::Eval()과 같은 구현임.
//
FLOAT UCurveEdPresetCurve::Eval( FLOAT InVal, FLOAT Default )
{
	const INT NumPoints = Points.Num();

	// If no point in curve, return the Default value we passed in.
	if( NumPoints == 0 )
	{
		return Default;
	}

	// If only one point, or before the first point in the curve, return the first points value.
	if( NumPoints < 2 || (InVal <= Points(0).KeyIn) )
	{
		return Points(0).KeyOut;
	}

	// If beyond the last point in the curve, return its value.
	if( InVal >= Points(NumPoints-1).KeyIn )
	{
		return Points(NumPoints-1).KeyOut;
	}

	// Somewhere with curve range - linear search to find value.
	for( INT i=1; i<NumPoints; i++ )
	{	
		if( InVal < Points(i).KeyIn )
		{
			const FLOAT Diff = Points(i).KeyIn - Points(i-1).KeyIn;

			if( Diff > 0.f && Points(i-1).IntepMode != CIM_Constant )
			{
				const FLOAT Alpha = (InVal - Points(i-1).KeyIn) / Diff;

				if( Points(i-1).IntepMode == CIM_Linear )
				{
					return Lerp( Points(i-1).KeyOut, Points(i).KeyOut, Alpha );
				}
				else
				{
					return CubicInterp( Points(i-1).KeyOut, Points(i-1).TangentOut * Diff, Points(i).KeyOut, Points(i).TangentIn * Diff, Alpha );
				}
			}
			else
			{
				return Points(i-1).KeyOut;
			}
		}
	}

	// Shouldn't really reach here.
	return Points(NumPoints-1).KeyOut;
}