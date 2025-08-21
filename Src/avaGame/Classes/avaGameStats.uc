
/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaGameStats extends Info
	native
	dependson(avaPawn);

`include(avaGame/avaGame.uci)

/** Local Logging */

var FileWriter 	StatsLog;				// The Reference to the `log file
var bool		bKeepLocalStatsLog;		// If True, write local stats to a `log

/** Remote Stats */

var string		StatsRemoteAddress;
var bool		bKeepRemoteStatsLog;

/** Runtime variables */

var GameReplicationInfo 	GRI;		// Quick pointer to the GRI

var int			CurrentRound;			// ���� Round Counter

/** Player Statistics */
struct native FireStat								// �ڽ��� �߻��� Weapon �� ���� ��� Stat
{
	var	int						Kill;				// ���� Ƚ��
	var int						ClassKill[`MAX_PLAYER_CLASS];
	var int						TeamKill;			// ��ų Ƚ��
	var	int						HeadShot;			// HeadShot Ƚ��
	var	int						Fire;				// �߻� Ƚ��
	var	int						Hit;				// ���� ���� Ƚ��
	var float					TotalDistance;		// ���� ���� ���� �Ÿ�... ��� ��Ÿ��� ���ϱ� ���ؼ�....
	var int						FriendlyHit;		// �Ʊ��� ���� Ƚ�� (TeamKill �� �����ϴ� �׷��� �ʴ����� Hit �Ǹ� �ö�)
	var	int						Damage;				// ������ ���� Damage ��
	var int						DoubleKill;			// �ѹ��� ������ �θ��� �ѹ��� ���� Ƚ��
	var int						MultiKill;			// �ѹ��� ������ ���� �̻��� �ѹ��� ���� Ƚ��

	var int						HighestKill;		// ������� Level �� ���� ���� ����� ���� Ƚ��
	var int						HigherKill;			// �ڱ⺸�� Level �� ���� ����� ���� Ƚ��
	// 
	var int						MultiKillCnt;		// Multi Kill �� Check �ϱ� ���� Counter
	var int						LastKillTime;		// ���������� ���� ���� Time

	var	int						ShotInfoCnt[`MAX_SHOT_INFO];		// ��� ��������� ���� Cnt
};

struct native WeaponStat											// Weapon stat summary
{
	var class<avaWeapon>		WeaponType;							// Weapon 
	var	FireStat				FireStats;							// FireStat
	var int						nPickUpCnt;							//
};

struct native  SpawnStat											// Spawn ���� Stat
{
	var int						SpawnTime;							// Spawn �� �ð�
	var int						SpawnEndTime;						// Spawn �� ���� �ð�, ���� �ð��� ������ ���� ������ Round ������ ��Ƴ��� ���̴�.
	var int						Damaged;							// ������ ���� Damage �� ��
	var int						DamagedCount;						// ������ Damage �� ���� Ƚ��
	var int						nTeam;								// Team Index
	var array<WeaponStat>		WeaponStats;						// Weapon Class Type �� Fire Stats
	var FireStat				SpawnFireStats;						// Spawn �� Fire Stats ��Ż
	var FireStat				WeaponTypeStats[`MAX_WEAPON_TYPE];	// Spawn �� Weapon Type �� Stats ��Ż
	var int						ClassType;							// Spawn �� Player ����
	var	int						SprintTime;							// Sprint �� �ð�
	var int						SprintStartTime;					// Sprint �� ������ �ð�
};

struct native DeathStat
{
	var int						Killed;								// ���� Ƚ��
	var int						HeadShoted;							// HeadShot ���� Ƚ��
	var int						Suicide;							// �ڻ��� Ƚ��
	var int						UnLuckDeath;						// ���� ù �߿� �׾���.
	var int						Trainee;							// ������ Damage �� ������ ���ϰ� �׾���.
};

struct native RoundStat												// Round ���� Stat 
{
	var int						SpawnCount;							// Spawn �� Ƚ��.
	var DeathStat				DeathStats;
	var int						Damaged;							// ������ ���� Damage �� ��(Round��)
	var	int						TakeOffHelmetCnt;					// Helmet �� ������ Counter
	var FireStat				RoundFireStats;						// Round �� Fire Stat Total
	var FireStat				WeaponTypeStats[`MAX_WEAPON_TYPE];	// Round �� Weapon Type �� Stats ��Ż
	var array<SpawnStat>		SpawnStats;							// Spawn ���� Stat ��
	var	int						RolePointManCnt;					// PointMan ���� Spawn �� Count
	var	int						RoleRifleManCnt;					// RifleMan ���� Spawn �� Count
	var int						RoleSniperCnt;						// Sniper �� Spawn �� Count

	var	int						KillRank;							// Round �� ���� Kill Rank
	var	int						HeadShotRank;						// Round �� ���� HeadShot Rank
	var int						RoundRank;							// Round �� ���� Score Rank
	var float					RoundScore;							// Round Score
	var float					RoundExp;							// Round �� ����ġ
};

struct native HitStat
{
	var int						Victim;				// Damage �� ���� �༮
	var class<avaWeapon>		Weapon;				// ����
	var	int						ShotInfo;			// ����
	var	float					Distance;			// �Ÿ�
	var float					Damage;				// Damage
	var bool					bKill;				//
	var vector					KillerLoc;
	var vector					VictimLoc;
	var float					HitTime;
};

struct native PlayerStat
{
	var	PlayerReplicationInfo	PRI;								// The PRI of this player.  If the PRI == none then he's no longer playing
	var string					GlobalID;							// The Global ID of this player
	var string					DisplayName;						// The name used when displaying info about this stat

    var int						TotalConnectTime;					// Amount of Time this player played
    var int						TotalScore;							// The Total Score of a player

	var float					AttackPoint;						// ���� Point
	var float					DefencePoint;						// ���� Point
	var float					LeaderPoint;						// �д��� Point
	var float					TacticsPoint;						// �д�� Point

	var int						NoConnects;							// # of times this player connected
	var int						NoDisconnects;						// # of times this player disconnected

