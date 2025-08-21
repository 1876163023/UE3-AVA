/*

	인자가 주어진다면 주어진 인자에 대해서 Squad Leader 인지 판단해서 Yes, No 를 판단해 준다.
	인자가 없다면 Squad Leader 를 가져온다.

*/


class avaSeqAct_GetSquadLeader extends SequenceAction;

var()	int		TeamIndex;
var		Actor	CheckActor;
var		Actor	Leader;

event Activated()
{
	// CheckActor 가 있다면 CheckActor 가 SquadLeader 인지 아닌지를 판단해서 알려주도록 한다..
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