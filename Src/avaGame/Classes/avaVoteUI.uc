class avaVoteUI extends avaStateUI;


var const Color						DefaultColor;
var const Color						InfoMsgColor;
var const float						DefaultLifeTime;

var globalconfig	int				MinimumVoters;			// �ڽ��� �����ϰ� ��ǥ�� �� �ִ� Minimum Voters, �� �ο��� �������� ������ ������ �� ����....

var int								KickablePlayerPage;
var int								KickPlayerIdx;			// Kick �ϰ��� �ϴ� KickablePlayerList �� Player Index
var int								KickPlayerReasonIdx;	// Kick �ϰ��� �ϴ� ����� Kick ����
var float							VoteRestrictionTime;	// Vote ��û �� ���û�� ���Ǵ� �ð�


function InitKickablePlayerList()
{
	avaPlayerController( GetLocalPC() ).InitKickablePlayerList( MinimumVoters );
}


// Input �� �޴´�... Vote �� Entry Point!!!
// Input �������� �ٷ� Vote�� ���� �ʴ� ���� Vote �� ������ �� ���� �ֱ� �����ΰ��ΰ�????
// ���� Vote �� ���������� �� �� �ִٸ� ���ڰ� �Ÿ����� �ϴ°� ������ ����...
// avaVoteUI �� ���ǿ� UI �θ� ����ϵ��� ����. ��ǥ�� UI�� ���� ����°� ���� ������????
function bool OnInputKey(int ControllerID, Name Key, int Number, EInputEvent Event)
{
	if(! IsEnable() )
		return false;

	if ( Key == 'F7' && bEnableVoteUI )
	{
		BeginKickVote();
		return true;
	}
	return false;
}

// Kick ���Ǹ� ���� Vote UI �� �����Ѵ�....
function bool BeginKickVote()
{
	if ( PeekMenuName() == "VoteKickPlayer" )
	{
		ClearMenu();
	}
	else
	{
		InitKickablePlayerList();
		if ( avaPlayerController( GetLocalPC() ).KickablePlayerList.length > 0 )
		{
			KickablePlayerPage = 0;
			bEnableQuickChat = false;
			PushMenu("VoteKickPlayer");
		}
	}
	return true;
}

function Initialized()
{
	Super.Initialized();
	AddState("VoteKickPlayer", OnKeyVoteKickPlayer, OnBeginVoteKickPlayer, OnEndState);
	AddState("VoteKickReason", OnKeyVoteKickReason, OnBeginVoteKickReason, OnEndState);
}

// Team Vote �� UI ����...
function OnBeginVoteKickPlayer( string PrevStateName )
{
	ComposeKickableMenu();
}

function ComposeKickableMenu()
{
	local array<string>	Msg;
	local int			KickableIdx;
	local int			i;
	Msg[Msg.length] =	class'avaVoteMessage'.default.VoteInfoMessage;
	AddMessage( Msg, 0, InfoMsgColor );
	Msg.length =	0;
	Msg[Msg.length]	=	class'avaVoteMessage'.default.VoteKickSelectPlayer;
	for ( i = 0 ; i < 8 ; ++ i )
	{
		KickableIdx = KickablePlayerPage * 8 + i;
		if ( KickableIdx >= avaPlayerController( GetLocalPC() ).KickablePlayerList.length )	break;
		Msg[Msg.length]	=	""$(i+1)$"."@avaPlayerController( GetLocalPC() ).KickablePlayerList[KickableIdx].PlayerName;
	}
	if ( KickablePlayerPage == 0 && avaPlayerController( GetLocalPC() ).KickablePlayerList.length > 8 )	Msg[Msg.length]	=	"9."@class'avaLocalizedMessage'.default.NextPage;
	else if ( KickablePlayerPage == 1 )								Msg[Msg.length]	=	"9."@Class'avaLocalizedMessage'.default.PrevPage;
	Msg[Msg.length]	=	"0."@class'avaLocalizedMessage'.default.Cancel;
	AddMessage( Msg, 0, DefaultColor, false );
}

function bool OnKeyVoteKickPlayer(int ControllerID, Name Key, int Number, EInputEvent Event)
{
	local int idx;
	OnInputKey(ControllerID, Key, Number, Event);
	if ( !IsEnable() )	return false;
	if ( Number >= 1 && Number <= 8 )
	{
		idx = KickablePlayerPage * 8 + Number - 1 ; 
		if ( idx >= 0 && idx < avaPlayerController( GetLocalPC() ).KickablePlayerList.length )
		{
			KickPlayerIdx = idx;	
			PushMenu( "VoteKickReason" );
		}
		return true;
	}
	else if ( Number == 9 )
	{
		if ( ( KickablePlayerPage == 0 && avaPlayerController( GetLocalPC() ).KickablePlayerList.length > 8 ) || KickablePlayerPage == 1 )
		{
			KickablePlayerPage = (KickablePlayerPage+1)%2;
			ComposeKickableMenu();
		}
		return true;
	}
	else if ( Number == 0 )
	{
		ClearMenu();
		return true;
	}
	return false;
}