	var array<RoundStat>		RoundStats;							// ���� ���� Stat ��
	var	DeathStat				GameDeathStats;						// ���� Death Stat
	var FireStat				GameFireStats;						// ���� Fire Stat Total
	var FireStat				WeaponTypeStats[`MAX_WEAPON_TYPE];	// ���� Weapon Type �� Stats ��Ż

	var float					RoundWinScore;						// ���� ���� �� ���� Win Score
	var float					RoundLoseScore;						// ���� ���� �� ���� Lose Score

	var	int						RolePointManCnt;					// PointMan ���� Spawn �� Count
	var	int						RoleRifleManCnt;					// RifleMan ���� Spawn �� Count
	var int						RoleSniperCnt;						// Sniper �� Spawn �� Count

	var	array<HitStat>			HitStats;							// Hit �� ���

	var	float					ExpPoint;							// ����ġ
	var	int						LastTeam;							// ������ Spawn�ÿ� �����ִ� Team
};

struct native GameRoundStat
{
	var	float					StartTime;			// Round �� ������ �ð�
	var float					EndTime;			// Round �� ������ �ð�
	var string					EndRoundDesc;		// Round ���� Description
	var int						Winner;				// Round �¸� Team
	var int						WinType;			// avaGame.uc �� EWinType ����
	var int						PlayerCnt[2];		// Round �� ������ �־��� Player ��
};


var array<PlayerStat>			PlayerStats;
var array<GameRoundStat>		GameRoundStats;

/** Game Statistics */

struct native GameStat
{
	var int						TimeStamp;				// When did this happen
	var	name					GameStatType;			// What type of stat is this

	var int						Team;					// What team was this event for (-1 = all)

	var int						InstigatorID;			// The ID of the player who caused the stat
	var int						AdditionalID;			// Any additional Player ID's needed
	var class<object>			AdditionalData;			// Any additional data needed
};


var array<GameStat>					GameStats;

var array<Class<avaStatsSummary> >	StatSummaryClasses;
var array<avaStatsSummary>			StatSummaryObjects;

/************************************************************************************
 * Native Functions
 ************************************************************************************/

/**
 * Returns the name of the the current map
 */
native final function string GetMapFilename();

function string GetGlobalID(PlayerReplicationInfo PRI)	// TEMP: update when CDEKYs are active
{
	return ( PRI != None ) ? PRI.PlayerName : "";
}

//function WEAPON_TYPE GetWeaponType(class<avaWeapon> weapon)
//{
//	if ( class<avaWeap_BaseKnife>(weapon) != None  )				return WEAPON_KNIFE;
//	else if ( class<avaWeap_BasePistol>(weapon) != None )			return WEAPON_PISTOL;
//	else if ( class<avaWeap_BaseSMG>(weapon) != None )				return WEAPON_SMG;
//	else if ( class<avaWeap_BaseRifle>(weapon) != None )			return WEAPON_RIFLE;
//	else if ( class<avaWeap_BaseSniperRifle>(weapon) != None )		return WEAPON_SNIPER;
//	else if ( class<avaWeap_Grenade>(weapon) != None )				return WEAPON_GRENADE;
//	return WEAPON_ETC;
//}

function Summary()
{
	local int i;
	for (i=0;i<StatSummaryObjects.Length;i++)
	{
		StatSummaryObjects[i].GenerateSummary();
	}
}

event Destroyed()
{
	ShutdownStats();
	Super.Destroyed();
}

function int GetTimeStamp()
{
	return int(WorldInfo.TimeSeconds);
}

function String GetMapName()
{
	local string mapname;
	local string tab;
	tab = ""$Chr(9);
	mapname = ""$GetMapFilename();

	// Remove the file name extention .ut3
	if( InStr(mapname,".ut3") != -1 )
		mapname = Left( mapname, InStr(mapname,".ut3") );

	ReplaceText(mapname, tab, "_");

	return mapname;
}

function int GetStatsIDFromGlobalID(string GlobalID)
{
	local int i;

	for (i=0;i<PlayerStats.length;i++)
	{
		if (PlayerStats[i].GlobalID ~= GlobalID)
		{
			return i;
		}
	}

	return -1;
}

/************************************************************************************
 * PlayerStats Accessor Functions
 ************************************************************************************/

function int AddNewPlayer(Controller NewPlayer, string GlobalID)
{
	local int NewID;

	NewID = PlayerStats.Length;
	PlayerStats.Length = NewID + 1;

	PlayerStats[NewID].PRI = NewPlayer.PlayerReplicationInfo;
	PlayerStats[NewID].GlobalID = GlobalID;
	PlayerStats[NewID].DisplayName = NewPlayer.PlayerReplicationInfo.PlayerName;

	return NewID;
}

/**
 * This function Adds an objective link to the game stats table
 */

function AddObjectiveLink(int StatID, int GameStatID)
{
	//local int i;

	//if (StatId >= 0 && StatId < PlayerStats.Length)
	//{
	//	i = PlayerStats[StatID].ObjectiveStats.Length;
	//	PlayerStats[StatID].ObjectiveStats.Length = i + 1;

	//	PlayerStats[StatID].ObjectiveStats[i] = GameStatID;
	//}
}

/**
 * This functions looks at a player's stats entry and returns the Total Connect time.
 */

function int GetPlayerConnectTime(int StatId)
{
	local int tm;
	if (StatId >= 0 && StatId < PlayerStats.Length)
	{
		tm = PlayerStats[StatId].TotalConnectTime;
		if (PlayerStats[StatID].PRI != none)
		{
			tm += WorldInfo.TimeSeconds - PlayerStats[StatID].PRI.StartTime;
		}
		return tm;
	}

	return 0;
}

/**
 * This functions looks at a player's stats entry and returns the Total Connect time.
 */

function int GetPlayerScore(int StatId)
{
	local int sc;
	if (StatId >= 0 && StatId < PlayerStats.Length)
	{
		sc = PlayerStats[StatId].TotalScore;
		if (PlayerStats[StatID].PRI != none)
		{
			sc += PlayerStats[StatID].PRI.Score;
		}
		return sc;
	}

	return 0;
}

function string GetDisplayName(int StatId)
{
	return (StatId >=0 && StatID < PlayerStats.Length) ? PlayerStats[StatID].DisplayName : "None";
}

/************************************************************************************
 * GameStats Accessor Functions
 ************************************************************************************/


/**
 * Adds a Game Stat to the collection
 */

function int AddGameStat(name NewStatType, int NewTeam, int NewInstigatorID, int NewAdditionalID, optional class<object> NewAdditionalData)
{
	local int NewID;
	local string s;
	NewID = GameStats.Length;
	GameStats.Length = NewID + 1;

	GameStats[NewID].TimeStamp		= GetTimeStamp();
	GameStats[NewID].GameStatType 	= NewStatType;
	GameStats[NewID].Team			= NewTeam;
	GameStats[NewID].InstigatorID 	= NewInstigatorID;
	GameStats[NewID].AdditionalID  	= NewAdditionalID;
	GameStats[NewID].AdditionalData	= NewAdditionalData;

	if (NewAdditionalID>=0)
	{
		s = "Additonal ID="@PlayerStats[NewAdditionalID].DisplayName;
	}

	if (NewAdditionalData != none)
	{
		s = s@"Data="@NewAdditionalData;
	}

	StatLog("Team="@NewTeam@"Instigator="@GetDisplayName(NewInstigatorID)@"at"@GameStats[NewID].TimeStamp, NewStatType );
	if (s != "")
	{
		StatLog(s,NewStatType);
	}

	return NewID;
}


/************************************************************************************
 * Interface Functions - Utility
 ************************************************************************************/

/**
 * Initialize the stats system.
 */

function Init()
{
	local int i,idx;
	local avaStatsSummary Sum;

	`log("Stats Collection system Active");

	if (bKeepLocalStatsLog)
	{
		StatsLog = spawn(class 'FileWriter');
		if (StatsLog!=None)
		{
			StatsLog.OpenFile("GameStats",FWFT_Log,,,true);
			`log("  Collecting Local Stats to:"@StatsLog.Filename);
		}
	}

	if (bKeepRemoteStatsLog)
	{		
		`log("  Collecting Remote Stats to:"@StatsRemoteAddress);
	}

	for (i=0;i<StatSummaryClasses.length;i++)
	{
		Sum = new(self) StatSummaryClasses[i];
		if (Sum!=none)
		{
			idx = StatSummaryObjects.Length;
			StatSummaryObjects.Length = idx + 1;
			StatSummaryObjects[idx] = Sum;
			`log("  Added Summary Object:"@Sum);
		}
	}
}

/**
 * Shutdown the `log system
 */

simulated function ShutdownStats()
{
	Summary();

	if (StatsLog!=None)
	{
		StatsLog.Destroy();
	}	
}

/**
 * Write a string to the stats `log file if it exists
 */

function StatLog(string LogString, optional coerce string Section)
{
	local string OutStr;	

	if (Section != "")
	{
		OutStr = "["$Section$"]"@LogString;
	}



	if (StatsLog!=None)
	{
		StatsLog.Logf(OutStr);
	}	
}

/************************************************************************************
 * Interface Functions - Events
 ************************************************************************************/

function NextRoundEvent()
{
	AddGameStat('NewRound',-1,-1,-1);
}

function MatchBeginEvent()
{
	AddGameStat('MatchBegins',-1,-1,-1);
}

function MatchEndEvent(name Reason, PlayerReplicationInfo Winner)
{
	AddGameStat(Reason,Winner.GetTeamNum(),-1,-1);
}

// Player �� Connet ����...
function ConnectEvent(Controller NewPlayer)
{
	local int StatsID;
	local string GlobalID;

	GlobalId = GetGlobalID(NewPlayer.PlayerReplicationInfo);
	StatsId = GetStatsIDFromGlobalID(GlobalID);

	if ( StatsID>=0 )
	{
		PlayerStats[StatsID].PRI = NewPlayer.PlayerReplicationInfo;
		PlayerStats[StatsID].DisplayName = NewPlayer.PlayerReplicationInfo.PlayerName;
		AddGameStat('Reconnect', -1, StatsID, -1);
	}
	else
	{
		StatsID = AddNewPlayer(NewPlayer, GlobalID);
		AddGameStat('Connect', -1, StatsID, -1);
	}

	PlayerStats[StatsID].NoConnects++;

	// ���� Round ��ŭ ������ ��� �Ѵ�...
	if ( CurrentRound > 0 )
		PlayerStats[StatsID].RoundStats.length = CurrentRound;
}

// Connect Events get fired every time a player connects or leaves from a server
function DisconnectEvent(Controller DepartingPlayer)
{
	local int StatsID;
	local string GlobalID;

	GlobalId = GetGlobalID(DepartingPlayer.PlayerReplicationInfo);
	StatsId = GetStatsIDFromGlobalID(GlobalID);

	if (StatsID>=0)
	{
		AddGameStat('Disconnect', StatsID, -1, -1);
		PlayerStats[StatsID].PRI = none;
		PlayerStats[StatsID].DisplayName = DepartingPlayer.PlayerReplicationInfo.PlayerName;
		PlayerStats[StatsID].TotalConnectTime += WorldInfo.TimeSeconds - DepartingPlayer.PlayerReplicationInfo.StartTime;
		PlayerStats[StatsID].TotalScore += DepartingPlayer.PlayerReplicationInfo.Score;
		PlayerStats[StatsID].NoDisconnects++;
	}
}

function DamageEvent( int KillerStatsID, int VictimStatsID, int Damage, class<weapon> Weapon, bool bTakeOffHelmet )
{
	if ( IsValidPlayer( KillerStatsID ) )	TrackPlayerDamageStat( KillerStatsID, Weapon, Damage );
	if ( IsValidPlayer( VictimStatsID ) )	TrackPlayerDamagedStat( VictimStatsID, Damage, bTakeOffHelmet );
}

function KillEventEx( PlayerReplicationInfo Killer, PlayerReplicationInfo Victim, class<DamageType> Damage, class<Weapon> Weapon, bool bHeadShot, int nKilledClass )
{
	local int VictimStatsID, KillerStatsID;
	local bool bTeamKill;
	local int HighestLvl;
	local int WeaponStatID;

	VictimStatsID	= GetStatsIDFromGlobalID( GetGlobalID(Victim) );
	KillerStatsID	= GetStatsIDFromGlobalID( GetGlobalID(Killer) );
	bTeamKill		= WorldInfo.GRI.OnSameTeam(Killer,Victim);

	WeaponStatID	= GetWeaponStatID( KillerStatsID, class<avaWeapon>(Weapon) );

	if ( VictimStatsID != KillerStatsID && IsValidPlayer( KillerStatsID ) && WeaponStatID >= 0 )
	{
		if ( !bTeamKill && WorldInfo.Game.bTeamGame == true )
		{
			HighestLvl = avaPlayerReplicationInfo( avaGameReplicationInfo( WorldInfo.GRI ).TopLevelPRI[Victim.Team.TeamIndex] ).Level;
			if ( avaPlayerReplicationInfo( Victim ).Level == HighestLvl )								TrackFireStat( KillerStatsID, Weapon, 'HST' );
			if ( avaPlayerReplicationInfo( Victim ).Level > avaPlayerReplicationInfo( Killer ).Level )	TrackFireStat( KillerStatsID, Weapon, 'HER' );
		}
		TrackPlayerKillStat( KillerStatsID, Weapon, bTeamKill, bHeadShot, nKilledClass );
	}

	if ( IsValidPlayer( VictimStatsID ) )	
		TrackPlayerDeathStat(VictimStatsID, (VictimStatsID == KillerStatsID), bHeadShot );
}

function HitEvent( PlayerReplicationInfo Killer, PlayerReplicationInfo Victim, class<Weapon> WeaponBy, int ShotInfo, float Distance, float Damage, bool bKill, bool bTakeOffHelmet, vector killerLoc, vector victimLoc )
{
	local int VictimStatsID, KillerStatsID;
	VictimStatsID	= GetStatsIDFromGlobalID( GetGlobalID(Victim) );
	KillerStatsID	= GetStatsIDFromGlobalID( GetGlobalID(Killer) );

	TrackPlayerHitStat( KillerStatsID, VictimStatsID, WeaponBy, ShotInfo, Distance, Damage, bKill, killerLoc, victimLoc );
	DamageEvent( KillerStatsID, VictimStatsID, Damage, class<avaWeapon>(WeaponBy), bTakeOffHelmet );
}

function TrackPlayerHitStat( int ID1, int ID2, class<Weapon> WeaponBy, int ShotInfo, float Distance, float Damage, bool bKill, vector killerLoc, vector victimLoc )
{
	local int	NewID;
	NewID = PlayerStats[ID1].HitStats.length;
	PlayerStats[ID1].HitStats.length			= NewID + 1;
	PlayerStats[ID1].HitStats[NewID].Victim		= ID2;
	PlayerStats[ID1].HitStats[NewID].Weapon		= class<avaWeapon>(WeaponBy);
	PlayerStats[ID1].HitStats[NewID].ShotInfo	= ShotInfo;
	PlayerStats[ID1].HitStats[NewID].Distance	= Distance;
	PlayerStats[ID1].HitStats[NewID].Damage		= Damage;
	PlayerStats[ID1].HitStats[NewID].bKill		= bKill;
	PlayerStats[ID1].HitStats[NewID].KillerLoc	= KillerLoc;
	PlayerStats[ID1].HitStats[NewID].VictimLoc	= VictimLoc;
	PlayerStats[ID1].HitStats[NewID].HitTime	= GetTimeStamp();

	TrackFireStat( ID1, class<avaWeapon>(WeaponBy), 'SI', ShotInfo );
}

function TrackSprintStat( PlayerReplicationInfo PRI, bool bStart )
{
	local int SpawnCnt;
	local int SprintStartTime;
	local int SprintEndTime;
	local int ID;
	ID = GetStatsIDFromGlobalID( GetGlobalID(PRI) );
	SpawnCnt = GetCurrentRoundSpawnCount( ID );
	if ( bStart )
	{
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].SprintStartTime = GetTimeStamp();
	}
	else
	{
		SprintStartTime = PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].SprintStartTime;
		if ( SprintStartTime > 0 )
		{
			SprintEndTime = GetTimeStamp();
			if ( SprintEndTime > SprintStartTime )
			{
				PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].SprintTime += ( SprintEndTime - SprintStartTime );
			}
		}
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].SprintStartTime = 0;
	}
}


function TrackFireStat( int ID, class<Weapon> Weapon, name Var, optional int val = 1, optional float fVal = 0.0 )
{
	local WeaponStat weaponStats;
	local int SpawnCnt;
	local int weaponType;
	local int WID;

	//`log("avaGameStats.TrackFireStat" @ID @Weapon @Var @val @fVal);

	// array out of bounds?
	if ( ID < 0 || ID >= PlayerStats.Length ) return;

	/// suppress warning :)
	if (PlayerStats[ID].RoundStats.Length == 0) return;

	WID = GetWeaponStatID( ID, class<avaWeapon>(Weapon) );

	SpawnCnt	= GetCurrentRoundSpawnCount( ID );

	if (SpawnCnt<=0)	return;
	if (SpawnCnt >= PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats.Length) return;
	if (WID >= PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats.Length) return ;

	//`log("WID =" @WID 
	//	@"PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats.Length =" @PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats.Length
	//	@"PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats.Length =" @PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats.Length
	//	@"weaponStats.WeaponType =" @weaponStats.WeaponType);

	weaponStats = PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats[WID];

	if ( weaponStats.WeaponType != None )
		weaponType = weaponStats.WeaponType.default.WeaponType;




	if ( Var == 'K' )				// Kill
	{
		PlayerStats[ID].GameFireStats.Kill+=1;
		PlayerStats[ID].WeaponTypeStats[weaponType].Kill+=1;
		PlayerStats[ID].RoundStats[CurrentRound-1].RoundFireStats.Kill+=1;
		PlayerStats[ID].RoundStats[CurrentRound-1].WeaponTypeStats[weaponType].Kill+=1;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].SpawnFireStats.Kill+=1;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponTypeStats[weaponType].Kill+=1;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats[WID].FireStats.Kill+=1;

		PlayerStats[ID].GameFireStats.ClassKill[val]+=1;
		PlayerStats[ID].WeaponTypeStats[weaponType].ClassKill[val]+=1;
		PlayerStats[ID].RoundStats[CurrentRound-1].RoundFireStats.ClassKill[val]+=1;
		PlayerStats[ID].RoundStats[CurrentRound-1].WeaponTypeStats[weaponType].ClassKill[val]+=1;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].SpawnFireStats.ClassKill[val]+=1;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponTypeStats[weaponType].ClassKill[val]+=1;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats[WID].FireStats.ClassKill[val]+=1;
	}
	else if ( Var == 'TK' )			// Team Kill
	{
		PlayerStats[ID].GameFireStats.TeamKill+=1;
		PlayerStats[ID].WeaponTypeStats[weaponType].TeamKill+=1;
		PlayerStats[ID].RoundStats[CurrentRound-1].RoundFireStats.TeamKill+=1;
		PlayerStats[ID].RoundStats[CurrentRound-1].WeaponTypeStats[weaponType].TeamKill+=1;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].SpawnFireStats.TeamKill+=1;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponTypeStats[weaponType].TeamKill+=1;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats[WID].FireStats.TeamKill+=1;

		PlayerStats[ID].GameFireStats.ClassKill[val]+=1;
		PlayerStats[ID].WeaponTypeStats[weaponType].ClassKill[val]+=1;
		PlayerStats[ID].RoundStats[CurrentRound-1].RoundFireStats.ClassKill[val]+=1;
		PlayerStats[ID].RoundStats[CurrentRound-1].WeaponTypeStats[weaponType].ClassKill[val]+=1;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].SpawnFireStats.ClassKill[val]+=1;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponTypeStats[weaponType].ClassKill[val]+=1;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats[WID].FireStats.ClassKill[val]+=1;
	}
	else if ( Var == 'HS' )			// Head Shot Kill
	{
		PlayerStats[ID].GameFireStats.HeadShot+=val;
		PlayerStats[ID].WeaponTypeStats[weaponType].HeadShot+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].RoundFireStats.HeadShot+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].WeaponTypeStats[weaponType].HeadShot+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].SpawnFireStats.HeadShot+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponTypeStats[weaponType].HeadShot+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats[WID].FireStats.HeadShot+=val;
	}
	else if ( Var == 'F' )			//	Fire Count
	{
		PlayerStats[ID].GameFireStats.Fire+=val;
		PlayerStats[ID].WeaponTypeStats[weaponType].Fire+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].RoundFireStats.Fire+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].WeaponTypeStats[weaponType].Fire+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].SpawnFireStats.Fire+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponTypeStats[weaponType].Fire+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats[WID].FireStats.Fire+=val;
	}
	else if ( Var == 'H' )			//	Hit Count
	{
		PlayerStats[ID].GameFireStats.Hit+=val;
		PlayerStats[ID].WeaponTypeStats[weaponType].Hit+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].RoundFireStats.Hit+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].WeaponTypeStats[weaponType].Hit+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].SpawnFireStats.Hit+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponTypeStats[weaponType].Hit+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats[WID].FireStats.Hit+=val;

		PlayerStats[ID].GameFireStats.TotalDistance+=fVal;
		PlayerStats[ID].WeaponTypeStats[weaponType].TotalDistance+=fVal;
		PlayerStats[ID].RoundStats[CurrentRound-1].RoundFireStats.TotalDistance+=fVal;
		PlayerStats[ID].RoundStats[CurrentRound-1].WeaponTypeStats[weaponType].TotalDistance+=fVal;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].SpawnFireStats.TotalDistance+=fVal;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponTypeStats[weaponType].TotalDistance+=fVal;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats[WID].FireStats.TotalDistance+=fVal;
	}
	else if ( Var == 'FH' )			//	Friendly Hit Count
	{
		PlayerStats[ID].GameFireStats.FriendlyHit																	+=val;
		PlayerStats[ID].WeaponTypeStats[weaponType].FriendlyHit														+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].RoundFireStats.FriendlyHit										+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].WeaponTypeStats[weaponType].FriendlyHit							+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].SpawnFireStats.FriendlyHit				+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponTypeStats[weaponType].FriendlyHit	+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats[WID].FireStats.FriendlyHit	+=val;

		PlayerStats[ID].GameFireStats.TotalDistance+=fVal;
		PlayerStats[ID].WeaponTypeStats[weaponType].TotalDistance+=fVal;
		PlayerStats[ID].RoundStats[CurrentRound-1].RoundFireStats.TotalDistance+=fVal;
		PlayerStats[ID].RoundStats[CurrentRound-1].WeaponTypeStats[weaponType].TotalDistance+=fVal;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].SpawnFireStats.TotalDistance+=fVal;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponTypeStats[weaponType].TotalDistance+=fVal;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats[WID].FireStats.TotalDistance+=fVal;
	}
	else if ( Var == 'D' )			//	Damage
	{
		PlayerStats[ID].GameFireStats.Damage+=val;
		PlayerStats[ID].WeaponTypeStats[weaponType].Damage+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].RoundFireStats.Damage+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].WeaponTypeStats[weaponType].Damage+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].SpawnFireStats.Damage+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponTypeStats[weaponType].Damage+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats[WID].FireStats.Damage+=val;
	}
	else if ( Var == 'DK' )			//	Double Kill
	{
		PlayerStats[ID].GameFireStats.DoubleKill+=val;
		PlayerStats[ID].WeaponTypeStats[weaponType].DoubleKill+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].RoundFireStats.DoubleKill+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].WeaponTypeStats[weaponType].DoubleKill+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].SpawnFireStats.DoubleKill+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponTypeStats[weaponType].DoubleKill+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats[WID].FireStats.DoubleKill+=val;
	}
	else if ( Var == 'MK' )			// Multi Kill
	{
		PlayerStats[ID].GameFireStats.MultiKill+=val;
		PlayerStats[ID].WeaponTypeStats[weaponType].MultiKill+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].RoundFireStats.MultiKill+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].WeaponTypeStats[weaponType].MultiKill+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].SpawnFireStats.MultiKill+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponTypeStats[weaponType].MultiKill+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats[WID].FireStats.MultiKill+=val;
	}
	else if ( Var == 'SI' )			// Multi Kill
	{
		PlayerStats[ID].GameFireStats.ShotInfoCnt[val]++;
		PlayerStats[ID].WeaponTypeStats[weaponType].ShotInfoCnt[val]++;
		PlayerStats[ID].RoundStats[CurrentRound-1].RoundFireStats.ShotInfoCnt[val]++;
		PlayerStats[ID].RoundStats[CurrentRound-1].WeaponTypeStats[weaponType].ShotInfoCnt[val]++;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].SpawnFireStats.ShotInfoCnt[val]++;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponTypeStats[weaponType].ShotInfoCnt[val]++;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats[WID].FireStats.ShotInfoCnt[val]++;
	}
	else if ( Var == 'HST' )			// Highest Kill
	{
		PlayerStats[ID].GameFireStats.HighestKill+=val;
		PlayerStats[ID].WeaponTypeStats[weaponType].HighestKill+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].RoundFireStats.HighestKill+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].WeaponTypeStats[weaponType].HighestKill+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].SpawnFireStats.HighestKill+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponTypeStats[weaponType].HighestKill+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats[WID].FireStats.HighestKill+=val;
	}
	else if ( Var == 'HER' )			// Higher Kill
	{
		PlayerStats[ID].GameFireStats.HigherKill+=val;
		PlayerStats[ID].WeaponTypeStats[weaponType].HigherKill+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].RoundFireStats.HigherKill+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].WeaponTypeStats[weaponType].HigherKill+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].SpawnFireStats.HigherKill+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponTypeStats[weaponType].HigherKill+=val;
		PlayerStats[ID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats[WID].FireStats.HigherKill+=val;
	}
}

