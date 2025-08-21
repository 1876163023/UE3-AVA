class avaVoteSystem extends Info;
	

var float	VoteTime;

enum EVoteSubjectType
{
	VOTESUBJECT_KickPlayer,
};

struct VoteInfo
{
	var avaPlayerReplicationInfo		proposer;		// �������� PRI �̴�.
	var avaPlayerReplicationInfo		target;			// ������� PRI �̴�.
	var int								targetID;		// ���� ����� ���ؼ��� Vote �� ���� �ؾ߸� �Ѵ�....
	var int								proposerID;		//
	var bool							bVoting;		// treu �̸� Vote �� �������̴�...
	var float							LeftTime;		// Vote �� ���� �ð�
	var	EVoteSubjectType				Type;			// ���� ����ǰ� �ִ� Vote �� Type �̴�.
	var int								Subject;		// ���� �׸�
	var array<PlayerReplicationInfo>	Voters;			// ���ĺ� ����� �����ڵ�
	var array<PlayerReplicationInfo>	Accept;			// ������ �����
	var array<PlayerReplicationInfo>	Deny;			// �ݴ��� �����
};

var VoteInfo	TeamVoteInfo[2];

function bool StartVote(EVoteSubjectType Subject,avaPlayerController proposer, avaPlayerReplicationInfo target, int nSubject )
{
	local int					proposerTeam;
	local avaPlayerController	avaPC;
	local int					i;
	
	proposerTeam = proposer.GetTeamNum();

	// TeamGame �� ��쿡 �������� Team �� Valid ���� ����...
	if ( WorldInfo.Game.bTeamGame )
	{
		proposerTeam = proposer.GetTeamNum();
		if ( proposerTeam < 0 || proposerTeam > 1 )	
			return true;
	}
	else
	{
		proposerTeam = 0;
	}

	if ( TeamVoteInfo[proposerTeam].bVoting == true )	
		return false;	// �������� Team�� ���� Voting �� ��������

	TeamVoteInfo[proposerTeam].proposer		= avaPlayerReplicationInfo( proposer.PlayerReplicationInfo );
	TeamVoteInfo[proposerTeam].target		= target;
	TeamVoteInfo[proposerTeam].proposerID	= avaPlayerReplicationInfo( proposer.PlayerReplicationInfo ).AccountID;
	TeamVoteInfo[proposerTeam].targetID		= target.AccountID;
	TeamVoteInfo[proposerTeam].bVoting		= true;
	TeamVoteInfo[proposerTeam].LeftTime		= VoteTime;
	TeamVoteInfo[proposerTeam].Type			= Subject;
	TeamVoteInfo[proposerTeam].Subject		= nSubject;

	TeamVoteInfo[proposerTeam].Voters.length = 0;
	TeamVoteInfo[proposerTeam].Accept.length = 0;
	TeamVoteInfo[proposerTeam].Deny.length = 0;

	foreach WorldInfo.AllControllers( class'avaPlayerController', avaPC )
	{
		// TeamGame �� ��쿡 �����ڿ� ���� ���� �ʴٸ� �����Ѵ�.
		if ( WorldInfo.Game.bTeamGame && avaPC.GetTeamNum() != proposerTeam )
			continue;

		// Spectator �� ���� ��쿡�� ��ǥ�� ������ �� ����.
		if ( avaPC.PlayerReplicationInfo.bOnlySpectator == true )
			continue;
		
		if ( avaPC.PlayerReplicationInfo == target )	
			continue;	// ��ǥ ����ڴ� �����ڿ��� �����Ѵ�..
		i =	TeamVoteInfo[proposerTeam].Voters.length;
		TeamVoteInfo[proposerTeam].Voters[i] = avaPC.PlayerReplicationInfo;
	}

	if ( proposerTeam == 0 )	SetTimer( VoteTime, false, 'VoteTimeOver_EU' );
	else						SetTimer( VoteTime, false, 'VoteTimeOver_NRF' );

	// ��ǥ ������ �����ڿ��� �˸���....
	ReportStartVote( TeamVoteInfo[proposerTeam] );

	// �����ڴ� �������� �����Ѵ�...
	PlayerVote( proposer, true );
	return true;
}

function VoteTimeOver_EU()
{
	CheckVoteOver( 0, true );
}

function VoteTimeOver_NRF()
{
	CheckVoteOver( 1, true );
}

function bool IsAvailableVote( int TeamIdx, avaPlayerController voter )
{
	local int	i;

	// ��ǥ���� Team �� Valid ���� Check �Ѵ�. TeamGame �� ��쿡�� Check �ϵ��� �Ѵ�...
	if ( WorldInfo.Game.bTeamGame && TeamIdx < 0 || TeamIdx > 1 )					
		return false;	

	if ( TeamVoteInfo[TeamIdx].bVoting == false )		return false;	// ��ǥ�� �������� �ƴ�
	for ( i = 0 ; i < TeamVoteInfo[TeamIdx].Accept.length ; ++ i )		// ��ǥ�ڰ� �̹� �����ߴ��� Check �Ѵ�.
	{
		if ( TeamVoteInfo[TeamIdx].Accept[i] == voter.PlayerReplicationInfo )	return false;
	}
	for ( i = 0 ; i < TeamVoteInfo[TeamIdx].Deny.length ; ++ i )		// ��ǥ�ڰ� �̹� �ݴ��ߴ��� Check �Ѵ�.
	{
		if ( TeamVoteInfo[TeamIdx].Deny[i] == voter.PlayerReplicationInfo )	return false;
	}
	for ( i = 0 ; i < TeamVoteInfo[TeamIdx].Voters.length ; ++ i )		// ��ǥ�ڰ� �������߿� �ִ��� Check �Ѵ�.
	{
		if ( TeamVoteInfo[TeamIdx].Voters[i] == voter.PlayerReplicationInfo )
			return true;
	}
	return false;
}

