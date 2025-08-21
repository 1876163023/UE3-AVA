// ====================================================================
// (C) 2004, Epic Games
// ====================================================================

class avaGame extends GameInfo
	config(Game)
	abstract
	dependson(avaPlayerModifierInfo)
	native;

`include(avaGame/avaGame.uci)

var bool					bIgnoreCheckGameGoOn; // [+] 20070518 dEAthcURe|HM
var bool					bAllowMPGameSpeed;

var localized string		GameRatingInformationTeen1;
var localized string		GameRatingInformationTeen2;
var localized string		GameRatingInformation1;
var localized string		GameRatingInformation2;
var localized string		SafeGameRecommendation;
var localized string		SafeGameRecommendation_0HR;
var localized string		SafeGameRecommendation_3HR;
var localized string		SafeGameRecommendation_4HR;
var localized string		SafeGameRecommendation_5HR;
var localized string		SafeGameRecommendation_6HR;
var localized string		SafeGameRecommendation_7HR;
var localized string		SafeGameRecommendation_12HR;
var localized string		SafeGameRecommendation_24HR;

var globalconfig	float	EndTimeDelay;

var globalconfig 	bool	bWarmupRound;			// If true, this match will have a warmup round
var					int		WarmupTime;				// How long is the warmup round (In Seconds)

var	bool					bBeautyMode;

var float					SpawnProtectionTime;
var int						DefaultMaxLives;

var int						SpawnAllowTime;					// ���� ���� �� SpawnAllowTime ���Ŀ��� ������ Spectator �� �����ؾ� �Ѵ�...
var int						RemainingTime, ElapsedTime;
var int						CountDown;

var NavigationPoint			LastPlayerStartSpot;    // last place current player looking for start spot started from
var NavigationPoint			LastStartSpot;          // last place any player started from

var float					EndTime;

var actor					EndGameFocus;

var() int                   ResetCountDown;
var() config int            ResetTimeDelay;           // time (seconds) before restarting teams

var array< class<Inventory> >	DefaultInventory;

var bool					bInitKismetSetting;
var int						InitRoundTimeLimit;		// 
var int						RoundTimeLimit;			// ���� Game �� Round Time Limit
var int						CurrentRoundTimeLimit;	// ���� Round Time Limit, 
var bool					bStopCountDown;

var	array< avaSeqEvent_TimeElapse >		TimeElapseEvent;

struct native GameTypePrefix
{
	var string Prefix;
	var string GameType;
};
var array<GameTypePrefix> MapPrefixes; /** Used for loading appropriate game type if non-specified in URL */
var globalconfig array<GameTypePrefix> CustomMapPrefixes; /** Used for loading appropriate game type if non-specified in URL */
var   string			      MapPrefix;				// Prefix characters for names of maps for this game type.

// Stats

var avaGameStats				GameStats;				// Holds the GameStats actor
var globalconfig string			GameStatsClass;			// Type of GameStats actor to spawn

var globalconfig string			EUPawnClassName[`MAX_PLAYER_CLASS];
var globalconfig string			NRFPawnClassName[`MAX_PLAYER_CLASS];
var globalconfig string			PlayerControllerClassName;
var globalconfig string			PlayerReplicationInfoClassName;
var int							WinCondition;			// Server �� ���� �޾ƿ��� �¸�����...
var	int							nWinTeam;

var array<avaSeqEvent_Killed>	SeqEventsKilled;

struct native KillEventData
{
	var Controller	Killer;
	var Controller	Victim;
};

var array<KillEventData>		KillEventQueue;	

enum ETeamType
{
	TEAM_EU,
	TEAM_USSR,
	TEAM_Unknown,
};

// � ���·� ������ �¸��Ͽ��°�?
enum EWinType
{
	WINTYPE_Annihilation,		//	���꿡 ���� �¸�
	WINTYPE_TimeOver,			//	Time Over�� ���� �¸�
	WINTYPE_MissionComplete,	//	�̼Ǽ��࿡ ���� �¸�
	WINTYPE_AllOut,				//	������� ��� �������� ���� �¸�
};

enum EPlayerClassType
{
	PCT_PointMan,
	PCT_RifleMan,
	PCT_Sniper,
};

enum EUseActionType
{
	UAT_None,
	UAT_UseDoor,
	UAT_UsePickUpProvider,
	UAT_UseRepair,
};

cpptext 
{
	#ifdef EnableHostMigration
	virtual void hmSerialize(FArchive& Ar); // 20061102 dEAthcURe|HM
	#endif
}

native function HmClear(bool bHM);
native function FullscreenMovie_PlayMovie( string MovieFilename );
native function FullscreenMovie_StopMovie();

function PreBeginPlay()
{
    InitLogging();
	Super.PreBeginPlay();
}

function PostBeginPlay()
{
	Super.PostBeginPlay();
	GameReplicationInfo.RemainingTime	=	RemainingTime;
	PlayerControllerClass				=	class<PlayerController>(DynamicLoadObject(PlayerControllerClassName,class'class'));
	PlayerReplicationInfoClass			=	class<PlayerReplicationInfo>(DynamicLoadObject(PlayerReplicationInfoClassName,class'class'));
}

function bool CheckRelevance(Actor Other)
{
	if ( super.CheckRelevance(Other) )
	{
		if ( bBeautyMode && !Other.bDeleteMe && Pawn(Other) != None )
			Pawn(Other).Mesh.bCastDynamicShadow = true;
		return true;
	}
	return false;
}

static event class<GameInfo> SetGameType(string MapName, string Options)
{
    local string ThisMapPrefix;
	local int i,pos;
	local class<avaGame> NewGameType;

	`log( "avaGame.SetGameType" @MapName );

	if (left(MapName,13)~= "avaEntryFinal" )
		return class'avaNetEntryGameEx';

	if (left(MapName,10)~= "avaEntryRe" )
		return class'avaNetEntryGameRe';

	if (left(MapName,10)~= "EnvyEntry." )
		return class'avaEntryGame';

	if (left(MapName,9)~= "avaEntry3" )
		return class'avaNetEntryGameEx';

	if (left(MapName,8)~= "avaEntry" )
		return class'avaNetEntryGameEx';

	// strip the UEDPIE_ from the filename, if it exists (meaning this is a Play in Editor game)
	if (Left(MapName, 7) ~= "UEDPIE_")
	{
		MapName = Right(MapName, Len(MapName) - 7);
	}

	// replace self with appropriate gametype if no game specified
	pos = InStr(MapName,"-");
	ThisMapPrefix = left(MapName,pos);
	if ( ThisMapPrefix ~= Default.MapPrefix )
		return Default.class;

	// change game type
	for ( i=0; i<Default.MapPrefixes.Length; i++ )
	{
		if ( Default.MapPrefixes[i].Prefix ~= ThisMapPrefix )
		{
			NewGameType = class<avaGame>(DynamicLoadObject(Default.MapPrefixes[i].GameType,class'Class'));
			if ( NewGameType != None )
			{
				return NewGameType;
			}
		}
	}
	for ( i=0; i<Default.CustomMapPrefixes.Length; i++ )
	{
		if ( Default.CustomMapPrefixes[i].Prefix ~= ThisMapPrefix )
		{
			NewGameType = class<avaGame>(DynamicLoadObject(Default.CustomMapPrefixes[i].GameType,class'Class'));
			if ( NewGameType != None )
			{
				return NewGameType;
			}
		}
	}

	// if no gametype found, use DemoGame
    return class'avaEntryGame';
}

