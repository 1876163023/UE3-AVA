class avaVoteMessage extends avaLocalMessage;

var const Color	ConsoleColor;
var const float LifeSpan;

var localized string	VoteInfoMessage;			//	"강퇴 안내 Message"
var localized string	VoteKickSelectPlayer;		//	"강제 퇴장 유저 선택" Message
var localized string	VoteKickSelectReason;		//	"강제 퇴장 이유 선택" Message
var localized string	VoteKickReason[4];			//	"강제 퇴장 이유"	불법 프로그램 사용, 버그 악용, 욕설 및 비방, 기타
var localized string	VoteKickTitle;				//	"%s 님께서 %s 님을 %s 의 이유로 강제퇴장을 신청하였습니다."
var localized string	VoteWarn;					//  경고 Message
var localized string	VotingMsg;					//	동의하십니까? 반대[F11]/ 찬성[F12]
var localized string	VoteCount;					//	"찬성 %d표/반대 %d표"
var localized string	VoteAccept;					//	%s 님이 찬성하셨습니다.
var localized string	VoteDeny;					//	%s 님이 반대하셨습니다.
var localized string	VoteKickAccepted;			//	%s 님이 투표에 의해서 강제퇴장 되었습니다.
var localized string	VoteKickDenied;				//	%s 님의 강제퇴장 투표가 부결 되었습니다.
var localized string	VoteLeftTime;				//	남은시간 %n 초:

var localized string	MSG_VOTE_PROGRESS;			//	현재 다른 투표가 진행중입니다.
var localized string	MSG_VOTE_DISABLE;			//	투표요청후 3분30초 안에 다시 요청할 수 없습니다.
var localized string	MSV_VOTE_INSUFFICIENTVOTERS;				//	투표 인원이 부족합니다.
var localized string	MSG_VOTE_INSUFFICIENTMONEY;					//	소지금부족으로 투표할 수 없음.
var localized string	MSG_VOTE_INSUFFICIENTMONEY_FOR_PROPOSAL;	//	소지금부족으로 투표할 수 없음.




