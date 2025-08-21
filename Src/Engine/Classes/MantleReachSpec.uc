class MantleReachSpec extends ForcedReachSpec
	native;

native function int CostFor(Pawn P);

defaultproperties
{
	ForcedPathSizeName=Common
	bSkipPrune=TRUE
}