/** Reset() - reset actor to initial state - used when restarting level without reloading. */
function Reset()
{
	Super.Reset();

	//reset timelimit
	RemainingTime = 60 * TimeLimit;
	GameReplicationInfo.RemainingMinute = RemainingTime;
	
	HmClear(bMigratedHost); // [+] 20070531 ��ġ�̵� 20070303 dEAthcURe|HM
	if(bMigratedHost) {
		`log("[dEAthcURe|avaGame::Reset] migrated host returns to normal state");// dEAthcURe|HM		
		bMigratedHost = false; // 20070214 dEAthcURe|HM
	}	
	avaGameReplicationInfo(GameReplicationInfo).HmSetRoundEnd(false); // 20070302 dEAthcURe|HM		
}

function bool AllowGameSpeedChange()
{
	return bAllowMPGameSpeed || (WorldInfo.NetMode == NM_Standalone);
}

//
// Set gameplay speed.
//
function SetGameSpeed( Float T )
{
    local float OldSpeed;

	if ( !AllowGameSpeedChange() )
		WorldInfo.TimeDilation = 1.0;
	else
	{
		OldSpeed = GameSpeed;
		GameSpeed = FMax(T, 0.01);
		WorldInfo.TimeDilation = 1.0 * GameSpeed;
		if ( GameSpeed != OldSpeed )
		{
			Default.GameSpeed = GameSpeed;
			class'GameInfo'.static.StaticSaveConfig();
		}
	}
	SetTimer(WorldInfo.TimeDilation, true);
}

event Tick( float DeltaTime )
{
	local int i;
	Super.Tick( DeltaTime );
	if ( KillEventQueue.length > 0 )
	{
		for ( i = 0 ; i < SeqEventsKilled.length ; ++ i )
		{
			SeqEventsKilled[i].Trigger( KillEventQueue[0].Killer, KillEventQueue[0].Victim );
		}
		KillEventQueue.Remove( 0, 1 );
	}
}

function ActivateKillEvent( Controller Killer, Controller Victim )
{
	local int	index;
	index = KillEventQueue.length;
	KillEventQueue.length = index + 1;
	KillEventQueue[index].Killer = Killer;
	KillEventQueue[index].Victim = Victim;
}

function ScoreKill(Controller Killer, Controller Other)
{
	local PlayerReplicationInfo OtherPRI;
	if ( !IsValidRoundState() )	return;
	OtherPRI = Other.PlayerReplicationInfo;
    if ( OtherPRI != None )
    {
		OtherPRI.NumLives++;
		if ( (MaxLives > 0) && ( OtherPRI.NumLives >= MaxLives ) )
		{
			OtherPRI.bOutOfLives = true;
		}
    }
    Super.ScoreKill(Killer,Other);
}

// Monitor killed messages for fraglimit
function Killed( Controller Killer, Controller KilledPlayer, Pawn KilledPawn, class<DamageType> damageType, optional class<Weapon> weaponBy )
{
	local bool							bEnemyKill;
	local bool							bHeadShotKill;
	local bool							bWallShotKill;
	local avaPlayerReplicationInfo		KillerPRI, KilledPRI;	
	local int							WeaponType;
	local int							nSwitch;

	nSwitch = 0;

	if ( Killer			!= None )		KillerPRI = avaPlayerReplicationInfo( Killer.PlayerReplicationInfo );
	if ( KilledPlayer	!= None )		KilledPRI = avaPlayerReplicationInfo( KilledPlayer.PlayerReplicationInfo );

	bEnemyKill		= ( ((KillerPRI != None) && (KillerPRI != KilledPRI) && (KilledPRI != None)) && (!bTeamGame || (KillerPRI.Team != KilledPRI.Team)) );

	if ( KilledPawn != None )
	{
		bHeadShotKill	= avaPawn(KilledPawn).bDamageHead;
		bWallShotKill	= avaPawn(KilledPawn).bWallShot;
	}
	
	if ( class<avaWeapon>(weaponBy) != None )	WeaponType	=	class<avaWeapon>(weaponBy).default.WeaponType;
	else										WeaponType	=	-1;

	//
	if ( KilledPRI != None && KillerPRI != None && KilledPRI != KillerPRI )
	{
		if ( bEnemyKill )	KillerPRI.AddKillCount( WeaponType, bHeadShotKill, KilledPRI );
		else				KillerPRI.AddTeamKillCount();
	}

	// �׾���.
	KilledPRI.SpawnEnd();
	if ( class<avaWeapon>(weaponBy) != None )
	{
		nSwitch = 1;
		if ( bHeadShotKill == true )
		{
			if ( bWallShotKill == true )	nSwitch = 4;
			else							nSwitch = 2;
		}
		else if ( bWallShotKill == true )	nSwitch = 3;
		BroadcastLocalized(Self, class'avaKilledIconMessage', nSwitch, KillerPRI, KilledPRI, class<avaWeapon>(weaponBy).default.AttachmentClass );
	}
	else
	{
		BroadcastLocalized(Self, class'avaKilledIconMessage', nSwitch, KillerPRI, KilledPRI, damageType);
	}

	//local int	WeaponStatID;
	if ( weaponBy != None )	KillEventEx( KillerPRI,KilledPRI, damageType, weaponBy, bHeadShotKill, avaCharacter(KilledPawn).TypeID );

	// Super �� �θ��� �ʰ� ���⼭ ���� ó���Ѵ�.
	// ���� avaDeathMessage �� ������� �ʴ´�. �ʿ��ϸ� �߰��Ұ�!
	if( KilledPlayer != None )
	{
		if ( bTeamGame && Killer.PlayerReplicationInfo.Team == KilledPlayer.PlayerReplicationInfo.Team && class<avaDamageType>(damageType).default.bForceTeamDamage == true ) 
		{
			ScoreKill(None, KilledPlayer);
		}
		else
		{
			ScoreKill(Killer, KilledPlayer);
		}
	}

	KilledPlayer.PlayerReplicationInfo.AddDeathCount();
	KilledPlayer.PlayerReplicationInfo.NetUpdateTime = FMin(KilledPlayer.PlayerReplicationInfo.NetUpdateTime, WorldInfo.TimeSeconds + 0.3 * FRand());
	BroadcastDeathMessage(Killer, KilledPlayer, damageType);
	DiscardInventory(KilledPawn);
    NotifyKilled(Killer, KilledPlayer, KilledPawn);
}

// Parse options for this game...
event InitGame( string Options, out string ErrorMessage )
{
    local string InOpt;

    Super.InitGame(Options, ErrorMessage);

    SetGameSpeed(GameSpeed);
	
	InOpt = ParseOption( Options, "Character");
	if ( InOpt != "" )
	{
		`log("Character: "$InOpt);
		DefaultPawnClass = class<Pawn>(DynamicLoadObject("avaGame.avaCharacter_"$InOpt, class'Class'));
		if ( DefaultPawnClass == None )
			DefaultPawnClass = Default.DefaultPawnClass;
	}

	if (WorldInfo.NetMode != NM_StandAlone)
	{
		InOpt = ParseOption( Options, "WarmupTime");
		if (InOpt != "")
		{
			WarmupTime   	= float(InOpt);
			bWarmupRound 	= (WarmupTime>0.0);

		}
	}
	else
	{
		bWarmupRound = false;
	}

	InOpt = ParseOption( Options, "Beauty");
	if ( InOpt != "" )
	{
		bBeautyMode = true;
	}

	WinCondition = GetIntOption( Options, "WinCondition", 100 );
	
	// Swap Team Check �� ���ؼ�....
	// WinCondition *= 2;
}

function int CheckWinTeam()
{
	return -1;
}

function EndGameEx(PlayerReplicationInfo Winner, string Reason, int WinType )
{
	avaGameReplicationInfo( GameReplicationInfo ).EndRound();
	// ���� Stat �� EndRound ������ �˷��ش�.
	if ( GameStats != None )
		GameStats.EndRound( Reason, nWinTeam, WinType );
	EndGame( Winner, Reason );
}

/**
 * Returns the default pawn class for the specified controller,
 *
 * @param	C - controller to figure out pawn class for
 *
 * @return	default pawn class
 */
function class<Pawn> GetDefaultPlayerClass(Controller C);

function bool IsPracticeMode()
{
	return ( class'avaNetHandler'.static.GetAvaNetHandler().GetCurrentChannelFlag() == EChannelFlag_Practice );
}

function bool IsValidPlayer( int AccountID )
{
	return class'avaNetHandler'.static.GetAvaNetHandler().IsPlayerInGame( AccountID );
}

function bool ShouldReset(Actor ActorToReset)
{
	return ( Super.ShouldReset(ActorToReset) && !ActorToReset.IsA('TeamInfo') &&
		!ActorToReset.IsA('PlayerReplicationInfo') && !ActorToReset.IsA('avaKActor_Debris') );
}

function int GetTeamIndex( string Options )
{
	local int	AccountID;
	AccountID	=	GetIntOption( Options, "AID", -1 );
	if ( AccountID == -1 )
		return Super.GetTeamIndex( Options );
	return class'avaNetHandler'.static.GetAvaNetHandler().GetPlayerTeamIndex(AccountID);
}