// ��ǥ�� �޴´�....
function PlayerVote( avaPlayerController voter, bool bResult )
{
	// ��ǥ�� �������� Check �Ѵ�...
	local int	TeamIdx;
	local int	i;

	if ( WorldInfo.Game.bTeamGame )
		TeamIdx = voter.GetTeamNum();
	else
		TeamIdx = 0;

	if ( IsAvailableVote( TeamIdx, voter ) == false )	return;
	if ( bResult == true )	TeamVoteInfo[TeamIdx].Accept[TeamVoteInfo[TeamIdx].Accept.length]	= avaPlayerReplicationInfo( voter.PlayerReplicationInfo );
	else					TeamVoteInfo[TeamIdx].Deny[TeamVoteInfo[TeamIdx].Deny.length]		= avaPlayerReplicationInfo( voter.PlayerReplicationInfo );

	`log( "Playervote" @voter @bResult @TeamVoteInfo[TeamIdx].Voters.length @TeamVoteInfo[TeamIdx].Accept.length @TeamVoteInfo[TeamIdx].Deny.length );

	for ( i = 0 ; i < TeamVoteInfo[TeamIdx].Voters.length ; ++ i )
	{
		avaPlayerController( TeamVoteInfo[TeamIdx].Voters[i].Owner ).Client_VoteAccepted( avaPlayerReplicationInfo( voter.PlayerReplicationInfo ), bResult );
	}
	// ��ǥ�� ���� �Ǿ����� Check �Ѵ�.
	CheckVoteOver( TeamIdx );
}

function CheckVoteOver( int TeamIdx, optional bool bTimeOver = false )
{
	local int VotersCnt, AcceptCnt, DenyCnt;
	if ( TeamVoteInfo[TeamIdx].bVoting == false )	return;
	VotersCnt = TeamVoteInfo[TeamIdx].Voters.length;
	AcceptCnt = TeamVoteInfo[TeamIdx].Accept.length;
	DenyCnt	  = TeamVoteInfo[TeamIdx].Deny.length;
	if ( bTimeOver == true )
	{
		// ������ �������� 50% �̻��̹Ƿ� �����
		if ( VotersCnt > 0 && AcceptCnt == VotersCnt )		ReportVoteOver( TeamIdx, true );
		else												ReportVoteOver( TeamIdx, false );								
	}
	else
	{
		// �����̳� �ݴ밡 ���ݼ��� ������ Vote �� �����Ѵ�....
		if ( VotersCnt > 0 && AcceptCnt == VotersCnt )		ReportVoteOver( TeamIdx, true );	
		else if ( DenyCnt > 0 )								ReportVoteOver( TeamIdx, false );
	}
}

// �߰��� ���� ����� �ݴ�ǥ�� ���� ������ �����Ѵ�...
function DisconnectEvent( Controller Exiting )
{
	PlayerVote( avaPlayerController( Exiting ), false );	
}

// Vote �� ���۵Ǿ����� �����ڵ鿡�� �˷��ش�...
function ReportStartVote( VoteInfo info ) 
{
	local int i;
	for ( i = 0 ; i < info.Voters.length ; ++ i )
	{
		avaPlayerController( info.Voters[i].Owner ).Client_StartVote( info.Type, info.proposer, info.target, info.LeftTime, info.Subject );
	}
}

function ReportVoteOver( int TeamIdx, bool bResult )
{
	local int			i;
	local array< int >	VoterList;
	for ( i = 0 ; i < TeamVoteInfo[TeamIdx].Voters.length ; ++ i )
	{
		avaPlayerController( TeamVoteInfo[TeamIdx].Voters[i].Owner ).Client_EndVote( bResult );
	}

	if ( bResult )
	{
		for ( i = 0 ; i < TeamVoteInfo[TeamIdx].Accept.length ; ++ i )
		{
			if ( TeamVoteInfo[TeamIdx].proposerID != avaPlayerReplicationInfo( TeamVoteInfo[TeamIdx].Accept[i] ).AccountID )
			{
				VoterList.length =	VoterList.length + 1;
				VoterList[i]	=	avaPlayerReplicationInfo( TeamVoteInfo[TeamIdx].Accept[i] ).AccountID;
			}
		}

		class'avaNetHandler'.static.GetAvaNetHandler().ReportVoteNew( 
			EVC_Kick, 
			TeamVoteInfo[TeamIdx].proposerID, 
			VoterList, 
			TeamVoteInfo[TeamIdx].targetID, 
			TeamVoteInfo[TeamIdx].Subject );

		// vote kick --> ReportVote(EVC_Kick, Caller_Account_ID, Victim_Account_ID, EVoteKickReason)
		//class'avaNetHandler'.static.GetAvaNetHandler().ReportVote(
		//	EVC_Kick, 
		//	TeamVoteInfo[TeamIdx].proposerID, 
		//	TeamVoteInfo[TeamIdx].targetID, 
		//	TeamVoteInfo[TeamIdx].Subject );
		//`log( "ReportVoteOver!!!" );
	}

	// Vote Info �ʱ�ȭ....
	TeamVoteInfo[TeamIdx].bVoting		= false;
	TeamVoteInfo[TeamIdx].Voters.length	= 0;
	TeamVoteInfo[TeamIdx].Accept.length	= 0;
	TeamVoteInfo[TeamIdx].Deny.length	= 0;

	if ( TeamIdx == 0 )			ClearTimer( 'VoteTimeOver_EU' );
	else if ( TeamIdx == 1 )	ClearTimer( 'VoteTimeOVer_NRF' );
}

defaultproperties
{
	RemoteRole	=	ROLE_None
	VoteTime	=	30.0
}


