/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaPlayerReplicationInfo extends PlayerReplicationInfo
	native 
	nativereplication;

`include(avaGame/avaGame.uci)

var repnotify bool					bReplicatedByNewHost;		
var repnotify hmserialize int		AccountID;					// server 에서 사용하는 unique id, PlayerReplicationInfo 에서 PlayerID 가 있지만 순서대로 증가하는 값일 뿐이다.
var hmserialize	int					ClanMarkID;					// Clan ID 
var hmserialize int					Level;						// User Level, Server 에서 받아오는 Data 입니다. 게임중에 바뀔일은 없을것이다.

var hmserialize cipher BYTE				RoundWinScore;				// 게임 참여 후 얻은 Win Score
var hmserialize cipher BYTE				RoundLoseScore;				// 게임 참여 후 잃은 Lose Score

enum EPointType
{
    PointType_Attack,
	PointType_Defence,
	PointType_Leader,
	PointType_Tactics,
};

var	hmserialize cipher BYTE				AttackPoint;				// 공격적인 행동을 했을때의 점수
var	hmserialize cipher BYTE				DefencePoint;				// 방어적인 측면의 점수
var hmserialize cipher BYTE				LeaderPoint;				// 분대장의 보너스 점수
var hmserialize cipher BYTE				TacticsPoint;				// 분대 단위의 점수

var hmserialize int					GaugeMax;					// 게이지 max
var hmserialize int					GaugeCur;					// 게이지 현재 값.
var hmserialize	BYTE				CurrentUseActionType;		// Use Action Type...

var hmserialize repnotify BYTE		PlayerClassID;				//	다음 Spawn 될 Class 입니다.
var	hmserialize BYTE				CurrentSpawnClassID;		//	현재 Spawn 되어 있는 Class 입니다. Host 만 알고 있습니다.

// 서버에서 받는 Charcter 에 대한 정보를 담고 있는 String 이다....
var hmserialize string				CharacterItem;				// Character 정보를 가지고 있는 String
var hmserialize string				PointManItem;
var hmserialize string				RifleManItem;
var hmserialize string				SniperItem;

var hmserialize string				WeaponPointMan;				// PointMan Weapon 정보를 가지고 있는 String
var hmserialize string				WeaponRifleMan;				// RifleMan Weapon 정보를 가지고 있는 String
var hmserialize string				WeaponSniperMan;			// SniperMan Weapon 정보를 가지고 있는 String
		
var hmserialize int					LastClass;					//!< 마지막으로 사용한 병과.
var hmserialize int					LastTeam;					//!< 마지막으로 사용한 팀.
var bool							bUpdate;					

var hmserialize bool				bFetchCharacter;			
var hmserialize bool				bFetchPointMan;				
var hmserialize bool				bFetchRifleMan;				
var hmserialize bool				bFetchSniper;				

var hmserialize bool				bFetchWeaponPointMan;		
var hmserialize bool				bFetchWeaponRifleMan;		
var hmserialize bool				bFetchWeaponSniperMan;		

var avaPlayerModifierInfo			avaPMI;

// Vote
var hmserialize bool				bSquadLeader;						// [!] 20070323 dEAthcURe|HM 'hmserialize'	// true이면 Vote로 선출된 분대장(SquadLeader)이다. 디폴트 false
// Localized String For Pawn's Status
var localized string				InvincibleStr;						// 무적 모드
var localized string				SpectatorStr;						// [!] 20070323 dEAthcURe|HM 'hmserialize'
var localized string				WhenDeadStr;						// [!] 20070323 dEAthcURe|HM 'hmserialize'
var localized string				WhenWaitStr;						//
var localized string				PlayerClassStr[`MAX_PLAYER_CLASS];	// [!] 20070323 dEAthcURe|HM 'hmserialize'	//
var	avaClassReplicationInfo			avaCRI[`MAX_PLAYER_CLASS];			// Class 별 Replication 되어야 하는 정보들

var hmserialize string				StatusStr;							// [!] 20070323 dEAthcURe|HM 'hmserialize'	// Weapon 을 가지고 있을 경우에 상태창에 표시되는 String

var	hmserialize bool				bReadyInGame;	// [!] 20070323 dEAthcURe|HM 'hmserialize'	// Game 을 할 준비가 완료되었음...
																	// Client 의 Controller 가 PlayerReplication 정보를 받았음 정도의 의미
var hmserialize int					TeamKillCnt;	// [!] 20070323 dEAthcURe|HM 'hmserialize'
var	bool							bHost;									// True 이면 호스트임...

// 정산 관련...
var	cipher byte				RoundKillCnt;							// Round 별 Kill Count - Round Reset 시 Reset 됨
var	cipher byte				RoundHeadShotKillCnt;					// Round 별 Head shot Kill Count - Round Reset 시 Reset 됨
var int						RoundDamage;							// Round 별 입은 Damage - Round Reset 시 Reset 됨
var cipher byte				RoundScore;								// Round 별 Score - Round Reset 시 Reset 됨
var cipher byte				RoundDeathCnt;							// Round 별 Death Count
var float					LastKillTime;							// 마지막으로 Kill 한 Time

var cipher byte					RoundTopKillCnt;						// Round 에서 가장 높은 Kill 을 한 횟수
var cipher byte					RoundTopHeadShotKillCnt;				// Round 에서 가장 높은 HeadShot Kill 을 한 횟수
var	cipher byte					RoundNoDamageCnt;						// Damage 를 입지 않고 Round 를 마친 Count
var cipher byte					RoundTopScoreCnt;						// Round 에서 가장 높은 Score 을 올린 횟수
var	cipher byte					RoundNoDeathCnt;						// Round 에서 킬 당하지 않은 횟수
var	cipher byte					TopLevelKillCnt;						// 상대팀의 가장 상위 Level 의 User 를 죽인 Count
var cipher byte					HigherLevelKillCnt;						// 자신보다 높은 Level 의 User 를 죽인 Count
var	cipher byte					BulletMultiKillCnt;						// 총으로 Multi Kill 을 한 Count
var cipher byte					GrenadeMultiKillCnt;					// 수류탄으로 Multi Kill 을 한 Count
var	cipher byte					HelmetDropCnt;							// Helmet 을 Drop 시킨 횟수


var	hmserialize	int			LeaderScore;							// 분대장 선정을 위한 Leader Score
var	hmserialize	bool		bSilentLogIn;							// Admin Login

var string					LocationName;
var	int						SlotNum;								// 대기방에서의 Slot Index... 대회채널에서 번호를 부여하기 위해서 사용된다...


var	hmserialize	int			LastDeathTime;


cpptext
{
	#ifdef EnableHostMigration
	virtual void hmSerialize(FArchive& Ar); // 20061031 dEAthcURe|HM
	#endif
	
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );
}

replication
{
	if ( Role==ROLE_Authority )
		bSquadLeader, StatusStr; 

	if ( bNetDirty && ROLE==ROLE_Authority )
		bReplicatedByNewHost, // [+] 20070523 dEAthcURe 'bReplicatedByNewHost'
		GaugeMax, GaugeCur, PlayerClassID, 
		//CharacterItem,PointManItem,RifleManItem,SniperItem,
		//WeaponPointMan,WeaponRifleMan,WeaponSniperMan, 
		AccountID, Level,
		AttackPoint, DefencePoint, LeaderPoint, TacticsPoint,
		RoundWinScore, RoundLoseScore, avaCRI, TeamKillCnt, CurrentSpawnClassID, bHost,
		RoundTopKillCnt,
		RoundTopHeadShotKillCnt,
		RoundNoDamageCnt,
		RoundTopScoreCnt,
		RoundNoDeathCnt,
		TopLevelKillCnt,
		HigherLevelKillCnt,
		BulletMultiKillCnt,
		GrenadeMultiKillCnt,
		HelmetDropCnt,
		bSilentLogIn,
		LeaderScore,
		LastDeathTime,
		CurrentUseActionType;
}

simulated function PostBeginPlay()
{
	local int i;
	avaPMI = Spawn(class'avaPlayerModifierInfo',self);
	// ClassReplicationInfo 를 만들어 주자!
	if ( Role == ROLE_Authority )
	{
		for ( i = 0 ; i < `MAX_PLAYER_CLASS ; ++ i )
		{
			avaCRI[i] = Spawn( class'avaClassReplicationInfo', self );
		}
	}
	Super.PostBeginPlay();
}