event PlayerController Login
(
    string Portal,
    string Options,
    out string ErrorMessage,
    optional bool bForceNotSpectating = false // 20070305 dEAthcURe|HM
)
{
	local PlayerController NewPlayer;
    local string    InSex;
	local int		i;
	local int		ClassType;
	local avaPlayerReplicationInfo	avaPRI;
	local float		sdr;

	NewPlayer = Super.Login(Portal, Options, ErrorMessage);

	if ( avaPlayerController(NewPlayer) != None )
	{
		InSex = ParseOption(Options, "Sex");
		if ( Left(InSex,1) ~= "F" )
			avaPlayerController(NewPlayer).SetPawnFemale();	// only effective if character not valid
	}

	// ó�� ���۽� Client �� Server �� �ð��� ���� �ʴ� Bug�� �����ϱ� ���ؼ�...
	avaGameReplicationInfo(GameReplicationInfo).RemainingMinute = RemainingTime;

	avaPRI = avaPlayerReplicationInfo( NewPlayer.PlayerReplicationInfo );
	avaPRI.NetUpdateTime = WorldInfo.TimeSeconds - 1; // 20070306 dEAthcURe	
	
	avaPRI.AccountID		=	GetIntOption( Options, "AID", -1 );
	avaPRI.Level			=	GetIntOption( Options, "ChLevel", 0 );
	avaPRI.LeaderScore		=	GetIntOption( Options, "LeaderScore", 0 );
	avaPRI.bSilentLogIn		=	BOOL( GetIntOption( Options, "bSilentLogIn", 0 ) );
	
	if ( avaPRI.bSilentLogIn == false )
	{
		if ( !IsValidPlayer( avaPRI.AccountID ) )
			return None;
	}

	avaPRI.CharacterItem	=	ParseOption( Options, "ChItem" );
	avaPRI.PointManItem		=	ParseOption( Options, "SkillP" );
	avaPRI.RifleManItem		=	ParseOption( Options, "SkillR" );
    avaPRI.SniperItem		=	ParseOption( Options, "SkillS" );
	avaPRI.WeaponPointMan	=	ParseOption( Options, "WeaponP" );
	avaPRI.WeaponRifleMan	=	ParseOption( Options, "WeaponR" );
	avaPRI.WeaponSniperMan	=	ParseOption( Options, "WeaponS" );
	avaPRI.SlotNum			=	class'avaNetHandler'.static.GetAvaNetHandler().GetPlayerRoomSlot(avaPRI.AccountID);
	avaPRI.SlotNum			=	avaPRI.SlotNum % 12;
	if ( avaPRI.SlotNum < 0 )
		avaPRI.SlotNum = 0;
	// modified by alcor
	if ( avaPRI.CharacterItem == "" )	avaPRI.CharacterItem	= class'avaNetHandler'.static.GetAvaNetHandler().GetURLString(avaPRI.AccountID, "ChItem");
	if ( avaPRI.PointManItem == "" )	avaPRI.PointManItem		= class'avaNetHandler'.static.GetAvaNetHandler().GetURLString(avaPRI.AccountID, "SkillP");
	if ( avaPRI.RifleManItem == "" )	avaPRI.RifleManItem		= class'avaNetHandler'.static.GetAvaNetHandler().GetURLString(avaPRI.AccountID, "SkillR");
	if ( avaPRI.SniperItem == "" )		avaPRI.SniperItem		= class'avaNetHandler'.static.GetAvaNetHandler().GetURLString(avaPRI.AccountID, "SkillS");
	if ( avaPRI.WeaponPointMan == "" )	avaPRI.WeaponPointMan	= class'avaNetHandler'.static.GetAvaNetHandler().GetURLString(avaPRI.AccountID, "WeaponP");
	if ( avaPRI.WeaponRifleMan == "" )	avaPRI.WeaponRifleMan	= class'avaNetHandler'.static.GetAvaNetHandler().GetURLString(avaPRI.AccountID, "WeaponR");
	if ( avaPRI.WeaponSniperMan == "" )	avaPRI.WeaponSniperMan	= class'avaNetHandler'.static.GetAvaNetHandler().GetURLString(avaPRI.AccountID, "WeaponS");

	sdr = float(ParseOption( Options, "SDR" ));
	if (sdr > 0.0 && sdr < 1.0)
	{
		avaPRI.PointManItem = avaPRI.PointManItem $ ";100";
		avaPRI.RifleManItem = avaPRI.RifleManItem $ ";100";
		avaPRI.SniperItem = avaPRI.SniperItem $ ";100";
	}

	avaPRI.ClanMarkID		=	class'avaNetHandler'.static.GetAvaNetHandler().GetClanMarkID(avaPRI.AccountID);

	`log("Full URL = " $ Options $ "?ChItem=" $ avaPRI.CharacterItem $
		"?SkillP=" $ avaPRI.PointManItem $ "?SkillR=" $ avaPRI.RifleManItem $ "?SkillS=" $ avaPRI.SniperItem $
		"?WeaponP=" $ avaPRI.WeaponPointMan $ "?WeaponR=" $ avaPRI.WeaponRifleMan $ "?WeaponS=" $ avaPRI.WeaponSniperMan);
	// end -- alcor

    ClassType	= int( ParseOption( Options, "Class" ) );
	if ( ClassType >= 0 && ClassType < `MAX_PLAYER_CLASS )
		avaPRI.SetPlayerClassID( ClassType );

	avaPRI.FetchCharacterModifier();

	for ( i = 0 ; i < 3 ; ++ i )
	{
		avaPRI.FetchCharacterModifier( i );
		avaPRI.FetchWeaponModifier( i );
	}

	avaGameReplicationInfo(GameReplicationInfo).WinCondition = WinCondition;

	if(!bForceNotSpectating) { // 20070305 dEAthcURe|HM
	// ���� �������� �����ð��� ������ Spectator �� �ۿ� ������ �� ������ �Ѵ�...
	if ( SpawnAllowTime > 0 && 
		 !avaGameReplicationInfo(GameReplicationInfo).bReinforcement && 
		 !IsWarmupRound() && 
		 !IsPracticeMode() )
	{
		if ( !JustStarted( SpawnAllowTime ) )
		{
			NewPlayer.GotoState('Spectating');
			NewPlayer.PlayerReplicationInfo.bOutOfLives = true;
			NumSpectators++;
			`log( "============================== SpawnAllowTime Passed =================================" @NewPlayer );
		}
	}
	} // 20070305 dEAthcURe|HM

	NewPlayer.bHmRestoring = false; // [+] 20070301 dEAthcURe
	return NewPlayer;
}

function SetSquadLeader( avaPlayerReplicationInfo PRI );
function ResignSquadLeader( avaPlayerReplicationInfo PRI );
function avaPlayerReplicationInfo HasSquadLeader( int nTeam );
function avaPlayerReplicationInfo GetSquadLeader( int nTeam, optional PlayerReplicationInfo exceptPRI );

// {{ 20061212 dEAthcURe|HM
event PlayerController HmLogin
(
    string Portal,
    string Options,
    vector inLocation,
    rotator inRotation,
    bool bDead,
    out string ErrorMessage
)
{
	local PlayerController NewPlayer;
    local string    InSex;
	local int		i;
	local int		ClassType;
	local avaPlayerReplicationInfo	avaPRI;
	
	NewPlayer = Super.HmLogin(Portal, Options, inLocation, inRotation, bDead, ErrorMessage);

	if ( avaPlayerController(NewPlayer) != None )
	{
		InSex = ParseOption(Options, "Sex");
		if ( Left(InSex,1) ~= "F" )
			avaPlayerController(NewPlayer).SetPawnFemale();	// only effective if character not valid
	}

	// ó�� ���۽� Client �� Server �� �ð��� ���� �ʴ� Bug�� �����Ͻ� ���ؼ�...
	avaGameReplicationInfo(GameReplicationInfo).RemainingMinute = RemainingTime;

	avaPRI = avaPlayerReplicationInfo( NewPlayer.PlayerReplicationInfo );
	
	//avaPRI.bReplicatedByNewHost = TRUE;  // [+] 20070523 dEAthcURe|HM 			
	avaPRI.AccountID		=	GetIntOption( Options, "AID", -1 );
	avaPRI.Level			=	GetIntOption( Options, "ChLevel", 0 );
	avaPRI.CharacterItem	=	ParseOption( Options, "ChItem" );
	avaPRI.PointManItem		=	ParseOption( Options, "SkillP" );
	avaPRI.RifleManItem		=	ParseOption( Options, "SkillR" );
    avaPRI.SniperItem		=	ParseOption( Options, "SkillS" );
	avaPRI.WeaponPointMan	=	ParseOption( Options, "WeaponP" );
	avaPRI.WeaponRifleMan	=	ParseOption( Options, "WeaponR" );
	avaPRI.WeaponSniperMan	=	ParseOption( Options, "WeaponS" );
	avaPRI.SlotNum			=	class'avaNetHandler'.static.GetAvaNetHandler().GetPlayerRoomSlot(avaPRI.AccountID);

	// modified by alcor
	if ( avaPRI.CharacterItem == "" )	avaPRI.CharacterItem	= class'avaNetHandler'.static.GetAvaNetHandler().GetURLString(avaPRI.AccountID, "ChItem");
	if ( avaPRI.PointManItem == "" )	avaPRI.PointManItem		= class'avaNetHandler'.static.GetAvaNetHandler().GetURLString(avaPRI.AccountID, "SkillP");
	if ( avaPRI.RifleManItem == "" )	avaPRI.RifleManItem		= class'avaNetHandler'.static.GetAvaNetHandler().GetURLString(avaPRI.AccountID, "SkillR");
	if ( avaPRI.SniperItem == "" )		avaPRI.SniperItem		= class'avaNetHandler'.static.GetAvaNetHandler().GetURLString(avaPRI.AccountID, "SkillS");
	if ( avaPRI.WeaponPointMan == "" )	avaPRI.WeaponPointMan	= class'avaNetHandler'.static.GetAvaNetHandler().GetURLString(avaPRI.AccountID, "WeaponP");
	if ( avaPRI.WeaponRifleMan == "" )	avaPRI.WeaponRifleMan	= class'avaNetHandler'.static.GetAvaNetHandler().GetURLString(avaPRI.AccountID, "WeaponR");
	if ( avaPRI.WeaponSniperMan == "" )	avaPRI.WeaponSniperMan	= class'avaNetHandler'.static.GetAvaNetHandler().GetURLString(avaPRI.AccountID, "WeaponS");

	`log("Full URL = " $ Options $ "?ChItem=" $ avaPRI.CharacterItem $
		"?SkillP=" $ avaPRI.PointManItem $ "?SkillR=" $ avaPRI.RifleManItem $ "?SkillS=" $ avaPRI.SniperItem $
		"?WeaponP=" $ avaPRI.WeaponPointMan $ "?WeaponR=" $ avaPRI.WeaponRifleMan $ "?WeaponS=" $ avaPRI.WeaponSniperMan);
	// end -- alcor

    ClassType	= int( ParseOption( Options, "Class" ) );
	if ( ClassType >= 0 && ClassType < `MAX_PLAYER_CLASS )
		avaPRI.SetPlayerClassID( ClassType );

	avaPRI.FetchCharacterModifier();

	for ( i = 0 ; i < 3 ; ++ i )
	{
		avaPRI.FetchCharacterModifier( i );
		avaPRI.FetchWeaponModifier( i );
	}
	
	avaGameReplicationInfo(GameReplicationInfo).WinCondition = WinCondition;

	// x)���� �������� �����ð��� ������ Spectator �� �ۿ� ������ �� ������ �Ѵ�...
	// ���� �÷��̾�� spectator�ιۿ� �����Ҽ� ���� �Ѵ�.
	if ( bDead ) // || SpawnAllowTime > 0 && !bReinforcement )
	{
		if ( bDead ) // || !JustStarted( SpawnAllowTime ) )
		{
			if(!avaGameReplicationInfo(GameReplicationInfo).bReinforcement) { // 20070307 �����߰�
				NewPlayer.bHmForceSpectating = true; // [+] 20070306
				NewPlayer.PlayerReplicationInfo.bOutOfLives = true;
			}
			NewPlayer.GotoState('Spectating');			
			NumSpectators++;
			`log( "============================== SpawnAllowTime Passed =================================" @NewPlayer );
		}
	}
	else 
	{
		// ���׾����� ��ٷ� restart�Ѵ�		
		NewPlayer.GotoState('PlayerHmRestarting');
	}
	
	NewPlayer.bHmRestoring = true; // [+] 20070301 dEAthcURe
	return NewPlayer;
}
// }} 20061212 dEAthcURe|HM

//=================================================================================================
// Player �� Logout �� ������ ��� ������ �� �ִ� ���� Check �Ѵ�...
//=================================================================================================
event function bool CheckGameGoOn( Controller except );

// ���� Loading�� �ȳ��� Player�� �ֳ� Check �Ѵ�....
function bool HasUnreadyPlayer()
{
	local int	i;
	for ( i = 0 ; i < class'avaNetHandler'.static.GetAvaNetHandler().RoomStartingPlayerList.length ; ++ i )
	{
		if ( class'avaNetHandler'.static.GetAvaNetHandler().RoomStartingPlayerList[i].LoadingProgress < 100 )
		{
			return true;
		}
	}
	return false;
}

function PlayerReady( int StartPlayerIdx )
{
	if ( !IsWarmUpRound() )	return;
	if ( HasUnreadyPlayer() )
		return;
	// All Load Complete...
	if ( bIgnoreCheckGameGoOn || CheckGameGoOn( None ) == true ) // [!] 20070518 dEAthcURe|HM // if ( CheckGameGoOn( None ) == true )
	{
		StartWarmUpTimer();
	}
	// Start WarmUp....
}

`devexec function EndWarmUp()
{
	if ( !IsWarmUpRound() )	return;
	StartWarmUpTimer();
}

function bool IsWarmupRound()
{
	return avaGameReplicationInfo(GameReplicationInfo).bWarmupRound;
}

function bool IsValidRoundState()
{
	local avaGameReplicationInfo GRI;
	GRI = avaGameReplicationInfo( GameReplicationInfo );
	return ( !GRI.bWarmupRound && !GRI.bReportEndGameToServer );
}

function StartWarmUpTimer()
{
	local avaGameReplicationInfo avaGRI;
	avaGRI = avaGameReplicationInfo(GameReplicationInfo);
	CurrentRoundTimeLimit			= WarmupTime;			
	avaGRI.CurrentRoundTimeLimit	= CurrentRoundTimeLimit;
	RemainingTime					= CurrentRoundTimeLimit;
	avaGRI.RemainingMinute			= RemainingTime;
	avaGRI.RemainingTime			= RemainingTime;
	avaGRI.ElapsedTime				= 0;
	avaGRI.ElapsedSyncTime			= 0;
	ElapsedTime						= 0;
	avaGRI.TimeLimit				= 1;
	TimeLimit						= 1;
	avaGRI.ReadyForWarfare();
}

function avaPlayerReplicationInfo GetPRIFromAccount( int AccountID )
{
	local avaPlayerController	P;
	foreach WorldInfo.AllControllers(class'avaPlayerController', P)
	{
		if ( avaPlayerReplicationInfo (P.PlayerReplicationInfo).AccountID == AccountID )
			return avaPlayerReplicationInfo( P.PlayerReplicationInfo );
	}
	return None;
}

function EndRoundEx( actor focus, string desc, int nWinner = -1, int WinType = WINTYPE_Annihilation )
{
	local Controller				C;
	local avaPlayerReplicationInfo	pri;

	foreach WorldInfo.AllControllers( class'Controller', C )
	{
		pri = avaPlayerReplicationInfo(C.PlayerReplicationInfo);
		pri.SpawnEnd();
	}
	// ���⼭ ��ź ���� ����� ����.
	foreach WorldInfo.AllControllers( class'Controller', C )
	{
		if ( focus == None )	
			focus = C;
		C.RoundHasEnded( focus );
	}
	ResetCountDown = ResetTimeDelay + 1;
	if ( avaGameReplicationInfo( GameReplicationInfo ).bWarmupRound == false )
	{
		avaGameReplicationInfo( GameReplicationInfo ).EndRound();
		// ���� Stat �� EndRound ������ �˷��ش�.
		if ( GameStats != None )
			GameStats.EndRound( desc, nWinner, WinType );
		class'avaNetHandler'.static.GetAvaNetHandler().UpdateGameState( avaGameReplicationInfo(GameReplicationInfo).CurrentRound, EGS_RoundEnd );
	}
	else
	{
		foreach WorldInfo.AllControllers( class'Controller', C )
		{
			avaPlayerReplicationInfo( C.PlayerReplicationInfo ).ClearScore();
		}
		// Warmup round ����
		avaGameReplicationInfo(GameReplicationInfo).SetWarmupRound( false );
	}
	GotoState( 'RoundOver' );
}

/// avaGame �� RoundOver state �� �ִµ�, �츮�� �ҷ��°Ŷ� �� �� �´´�.
/// �������ϵ��� ��.
state RoundOver
{
	function EndRoundEx( actor focus, string desc, int nWinner, int WinType );

	function RestartPlayer( Controller aPlayer );
	function ScoreKill(Controller Killer, Controller Other);

	function ReduceDamage( out int Damage, pawn injured, Controller instigatedBy, class<DamageType> DamageType )
	{
		Damage = 0;
	}

	event BeginState( name PrevState )
	{
		super.BeginState( PrevState );
		GameReplicationInfo.bStopCountDown = true;
		BeginNewRound();
	}

	function bool ChangeTeam(Controller Other, int Num, bool bNewTeam)
	{
		return Global.ChangeTeam(Other, Num, bNewTeam);
	}

	`devexec function ResetLevel()
	{
		GotoState( 'MatchInProgress' );
		global.ResetLevel();
		ResetCountDown = 0;
	}
}

