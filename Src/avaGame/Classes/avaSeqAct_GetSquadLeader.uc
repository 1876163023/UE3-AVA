/*

	���ڰ� �־����ٸ� �־��� ���ڿ� ���ؼ� Squad Leader ���� �Ǵ��ؼ� Yes, No �� �Ǵ��� �ش�.
	���ڰ� ���ٸ� Squad Leader �� �����´�.

*/


class avaSeqAct_GetSquadLeader extends SequenceAction;

var()	int		TeamIndex;
var		Actor	CheckActor;
var		Actor	Leader;

event Activated()
{
	// CheckActor �� �ִٸ� CheckActor �� SquadLeader ���� �ƴ����� �Ǵ��ؼ� �˷��ֵ��� �Ѵ�..
	local PlayerController		P;
	local avaPlayerController	avaPC;
	local bool					bLeader;
	local WorldInfo				WI;

	bLeader = false;
	if ( CheckActor != None )
	{
		if ( avaPlayerController( CheckActor ) != None )	avaPC = avaPlayerController( CheckActor );
		else if ( avaPawn( CheckActor )	!= None )			avaPC = avaPlayerController( Pawn( CheckActor ).Controller );
		if ( avaPC != None )	bLeader = avaPlayerReplicationInfo ( avaPC.PlayerReplicationInfo ).IsSquadLeader();
	}
	else
	{
		WI = GetWorldInfo();
		foreach WI.AllControllers(class'PlayerController', P)
		{
			if ( avaPlayerReplicationInfo( P.PlayerReplicationInfo ).IsSquadLeader() )
			{
				if ( P.PlayerReplicationInfo.Team.TeamIndex == TeamIndex )
				{
					avaPC   = avaPlayerController( P );
					bLeader = true;
					break;
				}					
			}
		}
	}
	Leader = avaPC;
	GenerateImpulse( bLeader );
}

function GenerateImpulse(bool bMatched)
{
	if(bAutoActivateOutputLinks)	bAutoActivateOutputLinks = false;
	OutputLinks[ int(!bMatched) ].bHasImpulse = true;
}

defaultproperties
{
	ObjCategory="Pawn"
	ObjName="Get SquadLeader"
	bCallHandler=false
	bAutoActivateOutputLinks=false

	OutputLinks(0)=(LinkDesc="Yes")
	OutputLinks(1)=(LinkDesc="No")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Team",PropertyName=TeamIndex, MinVars=0, MaxVars=1 )
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Check Actor",PropertyName=CheckActor,MinVars=0, MaxVars=1 )
	VariableLinks(2)=(ExpectedType=class'SeqVar_Object',LinkDesc="Leader",PropertyName=Leader,MinVars=0, MaxVars=1, bWriteable=true )
}