function TrackPlayerDamageStat( int id, class<weapon>  weapon, int damage )
{
	TrackFireStat( id, weapon, 'D', damage );
}

function TrackPlayerDamagedStat( int id, int damage, bool bTakeOffHelmet )
{
	local int SpawnCnt;
	SpawnCnt	= GetCurrentRoundSpawnCount( id );
	if ( bTakeOffHelmet )	PlayerStats[id].RoundStats[CurrentRound-1].TakeOffHelmetCnt += 1;
	PlayerStats[id].RoundStats[CurrentRound-1].Damaged += damage;
	PlayerStats[id].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].Damaged += damage;
	++PlayerStats[id].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].DamagedCount;
}

function TrackPlayerKillStat(int StatsID, class<Weapon> weapon, bool bTeamKill, bool bHeadShot, int KilledClass )
{
	local int SpawnCnt;
	local WeaponStat weaponStats;
	local int WeaponStatsID;
	WeaponStatsID = GetWeaponStatID( StatsID, class<avaWeapon>(weapon) );

	SpawnCnt	= GetCurrentRoundSpawnCount( StatsID );
	weaponStats = PlayerStats[StatsID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats[WeaponStatsID];
	if ( bTeamKill )	TrackFireStat( StatsID, weapon, 'TK', KilledClass );
	else				TrackFireStat( StatsID, weapon, 'K', KilledClass );
	if ( bHeadShot )	TrackFireStat( StatsID, weapon, 'HS' );
	if ( weaponStats.FireStats.LastKillTime == GetTimeStamp() )	++PlayerStats[StatsID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats[WeaponStatsID].FireStats.MultiKillCnt;
	else														PlayerStats[StatsID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats[WeaponStatsID].FireStats.MultiKillCnt=0;
	// Double Kill
	if ( PlayerStats[StatsID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats[WeaponStatsID].FireStats.MultiKillCnt > 0 )
	{
		if ( PlayerStats[StatsID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats[WeaponStatsID].FireStats.MultiKillCnt == 1 )		
		{
			TrackFireStat( StatsID, weapon, 'DK' );
		}
		else if ( PlayerStats[StatsID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats[WeaponStatsID].FireStats.MultiKillCnt == 2 )	TrackFireStat( StatsID, weapon, 'MK' );
	}
	PlayerStats[StatsID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats[WeaponStatsID].FireStats.LastKillTime = GetTimeStamp();
}

function TrackSpawnEnd(int StatsID)
{
	local int SpawnCnt;
	SpawnCnt	= GetCurrentRoundSpawnCount( StatsID );
	PlayerStats[StatsID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].SpawnEndTime = GetTimeStamp();
}

function TrackPlayerDeathStat(int StatsID, bool bSuicide, bool bHeadShot )
{
	local int SpawnCnt;
	SpawnCnt	 = GetCurrentRoundSpawnCount( StatsID );
	TrackSpawnEnd( StatsID );
	if ( bSuicide )	
	{
		++PlayerStats[StatsID].GameDeathStats.Suicide;
		++PlayerStats[StatsID].RoundStats[CurrentRound-1].DeathStats.Suicide;
	}
	else
	{
		++PlayerStats[StatsID].GameDeathStats.Killed;
		++PlayerStats[StatsID].RoundStats[CurrentRound-1].DeathStats.Killed;
		// Unlucky Death Check
		if ( PlayerStats[StatsID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].DamagedCount == 1 )
		{
			++PlayerStats[StatsID].GameDeathStats.UnluckDeath;
			++PlayerStats[StatsID].RoundStats[CurrentRound-1].DeathStats.UnluckDeath;
		}
		// trainee Check - ������ �ƹ� Damage �� ������ ���ϰ� ������ ������
		if ( PlayerStats[StatsID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].SpawnFireStats.Damage == 0 )
		{
			++PlayerStats[StatsID].GameDeathStats.Trainee;
			++PlayerStats[StatsID].RoundStats[CurrentRound-1].DeathStats.Trainee;
		}
	}

	if ( bHeadShot )	
	{
		++PlayerStats[StatsID].GameDeathStats.HeadShoted;
		++PlayerStats[StatsID].RoundStats[CurrentRound-1].DeathStats.HeadShoted;
	}
}

// Game Event
//	'NewRound'
function GameEvent(name EventType, int Team, PlayerReplicationInfo InstigatorPRI)
{
	local int		InstigatorID;
	local int		StatsID;
	local string	GlobalID;

	if ( InstigatorPRI != None )
	{
		InstigatorID = GetStatsIDFromGlobalID( GetGlobalID(InstigatorPRI) );
		if (InstigatorID>=0)
		{
			AddGameStat(EventType, Team, InstigatorID,-1);
			if (EventType=='NameChange')
			{
				GlobalId = GetGlobalID(InstigatorPRI);
				StatsId = GetStatsIDFromGlobalID(GlobalID);
				PlayerStats[StatsId].DisplayName = InstigatorPRI.PlayerName;
			}
		}
	}
	else
	{
		if ( EventType == 'NewRound' )			BeginNewRound();
	}
}


// PlayerEvent
//		'NewSpawn'
function PlayerEvent(name EventType, PlayerReplicationInfo InstigatorPRI )
{
	local int		StatsID;
	StatsID = GetStatsIDFromGlobalID( GetGlobalID(InstigatorPRI) );
	if ( EventType == 'NewSpawn' )	PlayerSpawned( StatsID );
}

// ���ο� Round �� ���۵Ǿ���.
function BeginNewRound()
{
	local int i;
	++CurrentRound;
	// Player �� RoundStats �� Update ����� �Ѵ�...
	for ( i = 0 ; i < PlayerStats.length ; ++ i )
		PlayerStats[i].RoundStats.length = CurrentRound;

	GameRoundStats.length = CurrentRound;
	GameRoundStats[CurrentRound-1].StartTime = GetTimeStamp();
	GameRoundStats[CurrentRound-1].Winner	 = -1;
}

function EndRound( string desc, int nWinner, int WinType )
{
	local int			i,j;
	local float			PrvRoundScore;
	local float			ExpPoint;
	local int			TotalPlayer;
	local int			TeamNum[2];
	local PlayerStat	SrcPlayer,DstPlayer;
	local int			Round;

	Round = CurrentRound-1;

	// �߰��� ���� Player �� ����ؾ� �Ѵ�...
	// �߰��� ���� Player �� ��� PRI �� None �̴�...

	for ( i = 0 ; i < PlayerStats.length ; ++ i )
	{
		if ( PlayerStats[i].PRI == None )		continue;

		TrackSpawnEnd( i );

		// ���� ���� �� �� Point �� Stat �� Update �Ѵ�...
		PlayerStats[i].AttackPoint	=	avaPlayerReplicationInfo( PlayerStats[i].PRI ).GetPoint( PointType_Attack );
		PlayerStats[i].DefencePoint	=	avaPlayerReplicationInfo( PlayerStats[i].PRI ).GetPoint( PointType_Defence );
		PlayerStats[i].LeaderPoint	=	avaPlayerReplicationInfo( PlayerStats[i].PRI ).GetPoint( PointType_Leader );
		PlayerStats[i].TacticsPoint =	avaPlayerReplicationInfo( PlayerStats[i].PRI ).GetPoint( PointType_Tactics );

		if ( PlayerStats[i].PRI.Team == None )	continue;
		if ( PlayerStats[i].PRI.Team.TeamIndex < 0 || PlayerStats[i].PRI.Team.TeamIndex > 1 )	continue;
		++TeamNum[ PlayerStats[i].PRI.Team.TeamIndex ];

		PrvRoundScore = 0.0;
		for ( j = 0 ; j < PlayerStats[i].RoundStats.length ; ++ j )
			PrvRoundScore += PlayerStats[i].RoundStats[j].RoundScore;
		PlayerStats[i].RoundStats[Round].RoundScore = PlayerStats[i].PRI.Score - PrvRoundScore;
	}

	TotalPlayer = TeamNum[0] + TeamNum[1];

	// Round �� ������ ���� Rank �� �ο��Ѵ�...
	for ( i = 0 ; i < PlayerStats.length ; ++ i )
	{	
		SrcPlayer = PlayerStats[i];

		if ( SrcPlayer.PRI == None )	continue;
		if ( SrcPlayer.PRI.Team == None )	continue;
		if ( SrcPlayer.PRI.Team.TeamIndex < 0 || SrcPlayer.PRI.Team.TeamIndex > 1 )	continue;

		for ( j = 0 ; j < PlayerStats.length ; ++ j )
		{
			DstPlayer = PlayerStats[j];
			if ( i == j )																continue;
			if ( DstPlayer.PRI == None )												continue;
			if ( DstPlayer.PRI.Team == None )											continue;
			if ( DstPlayer.PRI.Team.TeamIndex < 0 || DstPlayer.PRI.Team.TeamIndex > 1 )	continue;
			if ( SrcPlayer.PRI.Team.TeamIndex != DstPlayer.PRI.Team.TeamIndex )			continue;

			if ( SrcPlayer.RoundStats[Round].RoundFireStats.Kill < DstPlayer.RoundStats[Round].RoundFireStats.Kill )			++PlayerStats[i].RoundStats[Round].KillRank;
			if ( SrcPlayer.RoundStats[Round].RoundFireStats.HeadShot < DstPlayer.RoundStats[Round].RoundFireStats.HeadShot )	++PlayerStats[i].RoundStats[Round].HeadShotRank;
			if ( SrcPlayer.RoundStats[Round].RoundScore < DstPlayer.RoundStats[Round].RoundScore )								++PlayerStats[i].RoundStats[Round].RoundRank;
		}

		if ( PlayerStats[i].PRI.Team.TeamIndex == nWinner )	ExpPoint += avaGameReplicationInfo( WorldInfo.Game.GameReplicationInfo ).BaseScore;
		else												ExpPoint += avaGameReplicationInfo( WorldInfo.Game.GameReplicationInfo ).BaseScore * 0.3;

		ExpPoint += PlayerStats[i].RoundStats[Round].RoundScore * 10;
		ExpPoint -= PlayerStats[i].RoundStats[Round].DeathStats.Killed * 3;
		ExpPoint *= TotalPlayer / TeamNum[PlayerStats[i].PRI.Team.TeamIndex] / 2.0;
		if ( TotalPlayer <= 6 )	ExpPoint *= 0.5;
		if ( ExpPoint < 0 )		ExpPoint = 0.0;

		PlayerStats[i].RoundStats[Round].RoundExp = ExpPoint;
		PlayerStats[i].ExpPoint += ExpPoint;
	}

	GameRoundStats[Round].EndTime		= GetTimeStamp();
	GameRoundStats[Round].EndRoundDesc	= desc;
	GameRoundStats[Round].Winner		= nWinner;
	GameRoundStats[Round].WinType		= WinType;
	GameRoundStats[Round].PlayerCnt[0]	= TeamNum[0];
	GameRoundStats[Round].PlayerCnt[1]	= TeamNum[1];
	
}

// Player �� Spawn �Ǿ���.
function PlayerSpawned( int StatID )
{
	local int SpawnCount;
	SpawnCount = ++PlayerStats[StatID].RoundStats[CurrentRound-1].SpawnCount;
	PlayerStats[StatID].RoundStats[CurrentRound-1].SpawnStats.length = SpawnCount;
	PlayerStats[StatID].RoundStats[CurrentRound-1].SpawnStats[SpawnCount-1].SpawnTime	= GetTimeStamp();
	PlayerStats[StatID].RoundStats[CurrentRound-1].SpawnStats[SpawnCount-1].ClassType	= avaPlayerReplicationInfo( PlayerStats[StatID].PRI ).PlayerClassID;
	if ( PlayerStats[StatID].PRI.Team != None )
	{
		PlayerStats[StatID].RoundStats[CurrentRound-1].SpawnStats[SpawnCount-1].nTeam		= PlayerStats[StatID].PRI.Team.TeamIndex;		
		PlayerStats[StatID].LastTeam														= PlayerStats[StatID].PRI.Team.TeamIndex;
	}
	switch( avaPlayerReplicationInfo( PlayerStats[StatID].PRI ).PlayerClassID )
	{
	case 0:		++PlayerStats[StatID].RoundStats[CurrentRound-1].RolePointManCnt;	
				++PlayerStats[StatID].RolePointManCnt;
				break;
	case 1:		++PlayerStats[StatID].RoundStats[CurrentRound-1].RoleRifleManCnt;	
				++PlayerStats[StatID].RoleRifleManCnt;
				break;
	case 2:		++PlayerStats[StatID].RoundStats[CurrentRound-1].RoleSniperCnt;		
				++PlayerStats[StatID].RoleSniperCnt;
				break;
	}
}

// helper function....
function bool IsValidPlayer( int StatID )
{
	return ( StatID < 0 || StatID > PlayerStats.length ) ? false : true;
}

// Player �� Weapon �� ������....
function PickupWeaponEvent(class<avaWeapon> PickupWeapon, PlayerReplicationInfo PickupPlayer, out int OwnerStatsID)
{
	OwnerStatsID = GetStatsIDFromGlobalID( GetGlobalID(PickupPlayer) );
	AddNewWeapon(OwnerStatsID, PickupWeapon);
}

// Player �� ���� Round �� Spawn Count �� �����ش�.
function int GetCurrentRoundSpawnCount( int StatID )
{
	return PlayerStats[StatID].RoundStats[CurrentRound-1].SpawnCount;
}
//SpawnCount

function int GetWeaponStatID( int OwnerID, class<avaWeapon> weapon )
{
	local int				SpawnCnt;
	local int				i;
	if ( !IsValidPlayer( OwnerID ) )	return -1;
	SpawnCnt = GetCurrentRoundSpawnCount( OwnerID );
	for ( i = 0 ; i < PlayerStats[OwnerID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats.length ; ++ i )
	{	
		if ( PlayerStats[OwnerID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats[i].WeaponType == weapon )
			return i;
	}
	return -1;
}

function int AddNewWeapon(int StatID, class<avaWeapon> WeaponType)
{
	local int				i;
	local int				SpawnCnt;
	if ( !IsValidPlayer( StatID ) )	return -1;
	SpawnCnt = GetCurrentRoundSpawnCount( StatID );
	// SpawnCnt �� 0���� ���ų� �۴ٸ� �����̴�....

	for ( i = 0 ; i < PlayerStats[StatID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats.length ; ++ i )
	{
		if ( WeaponType == PlayerStats[StatID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats[i].WeaponType )
		{
			++PlayerStats[StatID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats[i].nPickUpCnt;
			return i;
		}
	}
	i = PlayerStats[StatID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats.length;
	PlayerStats[StatID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats.length = i + 1;
	PlayerStats[StatID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats[i].WeaponType = WeaponType;
	++PlayerStats[StatID].RoundStats[CurrentRound-1].SpawnStats[SpawnCnt-1].WeaponStats[i].nPickUpCnt;
	return i;
}

// Weapon ���� Event
//		'fire'
//		'hit'
function WeaponEvent(int OwnerStatsID, class<Weapon> weapon, int CurrentFireMode, PlayerReplicationInfo PawnOwner, name Action, optional float Distance )
{
	if ( Action == 'fired' )			TrackFireStat( OwnerStatsID, weapon, 'F' );
	else if ( Action == 'hit' )			TrackFireStat( OwnerStatsID, weapon, 'H' ,, Distance );
	else if ( Action == 'FriendlyHit' )	TrackFireStat( OwnerStatsID, weapon, 'FH',, Distance );
}

function ObjectiveEvent(name StatType, int TeamNo, PlayerReplicationInfo InstigatorPRI, int ObjectiveIndex)
{
	local int GameStatID;
	local int InstigatorStatID;

	InstigatorStatID = GetStatsIDFromGlobalID(GetGlobalID(InstigatorPRI));

	GameStatID = AddGameStat(StatType, TeamNo, InstigatorStatID, ObjectiveIndex);
	AddObjectiveLink(InstigatorStatID,	GameStatID);
}

function TeamScoreEvent( int Team, float Points )
{
	local int i;
	for (i = 0 ; i < PlayerStats.length ;i++ )
	{
		if ( PlayerStats[i].PRI == None )	continue;
		if ( PlayerStats[i].PRI.Team != None && ( PlayerStats[i].PRI.Team.TeamIndex == 0 || PlayerStats[i].PRI.Team.TeamIndex == 1 ))
		{
			if ( PlayerStats[i].PRI.Team.TeamIndex == Team )	
			{
				PlayerStats[i].RoundWinScore += Points;
			}
			else
			{
				PlayerStats[i].RoundLoseScore += Points;
			}
		}
	}
}

defaultproperties
{
	bKeepLocalStatsLog=true
	bKeepRemoteStatsLog=false
	StatSummaryClasses(0)=class'avaStats_LocalSummary'
	StatsRemoteAddress="127.1.1.1"
	CurrentRound=0
}