function bool ShouldRespawn(PickupFactory Other)
{
	return true;
}

function RestartGame()
{
	if ( bGameRestarted )
		return;
	if ( EndTime > WorldInfo.TimeSeconds ) // still showing end screen
		return;

	Super.RestartGame();
}

function bool CheckEndGame(PlayerReplicationInfo Winner, string Reason)
{
	local Controller P;
	// all player cameras focus on winner or final scene (picked by gamerules)
	foreach WorldInfo.AllControllers(class'Controller', P)
	{
		P.GameHasEnded();
	}
	return true;
}

function bool AtCapacity(bool bSpectator)
{
	// �������� �ִµ��� �� �޾Ƶ��δ�....!!!
	return false;
}

event PostLogin( playercontroller NewPlayer )
{
	local avaPlayerReplicationInfo	avaPRI; // 20070523 dEAthcURe|HM
	
	Super.PostLogin(NewPlayer);

	// `log player's login.
	if (GameStats!=None)
	{
		GameStats.ConnectEvent(NewPlayer);
	}

	// {{ 20070523 dEAthcURe|HM
	if(bMigratedHost) {
		avaPRI = avaPlayerReplicationInfo( NewPlayer.PlayerReplicationInfo );
		avaPRI.bReplicatedByNewHost = TRUE;  // [+] 20070523 dEAthcURe|HM 
	}
	// }} 20070523 dEAthcURe|HM


	// Host �� ��� LocalizedTeamPack Setting
	if ( NewPlayer.IsLocalPlayerController() )
	{
		avaPlayerController( NewPlayer ).SetTeam( NewPlayer.PlayerReplicationInfo.Team != None ? NewPlayer.PlayerReplicationInfo.Team.TeamIndex : 255 );
	}
}

function RestartPlayer( Controller aPlayer )
{		
	local avaPlayerReplicationInfo	avaPRI;
	local avaPawn					P;

	avaPRI = avaPlayerReplicationInfo( aPlayer.PlayerReplicationInfo );

	// TODO: ���߿��� Team üũ �������?? avaPRI.Team != None �̷���?
	// Demo Play �� ��쿡�� �ٷ� �����Ѵ�.
	if( avaPRI != None && !avaPRI.IsValidClassType() )
	{
		//`log("[dEAthcURe|avaGame::RestartPlayer.1] return");
		return;
	}	

	if ( !IsWarmupRound() )
	{
		if ( aPlayer.PlayerReplicationInfo.bOutOfLives ) 
			return;
	}
	
	//`log("[dEAthcURe|avaGame::RestartPlayer.4] before ClientSetBehindView");

	// RestartPlayer �ϸ� Behindview �� ���ߵȴ�....��....
	avaPlayerController(aPlayer).ClientSetBehindView(false);	
	
	//`log("[dEAthcURe|avaGame::RestartPlayer.5] after ClientSetBehindView");

	ForEach DynamicActors(class'avaPawn', P)
	{
//		`log( "Pawn"@P@P.Controller );
		if (P.Controller == aPlayer)
		{
//			`log( "Found old pawn! destroy it" );
			P.Destroy();
		}
	}
	
	//`log("[dEAthcURe|avaGame::RestartPlayer.5] before super");

    Super.RestartPlayer(aPlayer);
}