//enum EVoteSubjectType
//{
//	VOTESUBJECT_SquadLeader_EU,
//	VOTESUBJECT_SquadLeader_USSR,
//	VOTESUBJECT_MapSelection,
//	VOTESUBJECT_KickPlayer,
//	VOTESUBJECT_DrawGame,
//};
//
//enum EVoteProgressType
//{
//	// Drawing Progress
//	VOTEPROGRESS_Invalid,
//	VOTEPROGRESS_Insufficient,
//	VOTEPROGRESS_Another,
//	VOTEPROGRESS_Timeout,
//	VOTEPROGRESS_Already,
//	VOTEPROGRESS_Freeze,
//	VOTEPROGRESS_Elected,
//	VOTEPROGRESS_Rejected,
//	VOTEPROGRESS_Vote,
//	VOTEPROGRESS_Cancel,
//	VOTEPROGRESS_Retire,
//	VOTEPROGRESS_Submit,
//	VOTEPROGRESS_Logout,
//
//	// One chance Event (Sound, Notify Msg, ...)
//	VOTEPROGRESS_EventStart,
//	VOTEPROGRESS_EventCancel,
//	VOTEPROGRESS_EventEnd,
//	VOTEPROGRESS_EventAccept,
//	VOTEPROGRESS_EventDeny,
//};
//
//enum EVoteRangeType
//{
//	VOTERANGE_Team,
//	VOTERANGE_All,
//};
//
//enum EShowFlagType
//{
//	VOTEMSG_Show,
//	VOTEMSG_Hide,
//};
//
//struct native FreezeInfo
//{
//	var PlayerReplicationInfo				FreezePRI;
//	var float								fTimeleft;
//};
//
//struct native VoteInfo
//{
//	/* Vote SubjectInfo*/
//	var Name				Name;		// "MapSelection", "SquadLeader", "KickPlayer"	
//	var EVoteRangeType		Range;				// Team - ���� ��ǥ, All - ��� ��ǥ
//	var float				Duration;		// ��ǥ�� ���� �ð�. ex) KickPlayer�� 20�� ��ǥ���
//	var int					nMinJoinNum;		// ��ǥ���� �ּ��ο�.		
//	var float				fMinVoteRate;		// �� �ۼ�Ʈ�̻� ��ǥ������, ��ǥ ����� Ȯ���� ���ΰ�
//												// ex) 20%���� ��ǥ�ÿ��� ��ǥ ��� ���.
//	var float				fAcceptRate;		// ���ݼ�(�ټ���) - 0.5
//
//	/* Vote Internal Logic ( State, Condition , etc ...)*/
//	var bool									bVoting;			// ��ǥ�� ...
//	var array<PlayerReplicationInfo>			Voters;				// ���ĺ� ����� �����ڵ�
//	var array<PlayerReplicationInfo>			Accept;				// ������ �����
//	var array<PlayerReplicationInfo>			Deny;				// �ݴ��� �����
//	var array<PlayerReplicationInfo>			Left;				// ��ǥ ���� �����
//	var array<PlayerReplicationInfo>			Submit;				// ��ǥ�� �����
//	var float									fTimeLeft;			// ��ǥ �����ð�
//
//	var array<int>								TeamVoterCount;		// ���� �����ڼ� (ex. EU - 3��, NRF - 4�� )
//	var array<int>								TeamAcceptCount;	// ���� ������ (ex. EU - 1��, NRF - 0��)
//	var array<int>								TeamDenyCount;		// ���� �ݴ�� (ex. EU - 1��, NRF - 1��)
//	
//	var array<FreezeInfo>						Freeze;
//	var PlayerReplicationInfo					CandidatePRI;
//	var PlayerReplicationInfo					ElectedPRI;
//}
//
//var array<VoteInfo>				Vote;
//var const float					fUpdatePeriod;
//var const float					fMsgDuration;
//var const float					fFreezePeriod;
//
//var PlayerReplicationInfo						TempPRI;
//
//
//static native function string ParsePlayerName(coerce string PlayerName, string Message);
//static native function string ParseTimeLeft(coerce string fTimeLeft, string Message);
//static native function string ParseVoteCount(coerce string accepts, coerce string denies, string Message);
//
//
//function NotifyInvalid(EVoteSubjectType Subject)
//{
//	SendToCandidate(Subject, VOTEPROGRESS_Invalid, true);
//}
//
//function NotifyAnother(EVoteSubjectType Subject, EVoteSubjectType AnotherSubject)
//{
//	SendToCandidate(Subject, VOTEPROGRESS_Another, true, Vote[AnotherSubject].fTimeLeft);
//}
//
//function NotifyInsufficient(EVoteSubjectType Subject)
//{
//	SendToCandidate(Subject, VOTEPROGRESS_Insufficient, true);
//}
//
//function NotifyTimeout(EVoteSubjectType Subject)
//{
//	Local string ElectedName;
//	ElectedName = Vote[Subject].ElectedPRI != None ? Vote[Subject].ElectedPRI.PlayerName : "";
//	SendToCandidate(Subject, VOTEPROGRESS_Rejected, true, , Vote[Subject].CandidatePRI.PlayerName,ElectedNAme ,Vote[Subject].Accept.Length, Vote[Subject].Deny.Length);
//	SendToVoters(Subject, VOTEPROGRESS_Rejected, false, , Vote[Subject].CandidatePRI.PlayerName,ElectedName ,Vote[Subject].Accept.Length, Vote[Subject].Deny.Length);
//}
//
//function NotifyAlready(EVoteSubjectType Subject)
//{
//	SendToCandidate(Subject, VOTEPROGRESS_Already, true);
//}
//
//function NotifyCancel(EVoteSubjectType Subject)
//{
//	SendToCandidate(Subject, VOTEPROGRESS_Cancel, true, , Vote[Subject].CandidatePRI.PlayerName);
//	SendToVoters(Subject, VOTEPROGRESS_Cancel, false, , Vote[Subject].CandidatePRI.PlayerName);
//}
//
//function NotifyRetire(EVoteSubjectType Subject, PlayerReplicationInfo PRI)
//{
////	SendToElected(Subject, VOTEPROGRESS_Retire, true, , PRI.PlayerName);
//	SendToTeam(Subject, VOTEPROGRESS_Retire, false, , PRI.PlayerName);
//}
//
//function NotifyLogout(EVoteSubjectType Subject, PlayerReplicationInfo PRI)
//{
//	SendToCandidate(Subject, VOTEPROGRESS_Logout, Vote[Subject].CandidatePRI == PRI , ,PRI.PlayerName);
//	SendToVoters(Subject, VOTEPROGRESS_Logout, Vote[Subject].CandidatePRI == PRI , ,PRI.PlayerName);
//}
//
///* ElectedPRI = CandidatePRI �ϱ� ���� �Ҹ�. ���� ElectedPRI �� ���� ������, CandidatePRI �̹� ���ĺ���*/
//function NotifyElected(EVoteSubjectType Subject)
//{
//	Local string ElectedName;
//	Local int n1, n2;
//
//	switch(Subject)
//	{
//	case VOTESUBJECT_SquadLeader_EU:	
//	case VOTESUBJECT_SquadLeader_USSR:		n1 = Vote[Subject].Accept.Length; n2 = Vote[Subject].Deny.Length;	break;
//	case VOTESUBJECT_DrawGame:	n1 = (CheckEachTeamAccept(Subject, TEAM_EU) ? 1 : 0); 
//								n2 = (CheckEachTeamAccept(Subject, TEAM_USSR) ? 1 : 0);
//									break;
//	}
//	// Elected �� ���� �ĺ���, Candidate�� ���� �ĺ���
//	ElectedName = Vote[Subject].ElectedPRI != None ? Vote[Subject].ElectedPRI.PlayerName : "";
//	SendToCandidate(Subject, VOTEPROGRESS_Elected, true, , Vote[Subject].CandidatePRI.PlayerName,ElectedName ,n1, n2);
//	SendToVoters(Subject, VOTEPROGRESS_Elected, false, , Vote[Subject].CandidatePRI.PlayerName,ElectedName ,n1, n2);
//
//	SendToVoters(Subject, VOTEPROGRESS_EventEnd);
//}
//
//function NotifyRejected(EVoteSubjectType Subject)
//{
//	Local string ElectedName;
//	Local int n1, n2;
//
//	switch(Subject)
//	{
//	case VOTESUBJECT_SquadLeader_EU:	
//	case VOTESUBJECT_SquadLeader_USSR:		n1 = Vote[Subject].Accept.Length; n2 = Vote[Subject].Deny.Length;	break;
//	case VOTESUBJECT_DrawGame:	n1 = (CheckEachTeamAccept(Subject, TEAM_EU) ? 1 : 0); 
//								n2 = (CheckEachTeamAccept(Subject, TEAM_USSR) ? 1 : 0);
//									break;
//	}
//
//	ElectedName = Vote[Subject].ElectedPRI != None ? Vote[Subject].ElectedPRI.PlayerName : "";
//	SendToCandidate(Subject, VOTEPROGRESS_Rejected, true, , Vote[Subject].CandidatePRI.PlayerName,ElectedNAme , n1, n2);
//	SendToVoters(Subject, VOTEPROGRESS_Rejected, false, , Vote[Subject].CandidatePRI.PlayerName,ElectedName , n1, n2);
//
//	SendToVoters(Subject, VOTEPROGRESS_EventEnd);
//}
//
//function NotifyVote(EVoteSubjectType Subject)
//{	
//	Local string ElectedName;
//	Local int n1, n2, TeamIndex, i;
//
//	TeamIndex = Vote[Subject].CandidatePRI.Team.TeamIndex;
//	ElectedName = Vote[Subject].ElectedPRI != None ? Vote[Subject].ElectedPRI.PlayerName : "";
//	
//	switch(Subject)
//	{
//	case VOTESUBJECT_SquadLeader_EU:	
//	case VOTESUBJECT_SquadLeader_USSR:		
//		n1 = Vote[Subject].Accept.Length; n2 = Vote[Subject].Deny.Length;		
//		SendToCandidate(Subject, VOTEPROGRESS_Vote, true, Vote[Subject].fTimeLeft, Vote[Subject].CandidatePRI.PlayerName,ElectedName, n1, n2);
//		SendToLeft(Subject, VOTEPROGRESS_Vote, false,Vote[Subject].fTimeLeft, Vote[Subject].CandidatePRI.PlayerName,ElectedName, n1, n2);
//			break;
//	case VOTESUBJECT_DrawGame:	
//		n1 = Vote[Subject].TeamAcceptCount[TeamIndex]; 
//		n2 = Vote[Subject].TeamDenyCount[TeamIndex];
//		SendToCandidate(Subject, VOTEPROGRESS_Vote, true, Vote[Subject].fTimeLeft, Vote[Subject].CandidatePRI.PlayerName,ElectedName, n1, n2);
//		for( i = 0 ; i < Vote[Subject].Left.Length ; i++)
//		{
//			TeamIndex = Vote[Subject].Left[i].Team.TeamIndex;
//			n1 = Vote[Subject].TeamAcceptCount[TeamIndex]; 
//			n2 = Vote[Subject].TeamDenyCount[TeamIndex];
//			SendToSinglePRI(Vote[Subject].Left[i],Subject, VOTEPROGRESS_Vote, false, Vote[Subject].fTimeLeft, Vote[Subject].CandidatePRI.PlayerName,ElectedName, n1, n2);
//		}
//		break;
//	}
//}
//
//function NotifySubmit(EVoteSubjectType Subject)
//{
//	Local int n1, n2, i, TeamIndex;
//
//	switch(Subject)
//	{
//	case VOTESUBJECT_SquadLeader_EU:	
//	case VOTESUBJECT_SquadLeader_USSR:		
//		n1 = Vote[Subject].Accept.Length; n2 = Vote[Subject].Deny.Length;		
//		SendToSubmit(Subject, VOTEPROGRESS_Submit, false,Vote[Subject].fTimeLeft, , , n1, n2);
//			break;
//	case VOTESUBJECT_DrawGame:	
//		for( i = 0 ; i < Vote[Subject].Submit.Length ; i++)
//		{
//			TeamIndex = Vote[Subject].Submit[i].Team.TeamIndex;
//			n1 = Vote[Subject].TeamAcceptCount[TeamIndex]; 
//			n2 = Vote[Subject].TeamDenyCount[TeamIndex];
//			SendToSinglePRI(Vote[Subject].Submit[i],Subject, VOTEPROGRESS_Submit, false, Vote[Subject].fTimeLeft, , , n1, n2);
//		}
//		break;
//	}
//
//}
//
//function NotifyFreeze(EVoteSubjectType Subject, PlayerReplicationInfo PRI)
//{
//	Local int Index;
//
//	Index = Vote[Subject].Freeze.find('FreezePRI', PRI);
//	Assert(Index >= 0);
//	SendToCandidate(Subject, VOTEPROGRESS_Freeze, true, Vote[Subject].Freeze[index].fTimeLeft);
//}
//
//// Call from Start() only
//function NotifyEventStart(EVoteSubjectType Subject)
//{
//	SendToCandidate(Subject, VOTEPROGRESS_EventStart);
//	SendToVoters(Subject, VOTEPROGRESS_EventStart);
//}
//
//// Call from NotifyRejected, NotifyElected
//function NotifyEventEnd(EVoteSubjectType Subject)
//{
//	SendToCandidate(Subject, VOTEPROGRESS_EventEnd);
//	SendToVoters(Subject, VOTEPROGRESS_EventEnd);
//}
//
//// Call from Cancel() only
//function NotifyEventCancel(EVoteSubjectType Subject)
//{
//	SendToCandidate(Subject, VOTEPROGRESS_EventCancel);
//	SendToVoters(Subject, VOTEPROGRESS_EventCancel);
//}
//
//function NotifyEventAccept(EVoteSubjectType Subject, PlayerReplicationInfo PRI)
//{
//	SendToSinglePRI(PRI, Subject, VOTEPROGRESS_EventAccept);
//}
//
//function NotifyEventDeny(EVoteSubjectType Subject, PlayerReplicationInfo PRI)
//{
//	SendToSinglePRI(PRI, Subject, VOTEPROGRESS_EventDeny);
//}
//
//
///* SendMsg functions to Voters, Candidate, All, and so on */
//
///* PRIArray�� �ִ� ��󿡰� Msg�� ���� */
//
//function SendToSinglePRI(PlayerReplicationInfo PRI, 
//				   EVoteSubjectType Subject, EVoteProgressType Progress,
//				   optional bool bCandidate, optional float Timeleft, 
//				   optional string PlayerName, optional string AnotherName,
//				   optional int n1, optional int n2)
//{
//	AvaPlayerController(PRI.Owner).ClientReceiveVoteMsg(Subject, Progress, bCandidate, Timeleft, PlayerName, AnotherName,n1,n2);
//}
//
//function SendToPRI(array<PlayerReplicationInfo> PRI, 
//				   EVoteSubjectType Subject, EVoteProgressType Progress,
//				   optional bool bCandidate, optional float Timeleft, 
//				   optional string PlayerName, optional string AnotherName,
//				   optional int n1, optional int n2)
//{
//	Local int i;
//	for( i = 0 ; i < PRI.Length ; i++)
//		AvaPlayerController(PRI[i].Owner).ClientReceiveVoteMsg(Subject, Progress, bCandidate, Timeleft, PlayerName, AnotherName,n1,n2);
//}
//

