class avaVoteMessage extends avaLocalMessage;

var const Color	ConsoleColor;
var const float LifeSpan;

var localized string	VoteInfoMessage;			//	"���� �ȳ� Message"
var localized string	VoteKickSelectPlayer;		//	"���� ���� ���� ����" Message
var localized string	VoteKickSelectReason;		//	"���� ���� ���� ����" Message
var localized string	VoteKickReason[4];			//	"���� ���� ����"	�ҹ� ���α׷� ���, ���� �ǿ�, �弳 �� ���, ��Ÿ
var localized string	VoteKickTitle;				//	"%s �Բ��� %s ���� %s �� ������ ���������� ��û�Ͽ����ϴ�."
var localized string	VoteWarn;					//  ��� Message
var localized string	VotingMsg;					//	�����Ͻʴϱ�? �ݴ�[F11]/ ����[F12]
var localized string	VoteCount;					//	"���� %dǥ/�ݴ� %dǥ"
var localized string	VoteAccept;					//	%s ���� �����ϼ̽��ϴ�.
var localized string	VoteDeny;					//	%s ���� �ݴ��ϼ̽��ϴ�.
var localized string	VoteKickAccepted;			//	%s ���� ��ǥ�� ���ؼ� �������� �Ǿ����ϴ�.
var localized string	VoteKickDenied;				//	%s ���� �������� ��ǥ�� �ΰ� �Ǿ����ϴ�.
var localized string	VoteLeftTime;				//	�����ð� %n ��:

var localized string	MSG_VOTE_PROGRESS;			//	���� �ٸ� ��ǥ�� �������Դϴ�.
var localized string	MSG_VOTE_DISABLE;			//	��ǥ��û�� 3��30�� �ȿ� �ٽ� ��û�� �� �����ϴ�.
var localized string	MSV_VOTE_INSUFFICIENTVOTERS;				//	��ǥ �ο��� �����մϴ�.
var localized string	MSG_VOTE_INSUFFICIENTMONEY;					//	�����ݺ������� ��ǥ�� �� ����.
var localized string	MSG_VOTE_INSUFFICIENTMONEY_FOR_PROPOSAL;	//	�����ݺ������� ��ǥ�� �� ����.