function AddDefaultInventory( pawn PlayerPawn )
{
	local int						i,j;
	local avaPlayerReplicationInfo	avaPRI;
	local int						typeID;
	local bool						bDoNotActivate;
	local array< WeaponInfo >		WeaponInfos;
	local int						CreateWeaponCnt;
	local Inventory					Inv;
	local avaWeapon					avaWeap;
	
	// {{ [+] 20080228 dEAthcURe|HM HM�ÿ��� serialize�Ŀ� add
	if(bMigratedHost && PlayerController(PlayerPawn.Controller).bHmRestoring) {
		PlayerController(PlayerPawn.Controller).bHmRestoring = false;
		return;
	}
	// }} [+] 20080228 dEAthcURe|HM HM�ÿ��� serialize�Ŀ� add

	avaPRI = avaPlayerReplicationInfo( PlayerPawn.PlayerReplicationInfo );

	if( avaPRI != None )
	{	
		typeID			=	avaPawn(PlayerPawn).GetTypeID();
		bDoNotActivate	=	bMigratedHost ? true : false; // [!] 20070213 dEAthcURe|HM // bDoNotActivate	=	false;
		bDoNotActivate	=	false; // [+] 20070301 dEAthcURe|HM
		WeaponInfos		=	avaPRI.avaPMI.ClassTypeInfos[typeID].WeaponInfos;
		for ( i = 0 ; i < WeaponInfos.length ; ++ i )
		{
			if ( WeaponInfos[i].Class == None )	continue;
			Inv = Spawn( WeaponInfos[i].Class );
			avaWeap = avaWeapon( Inv );

			for ( j = 0 ; j < WeaponInfos[i].Mod.length ; ++ j )
				avaWeap.AddWeaponModifier( WeaponInfos[i].Mod[j] );

			// Modifier �� �ϳ��̻��̶� �ִٸ� ModifierDone �� �ҷ��ش�... ���µ� �ҷ��� �ʿ䰡 ����...
			avaWeap.WeaponModifierDone(PlayerPawn);
			
			avaWeap.AmmoCount					=	avaWeap.MaxAmmoCount;
			`log("          <---------- avaGame/AddDefaultInventory/avaWeap.AmmoCount " @ avaWeap.AmmoCount);

			if ( avaWeap_BaseGun(avaWeap) != None ) 
				avaWeap_BaseGun(avaWeap).ReloadCnt	=	avaWeap_BaseGun(avaWeap).ClipCnt;
			
			if ( WeaponInfos[i].MaintenanceRate >= 0 )
				avaWeap.SetMaintenanceRate( WeaponInfos[i].MaintenanceRate );

			
			PlayerPawn.InvManager.AddInventory( Inv, bDoNotActivate );
			bDoNotActivate = true;
			++ CreateWeaponCnt;
		}		

		// �д����� ��쿡�� �д��� �������� �����Ѵ�...
		if ( avaPRI.bSquadLeader == true )
		{
			Inv = Spawn( class'avaWeap_Binocular' );
			PlayerPawn.InvManager.AddInventory( Inv, true );
		}

		if ( CreateWeaponCnt == 0 )
			avaPRI.CreateWeaponTemp( avaPawn( PlayerPawn ) );
	}
	else
	{
		for (i=0; i<DefaultInventory.Length; i++)
		{
			// Ensure we don't give duplicate items
			if (PlayerPawn.FindInventoryType( DefaultInventory[i] ) == None)
			{
				// Only activate the first weaponm
				if (i>0)
				{
					PlayerPawn.CreateInventory(DefaultInventory[i], true);
				}
				else
				{
					PlayerPawn.CreateInventory(DefaultInventory[i], false);
				}
			}
		}
	}
	PlayerPawn.AddDefaultInventory();
}

function bool CanSpectate( PlayerController Viewer, PlayerReplicationInfo ViewTarget )
{
    return ( (ViewTarget != None) && ((WorldInfo.NetMode == NM_Standalone) || Viewer.PlayerReplicationInfo.bOnlySpectator) );
}

function ChangeName(Controller Other, string S, bool bNameChange)
{
    if ( S == "" )
		return;

	S = StripColor(s);	// Strip out color codes

    if ( Other.PlayerReplicationInfo.playername~=S )
		return;

	S = Left(S,20);
    ReplaceText(S, " ", "_");

	if( bNameChange )
		GameEvent('NameChange',-1,Other.PlayerReplicationInfo);

    Other.PlayerReplicationInfo.SetPlayerName(S);
}

function Logout(controller Exiting)
{	
	Super.Logout(Exiting);
	if ( GameStats!=None)
		GameStats.DisconnectEvent(Exiting);
	if ( avaGameReplicationInfo(GameReplicationInfo) != None && 
		avaGameReplicationInfo(GameReplicationInfo).Vote != None )
		avaGameReplicationInfo(GameReplicationInfo).Vote.DisconnectEvent(Exiting);
}

function InitGameReplicationInfo()
{
    Super.InitGameReplicationInfo();
	InitSettingFromKismet();
}

// InitSettingFromKistmet
function InitSettingFromKismet()
{
	local Sequence					GameSeq;
	local array<SequenceObject>		AllSeqEvents;
	local array<SequenceObject>		SeqEventsA, SeqEventsB;
	local avaSeqAct_SWGameInit		swgame;
	local int						i;
	local avaGameReplicationInfo	avaGRI;
	
	local avaSeqEvent_TimeElapse	aTimeElapseEvent;
	local avaSeqEvent_Killed		aKilledEvent;

	if ( bInitKismetSetting == true )	return;

	GameSeq	=	WorldInfo.GetGameSequence();
	avaGRI	=	avaGameReplicationInfo( GameReplicationInfo );	

	if ( GameSeq != none )
	{
		GameSeq.FindSeqObjectsByClass( class'avaSeqAct_SWGameInit', true, AllSeqEvents );
		if ( AllSeqEvents.Length > 0 )
		{
			swgame = avaSeqAct_SWGameInit( AllSeqEvents[0] );
			avaGRI.DogTagPackCnt	=	swgame.DogTagPackCnt;
			avaGRI.bReinforcement	=	swgame.bReinforcement;
			avaGRI.MissionType		=	EMissionType( swgame.MissionType );
			avaGRI.AttackTeam		=	ETEAMType( swgame.AttackTeam );
			avaGRI.CurrentRound		=	0;
			avaGRI.BaseScore		=	swgame.BaseScore;
			RoundTimeLimit			=	swgame.RoundTimeLimit;
			InitRoundTimeLimit		=	RoundTimeLimit;
			// Reinforcement frequency
			for ( i = 0 ; i < `MAX_TEAM ; ++ i )
			{
				avaGRI.MissionHelp[i] = swgame.MissionHelpIdx[i];
				ChangeRespawnTime( i, swgame.ReinforcementFreq[i] );
			}

			// GRI �� �Űܾ� �� �͵�...
			DefaultMaxLives			= swgame.MaxLives;
			MaxLives				= DefaultMaxLives;
			avaGRI.SetOptionalSceneData(swgame.SceneStateData);
			bInitKismetSetting = true;
		}

		// Find TimerElapse Event
		GameSeq.FindSeqObjectsByClass( class'avaSeqEvent_TimeElapse', true, SeqEventsA );
		GameSeq.FindSeqObjectsByClass( class'avaSeqEvent_Killed', true, SeqEventsB );

		for (i=0; i<SeqEventsA.Length; ++i)
		{
			aTimeElapseEvent = avaSeqEvent_TimeElapse( SeqEventsA[i] );

			if (aTimeElapseEvent != None)
			{
				TimeElapseEvent[TimeElapseEvent.Length] = aTimeElapseEvent;
			}
		}

		for (i=0; i<SeqEventsB.Length; ++i)
		{
			aKilledEvent  = avaSeqEvent_Killed( SeqEventsB[i] );

			if (aKilledEvent != None)
			{
				SeqEventsKilled[SeqEventsKilled.Length] = aKilledEvent;
			}
		}		
	}
}

function ChangeRespawnTime( int nTeam, int nTime )
{
	avaGameReplicationInfo( GameReplicationInfo ).ReinforcementFreq[nTeam] = nTime;
}

function ChangeRemainingTime( int InRemainingTime )
{
	RemainingTime = InRemainingTime;
	avaGameReplicationInfo( GameReplicationInfo ).RemainingTime		= InRemainingTime;
	avaGameReplicationInfo( GameReplicationInfo ).RemainingMinute	= InRemainingTime;
}

function ReduceDamage( out int Damage, pawn injured, Controller instigatedBy, class<DamageType> DamageType )
{
	if ( IsPracticeMode() || !IsValidRoundState() )
	{
		Damage = 0;
		return;
	}

	if ( (instigatedBy != None) && (InstigatedBy != Injured.Controller) && (WorldInfo.TimeSeconds - injured.SpawnTime < SpawnProtectionTime)
		&& (class<avaDamageType>(DamageType) != None) )
	{
		Damage = 0;
		return;
	}

    Super.ReduceDamage( Damage, injured, InstigatedBy, DamageType );

    if ( instigatedBy == None )
		return;

	if ( InstigatedBy.Pawn != None )
		Damage = Damage * instigatedBy.Pawn.GetDamageScaling();
}

//------------------------------------------------------------------------------
// Game States

function TriggerAttainScore( avaPlayerController ScorePlayer );

function CheckPrivateScore( avaPlayerController ScorePlayer, int nScore );

// {{ 20070515 dEAthcURe|HM avaSWGame���� ������
function TriggerTimeOverEvent()
{
	// seqevent �� ã�Ƽ� ȣ���ϴ� �κ��� GameInfo.ResetLevel ���� ����.
	local Sequence					GameSeq;
	local array<SequenceObject>		AllSeqEvents;
	local int						i;
	local Controller				C;
	local avaPlayerReplicationInfo	pri;

	GameSeq = WorldInfo.GetGameSequence();
	if ( GameSeq != none )
	{
		// find DesiredClass 
		GameSeq.FindSeqObjectsByClass( class'avaSeqEvent_TimeOver', true, AllSeqEvents );

		//	activate them
		for ( i=0; i<AllSeqEvents.Length; ++i )
		{
			avaSeqEvent_TimeOver( AllSeqEvents[i] ).Trigger( WorldInfo, false, avaGameReplicationInfo( GameReplicationInfo ).CurrentRound );
		}
	}

	// Round Time is Over Spawn End
	avaGameReplicationInfo( GameReplicationInfo ).bRoundTimeOver = true;
	foreach WorldInfo.AllControllers( class'Controller', C )
	{
		pri = avaPlayerReplicationInfo(C.PlayerReplicationInfo);
		pri.SpawnEnd();
	}
}

// }} 20070515 dEAthcURe|HM avaSWGame���� ������
function StartMatch()
{	
	local PlayerController	PC;	

	if ( bMigratedHost && bWarmupRound || WorldInfo.NetMode != NM_StandAlone && !bMigratedHost )	
		avaGameReplicationInfo(GameReplicationInfo).SetWarmupRound( true ); 
	else										
		avaGameReplicationInfo(GameReplicationInfo).SetWarmupRound( false );

	GameEvent( 'NewRound', -1, None );

	GotoState('MatchInProgress');

    GameReplicationInfo.RemainingMinute = RemainingTime;
    Super.StartMatch();

	if (GameStats != none)
		GameStats.MatchBeginEvent();

	foreach LocalPlayerControllers(PC)
	{
		avaPlayerReplicationInfo( PC.PlayerReplicationInfo ).bHost = true;
		break;
	}
	
	// {{ 20070504 dEAthcURe|HM HM event�� ���⼭ fire�غ���.
	`log("          <---------- [avaGame::StartMatch] bMigratedHost=" @bMigratedHost @ RemainingTime @ avaGameReplicationInfo(GameReplicationInfo).bHmRoundEnd);
	if(bMigratedHost) {	
		avaGameReplicationInfo(GameReplicationInfo).triggerHmEvent();
		
		if(RemainingTime == 0 && !avaGameReplicationInfo(GameReplicationInfo).bHmRoundEnd) {
			`log("          <---------- [avaGame::StartMatch] TriggerTimeOverEvent");
			TriggerTimeOverEvent();
		}
	}	
	// }} 20070504 dEAthcURe|HM HM event�� ���⼭ fire�غ���.

	//	ó�� ������.
	InitRound();
	BeginRound();

	class'avaNetHandler'.static.GetAvaNetHandler().UpdateGameState( avaGameReplicationInfo(GameReplicationInfo).CurrentRound, EGS_GameBegin );
}

