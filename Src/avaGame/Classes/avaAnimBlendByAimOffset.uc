/**
 * Copyright 2006 Ntix Soft, Inc. All Rights Reserved.
 * 
 *	Pawn �� rataton �� ���� �����ϱ� ���� Blender
 * 
  */

class avaAnimBlendByAimOffset extends AnimNodeAimOffset
	native;

var() int	SeatIndex;

cpptext
{
	virtual FVector2D GetAim();
}

defaultproperties
{
	SeatIndex = -1
}