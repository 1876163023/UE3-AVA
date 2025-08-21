//=============================================================================
// GameReplicationInfo.
// Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class GameReplicationInfo extends ReplicationInfo
	config(Game)
	native nativereplication;

`include(Core/Globals.uci)

var class<GameInfo> GameClass;				// Class of the server's gameinfo, assigned by GameInfo.

/**
 * The data store instance responsible for presenting state data for the current game session.
 */
var	private		CurrentGameDataStore		CurrentGameData;

var bool bStopCountDown;
var repnotify bool bMatchHasBegun;
var hmserialize repnotify bool bMatchIsOver; // [!] 20070323 dEAthcURe|HM 'hmserialize'
/**
 * Used to determine if the end of match/session clean up is needed. Game invites
 * might have already cleaned up the match/session so doing so again would break
 * the traveling to the invited game
 */
var bool bNeedsOnlineCleanup;
/** Used to determine who handles session ending */
var hmserialize bool bIsArbitrated; // [!] 20070323 dEAthcURe|HM 'hmserialize'

var hmserialize databinding int  RemainingTime, ElapsedTime, RemainingMinute;	// [!] 20070323 dEAthcURe|HM 'hmserialize'
var databinding float SecondCount;
var hmserialize databinding int GoalScore;	// [!] 20070323 dEAthcURe|HM 'hmserialize'
var hmserialize databinding int TimeLimit;	// [!] 20070323 dEAthcURe|HM 'hmserialize'
var hmserialize databinding int MaxLives;	// [!] 20070323 dEAthcURe|HM 'hmserialize'

var databinding TeamInfo Teams[2];

var() hmserialize databinding globalconfig string ServerName;	// [!] 20070323 dEAthcURe|HM 'hmserialize'	// Name of the server, i.e.: Bob's Server.
var() hmserialize databinding globalconfig string ShortName;	// [!] 20070323 dEAthcURe|HM 'hmserialize'	// Abbreviated name of server, i.e.: B's Serv (stupid example)
var() hmserialize databinding globalconfig string AdminName;	// [!] 20070323 dEAthcURe|HM 'hmserialize'	// Name of the server admin.
var() hmserialize databinding globalconfig string AdminEmail;	// [!] 20070323 dEAthcURe|HM 'hmserialize'	// Email address of the server admin.
var() hmserialize databinding globalconfig int	  ServerRegion;	// [!] 20070323 dEAthcURe|HM 'hmserialize'	// Region of the game server.

var() hmserialize databinding globalconfig string MessageOfTheDay;	// [!] 20070323 dEAthcURe|HM 'hmserialize'

var databinding Actor Winner;			// set by gameinfo when game ends

var		array<PlayerReplicationInfo> PRIArray;

/** This list mirror's the GameInfo's list of inactive PRI objects */
var		array<PlayerReplicationInfo> InactivePRIArray;

enum ECarriedObjectState
{
    COS_Home,
    COS_HeldFriendly,
    COS_HeldEnemy,
    COS_Down,
};
var hmserialize ECarriedObjectState CarriedObjectState[2]; // [!] 20070323 dEAthcURe|HM 'hmserialize'

// stats

var hmserialize int MatchID;	// [!] 20070323 dEAthcURe|HM 'hmserialize'
var hmserialize bool bTrackStats; // [!] 20070323 dEAthcURe|HM 'hmserialize'

cpptext
{
	#ifdef EnableHostMigration
	virtual void hmSerialize(FArchive& Ar); // 20061031 dEAthcURe|HM
	#endif
	
	// AActor interface.
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );
}

replication
{
	if ( bNetDirty && (Role == ROLE_Authority) )
		bStopCountDown, Winner, Teams, CarriedObjectState, bMatchHasBegun, bMatchIsOver, MatchID;

	if ( !bNetInitial && bNetDirty && (Role == ROLE_Authority) )
		RemainingMinute;

	if ( bNetInitial && (Role==ROLE_Authority) )
		GameClass,
		RemainingTime, ElapsedTime,MessageOfTheDay,
		ServerName, ShortName, AdminName,
		AdminEmail, ServerRegion, 
		bTrackStats, bIsArbitrated;

	if ( bNetDirty && (Role==ROLE_Authority) )
		GoalScore, MaxLives, TimeLimit;
}


simulated event PostBeginPlay()
{
	local PlayerReplicationInfo PRI;
	if( WorldInfo.NetMode == NM_Client )
	{
		// clear variables so we don't display our own values if the server has them left blank
		ServerName = "";
		AdminName = "";
		AdminEmail = "";
		MessageOfTheDay = "";
	}

	SecondCount = WorldInfo.TimeSeconds;
	SetTimer(WorldInfo.TimeDilation, true);

	WorldInfo.GRI = self;

	// associate this GRI with the "CurrentGame" data store
	InitializeGameDataStore();

	ForEach DynamicActors(class'PlayerReplicationInfo',PRI)
	{
		AddPRI(PRI);
	}
	
	bNeedsOnlineCleanup = true;
}

/* Reset()
reset actor to initial state - used when restarting level without reloading.
*/
function Reset()
{
	Super.Reset();
	Winner = None;
}

/**
 * Called when this actor is destroyed
 */
simulated event Destroyed()
{
	Super.Destroyed();

	// de-associate this GRI with the "CurrentGame" data store
	CleanupGameDataStore();
}

simulated event Timer()
{
	if ( WorldInfo.NetMode == NM_Client )
	{
		ElapsedTime++;
		if ( RemainingMinute != 0 )
		{
			RemainingTime = RemainingMinute;
			RemainingMinute = 0;
		}
		if ( (RemainingTime > 0) && !bStopCountDown )
			RemainingTime--;
		SetTimer(WorldInfo.TimeDilation, true);
	}
}

/**
 * Checks to see if two actors are on the same team.
 *
 * @return	true if they are, false if they aren't
 */
simulated event bool OnSameTeam(Actor A, Actor B)
{
	local byte ATeamIndex, BTeamIndex;

	if ( !GameClass.Default.bTeamGame || (A == None) || (B == None) )
		return false;

	ATeamIndex = A.GetTeamNum();
	if ( ATeamIndex == 255 )
		return false;

	BTeamIndex = B.GetTeamNum();
	if ( BTeamIndex == 255 )
		return false;

	return ( ATeamIndex == BTeamIndex );
}

simulated function PlayerReplicationInfo FindPlayerByID( int PlayerID )
{
    local int i;

    for( i=0; i<PRIArray.Length; i++ )
    {
	if( PRIArray[i].PlayerID == PlayerID )
	    return PRIArray[i];
    }
    return None;
}

simulated function AddPRI(PlayerReplicationInfo PRI)
{
	local int i;

	// Determine whether it should go in the active or inactive list
	if (!PRI.bIsInactive)
	{
		// make sure no duplicates
		for (i=0; i<PRIArray.Length; i++)
		{
			if (PRIArray[i] == PRI)
				return;
		}

		PRIArray[PRIArray.Length] = PRI;
	}
	else
	{
		// Add once only
		if (InactivePRIArray.Find(PRI) == INDEX_NONE)
		{
			InactivePRIArray[InactivePRIArray.Length] = PRI;
		}
	}

    if ( CurrentGameData == None )
    {
    	InitializeGameDataStore();
    }

	if ( CurrentGameData != None )
	{
		CurrentGameData.AddPlayerDataProvider(PRI);
	}
}

simulated function RemovePRI(PlayerReplicationInfo PRI)
{
    local int i;

    for (i=0; i<PRIArray.Length; i++)
    {
		if (PRIArray[i] == PRI)
		{
			if ( CurrentGameData != None )
			{
				CurrentGameData.RemovePlayerDataProvider(PRI);
			}

		    PRIArray.Remove(i,1);
			return;
		}
    }
}

/**
 * Assigns the specified TeamInfo to the location specified.
 *
 * @param	Index	location in the Teams array to place the new TeamInfo.
 * @param	TI		the TeamInfo to assign
 */
simulated function SetTeam( int Index, TeamInfo TI )
{
	if ( Index >= 0 && Index < ArrayCount(Teams) )
	{
        if ( CurrentGameData == None )
        {
    	    InitializeGameDataStore();
        }

		if ( CurrentGameData != None )
		{
			if ( Teams[Index] != None )
			{
				CurrentGameData.RemoveTeamDataProvider( Teams[Index] );
			}

			if ( TI != None )
			{
				CurrentGameData.AddTeamDataProvider(TI);
			}
		}

		Teams[Index] = TI;
	}
}

simulated function GetPRIArray(out array<PlayerReplicationInfo> pris)
{
    local int i;
    local int num;

    pris.Remove(0, pris.Length);
    for (i=0; i<PRIArray.Length; i++)
    {
		if (PRIArray[i] != None)
			pris[num++] = PRIArray[i];
    }
}


simulated function bool InOrder( PlayerReplicationInfo P1, PlayerReplicationInfo P2 )
{
    if( P1.bOnlySpectator )
    {
	if( P2.bOnlySpectator )
	    return true;
	else
	    return false;
    }
    else if ( P2.bOnlySpectator )
	return true;

    if( P1.Score < P2.Score )
	return false;
    if( P1.Score == P2.Score )
    {
		if ( P1.Deaths > P2.Deaths )
			return false;
		if ( (P1.Deaths == P2.Deaths) && (PlayerController(P2.Owner) != None) && (LocalPlayer(PlayerController(P2.Owner).Player) != None) )
			return false;
	}
    return true;
}

simulated function SortPRIArray()
{
    local int i,j;
    local PlayerReplicationInfo tmp;

    for (i=0; i<PRIArray.Length-1; i++)
    {
	for (j=i+1; j<PRIArray.Length; j++)
	{
	    if( !InOrder( PRIArray[i], PRIArray[j] ) )
	    {
		tmp = PRIArray[i];
		PRIArray[i] = PRIArray[j];
		PRIArray[j] = tmp;
	    }
	}
    }
}

simulated event ReplicatedEvent(name VarName)
{
	if ( VarName == 'bMatchHasBegun' )
	{
		if (bMatchHasBegun)
		{
			WorldInfo.NotifyMatchStarted();
			OnlineSession_StartMatch();
		}
	}
	else if ( VarName == 'bMatchIsOver' )
	{
		if ( bMatchIsOver )
		{
			EndGame();	
		}
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

/**
 * Creates and registers a data store for the current game session.
 */
simulated function InitializeGameDataStore()
{
	local DataStoreClient DataStoreManager;

	DataStoreManager = class'UIInteraction'.static.GetDataStoreClient();
	if ( DataStoreManager != None )
	{
		CurrentGameData = CurrentGameDataStore(DataStoreManager.FindDataStore('CurrentGame'));
		if ( CurrentGameData != None )
		{
			CurrentGameData.CreateGameDataProvider(Self);
		}
		else
		{
			`log("Primary game data store not found!");
		}
	}
}