function InitRound()
{
	if ( IsWarmupRound() )
	{
		CurrentRoundTimeLimit = 0.0;
	}
	else
	{
		CurrentRoundTimeLimit = RoundTimeLimit;
	}
}

function BeginRound()
{
	local avaGameReplicationInfo	avaGRI;
	local PlayerController			PC;
	avaGRI = avaGameReplicationInfo(GameReplicationInfo);
	// GameStat �� NewRound �� �˷��ش�.
	if ( avaGRI.CurrentRound > 0 )
		GameEvent( 'NewRound', -1, None );

	// Reset Level�� �ϴ� ���߿� Remaining Time �� ���µǱ� ������,	Reset Level�� �����ϰ�, ���߿� �����ϵ��� �Ѵ�.
	if ( avaGRI.CurrentRound > 0 )
		ResetLevel();

	// ????
	if( !bMigratedHost )
		avaGameReplicationInfo( GameReplicationInfo ).bRoundTimeOver = false;

	// {{ 20070212 20061219 dEAthcURe|HM HM�ÿ��� ����
	if(!bMigratedHost || bMigratedHost && bWarmupRound) { // [+] 20070306 dEAthcURe|HM warmupround
		/// remain time ������ ����

		`log("avaGame.BeginRound performing" @RemainingTime @CurrentRoundTimeLimit ); // 20061219 dEAthcURe|HM
		avaGRI.CurrentRoundTimeLimit	= CurrentRoundTimeLimit;
		RemainingTime					= CurrentRoundTimeLimit;
		avaGRI.RoundWinner				= TEAM_Unknown;
		avaGRI.NetUpdateTime            = WorldInfo.TimeSeconds -1;
		avaGRI.RemainingMinute			= RemainingTime;
		avaGRI.ElapsedTime				= 0;
		avaGRI.ElapsedSyncTime			= 0;
		ElapsedTime						= 0;		

		if ( !avaGRI.bWarmupRound )
			TriggerStartRound( avaGRI.CurrentRound );
		
		if(bMigratedHost && bWarmupRound) { // [+] 20070306 dEAthcURe|HM ���������� migration�Ǿ������� �ٷ� countdown����
			//`log("[dEAthcURe|avaSWGame::BeginRound] StartWarmupTimer <- bMigratedHost && bWarmupRound");
			StartWarmupTimer();		
		}
	}
	else {
		avaGRI.CurrentRoundTimeLimit	= CurrentRoundTimeLimit;
		avaGRI.RoundWinner				= TEAM_Unknown;
		avaGRI.NetUpdateTime            = WorldInfo.TimeSeconds -1;
		avaGRI.RemainingMinute			= RemainingTime;
		avaGRI.ElapsedTime				= CurrentRoundTimeLimit - RemainingTime;
		avaGRI.ElapsedSyncTime			= CurrentRoundTimeLimit - RemainingTime;
		ElapsedTime						= CurrentRoundTimeLimit - RemainingTime;
		`log("[BeginRound performing HM]" @RemainingTime); // 20061219 dEAthcURe|HM
	}
	// }} 20070212 20061219 dEAthcURe|HM HM�ÿ��� ����

	/// ��Ȯ�� timelimit�� Remaining Time�� ���� �����ϹǷ�,
	/// �̰��� �����ϴ� �ǹ̴�, time over ������ �ִٴ� ����.
	if (  avaGRI.CurrentRoundTimeLimit > 0 )
	{
		avaGRI.TimeLimit	= 1;
		TimeLimit           = 1;
	}
	else
	{
		avaGRI.TimeLimit	= 0;
		TimeLimit           = 0;
	}

	/// round reset�ȵǴ� ���� �����߳� �𸣰ڳ�~~~ 2006/2/23, deif/oz spectating.. -_-;
	foreach WorldInfo.AllControllers(class'PlayerController', PC)
	{
		if(PC.bHmForceSpectating && !avaGameReplicationInfo(GameReplicationInfo).bReinforcement) { // [+] 20070306 dEAthcURe|HM		
			`log("[avaGame::BeginRound] Skipping reseting player state");
		}
		else { // [+] 20070306 dEAthcURe|HM
			PC.PlayerReplicationInfo.NumLives = 0;
			PC.PlayerReplicationInfo.bOutOfLives = false;
		}
		avaPlayerController(PC).Client_BeginRound();
	}
	
	// CurrentRound == 0 ������ StartMatch() ���� �� ���� ��.
	if ( avaGRI.CurrentRound > 0 )
		StartHumans();

	SetGameSpeed( GameSpeed );
}

function BeginNewRound()
{
	++avaGameReplicationInfo(GameReplicationInfo).CurrentRound;
	InitRound();
	BeginRound();
}

function TriggerStartRound( int nRound )
{
	local Sequence GameSeq;
	local array<SequenceObject> AllSeqEvents;
	local int i;

	GameSeq = WorldInfo.GetGameSequence();
	if ( GameSeq != none )
	{
		// find DesiredClass 
		GameSeq.FindSeqObjectsByClass( class'avaSeqEvent_StartRound', true, AllSeqEvents );

		//	activate them
		for ( i=0; i<AllSeqEvents.Length; ++i )
		{
			avaSeqEvent_StartRound( AllSeqEvents[i] ).Trigger( WorldInfo, nRound );
		}
	}
}

function EndGame(PlayerReplicationInfo Winner, string Reason )
{
	local Controller				C;
	local avaPlayerReplicationInfo	pri;
	foreach WorldInfo.AllControllers( class'Controller', C )
	{
		pri = avaPlayerReplicationInfo(C.PlayerReplicationInfo);
		pri.SpawnEnd();
	}

	if (GameStats != none && Winner != none )
	{
		GameStats.MatchEndEvent( NAME("MatchEnded_"$Reason),Winner);
	}

	class'avaNetHandler'.static.GetAvaNetHandler().UpdateGameState( avaGameReplicationInfo(GameReplicationInfo).CurrentRound, EGS_GameEnd );

	if (GameStats != none && Winner != none )
	{
		GameStats.MatchEndEvent( NAME("MatchEnded_"$Reason),Winner);
	}

    Super.EndGame(Winner,Reason);
	if ( bGameEnded )
		GotoState('MatchOver');
}

/** FindPlayerStart()
* Return the 'best' player start for this player to start from.  PlayerStarts are rated by RatePlayerStart().
* @param Player is the controller for whom we are choosing a playerstart
* @param InTeam specifies the Player's team (if the player hasn't joined a team yet)
* @param IncomingName specifies the tag of a teleporter to use as the Playerstart
* @returns NavigationPoint chosen as player start (usually a PlayerStart)
 */
function NavigationPoint FindPlayerStart(Controller Player, optional byte InTeam, optional string incomingName)
{
    local NavigationPoint Best;

	// Save LastPlayerStartSpot for use in RatePlayerStart()
    if ( (Player != None) && (Player.StartSpot != None) )
		LastPlayerStartSpot = Player.StartSpot;

    Best = Super.FindPlayerStart(Player, InTeam, incomingName );

	// Save LastStartSpot for use in RatePlayerStart()
    if ( Best != None )
		LastStartSpot = Best;
    return Best;
}

function bool JustStarted(float MaxElapsedTime)
{
	return ElapsedTime < MaxElapsedTime;
}