function OnBeginVoteKickReason( string PrevStateName )
{
	local array<string>	Msg;
	local int			i;

	Msg[Msg.length]	=	""$avaPlayerController( GetLocalPC() ).KickablePlayerList[KickPlayerIdx].PlayerName$""@class'avaVoteMessage'.default.VoteKickSelectReason;
	for ( i = 0 ; i < 4 ; ++ i )
	{
		Msg[Msg.length]	=	""$(i+1)$"."@class'avaVoteMessage'.default.VoteKickReason[i];
	}
	Msg[Msg.length]	=	"0."@class'avaLocalizedMessage'.default.Cancel;
	AddMessage( Msg, 0, DefaultColor );	
}

function bool OnKeyVoteKickReason(int ControllerID, Name Key, int Number, EInputEvent Event)
{
	OnInputKey(ControllerID, Key, Number, Event);
	if ( !IsEnable() )	return false;
	switch ( Number )
	{
	case 1:	case 2:	case 3:	case 4:
		KickPlayerReasonIdx = (Number-1);
		RequestKickVote();
		ClearMenu();
		return true;
		break;
	case 0:	// Cancel
		PopMenu();
		return true;
		break;
	}
	return false;
}

// Host ��  Kick Vote �� ��û�Ѵ�....
function RequestKickVote()
{
	local avaPlayerController		LocalPC;
	LocalPC		= avaPlayerController( GetLocalPC() );
	avaGameReplicationInfo( GetLocalPC().WorldInfo.GRI ).RequestKickVote( LocalPC, avaPlayerController( GetLocalPC() ).KickablePlayerList[KickPlayerIdx].PRI, KickPlayerReasonIdx );
}

function OnEndState(string NextStateName)
{
	ClearMessage();
//	`Log("ClearMessage");
}

function bool IsEnable()
{
	Local int MyNetLocation;

	if(!bOpened || GetLocalPC() == None)
		return false;
	
	MyNetLocation = class'avaNetHandler'.static.GetAvaNetHandler().CheckMyLocation();
	// MyLocation '3' means that you're playing games.
	if( MyNetLocation != 3 && MyNetLocation != 0 )
	{
		ClearMenu();
		return false;
	}

	return true;	
}

function ClearMenu()
{
	bEnableQuickChat = true;
	Super.ClearMenu();
}






