//var localized string VoteTeamMenu[3];
//var localized string VoteAllMenu[2];
//var localized string VoteRetireMenu[3];
//
//var localized string SquadLeader_Invalid;	//"�ϻ� ��� �̻� �д��� ������ �����մϴ�."
//var localized string SquadLeader_Already;	//"����� �̹� �д����Դϴ�."
//var localized string SquadLeader_Freeze;	//"���� ���ĺ����� �ð����� Seconds �ʰ� ���ҽ��ϴ�."
//var localized string SquadLeader_Vote[2];	//"�д��� ������ǥ( Seconds �� ����)"
////var localized string SquadLeader_Vote[1];	//"PlayerName ���� �д��忡 �����Ͽ����ϴ�."
//var localized string SquadLeader_Elected[3];	//"�д��� ���� ��ǥ ����!"
////var localized string SquadLeader_Elected[1];	//"PlayerName ���� �д������� �Ӹ�Ǿ����ϴ�."
//var localized string SquadLeader_Vote2[2];	//"�д��� ������ǥ( Seconds �� ����)"
////var localized string SquadLeader_Vote2[1];	//"PlayerName ���� ���� �д��忡 �����Ͽ����ϴ�."
//var localized string SquadLeader_Timeout;	//"�ð����� ���ݼ��� ���� ���߽��ϴ�."
//
//var localized string SquadLeader_Rejected[3];	//"�д��� ���� ��ǥ ����"
////var localized string SquadLeader_Rejected[1];	//"%s ���� �д����� ���� ���߽��ϴ�"
//
//var localized string SquadLeader_InSufficient;	//"��ǥ �ο��� �����մϴ�."
//var localized string SquadLeader_Another[2];	//"���� �ٸ� ��ǥ�� �������Դϴ�."
////var localized string SquadLeader_Another[1];	//"�������� ��ǥ ���� �ð� : %t ��"
//
//var localized string SquadLeader_VoteStatus;	//"���� %s %n ǥ : %s %n ǥ"
//var localized string SquadLeader_Retire;		//"%s ���� �д��忡�� ���������ϴ�."
//
//
//var localized string VoteSystem_Insufficient;	//"��ǥ �ο��� �����մϴ�."
//var localized string VoteSystem_VoteStatus;	//"���� ���� %n ǥ : �ݴ� %n ǥ"
//var localized string VoteSystem_VoteResult;	//"���� %n ǥ : �ݴ� %n ǥ"
//
//var localized string VoteSystem_Accept;	//"F5 : ����"
//var localized string VoteSystem_Deny;	//"F6 : �ݴ�"
//
//
//var localized string DrawGame_Invalid;	//"����� ������ ��Ȳ�� �ƴմϴ�."
//var localized string DrawGame_Already;	//"�̹� ������ ����� �����Ͽ����ϴ�."
//var localized string DrawGame_Insufficient;	//"��ǥ �ο��� �����մϴ�."
//var localized string DrawGame_Another[2];	//"���� �ٸ� ��ǥ�� �������Դϴ�"
////var localized string DrawGame_Another[1];	//"�������� ��ǥ ���� �ð� : %t ��"
//var localized string DrawGame_Freeze;	//"���� ������ Seconds ���� ���ֽʽÿ�"
//var localized string DrawGame_Vote[2];	//"���� ���� ��ǥ ( Seconds �� ����)"
////var localized string DrawGame_Vote[1];	//"PlayerName ���� ���� �ߴ��� ��û�Ͽ����ϴ�."
//
//var localized string DrawGame_Elected[2];	//"���� ����"
////var localized string DrawGame_Elected[1];	//"������ �ߴ��ϰ� ���������� �ϱ�� �߽��ϴ�."
//
//var localized string DrawGame_Rejected[2];		//"���� ����"
////var localized string DrawGame_Rejected[1];		//"��� �������� ���� �ʱ�� �Ͽ����ϴ�."
//var localized string DrawGame_Timeout;	//"�ð����� ���ݼ��� ���� ���߽��ϴ�."
//var localized string DrawGame_VoteStatus;	//"�츮�� ���� %n ǥ : �ݴ� %n ǥ"
//var localized string DrawGame_VoteResult;	//"EU �� %s / NRF �� %s"
//
//var localized string VoteSystem_AcceptHead;		//"F5 : "
//var localized string VoteSystem_DenyHead;		//"F6 : "
//var localized string SquadLeader_Cancel;		//"%s ���� ��û�� ��ǥ�� ����ϼ̽��ϴ�."
//var localized string SquadLeader_CancelMenu;	//"F7 : ��û�� ��ǥ �ߴ�"
//
//var localized string SquadLeader_Submit[2];		//"�д��� ������ǥ( %t �� ����)"
////var localized string SquadLeader_Submit[1];		//"��ǥ�� �����Ͽ����ϴ�."
//var localized string DrawGame_Submit[2];		//"���� ���� ��ǥ ( %t �� ����)"
////var localized string DrawGame_Submit[1];		//"��ǥ�� �����Ͽ����ϴ�."
//
//var localized string VoteSystem_Another;	//"���� ��ǥ�� ���� ���Դϴ�."
//var localized string VoteSystem_AnotherTimeLeft;	//"�������� ��ǥ ���� �ð� : %t ��."
//var localized string VoteSystem_AnotherOtherTeam;	//"���� �ٸ� �� ������ ��ǥ�� ���� ���Դϴ�."
//
//var localized string SquadLeader_CandidateLogout;	//"�д��� �ĺ��ڰ� ������ �д��� ������ ��ҵǾ����ϴ�."
//var localized string SquadLeader_VoterLogout;		//"���� �̴޷� �д��� ������ ��ҵǾ����ϴ�."
//
//var localized string DrawGame_CandidateLogout;	//" �����ڰ� ������ ���� ���Ⱑ ��ҵǾ����ϴ�."
//var localized string DrawGame_VoterLogout;	//"���� �̴޷� ���� ���Ⱑ ��ҵǾ����ϴ�."
//var localized string DrawGame_Cancel;		//"%s ���� �����⸦ ����Ͽ����ϴ�"
//
//var localized string VoteSystem_Logout;		//"%s ���� ���ӿ��� �������ϴ�."
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