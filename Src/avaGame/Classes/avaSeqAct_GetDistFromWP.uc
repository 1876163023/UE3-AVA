/*

	WayPoint 로 부터의 거리를 구해준다.

*/

class avaSeqAct_GetDistFromWP extends SequenceAction;

var float	Distance;
var Actor	Target;

event Activated()
{
	local avaGameReplicationInfo	avaGRI;
	local vector					targetPos;
	local pawn						targetPawn;
	local int						targetTeam;
	local int						targetWP;
	local bool						bMatch;

	bMatch = false;
	if ( Target != None )
	{
		if ( Controller(Target) != None )	targetPawn = Controller(Target).pawn;
		else if ( Pawn( Target ) != None )	targetPawn = Pawn(Target);

		if ( TargetPawn != None )
		{
			targetTeam	= targetPawn.GetTeamNum();
			targetWP	= WPTeam_Blue;	
			if ( targetTeam >= 0 && targetTeam <= 2 )
			{
				if (InputLinks[2].bHasImpulse || InputLinks[3].bHasImpulse )	targetTeam = WPTeam_Yellow;
				if (InputLinks[1].bHasImpulse || InputLinks[3].bHasImpulse )	targetWP = WPTeam_Yellow;

				avaGRI = avaGameReplicationInfo ( GetWorldInfo().GRI );
				targetPos = avaGRI.CurrentWaypoint[targetTeam * WPTEAM_MAX + targetWP];
				if ( VSize( targetPos ) != 0 )
				{
					Distance = VSize( targetPos - TargetPawn.Location );
					bMatch = true;
				}
			}
		}
	}
	GenerateImpulse(bMatch);
}

function GenerateImpulse(bool bMatched)
{
	if(bAutoActivateOutputLinks)	bAutoActivateOutputLinks = false;
	OutputLinks[ int(!bMatched) ].bHasImpulse = true;
}

defaultproperties
{
	ObjCategory="Pawn"
	ObjName="Distance To WayPoint"
	bCallHandler=false
	bAutoActivateOutputLinks=false

	InputLinks(0)=(LinkDesc="Blue")
	InputLinks(1)=(LinkDesc="Yellow")
	InputLinks(2)=(LinkDesc="Opp-Blue")
	InputLinks(3)=(LinkDesc="Opp-Yellow")

	OutputLinks(0)=(LinkDesc="Yes")
	OutputLinks(1)=(LinkDesc="No")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Target",PropertyName=Target, MinVars=1, MaxVars=1 )
	VariableLinks(1)=(ExpectedType=class'SeqVar_Float',LinkDesc="Distance",PropertyName=Distance, MinVars=0, MaxVars=1, bWriteable=true )
}