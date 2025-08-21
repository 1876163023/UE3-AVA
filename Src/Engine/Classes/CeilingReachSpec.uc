class CeilingReachSpec extends ReachSpec
	native;

native function int CostFor(Pawn P);
native function int AdjustedCostFor( Pawn P, NavigationPoint Anchor, NavigationPoint Goal, int Cost );

defaultproperties
{
}