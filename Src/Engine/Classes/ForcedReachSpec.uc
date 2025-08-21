//=============================================================================
// ForcedReachSpec.
//
// A ForcedReachspec is forced by the level designer
//
// Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class ForcedReachSpec extends ReachSpec
	native;

cpptext
{
	virtual FPlane PathColor()
	{
		// yellow for forced paths
		return FPlane(1.f, 1.f, 0.f, 0.f);
	}

	virtual UBOOL IsForced() { return true; }
	virtual UBOOL PrepareForMove( AController * C );
}

/** CostFor()
Returns the "cost" in unreal units
for Pawn P to travel from the start to the end of this reachspec
*/
native function int CostFor(Pawn P);

defaultproperties
{
	ForcedPathSizeName=Max
}

