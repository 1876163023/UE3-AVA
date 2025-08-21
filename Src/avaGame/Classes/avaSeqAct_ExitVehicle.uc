/*
	Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved.

	2007/05/10	����
		UTSeqAct_ExitVehicle�� �״�� ����.
*/
class avaSeqAct_ExitVehicle extends SequenceAction;

//! Vehicle���� ���� SeatIndex(�⺻��=0).
var() int SeatIndex;
//! ���� SeatIndex�� �����ϰ� ��� Driver�� ������ ������ �Ѵ�.
var() bool bAllSeats;

defaultproperties
{
	ObjCategory="Pawn"
	ObjName="Exit Vehicle"
	ObjClassVersion=2

	bAllSeats=true
}
