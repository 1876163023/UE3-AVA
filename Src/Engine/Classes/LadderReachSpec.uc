//=============================================================================
// LadderReachSpec.
//
// A LadderReachSpec connects Ladder NavigationPoints in a LadderVolume
//
// Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class LadderReachSpec extends ReachSpec
	native;

cpptext
{
	virtual FPlane PathColor()
	{
		// light purple = ladder
		return FPlane(1.f,0.5f, 1.f,0.f);
	}
}

/** CostFor()
Returns the "cost" in unreal units
for Pawn P to travel from the start to the end of this reachspec
*/
native function int CostFor(Pawn P);

defaultproperties
{
	bCanCutCorners=false
}