// Score 를 Clear 한다....
function ClearScore()
{
	local int i;
	Score			= 0;
	Kills			= 0;
	Deaths			= 0;
	RoundWinScore	= 0;
	RoundLoseScore	= 0;
	AttackPoint		= 0;
	DefencePoint	= 0;
	LeaderPoint		= 0;
	TacticsPoint	= 0;
	TeamKillCnt		= 0;
	for ( i = 0 ; i < `MAX_PLAYER_CLASS ; ++ i )
	{
		avaCRI[i].ClearScore();
	}
}

simulated event ReplicatedEvent(name VarName)
{
	local Pawn				P;
	local PlayerController	PC;
	local Actor				A;
	local int				i;
	local avaNetHandler		NetHandler;

	if ( VarName == 'Team' )
	{
		ForEach DynamicActors(class'Pawn', P)
		{
			// find my pawn and tell it
			if ( P.PlayerReplicationInfo == self )
			{
				P.NotifyTeamChanged();
				break;
			}
		}
		ForEach LocalPlayerControllers(PC)
		{
			if ( PC.PlayerReplicationInfo == self )
			{
				avaPlayerController( PC ).SetTeam( Team.TeamIndex );
				ForEach AllActors(class'Actor', A)
					A.NotifyLocalPlayerTeamReceived();					
				
				updatePickupIndicators(); // 20070525 dEAthcURe|HM update pickup indicator
			}
			break;
		}
	}
	else if ( VarName == 'AccountID' )
	{
		NetHandler		=	class'avaNetHandler'.static.GetAvaNetHandler();
		CharacterItem	=	NetHandler.GetURLString( AccountID, "ChItem");
		PointManItem	=	NetHandler.GetURLString( AccountID, "SkillP");
		RifleManItem	=	NetHandler.GetURLString( AccountID, "SkillR");
		SniperItem		=	NetHandler.GetURLString( AccountID, "SkillS");
		WeaponPointMan	=	NetHandler.GetURLString( AccountID, "WeaponP");
		WeaponRifleMan	=	NetHandler.GetURLString( AccountID, "WeaponR");
		WeaponSniperMan	=	NetHandler.GetURLString( AccountID, "WeaponS");
		SlotNum			=	NetHandler.GetPlayerRoomSlot( AccountID );
		SlotNum			=	SlotNum % 12;
		if ( SlotNum < 0 )
			SlotNum = 0;
		FetchCharacterModifier();
		for ( i = 0 ; i < 3 ; ++ i )
		{
			FetchCharacterModifier(i);
			FetchWeaponModifier(i);
		}
		ClanMarkID		=	NetHandler.GetClanMarkID( AccountID );
	}
	//else if ( VarName == 'CharacterItem' )		FetchCharacterModifier();
	//else if ( VarName == 'PointManItem' )		FetchCharacterModifier(0);
	//else if ( VarName == 'RifleManItem' )		FetchCharacterModifier(1);
	//else if ( VarName == 'SniperItem' )			FetchCharacterModifier(2);	
	//else if ( VarName == 'WeaponPointMan' )		FetchWeaponModifier( 0 );
	//else if ( VarName == 'WeaponRifleMan' )		FetchWeaponModifier( 1 );
	//else if ( VarName ==  'WeaponSniperMan' )	FetchWeaponModifier( 2 );
	else if ( VarName == 'PlayerClassID' )		
	{
		UpdatePlayerClassID();
	}
	else if ( VarName == 'PlayerName' )
	{
		OldName = PreviousName;
		PreviousName = PlayerName;
	}
	else if ( VarName == 'bOnlySpectator' )
	{
		//`log( "avaPlayerReplicationInfo.ReplicatedEvent Call UpdateSpectatorInof" @VarName );
		//UpdateSpectatorInfo();
	}
	// {{ 20070523 dEAthcURe|HM
	else if ( VarName == 'bReplicatedByNewHost' )
	{		
		onReplicationCompleteFromNewHost();
	}
	else if ( VarName == 'bOutOfLives' )
	{
		ForEach LocalPlayerControllers(PC)
		{
			if ( PC.PlayerReplicationInfo == self )
				avaPlayerController( PC ).NotifyOutOfLives( bOutOfLives );
		}
	}
	else if ( VarName == 'PlayerLocationHint' )
	{
		UpdateLocationName();
	}
	// }} 20070523 dEAthcURe|HM
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

simulated function UpdatePlayerClassID()
{
	if ( PlayerController(Owner).myHUD != None )
		avaHUD( PlayerController(Owner).myHUD ).ActivateClassSelectScene();
}

simulated function UpdateSpectatorInfo()
{
	if ( PlayerController(Owner).myHUD != None )
		avaHUD( PlayerController(Owner).myHUD ).UpdateSpectatorInfo( bOnlySpectator );

}

//! 캐릭터 정보를 갱신해야 하는 경우가 있어서 추가.(2007/02/07 고광록).
simulated function ResetCharacterModifiers()
{
	bFetchCharacter	= false;
	bFetchPointMan	= false;
	bFetchRifleMan	= false;
	bFetchSniper	= false;

	avaPMI.ResetCharacterModifiers();
}

simulated function FetchCharacterModifier( optional int classType = -1 )
{
	local bool		bFetch;
	local string	Code;
	
	if ( WorldInfo.GRI == None )	return;
	
	switch( classType )
	{
	case -1:	bFetch	= bFetchCharacter;	Code	= CharacterItem;	break;
	case  0:	bFetch	= bFetchPointMan;	Code	= PointManItem;		break;	
	case  1:	bFetch	= bFetchRifleMan;	Code	= RifleManItem;		break;
	case  2:	bFetch	= bFetchSniper;		Code	= SniperItem;		break;
	}

	if ( bFetch == true || Code == "" )		return;

	avaPMI.FetchCharMod( Code, ClassType );

	switch( classType )
	{
	case -1:	bFetchCharacter = true;	break;
	case  0:	bFetchPointMan	= true;	break;
	case  1:	bFetchRifleMan	= true;	break;
	case  2:	bFetchSniper	= true;	break;
	}
}

//! 무기 정보를 갱신해야 하는 경우가 있어서 추가.(2007/02/07 고광록).
simulated function ResetWeaponModifiers()
{
	bFetchWeaponPointMan  = false;
	bFetchWeaponRifleMan  = false;
	bFetchWeaponSniperMan = false;

	avaPMI.ResetWeaponModifiers();
}

simulated function FetchWeaponModifier( int i )
{
	local string	Code;
	if ( WorldInfo.GRI != None )
	{
		if ( i == 0 )
		{
			if ( bFetchWeaponPointMan == true || WeaponPointMan == "" )		return;
			Code = WeaponPointMan;
			bFetchWeaponPointMan = true;
		}
		else if ( i == 1 )
		{
			if ( bFetchWeaponRifleMan == true || WeaponRifleMan == "" )		return;
			Code = WeaponRifleMan;
			bFetchWeaponRifleMan = true;
		}
		else if ( i == 2 )
		{
			if ( bFetchWeaponSniperMan == true || WeaponSniperMan == "" )	return;
			Code = WeaponSniperMan;
			bFetchWeaponSniperMan = true;
		}
		avaPMI.FetchWeaponMod( Code, i );
	}
}

simulated function DisplayDebug(HUD HUD, out float YL, out float YPos)
{
	HUD.Canvas.SetDrawColor(255,0,0);
	HUD.Canvas.SetPos(4, YPos);
    HUD.Canvas.Font	= class'Engine'.Static.GetSmallFont();
	HUD.Canvas.DrawText(" bOutOfLives : "$bOutOfLives$ "NumLives : "$NumLives );
	YPos += YL;
	Super.DisplayDebug( HUD, YL, YPos );
}

function UpdateCharacter();

/* epic ===============================================
* ::OverrideWith
Get overridden properties from old PRI
*/
function OverrideWith(PlayerReplicationInfo PRI)
{
	local avaPlayerReplicationInfo avaPRI;

	Super.OverrideWith(PRI);
	
	avaPRI = avaPlayerReplicationInfo(PRI);
	if ( avaPRI == None )
		return;
}

/* epic ===============================================
* ::CopyProperties
Copy properties which need to be saved in inactive PRI
*/
function CopyProperties(PlayerReplicationInfo PRI)
{
	//local avaPlayerReplicationInfo avaPRI;
	//Super.CopyProperties(PRI);
	//avaPRI = avaPlayerReplicationInfo(PRI);
	//if ( avaPRI == None )
	//	return;
}

function SetUseActionType( EUseActionType ActionType )
{
	`log( "avaPlayerReplicationInfo.SetUseActionType" @ActionType );
	CurrentUseActionType = ActionType;
}

`devexec simulated function SetGauge( int ncur, int nmax )
{
	if ( nmax != GaugeMax )
		GaugeMax = nmax;
	if ( ncur < 0 )
		ncur = 0;
	if ( ncur > nmax )
		ncur = nmax;
	if ( ncur != GaugeCur )
	{
		GaugeCur = ncur;
		if ( nCur == 0 )
			SetUseActionType( UAT_None );
	}
	
	NetUpdateTime = WorldInfo.TimeSeconds - 1;
}

simulated function bool IsValidClassType()
{
	if ( PlayerClassID >= 0 && PlayerClassID < `MAX_PLAYER_CLASS )
		return true;
	else
		return false;
}

simulated function int GetPlayerClassID()
{
	return PlayerClassID;
}

reliable server function SetPlayerClassID( int nID)
{
	PlayerClassID = nID;
	NetUpdateTime = WorldInfo.TimeSeconds - 1;
	UpdatePlayerClassID();
}

simulated function avaPawn GetAvaPawn()
{
	local Pawn temp;
	ForEach DynamicActors(class'Pawn', temp)
	{
		if ( temp.PlayerReplicationInfo == self )
		{
			if ( avaPawn( temp ) != None )
				return avaPawn( temp );
			else if ( Vehicle( temp ) != None )
				return avaPawn( Vehicle( temp ).Driver );
		}
	}
	return None;
}

simulated function CreateWeaponTemp( avaPawn Pawn )
{
	local int			i;
	local Inventory		Inv;
	local bool			bActive;
	
	bActive = WorldInfo.Game.bMigratedHost ? true : false;

	for ( i = 0 ; i < Pawn.default.DefaultWeapons.length ; ++ i )
	{
		Inv = Spawn( Pawn.default.DefaultWeapons[i] );
		if ( Inv == None )	continue;
		if ( avaWeap_BaseGun( Inv ) != None )
		{
			avaWeap_BaseGun( Inv ).ReloadCnt = avaWeap_BaseGun( Inv ).ClipCnt;
		}
		avaWeapon( Inv ).AmmoCount= avaWeapon( Inv ).MaxAmmoCount;
		avaWeapon( Inv ).SetMaintenanceRate( 30 );
		Pawn.InvManager.AddInventory( Inv, bActive );
		bActive = true;
	}
}

// 생성한 Pawn 을 위하여 Weapon 들을 만들어준다...
//simulated function CreateWeapon( avaPawn Pawn )
//{
//	local int					i,j;
//	local int					ChrTypeId;
//	local Inventory				Inv;
//	local avaWeapon				avaWeap;
//	local int					CreateWeaponCnt;
//	local bool					bDoNotActivateWeapon;
//	local array< WeaponInfo >	WeaponInfos;
//
//	ChrTypeId = Pawn.GetTypeId();
//
//	bDoNotActivateWeapon = WorldInfo.Game.bMigratedHost ? true : false; // [!] 20070213 dEAthcURe|HM // bDoNotActivateWeapon = false;
//
//	WeaponInfos = avaPMI.ClassTypeInfos[ChrTypeId].WeaponInfos;
//
//	for ( i = 0 ; i < WeaponInfos.length ; ++ i )
//	{
//		if ( WeaponInfos[i].Class == None )	continue;
//
//		++ CreateWeaponCnt;
//		Inv = Spawn( WeaponInfos[i].Class );
//		avaWeap = avaWeapon( Inv );
//
//		avaWeap.MaxAmmoCount *= Pawn.WeapTypeAmp[avaWeap.WeaponType].AmmoAmp;
//		avaWeap.AmmoCount = avaWeap.MaxAmmoCount;
//
//		// Add Weapon Modifier...
//		for ( j = 0 ; j < WeaponInfos[i].Mod.length ; ++ j )
//			avaWeap.AddWeaponModifier( WeaponInfos[i].Mod[j] );
//
//		avaWeap.WeaponModifierDone();
//		Pawn.InvManager.AddInventory( Inv, bDoNotActivateWeapon );
//		bDoNotActivateWeapon = true;
//	}
//
//	if ( CreateWeaponCnt == 0 )
//		CreateWeaponTemp( Pawn );
//}

/* Strategy Message 관련 */
simulated function bool IsSquadLeader()
{
	return bSquadLeader;
}

function AddTeamKillCount()
{
	++TeamKillCnt;
}

function AddDeathCount()
{
	if ( avaGameReplicationInfo( WorldInfo.GRI ).bWarmupRound )	return; 
	++RoundDeathCnt;
	Deaths += 1;
	Deaths = Clamp( Deaths, 0, `MAX_SCORE );
	//if ( CurrentSpawnClassID >= 0 && CurrentSpawnClassID < `MAX_PLAYER_CLASS )
	//	avaCRI[CurrentSpawnClassID].AddDeathCount();
}

function int GetAvailableScore( int nScore )
{
	local int nRemain;
	nRemain = Score + nScore - `MAX_SCORE;
	if ( nRemain > 0 )
		nScore -= nRemain;
	return nScore;
}

function int GetAvailablePoint( EPointType Type, int nScore )
{
	local float	CapRate;
	CapRate = avaGame(WorldInfo.Game).GetScoreCapRate();		// Game Rule 에 의한 Score Capacity 변화...
	switch ( Type )
	{
	case PointType_Attack	:	return GetAvailableEachPoint( AttackPoint,	`MAX_ATTACKSCORE * CapRate,		nScore );	break;
	case PointType_Defence	:	return GetAvailableEachPoint( DefencePoint, `MAX_DEFENCESCORE * CapRate,	nScore );	break;
	case PointType_Leader	:	return GetAvailableEachPoint( LeaderPoint,	`MAX_LEADERSCORE * CapRate,		nScore );	break;
	case PointType_Tactics	:	return GetAvailableEachPoint( TacticsPoint, `MAX_TACTICSSCORE * CapRate,	nScore );	break;
	}
	return 0;
}

function int GetAvailableEachPoint( int nCur, int nMax, int nScore )
{
	local int nRemain;
	nRemain = nCur + nScore - nMax;
	if ( nRemain > 0 )
		nScore -= nRemain;
	return nScore;
}

function AddScore( int nScore )
{
	if ( avaGameReplicationInfo( WorldInfo.GRI ).bWarmupRound )	return; 
	nScore = GetAvailableScore( nScore );
	Score = Clamp( Score + nScore, 0, `MAX_SCORE );
	RoundScore += nScore;
	avaGame( WorldInfo.Game ).CheckPrivateScore( avaPlayerController( Owner ), Score );
}

function AddPoint( EPointType Type, int nScore )
{
	local float	CapRate;
	CapRate = avaGame(WorldInfo.Game).GetScoreCapRate();		// Game Rule 에 의한 Score Capacity 변화...

	if ( avaGameReplicationInfo( WorldInfo.GRI ).bWarmupRound )	return; 

	nScore = GetAvailableScore( nScore );
	if ( WorldInfo.Game.bTeamGame )
		nScore = avaTeamInfo( Team ).GetAvailableScore( Type, nScore );
	nScore = GetAvailablePoint( Type, nScore );

	if( nScore <= 0 )	return;

	AddScore( nScore );
	
	if ( WorldInfo.Game.bTeamGame )
		avaTeamInfo( Team ).AddPoint( Type, nScore );

	switch ( Type )
	{
	case PointType_Attack	:	AttackPoint		= Clamp( AttackPoint  + nScore, 0, `MAX_ATTACKSCORE	* CapRate );	break;
	case PointType_Defence	:	DefencePoint	= Clamp( DefencePoint + nScore, 0, `MAX_DEFENCESCORE * CapRate );	break;
	case PointType_Leader	:	LeaderPoint		= Clamp( LeaderPoint  + nScore, 0, `MAX_LEADERSCORE	* CapRate );	break;
	case PointType_Tactics	:	TacticsPoint	= Clamp( TacticsPoint + nScore, 0, `MAX_TACTICSSCORE * CapRate );	break;
	}
}

function int GetPoint( EPointType Type )
{
	switch ( Type )
	{
	case PointType_Attack	:	return AttackPoint;
	case PointType_Defence	:	return DefencePoint;
	case PointType_Leader	:	return LeaderPoint;
	case PointType_Tactics	:	return TacticsPoint;
	}
	return 0;
}

function AddHelmetDropCnt()
{
	++HelmetDropCnt;
}

function AddKillCount( int weaponType, bool bHeadShot, avaPlayerReplicationInfo KilledPRI )
{
	local int TopLevel;
	local avaGameReplicationInfo avaGRI;

	avaGRI = avaGameReplicationInfo( WorldInfo.GRI );
	if ( avaGRI.bWarmupRound )	return; 

	if ( Level < KilledPRI.Level )	++HigherLevelKillCnt;

	// Top Level Kill Count - TeamGame일 경우에만 Check 하도록 한다...
	if ( WorldInfo.Game.bTeamGame == true )
	{
		if ( avaGRI.TopLevelPRI[KilledPRI.Team.TeamIndex] != None )
			TopLevel = avaPlayerReplicationInfo( avaGRI.TopLevelPRI[KilledPRI.Team.TeamIndex] ).Level;
		if ( TopLevel > 1 && TopLevel == KilledPRI.Level )	
			++TopLevelKillCnt;
	}

	++RoundKillCnt;
	if ( bHeadShot )	
		++RoundHeadShotKillCnt;

	if ( CurrentSpawnClassID >= 0 && CurrentSpawnClassID < `MAX_PLAYER_CLASS && Score < `MAX_SCORE )
	{
		avaCRI[CurrentSpawnClassID].AddKillCount( weaponType, bHeadShot );
	}

	if ( LastKillTime == WorldInfo.TimeSeconds )
	{
		if ( weaponType == WEAPON_PISTOL ||
			 weaponType == WEAPON_SMG ||
			 weaponType == WEAPON_RIFLE ||
			 weaponType == WEAPON_SNIPER ||
			 weaponType == WEAPON_SHOTGUN )
			 ++BulletMultiKillCnt;

		if ( weaponType == WEAPON_GRENADE )
			++GrenadeMultiKillCnt;
	}

	LastKillTime = WorldInfo.TimeSeconds;
}

simulated function AddFireCount( int weaponType )
{
	if ( !avaGame( WorldInfo.Game ).IsValidRoundState() )	return;
	if ( CurrentSpawnClassID >= 0 && CurrentSpawnClassID < `MAX_PLAYER_CLASS )
		avaCRI[CurrentSpawnClassID].AddFireCount( weaponType );
}

function AddHitCount( int weaponType, int Damage, bool bHeadHit )
{
	if ( !avaGame( WorldInfo.Game ).IsValidRoundState() )	return;
	if ( CurrentSpawnClassID >= 0 && CurrentSpawnClassID < `MAX_PLAYER_CLASS )
		avaCRI[CurrentSpawnClassID].AddHitCount( weaponType, Damage, bHeadHit );
}

function DamageTaken( int Damage )
{
	if ( !avaGame( WorldInfo.Game ).IsValidRoundState() )	return;
	++RoundDamage;
	if ( CurrentSpawnClassID >= 0 && CurrentSpawnClassID < `MAX_PLAYER_CLASS )
		avaCRI[CurrentSpawnClassID].DamageTaken( Damage );
}

function SpawnStart( int nClassType )
{
	if ( !avaGame( WorldInfo.Game ).IsValidRoundState() )			return;
	// Round Time Over Do not Count Play Time
	if ( avaGameReplicationInfo( WorldInfo.GRI ).bRoundTimeOver )	return; 
	CurrentSpawnClassID	= nClassType;
	if ( CurrentSpawnClassID >= 0 && CurrentSpawnClassID < `MAX_PLAYER_CLASS )
		avaCRI[CurrentSpawnClassID].SpawnStart();
}

function SpawnEnd()
{
	if ( !avaGame( WorldInfo.Game ).IsValidRoundState() )	return;
	if ( CurrentSpawnClassID >= 0 && CurrentSpawnClassID < `MAX_PLAYER_CLASS )
		avaCRI[CurrentSpawnClassID].SpawnEnd();
	SprintStat( false );
}

function SprintStat( bool bStart )
{
	if ( !avaGame( WorldInfo.Game ).IsValidRoundState() )	return;
	if ( bStart == true && avaGameReplicationInfo( WorldInfo.GRI ).bRoundTimeOver )	return; 
	if ( CurrentSpawnClassID >= 0 && CurrentSpawnClassID < `MAX_PLAYER_CLASS )
		avaCRI[CurrentSpawnClassID].SprintStat( bStart );
}

function SetWeaponStatusStr( string str )
{
	StatusStr = str;	
}

// {{ 20070523 dEAthcURe|HM 새 호스트로부터 replication이 끝나면 불린다

simulated function updatePickupIndicators()
{
	local avaPickUp		PickUp;
	ForEach DynamicActors(class'avaPickUp', PickUp) {		
		PickUp.CreateIndicator();
		PickUp.CheckIndicatorTeam();
	}
}

simulated function onReplicationCompleteFromNewHost()
{
}
// }} 20070523 dEAthcURe|HM 새 호스트로부터 replication이 끝나면 불린다

function bool UpdatePlayerLocation()
{
	local bool bUpdated;
	bUpdated = Super.UpdatePlayerLocation();
	
	if ( bUpdated == true )
		UpdateLocationName();

	return bUpdated;
}

simulated function UpdateLocationName()
{
	local string TempLocationName;
	if( PlayerLocationHint == None )
	{
		TempLocationName = StringSpectating;
	}
	else if ( Volume( PlayerLocationHint ) != None )
	{
		TempLocationName = GetLocationNameByID( Volume( PlayerLocationHint ).GetLocationNameIndex() );
	}
	if ( TempLocationName == "" )
	{
		TempLocationName = PlayerLocationHint.GetLocationStringFor(self);
	}
	if ( TempLocationName == "" )
		TempLocationName = StringUnknown;
	LocationName	=	TempLocationName;
}

simulated function string GetLocationNameByID( int nID )
{
	local int index;
	if ( nID < 0 )	return "";
	index = class'avaLocalizedMessage'.default.LocationNameList.Find( 'id', nID );
	if ( index == INDEX_NONE )	return "";
	return class'avaLocalizedMessage'.default.LocationNameList[index].LocationName;
}

simulated function string GetLocationName()
{
	return LocationName;
}

simulated function int GetClanMarkID()
{
	return ClanMarkID;
}

defaultproperties
{
	bSquadLeader		=	false
	GameMessageClass	=	class'avaGameMessage'
	bUpdate				=	false
}
