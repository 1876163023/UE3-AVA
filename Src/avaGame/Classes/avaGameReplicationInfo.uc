/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaGameReplicationInfo extends GameReplicationInfo
	config(game)
	native
	nativereplication
	dependson(avaNetHandler)
	dependson(avaGameStats);

`include(avaGame/avaGame.uci)

var hmserialize bool			bHmRoundEnd;				// [!] 20070323 dEAthcURe|HM 'hmserialize' [+] 20070302 dEAthcURe|HM
var hmserialize repnotify bool	bWarmupRound;				// [!] 20070323 dEAthcURe|HM 'hmserialize'	// Amount of Warmup Time Remaining
var hmserialize bool			bPracticeRound;				// [!] 20070323 dEAthcURe|HM 'hmserialize'	// SWGame ���� ���.

var	int							DominanceTeamIdx;			//
var int							MissionIndicatorIdx;		// Mission Indicator Index
var float						MissionTime;				// Mission Time 
var hmserialize	float			TargetMissionTime;			// 0 ���� ū ��쿡�� �ǹ̰� ����....
var float						MissionTimeRate;			// Timer Rate
var float						MissionMaxTime;				// MaxTime...
var	float						MissionTargetTime;			// Mission �� Target Time
var int							MissionTimeDir;				// 1�̸� Mission Time ����, -1 �̸� ����, 0�̸� Stop
var float						MissionReplTmpTime;			// 1�ʸ��� ReplcatedMissionTime �� ������.
var repnotify float				ReplicatedMissionTime;		// Replcation �� ���� Mission Time
var	float						MissionCheckPoint[5];		// Mission Check Point

var	int							WinCondition;				// ��ǥ�¼�

var PlayerReplicationInfo		TopLevelPRI[2];				// ���� Top Level PRI

var	float						BaseScore;					// Level �� ���� Score

var	hmserialize repnotify int	ElapsedSyncTime;			// [!] 20070323 dEAthcURe|HM 'hmserialize'	//

var avaObjectPool				ObjectPool;

var bool						bReportEndGame;
var bool						bReportEndGameToServer;
var hmserialize int				nWinTeam;					// [!] 20070323 dEAthcURe|HM 'hmserialize'

var	localized	string			TeamNames[3];

var	int							FriendlyFireType;			// Friendly Fire Type
var hmserialize	bool			bEnableGhostChat;			// true �̸� Ghost Chat �� ����մϴ�.
var hmserialize bool			bSwapRule;					// true �̸� Swap Rule�� �����Ѵ�...
var hmserialize bool			bSwappedTeam;				// true �̸� Swap Rule�� ����Ȱ��̴�....

var globalconfig string			LocalizedTeamPackName[2];
var globalconfig string			InternationalTeamPackName[2];
var	class<avaLocalizedTeamPack>	LocalizedTeamPack[2];

var repnotify int				MissionHelp[2];

var hmserialize	bool			bRoundTimeOver;

struct native KOTH3TeamStatus
{
	var		int				NumPlayersInside;	// �� ������ �ִ� ��� ��
	var		float			TimeRemains;			// ������� ȹ���� �ð�
};

var KOTH3TeamStatus				KOTH3[2];

var bool						bTestBroadcast;

enum EFlagState
{
    FLAG_Home,
    FLAG_HeldFriendly,
    FLAG_HeldEnemy,
    FLAG_Down,
};

var private EFlagState FlagState[2];

enum EMissionType
{
	MISSION_Bombing,							//< ��ź ��ġ (���Ĺ̼�)
	MISSION_KOTH,								//< �Ϲ� King Of The Hill (���ɹ̼�)
	MISSION_KOTH_EX,							//< Ȯ��� King Of The Hill (���ɹ̼�)
	MISSION_Transport,							// (���۹̼�)
	MISSION_Annihilation,						// ������
	MISSION_TotalWar,							// ������
};

enum EMissionCategoryType
{
	MISSIONCATEGORY_DESTRUCTION,
	MISSIONCATEGORY_CONQUER,
	MISSIONCATEGORY_TRANSPORT,
	MISSIONCATEGORY_ANNIHILATION,
	MISSIONCATEGORY_TOTALWAR,
};

var EMissionType MissionType;
var ETeamType AttackTeam;

var ETeamType			RoundWinner;					//
var hmserialize int		CurrentRound;	// [!] 20070323 dEAthcURe|HM 'hmserialize'	//
var int					CurrentRoundTimeLimit;			// 
var hmserialize bool	bReinforcement;	// [!] 20070323 dEAthcURe|HM 'hmserialize'	// Respawn �� �Ǵ°�?

var int					ReinforcementFreq[`MAX_TEAM];	// ������ ���ʸ��� �����ϴ°�?
var	hmserialize int		DogTagPackCnt;	// [!] 20070323 dEAthcURe|HM 'hmserialize'
var hmserialize bool	bAllowThirdPersonCam;	// true �̸� Third Person Camera �� �����Ѵ�...

var avaVoteSystem		Vote;					// �д��� ����, ���� ���⸦ ����. ��ǥ ���۰� ����� �̺�Ʈ�� �߻��Ѵ�.

var string				VoteTitleMsg;			// Vote �� Title Message
var string				VoteWarn;				// Vote warn Message
var string				VotingMsg;				// ���� ��ǥ�� �������� ������ �ϴ� Message
var string				VoteProgressMsg;		// Vote �� Progress Message
var string				VoteResultMsg;			// Vote �� ���� ��� Message
var string				VoteTargetName;			// Vote ������� �̸�
var string				VoteTimerMsg;			// Vote �� ���� �ð��� ǥ��
var bool				bVoting;				// ���� Vote �� �������ΰ�?
var bool				AmIVote;				// ���� Vote �� �Ϸ��߳�?
var bool				AmITarget;				// ���� Vote �� ������ΰ�?
var bool				bVoteProgressMsg;		// true �̸� VoteProgressMsg �� ����ش�.
var bool				bVoteResultMsg;			// true �̸� VoteResultMsg �� ����ش�..
var int					VoteLeftTime;			// Vote �����ð�.


var int					VoteAcceptCnt;			// Vote �� ������ ����
var int					VoteDenyCnt;			// Vote �� �ݴ��� ����

/* Strategy Command ���� */
enum EWaypointTeamType
{
	WPTeam_Blue,
	WPTeam_Yellow,
};

enum EWaypointActionType
{
	WPAction_Set,
	WPAction_Reset,
	WpAction_Clear,
};

struct native WPInfo
{
	var array<vector>	History;
};


var array<WPInfo>			Waypoint;		// �д�1�� ���� ��������Ʈ �����丮. �����Ǿ� (Lenght - 1)�� ��ġ�� ������ ��ġ�̴�.
var repnotify vector		CurrentWaypoint[4];	// CurrentWaypoint[2][2], 
											// EU, USSR���� 2������ ���� 2���� ��������Ʈ. ���� Waypoint1,2�� �ߺ�������
											// Replication�� ���� ������ �ִ�.

/** avaSeqAct_SWGameInit�� avaGameReplicationInfo���� ����ϱ� ���� Enumeration */
enum EHUDStateType
{
	HUDSTATE_DefaultHUD,
	HUDSTATE_Spectator,
	HUDSTATE_Dead
};

/** avaSeqAct_SWGameInit�� avaGameReplicationInfo�� SceneStateDataType������ ����ϱ� ���� structure */
struct native OpenStatusType
{
	var EHUDStateType	HUDStateName;
	var() bool			bOpen;
	structdefaultproperties
	{
		bOpen=false
	}
};

/** avaSeqAct_SWGameInit�� avaGameReplicationInfo���� �����ϰ� avaHUD���� ����ϱ� ���� OptionalScene-State Data structure */
struct native SceneStateDataType
{
	var() string					SceneName;
	var() OpenStatusType			OpenStatus[EHUDStateType.HUDSTATE_MAX];	/**< open or not */

	structdefaultproperties
	{
		OpenStatus(HUDState_DefaultHUD) = (HUDStateName=HUDSTATE_DefaultHUD)
		OpenStatus(HUDState_Spectator) = (HUDStateName=HUDSTATE_Spectator)
		OpenStatus(HUDState_Dead) = (HUDStateName=HUDSTATE_Dead)
	}
};

/** OptionalScene Names�� Replication. OptionalScene�� Level���� �ʿ信 ���� ������ Scene�� ����Ŵ */
`define MAX_OPTIONAL_SCENES		5
var repnotify SceneStateDataType		OptionalSceneData[`MAX_OPTIONAL_SCENES];
var repnotify int						OptionalSceneCount;
var const int							MaxOptionalScenes;
var bool								bUpdateOptionalScenes;

/**	�ؽ�Ʈ ��Ÿ�Ͽ� ���� ������
  * ���� ��� ���޼����� ������� ���, �ӼӸ��� �ϴû����� ��� ��� */
struct native TextStyleInfo
{
	var int			id;
	var Name		Name;
	var string		StyleTag;

	structdefaultproperties
	{
		StyleTag="DefaultListTextStyle"
	}
};

/** DefaultGame.ini���� TextStyle������ �߰��ϵ����Ͽ��� */
var globalconfig array<TextStyleInfo>		TextStyleData;