/**
 * Unregisters the data store for the current game session.
 */
simulated function CleanupGameDataStore()
{
	`log(`location,,'DataStore');
	if ( CurrentGameData != None )
	{
		CurrentGameData.ClearDataProviders();
	}

	CurrentGameData = None;
}

/**
 * Called on the server when the match has begin
 *
 * Network - Server and Client (Via ReplicatedEvent)
 */

simulated function StartMatch()
{
	// Notify the Online Session that the Match has begun
	
	if (WorldInfo.NetMode != NM_Standalone && !bMatchHasBegun)
	{
		OnlineSession_StartMatch();
	}

	bMatchHasBegun = true;
	
}

/**
 * Called on the server when the match is over
 *
 * Network - Server and Client (Via ReplicatedEvent)
 */

simulated function EndGame()
{
	bMatchIsOver = true;
	if (WorldInfo.NetMode != NM_StandAlone)
	{
		OnlineSession_EndMatch();
	}
}

/**
 * @Returns a reference to the OnlineGameInterface if one exists. 
 *
 * Network: Client and Server
 */
simulated function OnlineGameInterface GetOnlineGameInterface()
{
	local OnlineSubsystem OnlineSub;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		return OnlineSub.GameInterface;
	}
	else
	{
		`log("Could not get Online SubSystem");
	}
	return none;
}	


/**
 * Signal that this match has begun
 *
 * NETWORK - Both Client and Server
 */

simulated event OnlineSession_StartMatch()
{
	local OnlineGameInterface GI;

	if ( WorldInfo.NetMode != NM_Standalone )
	{
		GI = GetOnlineGameInterface();

		if ( GI != None )
		{
			GI.StartOnlineGame();
		}
	}
	
}


/**
 * Signal that this match is over.  
 *
 * NETWORK - Both Client and Server
 */

simulated event OnlineSession_EndMatch()
{
	local OnlineGameInterface GI;
	GI = GetOnlineGameInterface();

	if ( WorldInfo.NetMode != NM_Standalone )
	{
		// No Stats in Single Player Games
		if ( GI != none )
		{
			// Only clean up if required
			if (bNeedsOnlineCleanup && !bIsArbitrated)
			{
				// Send the end request, which will flush any stats
				GI.EndOnlineGame();
			}
		}
	}
}

/**
 * Signal that this session is over.  Called natively
 *
 * NETWORK - Both Client and Server
 */

simulated event OnlineSession_EndSession(bool bForced)
{
	local OnlineGameInterface GI;
	GI = GetOnlineGameInterface();
	if ( bForced || WorldInfo.NetMode != NM_StandAlone )
	{
		// Only clean up if required
		if (bNeedsOnlineCleanup)
		{
			GI.DestroyOnlineGame();
		}
	}
}

defaultproperties
{
	TickGroup=TG_DuringAsyncWork

	CarriedObjectState[0]=COS_Home
	CarriedObjectState[1]=COS_Home
	bStopCountDown=true
	RemoteRole=ROLE_SimulatedProxy
	bAlwaysRelevant=True
}
