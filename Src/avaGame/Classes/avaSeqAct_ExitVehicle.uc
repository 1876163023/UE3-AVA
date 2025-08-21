/*
	Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved.

	2007/05/10	고광록
		UTSeqAct_ExitVehicle를 그대로 들고옴.
*/
class avaSeqAct_ExitVehicle extends SequenceAction;

//! Vehicle에서 내릴 SeatIndex(기본값=0).
var() int SeatIndex;
//! 위의 SeatIndex를 무시하고 모든 Driver를 강제로 내리게 한다.
var() bool bAllSeats;

defaultproperties
{
	ObjCategory="Pawn"
	ObjName="Exit Vehicle"
	ObjClassVersion=2

	bAllSeats=true
}
