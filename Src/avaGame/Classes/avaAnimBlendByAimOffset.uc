/**
 * Copyright 2006 Ntix Soft, Inc. All Rights Reserved.
 * 
 *	Pawn 의 rataton 을 직접 접근하기 위한 Blender
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