//var localized string VoteTeamMenu[3];
//var localized string VoteAllMenu[2];
//var localized string VoteRetireMenu[3];
//
//var localized string SquadLeader_Invalid;	//"하사 계급 이상만 분대장 지원이 가능합니다."
//var localized string SquadLeader_Already;	//"당신은 이미 분대장입니다."
//var localized string SquadLeader_Freeze;	//"다음 입후보가능 시간까지 Seconds 초가 남았습니다."
//var localized string SquadLeader_Vote[2];	//"분대장 선출투표( Seconds 초 남음)"
////var localized string SquadLeader_Vote[1];	//"PlayerName 님이 분대장에 지원하였습니다."
//var localized string SquadLeader_Elected[3];	//"분대장 선출 투표 종료!"
////var localized string SquadLeader_Elected[1];	//"PlayerName 님이 분대장으로 임명되었습니다."
//var localized string SquadLeader_Vote2[2];	//"분대장 선출투표( Seconds 초 남음)"
////var localized string SquadLeader_Vote2[1];	//"PlayerName 님이 차기 분대장에 지원하였습니다."
//var localized string SquadLeader_Timeout;	//"시간내에 과반수를 넘지 못했습니다."
//
//var localized string SquadLeader_Rejected[3];	//"분대장 선출 투표 종료"
////var localized string SquadLeader_Rejected[1];	//"%s 님이 분대장이 되지 못했습니다"
//
//var localized string SquadLeader_InSufficient;	//"투표 인원이 부족합니다."
//var localized string SquadLeader_Another[2];	//"현재 다른 투표가 진행중입니다."
////var localized string SquadLeader_Another[1];	//"진행중인 투표 남은 시간 : %t 초"
//
//var localized string SquadLeader_VoteStatus;	//"현재 %s %n 표 : %s %n 표"
//var localized string SquadLeader_Retire;		//"%s 님이 분대장에서 물러났습니다."
//
//
//var localized string VoteSystem_Insufficient;	//"투표 인원이 부족합니다."
//var localized string VoteSystem_VoteStatus;	//"현재 찬성 %n 표 : 반대 %n 표"
//var localized string VoteSystem_VoteResult;	//"찬성 %n 표 : 반대 %n 표"
//
//var localized string VoteSystem_Accept;	//"F5 : 찬성"
//var localized string VoteSystem_Deny;	//"F6 : 반대"
//
//
//var localized string DrawGame_Invalid;	//"비김을 제안할 상황이 아닙니다."
//var localized string DrawGame_Already;	//"이미 게임을 비기기로 합의하였습니다."
//var localized string DrawGame_Insufficient;	//"투표 인원이 부족합니다."
//var localized string DrawGame_Another[2];	//"현재 다른 투표가 진행중입니다"
////var localized string DrawGame_Another[1];	//"진행중인 투표 남은 시간 : %t 초"
//var localized string DrawGame_Freeze;	//"비기기 제안을 Seconds 초후 해주십시오"
//var localized string DrawGame_Vote[2];	//"게임 비기기 투표 ( Seconds 초 남음)"
////var localized string DrawGame_Vote[1];	//"PlayerName 님이 게임 중단을 요청하였습니다."
//
//var localized string DrawGame_Elected[2];	//"게임 비기기"
////var localized string DrawGame_Elected[1];	//"게임을 중단하고 비긴게임으로 하기로 했습니다."
//
//var localized string DrawGame_Rejected[2];		//"게임 비기기"
////var localized string DrawGame_Rejected[1];		//"비긴 게임으로 하지 않기로 하였습니다."
//var localized string DrawGame_Timeout;	//"시간내에 과반수를 넘지 못했습니다."
//var localized string DrawGame_VoteStatus;	//"우리팀 찬성 %n 표 : 반대 %n 표"
//var localized string DrawGame_VoteResult;	//"EU 팀 %s / NRF 팀 %s"
//
//var localized string VoteSystem_AcceptHead;		//"F5 : "
//var localized string VoteSystem_DenyHead;		//"F6 : "
//var localized string SquadLeader_Cancel;		//"%s 님이 신청한 투표를 취소하셨습니다."
//var localized string SquadLeader_CancelMenu;	//"F7 : 신청한 투표 중단"
//
//var localized string SquadLeader_Submit[2];		//"분대장 선출투표( %t 초 남음)"
////var localized string SquadLeader_Submit[1];		//"투표에 참여하였습니다."
//var localized string DrawGame_Submit[2];		//"게임 비기기 투표 ( %t 초 남음)"
////var localized string DrawGame_Submit[1];		//"투표에 참여하였습니다."
//
//var localized string VoteSystem_Another;	//"현재 투표가 진행 중입니다."
//var localized string VoteSystem_AnotherTimeLeft;	//"진행중인 투표 남은 시간 : %t 초."
//var localized string VoteSystem_AnotherOtherTeam;	//"현재 다른 팀 내에서 투표가 진행 중입니다."
//
//var localized string SquadLeader_CandidateLogout;	//"분대장 후보자가 나가서 분대장 선출이 취소되었습니다."
//var localized string SquadLeader_VoterLogout;		//"정원 미달로 분대장 선출이 취소되었습니다."
//
//var localized string DrawGame_CandidateLogout;	//" 제안자가 나가서 게임 비기기가 취소되었습니다."
//var localized string DrawGame_VoterLogout;	//"정원 미달로 게임 비기기가 취소되었습니다."
//var localized string DrawGame_Cancel;		//"%s 님이 팀비기기를 취소하였습니다"
//
//var localized string VoteSystem_Logout;		//"%s 님이 게임에서 나갔습니다."
//
//enum EVoteSoundType
//{
//	VOTESOUND_Start,
//	VOTESOUND_End,
//	VOTESOUND_Cancel,
//	VOTESOUND_Yes,
//	VOTESOUND_No,
//};
//
//static function ClientReceive(
//	PlayerController P,
//	optional int Switch,
//	optional PlayerReplicationInfo RelatedPRI_1,
//	optional PlayerReplicationInfo RelatedPRI_2,
//	optional Object OptionalObject
//	)
//{
//
//	Local int VoteProgress;
//
//	Assert(avaMsgParam(OptionalObject) != None);
//	if(avaMsgParam(OptionalObject) == None )
//		return;
//
//	VoteProgress = avaMsgParam(OptionalObject).FloatParam[1];
//	// Play Sound
//	switch( VoteProgress )
//	{
//	case VOTEPROGRESS_EventStart:	Play( "S",RelatedPRI_1, P);	break;
//	case VOTEPROGRESS_EventEnd:	Play( "E",RelatedPRI_1, P);	break;
//	case VOTEPROGRESS_EventCancel:	Play( "C",RelatedPRI_1, P);	break;
//	case VOTEPROGRESS_EventAccept:	Play( "Y",RelatedPRI_1, P);	break;
//	case VOTEPROGRESS_EventDeny:	Play( "N",RelatedPRI_1, P);	break;
//	default: break;
//	}
//
//	switch( Switch )
//	{
//	case VOTESUBJECT_DrawGame:			
//		ReceiveDrawGame(P, VoteProgress, RelatedPRI_1, RelatedPRI_2, OptionalObject);	break;
//
//	case VOTESUBJECT_SquadLeader_EU:
//	case VOTESUBJECT_SquadLeader_USSR:
//		ReceiveLeaderVote(P, VoteProgress, RelatedPRI_1, RelatedPRI_2, OptionalObject);	break;
//
//	default:
//		Assert(false);	break;
//	}
//}
//
//static function ReceiveLeaderVote(PlayerController P,
//	optional int Switch,
//	optional PlayerReplicationInfo RelatedPRI_1,
//	optional PlayerReplicationInfo RelatedPRI_2,
//	optional Object OptionalObject)
//{
//	Local avaVoteUI VoteUI;
//	Local array<string> Msg;
//
//	Local bool bCandidate, bOtherTeamVote;
//	Local float fTimeLeft;
//	Local String PlayerName, AnotherName;
//	Local int VoteCount[2];
//	Local int VoteRange;
//	Local avaMsgParam Param;
//
//	Param = avaMsgParam(OptionalObject);
//	bCandidate = Param.BoolParam1;
//	bOtherTeamVote = Param.BoolParam2;
//	fTimeLeft = Param.FloatParam[0];
//	PlayerName = Param.StringParam[0];
//	AnotherName = Param.StringParam[1];
//	VoteCount[0] = Param.IntParam[0];
//	VoteCount[1] = Param.IntParam[1];
//	VoteRange = Param.IntParam[0];
//
//	Assert(LocalPlayer(P.Player) != None);
//	Assert(avaGameViewportClient(LocalPlayer(P.Player).ViewportClient) != None);
//	VoteUI = avaGameViewportClient(LocalPlayer(P.Player).ViewportClient).VoteUI;
//
//	Msg.Length = 5;			// an arbitrary value such as MAX_CHAR
//
//	switch(Switch)
//	{	
//	case VOTEPROGRESS_Invalid:		Msg[0] = default.SquadLeader_Invalid;
//		break;
//	case VOTEPROGRESS_Insufficient:	Msg[0] = default.SquadLeader_Insufficient;
//		break;
//	case VOTEPROGRESS_Cancel:		Msg[0] = class'avaStringHelper'.static.Replace(PlayerName, "%s", default.SquadLeader_Cancel);
//		break;
//	case VOTEPROGRESS_Retire:		Msg[0] = class'avaStringHelper'.static.Replace(PlayerName, "%s", default.SquadLeader_Retire);
//		break;
//	case VOTEPROGRESS_Another:			
//		if( VoteRange == VOTERANGE_TEAM)
//		{	
//			Msg[0] = default.VoteSystem_Another;		
//			Msg[1] = class'avaStringHelper'.static.Replace( int(fTimeLeft),"%t", default.VoteSystem_AnotherTimeLeft);
//		}
//		else if (VoteRange == VOTERANGE_ALL)
//		{
//			Msg[0] = bOtherTeamVote ? default.VoteSystem_AnotherOtherTeam : default.VoteSystem_Another;
//			Msg[1] = class'avaStringHelper'.static.Replace(int(fTimeLeft), "%t",default.VoteSystem_AnotherTimeLeft);
//		}
//		break;
//	case VOTEPROGRESS_Already:		Msg[0] = default.SquadLeader_Already;							
//		break;
//	case VOTEPROGRESS_Freeze:		Msg[0] = class'avaStringHelper'.static.Replace(int(fTimeLeft),"%t",default.SquadLeader_Freeze);
//		break;
//	case VOTEPROGRESS_Elected:
//		Msg[0]=(default.SquadLeader_Elected[0]);
//		if(AnotherName == "")
//			Msg[1] = class'avaStringHelper'.static.Replace(PlayerName,"%s",default.SquadLeader_Elected[1]);
//		else
//			Msg[1] = class'avaStringHelper'.static.Replace(PlayerName,"%s",default.SquadLeader_Elected[2]);
//		Msg[2] = class'avaStringHelper'.static.Replace(VoteCount[0],"%n", default.VoteSystem_VoteResult);
//		Msg[2] = class'avaStringHelper'.static.Replace(VoteCount[1],"%n", Msg[2]);
//		break;
//	case VOTEPROGRESS_Timeout:
//	case VOTEPROGRESS_Rejected:
//		Msg[0]=(default.SquadLeader_Rejected[0]);
//		if(AnotherName == "")
//			Msg[1] = class'avaStringHelper'.static.Replace(PlayerName,"%s",default.SquadLeader_Rejected[1]);
//		else
//			Msg[1] = class'avaStringHelper'.static.Replace(AnotherName,"%s",default.SquadLeader_Rejected[2]);
//		Msg[2] = class'avaStringHelper'.static.Replace(VoteCount[0],"%n", default.VoteSystem_VoteResult);
//		Msg[2] = class'avaStringHelper'.static.Replace(VoteCount[1],"%n", Msg[2]);
//		break;
//	case VOTEPROGRESS_Vote:
//		Msg[0] = class'avaStringHelper'.static.Replace(int(fTimeLeft),"%t",default.SquadLeader_Vote[0]);			
//		if(bCandidate)
//		{
//			if(AnotherName == "")
//				Msg[1] = class'avaStringHelper'.static.Replace(PlayerName,"%s", AnotherName == "" ? default.SquadLeader_Vote[1] : default.SquadLeader_Vote2[1]);
//			Msg[2] = default.SquadLeader_CancelMenu;
//			Msg[3] = class'avaStringHelper'.static.Replace(VoteCount[0],"%n", default.VoteSystem_VoteStatus);
//			Msg[3] = class'avaStringHelper'.static.Replace(VoteCount[1], "%n", Msg[3]);
//		}
//		else
//		{
//			if(AnotherName == "")
//			{
//				Msg[1] = class'avaStringHelper'.static.Replace(PlayerName,"%s",default.SquadLeader_Vote[1]);
//				Msg[2] = default.VoteSystem_AcceptHead$default.VoteSystem_Accept;
//				Msg[3] = default.VoteSystem_DenyHead$default.VoteSystem_Deny;
//				Msg[4] = class'avaStringHelper'.static.Replace(VoteCount[0], "%n", default.VoteSystem_VoteStatus);
//				Msg[4] = class'avaStringHelper'.static.Replace(VoteCount[1], "%n", Msg[4]);			
//			}
//			else
//			{
//				Msg[1] = class'avaStringHelper'.static.Replace(PlayerName, "%s",default.SquadLeader_Vote2[1]);
//				Msg[2]=default.VoteSystem_AcceptHead$PlayerName;
//				Msg[3]=default.VoteSystem_DenyHead$AnotherName;
//				Msg[4] = class'avaStringHelper'.static.Replace(VoteCount[0],"%n", default.SquadLeader_VoteStatus);
//				Msg[4] = class'avaStringHelper'.static.Replace(VoteCount[1], "%n", Msg[4]);	
//				Msg[4] = class'avaStringHelper'.static.Replace(PlayerName,"%s",Msg[4]);
//				Msg[4] = class'avaStringHelper'.static.Replace(AnotherName,"%s",Msg[4]);
//			}		
//		}
//		break;
//	case VOTEPROGRESS_Submit:		Msg[0] = class'avaStringHelper'.static.Replace(int(fTimeLeft),"%t",default.SquadLeader_Submit[0]);
//									Msg[1] = default.SquadLeader_Submit[1];	
//									Msg[2] = class'avaStringHelper'.static.Replace(VoteCount[0],"%n", default.VoteSystem_VoteResult);
//									Msg[2] = class'avaStringHelper'.static.Replace(VoteCount[1],"%n", Msg[2]);	
//		break;
//	case VOTEPROGRESS_Logout:		Msg[0] = class'avaStringHelper'.static.Replace(PlayerName,"%s",default.VoteSystem_Logout);
//									Msg[1] = bCandidate ? default.SquadLeader_CandidateLogout : default.SquadLeader_VoterLogout;	
//		break;
//	
//	default:
//		break;
//	}
//
//	VoteUI.AddMessage(Msg, default.LifeSpan, default.ConsoleColor);
//}
//
//static function ReceiveDrawGame(PlayerController P,
//	optional int Switch,
//	optional PlayerReplicationInfo RelatedPRI_1,
//	optional PlayerReplicationInfo RelatedPRI_2,
//	optional Object OptionalObject
//	)
//{
//	Local bool bCandidate, bOtherTeamVote;
//	Local float fTimeLeft;
//	Local String PlayerName;
//	Local int VoteCount[2];
//	Local int VoteRange;
//	Local avaMsgParam Param;
//	Local avaVoteUI VoteUI;
//	Local array<string> Msg;
//	
//
//	Param = avaMsgParam(OptionalObject);
//	bCandidate = Param.BoolParam1;
//	bOtherTeamVote = Param.BoolParam2;
//	fTimeLeft = Param.FloatParam[0];
//	PlayerName = Param.StringParam[0];
//	VoteCount[0] = Param.IntParam[0];
//	VoteCount[1] = Param.IntParam[1];
//	VoteRange = Param.IntParam[0];
//
//	Assert(LocalPlayer(P.Player) != None);
//	Assert(avaGameViewportClient(LocalPlayer(P.Player).ViewportClient) != None);
//	VoteUI = avaGameViewportClient(LocalPlayer(P.Player).ViewportClient).VoteUI;
//
//	Msg.Length = 5;			// an arbitrary value such as MAX_CHAR
//
//	switch(Switch)
//	{	
//	case VOTEPROGRESS_Invalid:		Msg[0] = default.DrawGame_Invalid;
//		break;
//	case VOTEPROGRESS_Insufficient:	Msg[0] = default.DrawGame_Insufficient;
//		break;
//	case VOTEPROGRESS_Already:		Msg[0] = default.DrawGame_Already;							
//		break;
//	case VOTEPROGRESS_Another:
//		if( VoteRange == VOTERANGE_TEAM)
//		{	
//			Msg[0] = default.VoteSystem_Another;		
//			Msg[1] = class'avaStringHelper'.static.Replace( int(fTimeLeft),"%t", default.VoteSystem_AnotherTimeLeft);
//		}
//		else if (VoteRange == VOTERANGE_ALL)
//		{
//			Msg[0] = bOtherTeamVote ? default.VoteSystem_AnotherOtherTeam : default.VoteSystem_Another;
//			Msg[1] = class'avaStringHelper'.static.Replace(int(fTimeLeft), "%t",default.VoteSystem_AnotherTimeLeft);
//		}
//		break;
//	case VOTEPROGRESS_Freeze:		Msg[0] = class'avaStringHelper'.static.Replace(int(fTimeLeft),"%t",default.DrawGame_Freeze);
//		break;
//	case VOTEPROGRESS_Cancel:		Msg[0] = class'avaStringHelper'.static.Replace(PlayerName, "%s", default.DrawGame_Cancel);
//		break;
//	case VOTEPROGRESS_Retire:	
//		break;
//	case VOTEPROGRESS_Submit:
//		Msg[0] = class'avaStringHelper'.static.Replace(int(fTimeLeft),"%t",default.DrawGame_Submit[0]);
//		Msg[1] = default.DrawGame_Submit[1];	
//		Msg[2] = class'avaStringHelper'.static.Replace(VoteCount[0],"%n", default.VoteSystem_VoteResult);
//		Msg[2] = class'avaStringHelper'.static.Replace(VoteCount[1],"%n", Msg[2]);	
//		break;
//	case VOTEPROGRESS_Logout:
//		Msg[0] = class'avaStringHelper'.static.Replace(PlayerName,"%s",default.VoteSystem_Logout);
//		Msg[1] = bCandidate ? default.DrawGame_CandidateLogout : default.DrawGame_VoterLogout;
//		break;
//	case VOTEPROGRESS_Elected:
//		Msg[0]=(default.DrawGame_Elected[0]);
//		Msg[1]=(default.DrawGame_Elected[1]);		
//		Msg[2] = class'avaStringHelper'.static.Replace( (VoteCount[0] == 1) ? default.VoteSystem_Accept : default.VoteSystem_Deny, "%s", default.DrawGame_VoteResult);
//		Msg[2] = class'avaStringHelper'.static.Replace( (VoteCount[1] == 1) ? default.VoteSystem_Accept : default.VoteSystem_Deny, "%s", Msg[2]);
//		break;
//	case VOTEPROGRESS_Timeout:
//	case VOTEPROGRESS_Rejected:
//		Msg[0]=(default.DrawGame_Rejected[0]);
//		Msg[1]=(default.DrawGame_Rejected[1]);		
//		Msg[2] = class'avaStringHelper'.static.Replace( (VoteCount[0] == 1) ? default.VoteSystem_Accept : default.VoteSystem_Deny,"%s",default.DrawGame_VoteResult);
//		Msg[2] = class'avaStringHelper'.static.Replace( (VoteCount[1] == 1) ? default.VoteSystem_Accept : default.VoteSystem_Deny,"%s",Msg[2]);
//		break;
//	case VOTEPROGRESS_Vote:
//		Msg[0] = class'avaStringHelper'.static.Replace(int(fTimeLeft),"%t",default.DrawGame_Vote[0]);			
//		Msg[1] = class'avaStringHelper'.static.Replace(PlayerName,"%t",default.DrawGame_Vote[1]);
//		if(!bCandidate)
//		{
//			Msg[2]=(default.VoteSystem_AcceptHead$default.VoteSystem_Accept);
//			Msg[3]=(default.VoteSystem_DenyHead$default.VoteSystem_Deny);
//		}
//		Msg[4] = class'avaStringHelper'.static.Replace(VoteCount[0], "%n", default.DrawGame_VoteStatus);
//		Msg[4] = class'avaStringHelper'.static.Replace(VoteCount[1], "%n", Msg[4]);
//		break;
//	default:
//		break;
//	}
//
//	VoteUI.AddMessage(Msg, default.LifeSpan, default.ConsoleColor);
//}
//
//defaultproperties
//{
//	ConsoleColor = (R=0,G=230,B=100,A=170)
//	LifeSpan = 3.0
//
//	MessageData.Add(( Key="S", MsgIndex=VOTESOUND_Start,SoundCue[0]=SoundCue'avaGameVoices.VoteVoices.Vote_Begin',SoundCue[1]=SoundCue'avaGameVoices.VoteVoices.Vote_Begin'))
//	MessageData.Add(( Key="E", MsgIndex=VOTESOUND_End,SoundCue[0]=SoundCue'avaGameVoices.VoteVoices.Vote_Finish',SoundCue[1]=SoundCue'avaGameVoices.VoteVoices.Vote_Finish'))
//	MessageData.Add(( Key="C", MsgIndex=VOTESOUND_Cancel,SoundCue[0]=SoundCue'avaGameVoices.VoteVoices.Vote_Cancel',SoundCue[1]=SoundCue'avaGameVoices.VoteVoices.Vote_Cancel'))
//	MessageData.Add(( Key="Y", MsgIndex=VOTESOUND_Yes,SoundCue[0]=SoundCue'avaGameVoices.VoteVoices.Vote_Yes',SoundCue[1]=SoundCue'avaGameVoices.VoteVoices.Vote_Yes'))
//	MessageData.Add(( Key="N", MsgIndex=VOTESOUND_No,SoundCue[0]=SoundCue'avaGameVoices.VoteVoices.Vote_No',SoundCue[1]=SoundCue'avaGameVoices.VoteVoices.Vote_No'))
//
//	// INDISPENSIBLE FOR BINARY SEARCH ( ref. avaLocalMessage ). DO NOT DELETE
//	MessageData.Sort(Key)
//}