`devexec function ResetLevel()
{
	local avaPlayerController	PC;
	foreach WorldInfo.AllControllers(class'avaPlayerController', PC)
	{
		PC.Client_ResetLevel();
	}

	if (ResetCountDown > 0)
	{
		Super.ResetLevel();
	}
}

/** ends the current round; sends the game into state RoundOver and sets the ViewTarget for all players to be the given actor */
function EndRound(Actor EndRoundFocus)
{
	local Controller C;

	//round has ended
	if ( !bGameEnded )
	{
		foreach WorldInfo.AllControllers(class'Controller', C)
		{
			C.RoundHasEnded(EndRoundFocus);
		}
	}
	GotoState('RoundOver');
}

function bool MatchIsInProgress()
{
	return false;
}

auto State MatchPending
{
	function RestartPlayer( Controller aPlayer )
	{
		if ( CountDown <= 0 )
			Super.RestartPlayer(aPlayer);
		else if ( IsWarmupRound() )
			global.RestartPlayer(aPlayer);
	}

	// Override these 4 functions so that if we are in a warmup round, they get ignored.

	function CheckLives();
	function bool CheckScore(PlayerReplicationInfo Scorer);
	function ScoreKill(Controller Killer, Controller Other);	
	
    function Timer()
    {
	}

    function BeginState(Name PreviousStateName)
    {
		bWaitingToStartMatch = true;
    }

	function EndState(Name NextStateName)
	{
		//avaGameReplicationInfo(GameReplicationInfo).bWarmupRound = false;
	}

	function EndRoundEx( actor focus, string desc, int nWinner, int WinType );

Begin:
	StartMatch();
}

state MatchInProgress
{
	function bool MatchIsInProgress()
	{
		return true;
	}

	function BeginState(Name PreviousStateName)
	{
		local PlayerReplicationInfo PRI;

		if (PreviousStateName != 'RoundOver')
		{
			foreach DynamicActors(class'PlayerReplicationInfo', PRI)
			{
				PRI.StartTime = 0;
			}
			ElapsedTime = 0;
			bWaitingToStartMatch = false;
		}
	}

	function bool PlayerCanRestartGame( PlayerController aPlayer )
	{
		return false;
	}

	function bool PlayerCanRestart( PlayerController aPlayer )
	{
		if ( GameReplicationInfo.bMatchHasBegun && ResetCountDown == 0 && avaGameReplicationInfo(GameReplicationInfo).bReinforcement )
		{
			return false;
		}
		return true;
	}

	function Timer()
	{
		local avaGameReplicationInfo	avaGRI;
		local int	i;

		avaGRI = avaGameReplicationInfo( GameReplicationInfo );
		//mich: avaGame.MAtchInProgress.Timer() ���� EndGame�� ȣ���ϴ� ���� ���� ����, super.Timer() �� �θ��� �ʴ´�.
		global.Timer();

		if ( TimeLimit > 0 )
		{
			GameReplicationInfo.bStopCountDown = false;
			if ( RemainingTime > 0 && bStopCountDown == false )
			{
				// 10�� ���� �ѹ��� Time�� Replication ���ش�... Time Sync
				GameReplicationInfo.RemainingTime = --RemainingTime;
				if ( RemainingTime % 10 == 0 )
					GameReplicationInfo.RemainingMinute = RemainingTime;

				// Time Over Event �� �ѹ��� �߻��Ѵ�.
				if ( RemainingTime == 0 )
				{
					if ( IsWarmupRound() )
					{
						EndRoundEx( None, "" );
					}
					else
					{
						TriggerTimeOverEvent();
						GameReplicationInfo.bStopCountDown = true;
					}
				}
			}
		}
		else if ( IsWarmupRound() )
		{
			return;
		}

		ElapsedTime++;
		GameReplicationInfo.ElapsedTime = ElapsedTime;

		for ( i = 0 ; i < TimeElapseEvent.length ; ++ i )
		{
			if ( TimeElapseEvent[i].Timer == ElapsedTime )
			{
				TimeElapseEvent[i].Trigger( WorldInfo );
			}
		}

		if ( ElapsedTime % 10 == 0 )
			avaGRI.ElapsedSyncTime = ElapsedTime;

		// ���� ó��.
		//  ResetCountDown - ���� reset ���� �ð�, ���� �߿��� 0 �̾��ٰ�,
		//  �ٽ� objective�� �޼��Ǹ� ���� �ȴ�.
		//  ��, ���� ���� �߿��� ��� ����. RoundOver state�� ������ �� ����.
		if ( avaGRI.bReinforcement )
		{
			Reinforce();
		}

		if ( ResetCountDown > 0 )
		{
			ResetCountDown --;

            if ( ResetCountDown < 3 )
            {
				if ( (ResetCountDown > 0 ) && ( ResetCountDown < 10 ))
				{
					if ( ResetCountDown == 1 )
					{
						GameReplicationInfo.NetUpdateTime = WorldInfo.TimeSeconds - 1;
					}
				}
				else if ( ResetCountDown == 0 )
				{
					BeginNewRound();
				}
			}
		}
	}
}

// ���� ó��...
function Reinforce()
{

}

State MatchOver
{
	function RestartPlayer( Controller aPlayer ) {}
	function ScoreKill(Controller Killer, Controller Other) {}
	function EndRoundEx( actor focus, string desc, int nWinner, int WinType );

	function ReduceDamage( out int Damage, pawn injured, Controller instigatedBy, class<DamageType> DamageType )
	{
		Damage = 0;
	}

	function bool ChangeTeam(Controller Other, int num, bool bNewTeam)
	{
		return false;
	}

	function Timer()
	{
		local PlayerController PC;
		Global.Timer();
		if ( EndGameFocus != None )
		{
			EndGameFocus.bAlwaysRelevant = true;
			foreach WorldInfo.AllControllers(class'PlayerController', PC)
			{
				PC.ClientSetViewtarget(EndGameFocus);
			}
		}
	}

	function BeginState(Name PreviousStateName)
	{
		GameReplicationInfo.bStopCountDown = true;
	}

	`devexec function ResetLevel()
	{
		RestartGame();
	}
}

/** ChoosePlayerStart()
* Return the 'best' player start for this player to start from.  PlayerStarts are rated by RatePlayerStart().
* @param Player is the controller for whom we are choosing a playerstart
* @param InTeam specifies the Player's team (if the player hasn't joined a team yet)
* @returns NavigationPoint chosen as player start (usually a PlayerStart)
 */
function PlayerStart ChoosePlayerStart( Controller Player, optional byte InTeam )
{
	local PlayerStart P, BestStart;
	local float BestRating, NewRating;
	local array<playerstart> PlayerStarts;
	local int i, RandStart;
	local byte Team;

	// use InTeam if player doesn't have a team yet
	Team = ( (Player != None) && (Player.PlayerReplicationInfo != None) && (Player.PlayerReplicationInfo.Team != None) )
			? byte(Player.PlayerReplicationInfo.Team.TeamIndex)
			: InTeam;

	// make array of enabled playerstarts
	foreach WorldInfo.AllNavigationPoints(class'PlayerStart', P)
	{
		if ( P.bEnabled )
			PlayerStarts[PlayerStarts.Length] = P;
	}

	// start at random point to randomize finding "good enough" playerstart
	RandStart = Rand(PlayerStarts.Length);
	for ( i=RandStart; i<PlayerStarts.Length; i++ )
	{
		P = PlayerStarts[i];
		NewRating = RatePlayerStart(P,Team,Player);
		if ( NewRating >= 30 )
		{
			// this PlayerStart is good enough
			return P;
		}
		if ( NewRating > BestRating )
		{
			BestRating = NewRating;
			BestStart = P;
		}
	}
	for ( i=0; i<RandStart; i++ )
	{
		P = PlayerStarts[i];
		NewRating = RatePlayerStart(P,Team,Player);
		if ( NewRating >= 30 )
		{
			// this PlayerStart is good enough
			return P;
		}
		if ( NewRating > BestRating )
		{
			BestRating = NewRating;
			BestStart = P;
		}
	}
	return BestStart;
}

/** RatePlayerStart()
* Return a score representing how desireable a playerstart is.
* @param P is the playerstart being rated
* @param Team is the team of the player choosing the playerstart
* @param Player is the controller choosing the playerstart
* @returns playerstart score
*/
function float RatePlayerStart(PlayerStart P, byte Team, Controller Player)
{
    local float Score, NextDist;
    local Controller OtherPlayer;
	local bool bTwoPlayerGame;

    // Primary starts are more desireable
	Score = P.bPrimaryStart ? 30 : 20;

    if ( (P == LastStartSpot) || (P == LastPlayerStartSpot) )
	{
		// avoid re-using starts
		Score -= 11.0;
	}

	bTwoPlayerGame = ( NumPlayers + NumBots == 2 );

    ForEach WorldInfo.AllControllers(class'Controller', OtherPlayer)
    {
		if ( OtherPlayer.bIsPlayer && (OtherPlayer.Pawn != None) )
		{
			// check if playerstart overlaps this pawn
			if ( (Abs(P.Location.Z - OtherPlayer.Pawn.Location.Z) < P.CylinderComponent.CollisionHeight + OtherPlayer.Pawn.CylinderComponent.CollisionHeight)
				&& (VSize2D(P.Location - OtherPlayer.Pawn.Location) < P.CylinderComponent.CollisionRadius + OtherPlayer.Pawn.CylinderComponent.CollisionRadius) )
			{
				// overlapping - would telefrag
				return -10;
			}
			NextDist = VSize(OtherPlayer.Pawn.Location - P.Location);
			if ( (NextDist < 3000) && !WorldInfo.GRI.OnSameTeam(Player,OtherPlayer) && FastTrace(P.Location, OtherPlayer.Pawn.Location+vect(0,0,1)*OtherPlayer.Pawn.CylinderComponent.CollisionHeight) )
			{
				// avoid starts close to visible enemy
				Score -= (5 - 0.001*NextDist);
			}
			else if ( bTwoPlayerGame )
			{
				// in 2 player game, look for any visibility
				Score += FMin(2,0.001*NextDist);
				if ( FastTrace(P.Location, OtherPlayer.Pawn.Location) )
					Score -= 5;
			}
		}
    }
    return FMax(Score, 0.2);
}

function PulseSeqEvent( class<SequenceObject> DesiredClass )
{
	// ã�Ƽ� ȣ���ϴ� �κ��� GameInfo.ResetLevel ���� ����.
	local Sequence GameSeq;
	local array<SequenceObject> AllSeqEvents;
	local int i;

	GameSeq = WorldInfo.GetGameSequence();
	if ( GameSeq != none )
	{
		// find DesiredClass 
		GameSeq.FindSeqObjectsByClass( DesiredClass, true, AllSeqEvents );

		//	activate them
		for ( i=0; i<AllSeqEvents.Length; ++i )
		{
			SequenceEvent( AllSeqEvents[i] ).CheckActivate( WorldInfo, None );
		}
	}
}

event BroadcastAll( Controller Sender, class<LocalMessage> Message, optional int Switch, optional PlayerReplicationInfo RelatedPRI_1, optional PlayerReplicationInfo RelatedPRI_2, optional Object OptionalObject )
{
	avaBroadcastHandler(BroadcastHandler).AllowBroadcast(Sender,Message,Switch,RelatedPRI_1,RelatedPRI_2,OptionalObject);
}

event BroadcastLocalizedTeam( Controller Sender, class<LocalMessage> Message, optional int Switch, optional PlayerReplicationInfo RelatedPRI_1, optional PlayerReplicationInfo RelatedPRI_2, optional Object OptionalObject )
{
	avaBroadcastHandler(BroadcastHandler).AllowBroadcastLocalizedTeam(Sender,Message,Switch,RelatedPRI_1,RelatedPRI_2,OptionalObject);
}

function SetPlayerDefaults(Pawn PlayerPawn)
{		
}

// Player Controller �� Team �� Swap ��Ų��.
// Team Score �� Swap �Ѵ�
function SwapTeam();

function float GetScoreCapRate()
{
	local avaGameReplicationInfo GRI;
	GRI =	avaGameReplicationInfo( WorldInfo.GRI );
	// SwapRule �� ����ǰ� �ι�° Round �̸� Score Capacity �� �ι�� �þ��...
	if ( GRI.bSwapRule == true && GRI.bSwappedTeam == true )
		return 2.0;
	return 1.0;
}