//
//
//
//
//
//
//
//
//
//function RequestVote(int VoteRange)
//{
//	avaPlayerController(GetLocalPC()).RequestVote(VoteRange);	
//}
//
//function ResponseVote(int VoteRange, bool bAvailable,bool bOtherTeam, bool bInvolved ,float TimeLeft, string PlayerName)
//{
//	`log( "avaVoteUI.ResponseVote" @VoteRange @bAvailable @bOtherTeam @bInvolved @TimeLeft @PlayerName );
//	if( VoteRange == VOTERANGE_TEAM)
//	{
//		if(bAvailable)
//			PushMenu("VoteTeam");
//		else
//		{
//			if(PlayerName == GetLocalPC().PlayerReplicationInfo.PlayerNAme)
//				GetLocalPC().ConsoleCommand("Revoke Vote");
//			else if(!bInvolved)
//				DrawRangeAnother(VoteRange, true, TimeLeft, bOtherTeam);
//		}
//	}
//	else if( VoteRange == VOTERANGE_ALL)
//	{
//		if( ! bAvailable )
//			DrawRangeAnother(VoteRange, true, TimeLeft,false);
//		else
//		{
//			if(bOtherTeam)
//				DrawRangeAnother(VoteRange, true, TimeLeft,true);								
//			else	
//				PushMenu("VoteAll");
//		}
//	}
//}
//
//function DrawRangeAnother(int VoteRange, bool bCandidate, float TimeLeft, bool bOtherTeam)
//{
//	Local avaMsgParam Param;
//
//	Param = class'avaMsgParam'.static.Create();
//	Param.SetInt(VoteRange);
//	Param.SetFloat(TimeLeft);
//	Param.SetBool(bCandidate, bOtherTeam);
//
//	avaPlayerController(GetLocalPC()).ReceiveParam(class'avaVoteMessage', VOTEPROGRESS_Another, Param);
//}
//
//
//
//
//
//
//
//function bool OnKeyVoteTeam(int ControllerID, Name Key, int Number, EInputEvent Event)
//{
//	Local avaPlayerReplicationInfo PRI;
//
//	OnInputKey(ControllerID, Key, Number, Event);
//	if( ! IsEnable() )
//		return false;
//
//	PRI = avaPlayerReplicationInfo(GetLocalPC().PlayerReplicationInfo);
//	
//	switch(Number)
//	{
//	case 1:	
//		ClearMenu();
//		GetLocalPC().ConsoleCommand("Candidate");						break;
//	case 2:
//		if(PRI.bSquadLeader)
//			PushMenu("VoteRetire");										break;
//	case 0:	
//		ClearMenu();													break;
//	}
//
//	if( 0 <= Number && Number <= 9)
//		return true;
//
//	return false;
//}
//function OnBeginVoteAll(string PrevStateName)
//{
//	Local array<string> Msg;
//	Local int i;
//
//
//	Msg.Length = 2 + 1;
//	for( i = 0 ; i < Msg.Length - 1 ; i++ )
//		Msg[i] = (i == 0 ? "" : i$".")@class'avaVoteMessage'.default.VoteAllMenu[i];
//	
//	Msg[ Msg.Length - 1 ] = "0."@class'avaLocalizedMessage'.default.Cancel;
//	AddMessage(Msg, 0, DefaultColor);
//}
//
//function bool OnKeyVoteAll(int ControllerID, Name Key, int Number, EInputEvent Event)
//{
//	OnInputKey(ControllerID, Key, Number, Event);
//	if( ! IsEnable() )
//		return false;
//
//	switch(Number)
//	{
//	case 1:	
//		ClearMenu();
//		GetLocalPC().ConsoleCommand("DrawGame");
//																		break;
//	case 0:	
//		ClearMenu();													break;
//	}
//
//	if( 0 <= Number && Number <= 9)
//		return true;
//
//	return false;
//}
//
//function OnBeginVoteRetire(string PrevStateName)
//{
//	Local array<string> Msg;
//	Local int i;
//
//	Msg.Length = 3;
//	for( i = 0 ; i < Msg.Length ; i++ )
//		Msg[i] = (i == 0 ? "" : i$".")@class'avaVoteMessage'.default.VoteRetireMenu[i];
//	
//	AddMessage(Msg, 0, DefaultColor);
//}
//
//function bool OnKeyVoteRetire(int ControllerID, Name Key, int Number, EInputEvent Event)
//{
//
//	OnInputKey(ControllerID, Key, Number, Event);
//	
//	if( ! IsEnable() )
//		return false;
//
//	switch(Number)
//	{
//	case 1:	
//		ClearMenu();
//		GetLocalPC().ConsoleCommand("Retire SquadLeader");				break;
//	case 2:
//		ClearMenu();
//	case 0:	
//		ClearMenu();													break;
//	}
//
//	if( 0 <= Number && Number <= 9)
//		return true;
//
//	return false;
//}
//
//function BroadcastTeam(Class<avaLocalMessage> MessageClassName, int Switch, optional avaMsgParam Param)
//{
//	avaPlayerController(GetLocalPC()).BroadcastTeamParam(MessageClassName, Switch, Param);
//}
//
//
//
//
//
//
//// @deprecated. ��� ��������� Native�� �Ű���.
//function RenderMenu( Canvas Canvas )
//{
//	SetRenderPos(Canvas.ClipX/2 - 50, 30);
//	Super.RenderMenu( Canvas );
//}
//
function NotifyPawnDied()
{
	//ClearMenu();
 //	ClearMessage();
}

function bool CheckInputCondition()
{
	Local avaPlayerController PC;
	Local avaHUD HUD;
	Local UIInteraction UIController;

	PC = avaPlayerController(GetLocalPC());

	if( PC == None )
		return false;
	
	HUD = avaHUD(PC.MyHUD);
	if( HUD == None )
		return false;

	UIController = LocalPlayer(PC.Player).ViewportClient.UIController;

	if( UIController != None && 
		(UIController.FindSceneByTag( avaHUD(PC.MyHUD).ChatScene.SceneTag ) != none || 
		UIController.FindSceneByTag( avaHUD(PC.MyHUD).TeamChatScene.SceneTag ) != none) )
		return false;

	return true;
}

defaultproperties
{
	VoteRestrictionTime		= 210.0
	DefaultLifeTime			= 3.0
	DefaultColor			= (R=248,G=197,B=103,A=255)
	InfoMsgColor			= (R=255,G=0,B=0,A=255)
}