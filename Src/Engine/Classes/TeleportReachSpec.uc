// represents the path of a teleporter
class TeleportReachSpec extends ReachSpec
	native;

native function int CostFor(Pawn P);

defaultproperties
{
	Distance=100.0
	bAddToNavigationOctree=false
}
