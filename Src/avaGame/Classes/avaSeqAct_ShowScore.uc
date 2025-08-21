//=============================================================================
//  avaSeqAct_ShowScore
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
//
//	2006/02/28
//		1. Client 의 Score Board 를 Show/Hide 시킨다.
//=============================================================================
class avaSeqAct_ShowScore extends SequenceAction;

var() bool	bShowScores;
var() bool	bResult;

event Activated()
{
	local PlayerController		P;
	local WorldInfo				WI;
	
	WI = GetWorldInfo();
	foreach WI.AllControllers(class'PlayerController', P)
	{
		avaPlayerController(P).ShowScores( bResult, bShowScores );
	}
}

defaultproperties
{
	bCallHandler	=	false
	ObjCategory		=	"HUD"
	ObjName			=	"Show Score"

	bShowScores		=	true

	VariableLinks(0)=(ExpectedType=class'SeqVar_Bool',LinkDesc="Show",PropertyName=bShow,MaxVars=1)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Bool',LinkDesc="Result",PropertyName=bResult,MaxVars=1)
}