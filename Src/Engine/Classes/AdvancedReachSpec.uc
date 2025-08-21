//=============================================================================
// AdvancedReachSpec.
//
// An AdvancedReachspec can only be used by Controllers with bCanDoSpecial==true
//
// Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
class AdvancedReachSpec extends ReachSpec
	native;

cpptext
{
	virtual FPlane PathColor()
	{
		// purple path = advanced
		return FPlane(1.f,0.f,1.f, 0.f);
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

