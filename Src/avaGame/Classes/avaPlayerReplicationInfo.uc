/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaPlayerReplicationInfo extends PlayerReplicationInfo
	native 
	nativereplication;

`include(avaGame/avaGame.uci)

var repnotify bool					bReplicatedByNewHost;		
var repnotify hmserialize int		AccountID;					// server ���� ����ϴ� unique id, PlayerReplicationInfo ���� PlayerID �� ������ ������� �����ϴ� ���� ���̴�.
var hmserialize	int					ClanMarkID;					// Clan ID 
var hmserialize int					Level;						// User Level, Server ���� �޾ƿ��� Data �Դϴ�. �����߿� �ٲ����� �������̴�.

var hmserialize cipher BYTE				RoundWinScore;				// ���� ���� �� ���� Win Score
var hmserialize cipher BYTE				RoundLoseScore;				// ���� ���� �� ���� Lose Score

enum EPointType
{
    PointType_Attack,
	PointType_Defence,
	PointType_Leader,
	PointType_Tactics,
};

var	hmserialize cipher BYTE				AttackPoint;				// �������� �ൿ�� �������� ����
var	hmserialize cipher BYTE				DefencePoint;				// ������� ������ ����
var hmserialize cipher BYTE				LeaderPoint;				// �д����� ���ʽ� ����
var hmserialize cipher BYTE				TacticsPoint;				// �д� ������ ����

var hmserialize int					GaugeMax;					// ������ max
var hmserialize int					GaugeCur;					// ������ ���� ��.
var hmserialize	BYTE				CurrentUseActionType;		// Use Action Type...

var hmserialize repnotify BYTE		PlayerClassID;				//	���� Spawn �� Class �Դϴ�.
var	hmserialize BYTE				CurrentSpawnClassID;		//	���� Spawn �Ǿ� �ִ� Class �Դϴ�. Host �� �˰� �ֽ��ϴ�.

// �������� �޴� Charcter �� ���� ������ ��� �ִ� String �̴�....
var hmserialize string				CharacterItem;				// Character ������ ������ �ִ� String
var hmserialize string				PointManItem;
var hmserialize string				RifleManItem;
var hmserialize string				SniperItem;

var hmserialize string				WeaponPointMan;				// PointMan Weapon ������ ������ �ִ� String
var hmserialize string				WeaponRifleMan;				// RifleMan Weapon ������ ������ �ִ� String
var hmserialize string				WeaponSniperMan;			// SniperMan Weapon ������ ������ �ִ� String
		
var hmserialize int					LastClass;					//!< ���������� ����� ����.
var hmserialize int					LastTeam;					//!< ���������� ����� ��.
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
var hmserialize bool				bSquadLeader;						// [!] 20070323 dEAthcURe|HM 'hmserialize'	// true�̸� Vote�� ����� �д���(SquadLeader)�̴�. ����Ʈ false
// Localized String For Pawn's Status
var localized string				InvincibleStr;						// ���� ���
var localized string				SpectatorStr;						// [!] 20070323 dEAthcURe|HM 'hmserialize'
var localized string				WhenDeadStr;						// [!] 20070323 dEAthcURe|HM 'hmserialize'
var localized string				WhenWaitStr;						//
var localized string				PlayerClassStr[`MAX_PLAYER_CLASS];	// [!] 20070323 dEAthcURe|HM 'hmserialize'	//
var	avaClassReplicationInfo			avaCRI[`MAX_PLAYER_CLASS];			// Class �� Replication �Ǿ�� �ϴ� ������

var hmserialize string				StatusStr;							// [!] 20070323 dEAthcURe|HM 'hmserialize'	// Weapon �� ������ ���� ��쿡 ����â�� ǥ�õǴ� String

var	hmserialize bool				bReadyInGame;	// [!] 20070323 dEAthcURe|HM 'hmserialize'	// Game �� �� �غ� �Ϸ�Ǿ���...
																	// Client �� Controller �� PlayerReplication ������ �޾��� ������ �ǹ�
var hmserialize int					TeamKillCnt;	// [!] 20070323 dEAthcURe|HM 'hmserialize'
var	bool							bHost;									// True �̸� ȣ��Ʈ��...

// ���� ����...
var	cipher byte				RoundKillCnt;							// Round �� Kill Count - Round Reset �� Reset ��
var	cipher byte				RoundHeadShotKillCnt;					// Round �� Head shot Kill Count - Round Reset �� Reset ��
var int						RoundDamage;							// Round �� ���� Damage - Round Reset �� Reset ��
var cipher byte				RoundScore;								// Round �� Score - Round Reset �� Reset ��
var cipher byte				RoundDeathCnt;							// Round �� Death Count
var float					LastKillTime;							// ���������� Kill �� Time

var cipher byte					RoundTopKillCnt;						// Round ���� ���� ���� Kill �� �� Ƚ��
var cipher byte					RoundTopHeadShotKillCnt;				// Round ���� ���� ���� HeadShot Kill �� �� Ƚ��
var	cipher byte					RoundNoDamageCnt;						// Damage �� ���� �ʰ� Round �� ��ģ Count
var cipher byte					RoundTopScoreCnt;						// Round ���� ���� ���� Score �� �ø� Ƚ��
var	cipher byte					RoundNoDeathCnt;						// Round ���� ų ������ ���� Ƚ��
var	cipher byte					TopLevelKillCnt;						// ������� ���� ���� Level �� User �� ���� Count
var cipher byte					HigherLevelKillCnt;						// �ڽź��� ���� Level �� User �� ���� Count
var	cipher byte					BulletMultiKillCnt;						// ������ Multi Kill �� �� Count
var cipher byte					GrenadeMultiKillCnt;					// ����ź���� Multi Kill �� �� Count
var	cipher byte					HelmetDropCnt;							// Helmet �� Drop ��Ų Ƚ��


var	hmserialize	int			LeaderScore;							// �д��� ������ ���� Leader Score
var	hmserialize	bool		bSilentLogIn;							// Admin Login

var string					LocationName;
var	int						SlotNum;								// ���濡���� Slot Index... ��ȸä�ο��� ��ȣ�� �ο��ϱ� ���ؼ� ���ȴ�...


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
	// ClassReplicationInfo �� ����� ����!
	if ( Role == ROLE_Authority )
	{
		for ( i = 0 ; i < `MAX_PLAYER_CLASS ; ++ i )
		{
			avaCRI[i] = Spawn( class'avaClassReplicationInfo', self );
		}
	}
	Super.PostBeginPlay();
}

// Score �� Clear �Ѵ�....
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

//! ĳ���� ������ �����ؾ� �ϴ� ��찡 �־ �߰�.(2007/02/07 ����).
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

//! ���� ������ �����ؾ� �ϴ� ��찡 �־ �߰�.(2007/02/07 ����).
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

// ������ Pawn �� ���Ͽ� Weapon ���� ������ش�...
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

/* Strategy Message ���� */
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
	CapRate = avaGame(WorldInfo.Game).GetScoreCapRate();		// Game Rule �� ���� Score Capacity ��ȭ...
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
	CapRate = avaGame(WorldInfo.Game).GetScoreCapRate();		// Game Rule �� ���� Score Capacity ��ȭ...

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

	// Top Level Kill Count - TeamGame�� ��쿡�� Check �ϵ��� �Ѵ�...
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

// {{ 20070523 dEAthcURe|HM �� ȣ��Ʈ�κ��� replication�� ������ �Ҹ���

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
// }} 20070523 dEAthcURe|HM �� ȣ��Ʈ�κ��� replication�� ������ �Ҹ���

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