// {{ 20070420 dEAthcURe|HM
`define MAX_HmEvent 8
var hmserialize repnotify string HmEvents[`MAX_HmEvent];

`define MAX_HmVariable 8
var hmserialize repnotify string HmVariables[`MAX_HmVariable];

// {{ 20070423 hm int vars
`define Max_HmIntVar 8
var hmserialize repnotify string HmIntVars[`Max_HmIntVar];
// }} 20070423 hm int vars


// game option ����....
var globalconfig bool		UseLocalSound;			// ���������� Local Sound �� ����Ѵ�...
var	globalconfig bool		UseInGameHelp;			// InGameHelp �� �������� �Ⱥ������� flag
var globalconfig bool		UseFirstPersonEffect;	// ���������� FirstPerson Effect�� ����Ѵ�.

replication
{
	if ( bNetInitial && (Role==ROLE_Authority) )
		MissionType, OptionalSceneData, OptionalSceneCount;

	if (Role==ROLE_Authority)
		bWarmupRound, bPracticeRound, FlagState, CurrentRound, bReinforcement, 
		Vote, CurrentWaypoint, ReinforcementFreq, BaseScore, ElapsedSyncTime, 
		DogTagPackCnt, bHmRoundEnd, nWinTeam, HmEvents, HmVariables, HmIntVars, // 20070302 dEAthcURe|HM bHmRoundEnd �߰�
		bAllowThirdPersonCam,AttackTeam,bRoundTimeOver;

	if (bNetOwner && Role == ROLE_Authority)
		DominanceTeamIdx,MissionIndicatorIdx,ReplicatedMissionTime, 
		MissionTargetTime, MissionTimeDir, MissionTimeRate, MissionMaxTime, WinCondition, TargetMissionTime,
		FriendlyFireType, bEnableGhostChat, MissionHelp, MissionCheckPoint, KOTH3,
		bSwapRule, bSwappedTeam;
}

native final function float GetAverageFPS();