//
//function SendToLeft(EVoteSubjectType Subject, EVoteProgressType Progress,
//				   optional bool bCandidate, optional float Timeleft, 
//				   optional string PlayerName, optional string AnotherName,
//				   optional int n1, optional int n2)
//{
//	SendToPRI(Vote[Subject].Left,  Subject, Progress, bCandidate, Timeleft, PlayerName, AnotherName,n1, n2);
//}
//
//function SendToSubmit(EVoteSubjectType Subject, EVoteProgressType Progress,
//				   optional bool bCandidate, optional float Timeleft, 
//				   optional string PlayerName, optional string AnotherName,
//				   optional int n1, optional int n2)
//{
//	SendToPRI(Vote[Subject].Submit,  Subject, Progress, bCandidate, Timeleft, PlayerName, AnotherName,n1, n2);
//}
//
//
//function SendToCandidate(EVoteSubjectType Subject, EVoteProgressType Progress,
//				   optional bool bCandidate, optional float Timeleft, 
//				   optional string PlayerName, optional string AnotherName,
//				   optional int n1, optional int n2)
//{
//	Local PlayerReplicationInfo PRI;
//	PRI = Vote[Subject].bVoting ? Vote[Subject].CandidatePRI : TempPRI;
//
//	avaPlayerController(PRI.Owner).ClientReceiveVoteMsg(Subject, Progress, bCandidate, Timeleft, PlayerName,AnotherName, n1,n2);
//}
//
//function SendToElected(EVoteSubjectType Subject, EVoteProgressType Progress,
//				   optional bool bCandidate, optional float Timeleft, 
//				   optional string PlayerName, optional string AnotherName,
//				   optional int n1, optional int n2)
//{
//	Local PlayerReplicationInfo PRI;
//	PRI = Vote[Subject].ElectedPRI;
//
//	if(PRI != None);
//	avaPlayerController(PRI.Owner).ClientReceiveVoteMsg(Subject, Progress, bCandidate, Timeleft, PlayerName,AnotherName, n1,n2);
//}
//
//function SendToTeam(EVoteSubjectType Subject, EVoteProgressType Progress,
//				   optional bool bCandidate, optional float Timeleft, 
//				   optional string PlayerName, optional string AnotherName,
//				   optional int n1, optional int n2)
//{
//	Local PlayerController PC;
//	foreach WorldInfo.AllControllers(class'PlayerController' , PC)
//		if(PC.PlayerReplicationInfo.Team.TeamIndex == Vote[Subject].CandidatePRI.Team.TeamIndex)
//			avaPlayerController(PC).ClientReceiveVoteMsg(Subject, Progress, bCandidate, Timeleft, PlayerName, AnotherName,n1,n2);
//}
//
//function SendToAll(EVoteSubjectType Subject, EVoteProgressType Progress,
//				   optional bool bCandidate, optional float Timeleft, 
//				   optional string PlayerName, optional string AnotherName,
//				   optional int n1, optional int n2)
//{
//	Local PlayerController PC;
//	foreach WorldInfo.AllControllers(class'PlayerController' , PC)
//		avaPlayerController(PC).ClientReceiveVoteMsg(Subject, Progress, bCandidate, Timeleft, PlayerName, AnotherName,n1,n2);
//}
//
//function HidePRI(PlayerReplicationInfo PRI)
//{
//	//	avaPlayerController(PRI.Owner).ClientReceiveHideVoteMsg(V);	
//}
//
//
///* VoteLogic ����, ���� Vote�� �����Ȳ ����, ����  */
//
//function bool IsAcceptable(EVoteSubjectType Subject,PlayerReplicationInfo PRI)
//{
//	switch(Subject)
//	{
//	case VOTESUBJECT_MapSelection:	return true;
//	case VOTESUBJECT_SquadLeader_EU:
//	case VOTESUBJECT_SquadLeader_USSR:
//									return true;
//	case VOTESUBJECT_KickPlayer:	return true;
//	case VOTESUBJECT_DrawGame:		return true;
//	default:
//		Assert(false); return false;
//	}
//	return true;
//}
//
//function bool CheckVote(EVoteSubjectType Subject)
//{
//	return Vote[Subject].bVoting;
//}
//
//function bool CheckRange(EVoteSubjectType Subject, PlayerReplicationInfo PRI, out EVoteSubjectType AnotherSubject)
//{
//	Local int i , v;
//
//	for( i = 0 ; i < Vote.Length ; i++ )
//	{
//		if(Vote[i].bVoting && i != Subject)
//		{
//			// All Members are Voting on
//			if(Vote[i].Range == VOTERANGE_ALL)
//			{
//				AnotherSubject = EVoteSubjectType(i);
//				return false;
//			}
//
//			// Assume that Vote[i].Range == VOTERANGE_TEAM
//			// because there's no other type in Range at the moment
//			for( v = 0 ; v < Vote[i].Voters.Length ; v++)
//			{
//				if( Vote[i].Voters[v].Team.TeamIndex == PRI.Team.TeamIndex) 
//				{
//					AnotherSubject = EVoteSubjectType(i);
//					return false;
//				}
//			}
//		}
//	}
//	return true;
//}
//
//function bool CheckSufficient(EVoteSubjectType Subject, PlayerReplicationInfo PRI)
//{
//	Local int i;
//	Local int WholeVoters;
//	if(Vote[Subject].Range == VOTERANGE_Team)
//	{
//		if(Vote[Subject].Voters.Length < Vote[Subject].nMinJoinNum)
//			return false;
//	}
//	else if(Vote[Subject].Range == VOTERANGE_ALL)
//	{
//		if(Vote[Subject].TeamVoterCount.Length != TEAM_MAX - 1)
//			return false;
//		
//		for( i = 0 ; i < Vote[Subject].TeamVoterCount.Length ; i++)
//		{
//			WholeVoters = Vote[Subject].TeamVoterCount[i] + (i == PRI.Team.TeamIndex ? 1 : 0);
////			`log("WholeVoters["$i$"] = "$WholeVoters);
//			if(WholeVoters <= 0)
//				return false;
//		}
//	}
//	
//	return true;
//}
//
//
//function StartVote(EVoteSubjectType Subject,PlayerReplicationInfo PRI, optional Object VoteObject)
//{
//	Local EVoteSubjectType AnotherSubject;
//	
//	/* TempPRI Scope Begins*/
//	TempPRI = PRI;
//	
//	/* '����� �ϻ��̻�'�� ���� ������ �÷��̾������� ����*/
//	if(!IsAcceptable(Subject, PRI))
//	{
////		`log("Invalid");
//		NotifyInvalid(Subject);
//		return;
//	}
//
//	/* ����� �÷��̾ �ٽ� ��ǥ�� �õ��Ҷ�*/
//	if( Vote[Subject].ElectedPRI == PRI)
//	{
////		`log("Already");
//		NotifyAlready(Subject);
//		return;
//	}
//
//	/* ������ FreezeTime�� �������� �÷��̾ �ٽ� �缱�� �õ�*/
//	if( Vote[Subject].Freeze.find('FreezePRI', PRI) >= 0)
//	{
////		`log("Freeze");
//		NotifyFreeze(Subject, PRI);
//		return;
//	}
//
//	/* ���� ������ ��ǥ�� 2���̻� ���� �� ����*/
//	if( CheckVote(Subject) )
//	{
////		`log("Multiple Not Allowed");
//		return;
//	}
//
//	/* ��ǥ�� �ʿ�� �ϴ� �ο��߿� �ٸ� ��ǥ�� �����ϰ� �ִ� �ο��� �ִ°� ?*/
//	if(!CheckRange(Subject, PRI, AnotherSubject))
//	{
////		`log("Another");
//		NotifyAnother(Subject, AnotherSubject);
//		return;
//	}
//
//	/* �ּ� ��ǥ�ο��� �����ϴ°� ? */
//	GetCurrentVoters(Subject);
//	if(!CheckSufficient(Subject, TempPRI))
//	{
////		`log("Insufficient");
//		NotifyInsufficient(Subject);
//		return;
//	}
//	/* TempPRI Scope End*/
//
//	/* Check all conditions, Process Next*/
//
//	`log("Ready to Vote");
//
//	Vote[Subject].CandidatePRI = PRI;
//	Vote[Subject].bVoting = true;
//	Vote[Subject].fTimeLeft = Vote[Subject].Duration;
//	Vote[Subject].Accept[ Vote[Subject].Accept.Length ] = PRI;
//	Vote[Subject].TeamAcceptCount[ PRI.Team.TeamIndex ] = 1;	// ������ Accept�� ���� 1 (�ڱ��ڽ��� ������ �����̹Ƿ�)
//	if(!IsTimerActive())
//	{
////		`log("SetTimer");
//		SetTimer(fUpdatePeriod, true);
//	}
//	NotifyVote(Subject);
//	NotifyEventStart(Subject);
//}
//
//function Timer()
//{
//	Local int i,k;
//	Local int VoteCount;
//	Local int FreezeCount;
//
//	VoteCount = 0;
//	FreezeCount = 0;
//	for( i = 0 ; i < Vote.Length ; i++)
//	{
//
//		/* Manipulate Vote Status (Left-Time Update, Notify Timeout)  */
//		if(Vote[i].bVoting)
//		{
//			if(Vote[i].fTimeLeft <= 0.0)
//			{
//				NotifyRejected(EVoteSubjectType(i));
//				ResetVoteParam(EVoteSubjectType(i));
//			}
//			else
//			{
//				NotifyVote(EVoteSubjectType(i));
//				NotifySubmit(EVoteSubjectType(i));
//				Vote[i].fTimeLeft -= fUpdatePeriod;
//			}
//			VoteCount++;
//		}
//
//		/* Manipulate Cooltime (Left-Time Update, Melt Player when time is gone)*/
//		for(k = Vote[i].Freeze.Length-1 ; k >= 0 ; k--)
//		{
//			if( Vote[i].Freeze[k].fTimeLeft <= 0.0)
//			{
//				Vote[i].Freeze.Remove(k,1);
//			}
//			else
//			{
//				Vote[i].Freeze[k].fTimeLeft -= fUpdatePeriod;
////				`log("fTimeLeft = "$Vote[i].Freeze[k].fTimeLeft);
//			}
//			FreezeCount++;
//		}
//	}
//
//	if(VoteCount == 0 && FreezeCount == 0)
//	{
////		`log("ClearTimer");
//		ClearTimer();
//	}
//}
//
//function ResetVoteParam(EVoteSubjectType Subject)
//{
//
//	Vote[Subject].bVoting = false;
//	Vote[Subject].fTimeLeft = 0.0;
//	Vote[Subject].Voters.Length = 0;
//	Vote[Subject].Accept.Length = 0;
//	Vote[Subject].Deny.Length = 0;
//	Vote[Subject].Left.Length = 0;
//	Vote[Subject].Submit.Length = 0;
//	Vote[Subject].TeamVoterCount.Length = 0;
//	Vote[Subject].TeamAcceptCount.Length = 0;
//	Vote[Subject].TeamDenyCount.Length = 0;
//
//}
//
//function bool CheckEachTeamAccept(EVoteSubjectType Subject, int TeamIndex)
//{
//	Local int left, right;
//
//	left = Vote[Subject].TeamVoterCount[TeamIndex];
//	right = Vote[Subject].TeamAcceptCount[TeamIndex];
//
//	if( Vote[Subject].CandidatePRI.Team.TeamIndex == TeamIndex)
//		left = left + 1;
//
//	left = left * Vote[Subject].fAcceptRate;
//
//	return left < right;
//}
//
//function bool CheckAccept(EVoteSubjectType Subject)
//{
//	Local int left, right, i;
//	Local int count;
//	
//	switch(Subject)
//	{
//	case VOTESUBJECT_SquadLeader_EU:
//	case VOTESUBJECT_SquadLeader_USSR:		left = Vote[Subject].fAcceptRate * (Vote[Subject].Voters.Length + 1);
//											right = Vote[Subject].Accept.Length;					
//											return left < right;
//	case VOTESUBJECT_DrawGame:													
//										count = 0;
//										for( i = 0 ; i < Vote[Subject].TeamVoterCount.Length ; i++)
//										{
//											if( CheckEachTeamAccept(Subject, i) )
//												count++;
//										}
////										`log("Count = "$count$", TeamVoterCount.Length = "$Vote[Subject].TeamVoterCount.Length); 
//										return count == (Vote[Subject].TeamVoterCount.Length );
//
//	default:	Assert(false);		break;
//
//	}
//	// Vote[Subject].Accept.Length + 1 = (������ ���) + (�ڱ��ڽ�)
//	return left < right; 
//}
//
//
//function Submit(EVoteSubjectType Subject, bool bAgree, PlayerReplicationInfo PRI)
//{
//	Local int Index;
//	Local int TeamIndex;
//
//
//	Index = Vote[Subject].Left.find(PRI);
//	/* ���� �����ڵ� �߿� ���� (��ǥ �ʱ⿡ ������ ������ ���Ե��� �ʰų�, ��ǥ �߰��� ������) */
//	if(Index < 0)
//		return;
//
//	Vote[Subject].Left.Remove(Index, 1);
//	Vote[Subject].Submit[ Vote[Subject].Submit.Length ] = PRI;
//
//	TeamIndex = PRI.Team.TeamIndex;
//	if( bAgree)
//	{
//		Vote[Subject].Accept[ Vote[Subject].Accept.Length ] = PRI;
//		Vote[Subject].TeamAcceptCount[TeamIndex] = Vote[Subject].TeamAcceptCount[TeamIndex] + 1;
//		NotifyEventAccept(Subject, PRI);
//	}
//	else
//	{
//		Vote[Subject].Deny[ Vote[Subject].Deny.Length ] = PRI;
//		Vote[Subject].TeamDenyCount[TeamIndex] = Vote[Subject].TeamDenyCount[TeamIndex] + 1;
//		NotifyEventDeny(Subject, PRI);
//	}
//	
//
//	if(CheckAccept(Subject))
//	{
//		/* Elected */
//		NotifyElected(Subject);
//		ProcessElected(Subject);		
//		ResetVoteParam(Subject);
//		return;
//	}else if(Vote[Subject].Accept.Length + Vote[Subject].Deny.Length >= Vote[Subject].Voters.Length + 1)	// +1(�ڱ��ڽ�)
//	{
//		/* Rejected */
//		AddFreezePRI(Subject, Vote[Subject].CandidatePRI);
//		NotifyRejected(Subject);
//		ResetVoteParam(Subject);
//		return;
//	}
//}
//
//function ProcessElected(EVoteSubjectType Subject, optional PlayerReplicationInfo PRI)
//{
//	switch(Subject)
//	{
//	case VOTESUBJECT_SquadLeader_EU:				
//	case VOTESUBJECT_SquadLeader_USSR:				
//		if(Vote[Subject].ElectedPRI != None)
//		{
//			ProcessChanged(Subject, Vote[Subject].ElectedPRI);
//		}
//		Vote[Subject].ElectedPRI = Vote[Subject].CandidatePRI;
//		avaPlayerReplicationInfo(Vote[Subject].ElectedPRI).bSquadLeader = true;			
////		avaGameReplicationInfo(WorldInfo.GRI).ClearTeamWaypoint(Vote[Subject].ElectedPRI.Team.TeamIndex);
//
//		break;
//	case VOTESUBJECT_DrawGame:
//		/* Game ���� �������� Sequece*/												break;
//	default:	Assert(false);	break;
//	}
//
//}
//
//function ProcessChanged(EVoteSubjectType Subject, PlayerReplicationInfo PRI)
//{
//	switch(Subject)
//	{
//	case VOTESUBJECT_SquadLeader_EU:				
//	case VOTESUBJECT_SquadLeader_USSR:
//		if(PRI != None)
//		{
//			avaPlayerReplicationInfo(PRI).bSquadLeader = false;
//			avaGameReplicationInfo(WorldInfo.GRI).ClearTeamWaypoint(PRI.Team.TeamIndex);
//		}
//		break;
//	case VOTESUBJECT_DrawGame:
//		break;
//	default:	Assert(false);	break;
//	}
//}
//
//function Cancel(EVoteSubjectType Subject, PlayerReplicationInfo PRI)
//{
//	if(Vote[Subject].bVoting && Vote[Subject].CandidatePRI == PRI)
//	{
//		AddFreezePRI(Subject, Vote[Subject].CandidatePRI);
//		NotifyCancel(Subject);
//		NotifyEventCancel(Subject);
//		ResetVoteParam(Subject);
//		return;
//	}
//}
//
//function Retire(EVoteSubjectType Subject, PlayerReplicationInfo PRI)
//{
//	Local avaPlayerReplicationInfo ElectedPRI;
//
//	if(!Vote[Subject].bVoting && Vote[Subject].ElectedPRI == PRI)
//	{
//		ElectedPRI = avaPlayerReplicationInfo(Vote[Subject].ElectedPRI);
//		NotifyRetire(Subject, ElectedPRI);
//		ProcessChanged(Subject, ElectedPRI);
//		ElectedPRI.bSquadLeader = false;
//		Vote[Subject].ElectedPRI = None;
//		ResetVoteParam(Subject);
//		return;
//	}
//}
//
//function AddFreezePRI(EVoteSubjectType Subject, PlayerReplicationInfo PRI)
//{
//	Vote[Subject].Freeze.Length = Vote[Subject].Freeze.Length + 1;
//	Vote[Subject].Freeze[ Vote[Subject].Freeze.Length - 1].FreezePRI = PRI;
//	Vote[Subject].Freeze[ Vote[Subject].Freeze.Length - 1].fTimeLeft = fFreezePeriod;
//}
//
//function DisconnectEvent(Controller Exiting)
//{
//	Local PlayerReplicationInfo PRI;
//	Local EVoteSubjectType Subject;
//	Local int Index, TeamIndex;
//
//	PRI = PlayerController(Exiting).PlayerReplicationInfo;
//	
//	Index = EVoteSubjectType(GetCurrentSubject(PRI));
//	if( Index < 0 || Index >= Vote.Length)
//		return;
//
//	Subject = EVoteSubjectType(Index);
//	if(Vote[Subject].bVoting)
//	{
//		/* ���ĺ��ڰ� ������ ���, ��ǥ�ߴ�*/
//		if(Vote[Subject].CandidatePRI == PRI)
//		{
//			NotifyLogout(Subject, PRI);
//			ResetVoteParam(Subject);
//			return;
//		}
//		else
//		{
//			TeamIndex = PRI.Team.TeamIndex;
//
//			Index = Vote[Subject].Voters.find(PRI);
//			if( Index < 0 )
//				return;
//
//			Vote[Subject].TeamVoterCount[TeamIndex] = Vote[Subject].TeamVoterCount[TeamIndex] - 1;
//			Vote[Subject].Voters.Remove(Index, 1);
//			
//			Index = Vote[Subject].Submit.Find(PRI);
//			if( Index >= 0 )
//			{
//				Vote[Subject].Submit.Remove(Index,1);
//				Index = Vote[Subject].Accept.find(PRI);
//				if(Index >= 0)
//				{
//					Vote[Subject].Accept.Remove(Index,1);
//					Vote[Subject].TeamAcceptCount[TeamIndex] = Vote[Subject].TeamAcceptCount[TeamIndex] - 1;
//				}
//				Index = Vote[Subject].Deny.find(PRI);
//				if(Index >= 0)
//				{
//					Vote[Subject].Deny.Remove(Index,1);
//					Vote[Subject].TeamDenyCount[TeamIndex] = Vote[Subject].TeamDenyCount[TeamIndex] - 1;
//
//				}
//			}
//			
//			
//			Index = Vote[Subject].Left.Find(PRI);
//			if( Index >= 0 ) 
//				Vote[Subject].Left.Remove(Index,1);
//			
//			if( !CheckSufficient(Subject, Vote[Subject].CandidatePRI) )
//			{
//				NotifyLogout(Subject, PRI);
//				ResetVoteParam(Subject);
//				return;
//			}
//			
//			if( CheckAccept(Subject) )
//			{
//				NotifyElected(Subject);
//				ProcessElected(Subject);
//				ResetVoteParam(Subject);
//				return;
//			}
//		}
//	}
//	/* ��ǥ���� �ƴѵ� �缱�� ����� ������ ���  */
//	else if( PRI == Vote[Subject].ElectedPRI )
//	{
//		ProcessChanged(Subject, PRI);
//		Vote[Subject].ElectedPRI = None;
//	}
//}
//
///* Some Getters for (Role == Authority) */
//
//function array<PlayerReplicationInfo> GetPRIArray()
//{
//	return WorldInfo.GRI.PRIArray;
//}
//
///* ��ü ��ǥ�ο��� ������ �ִ� Voters�迭�� ä��. 
//��, ��ǥ�ϰ� ���� ����� ������ �ֱ� ���� Left�迭�� ���� ä��*/
//function int GetCurrentVoters(EVoteSubjectType Subject)
//{
//	Local int i, TeamIndex;
//	Local array<PlayerReplicationInfo> PRI;
//
//	PRI = GetPRIArray();
//
//	for( i = 0 ; i < PRI.Length ; i++)
//	{
//		if( Vote[Subject].Range == VOTERANGE_ALL || 
//			(Vote[Subject].Range == VOTERANGE_TEAM && TempPRI.Team.TeamIndex == PRI[i].Team.TeamIndex) )
//		{
//			TeamIndex = PRI[i].Team.TeamIndex;
//			if(TeamIndex >= Vote[Subject].TeamVoterCount.Length)
//			{
//				Vote[Subject].TeamVoterCount.Length = TeamIndex + 1;
//				Vote[Subject].TeamAcceptCount.Length = TeamIndex + 1;
//				Vote[Subject].TeamDenyCount.Length = TeamIndex + 1;
//			}
//			if(PRI[i] != TempPRI)
//			{
//				
////				`log("CandidatePRI.PlayerName = "$TempPRI.PlayerName);
//				Vote[Subject].Voters[Vote[Subject].Voters.Length] = PRI[i];
//				Vote[Subject].Left[ Vote[Subject].Left.Length ] = PRI[i];
//				Vote[Subject].TeamVoterCount[TeamIndex] = Vote[Subject].TeamVoterCount[TeamIndex] + 1;
//			}
//		}
//	}
//
////	`log("return Length = "$Vote[Subject].Voters.Length);
//	return Vote[Subject].Voters.Length;
//}
//
//function PlayerReplicationInfo GetPRIByID(int PlayerID)
//{
//	return WorldInfo.GRI.FindPlayerByID(PlayerID);
//}
//
//function int GetCurrentSubject(PlayerReplicationInfo PRI)
//{
//	Local int i;
//
//	for( i = 0 ; i < Vote.Length ; i++)
//	{
//		if( Vote[i].bVoting)
//		{	
//			if( (Vote[i].CandidatePRI == PRI) || (Vote[i].Voters.find(PRI) >= 0) )
//				return i;
//		}
//	}
//
//	return -1;
//}
//
//function Request(int VoteRange, PlayerReplicationInfo PRI)
//{
//	Local bool bAvail, bOtherTeam, bInvolved;
//	Local float TimeLeft;
//	Local int  i;
//	Local string Candidate;
//
//	bAvail = true;
//	bOtherTeam = false;
//	for( i = 0 ; i < Vote.Length ; i ++)
//	{
//		if(Vote[i].bVoting)
//		{
//			if(Vote[i].Range == VOTERANGE_ALL)
//			{
//				bAvail = false;
//				bOtherTeam = true;
//				TimeLeft = FMAX(TimeLeft,Vote[i].fTimeLeft);
//			}
//			else if(Vote[i].Range == VOTERANGE_TEAM)
//			{
//				if(PRI.Team.TeamIndex == Vote[i].CandidatePRI.Team.TeamIndex)
//				{
//					TimeLeft = FMAX(TimeLeft, Vote[i].fTimeLeft);
//					bAvail = false;
//				}
//				else if(VoteRange == VOTERANGE_ALL)
//				{
//					TimeLeft = FMAX(TimeLeft, Vote[i].fTimeLeft);
//					bOtherTeam = true;
//				}
//			}
//		}
//	}
//
//	i = GetCurrentSubject(PRI);
//	if( i >= 0)
//	{
//		bInvolved  = true;
//		Candidate = Vote[i].CandidatePRI.PlayerName;
//	}
//
//	avaPlayerController(PRI.Owner).ResponseVote(VoteRange, bAvail, bOtherTeam, bInvolved, TimeLeft, Candidate);
//}
//
//defaultproperties
//{
//	// [�׽�Ʈ����] fAcceptRate = 0.5 -> fAcceptRate = 0.2 
//	Vote(VOTESUBJECT_SquadLeader_EU)=(Name="SquadLeader", Range=VOTERANGE_TEAM, Duration=20.0, nMinJoinNum=1, fMinVoteRate=0.2, fAcceptRate = 0.2)
//	Vote(VOTESUBJECT_SquadLeader_USSR)=(Name="SquadLeader", Range=VOTERANGE_TEAM, Duration=20.0, nMinJoinNum=1, fMinVoteRate=0.2, fAcceptRate = 0.2)
//	Vote(VOTESUBJECT_KickPlayer)=(Name="SquadLeader", Range=VOTERANGE_TEAM, Duration=10.0, nMinJoinNum=1, fMinVoteRate=0.2, fAcceptRate = 0.5)
//	Vote(VOTESUBJECT_MapSelection)=(Name="MapSelection", Range=VOTERANGE_ALL, Duration=20.0, nMinJoinNum=1, fMinVoteRate=0.5, fAcceptRate = 0.5)
//	Vote(VOTESUBJECT_DrawGame)=(Name="DrawGame", Range=VOTERANGE_ALL, Duration=30.0, nMinJoinNum=1, fMinVoteRate=0.5, fAcceptRate = 0.5)
//	
//	fUpdatePeriod = 1.0
//	fMsgDuration = 3.0
//	fFreezePeriod = 30.0
//
//	TickGroup=TG_DuringAsyncWork
//	RemoteRole=ROLE_SimulatedProxy
//	bAlwaysRelevant=true
//}