//=============================================================================
// ProscribedReachSpec.
//
// A ProscribedReachSpec is forced off by the level designer
//
// Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class ProscribedReachSpec extends ReachSpec
	native;

cpptext
{
	virtual FPlane PathColor()
	{
		// red is reserved for proscribed paths
		return FPlane(1.f, 0.f, 0.f, 0.f);
	}

	virtual UBOOL IsProscribed() { return true; }
}

/** CostFor()
Returns the "cost" in unreal units
for Pawn P to travel from the start to the end of this reachspec
*/
native function int CostFor(Pawn P);

defaultproperties
{
	bAddToNavigationOctree=false
	bCanCutCorners=false
}
