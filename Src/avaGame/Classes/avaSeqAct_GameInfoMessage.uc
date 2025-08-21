//=============================================================================
//  avaSeqAct_GameInfoMessage
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
//
//	2006/02/27
//		1. �������࿡ ���� Message �� ǥ���ϱ� ���� Kismet Action �̴�.
//=============================================================================
class avaSeqAct_GameInfoMessage extends SequenceAction;

var() int		TeamNum;			// Team �� �����Ѵٸ�, Team ���Ը� Play �ȴ�...
var() int		InfoIndex;			// GIM Index
var() int		OppInfoIndex;		// Opponent GIM Index


var() string	Info;				// ���� Team ���� ���� �� Message
var() string	OppInfo;			// Oppenent Team ���� ���� �� Message
var() SoundCue	InfoSound;			// ���� Team ���� Play �� Sound
var() SoundCue	OppInfoSound;		// Oppenent Team ���� Play �� Sound
var() float		MsgTime;			// Message �� ������ �ð�
var	  Object	TargetObj;			// Ư�� Target �� ���ؼ��� Message �� Sound �� Play �Ѵ�.
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

	

	// TeamNum �� �ִ� ���
	// ���� Team ���Ը� Info �� InfoSound �� ����
	// �ٸ� Team �� OppInfo �� OppInfoSound �� �����Ѵ�.
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
		// TeamNum �� 0 �̳� 1�� �ƴѰ�쿡��
		// ��ο��� Info �� InfoSound �� �����Ѵ�.	
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