function int findHmEvent(string eventName)
{
	// return idx of event, -1 does not exist	
	local int lpp;
	
	for(lpp=0;lpp<`MAX_HmEvent;lpp++) {
		if(HmEvents[lpp] == eventName) {
			return lpp;
		}
	}
	return -1;
}

function bool setHmEvent(string eventName)
{
	local int lpp;
	
	if(findHmEvent(eventName)==-1) {
		for(lpp=0;lpp<`MAX_HmEvent;lpp++) {
			if(HmEvents[lpp] == "") {
				HmEvents[lpp] = eventName;
				NetUpdateTime = WorldInfo.TimeSeconds - 1;
				`log("   <---- setHmEvent HmEvents[lpp] " @ HmEvents[lpp]);
				return true;
			}
		}
	}
	return false;
}

function bool resetHmEvent(string eventName)
{
	local int idx;
	
	idx = findHmEvent(eventName);
	
	if(idx!=-1) {
		HmEvents[idx] = "";
		NetUpdateTime = WorldInfo.TimeSeconds - 1;
		`log("   <---- resetHmEvent HmEvents[lpp] " @ HmEvents[idx]);
		return true;
	}	
	return false;	
}

function ActivateRemoteEvent( string _EventName )
{
	// code from avaNetEntryGameEx.uc
	
	local Sequence					GameSeq;
	local int						i;
	local array<SequenceObject>		AllSeqEvents;
	local SeqEvent_RemoteEvent		RemoteEvent;

	// World���� Level Kismet�� Sequence�� ��´�.
	GameSeq = WorldInfo.GetGameSequence();
	if ( GameSeq != None )
	{
		// RemoteEvent�� ��� ã�Ƽ�.
		GameSeq.FindSeqObjectsByClass( class'SeqEvent_RemoteEvent', true, AllSeqEvents );

		for ( i = 0; i < AllSeqEvents.Length; ++ i )
		{
			RemoteEvent = SeqEvent_RemoteEvent( AllSeqEvents[i] );

			if ( RemoteEvent == None )
				continue;

			if ( RemoteEvent.bEnabled != true )
				continue;

			// �߻��ϰ� ���� �̺�Ʈ �̸��� RemoteEvent�� �߻���Ų��.
			if( string(RemoteEvent.EventName) == _EventName )
			{
				`log("### RemoteEvent - " @ _EventName @"###");
				RemoteEvent.CheckActivate( WorldInfo, None );
			}
		}
	}
}

// {{ 20070504 dEAthcURe|HM
function triggerHmEvent()
{
	local array<SequenceObject> hmEventList; // 20070423
	local avaSeqEvent_Hm hmEvent; // 20070423
	local Sequence GameSeq;
	local int lpp;
	
	`log("          <---------- [avaGameReplicationInfo::triggerHmEvent]");
	
	GameSeq = WorldInfo.GetGameSequence();
	if ( GameSeq != none )
	{
		GameSeq.FindSeqObjectsByClass( class'avaSeqEvent_Hm', true, hmEventList ); // if(FindEventsOfClass(class'avaSeqEvent_Hm', hmEventList)) {
		`log("[dEAthcURe|avaGRI|triggerHmEvent] #hmEventList=" @ hmEventList.Length);	
		for(lpp=0;lpp<hmEventList.Length;lpp++) {
			hmEvent = avaSeqEvent_Hm(hmEventList[lpp]);
			if(hmEvent != None) {
				`log("[dEAthcURe|avaGRI|triggerHmEvent] Triggering..." @ lpp);
				hmEvent.Trigger(WorldInfo);
			}
		}
	}
	else {
		`log("[dEAthcURe|avaGRI|triggerHmEvent] No GameSequence found");	
	}
}
// }} 20070504 dEAthcURe|HM

event function onHmRestore()
{
	//local array<avaSeqEvent_Hm> hmEventList; // [-] 20070504 dEAthcURe|HM // 20070423
	//local avaSeqEvent_Hm hmEvent; // [-] 20070504 dEAthcURe|HM // 20070423
	//local Sequence GameSeq; // [-] 20070504 dEAthcURe|HM // 20070423 
	local int lpp;
	
	`log("[dEAthcURe|avaGRI::onHmRestore]");
	for(lpp=0;lpp<`MAX_HmEvent;lpp++) {
		if(HmEvents[lpp] != "") {
			`log("[dEAthcURe|avaGRI::onHmRestore] firing HmEvent" @ HmEvents[lpp]);
			ActivateRemoteEvent(HmEvents[lpp]);	
		}		
	}	
	
	/* [-] 20070423 ��� avaSeqEvent_Hm���� ó���Ѵ�.
	`log("[dEAthcURe|avaGRI::onHmRestore] firing HmGRICompleted");
	ActivateRemoteEvent("HmGRICompleted");
	*/
	
	/* [-] 20070504
	// {{ 20070423 dEAthcURe|HM ���ο� event type����	
	GameSeq = WorldInfo.GetGameSequence();
	if ( GameSeq != none )
	{
		GameSeq.FindSeqObjectsByClass( class'avaSeqEvent_Hm', true, hmEventList ); // if(FindEventsOfClass(class'avaSeqEvent_Hm', hmEventList)) {
		`log("[dEAthcURe|avaGRI|onHmRestore] #hmEventList=" @ hmEventList.Length);	
		for(lpp=0;lpp<hmEventList.Length;lpp++) {
			hmEvent = hmEventList[lpp];
			if(hmEvent != None) {
				`log("[dEAthcURe|avaGRI|onHmRestore] Triggering..." @ lpp);
				hmEvent.Trigger(WorldInfo);
			}
		}
	}
	else {
		`log("[dEAthcURe|avaGRI|onHmRestore] No GameSequence found");	
	}
	// }} 20070423 dEAthcURe|HM ���ο� event type����
	*/
}

event function int findHmVariable(string hmVarName)
{
	// return idx of event, -1 does not exist	
	local int lpp;
	
	for(lpp=0;lpp<`MAX_HmVariable;lpp++) {
		if(HmVariables[lpp] == hmVarName) {
			return lpp;
		}
	}
	return -1;
}

event function bool setHmVariable(string hmVarName)
{
	local int lpp;
	
	if(findHmVariable(hmVarName)==-1) {	
		for(lpp=0;lpp<`MAX_HmVariable;lpp++) {
			if(HmVariables[lpp] == "") {
				HmVariables[lpp] = hmVarName;
				NetUpdateTime = WorldInfo.TimeSeconds - 1;
				`log("   <---- setHmVariable HmVariables[lpp] " @ HmVariables[lpp]);
				return true;
			}
		}
	}
	return false;
}

event function bool resetHmVariable(string hmVarName)
{
	local int idx;
	
	idx = findHmVariable(hmVarName);
	
	if(idx!=-1) {
		HmVariables[idx] = "";
		NetUpdateTime = WorldInfo.TimeSeconds - 1;
		`log("   <---- resetHmVariable HmVariables[lpp] " @ HmVariables[idx]);
		return true;
	}	
	return false;	
}
// }} 20070420 dEAthcURe|HM


cpptext
{
	// {{ 20070423
	int findHmIntVar(FString hmIntVarName);
	bool setHmIntVar(FString hmIntVarName, int value);
	bool resetHmIntVar(FString hmIntVarName);
	// }} 20070423
	
	#ifdef EnableHostMigration
	virtual void hmSerialize(FArchive& Ar); // 20061219 dEAthcURe|HM
	#endif
	
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );
}

simulated function PostBeginPlay()
{
	Local class<avaVoteSystem> VoteClass;
//	Local avaMissionTriggerVolume MissionVolume;
	//local int i;
	//local array< class<object> >	Modifiers;
	//class'ClassIterator'.static.FindSubClass( class'avaModifier', Modifiers );

	//for ( i = 0 ; i < Modifiers.length ; ++ i )
	//{
	//	if ( class<avaCharacterModifier>( Modifiers[i] ) != None   )
	//		CharacterModifierList[ CharacterModifierList.length ] = class<avaModifier>( Modifiers[i] );
	//	else if ( class<avaMod_Weapon>( Modifiers[i] ) != None  )
	//		WeaponModifierList[ WeaponModifierList.length ] = class<avaModifier>( Modifiers[i] );
	//}

	super.PostBeginPlay();

	if ( Role == ROLE_Authority )
	{
		VoteClass = class<avaVoteSystem>(DynamicLoadObject("avaGame.avaVoteSystem", class'class'));
		Assert(VoteClass != None);
		Vote = Spawn(VoteClass);
		Vote.WorldInfo.GRI = Self;
	}

	ObjectPool = Spawn( class'avaObjectPool' );
	UpdateWarmupRound();
}

// Team ���� Level �� ���� ���� User �� ����Ѵ�.
simulated function RecalcHighestUser()
{
	local int						i;
	local int						TopLevel[2];
	local avaPlayerReplicationInfo	avaPRI;
	local int						TeamIdx;
	local int						TopLevelCnt[2];
	TopLevel[0] = -1;
	TopLevel[1] = -1;
	for ( i = 0 ; i < PRIArray.Length ; ++ i )
	{
		avaPRI	= avaPlayerReplicationInfo( PRIArray[i] );
		if ( avaPRI.Team == None ) continue;
		TeamIdx	= PRIArray[i].Team.TeamIndex;
		if ( TopLevel[TeamIdx] < avaPRI.Level )
		{
			TopLevelPRI[TeamIdx]	= PRIArray[i];
			TopLevel[TeamIdx]		= avaPRI.Level;
			TopLevelCnt[TeamIdx]	= 1;
		}
		else if ( TopLevel[TeamIdx] == avaPRI.Level )
		{
			++ TopLevelCnt[TeamIdx];
		}
	}

	if ( TopLevelCnt[0] > 3 )	TopLevelPRI[0] = None;
	if ( TopLevelCnt[1] > 3 )	TopLevelPRI[1] = None;

}

simulated function AddPRI(PlayerReplicationInfo PRI)
{
	local int i;
    for (i=0; i<PRIArray.Length; i++)
    {
		if (PRIArray[i] == PRI)
			return;
    }
	Super.AddPRI( PRI );
	avaPlayerReplicationInfo( PRI ).FetchCharacterModifier();
	for ( i = 0 ; i < 3 ; ++ i )
	{
		avaPlayerReplicationInfo( PRI ).FetchCharacterModifier(i);
		avaPlayerReplicationInfo( PRI ).FetchWeaponModifier(i);
	}
}

simulated function RemovePRI(PlayerReplicationInfo PRI)
{
	Super.RemovePRI( PRI );
	RecalcHighestUser();
}

simulated event ReplicatedEvent(name VarName)
{
	local int lpp;
	
	if ( VarName == 'ReplicatedMissionTime' )
		MissionTime = ReplicatedMissionTime;
	//else if ( VarName == 'CurrentWaypoint')
	//	NotifyWaypoint();
	else if ( VarName == 'ElapsedSyncTime' )
		ElapsedTime = ElapsedSyncTime;
	else if ( VarName == 'OptionalSceneData')
		bUpdateOptionalScenes=true;
	else if ( VarName == 'bWarmupRound' )
		UpdateWarmupRound();
	// {{ 20070420 dEAthcURe|HM
	else if (VarName == 'HmEvents') {
		`log("          <---------- HmEvents replicated");
		for(lpp=0;lpp<`MAX_HmEvent;lpp++) {
			`log(HmEvents[lpp]);
		}
	}
	else if (VarName == 'HmVariables') {
		`log("          <---------- HmVariables replicated");
		for(lpp=0;lpp<`MAX_HmVariable;lpp++) {
			`log(HmVariables[lpp]);
		}
	}
	else if (VarName == 'HmIntVars') {
		`log("          <---------- HmIntVars replicated");
		for(lpp=0;lpp<`MAX_HmIntVar;lpp++) {
			`log(HmIntVars[lpp]);
		}
	}	
	else if (VarName == 'MissionHelp' )
	{
		UpdateMissonHelp();
	}
	// }} 20070420 dEAthcURe|HM
	else
		Super.ReplicatedEvent( VarName );
}

simulated function SetFlagHome(int TeamIndex)
{
	FlagState[TeamIndex] = FLAG_Home;
	NetUpdateTime = WorldInfo.TimeSeconds - 1;
}

simulated function bool FlagIsHome(int TeamIndex)
{
	return ( FlagState[TeamIndex] == FLAG_Home );
}

function SetWarmupRound( bool bFlag )
{
	bWarmupRound = bFlag;
	UpdateWarmupRound();
}

simulated function UpdateMissonHelp()
{
	local PlayerController PC;
	foreach LocalPlayerControllers(PC)
	{
		avaHUD( PC.myHUD ).ShowMissionHelp();
	}
}

simulated function UpdateWarmupRound()
{
	Local PlayerController PC;
	foreach LocalPlayerControllers(PC)
	{
		avaHUD( PC.myHUD ).UpdateWarmupRound();
	}
}

simulated function bool FlagsAreHome()
{
	return ( FlagState[0] == FLAG_Home && FlagState[1] == FLAG_Home );
}

function SetFlagHeldFriendly(int TeamIndex)
{
	FlagState[TeamIndex] = FLAG_HeldFriendly;
}

simulated function bool FlagIsHeldFriendly(int TeamIndex)
{
	return ( FlagState[TeamIndex] == FLAG_HeldFriendly );
}

function SetFlagHeldEnemy(int TeamIndex)
{
	FlagState[TeamIndex] = FLAG_HeldEnemy;
}

simulated function bool FlagIsHeldEnemy(int TeamIndex)
{
	return ( FlagState[TeamIndex] == FLAG_HeldEnemy );
}

function SetFlagDown(int TeamIndex)
{
	FlagState[TeamIndex] = FLAG_Down;
}

simulated function bool FlagIsDown(int TeamIndex)
{
	return ( FlagState[TeamIndex] == FLAG_Down );
}

simulated function ActivateUIRemoteEvent( name EventName )
{
	local int						i, j;
	local array<UIEvent>			AllUIEvents;
	local avaUIEvent_UIRemoteEvent	Event;
	local UIScene					ActiveScene;
	local UIInteraction				UIController;
	local PlayerController			localPC;

	foreach LocalPlayerControllers( localPC )
	{
		break;
	}

	if ( localPC == None )	return;

	UIController = LocalPlayer(localPC.Player).ViewportClient.UIController;

	if ( UIController.SceneClient == None )
	{
		return;
	}

	// ��� Ȱ��ȭ�� ��鿡 ���ؼ� ã�´�.
	for( i = 0 ; i < UIController.SceneClient.ActiveScenes.Length ; ++i )
	{
		ActiveScene = UIController.SceneClient.ActiveScenes[i];

		if( ActiveScene != None )
			ActiveScene.FindEventsOfClass(class'avaUIEvent_UIRemoteEvent', AllUIEvents);

		for ( j = 0; j < AllUIEvents.Length; ++j )
		{
			Event = avaUIEvent_UIRemoteEvent( AllUIEvents[j] );
			if ( Event == None )
				continue;

			// ���� �̸��� UI RemoteEvent�� Ȱ��ȭ �����ش�.
			if ( Event.EventName == EventName )
			{
				Event.ActivateUIEvent(0, ActiveScene);
			}
		}
	}
}

simulated function ReadyForWarfare()
{
	ActivateUIRemoteEvent( 'ReadyForWarfare' );
}

simulated function Timer()
{
	if ( WorldInfo.NetMode == NM_Client )
	{
		if ( !bWarmupRound )
		{
			ElapsedTime++;
		}

		// WarmUp Round �̸鼭 RemainingTime �� �ִٴ� ���� 
		// Waiting For Other Player ���� Ready For Warfare �� �ٲ���� �ȴٴ� ���̴�...
		if ( RemainingTime > 0 && bWarmupRound == true )
			ReadyForWarfare();

		if ( RemainingMinute != 0 )
		{
			RemainingTime = RemainingMinute;
			RemainingMinute = 0;
		}
		if ( (RemainingTime > 0) && !bStopCountDown )
			RemainingTime--;



		SetTimer(WorldInfo.TimeDilation, true);
	}


	if ( bVoting && VoteLeftTime > 0 )
	{
		--VoteLeftTime;
		UpdateVoteLeftTime();
	}

	//if ( WorldInfo.NetMode == NM_Client && bWarmupRound )
	//{
	//	if ( RemainingTime > 0 )
	//		RemainingTime--;
	//}
}

simulated function bool InOrder( PlayerReplicationInfo P1, PlayerReplicationInfo P2 )
{
	if( P1.bOnlySpectator == true && P2.bOnlySpectator == false )
		return false;
	else if( P1.bOnlySpectator == false && P2.bOnlySpectator == true )
		return true;

	if( P1.Team != None && P2.Team != None)
	{
		if( P1.Team.TeamIndex > P2.Team.TeamIndex )
			return false;
		else if( P1.Team.TeamIndex < P2.Team.TeamIndex ) 
			return true;
	}

    if( P1.Score < P2.Score )
		return false;
    if( P1.Score == P2.Score )
    {
		if ( P1.Deaths > P2.Deaths )
			return false;
		if ( P1.Deaths == P2.Deaths )
		{
			// @TODO: StrCmp�� ������ �̸����� �����Ϸ� �Ͽ�����, StrCmp�� UT2004������ �����ϱ� ������, ������ PlayerID�� �˻�
			if( P1.PlayerID > P2.PlayerID )
				return false;
			else if( P1.PlayerID < P2.PlayerID )
				return true;
		}		
	}
    return true;
}


function SetMissionMaxTime( float MaxTime )
{
	MissionMaxTime	= MaxTime;
	NetUpdateTime	= WorldInfo.TimeSeconds - 1;
}

// Swap Rule ������ �� Target Time �̴�... �����ǿ� ��ϵ� Mission Time �̴�...
function SetTargetMissionTime( float fTime )
{
	TargetMissionTime	=	fTime;
	NetUpdateTime	= WorldInfo.TimeSeconds - 1;
}

event function TriggerSucceedMission()
{
	local Sequence				GameSeq;
	local array<SequenceObject>	AllSeqEvents;
	local int					i;
	GameSeq = WorldInfo.GetGameSequence();
	if ( GameSeq != none )
	{
		GameSeq.FindSeqObjectsByClass( class'avaSeqEvent_SucceedMission', true, AllSeqEvents );
		for ( i=0; i<AllSeqEvents.Length; ++i )
			avaSeqEvent_SucceedMission( AllSeqEvents[i] ).Trigger( WorldInfo );
	}
}

function SetCurrentMissionTime( float fTime )
{
	ReplicatedMissionTime	= fTime;
	MissionTime				= ReplicatedMissionTime;
	NetUpdateTime			= WorldInfo.TimeSeconds - 1;

	if ( TargetMissionTime > 0 && MissionTime > TargetMissionTime )
		TriggerSucceedMission();
}

simulated function ClientSetCurrentMissionTime( float fTime )
{
	MissionTime				= fTime;
}

function SetMissionIndicatorIdx( int nIdx )
{
	MissionIndicatorIdx		= nIdx;
	NetUpdateTime			= WorldInfo.TimeSeconds - 1;
	`log( "avaGRI.SetMissionIndicatorIdx" @MissionIndicatorIdx );
}

function StartMissionTime( int nMissionIndicatorIdx, float StartTime, float TargetTime, optional float rate = 1.0, optional int nDominanceTeamIdx = -1 )
{
	DominanceTeamIdx		= nDominanceTeamIdx;	
	MissionIndicatorIdx		= nMissionIndicatorIdx;
	MissionTime				= StartTime;
	ReplicatedMissionTime	= StartTime;
	MissionTargetTime		= TargetTime;
	MissionTimeRate			= rate;
	if ( MissionTargetTime > MissionTime )		MissionTimeDir = 1;
	else if ( MissionTargetTime < MissionTime )	MissionTimeDir = -1;
	NetUpdateTime = WorldInfo.TimeSeconds - 1;
	`log( "avaGRI.StartMissionTime" @MissionIndicatorIdx );
}

function AddMissionCheckPoint( float fCheckPoint )
{
	local int i;
	for ( i = 0 ; i < 5 ; ++ i )
	{
		if ( MissionCheckPoint[i] == 0.0 )
		{
			MissionCheckPoint[i] = fCheckPoint;
			break;
		}
	}
}

function ResetMissionTime()
{
	DominanceTeamIdx		= -1;
	MissionIndicatorIdx		= -1;
	MissionTime				= 0.0;
	ReplicatedMissionTime	= 0.0;
	MissionTargetTime		= 0.0;
	MissionTimeDir			= 0;
	NetUpdateTime = WorldInfo.TimeSeconds - 1;
	`log( "avaGRI.ResetMissionTime" @MissionIndicatorIdx );
}

function StopMissionTime()
{
	MissionTimeDir			= 0;
	ReplicatedMissionTime	= MissionTime;
	NetUpdateTime = WorldInfo.TimeSeconds - 1;
}

function RestartMissionTime()
{
	if ( MissionTargetTime > MissionTime )		MissionTimeDir = 1;
	else if ( MissionTargetTime < MissionTime )	MissionTimeDir = -1;
	ReplicatedMissionTime	= MissionTime;
	NetUpdateTime = WorldInfo.TimeSeconds - 1;
}

function EndMissionTime()
{
	ReplicatedMissionTime	= MissionTargetTime;
	MissionTimeDir			= 0;
	NetUpdateTime = WorldInfo.TimeSeconds - 1;
}

simulated function Tick( float DeltaTime )
{
	Super.Tick( DeltaTime );
	if ( MissionTimeDir == 0 )	return;
	if ( MissionTimeDir > 0 )
	{
		if ( MissionTime < MissionTargetTime )	MissionTime += DeltaTime * MissionTimeRate;
		if ( MissionTime > MissionTargetTime )	MissionTime = MissionTargetTime;
	}
	else
	{
		if ( MissionTime > MissionTargetTime )	MissionTime -= DeltaTime * MissionTimeRate;
		if ( MissionTime < MissionTargetTime )	MissionTime = MissionTargetTime;
	}

	if ( Role == ROLE_Authority )
	{
		MissionReplTmpTime += DeltaTime;
		if ( MissionReplTmpTime > 1.0 )
		{
			MissionReplTmpTime = 0.0;
			ReplicatedMissionTime	= MissionTime;
			NetUpdateTime = WorldInfo.TimeSeconds - 1;
		}
	}
}

/************ WayPoint Related *****************

�ſ� ���� ������ ���׵�
* CurrentWaypoint[0] = (EU�� 1�� ��������Ʈ��) , CurrentWaypoint[1] = (EU�� 2�� ��������Ʈ��) ,
* CurrentWaypoint[2] = (USSR�� 1�� ��������Ʈ ��) , CurrentWaypoint[3] = (USSR�� 2�� ��������Ʈ ��)
* ��������Ʈ �� ���ڰ� �ø� CurrentWaypoint�� ����� 2�� �÷�����Ѵ�.
* ��ġ ������ �迭 CurrentWaypoint[2][2]�� CurrentWaypoint[3][2]�� �ø��°Ͱ� ����.

***********************************************/

//function SetCurrentWaypoint(vector WaypointLocation, EWaypointTeamType Index, PlayerReplicationInfo PRI)
//{
//	Local int TeamIndex;
//	Local int WaypointAction;
//	Local avaMsgParam Param;
//
//	if( index >= WPTeam_MAX)
//	{
//		Assert(false);
//		return;
//	}
//	
//	TeamIndex = (PRI.Team.TeamIndex * WPTeam_MAX) + Index;
//	if(CurrentWaypoint[TeamIndex] == WaypointLocation)
//		return;
//
//	WaypointAction = VSize(CurrentWaypoint[TeamIndex]) == 0 ? WPAction_Set  : WPAction_Reset;
//	CurrentWaypoint[TeamIndex] = WaypointLocation;
//
//	if(Waypoint.Length <= TeamIndex)
//		Waypoint.Length = TeamIndex + 1;
//
//	(Waypoint[TeamIndex]).History.Length = (Waypoint[TeamIndex]).History.Length + 1;
//	(Waypoint[TeamIndex]).History[ Waypoint[TeamIndex].History.Length - 1 ] = WaypointLocation;
//
//	Param = class'avaMsgParam'.static.Create();
//	Param.SetInt(WaypointAction, Index);
//
//	NotifyWaypoint();
//	avaPlayerController(PRI.Owner).BroadcastTeamParam(class'avaWaypointMessage', 0 , Param);
//}
//
//
//function ClearCurrentWaypoint(EWaypointTeamType Index, PlayerReplicationInfo PRI)
//{
//	Local int TeamIndex;
//	Local int WaypointAction;
//	Local avaMsgParam Param;
//	
//	TeamIndex = (PRI.Team.TeamIndex * WPTeam_MAX) + Index;
//	
//	if( VSize(CurrentWaypoint[TeamIndex]) == 0 )
//		return;
//
//	CurrentWaypoint[TeamIndex].x = 0;
//	CurrentWaypoint[TeamIndex].y = 0;
//	CurrentWaypoint[TeamIndex].z = 0;
//
//	WaypointAction = WPAction_Clear;
//	Param = class'avaMsgParam'.static.Create();
//	Param.SetInt(WaypointAction,Index);
//
//	NotifyWaypoint();
//	avaPlayerController(PRI.Owner).BroadcastTeamParam(class'avaWaypointMessage', 0 , Param);
//}
//
//function ClearTeamWaypoint(int TeamIndex)
//{
//	Local int i,WPIndex;
//	
//	for( i = 0 ; i < WPTeam_MAX ; i++)
//	{
//		WPIndex = i + (TeamIndex * WPTeam_MAX);
//		CurrentWaypoint[WPIndex].x = 0;
//		CurrentWaypoint[WPIndex].y = 0;
//		CurrentWaypoint[WPIndex].z = 0;
//	}
//}
//simulated function NotifyWaypoint()
//{
//	Local PlayerController PC;
//	foreach LocalPlayerControllers(PC)
//	{
//		avaPlayerController(PC).NotifyModifiedWaypoint();
//	}
//}
//
//simulated function bool IsEmptyWaypoint(EWaypointTeamType Index, optional PlayerReplicationInfo TargetPRI)
//{
//	Local PlayerReplicationInfo PRI;
//	Local PlayerController P, PC;
//	foreach LocalPlayerControllers(P)
//		PC = P;
//	
//	PRI = TargetPRI != None ? TargetPRI : PC.PlayerReplicationInfo;
//	
//	// Team == None , When the Game Started ( a Team of Player is not certain )
//	if( PRI == None || PRI.Team == None )
//		return true;
//	else
//		return VSize(CurrentWaypoint[PRI.Team.TeamIndex * WPTEAM_MAX + Index]) == 0;
//}
//
//simulated function vector GetWaypoint(EWaypointTeamType Index)
//{
//	Local PlayerController P, PC;
//	Local PlayerReplicationInfo PRI;
//
//	foreach LocalPlayerControllers(P)
//		PC = P;
//	PRI = PC.PlayerReplicationInfo;
//
//	return CurrentWaypoint[PRI.Team.TeamIndex * WPTEAM_MAX + Index];
//}

// Mission ���� ���� , (�ı�/����/����) //
simulated function EMissionCategoryType GetMissionCategory()
{
	switch( MissionType )
	{
	case MISSION_Bombing:			return MISSIONCATEGORY_DESTRUCTION;
	case MISSION_KOTH:
	case MISSION_KOTH_EX:			return MISSIONCATEGORY_CONQUER;
	case MISSION_Transport:			return MISSIONCATEGORY_TRANSPORT;
	case MISSION_Annihilation:		return MISSIONCATEGORY_ANNIHILATION;
	case MISSION_TotalWar:			return MISSIONCATEGORY_TOTALWAR;
	}
}

simulated function bool IsAttackTeam(PlayerReplicationInfo PRI)
{
	if ( AttackTeam == TEAM_Unknown )	return true;
	return (AttackTeam == PRI.Team.TeamIndex);
}

simulated function bool	IsWinnerTeam(PlayerReplicationInfo PRI)
{
	local avaGame	Game;
	Game = avaGame( WorldInfo.Game );
	if ( PRI.Team != None && ( PRI.Team.TeamIndex == 0 || PRI.Team.TeamIndex == 1 ) )
	{	
		if ( PRI.Team.TeamIndex == Game.nWinTeam )	return true;
		else										return false;
	}
	return true;
}

// Round �� ������. �ϴ� ��������...
function EndRound()
{
	local int						i;
	local avaPlayerReplicationInfo	avaPRI;
	local int						RoundMaxKillCnt;
	local int						RoundMaxHeadShotKillCnt;
	local int						RoundMaxScore;
	// ������ Max �� ã��...
	for (i = 0 ; i < PRIArray.Length ; i++ )
	{
		avaPRI = avaPlayerReplicationInfo( PRIArray[i] );
		if ( avaPRI.RoundKillCnt > RoundMaxKillCnt )					RoundMaxKillCnt = avaPRI.RoundKillCnt;
		if ( avaPRI.RoundHeadShotKillCnt > RoundMaxHeadShotKillCnt )	RoundMaxHeadShotKillCnt = avaPRI.RoundHeadShotKillCnt;
		if ( avaPRI.RoundScore > RoundMaxScore )						RoundMaxScore = avaPRI.RoundScore;
	}

	for (i = 0 ; i < PRIArray.Length ; i++ )
    {
		avaPRI = avaPlayerReplicationInfo( PRIArray[i] );

		if ( avaPRI.RoundKillCnt == RoundMaxKillCnt && RoundMaxKillCnt > 0 )							++avaPRI.RoundTopKillCnt;
		if ( avaPRI.RoundHeadShotKillCnt == RoundMaxHeadShotKillCnt && RoundMaxHeadShotKillCnt > 0 )	++avaPRI.RoundTopHeadShotKillCnt;
		if ( avaPRI.RoundScore == RoundMaxScore && RoundMaxScore > 0 )									++avaPRI.RoundTopScoreCnt;
		if ( avaPRI.RoundDamage == 0 )																	++avaPRI.RoundNoDamageCnt; 
		if ( avaPRI.RoundDeathCnt == 0 )																++avaPRI.RoundNoDeathCnt;
		// �ʱ�ȭ....
		avaPRI.RoundKillCnt				=	0;
		avaPRI.RoundHeadShotKillCnt		=	0;
		avaPRI.RoundDamage				=	0;	
		avaPRI.RoundScore				=	0;
		avaPRI.RoundDeathCnt			=	0;
    }
}

function int GetPlayerRank( avaPlayerReplicationInfo avaPRI )
{
	local avaPlayerController	PC;
	local PlayerReplicationInfo	OtherPRI;
	local int					Rank;
	
	Rank = 0;
	foreach WorldInfo.AllControllers(class'avaPlayerController', PC )
	{
		OtherPRI = PC.PlayerReplicationInfo;
		if ( OtherPRI == avaPRI )
			continue;
		if ( OtherPRI.Score > avaPRI.Score )											++Rank;
		else if ( OtherPRI.Score == avaPRI.Score && OtherPRI.Deaths < avaPRI.Deaths )	++Rank;
	}
	return Rank + 1;
}

function int GetRoundWinScore( avaPlayerReplicationInfo avaPRI )
{
	local int Rank;
	if ( WorldInfo.Game.bTeamGame )
		return avaPRI.RoundWinScore;
	else
	{
		Rank = GetPlayerRank( avaPRI );
		if ( Rank == 1 )
			return 1;
	}
	return 0;
}

function int GetRoundLoseScore( avaPlayerReplicationInfo avaPRI )
{
	local int Rank;
	if ( WorldInfo.Game.bTeamGame )
		return avaPRI.RoundLoseScore;
	else
	{
		Rank = GetPlayerRank( avaPRI );
		if ( Rank != 1 )
			return 1;
	}
	return 0;
}

function bool IsWinner( avaPlayerReplicationInfo avaPRI )
{
	return WorldInfo.Game.bTeamGame ? IsWinnerTeam( avaPRI ) : ( GetPlayerRank( avaPRI ) == 1 );
}

function avaPlayerScoreInfo GeneratePlayerResult( avaPlayerReplicationInfo avaPRI )
{	
	local avaPlayerScoreInfo	psi;
	local int					i;

	// ���������� �����ִ� Player���� PlayTime �� ������ ��� �Ѵ�....
	avaPRI.SpawnEnd();
	
	psi.idAccount		=	avaPRI.AccountID;	
	psi.GameWin			=	int( IsWinner( avaPRI ) );
	psi.RoundWin		=	GetRoundWinScore( avaPRI );
	psi.RoundDefeat		=	GetRoundLoseScore( avaPRI );
	psi.DeathCount		=	avaPRI.Deaths;
	psi.Score.Attacker	=	avaPRI.GetPoint( PointType_Attack );
	psi.Score.Defender	=	avaPRI.GetPoint( PointType_Defence );
	psi.Score.Leader	=	avaPRI.GetPoint( PointType_Leader );
	psi.Score.Tactic	=	avaPRI.GetPoint( PointType_Tactics );
	psi.TeamKillCount	=	avaPRI.TeamKillCnt;
	psi.CurrentClass	=	avaPRI.PlayerClassID;

	// �߰� 
	psi.RoundTopKillCount			= avaPRI.RoundTopKillCnt;
	psi.RoundTopHeadshotKillCount	= avaPRI.RoundTopHeadShotKillCnt;
	psi.TopLevelKillCount			= avaPRI.TopLevelKillCnt;
	psi.HigherLevelKillCount		= avaPRI.HigherLevelKillCnt;
	psi.BulletMultiKillCount		= avaPRI.BulletMultiKillCnt;
	psi.GrenadeMultiKillCount		= avaPRI.GrenadeMultiKillCnt;
	psi.NoDamageWinCount			= avaPRI.RoundNoDamageCnt;
	psi.TopScoreCount				= avaPRI.RoundTopScoreCnt;
	psi.NoDeathRoundCount			= avaPRI.RoundNoDeathCnt;
	psi.HelmetDropCount				= avaPRI.HelmetDropCnt;

	for ( i = 0 ; i < `MAX_PLAYER_CLASS ; ++ i )
	{
		psi.WeaponFireCount[0]						+=	avaPRI.avaCRI[i].FireCount[WEAPON_PISTOL];
		psi.WeaponHitCount[0]						+=	avaPRI.avaCRI[i].HitCount[WEAPON_PISTOL];
		psi.WeaponHeadshotCount[0]					+=	avaPRI.avaCRI[i].HeadshotKillCount[WEAPON_PISTOL];

		psi.WeaponFireCount[1]						+=	avaPRI.avaCRI[i].FireCount[WEAPON_SMG];
		psi.WeaponHitCount[1]						+=	avaPRI.avaCRI[i].HitCount[WEAPON_SMG];
		psi.WeaponHeadshotCount[1]					+=	avaPRI.avaCRI[i].HeadshotKillCount[WEAPON_SMG];

		psi.WeaponFireCount[2]						+=	avaPRI.avaCRI[i].FireCount[WEAPON_RIFLE];
		psi.WeaponHitCount[2]						+=	avaPRI.avaCRI[i].HitCount[WEAPON_RIFLE];
		psi.WeaponHeadshotCount[2]					+=	avaPRI.avaCRI[i].HeadshotKillCount[WEAPON_RIFLE];

		psi.WeaponFireCount[3]						+=	avaPRI.avaCRI[i].FireCount[WEAPON_SNIPER];
		psi.WeaponHitCount[3]						+=	avaPRI.avaCRI[i].HitCount[WEAPON_SNIPER];
		psi.WeaponHeadshotCount[3]					+=	avaPRI.avaCRI[i].HeadshotKillCount[WEAPON_SNIPER];

		switch ( i )
		{
		case 0:
			psi.ClassScoreInfo[i].WeaponDamage[3]		=	avaPRI.avaCRI[i].WeaponDamage[WEAPON_SMG] + avaPRI.avaCRI[i].WeaponDamage[WEAPON_SHOTGUN];			// Primary
			psi.ClassScoreInfo[i].WeaponKillCount[3]	=	avaPRI.avaCRI[i].WeaponKillCount[WEAPON_SMG] + avaPRI.avaCRI[i].WeaponKillCount[WEAPON_SHOTGUN];	// Primary	
			psi.ClassScoreInfo[i].HeadshotCount			=	avaPRI.avaCRI[i].HeadShotHitCount[WEAPON_SMG] + avaPRI.avaCRI[i].HeadShotHitCount[WEAPON_SHOTGUN];	// HeadShot ���� Hit�� Count
			psi.ClassScoreInfo[i].HeadshotKillCount		=	avaPRI.avaCRI[i].HeadShotKillCount[WEAPON_SMG] + avaPRI.avaCRI[i].HeadShotKillCount[WEAPON_SHOTGUN];				// HeadShot ���� ���� Count
			break;
		case 1:
			psi.ClassScoreInfo[i].WeaponDamage[3]		=	avaPRI.avaCRI[i].WeaponDamage[WEAPON_RIFLE];		// Primary
			psi.ClassScoreInfo[i].WeaponKillCount[3]	=	avaPRI.avaCRI[i].WeaponKillCount[WEAPON_RIFLE];		// Primary	
			psi.ClassScoreInfo[i].HeadshotCount			=	avaPRI.avaCRI[i].HeadShotHitCount[WEAPON_RIFLE];	// HeadShot ���� Hit�� Count
			psi.ClassScoreInfo[i].HeadshotKillCount		=	avaPRI.avaCRI[i].HeadShotKillCount[WEAPON_RIFLE];				// HeadShot ���� ���� Count
			break;
		case 2:
			psi.ClassScoreInfo[i].WeaponDamage[3]		=	avaPRI.avaCRI[i].WeaponDamage[WEAPON_SNIPER];		// Primary
			psi.ClassScoreInfo[i].WeaponKillCount[3]	=	avaPRI.avaCRI[i].WeaponKillCount[WEAPON_SNIPER];	// Primary	
			psi.ClassScoreInfo[i].HeadshotCount			=	avaPRI.avaCRI[i].HeadShotHitCount[WEAPON_SNIPER];	// HeadShot ���� Hit�� Count
			psi.ClassScoreInfo[i].HeadshotKillCount		=	avaPRI.avaCRI[i].HeadShotKillCount[WEAPON_SNIPER];				// HeadShot ���� ���� Count
			break;
		}

		psi.ClassScoreInfo[i].PlayTime				=	avaPRI.avaCRI[i].PlayTime;							// Play �� �ð�
		psi.ClassScoreInfo[i].PlayRound				=	avaPRI.avaCRI[i].SpawnCount;						// Play �� Round��
		psi.ClassScoreInfo[i].SprintTime			=	avaPRI.avaCRI[i].SprintTime;						// Sprint Time
		psi.ClassScoreInfo[i].KillCount				=	avaPRI.avaCRI[i].KillCount;							// Kill

		psi.ClassScoreInfo[i].TakenDamage			=	avaPRI.avaCRI[i].TakenDamage;						// ���� Damage��
		psi.ClassScoreInfo[i].WeaponDamage[0]		=	avaPRI.avaCRI[i].WeaponDamage[WEAPON_KNIFE];		// Knife
		psi.ClassScoreInfo[i].WeaponDamage[1]		=	avaPRI.avaCRI[i].WeaponDamage[WEAPON_PISTOL];		// Pistol
		psi.ClassScoreInfo[i].WeaponDamage[2]		=	avaPRI.avaCRI[i].WeaponDamage[WEAPON_GRENADE];		// Grenade
		psi.ClassScoreInfo[i].WeaponKillCount[0]	=	avaPRI.avaCRI[i].WeaponKillCount[WEAPON_KNIFE];		// Knife			
		psi.ClassScoreInfo[i].WeaponKillCount[1]	=	avaPRI.avaCRI[i].WeaponKillCount[WEAPON_PISTOL];	// Pistol
		psi.ClassScoreInfo[i].WeaponKillCount[2]	=	avaPRI.avaCRI[i].WeaponKillCount[WEAPON_GRENADE];	// Grenade
	}
	return psi;
}

function GenerateResult( out array<avaPlayerScoreInfo> psiArray )
{
	local int						i;
	local avaPlayerScoreInfo		psi;

	for ( i = 0 ; i < PRIArray.length ; ++ i )
	{
		psi = GeneratePlayerResult( avaPlayerReplicationInfo( PRIArray[i] ) );
		psiArray[ psiArray.length ] = psi;
	}
}

function GetPlayerResult( int AccountID )
{
	local avaNetHandler				NetHandler;
	local avaPlayerScoreInfo		psi;
	local avaPlayerReplicationInfo	pri;

	pri = avaGame( WorldInfo.Game ).GetPRIFromAccount( AccountID );
	if ( pri != None )
	{
		psi = GeneratePlayerResult( pri );
		NetHandler = class'avaNetHandler'.static.GetAvaNetHandler();
		NetHandler.UpdatePlayerScore( psi );
	}
}

function PlayerOut( Controller outPlayer )
{
	local avaNetHandler				NetHandler;
	local avaPlayerScoreInfo		psi;

	if ( bReportEndGame == true )	return;

	psi = GeneratePlayerResult( avaPlayerReplicationInfo( outPlayer.PlayerReplicationInfo ) );
	NetHandler = class'avaNetHandler'.static.GetAvaNetHandler();
	NetHandler.UpdatePlayerScore( psi );
}

function HostReportEndGameToServer()
{
	local PlayerController			PC;
	foreach WorldInfo.AllControllers(class'PlayerController', PC)
	{
		avaPlayerController( PC ).ReportEndGameToServer();
	}
	ReportEndGameToServer();
}

simulated function ReportEndGameToServer()
{
	local array<avaPlayerScoreInfo>	psiArray;
	local avaNetHandler				NetHandler;

	if ( bReportEndGameToServer == true )
		return;

	GenerateResult( psiArray );
	NetHandler = class'avaNetHandler'.static.GetAvaNetHandler();
	NetHandler.ReportGameResult( psiArray, Teams[0].Score, Teams[1].Score );

	bReportEndGameToServer = true;
}

simulated function EndGame()
{
	local array<avaPlayerScoreInfo>	psiArray;
	local avaNetHandler				NetHandler;
	local PlayerController			PC;
	local float						AvgPing;
	local int						PlayerCnt;

	psiArray.length = 0;

	// �̹� EndGame �� ȣ���ߴ�..
	if ( bReportEndGame == true )
		return;

	if ( Role == ROLE_Authority )
	{
		foreach WorldInfo.AllControllers(class'PlayerController', PC)
		{
			AvgPing += PC.PlayerReplicationInfo.Ping;
			++PlayerCnt;
			avaPlayerController(PC).ClientEndGame();
		}

		if ( PlayerCnt > 0 )	AvgPing /= PlayerCnt;
	}
	else
	{
		AvgPing = -1;
	}

	NetHandler = class'avaNetHandler'.static.GetAvaNetHandler();
	
	ReportEndGameToServer();

	NetHandler.EndGame( psiArray, AvgPing );

	bReportEndGame = true;

	ReportGameLog();

	Super.EndGame();
}

// Game Stats �� Server �� �����ֵ��� �Ѵ�!!!
// Reliable ���� �ʾƵ� �Ǵ� Data ���̴�.
function ReportGameLog()
{
	local avaGameStats				GameStat;
	local avaNetHandler				NetHandler;
	local avaStatLog				statLog;
	local int						i,j,k,l,m;
	local int						nTeam;
	local int						nID;
	local RoundStat					roundStat;
	local SpawnStat					spawnStat;
	local WeaponStat				weaponStat;

	NetHandler	=	class'avaNetHandler'.static.GetAvaNetHandler();
	GameStat	=	avaGame( WorldInfo.Game ).GameStats;

	statLog.RoundPlayLogs.length =	GameStat.GameRoundStats.length;
	for ( i = 0 ; i < statLog.RoundPlayLogs.length ; ++ i )
	{
		statLog.RoundPlayLogs[i].Winner		=	GameStat.GameRoundStats[i].Winner;
		statLog.RoundPlayLogs[i].WinType	=	GameStat.GameRoundStats[i].WinType;
		statLog.RoundPlayLogs[i].StartTime	=	GameStat.GameRoundStats[i].StartTime;
		statLog.RoundPlayLogs[i].RoundTime	=	GameStat.GameRoundStats[i].EndTime - GameStat.GameRoundStats[i].StartTime;
		statLog.RoundPlayLogs[i].PlayerCount[0]	= GameStat.GameRoundStats[i].PlayerCnt[0];
		statLog.RoundPlayLogs[i].PlayerCount[1]	= GameStat.GameRoundStats[i].PlayerCnt[1];
	}

	for ( i = 0 ; i < GameStat.PlayerStats.length ; ++ i )
	{
		nTeam = GameStat.PlayerStats[i].LastTeam;
		statLog.GameScoreLogs[nTeam].KillCount			+=	GameStat.PlayerStats[i].GameFireStats.Kill;
		statLog.GameScoreLogs[nTeam].SuicideCount		+=	GameStat.PlayerStats[i].GameDeathStats.Suicide;
		statLog.GameScoreLogs[nTeam].HeadShotKillCount	+=	GameStat.PlayerStats[i].GameFireStats.HeadShot;
		statLog.GameScoreLogs[nTeam].Score.Attacker		+=	GameStat.PlayerStats[i].AttackPoint;
		statLog.GameScoreLogs[nTeam].Score.Defender		+=	GameStat.PlayerStats[i].DefencePoint;
		statLog.GameScoreLogs[nTeam].Score.Leader		+=	GameStat.PlayerStats[i].LeaderPoint;
		statLog.GameScoreLogs[nTeam].Score.Tactic		+=	GameStat.PlayerStats[i].TacticsPoint;
		statLog.GameScoreLogs[nTeam].FriendlyFireCount	+=	GameStat.PlayerStats[i].GameFireStats.FriendlyHit;
		statLog.GameScoreLogs[nTeam].FriendlyKillCount	+=	GameStat.PlayerStats[i].GameFireStats.TeamKill;
		statLog.GameScoreLogs[nTeam].SpawnCount[0]		+=	GameStat.PlayerStats[i].RolePointManCnt;
		statLog.GameScoreLogs[nTeam].SpawnCount[1]		+=	GameStat.PlayerStats[i].RoleRifleManCnt;
		statLog.GameScoreLogs[nTeam].SpawnCount[2]		+=	GameStat.PlayerStats[i].RoleSniperCnt;

		for ( j = 0 ; j < GameStat.PlayerStats[i].RoundStats.length ; ++ j )
		{
			roundStat = GameStat.PlayerStats[i].RoundStats[j];
			for ( k = 0 ; k < roundStat.SpawnStats.length ; ++ k )
			{
				spawnStat = roundStat.SpawnStats[k];
				for ( l = 0 ; l < spawnStat.WeaponStats.length ; ++ l )
				{
					weaponStat = spawnStat.WeaponStats[l];
					for ( m = 0 ; m < statLog.RoundPlayLogs[j].WeaponLogs.length ; ++ m )
					{
						if ( statLog.RoundPlayLogs[j].WeaponLogs[m].Weapon == weaponStat.WeaponType )
							break;
					}

					if ( m == statLog.RoundPlayLogs[j].WeaponLogs.length )
						statLog.RoundPlayLogs[j].WeaponLogs.length = statLog.RoundPlayLogs[j].WeaponLogs.length + 1;

					statLog.RoundPlayLogs[j].WeaponLogs[m].Weapon				=	weaponStat.WeaponType;
					statLog.RoundPlayLogs[j].WeaponLogs[m].UsedCount			+=	weaponStat.nPickUpCnt;
					statLog.RoundPlayLogs[j].WeaponLogs[m].FireCount			+=	weaponStat.FireStats.Fire;
					statLog.RoundPlayLogs[j].WeaponLogs[m].HitCount_Head		+=	weaponStat.FireStats.ShotInfoCnt[SI_Head];
					statLog.RoundPlayLogs[j].WeaponLogs[m].HitCount_Body		+=	weaponStat.FireStats.ShotInfoCnt[SI_Chest];
					statLog.RoundPlayLogs[j].WeaponLogs[m].HitCount_Stomach		+=	weaponStat.FireStats.ShotInfoCnt[SI_Stomach];
					statLog.RoundPlayLogs[j].WeaponLogs[m].HitCount_LeftArm		+=	weaponStat.FireStats.ShotInfoCnt[SI_LeftArm];
					statLog.RoundPlayLogs[j].WeaponLogs[m].HitCount_RightArm	+=	weaponStat.FireStats.ShotInfoCnt[SI_RightArm];
					statLog.RoundPlayLogs[j].WeaponLogs[m].HitCount_LeftLeg		+=	weaponStat.FireStats.ShotInfoCnt[SI_LeftLeg];
					statLog.RoundPlayLogs[j].WeaponLogs[m].HitCount_RightLeg	+=	weaponStat.FireStats.ShotInfoCnt[SI_RightLeg];
					statLog.RoundPlayLogs[j].WeaponLogs[m].HitDistance			+=	weaponStat.FireStats.TotalDistance;
					statLog.RoundPlayLogs[j].WeaponLogs[m].HitDamage			+=	weaponStat.FireStats.Damage;
					statLog.RoundPlayLogs[j].WeaponLogs[m].KillCount[0]			+=	weaponStat.FireStats.ClassKill[0];
					statLog.RoundPlayLogs[j].WeaponLogs[m].KillCount[1]			+=	weaponStat.FireStats.ClassKill[1];
					statLog.RoundPlayLogs[j].WeaponLogs[m].KillCount[2]			+=	weaponStat.FireStats.ClassKill[2];
					statLog.RoundPlayLogs[j].WeaponLogs[m].HeadshotKillCount	+=	weaponStat.FireStats.HeadShot;
					statLog.RoundPlayLogs[j].WeaponLogs[m].MultiKillCount		+=	weaponStat.FireStats.MultiKill + weaponStat.FireStats.DoubleKill;
				}
			}
		}

		for ( j = 0 ; j < GameStat.PlayerStats[i].HitStats.length ; ++ j )
		{
			if ( GameStat.PlayerStats[i].HitStats[j].bKill )
			{
				nID = statLog.KillLogs.length;
				statLog.KillLogs.length = nID + 1;
				statLog.KillLogs[nID].KillTime			=	GameStat.PlayerStats[i].HitStats[j].HitTime;
				statLog.KillLogs[nID].Weapon			=	GameStat.PlayerStats[i].HitStats[j].Weapon;
				statLog.KillLogs[nID].KillerLocation	=	GameStat.PlayerStats[i].HitStats[j].KillerLoc;
				statLog.KillLogs[nID].VictimLocation	=	GameStat.PlayerStats[i].HitStats[j].VictimLoc;
			}
		}
	}
	NetHandler.ReportGameStat( statLog );
}


/**
 * This will determine which LOD to use based off the specific ParticleSystem passed in
 * and the distance to where that PS is being displayed
 **/
simulated function int GetLODLevelToUse(ParticleSystem PS, vector EffectLocation)
{
	local int				Retval;
	local int				LODIdx;
	local PlayerController	PC;
	local Vector			POVLoc;
	local Rotator			POVRot;
	local float				DistanceToEffect, LODDistance;

	// No particle system, ignore
	if( PS == None )
	{
		return 0;
	}

	// On dedicated servers, return highest LOD (lowest quality)
	// (this shouldn't be called on dedicated servers in the first place... but let's be safe).
	if( WorldInfo.NetMode == NM_DedicatedServer )
	{
		return PS.LODDistances.Length - 1;
	}

	// Run this for all local player controllers.
	// If several are found (split screen?). Take the closest for highest LOD.
	ForEach LocalPlayerControllers(PC)
	{
		PC.GetPlayerViewPoint(POVLoc, POVRot);

		DistanceToEffect = VSize(POVLoc - EffectLocation);

		// Take closest
		if( LODDistance == 0.f || DistanceToEffect < LODDistance )
		{
			LODDistance = DistanceToEffect;
		}
	}

	// Find appropriate LOD based on distance
	Retval = PS.LODDistances.Length - 1;
	for( LODIdx = 1; LODIdx < PS.LODDistances.Length; ++LODIdx )
	{
		if( DistanceToEffect < PS.LODDistances[LODIdx] )
		{
			Retval = LODIdx-1;
			break;
		}
	}

	return Retval;
}

/** 2006/12/18 ���½�, SetOptionalSceneData from SeqAct_SWGameInit */
function SetOptionalSceneData( array<SceneStateDataType> NewSceneStateData)
{
	Local int i;
	
	if( NewSceneStateData.Length > MaxOptionalScenes )
		`warn("not enough slot for OptionalSceneData, increase 'MaxOptionalSceneCount' of avaGameReplicationInfo" );

	OptionalSceneCount = 0;
	for( i = 0 ; i <  Min( NewSceneStateData.Length , MaxOptionalScenes ) ; i++ )
	{
		OptionalSceneData[i] = NewSceneStateData[i];
		OptionalSceneCount++;
	}

	bUpdateOptionalScenes = true;
}

simulated function bool GetUpdateOptionalScenesToggle()
{
	Local bool bResult;
	bResult = bUpdateOptionalScenes;
	bUpdateOptionalScenes = false;

	return bResult;
}

function HmSetRoundEnd(bool bSet)
{	
	bHmRoundEnd = bSet;		
	NetUpdateTime = WorldInfo.TimeSeconds - 1;
	
	ScriptTrace();
	`log("[dEAthcURe|HmSetRoundEnd] bHmRoundEnd=" @ bHmRoundEnd);
}

/* [-] 20070426 dEAthcURe|HM �̺�Ʈ �ʱ�ȭ ����
function Reset()
{
	local int lpp;
	
	`log("          <---------- [dEAthcURe|avaGameReplicationInfo::Reset] clearing HmEvents." @ `MAX_HmEvent);
	for(lpp=0;lpp<`MAX_HmEvent;lpp++)
		HmEvents[lpp] = "";	
	
	`log("          <---------- [dEAthcURe|avaGameReplicationInfo::Reset] clearing HmVariables." @ `MAX_HmVariable);
	for(lpp=0;lpp<`MAX_HmVariable;lpp++)
		HmVariables[lpp] = "";
	
	`log("          <---------- [dEAthcURe|avaGameReplicationInfo::Reset] clearing HmIntVars." @ `Max_HmIntVar);
	for(lpp=0;lpp<`Max_HmIntVar;lpp++)
		HmIntVars[lpp] = "";
}
*/

// Host �� Kick Vote �� ��û �޾���... ������, ���
reliable server function RequestKickVote( avaPlayerController proposer, avaPlayerReplicationInfo target, int Subject )
{
	if ( Vote.StartVote( VOTESUBJECT_KickPlayer, proposer, target, Subject ) )
	{
		proposer.ResponseKickVote();
	}
}

simulated function VoteProgressDone()
{
	bVoteProgressMsg = false;
}

simulated function UpdateVoteProgressMsg( avaPlayerReplicationInfo voter, bool bResult )
{
	bVoteProgressMsg = true;
	ClearTimer( 'VoteProgressDone' );
	SetTimer( 3.0, false, 'VoteProgressDone' );
	if ( bResult )	VoteProgressMsg = class'avaStringHelper'.static.Replace( voter.PlayerName, "%s", class'avaVoteMessage'.default.VoteAccept );
	else			VoteProgressMsg = class'avaStringHelper'.static.Replace( voter.PlayerName, "%s", class'avaVoteMessage'.default.VoteDeny );
}

simulated function StartVote( int Subject, avaPlayerReplicationInfo proposer, avaPlayerReplicationInfo targetPRI, float TimeLeft, int Param, bool bTarget )
{
	local string Msg;
	ClearTimer( 'VoteResultDone' );
	bVoteResultMsg	= true;
	bVoting			= true;
	VoteLeftTime	= int( TimeLeft );
	VoteTargetName	= targetPRI.PlayerName;
	VotingMsg		= class'avaVoteMessage'.default.VotingMsg;
	Msg				= class'avaStringHelper'.static.Replace( proposer.PlayerName, "%s", class'avaVoteMessage'.default.VoteKickTitle );
	Msg				= class'avaStringHelper'.static.Replace( targetPRI.PlayerName, "%s", Msg );
	Msg				= class'avaStringHelper'.static.Replace( class'avaVoteMessage'.default.VoteKickReason[Param], "%s", Msg );
	VoteTitleMsg	= Msg;
	VoteWarn		= class'avaVoteMessage'.default.VoteWarn;	
	AmITarget		= bTarget;
	UpdateVoteCount();
	UpdateVoteLeftTime();
}

simulated function EndVote( bool bResult )
{
	bVoting			=	false;
	AmIVote			=	false;
	VoteDenyCnt		=	0;
	VoteAcceptCnt	=	0;
	if ( bResult )	VoteResultMsg = class'avaStringHelper'.static.Replace( VoteTargetName, "%s", class'avaVoteMessage'.default.VoteKickAccepted );
	else			VoteResultMsg = class'avaStringHelper'.static.Replace( VoteTargetName, "%s", class'avaVoteMessage'.default.VoteKickDenied );
	SetTimer( 3.0, false, 'VoteResultDone' );
}

simulated function VoteAccept( avaPlayerReplicationInfo voter, bool bResult, bool bLocal )
{
	if ( bLocal == true )	AmIVote	=	bLocal;
	if ( bResult == true )	++VoteAcceptCnt;
	else					++VoteDenyCnt;
	UpdateVoteCount();
	UpdateVoteProgressMsg( voter, bResult );
}

simulated function UpdateVoteCount()
{
	local string Msg;
	Msg = class'avaStringHelper'.static.Replace( VoteAcceptCnt, "%n", class'avaVoteMessage'.default.VoteCount );
	Msg = class'avaStringHelper'.static.Replace( VoteDenyCnt, "%n", Msg );
	VoteResultMsg = Msg;
}

simulated function UpdateVoteLeftTime()
{
	if ( VoteLeftTime > 0 )		VoteTimerMsg	=	class'avaStringHelper'.static.Replace( VoteLeftTime, "%n", class'avaVoteMessage'.default.VoteLeftTime );
	else						VoteTimerMsg	=	"";
}

simulated function VoteResultDone()
{
	bVoteResultMsg = false;
}

// PC �� Team �� nTeam �� ���ٸ� myTeam �� �ƴ϶�� enemyTeam �� Return �Ѵ�...
simulated function class< avaLocalizedTeamPack > GetLocalizedTeamPack( avaPlayerController PC, int nTeam )
{
	// @avaDeathMatch : Death Match ���� ������ NRF ��Ҹ��� �����µ� ��������????
	if ( PC.PlayerReplicationInfo.bOnlySpectator )
	{
		if ( nTeam >= 0 && nTeam <= 1 )	return	LocalizedTeamPack[nTeam];
		else							return	LocalizedTeamPack[1];
	}
	else
	{
		if ( PC.IsSameTeamByIndex( nTeam ) )	return LocalizedTeamPack[0];
		else									return LocalizedTeamPack[1];
	}
}

simulated function LoadLocalizedTeamPack( int nTeam, PlayerController PC )
{
	local string	myTeamLocalizedPack, enemyTeamLocalizedPack;
	//if ( LocalizedTeamPack[0] != None && LocalizedTeamPack[1] != None )	return;
	if ( nTeam == 0 || nTeam == 1 )
	{
		enemyTeamLocalizedPack	=	InternationalTeamPackName[nTeam==0?1:0];
		if ( UseLocalSound )	myTeamLocalizedPack		=	LocalizedTeamPackName[nTeam];
		else					myTeamLocalizedPack		=	InternationalTeamPackName[nTeam];
	}
	else
	{
		if ( UseLocalSound )
		{
			myTeamLocalizedPack		=	LocalizedTeamPackName[0];
			enemyTeamLocalizedPack	=	LocalizedTeamPackName[1];
		}
		else
		{
			myTeamLocalizedPack		=	InternationalTeamPackName[0];
			enemyTeamLocalizedPack	=	InternationalTeamPackName[1];
		}
	}
	LocalizedTeamPack[0] = class<avaLocalizedTeamPack>(DynamicLoadObject(myTeamLocalizedPack,class'class'));
	LocalizedTeamPack[1] = class<avaLocalizedTeamPack>(DynamicLoadObject(enemyTeamLocalizedPack,class'class'));
	if (LocalPlayer(PC.Player).WorkingSet.Length >= 4)
	{
		LocalPlayer(PC.Player).WorkingSet.Remove(0,2);
	}
	LocalPlayer(PC.Player).WorkingSet[LocalPlayer(PC.Player).WorkingSet.Length] = LocalizedTeamPack[0];
	LocalPlayer(PC.Player).WorkingSet[LocalPlayer(PC.Player).WorkingSet.Length] = LocalizedTeamPack[1];
}

defaultproperties
{
	FlagState[0]=FLAG_Home
	FlagState[1]=FLAG_Home
	MissionIndicatorIdx = -1
	MaxOptionalScenes=`MAX_OPTIONAL_SCENES
	bWarmupRound=true
	nWinTeam=-1
	bReportEndGameToServer = false
	MissionHelp[0]=-1
	MissionHelp[1]=-1
	DominanceTeamIdx = -1
	TargetMissionTime	= 0.0
}
