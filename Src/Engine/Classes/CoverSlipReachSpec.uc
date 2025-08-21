/**
 * Copyright 1998 – 2006, Epic Games, Inc.
 */
class CoverSlipReachSpec extends ForcedReachSpec
	native;

native function int CostFor(Pawn P);

defaultproperties
{
	ForcedPathSizeName=Common
	bSkipPrune=TRUE
}