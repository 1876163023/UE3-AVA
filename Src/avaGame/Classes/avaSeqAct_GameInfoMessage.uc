//=============================================================================
//  avaSeqAct_GameInfoMessage
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
//
//	2006/02/27
//		1. 게임진행에 대한 Message 를 표시하기 위한 Kismet Action 이다.
//=============================================================================
class avaSeqAct_GameInfoMessage extends SequenceAction;

var() int		TeamNum;			// Team 을 지정한다면, Team 에게만 Play 된다...
var() int		InfoIndex;			// GIM Index
var() int		OppInfoIndex;		// Opponent GIM Index


var() string	Info;				// 같은 Team 에게 보여 줄 Message
var() string	OppInfo;			// Oppenent Team 에서 보여 줄 Message
var() SoundCue	InfoSound;			// 같은 Team 에서 Play 할 Sound
var() SoundCue	OppInfoSound;		// Oppenent Team 에서 Play 할 Sound
var() float		MsgTime;			// Message 를 보여줄 시간
var	  Object	TargetObj;			// 특정 Target 에 대해서만 Message 와 Sound 를 Play 한다.
var() int		nType;				// GameInfoMessage Type

event Activated()
{
	local PlayerController	P;
	local PlayerController	PC;
	local WorldInfo			WI;
	
	local Actor A;
	local int	SelectedTeam;	


	WI = GetWorldInfo();
	// Get Team ...
	if (InputLinks[0].bHasImpulse)			// To All
	{
		SelectedTeam = -1;
	}
	else if (InputLinks[1].bHasImpulse)			// Get Team by Number
	{
		SelectedTeam = TeamNum;
	}
	else if (InputLinks[2].bHasImpulse)		//	Get Team by Actor
	{
		A = Actor(TargetObj);
		SelectedTeam = (A != None) ? int(A.GetTeamNum()) : 255;
	}
	else if (InputLinks[3].bHasImpulse)
	{
		`log( "Show GIM #3" @TargetObj );

		p = PlayerController(TargetObj);
		if (Pawn(TargetObj) != None)
			p = PlayerController(Pawn(TargetObj).Controller);

		if (P != none && InfoIndex != -1 )
		{
			`log( "Show GIM #3" @P @InfoIndex );
			avaPlayerController(P).ShowGIM( InfoIndex,,MsgTime );
		}		

		return;
	}
	else if(InputLinks[4].bHasImpulse)
	{
		`log( "Show GIM #4" @TargetObj );

		p = PlayerController(TargetObj);
		if (Pawn(TargetObj) != None)
			p = PlayerController(Pawn(TargetObj).Controller);

		if ( p != None && InfoIndex != -1 )
		{
			foreach WI.AllControllers(class'PlayerController', PC)
			{
				if ( p == PC )
					continue;
				`log( "Show GIM #4" @PC @InfoIndex );
				avaPlayerController(PC).ShowGIM( InfoIndex,,MsgTime );
			}
		}
		return;
	}

	

	// TeamNum 가 있는 경우
	// 같은 Team 에게만 Info 와 InfoSound 를 전송
	// 다른 Team 은 OppInfo 와 OppInfoSound 를 전송한다.
	if ( SelectedTeam == 0 || SelectedTeam == 1 )
	{
		foreach WI.AllControllers(class'PlayerController', P)
		{
			if ( P.PlayerReplicationInfo.Team.TeamIndex == SelectedTeam )
			{
				if ( InfoIndex != -1 )		avaPlayerController(P).ShowGIM( InfoIndex,,MsgTime );
			}
			else
			{
				if ( OppInfoIndex != -1 )		avaPlayerController(P).ShowGIM( OppInfoIndex,,MsgTime );
			}
		}
	}
	else
	{
		// TeamNum 가 0 이나 1이 아닌경우에는
		// 모두에게 Info 와 InfoSound 를 전송한다.	
		foreach WI.AllControllers(class'PlayerController', P)
		{
			if ( InfoIndex != -1 )		avaPlayerController(P).ShowGIM( InfoIndex,,MsgTime );
		}
	}
}

defaultproperties
{
	bCallHandler	=	false
	ObjCategory		=	"HUD"
	ObjName			=	"Game Information"
	ObjClassVersion	=	3

	TeamNum			=	2
	MsgTime			=	3.0
	InfoIndex		=	-1
	OppInfoIndex	=	-1

	InputLinks(0)=(LinkDesc="To All")
	InputLinks(1)=(LinkDesc="To Team(by Number)")
	InputLinks(2)=(LinkDesc="To Team(by Actor)")
	InputLinks(3)=(LinkDesc="To Player(by Actor)")
	InputLinks(4)=(LinkDesc="To Player(except Acotr)")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Number",PropertyName=TeamNum,MaxVars=1)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Actor",PropertyName=TargetObj,MaxVars=1)



}