function bool FindInactivePRI(PlayerController PC)
{
	return false;
}

/** �����޼���, ��� ä��/�����޼���, �Ӹ� �޼���, ������ �ý��۸޼��� ��� */
function AddTypedMessage( name MsgType, string Msg );

`devexec function SetSpawnProtectionTime( float fTime )
{
	SpawnProtectionTime = fTime;
}

`devexec function AllowChangeGameSpeed( bool bAllow )
{
	bAllowMPGameSpeed = bAllow;
}

`devexec function ChangeGameSpeed( Float T )
{
	SetGameSpeed( T );
}

function KickPlayer( int AccountID )
{
	local avaPlayerController PC;
	foreach WorldInfo.AllControllers( class'avaPlayerController', PC )
	{
		if ( avaPlayerReplicationInfo( PC.PlayerReplicationInfo ).AccountID == AccountID )
		{
			Super.KickIdler( PC );
		}
	}
}

function bool PickupQuery(Pawn Other, class<Inventory> ItemClass, Actor Pickup)
{
	// WarmUp �߿��� PickUp�� �� �� ����...
	if ( IsWarmupRound() )
		return false;
	return Super.PickupQuery( Other, ItemClass, Pickup );
}

// {{ 20070308 dEAthcURe|HM
event function TriggerAllOutEvent( int teamNum )
{
	local Sequence				GameSeq;
	local array<SequenceObject>	AllSeqEvents;
	local int					i;
	
	GameSeq = WorldInfo.GetGameSequence();
	if ( GameSeq != none )
	{
		GameSeq.FindSeqObjectsByClass( class'avaSeqEvent_AllMembersOut', true, AllSeqEvents );
		for ( i=0; i<AllSeqEvents.Length; ++i )
			avaSeqEvent_AllMembersOut( AllSeqEvents[i] ).Trigger( WorldInfo, teamNum );
	}
}
// }} 20070308 dEAthcURe|HM

// {{ [+] 20070302 dEAthcURe|HM
event function HmRestored()
{
	`log("[dEAthcURe|avaGame::HmRestored] GameReplicationInfo.bHmRoundEnd=" @ avaGameReplicationInfo(GameReplicationInfo).bHmRoundEnd);
	if(avaGameReplicationInfo(GameReplicationInfo).bHmRoundEnd) {		
		`log("[dEAthcURe|avaGame::HmRestored] EndRoundEx");
		EndRoundEx( None, "", 0 );
	}	
}
// }} [+] 20070302 dEAthcURe|HM

// {{ 20070517 dEAthcURe|HM
event function TriggerHmEndRound()
{
	local Sequence				GameSeq;
	local array<SequenceObject>	AllSeqEvents;
	local int					i;
	GameSeq = WorldInfo.GetGameSequence();
	if ( GameSeq != none )
	{
		GameSeq.FindSeqObjectsByClass( class'avaSeqEvent_HmEndRound', true, AllSeqEvents );
		for ( i=0; i<AllSeqEvents.Length; ++i )
			avaSeqEvent_HmEndRound( AllSeqEvents[i] ).Trigger( WorldInfo);
	}
}
// }} 20070517 dEAthcURe|HM

// {{ [+] 20070303 dEAthcURe|HM 
event function HmEndRound()
{
	`log("[dEAthcURe|avaGame::HmEndRound]");
	EndRoundEx( None, "", 0 );
	TriggerHmEndRound();
}
// }} [+] 20070303 dEAthcURe|HM 

// {{ [+] 20070516 dEAthcURe|HM
event function TriggerMassacreEvent(int teamNum)
{
	// nothing to do
}
// }} [+] 20070516 dEAthcURe|HM

// {{ 20071024 dEAthcURe|HM
function bool IsBinocularPickupExist(int teamIdx)
{
	local avaPickUp_Binocular P;
	
	//`log("[dEAthcURe|avaGame::IsBinocularPickupExist] begin");
	
	ForEach DynamicActors(class'avaPickUp_Binocular', P) {		
		if(P.TeamIdx == teamIdx) {
			//`log("[dEAthcURe|avaGame::IsBinocularPickupExist] found for team " @ teamIdx);
			return true;
		}
	}
	return false;
}
// }} 20071024 dEAthcURe|HM

// {{ [+] 20070523 dEAthcURe|HM
event function HmElectSquadLeader(bool bForced, int teamIdx)
{	
	local avaGameReplicationInfo	avaGRI;
	local avaPlayerReplicationInfo	avaPRI;	
	
	avaGRI = avaGameReplicationInfo(GameReplicationInfo);
	if ( bForced || avaGRI.bWarmupRound == false ) {		
		if ( HasSquadLeader( teamIdx ) == None  && !IsBinocularPickupExist(teamIdx)) { // 20071024 pickup Ȯ�� �߰�
			avaPRI = GetSquadLeader( teamIdx );
			if ( avaPRI != None ) {
				SetSquadLeader( avaPRI );
				`log("[dEAthcURe|avaGame::HmElectSquadLeader]" @ avaPRI);
			}
		}		
	}	
}
// }} [+] 20070523 dEAthcURe|HM


//=================================================================================================
// Game Stat ���� �Լ���....
//=================================================================================================
function InitLogging()
{
	local class <avaGameStats> MyGameStatsClass;
	if ( ROLE == ROLE_Authority )
	{
		MyGameStatsClass	=	class<avaGameStats>(DynamicLoadObject(GameStatsClass,class'class'));
		if (MyGameStatsClass!=None)
	    {
			GameStats = spawn(MyGameStatsClass);
        	GameStats.Init();
	    }
	}
}

function EndLogging(string Reason)
{
	Super.EndLogging(Reason);
	if ( GameStats == None )	return;
	GameStats.Destroy();
	GameStats = None;
}

function GameEvent(name EventType, int Team, PlayerReplicationInfo InstigatorPRI)
{
	if ( GameStats != None && IsValidRoundState() )	
		GameStats.GameEvent(EventType, Team, InstigatorPRI);
}

function KillEventEx( PlayerReplicationInfo Killer, PlayerReplicationInfo Victim, class<DamageType> Damage, class<Weapon> Weapon, bool bHeadShot, int nKilledClass )
{
	if ( GameStats != None && IsValidRoundState() )	
		GameStats.KillEventEx( Killer, Victim, Damage, Weapon, bHeadShot, nKilledClass );
}

function KillEvent(string Killtype, PlayerReplicationInfo Killer, PlayerReplicationInfo Victim, class<DamageType> Damage)
{
}

function ObjectiveEvent(name StatType, int TeamNo, PlayerReplicationInfo InstigatorPRI, int ObjectiveIndex)
{
	if ( GameStats != none && IsValidRoundState() )	
		GameStats.ObjectiveEvent(StatType, TeamNo, InstigatorPRI, ObjectiveIndex);
}

function PlayerEvent(name EventType, PlayerReplicationInfo InstigatorPRI )
{
	if ( GameStats != none && IsValidRoundState() )	
		GameStats.PlayerEvent( EventType, InstigatorPRI );
}

function PickupWeaponEvent(class<avaWeapon> PickupWeapon, PlayerReplicationInfo PickupPlayer, out int OwnerStatsID )
{
	if ( GameStats != none && IsValidRoundState() )	
		GameStats.PickupWeaponEvent( PickupWeapon, PickupPlayer, OwnerStatsID );
}

function TrackSprintStat( PlayerReplicationInfo PRI, bool bStart )
{
	if ( GameStats != None && IsValidRoundState() )	
		GameStats.TrackSprintStat( PRI, bStart );
}

// ����� �Լ�.
// ���� �ð��� ��������.
`devexec function ZeroRemainingTime()
{
	RemainingTime = 3;
	GameReplicationInfo.RemainingTime = 3;
}


// Pause �� ������ ������� Check �� ��� ��....
function bool SetPause(PlayerController PC, optional delegate<CanUnpause> CanUnpauseDelegate)
{
	local int FoundIndex;
	// Pause �� ���������� ��û�� ����� authority �� ���� �����ؾ� �Ѵ�...
	// avaPlayerReplicationInfo( PC.PlayerReplicationInfo ).AccountID
//`if( `isdefined(FINAL_RELEASE) )
//	local int authority;
//	authority = class'avaNetHandler'.static.GetAvaNetHandler().GetCurrentChannelMaskLevel();
//    if ( authority < 2 )
//		return;
//`endif

	// Don't add the delegate twice (no need)
	FoundIndex = Pausers.Find(CanUnpauseDelegate);
	if (FoundIndex == INDEX_NONE)
	{
		// Not in the list so add it for querying
		FoundIndex = Pausers.Length;
		Pausers.Length = FoundIndex + 1;
		Pausers[FoundIndex] = CanUnpauseDelegate;
	}
	// Let the first one in "own" the pause state
	if (WorldInfo.Pauser == None)
	{
		WorldInfo.Pauser = PC.PlayerReplicationInfo;
	}
	return true;
}

defaultproperties
{
    HUDType=class'avaGame.avaHUD'
    ScoreBoardType=class'avaGame.avaScoreboard'
	DefaultPawnClass=class'avaGame.avaCharacter'
	GameReplicationInfoClass=class'avaGame.avaGameReplicationInfo'
	DeathMessageClass=class'avaDeathMessage'
	BroadcastHandlerClass=class'avaBroadcastHandler'
   	bRestartLevel=False
	bDelayedStart=True

    bLoggingGame=true    
    CountDown=1
    bPauseable=true
    DefaultMaxLives=0

	GameMessageClass=class'avaGame.avaGameMessage'

    MapPrefixes(0)=(Prefix="DM",GameType="avaGame.avaDeathmatch")    
	MapPrefixes(1)=(Prefix="SW",GameType="avaGame.avaSWGame")    

	nWinTeam = -1
	SpawnProtectionTime	=	5.0
}
