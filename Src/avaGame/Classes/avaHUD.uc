/**
 * avaHUD
 * ava Heads Up Display
 *
 *
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaHUD extends HUD
	config(Game)
	dependson(avaGameReplicationInfo)
	native;

`include(avaGame/avaGame.uci)

cpptext
{
	virtual VOID	TickSpecial(FLOAT DeltaTime);
}

var() globalconfig bool bCrosshairShow;
var array<Actor>		PostRenderedActors;
var bool				BombAlertBlink;
var bool				BombInstalled;
var float				LastAmmoPickUpTime, LastPickUpTime, LastHealthPickUpTime;
var	Actor				PrvViewTarget;
var bool				bInitFirstViewTarget;
var bool				bSeeThrough;

enum SceneControlGroupID
{
	GROUP_NONE,							// 0
	CHATBOX,							// 1
	TEAMCHATBOX,						// 2
	DEATHKILLCAM,						// 3
	DEATHEFFECT,						// 4
	RADAR,								// 5
	THROWABLEWEAPON,					// 6
	KILLICON,							// 7
	PROGRESSBAR,						// 8
	TARGETNAME,							// 9
	HUDCLOCK,							//10
	HEALTHGAUGE,						//11
	NVGGAUGE,							//12
	TOUCHEDWEAPON,						//13
	CROSSHAIR,							//14
	KILLCAM,							//15
	CHATCAM,							//16
	DAMAGEINDICATOR,					//17
	DAMAGEINDICATOR_FULLSCREEN,			//18
	SCREENINDICATOR,					//19
	DEVVERSION,							//20
	GAMEINFOMESSAGE,					//21
	VOTEMESSAGE,						//22
	CONSOLEMESSAGE,						//23	:	Chatting 창...
	DEATHLOG,							//24
	GAMESTATUS,							//25
	GAMETYPE_ANNIHILATION,				//26
	GAMETYPE_DEMOLITION,				//27
	GAMETYPE_PROGRESS_TANK,				//28
	INGAME_CLASSCHANGE,					//29
	OBSERVE_SPECTATORINFO,				//30
	OBSERVE_GAMEHELP,					//31
	QUITWINDOW,							//32
	QUITWINDOW_KICKMESSAGE,				//33
	QUITWINDOW_PRACTICE,				//34
	QUITWINDOW_WARNFORHOST,				//35
	QUITWINDOW_WARNFORHOST_PRACTICE,	//36
	REINFORCEMENT_COUNT,				//37
	SPECTATOR_CLASSCHANGEINFO,			//38
	SPECTATOR_INFO,						//39
	SPECTATOR_GAMEHELP,					//40
	WARMUP_CLASSCHANGEINFO,				//41
	WARMUP_COUNT,						//42
	GAMETYPE_ANNIHILATION_DOGTAGCOUNT,	//43
	GAMETYPE_DEMOLITION_C4ICON,			//44
	MEMBERINFO,							//45
};

var BYTE	ControlToggleInfo[45];

struct native SDeathMessage
{
	var string	KillerName, 
				KilledName;
	var	int		KillerLevel,
				KilledLevel;
	var int		KillerTeam, 
				KilledTeam;
	var int		KillerSlot,
				KilledSlot;
	var bool	bAmIKiller,		
				bAmIVictim;
	var Font	IconFont;
	var string	IconStr;
	var bool	bHeadshot;
	var bool	bWallshot;
	var float	DeathTime;
	var int		Height;
	var	int		IconHeight;
	var string	Str_Killer,	Str_Killed, Str_Icon;
	var int		Width_Killer, Width_Killed, Width_Icon;
};

var Array<SDeathMessage> DeathMessages;

// [2006/10/31, otterrrr] SDeathMessage는 플레이어 이름으로 상대방을 보관하므로 
// KillMessage(avaUIKillMessage)에 매번 이름을 비교해서 찍어줘야하며 
// 이름이 같은 플레이어가 있을경우 적과 아군을 분간못하는 경우가 생긴다.
// 따라서 아래의  별도의 저장을 한다
struct native KillMessageInfo
{
	var PlayerReplicationInfo KillerPRI;
	var PlayerReplicationInfo KilledPRI;
	var float TimeOfDeath;
	var bool bHeadShot;
	var bool bWallShot;
	var bool bExplosive;
};

var Array<KillMessageInfo> KillMessageData;
var	Array<KillMessageInfo> MyKillMessageData;

var Pawn PawnOwner;			// pawn for whom HUD is being displayed

struct native DamageIndicatorInfo
{
	var bool	bDisplayDirection;
	var float	DamageTime;
	var vector	HitLocAbs;
};

var array<DamageIndicatorInfo>	DamageIndicatorData;
var array< avaProjectile >		GrenadeForIndicatingList;

var Weapon CurrentWeapon;
//var float  WeaponBurstFadeTime;
//var float  WeaponBurstFade;
//var int CurrentWeaponBurstMode;

// Game Info Message
struct native GameInfoMessageInfo
{
	var string	GameInfoMessageTxt;
	var float	GameInfoMessageTime, GameInfoMessageSetTime, GameInfoMessageCode;
};

var GameInfoMessageInfo	GameInfo[3];

`define MAX_DAMAGE_INDICATOR_TIME	3.0

struct native RadarDisplayedActor
{
	var Actor	DisplayedActor;
	var BYTE	TeamIndex;
	var int		IconCode;
	var float	AddTime;
	var	bool	bOnlyLeader;		// Leader에게만 보이는 Actor 이다...
	var bool	bAdjustRotate;		// Actor 의 회전정보를 같이 반영해 준다...
};

//Test
var array<Actor> ARadarDisplayActors;

var array<RadarDisplayedActor>	RadarDisplayedActors;

var	Actor						TargetActor;
var float						TargettedTime;
//var	array<RadarDisplayedActor>	HUDDisplayedActors;

/******************************************************************************************
  Experimental Hud work using the new UI
 ******************************************************************************************/

// Spectator 용 Variable
var	Font		SpectatorFont;

var float		LastMissionAlertTime;

/* 3D Indicators & Flag for Strategy Action */

enum EIndicatorScreenArea
{
	ISA_Top,
	ISA_Right,
	ISA_Bottom,
	ISA_Left,
	ISA_Center,
	ISA_None,
};

enum EIndicatorScreenType
{
	INDICATORTYPE_Waypoint1,
	INDICATORTYPE_Waypoint2,
	INDICATORTYPE_QuickChat_Team,
	INDICATORTYPE_MissionObject,
	INDICATORTYPE_QuickChat_All,
};

struct native IndicatorInfo
{
	var float Life;
	var string Text;
	var vector Pos;
	var Color Color;
	var EIndicatorScreenArea ScreenArea;
	var float Distance;
	var bool bBlink;
	var EIndicatorScreenType Type;
};

struct native QuickChatIndicatorInfo
{
	var avaPawn Pawn;
	var float Life;
	var EIndicatorScreenType Type;
};

var bool							bSignalActivate;
var vector							SignalPos;

var array<IndicatorInfo>			Indicators;
var array<QuickChatIndicatorInfo>	QuickChatIndicatorBuffer;

/* QuickChat & Vote UI */
struct native StateUIMessage
{
	var string	Text;
	var float	Life;
	var Color	Color;
};

var array<StateUIMessage> QuickChatMenuMessages;
var array<StateUIMessage> VoteMenuMessages;

struct native DrawLineUnit
{
	var string Text;
	var Color Color;
};

/** Holds the actual Scene */
var UIScene HudScene,
			HUDSceneBroadcast,
			InGameGlobalScene,
			ClassSelectScene,
			//QuitMenuScene,
			//QuitKickScene,
			//QuitBlockScene,
			ScoreScene,
			ScoreScene_DM,
			SpectatorScene,
			DeathScene,
			ChatScene,
			TeamChatScene,
			WarmupScene,
			RespawnScene,
			GameInfoScene,
			ObserverScene,
			ObserverSceneBroadCast,
			LargeMapScene;

/// UIScene Instances
var UIScene HUDSceneInstance, 
			InGameGlobalSceneInstance,
			ClassSelectSceneInstance, 
			//QuitMenuSceneInstance,
			//QuitKickSceneInstance,
			//QuitBlockSceneInstance,
			ScoreSceneInstance, 
			SpectatorSceneInstance,
			DeathSceneInstance,
			//ChatSceneInstance,
			//TeamChatSceneInstance,
			WarmupSceneInstance,
			RespawnSceneInstance,
			GameInfoSceneInstance,
			ObserverSceneInstance,
			LargeMapSceneInstance;

var array<UIScene>	SceneInstances;

var float ClassSelectSceneActivateTime;
var float ClassSelectSceneActiveTime;
var float ClassSelectSceneBlendInTime;
var float ClassSelectSceneBlendOutTime;

/** 2006/11/12 YTS , 추가 */
struct native DeadPlayerInfo
{
	var PlayerReplicationInfo PRI;
	var vector Location;
	var float UpdateTime;
};

var array<DeadPlayerInfo> DeadPlayerList;					/**< 2006/11/12 윤태식, DeadPlayerList - 죽은 Pawn들의 정보 */

var UIScene					ColorCorrectionScene;			/**< 2006/11/16 윤태식 */
var UIscene					ColorCorrectionSceneInstance;	/**< 2006/11/16 윤태식*/

/**	2006/12/18 윤태식, Optional Scene들을 보관, 조작하기 위한 구조체 
  *	주로 avaHUD state( Scene State )에 의해 검색되고 그 결과에 따라 동작을 바꿈. */
struct native OptionalSceneDataType
{
	var SceneStateDataType		SceneData;
	var UIScene					SceneInstance;
	var UIScene					Scene;
};

var array<OptionalSceneDataType>	OptionalSceneData;

/** FullScreenMap (M키를 누르면 나올 전술맵) */
var bool							bShowFullScreenMap;

/** 세부 플레이어 정보를 화면에 출력해준다 */
var bool							bShowPlayerDetail;

/** 시스템 공지 앞에 붙는 라벨 ex) [시스템]: 12시부터 서버점검에 들어갑니다 */
var localized string				SystemMessageLabel;
var localized string				GameMessageLabel;

/** ScreenPercentage 변경 */
var localized string				StrChangeScreenQuality;

/** 채팅 금지 메세지. ex) 채팅 금지. 남은시간 3초 */
var localized string				ChatOffMessage;
var localized string				ChatOffMinute;
var localized string				ChatOffSecond;

/** 추방 메세지 ex) 아바미워 님이 쫒겨났습니다.*/
var localized string				KickMessage;

var	localized string				PlaceOfInterest[255];			// 목표지역 설정 Local string
var localized string				ChatAll;						// 전체..

var localized string				SpectatorHelp;					// 마우스좌측키를 누르면 다음 사용자를 볼 수 있습니다.
var localized string				SpectatorHelpEx;				// 스페이스바키를 누르면 3인칭 뷰로 볼 수 있습니다.
var localized string				PracticeModeHelp;				// 연습모드일때 안내...
var localized string				PauseHelp;						// 게임이 Pause 되었을 때 안내 문구
var localized string				FreeCamHelp;					// FreeCam 일때 Help, F1,F2 키를 이용해 모드를 전환할 수 있습니다.
var localized string				RefereeNotify;					// 심판임을 알려주는 화면입니다.

var string							DisplayedSpectatorHelp;			//
var string							DisplayedSpectatorHelpEx;	


var float							DeathSceneStartTime;

var const float						HostBlockTime;

var string							RTNoticeMessage;
var float							RTNoticeUpdateTime;

var array<avaMissionNavPoint>		POIList;

var Texture2D						SlotNumTexture;
var	Texture2D						LvlTexture;
var array< Texture2D >				ClanMarkTextures;
var	Texture2D						LeaderTexture;
var float							LeaderU,LeaderV,LeaderUL,LeaderVL;

//============================================================================================================
// 각종 Message에 사용되는 기본 Color 는 여기에 있는 것들을 사용하도록 합니다...
//============================================================================================================
var const	Color	ConsoleDefaultColor;		//	Message 를 찍을 때 기본 색상이다.
var const	Color	FriendlyColor;				//	자기편의 색상을 찍어주기 위해서 사용한다.
var const	Color	EnemyColor;					//	상대편의 색상을 찍어주기 위해서 사용한다.
var const	Color	SpectatorColor;				//	Spectator 의 색상을 찍어주기 위해서 사용한다.
var	const	Color	TeamColor[2];				//	0 : EU의 Color, 1 : NRF의 Color, 2 : Spectator 의 Color
var const	Color	QuickChatIndicatorColor;	//	Quick Chat 시 Indicator의 Color 이다.	
var const	Color	QuickChatTauntColor;		//	Quick Chat[적도발시] Indicator의 Color 이다.
var const	Color	LocationColor;				//	Chat 시 Location 을 표시하기위한 Color 이다.
var const	Color	ShadowColor;				//	Font 의 Shadow Color
var const	Color	ComandCenterMsgColor;		//	CommandCenter 에서 보낸 Message 의 Color 이다..
var const	Color	ChatColor;					//	기본 ChatColor
var const	Color	BIAColor;					//	전우 Color

struct native DrawPawnData
{
	var	Pawn		Pawn;
	var float		Distance;
};

var array<DrawPawnData>	DrawPawnList;


struct native IconCodeInfo
{
	var	int					id;
	var Texture2D			Image;
	var color				Color;
	var TextureCoordinates	Coord;
	var bool				bAnimated;
	var int					AnimImgCnt;
	var float				AnimPeriod;

	structdefaultproperties
	{
		bAnimated	=false
		AnimImgCnt	=1
		AnimPeriod	=2.0
		Color		=(R=255,G=255,B=255,A=255)
	}
};

var(Icon) globalconfig array<IconCodeInfo>	Icons;

var	int	RadarTopTargetIcon;
var	int	HUDTopTargetIcon;
var int	SignalIcon;

var string	InGameHelp;

var	string	SwapKeyName;

/** ConsoleMessage Extensions */
	/** circular queue */
var transient			int				ConsoleMessageInitIndex;		// 원형 큐. 첫번째 아이템인덱스
var transient			int				ConsoleMessageEndIndex;			// 원형 큐, 마지막-1 아이템인덱스
var transient			int				ConsoleMessageSize;				// 원형 큐 아이템 개수

	/** view perspective ( HUD ) */
var transient			int				UpdateConsoleMessageCount;		// 새로 들어온 아이템 개수( HUD업데이트에 쓰임 )
var transient			int				UpdateConsoleMessagePageUpDown;		// 페이지 업 1 /다운 -1 요청이 들어옴
var transient			int				UpdateConsoleMessageThrowOuts;		// 큐에서 삭제된 항목 갯수 (리스트뷰를 유지하기 위해 필요)
																		// 삭제된 항목만큼 리스트 뷰 인덱스를 변경해줘야한다

var float								PrevScreenQuality;

var localized string					strNotifyHide;
var bool								bNotifyHide;

simulated function PostBeginPlay()
{
	Local LocalPlayer			LP;
	Local avaGameViewportClient Viewport;
	Local avaMissionNavPoint	NavPoint;
	local int					i;
	local Pawn					P;

	super.PostBeginPlay();
    SetTimer(1.0, true);

	foreach WorldInfo.AllNavigationPoints(class'avaMissionNavPoint', NavPoint)
	{
		if ( NavPoint.bShowRadar )
			AddActorToRadar(NavPoint, NavPoint.TeamIndex, NavPoint.IconCode, NavPoint.bOnlyLeader);
		
		// 특수 지역 이름.
		if ( NavPoint.NameIndex >= 0 )
		{
			for ( i = 0 ; i < POIList.length ; ++ i )
			{
				if ( POIList[i].NameIndex > NavPoint.NameIndex )
					break;
			}

			if ( i < POIList.length )
			{
				POIList.Insert(i,1);
				POIList[i]	=	NavPoint;
			}
			else
			{
				POIList.Length	=	i+1;
				POIList[i]		=	NavPoint;		
			}
		}
	}

	ForEach DynamicActors(class'Pawn', P)
	{
		if ( avaPawn(P) != None || avaVehicle(P) != None )
			AddPostRenderedActor(P);
	}

	LP = LocalPlayer(PlayerOwner.Player);
	Viewport = avaGameViewportClient(LP.ViewportClient);
	Viewport.QuickChatUI.RedirectMessages(QuickChatMenuMessages);
	Viewport.VoteUI.RedirectMessages(VoteMenuMessages);

	OpenPrevScene();

	CalcBindingKeyName();

	if ( PlayerOwner.PlayerReplicationInfo != None && PlayerOwner.PlayerReplicationInfo.bOnlySpectator == true )
		SetObserverKeyBinding();
		
}

simulated function OpenPrevScene()
{
	// State 와 상관없이 나와야 하는 Scene 들을 만든다...
	OpenScene( InGameGlobalScene,	InGameGlobalSceneInstance );
	OpenScene( DeathScene,			DeathSceneInstance );

	if ( HasBroadCastAuthority() )
	{
		OpenScene( HUDSceneBroadcast,		HudSceneInstance );
		OpenScene( SpectatorScene,			SpectatorSceneInstance );
		OpenScene( ObserverSceneBroadcast,	ObserverSceneInstance );
	}
	else
	{
		OpenScene( HudScene,			HudSceneInstance );
		OpenScene( SpectatorScene,		SpectatorSceneInstance );
		OpenScene( ObserverScene,				ObserverSceneInstance );
	}

	if ( PlayerOwner.ViewTarget == None || PlayerOwner.ViewTarget == PlayerOwner )
	{
		// Hide Viewtarget Info
		EnableChildGroupVisibility( HUDSceneInstance, THROWABLEWEAPON, false );
		EnableChildGroupVisibility( HUDSceneInstance, HEALTHGAUGE, false );
	}

	if ( RespawnScene != None )		OpenScene( RespawnScene,	RespawnSceneInstance );
	
	HudSceneInstance.SetVisibility(true);
	SpectatorSceneInstance.SetVisibility(false);
	ObserverSceneInstance.SetVisibility(false);

	DeathSceneInstance.SetVisibility(false);
	if ( RespawnSceneInstance != None )	RespawnSceneInstance.SetVisibility(false);
	
	// Scene이 열릴때 GameStatus 내용을 업데이트 하는데서 프로그램이 죽는 경우가 있음. 
	// 따라서 Tab이 눌려 필요한 순간에 Scene을 불러줄것임.
	//// Open ScoreScene, ResultScene...
	//// Base Scene 보다 늦게 만들어져야 한다...
	//// Scene 과 Kismet 을 연결하려면 Opend Scene 에서 하면 안된다... 어떻게???
	OpenScene(WarmupScene,WarmupSceneInstance);

	
	UpdateWarmupRound();
}

simulated function OpenPostScene()
{
	if ( WorldInfo.GRI.GameClass.default.bTeamGame == false )	OpenScene( ScoreScene_DM, ScoreSceneInstance );
	else														OpenScene( ScoreScene,	ScoreSceneInstance );											

	ScoreSceneInstance.SetVisibility(false);
	ScoreSceneInstance.SetSceneInputMode( INPUTMODE_None );

	// OnlySpectator Mode 가 아니라면, Class Select Scene 을 만든다...
	if ( PlayerOwner.PlayerReplicationInfo.bOnlySpectator == false )
	{
		OpenScene(ClassSelectScene,ClassSelectSceneInstance);
		ClassSelectSceneInstance.Opacity = 0.0;
		ClassSelectSceneInstance.SetVisibility(false);
	}

	/// Chat, TeamChat 화면은 InGameGlobalScene 안으로 옮겨졌음

	// Open ChatScene, TeamChatScene...
	// Base Scene 보다 늦게 만들어져야 한다...
	//OpenScene( ChatScene,		ChatSceneInstance );
	//OpenScene( TeamChatScene,	TeamChatSceneInstance );

	//ChatSceneInstance.SetVisibility(false);
	//TeamChatSceneInstance.SetVisibility(false);
	//ChatSceneInstance.SetSceneInputMode( INPUTMODE_None );
	//TeamChatSceneInstance.SetSceneInputMode( INPUTMODE_None );

	if ( !HasBroadCastAuthority() )
	{
		OpenScene(GameInfoScene,GameInfoSceneInstance);
		GameInfoSceneInstance.SetSceneInputMode( INPUTMODE_None );
	}
	else
	{
		OpenScene(LargeMapScene,LargeMapSceneInstance);
		LargeMapSceneInstance.SetSceneInputMode( INPUTMODE_None );
		LargeMapSceneInstance.SetVisibility( false );
	}
}

simulated function string GetCommandNameToKeyName( string CommandName )
{
	local int BindingIndex;
	local int Code;
	BindingIndex = PlayerOwner.PlayerInput.Bindings.find('Command', CommandName );
	if ( BindingIndex >= 0 )
	{
		Code = PlayerOwner.PlayerInput.GetKeyCodeByName( PlayerOwner.PlayerInput.Bindings[BindingIndex].Name );
		return Localize( "PlayerInput", "KeyCode["$Code$"]", "avaGame" );
	}
	return "";
}

simulated function CalcBindingKeyName( )
{
	SwapKeyName = GetCommandNameToKeyName( "SwapDroppedWeapon" );
	DisplayedSpectatorHelp		= class'avaStringHelper'.static.Replace( GetCommandNameToKeyName( "Fire" ),	 "%s", SpectatorHelp );
	DisplayedSpectatorHelpEx	= ","$class'avaStringHelper'.static.Replace( GetCommandNameToKeyName( "Jump" ), "%s", SpectatorHelpEx );
}

// Pawn 이 새롭게 Setting 되었다. 초기화 할것이 있으면 하자!!!
simulated function PawnSpawned()
{
	if ( avaPlayerController(PlayerOwner).IsInState( 'Spectating' ) )
		OpenSpectatorScene();
	else
		OpenHudScene();

	//ResetFlagPosition();
	ResetDamageTime();	// Damage Indicator 를 초기화 한다...
	ShowMissionHelp();	
}

simulated function NotifyHide()
{
	bNotifyHide	=	true;
	ClearTimer( 'ClearNotifyHide' );
	SetTimer( 3.0, false, 'ClearNotifyHide' );
}

simulated function ClearNotifyHide()
{
	bNotifyHide =	false;
}

event PostRender()
{
	// 아예 HUD.uc에 있는 것을 모두 가져와버렸다. super.PostRender() 부르지 말 것
	local float		XL, YL, YPos;
	local avaPawn	P;
	//local Pawn		Test;
	//local int		i,j;

    local float FontDX, FontDY;
    local float X, Y;
	local int i;
	local float Spacing;
	local float NickWidth;
	local float BarWidth, BarHeight;
	local float Margin;
	local float Progress;
	local float XP, YP, FontWidth, FontHeight;

	//DrawActorOverlays();

	// Set up delta time
	RenderDelta = WorldInfo.TimeSeconds - LastHUDRenderTime;

	if ( bNotifyHide == true )
	{
		Canvas.Font = class'Engine'.Static.GetMediumFont();
		Canvas.TextSize ( strNotifyHide, FontDX, FontDY);
		Canvas.DrawColor = WhiteColor;
		Canvas.DrawColor.A = 255;
		Canvas.SetPos( (Canvas.SizeX-FontDX)/2.0, (Canvas.SizeY-FontDY)/2.0);
		Canvas.DrawText( strNotifyHide );
	}

	// Pre calculate most common variables
	//if ( SizeX != Canvas.SizeX || SizeY != Canvas.SizeY )
	//{
	//	PreCalcValues();
	//}

	if ( bShowDebugInfo )
	{
		Canvas.Font = class'Engine'.Static.GetTinyFont();
		Canvas.DrawColor = ConsoleColor;
		Canvas.StrLen("X", XL, YL);
		YPos = 0;

		Canvas.SetPos(4,YPos);
		Canvas.DrawText("HUD State" @GetStateName() );
		YPos += YL;

		PlayerOwner.ViewTarget.DisplayDebug(self, YL, YPos);

		if (ShouldDisplayDebug('Others'))
		{
			ForEach DynamicActors(class'avaPawn', P)
			{
				if ( P != PlayerOwner.ViewTarget )
					P.DisplayDebug( self, YL, YPos );
			}
		}
		else
		{
			ForEach DynamicActors(class'avaPawn', P)
			{
				if (  P != PlayerOwner.ViewTarget && ( P.PlayerReplicationInfo != NONE && ShouldDisplayDebug( name( P.PlayerReplicationInfo.PlayerName ) ) ) )
				{
					P.DisplayDebug( self, YL, YPos );
				}
			}
		}

		if (ShouldDisplayDebug('AI') && (Pawn(PlayerOwner.ViewTarget) != None))
		{
			DrawRoute(Pawn(PlayerOwner.ViewTarget));
		}
	}
	
	if ( bShowHud )
	{
		// Controller 의 DrawHUD 는 부르지 않도록 한다....
		// Inventory 의 RenderOverlay 나 ActiveRenderOverlay 를 사용했었는데 현재는 사용하지 않는다....
		//PlayerOwner.DrawHud( Self );

		DrawHud();

		/// 이건 뭐여!
		//if ( PlayerOwner.ProgressTimeOut > WorldInfo.TimeSeconds )
		//{
		//	DisplayProgressMessage();
		//}

		// 플레이어 로딩 진행 상황 -- alcor
		if (class'avaNetHandler'.static.GetAvaNetHandler().RoomStartingPlayerList.length > 0)
		{
			Canvas.Font = class'Engine'.Static.GetMediumFont();
			Canvas.TextSize ("아빠아바아봐아놔아좌", FontDX, FontDY);

			Spacing = 8;
			NickWidth = Max( FontDX, 100 );
			BarWidth  = NickWidth;
			BarHeight = FontDY / 2;
			Margin    = float(int(Canvas.SizeX % 4) + 44);

			X = Canvas.SizeX - (BarWidth + Spacing + NickWidth + Margin);
			Y = Margin + 200;

//			X = (0.05 * HudCanvasScale * Canvas.SizeX);
//			Y = (0.05 * HudCanvasScale * Canvas.SizeY);

			//Y -= FontDY * (float (LineCount) / 2.0);

			Canvas.DrawColor = WhiteColor;

			for (i = 0; i < class'avaNetHandler'.static.GetAvaNetHandler().RoomStartingPlayerList.length; i++)
			{
				Progress = Min(100, class'avaNetHandler'.static.GetAvaNetHandler().RoomStartingPlayerList[i].LoadingProgress);
				Progress = Max(0, Progress);

				Canvas.DrawColor.A = 255;
				Canvas.TextSize(class'avaNetHandler'.static.GetAvaNetHandler().RoomStartingPlayerList[i].Nickname, FontWidth, FontHeight);
				Canvas.SetPos(X + NickWidth - FontWidth, Y);
				Canvas.DrawText( class'avaNetHandler'.static.GetAvaNetHandler().RoomStartingPlayerList[i].Nickname );

				XP = X + Spacing + NickWidth;
				YP = Y + (FontDY - BarHeight) / 2;

				Canvas.DrawColor.A = 64;
				Canvas.SetPos(XP, YP);
				Canvas.FillColor(1, BarHeight);		// Left
				Canvas.FillColor(BarWidth, 1);		// Top
				Canvas.SetPos(XP+BarWidth, YP);
				Canvas.FillColor(1, BarHeight);		// Right
				Canvas.SetPos(XP, YP+BarHeight);
				Canvas.FillColor(BarWidth, 1);		// Bottom

				Canvas.DrawColor.A = 255;
				Canvas.SetPos(XP, YP);
				Canvas.FillColor(BarWidth * Progress / 100, BarHeight);	// Bar

				Y += FontDY;
			}
		}

		//Canvas.Font = class'Engine'.Static.GetMediumFont();
		//Canvas.TextSize ("A", FontDX, FontDY);

		//X = (0.05 * HudCanvasScale * Canvas.SizeX);
		//Y += (0.05 * HudCanvasScale * Canvas.SizeY);

		//foreach WorldInfo.DynamicActors(class'avaPawn', P)
		//{
		//	if (P.PlayerReplicationInfo != None && P.PlayerReplicationInfo.Ping > 300)
		//	{	
		//		Canvas.DrawColor = WhiteColor;
		//		Canvas.DrawColor.A = 255;

		//		Canvas.SetPos (X, Y);
		//		Canvas.DrawText ( P.PlayerReplicationInfo.PlayerName @ P.PlayerReplicationInfo.Ping @ P.PlayerReplicationInfo.PacketLoss );
		//		Y += FontDY;
		//	}
		//}			
	}
	else 
	{
		PlayerOwner.DrawHud( Self );
	}

	if ( bShowBadConnectionAlert )
	{
		DisplayBadConnectionAlert();
	}
	LastHUDRenderTime = WorldInfo.TimeSeconds;
}



/* DrawActorOverlays()
draw overlays for actors that were rendered this tick
*/
function DrawActorOverlays()
{
	local int i;
	local Actor A;

	if ( !bShowHud )
		return;
	Canvas.Font = class'Engine'.Static.GetTinyFont();

	for ( i=0; i<PostRenderedActors.Length; i++ )
	{
		A = PostRenderedActors[i];
		//if ( (A != None) && (A.LastRenderTime == WorldInfo.TimeSeconds) && ((vector(PlayerOwner.Rotation) dot (A.Location - PlayerOwner.ViewTarget.Location)) > 0) )
		if ( (A != None) && ((vector(PlayerOwner.Rotation) dot (A.Location - PlayerOwner.ViewTarget.Location)) > 0) )
		{
			A.PostRenderFor(PlayerOwner,Canvas,PlayerOwner.ViewTarget.Location,PlayerOwner.ViewTarget.Location);
		}
	}
}

/** RemovePostRenderedActor()
remove an actor from the PostRenderedActors array
*/
function RemovePostRenderedActor(Actor A)
{
	local int i;

	for ( i=0; i<PostRenderedActors.Length; i++ )
	{
		if ( PostRenderedActors[i] == A )
		{
			PostRenderedActors[i] = None;
			return;
		}
	}
}

/** AddPostRenderedActor()
add an actor to the PostRenderedActors array
*/
function AddPostRenderedActor(Actor A)
{
	local int i;

	// make sure that A is not already in list
	for ( i=0; i<PostRenderedActors.Length; i++ )
	{
		if ( PostRenderedActors[i] == A )
		{
			return;
		}
	}

	// add A at first empty slot
	for ( i=0; i<PostRenderedActors.Length; i++ )
	{
		if ( PostRenderedActors[i] == None )
		{
			PostRenderedActors[i] = A;
			return;
		}
	}

	// no empty slot found, so grow array
	PostRenderedActors[PostRenderedActors.Length] = A;
}

function DisplayHit(vector HitDir, int Damage, class<DamageType> damageType)
{
	//local float NewDamageTime;
	Local vector HitDirRel;
	if ( PawnOwner == None )	return;
	HitDirRel = HitDir;
	// NewDamageTime = Clamp(Damage ,20 ,30) / 10.0;
	Assert(PawnOwner != None);
	// DamageType 이 Silencer 인 경우에는 Indicator 를 찍어주지 않는다.
	DamageIndicatorData.Length = DamageIndicatorData.Length + 1;
	DamageIndicatorData[DamageIndicatorData.Length - 1].DamageTime	= WorldInfo.TimeSeconds;	//NewDamageTime;
	DamageIndicatorData[DamageIndicatorData.Length - 1].HitLocAbs	= PawnOwner.Location + HitDirRel;
	
	if ( class<avaDmgType_GunSilencer>(damageType) != None )	DamageIndicatorData[DamageIndicatorData.Length - 1].bDisplayDirection = false;
	else														DamageIndicatorData[DamageIndicatorData.Length - 1].bDisplayDirection = true;
}

exec function ActivateClassSelectScene()
{
	// 관전모드로 들어갔을 경우에는 병과 선택 화면을 보여주지 않는다...
	if ( PlayerOwner.PlayerReplicationInfo.bOnlySpectator == true )
		return;

	OpenClassSelectScene();

	ClassSelectSceneActivateTime = WorldInfo.TimeSeconds;
	ClassSelectSceneActiveTime	 = 2.0;
	ClassSelectSceneBlendInTime	 = 1.0 - ClassSelectSceneInstance.Opacity; 
	ClassSelectSceneBlendOutTime = 1.0;
	//ClassSelectSceneInstance.Opacity = 1.0;
}

function bool HasViewtarget()
{
	return ( PlayerOwner.ViewTarget != None && PlayerOwner.ViewTarget != PlayerOwner );
}

function bool HadViewtarget()
{
	return ( PrvViewTarget != None && PrvViewTarget != PlayerOwner );
}

event Tick(float DeltaTime)
{
	if ( PrvViewTarget != PlayerOwner.ViewTarget || bInitFirstViewTarget == false )
	{
		if ( !HadViewtarget() && HasViewtarget() )
		{
			// Show Viewtarget Info
			if ( ControlToggleInfo[THROWABLEWEAPON] == 0 )
				EnableChildGroupVisibility( HUDSceneInstance, THROWABLEWEAPON, true );
			if ( ControlToggleInfo[HEALTHGAUGE] == 0 )
				EnableChildGroupVisibility( HUDSceneInstance, HEALTHGAUGE, true );
		}
		else if ( HadViewtarget() && !HasViewtarget() )
		{
			// Hide Viewtarget Info
			EnableChildGroupVisibility( HUDSceneInstance, THROWABLEWEAPON, false );
			EnableChildGroupVisibility( HUDSceneInstance, HEALTHGAUGE, false );
		}
		PrvViewTarget =	PlayerOwner.ViewTarget;
		bInitFirstViewTarget = true;
	}
	
	if ( ClassSelectSceneBlendInTime > 0.0 )
	{
		ClassSelectSceneBlendInTime -= DeltaTime * 2.0;
		ClassSelectSceneInstance.Opacity = 1.0 - ClassSelectSceneBlendInTime;
		if ( ClassSelectSceneInstance.Opacity > 1.0 )
			ClassSelectSceneInstance.Opacity = 1.0;
	}
	else if ( ClassSelectSceneActiveTime > 0.0 )
	{
		ClassSelectSceneActiveTime -= DeltaTime;
	}
	else if ( ClassSelectSceneBlendOutTime > 0.0 )
	{
		ClassSelectSceneBlendOutTime -= DeltaTime;
		if ( ClassSelectSceneBlendOutTime > 0.0 )
		{
			ClassSelectSceneInstance.Opacity = ClassSelectSceneBlendOutTime / 1.0;
		}
		else
		{
			ClassSelectSceneInstance.Opacity = 0.0;
			CloseClassSelectScene();
		}
	}

	Super.Tick(DeltaTime);

	UpdateOptionalScenes();
}

function InstalledBomb( bool bEnable, optional Actor bomb )
{
	BombInstalled = bEnable;
	if ( bEnable )	AddActorToRadar( bomb, avaProj_C4(bomb).Instigator.GetTeamNum(), 16 );	// 폭탄은 Icon Code 가 16 이라고 하던데... 제대로 하려면 owner 를 가져온뒤 pickup class 를 가져오도록 한다...
	else			RemoveActorFromRadar( bomb );
}

function UpdateBombAlertBlink( bool bEnable )
{
	BombAlertBlink = bEnable;
}

function int CalculateXYPosition(string pos, int BASE_LENGTH, int CanvasSize )
{
	local string tempStr;
	local int mode, num;
	mode = 0;

	tempStr = Left(pos, 1);

	if( tempStr == "r" || tempStr == "R")
		mode = 1;
	else if( tempStr == "c" || tempStr == "C")
		mode = 2;
	else if( tempStr == "a" || tempStr == "A")
		mode = 3;

	if( mode == 0 )
		num = int(pos) * CanvasSize / BASE_LENGTH;
	else
	{
		num = int(Right(pos, Len(pos)-1));
		if( mode != 3 )
		{
			num=num*CanvasSize / BASE_LENGTH;
			if( mode == 1)
				num = CanvasSize - num;
			else if( mode == 2 )
				num = CanvasSize / 2 + num;
		}
	}
	return num;
}

function DrawHUD()
{
	if ( PlayerOwner != None )
		PawnOwner = Pawn(PlayerOwner.ViewTarget);
	if( PawnOwner != None )
	{
		DrawIndicators();
	}
}

function string GetFormattedTime( float fTime, optional bool bHour, optional bool ms )
{
	local int hour, minute, second;
	local float milisecond;
	local string Result;
	second = fTime;
	if ( fTime > 3600 && bHour )
	{
		hour	= fTime / 3600;
		second	= fTime - hour * 3600;
	}
	minute		= second / 60;
	second		= fTime - minute * 60;
	milisecond	= fTime - minute * 60 - second;

	if ( bHour )
	{
		if( hour > 0 )	Result = string(hour)$":";
	}

	if ( minute < 10 )
	{
		if ( hour == 0 )	Result = Result$" ";
		else				Result = Result$"0";
	}
	Result = Result$minute$":";
	if ( second < 10 )
	{
		Result = Result$"0";
	}
	Result = Result$second;
	if ( ms )
	{
		milisecond *= 100;
		if ( milisecond < 10 )
			Result = Result$":0"$int(milisecond);
		else
			Result = Result$":"$int(milisecond);
	}
	return Result;
}

simulated function DrawTextOrg( float x, float y, float w, float h, coerce string str, int va, int ha, Font font )
{
	local float XL, YL;
	local int XPos, YPos;	// 실제 Text 를 찍을 위치

	Canvas.Font = font;
	Canvas.StrLen( str, XL, YL );

		 if ( va == 0 )	YPos = y + h/2 - YL/2;
	else if ( va == 1 )	YPos = y;
	else if ( va == 2 ) YPos = y + h - YL;

		 if ( ha == 0 ) XPos = x + w/2 - XL/2;
	else if ( ha == 1 )	XPos = x;
	else if ( ha == 2 ) XPos = x + w - XL;

	Canvas.SetPos( XPos, YPos );
	Canvas.DrawText( str );
}

// va ( 0: Center 1:Top 2:Bottom )
// ha ( 0: Center 1:Left 2:Right )
simulated function DrawText( float x, float y, float w, float h, coerce string str, int va, int ha, Font font )
{
	local float XL, YL;
	local int XPos, YPos;	// 실제 Text 를 찍을 위치

	x = x * Canvas.ClipX / 1024;
	y = y * Canvas.ClipY / 768;
	w = w * Canvas.ClipX / 1024;
	h = h * Canvas.ClipY / 768;

	//Canvas.DrawColor = c;
	Canvas.Font = font;
	Canvas.StrLen( str, XL, YL );

		 if ( va == 0 )	YPos = y + h/2 - YL/2;
	else if ( va == 1 )	YPos = y;
	else if ( va == 2 ) YPos = y + h - YL;

		 if ( ha == 0 ) XPos = x + w/2 - XL/2;
	else if ( ha == 1 )	XPos = x;
	else if ( ha == 2 ) XPos = x + w - XL;

	Canvas.SetPos( XPos, YPos );
	Canvas.DrawText( str );
}

simulated function MessageEx( PlayerReplicationInfo PRI, coerce string Msg, Name MsgType, optional float LifeTime, optional avaMsgParam Param)
{
	Local Class<LocalMessage> LocalMessageClass;

	LocalMessageClass = class'LocalMessage';

	AddConsoleMessageEx(Msg,LocalMessageClass,PRI,LifeTime, MsgType, Param);
}

simulated function Message( PlayerReplicationInfo PRI, coerce string Msg, name MsgType, optional float LifeTime)
{
	local Class<LocalMessage>	LocalMessageClass;
	local Color					MessageColor;

	MessageColor = ConsoleDefaultColor;
	switch( MsgType )
	{
		case 'Say':
			if ( PRI == None )
				return;
//			Msg = PRI.PlayerName$": "$Msg;
			LocalMessageClass = class'avaSayMsg';
			break;
		case 'TeamSay':
			if ( PRI == None )
				return;
//			Msg = PRI.PlayerName$"("$PRI.GetLocationName()$"): "$Msg;
			LocalMessageClass = class'avaTeamSayMsg';
			break;
		case 'CriticalEvent':
			LocalMessageClass = class'avaCriticalEventPlus';
			break;
		case 'QuickVoice':
			if( PRI == None )
				return;
			Msg = PRI.PlayerName$"(Radio): "$Msg;
			LocalMessageClass = class'avaTeamSayMsg';
			break;
		case 'UnknownTeam':
			Msg = PRI.PlayerName@":"@Msg;
			LocalMessageClass = class'avaMemberMessage';
			break;
		case 'BlueTeam':
			Msg = PRI.PlayerName@":"@Msg;
			LocalMessageClass = class'avaBlueTeamMessage';
			break;

		case 'YellowTeam':
			Msg = PRI.PlayerName@":"@Msg;
			LocalMessageClass = class'avaYellowTeamMessage';
			break;

		case 'Leader':
			Msg = PRI.PlayerName@":"@Msg;
			Msg = class'avaLeaderMessage'.default.LeaderLabel@Msg;
			LocalMessageClass = class'avaLeaderMessage';
			break;
		case 'CommandCenter':
			Msg = class'avaLocalizedMessage'.default.CommandCenterLabel@":"@Msg;
			LocalMessageClass	= class'LocalMessage';
			MessageColor		= ComandCenterMsgColor;
			break;
		case 'WaypointQuickChat':


		default:
			LocalMessageClass = class'LocalMessage';
			break;
	}

	AddConsoleMessageEx(Msg,LocalMessageClass,PRI,LifeTime, MsgType, , MessageColor );
}

function AddConsoleMessage(coerce string Msg, class<LocalMessage> InMessageClass, PlayerReplicationInfo PRI, optional float LifeTime)
{
	AddConsoleMessageEx(Msg, InMessageClass, PRI, LifeTime);
}

// HUD의 AddConsoleMessage 코드를 들고와 수정하였음
function AddConsoleMessageEx(coerce string Msg, class<LocalMessage> InMessageClass, PlayerReplicationInfo PlayerReplicationInfo,
							 optional float LifeTime, optional Name MsgType, optional avaMsgParam Param, optional Color TextColor = ConsoleDefaultColor)
{
	Local avaPlayerReplicationInfo PRI;
	//Local int Idx, MsgIdx;
	Local ConsoleMessage NewConsoleMessage;

	PRI = avaPlayerReplicationInfo(PlayerReplicationInfo);

	// check for beep on message receipt
	if( bMessageBeep && InMessageClass.default.bBeep )
	{
		PlayerOwner.PlayBeepSound();
	}

	//MsgIdx = -1;
	//// find the first available entry
	//if (ConsoleMessages.Length < ConsoleMessageCount)
	//{
	//	MsgIdx = ConsoleMessages.Length;
	//}
	//else
	//{
	//	// look for an empty entry
	//	for (Idx = 0; Idx < ConsoleMessages.Length && MsgIdx == -1; Idx++)
	//	{
	//		if (ConsoleMessages[Idx].Text == "")
	//		{
	//			MsgIdx = Idx;
	//		}
	//	}
	//}
 //   if( MsgIdx == ConsoleMessageCount || MsgIdx == -1)
 //   {
	//	// push up the array
	//	for(Idx = 0; Idx < ConsoleMessageCount-1; Idx++ )
	//	{
	//		ConsoleMessages[Idx] = ConsoleMessages[Idx+1];
	//	}
	//	MsgIdx = ConsoleMessageCount - 1;
 //   }
	//// fill in the message entry
	//if (MsgIdx >= ConsoleMessages.Length)
	//{
	//	ConsoleMessages.Length = MsgIdx + 1;
	//}

	//ConsoleMessages[MsgIdx].Text		= Msg;
	//ConsoleMessages[MsgIdx].TextColor	= TextColor;			

	//if ( LifeTime != 0.f )	ConsoleMessages[MsgIdx].MessageLife = WorldInfo.TimeSeconds + LifeTime;
	//else					ConsoleMessages[MsgIdx].MessageLife = WorldInfo.TimeSeconds + InMessageClass.default.LifeTime;
 //   ConsoleMessages[MsgIdx].PRI			= PRI;
	//ConsoleMessages[MsgIdx].TypeName = MsgType;
	//if( PRI != None )
	//{
	//	ConsoleMessages[MsgIdx].LocationName = PRI.GetLocationName();
	//	ConsoleMessages[MsgIdx].bDead		= ( PRI.bOnlySpectator == true || PRI.bIsSpectator == true );
	//	ConsoleMessages[MsgIdx].PlayerName	= (PRI.IsSquadLeader() ? class'avaLeaderMessage'.default.LeaderLabel : "") $" "$PRI.PlayerName;
	//	ConsoleMessages[MsgIdx].TeamIndex	= (PRI.Team == None) ? TEAM_Unknown : PRI.Team.TeamIndex;
	//}

	NewConsoleMessage.Text			= Msg;
	NewConsoleMessage.TextColor		= TextColor;			
	NewConsoleMessage.MessageLife	= WorldInfo.TimeSeconds + 6.0f;
    NewConsoleMessage.PRI			= PRI;
	NewConsoleMessage.TypeName = MsgType;
	if( PRI != None )
	{
		NewConsoleMessage.LocationName = PRI.GetLocationName();
		NewConsoleMessage.bDead		= ( PRI.bOnlySpectator == true || PRI.bIsSpectator == true );
		NewConsoleMessage.PlayerName	= (PRI.IsSquadLeader() ? class'avaLeaderMessage'.default.LeaderLabel : "") $" "$PRI.PlayerName;
		NewConsoleMessage.TeamIndex	= (PRI.Team == None) ? TEAM_Unknown : PRI.Team.TeamIndex;
	}


	PushBackConsoleMessage( NewConsoleMessage );
}

// 방송용 권한이 있는가?
function bool HasBroadcastAuthority()
{
`if( `isdefined(FINAL_RELEASE) )
local int Authority;
	Authority = class'avaNetHandler'.static.GetAvaNetHandler().GetCurrentChannelMaskLevel();
	if ( Authority >= 2 )
		return true;
	return false;
`endif
	return avaGameReplicationInfo(WorldInfo.GRI).bTestBroadcast;
}

function DeathMessage( avaPlayerReplicationInfo KillerPRI, avaPlayerReplicationInfo KilledPRI, string IconStr, Font font, bool bHeadshot, bool bWallshot )
{
	local SDeathMessage dm;
	local bool			BroadCastAuthority;

	BroadCastAuthority = HasBroadcastAuthority();

	dm.KillerName	= ( KillerPRI != None && KillerPRI != KilledPRI ) ? KillerPRI.PlayerName : "";
	dm.KillerTeam	= KillerPRI.Team != None ? KillerPRI.Team.TeamIndex : 255;
	dm.KillerLevel	= KillerPRI != None ? KillerPRI.Level : 0;
	dm.KillerSlot	= ( KillerPRI != None && BroadCastAuthority == true ) ? KillerPRI.SlotNum : -1;

	dm.KilledName	= KilledPRI.PlayerName;
	dm.KilledTeam	= KilledPRI.Team != None ? KilledPRI.Team.TeamIndex : 255;
	dm.KilledLevel	= KilledPRI.Level;
	dm.KilledSlot	= ( BroadCastAuthority == true ) ? KilledPRI.SlotNum : -1;

	dm.bAmIKiller	= PlayerOwner.PlayerReplicationInfo == KillerPRI;
	dm.bAmIVictim	= PlayerOwner.PlayerReplicationInfo == KilledPRI;
	
	dm.IconStr		= IconStr;
	dm.IconFont		= font;
	dm.bHeadshot	= bHeadshot;
	dm.bWallshot	= bWallshot;
	dm.DeathTime	= WorldInfo.TimeSeconds;
	DeathMessages[DeathMessages.Length] = dm;
}

function KillMessage( PlayerReplicationInfo KillerPRI, PlayerReplicationInfo KilledPRI, bool bExplosive, bool bHeadShot, bool bWallShot)
{
	Local KillMessageInfo KillMsgInfo;

	KillMsgInfo.KillerPRI = killerPRI;
	KillMsgInfo.KilledPRI = KilledPRI;
	KillMsgInfo.TimeOfDeath = WorldInfo.TimeSeconds;
	KillMsgInfo.bHeadShot = bHeadShot;
	KillMsgInfo.bWallShot = bWallShot;
	KillMsgInfo.bExplosive = bExplosive;

	KillMessageData[ KillMessageData.Length ] = KillMsgInfo;

	if ( KillerPRI != KilledPRI )
	{
		if ( ( ( PlayerOwner.PlayerReplicationInfo == KillerPRI || ( PlayerOwner.RealViewTarget != None && PlayerOwner.RealViewTarget == KillerPRI ) ) && KilledPRI != None ) || 
			 ( ( PlayerOwner.PlayerReplicationInfo == KilledPRI || ( PlayerOwner.RealViewTarget != None && PlayerOwner.RealViewTarget == KilledPRI ) ) && KillerPRI != None ) )
		{
			MyKillMessageData[ MyKillMessageData.length ] = KillMsgInfo;
		}
	}

	if( KilledPRI != None )
	{
		DeadPlayerList.Length = DeadPlayerList.Length + 1;
		DeadPlayerList[ DeadPlayerList.Length - 1].PRI = KilledPRI;
		DeadPlayerList[ DeadPlayerList.Length - 1].Location = avaPlayerReplicationInfo(KilledPRI).GetAvaPawn().Location;
		DeadPlayerList[ DeadPlayerList.Length - 1].UpdateTime = WorldInfo.TimeSeconds;
	}
}

function ClearGameInfoMessage( int nType )
{
	GameInfo[nType].GameInfoMessageCode		= -1;
	GameInfo[nType].GameInfoMessageTxt		= "";
	GameInfo[nType].GameInfoMessageSetTime	= 0.0;
	GameInfo[nType].GameInfoMessageTime		= 1.0;
}

function GameInfoMessage( coerce string Info, optional float lifeTime = 3.0, optional int code = 0, optional int nType = 0 )
{
	if ( nType >= 0 && nType <= 2 )	// 일단은 GameInfoMessage 는 두가지 type 이 있다...
	{
		GameInfo[nType].GameInfoMessageSetTime = WorldInfo.TimeSeconds;
		GameInfo[nType].GameInfoMessageCode = code;
		GameInfo[nType].GameInfoMessageTxt	= Info;
		if ( lifeTime > 0.0 )	GameInfo[nType].GameInfoMessageTime	= WorldInfo.TimeSeconds + lifeTime;	
		else					GameInfo[nType].GameInfoMessageTime = 0;
	}
}

simulated function AddTarget( Actor actor, optional BYTE TeamIdx = 255 )
{
	TargetActor		=	actor;
	TargettedTime	=	WorldInfo.TimeSeconds;;
//	HUDDisplayedActors.length = 0;
//	AddActorToHUD( actor, TeamIdx );
}

simulated function AddActorToRadar( Actor actor	, optional BYTE TeamIdx = 255, optional int IconCode = 0, optional bool bOnlyLeader, optional bool bAdjustRotate )
{
	local	RadarDisplayedActor	rda;
	local	int					i;

	`log( "avaHUD.AddActorToRadar" @actor @TeamIdx @IconCode @bOnlyLeader @bAdjustRotate );

	for ( i = 0 ; i < RadarDisplayedActors.length ; ++ i )
	{
		if ( RadarDisplayedActors[i].DisplayedActor == actor )
		{
			RadarDisplayedActors[i].TeamIndex	= TeamIdx;
			RadarDisplayedActors[i].IconCode	= IconCode;
			RadarDisplayedActors[i].bOnlyLeader	= bOnlyLeader;
			return;
		}
	}
	rda.DisplayedActor	= actor;
	rda.TeamIndex		= TeamIdx;
	rda.IconCode		= IconCode;
	rda.bOnlyLeader		= bOnlyLeader;
	rda.bAdjustRotate	= bAdjustRotate;
	RadarDisplayedActors[ RadarDisplayedActors.length ] = rda;
}

simulated function RemoveActorFromRadar( Actor actor )
{
	local int i;
	for ( i = 0 ; i < RadarDisplayedActors.length ; ++  i )
	{
		if ( RadarDisplayedActors[i].DisplayedActor == actor )
		{
			RadarDisplayedActors.Remove( i, 1 );
			return;
		}
	}
}

simulated function ResetDamageTime()
{
	DamageindicatorData.Length = 0;
}

// 라운드가 종료 되었다. Reset 할 거 있으면 여기서 하자....
simulated function RoundEnded()
{
	// Damage Indicator 를 Reset 해 준다.
	DeadPlayerList.Length = 0;		// [11/06/2006 YTS] DeadList를 Round가 끝나면 비운다.
	ResetDamageTime();
	BombAlertBlink = false;
	AddTarget( None );
}

function DisplayConsoleMessages();

// 초기화, Pawn이 Spawn될 당시의 Flag위치 조정 (안하면 신규진입자는 Flag가 안나옴)
//function ResetFlagPosition()
//{
//	Local PlayerController P , PC;
//
//	foreach LocalPlayerControllers(P)
//		PC = P;
//
//	Assert(PC != None);
//	avaPlayerController(PC).NotifyModifiedWaypoint();
//}

/* Indicator (Waypoint, QuickChat메세지 등등) */
function AddQuickChatIndicator(avaPawn P , bool bAll = false)
{
	if( P == PawnOwner)
		return;

	`log( "AddQuickChatIndicator" @P @bAll );

	QuickChatIndicatorBuffer.Length = QuickChatIndicatorBuffer.Length + 1;
	QuickChatIndicatorBuffer[QuickChatIndicatorBuffer.Length - 1].Pawn = P;
	QuickChatIndicatorBuffer[QuickChatIndicatorBuffer.Length - 1].Life = WorldInfo.TimeSeconds + 3.0;
	QuickChatIndicatorBuffer[QuickChatIndicatorBuffer.Length - 1].Type = bAll ? INDICATORTYPE_QuickChat_All : INDICATORTYPE_QuickChat_Team;

	// 퀵챗을 레이더에 원하는 시간만큼 찍어주려고 한다. 
	// 따라서 퀵챗이 발생한 시간을 QuickChat을 발생시킨 Pawn이 기록해두면 된다.
	if( P != None )
	{
		P.QuickChatUpdateTime = WorldInfo.TimeSeconds;
	}
}

function CollectQuickChatIndicatorInfo()
{
	Local string				Msg;
	Local float					Dist, Angle;
	Local EIndicatorScreenArea	ISA;
	Local int					i, nTimeLeft;
	Local Color					QuickChatColor;
	Local vector				QCLoc, ScreenLoc;
	local Pawn					SenderPawn;
	local PlayerReplicationInfo	SenderPRI,OwnerPRI;

	OwnerPRI = PlayerOwner.PlayerReplicationInfo;
	for( i = 0 ; i < QuickChatIndicatorBuffer.Length ; i ++)
	{
		SenderPawn = QuickChatIndicatorBuffer[i].Pawn;
		SenderPRI  = QuickChatIndicatorBuffer[i].Pawn.PlayerReplicationInfo;

		if( SenderPawn == None || SenderPRI == None )
			continue;

		if( QuickChatIndicatorBuffer[i].Type == INDICATORTYPE_QuickChat_Team )
		{
			if ( SenderPRI.Team == None || OwnerPRI.Team == None || SenderPRI.Team.TeamIndex != OwnerPRI.Team.TeamIndex )
				continue;
		}


		// TimeLeft in milli-seconds
		nTimeLeft = (QuickChatIndicatorBuffer[i].Life - WorldInfo.TimeSeconds) * 1000;

		if( QuickChatIndicatorBuffer[i].Life < WorldInfo.TimeSeconds )
		{
			QuickChatIndicatorBuffer.Remove(i--, 1);
			continue;
		}
		else if( nTimeLeft % 700 > 700/2 )
		{
			// Skip for Blink Message
			continue;
		}

		QCLoc		= SenderPawn.GetPawnViewLocation();
		ScreenLoc	= GetIndicatorScreenLocation(QCLoc, Dist, ISA, Angle);

		if( VSize(ScreenLoc) == 0)
			continue;

		Msg				= SenderPRI.PlayerName;
		QuickChatColor	= QuickChatIndicatorBuffer[i].Type == INDICATORTYPE_QuickChat_All ? QuickChatTauntColor : QuickChatIndicatorColor;
		AdjustIndicatorAlphaColor(QuickChatColor, Angle);

		AppendIndicator(Msg, 0, QuickChatColor, ScreenLoc, ISA, Dist, QuickChatIndicatorBuffer[i].Type);
	}
}


// 위치조정, 매 렌더링 마다 WaypointIndicator를 재설정한다.
//function CollectWaypointIndicatorInfo()
//{
//	Local avaGameReplicationInfo GRI;
//	Local float Angle, Dist;
//	Local int i;
//	Local string Msg;
//	Local EIndicatorScreenArea ISA;
//	Local Color WaypointColor;
//	Local vector ScreenLoc, WaypointLocation;
//
//	GRI = avaGameReplicationInfo(WorldInfo.GRI);
//
//	for(i = 0 ; i < WPTeam_MAX ; i++)
//	{
//		if( GRI.IsEmptyWaypoint(EWaypointTeamType(i)) )
//			continue;
//
//		WaypointLocation = GRI.GetWaypoint(EWaypointTeamType(i));
//		ScreenLoc = GetIndicatorScreenLocation(WaypointLocation, Dist, ISA, Angle);
//
//		if( VSize(ScreenLoc) == 0 )
//			continue;
//
//		Msg = "WP"$(i+1)$"( "$int(Dist/30)$"M )";
//		AdjustIndicatorAlphaColor(WaypointColor , Angle);
//		AppendIndicator(Msg, 0, WaypointColor, ScreenLoc, ISA, Dist, i == WPTeam_Blue ? INDICATORTYPE_Waypoint1 : INDICATORTYPE_Waypoint2);
//	}
//}

function vector GetIndicatorScreenLocation( vector WorldLocation , out float _Dist, out EIndicatorScreenArea _ISA, out float _Angle)
{
	Local vector ViewPos, ViewDir,WPDirAbs, WPDirRel, WPRotDir,WPLoc, ScreenLoc;
	Local vector WPDirAbs2D, ViewDir2D;
	Local rotator ViewRot, DiffRot;
	Local float HalfFov;

	ViewPos = PawnOwner.GetPawnViewLocation();
	ViewRot = PawnOwner.GetViewRotation();
	ViewDir = vector(ViewRot);

	if( VSize(WorldLocation) == 0.0 )
		return vect(0,0,0);

	WPLoc = WorldLocation;
	WPDirAbs = WPLoc - ViewPos;
	WPDirRel = WPDirAbs << ViewRot;
	_Dist = VSize(WPDirAbs);


	HalfFov = PlayerOwner.GetFovAngle()/2;
	WPDirAbs2D = WPDirAbs;
	WPDirAbs2D.Z = 0;
	ViewDir2D = ViewDir;
	ViewDir2D.Z = 0;
	_Angle = ACos( Normal(WPDirAbs2D) Dot Normal(ViewDir2D) ) * 180 / 3.141592;
	_Angle -= HalfFov;

	ScreenLoc = Canvas.Project(WPLoc);


	// When the Waypoint is in a Invisible Region.
	if( _Angle >= 0)
	{
		_ISA = (WPDirRel.Y > 0 ? ISA_RIght : ISA_Left);
		DiffRot.Yaw =  ( WPDirRel.Y > 0 ? 1 : -1 )* (65535/360) * _Angle;
		WPRotDir = WPDirAbs << DiffRot;
		ScreenLoc = Canvas.Project(ViewPos + WPRotDir);
		ScreenLoc.X = (WPDirRel.Y > 0 ? Canvas.ClipX - 30 : 30.0);
	}
	// When the Waypoint is in Visible Region.
	else
	{
		//  In Screen
		if( (15 <= ScreenLoc.Y && (ScreenLoc.Y <= Canvas.ClipY - 15)) &&
			(15 <= ScreenLoc.X && (ScreenLoc.X <= Canvas.ClipX - 15)) )
		{
			_ISA = ISA_Center;
		}
		// Above Screen Or Below Screen.
		else if ( ScreenLoc.Y < 15 || ScreenLoc.Y > (Canvas.ClipY - 15))
		{
			_ISA = WPDirRel.Z > 0 ? ISA_top : ISA_Bottom;
			ScreenLoc.Y = WPDirRel.Z > 0 ? 15.0 : Canvas.ClipY - 15.0;
		}
		else
			return vect(0,0,0);
	}

	return ScreenLoc;
}

function AdjustIndicatorAlphaColor(out Color _Color, float Angle)
{
	Local float HalfFOV;

	HalfFOV = PlayerOwner.GetFOVAngle()/2;
	if(Angle >= 0)
		_Color.A = _Color.A * (1 - (( Angle / (180 - HalfFov)) * 0.6));
}

function AppendIndicator( string Msg, float Life ,Color c, vector Pos, EIndicatorScreenArea ISA, float Dist, EIndicatorScreenType Type,optional bool bBlink = false)
{
	`log( "AppendIndicator" @Msg @Life @ISA @Type );

	Indicators.Length = Indicators.Length + 1;
	Indicators[Indicators.Length - 1].Text = Msg;
	Indicators[Indicators.Length - 1].Life = WorldInfo.TimeSeconds + Life;
	Indicators[Indicators.Length - 1].Color = c;
	Indicators[Indicators.Length - 1].Pos = Pos;
	Indicators[Indicators.Length - 1].Text = Msg;
	Indicators[Indicators.Length - 1].ScreenArea = ISA;
	Indicators[Indicators.Length - 1].Distance = Dist;
	Indicators[Indicators.Length - 1].bBlink = bBlink;
	Indicators[Indicators.Length - 1].Type = Type;
}

// 드로잉, 매 렌더드로잉마다 Indicator를 재조정
function DrawIndicators()
{
	CollectQuickChatIndicatorInfo();
}

function ShowHUD(optional bool bDefaultHUD = true)
{
	bShowHUD = bDefaultHUD;
}

// New Hud 관련...
function OpenScene( UIScene Scene, optional out UIScene SceneInstance )
{
	local UIInteraction UIController;
	if ( SceneInstance != None )	return;
	
	UIController = LocalPlayer(PlayerOwner.Player).ViewportClient.UIController;	
	if ( UIController != None )
	{
		UIController.OpenScene( Scene,, SceneInstance );
	}

	SceneInstances.length = SceneInstances.length + 1;
	SceneInstances[SceneInstances.length-1] = SceneInstance;

	if ( !bShowHud )
	{
		SceneInstance.SetVisibility( false );	
	}
}

function CloseScene( UIScene SceneInstance )
{
	local UIInteraction UIController;
	if ( SceneInstance == None )	return;
	UIController = LocalPlayer(PlayerOwner.Player).ViewportClient.UIController;	
	if ( UIController != None )
		UIController.CloseScene( SceneInstance );
}

// @ deprecated
//exec function OpenQuitMenu()
//{
//	// WorldInfo.Game이 있고(호스트이고) 
//	// 현재 라운드가 웜업라운드를 제외한 라운드중 두번째 라운드이거나 
//	// 라운드가 시작한지 2분이 지난 상태이면
//	//if( WorldInfo.Game == None || 
//	//	(avaGameReplicationInfo(WorldInfo.GRI).CurrentRound > 1 || WorldInfo.TimeSeconds >= HostBlockTime))
//	if( class'avaNetHandler'.static.GetAvaNetHandler().CanExitGame() ) 
//	{
//		OpenScene(QuitMenuScene);
//	}
//	else
//	{
//		OpenScene(QuitBlockScene);
//	}
//}

// @ deprecated
//function OpenKickWindow( bool bForceExit , string Message)
//{
//	OpenQuitKickScene();
//	class'avaEventTrigger'.static.ActivateEventByClass(class'avaUIEvent_SetPopupMessage', Message, int(bForceExit));
//}
//exec function OpenQuitKickScene()
//{
//	OpenScene(QuitKickScene, QuitKickSceneInstance);
//}
//
//exec function CloseQuitKickScene()
//{
//	if( QuitKickSceneInstance != None )
//	{
//		CloseScene(QuitKickSceneInstance);
//	}
//}

exec function OpenClassSelectScene()
{
	if ( ClassSelectSceneInstance.IsVisible() )	return;
	ClassSelectSceneInstance.SetVisibility(true);
	ClassSelectSceneInstance.Opacity = 0.0;
}

exec function CloseClassSelectScene()
{
	ClassSelectSceneInstance.SetVisibility(false);
}

//function OnSceneDeactivated( UIScene DeactivatedScene )
//{
//	if ( DeactivatedScene == QuitMenuSceneInstance )
//		QuitMenuSceneInstance = None;
//	else if ( DeactivatedScene == QuitBlockSceneInstance )
//		QuitBlockSceneInstance = None;
//}

exec function ToggleHUD()
{
	Super.ToggleHUD();

	if ( bShowHUD )
	{
		if ( GameInfoSceneInstance != None )
			GameInfoSceneInstance.SetVisibility( true );
		HudSceneInstance.SetVisibility(true);
	}
	else
	{
		if ( GameInfoSceneInstance != None )
			GameInfoSceneInstance.SetVisibility( false );
		HudSceneInstance.SetVisibility(false);	
	}
}


simulated state DefaultHUD
{
	simulated function BeginState( name PrevState )
	{
		if ( bShowHUD )		HudSceneInstance.SetVisibility(true);
		ProcessOptionalScene();
	}

	simulated function EndState( name NextState )
	{
	}

	exec function ToggleHUD()
	{
		global.ToggleHUD();

		ProcessOptionalScene();
	}
}

simulated function SetObserverKeyBinding()
{
	// Channel 이 방송용 Channel 인가?
	// Staff 인가?
	// Check 해서 적용해야함...
	if ( HasBroadCastAuthority() )
	{
		avaPlayerInput( PlayerOwner.PlayerInput ).SetObserverKeyBinding();	
	}
}

simulated function UpdateSpectatorInfo( bool bOnlySpectator )
{
	if ( bOnlySpectator )
	{
		SetObserverKeyBinding();
	}
}

auto simulated state Spectator
{
	simulated function BeginState( name PrevState )
	{
		if ( bShowHUD )
		{
			if ( PlayerOwner.PlayerReplicationInfo.bOnlySpectator )
			{
				ObserverSceneInstance.SetVisibility(true);
			}
			else
			{
				SpectatorSceneInstance.SetVisibility(true);
			}
		}

		ProcessOptionalScene();
	}

	simulated function EndState( name NextState )
	{
		ObserverSceneInstance.SetVisibility(false);
		SpectatorSceneInstance.SetVisibility(false);
		if ( RespawnSceneInstance != None )
			RespawnSceneInstance.SetVisibility( false );
	}

	simulated function UpdateSpectatorInfo(  bool bOnlySpectator  )
	{
		global.UpdateSpectatorInfo( bOnlySpectator );

		ObserverSceneInstance.SetVisibility(false);
		SpectatorSceneInstance.SetVisibility(false);	
		if ( bShowHUD )
		{
			if ( bOnlySpectator )
			{
				ObserverSceneInstance.SetVisibility(true);
			}
			else
			{
				SpectatorSceneInstance.SetVisibility(true);
			}
		}
	}

	exec function ToggleHUD()
	{
		global.ToggleHUD();
		if ( bShowHUD )		
		{
			if ( PlayerOwner.PlayerReplicationInfo.bOnlySpectator )
			{
				ObserverSceneInstance.SetVisibility(true);
			}
			else
			{
				SpectatorSceneInstance.SetVisibility(true);
			}
		}
		else
		{
			ObserverSceneInstance.SetVisibility(false);
			SpectatorSceneInstance.SetVisibility(false);	
		}
		ProcessOptionalScene();
	}
}

// Game Mode 가 Respawn 이 가능한지 Check 해서 Respawn Scene을 보여줄지 안보여줄지를 결정한다.
simulated function CheckRespawnScene()
{
	if ( RespawnSceneInstance == None )								return;
	if ( !avaGameReplicationInfo( WorldInfo.GRI ).bReinforcement )	return;
	RespawnSceneInstance.SetVisibility( avaPlayerController( PlayerOwner ).CanRestart() );
}

simulated state Dead
{
	simulated function BeginState( name PrevState )
	{
		DeathSceneStartTime	= WorldInfo.TimeSeconds;
		if ( bShowHUD )		DeathSceneInstance.SetVisibility(true);
		ProcessOptionalScene();
	}

	simulated function EndState( name NextState )
	{
		DeathSceneInstance.SetVisibility(false);
	}

	exec function ToggleHUD()
	{
		global.ToggleHUD();
		if ( bShowHUD )		DeathSceneInstance.SetVisibility(true);
		else				DeathSceneInstance.SetVisibility(false);	
		ProcessOptionalScene();
	}
}

exec function OpenHudScene()
{
	if ( !IsInState( 'DefaultHUD' ) )
		GotoState( 'DefaultHUD' );
}

exec function OpenSpectatorScene()
{
	if ( !IsInState( 'Spectator' ) )
	{
		GotoState( 'Spectator' );
	}
}

exec function OpenDeathScene()
{
	if ( !IsInState( 'Dead' ) )
		GotoState( 'Dead' );	
}

`devexec function ToggleObserverScene()
{
	if ( ObserverSceneInstance.IsVisible() )
	{
		ObserverSceneInstance.SetVisibility( false );
	}
	else
	{
		ObserverSceneInstance.SetVisibility( true );
	}
}

exec function ToggleScore()
{
	if ( ScoreSceneInstance.IsVisible() )
	{
		ScoreSceneInstance.SetVisibility( false );
	}
	else
	{
		ScoreSceneInstance.SetVisibility( true );
	}
}

exec function ShowScores()
{
	if ( ScoreSceneInstance == None )
		OpenPostScene();
	ScoreSceneInstance.SetVisibility(true);
}

exec function HideScores()
{
	ScoreSceneInstance.SetVisibility(false);
}

//[2006/11/15 YTS, insert] open chatscene for 'Say','TeamSay'
//exec function OpenChatScene()
//{
//	CloseTeamChatScene();
//	ChatSceneInstance.SetSceneInputMode( INPUTMODE_Locked );
//	ChatSceneInstance.SetVisibility(true);
//}
//
//exec function CloseChatScene()
//{
//	ChatSceneInstance.SetSceneInputMode( INPUTMODE_None );
//	ChatSceneInstance.SetVisibility(false);
//}

//exec function OpenTeamChatScene()
//{
//	CloseChatScene();
//	TeamChatSceneInstance.SetSceneInputMode( INPUTMODE_Locked );
//	TeamChatSceneInstance.SetVisibility(true);
//}
//
//exec function CloseTeamChatScene()
//{
//	TeamChatSceneInstance.SetSceneInputMode( INPUTMODE_None );
//	TeamChatSceneInstance.SetVisibility(false);
//}


//[2006/11/15 YTS]

//[2006/11/16 YTS, insert] open ColorCorrectionScene
exec function OpenColorCorrectionScene()
{
	if( ColorCorrectionSceneInstance != None )
		return;
	OpenScene(ColorCorrectionScene, ColorCorrectionSceneInstance);
}

exec function CloseColorCorrectionScene()
{
	CloseScene( ColorCorrectionSceneInstance );
	ColorCorrectionSceneInstance = None;
}

exec function ToggleColorCorrectionScene()
{
	if( ColorCorrectionSceneInstance != None )
		CloseColorCorrectionScene();
	else
		OpenColorCorrectionScene();
}
//[2006/11/16 YTS]

/** UpdateOptionalScenes 
 *
 *	SWGAME으로 부터 OptionalScene이 새로 업데이트 되었음을 알면. 그에 해당하는 실제 Scene Instance를 만들어줘야한다.
 *  새로 생긴 Scene Instance와 식별자로서의 Name을 보관한다.
 */
simulated function UpdateOptionalScenes()
{
	Local avaGameReplicationInfo GRI;
	Local UIScene Scene, SceneInstance;
	Local int i, index;
	Local bool bUpdate;

	GRI = avaGameReplicationInfo( WorldInfo.GRI );

	// GRI가 없거나 업데이트된 Scene들이 없으면 그냥 지나간다.
	if( GRI == None )
		return;

	// GetUpdateOptionalScenesToggle은 이전의 GRI.bUpdate값을 반환하고 현재의 GRI.bUpdate값을 false로 전환한다.
	bUpdate = GRI.GetUpdateOptionalScenesToggle();
	if( ! bUpdate )
		return;
	
	OptionalSceneData.Length = 0;

	// 업데이트 된 Scene들을 등록
	for( i = 0 ; i < GRI.OptionalSceneCount ; i++ )
	{
		Scene = None;
		SceneInstance = None;

		Scene = UIScene(DynamicLoadObject(GRI.OptionalSceneData[i].SceneName, class'UIScene'));
		// DynamicLoadObject<UIScene> 실패
		if( Scene == None )
		{
			`warn("DynamicLoadObject("$GRI.OptionalSceneData[i].SceneName$") failed");
			continue;
		}
		OpenScene(Scene,SceneInstance);
		
		// Opening a Scene 실패
		if( SceneInstance == None )
		{
			`warn("OpenScene("$Scene$") failed");
			continue;
		}

		index = OptionalSceneData.Length;
		OptionalSceneData.Length = OptionalSceneData.length + 1;
		OptionalSceneData[ index ].SceneData = GRI.OptionalSceneData[i];
		OptionalSceneData[ index ].SceneInstance = SceneInstance;
		OptionalSceneData[ index ].Scene = Scene;
	}

	// 새로 갱신된 OptionalScene들을 Refresh (현재 상태에 따라 지울것은 지우고 보여줄것은 보여준다)
	ProcessOptionalScene();

	OpenPostScene();
}

simulated function ProcessOptionalScene()
{
	Local int i;
	Local EHUDStateType HUDState;
	local bool	bShow;

	HUDState = HUDSTATE_MAX;
	for( i = 0 ; i < OptionalSceneData.Length ; i++ )
	{
		if( OptionalSceneData[i].SceneInstance != None )
		{
			if( GetStateName() == 'DefaultHUD' )
				HUDState = HUDSTATE_DefaultHUD;
			else if ( GetStateName() == 'Spectator')
				HUDState = HUDSTATE_Spectator;
			else if ( GetStateName() == 'Dead')
				HUDState = HUDSTATE_Dead;
			
			if ( bShowHUD == false )	
			{
				bShow = false;
			}
			else
			{
				bShow = OptionalSceneData[i].SceneData.OpenStatus[HUDState].bOpen;
			}

			if( HUDState != HUDSTATE_MAX )
			{
				OptionalSceneData[i].SceneInstance.SetVisibility( bShow );
			}
		}
	}
}

/** FullScreenMap을 위한 exec functions */
exec function ShowFullScreenMap()
{
	bShowFullScreenMap = true;
}
exec function HideFullScreenMap()
{
	bShowFullScreenMap = false;
}
exec function ToggleFullScreenMap()
{	
	bShowFullScreenMap = !bShowFullScreenMap;
}

simulated function UpdateWarmupRound()
{
	local bool bWarmUpRound;
	if ( WorldInfo.GRI == None )	return;
	bWarmUpRound = avaGameReplicationInfo( WorldInfo.GRI ).bWarmupRound;
	if ( bWarmUpRound == FALSE )	
		avaPlayerController( PlayerOwner ).ClearGIM( 1 );
	else
		ShowMissionHelp();
	WarmupSceneInstance.SetVisibility( bWarmUpRound );
}

// WarmUp Round 중 GameInfoMessage 영역에 Mission에 대한 Help 를 보여주도록 한다...
simulated function ShowMissionHelp()
{
	local int						nTeamNum;
	local avaGameReplicationInfo	GRI;
	nTeamNum	= PlayerOwner.GetTeamNum();
	GRI			= avaGameReplicationInfo(WorldInfo.GRI);
	if ( GRI.bWarmupRound == FALSE )	return;
	if ( nTeamNum == 0 || nTeamNum == 1 )
	{
		if ( GRI.MissionHelp[nTeamNum] != -1 )
			avaPlayerController( PlayerOwner ).ShowGIM( GRI.MissionHelp[nTeamNum], 1, 0.0 );
	}
}

exec function TogglePlayerDetail()
{
	bShowPlayerDetail = !bShowPlayerDetail;
}

// 호스트 이전중에 긴급하게 보내는 채팅 메세지
exec function AddEmergencyChatMessage( string Msg, int AccountID, bool bTeamSay )
{
	Local avaGameReplicationInfo GRI;
	Local avaPlayerReplicationInfo PRI;
	Local int PRIIndex;
	Local int FindIndex;

	GRI = avaGameReplicationInfo(WorldInfo.GRI);
	if( GRI == none )
	{
		return;
	}

	FindIndex = INDEX_NONE;
	for( PRIIndex = 0 ; PRIIndex < GRI.PRIArray.Length ; PRIIndex++ )
	{
		PRI = avaPlayerReplicationInfo(GRI.PRIArray[PRIIndex]);
		if( PRI != none && PRI.AccountID == AccountID)
		{
			FindIndex = PRIIndex;
			break;
		}
	}

	if( FindIndex != INDEX_NONE )
	{
		AddConsoleMessageEx( Msg, class'LocalMessage' , GRI.PRIArray[FindIndex], 0.0, bTeamSay ? 'TeamSay' : 'Say');
	}
}

exec function AddTypedMessage( string Msg , EChatMsgType MsgType)
{
	Local int FindIndex;
	Local avaGameReplicationInfo GRI;
	Local name MsgTypeName;

	GRI = avaGameReplicationInfo(WorldInfo.GRI);
	FindIndex = GRI.TextStyleData.Find('id', int(MsgType));

	if( GRI == none )
	{
		`warn("There's no GRI in avaHUD @ AddTypedMessage");
		return;
	}
	if( FindIndex == INDEX_NONE )
	{
		FindIndex = GRI.TextSTyleData.Find('id',0);
	}
	if( FindIndex == INDEX_NONE )
	{
		`warn("There's no default text-style in GRI.TextStyleData");
		return;
	}

	MsgTypeName = GRI.TextStyleData[FindIndex].Name;
	AddConsoleMessageEx( Msg, class'LocalMessage' , None, 0.0, MsgTypeName);
}

exec function AddChatOffMessage( float TimeLeft , optional bool bFirstNotify)
{
	Local int Minutes, Seconds;
	Local string FullMessage;
	Minutes = (int(TimeLeft)) / 60;
	Seconds = (int(TImeLeft)) % 60;

	if( bFirstNotify )
		FullMessage = Localize("UIMiscScene", "Text_Chat_BlockedByAdmin", "AVANET");
	FullMessage @= ChatOffMessage @ Minutes @ ChatOffMinute @ Seconds @ ChatOffSecond;
	AddTypedMessage( FullMessage, EChat_PlayerSystem );
}

exec function AddGameMessage( string Message )
{
	AddTypedMessage( Message, EChat_InGameSystem );
}

exec function UpdateRTNotice(string Msg)
{
	RTNoticeMessage = Msg;
	RTNoticeUpdateTime = WorldInfo.TimeSeconds;
}

static function DLO( string resource )
{
	if (resource != "")
	{		
		DynamicLoadObject( resource, class'Object' );
	}
}

static event LoadDLOs()
{
}

function SetSignalPos( vector Pos )
{
	ClearSignal();
	bSignalActivate	=	true;
	SignalPos		=	Pos;
	SetTimer( 5.0, false, 'ClearSignal' );
}

function ClearSignal()
{
	ClearTimer( 'ClearSignal' );
	bSignalActivate =	false;
}

function AddInGameHelp( string Help )
{
	InGameHelp = Help;
	ClearTimer( 'ClearInGameHelp' );
	SetTimer( 4.0, false, 'ClearInGameHelp' );
}

function ClearInGameHelp()
{
	InGameHelp = "";
}

/** functions related with ConsoleMessages */
function PushBackConsoleMessage( ConsoleMessage NewConsoleMessage )
{
	if( ConsoleMessages.Length < ConsoleMessageCount )
		ConsoleMessages.Add(ConsoleMessageCount - ConsoleMessages.Length);

	if( ConsoleMessageSize == ConsoleMessageCount )
	{
		assert(ConsoleMessageInitIndex == ConsoleMessageEndIndex);
		ConsoleMessages[ConsoleMessageInitIndex] = NewconsoleMessage;
		ConsoleMessageInitIndex = (ConsoleMessageInitIndex + 1) % ConsoleMessageCount;
		ConsoleMessageEndIndex = ConsoleMessageInitIndex;
		UpdateConsoleMessageThrowOuts++;
	}
	else
	{
		ConsoleMessages[ConsoleMessageEndIndex] = NewConsoleMessage;
		ConsoleMessageEndIndex = (ConsoleMessageEndIndex + 1) % ConsoleMessageCount;
		ConsoleMessageSize++;
	}

	UpdateConsoleMessageCount++;
}

function PopFrontConsoleMessage()
{
	if( ConsoleMessageSize == 0 )
	{
		assert(ConsoleMessageInitIndex == ConsoleMessageEndIndex);
		return;
	}
	else
	{
		ConsoleMessageInitIndex = (ConsoleMessageInitIndex + 1) % ConsoleMessageCount;
		ConsoleMessageSize--;
	}
}

function int GetConsoleMessageSize()
{
	return ConsoleMessageSize;
}

exec function ConsoleMessagePageUp()
{
	UpdateConsoleMessagePageUpDown = 1;
}

exec function ConsoleMessagePageDown()
{
	UpdateConsoleMessagePageUpDown = -1;
}

native function float AlterScreenQuality( float Amount );

function DisplayNewScreenQuality( float NewQuality )
{
	local string	Msg;

	if ( PrevScreenQuality == NewQuality )	return;

	PrevScreenQuality = NewQuality;

	Msg	 =  class'avaStringHelper'.static.Replace( ""$int(NewQuality) , "%d", StrChangeScreenQuality );	
	
	Message(None, Msg, '');
}

exec function IncScreenQuality()
{
	DisplayNewScreenQuality( AlterScreenQuality( +1 ) );
}

exec function DecScreenQuality()
{
	DisplayNewScreenQuality( AlterScreenQuality( -1 ) );
}

exec function ChangeCameraMode( int nMode )
{
	// nMode == 0 이면 일반 Specatator Cam...
	// nMode == 1 이면 Free Cam
	// nMode == 2 이면 방송용 Cam ( Not Used )

	// FreeCam 일 경우 불필요한 UI 를 끄도록 한다...
	//if ( nMode == 1 )
	//{
	//	SetChildGroupVisibility( THROWABLEWEAPON, false, false );
	//	SetChildGroupVisibility( HEALTHGAUGE, false, false );
	//}
	//else if ( nMode == 0 )
	//{
	//	SetChildGroupVisibility( THROWABLEWEAPON, true, false );
	//	SetChildGroupVisibility( HEALTHGAUGE, true, false );
	//}
}

exec event function ToggleRadar()
{
	ToggleChildGroupVisibility( RADAR );
}

exec event function ToggleMissionUI()
{
	ToggleChildGroupVisibility( GAMETYPE_PROGRESS_TANK );
}

exec event function ToggleViewTargetUI()
{
	ToggleChildGroupVisibility( THROWABLEWEAPON );
	ToggleChildGroupVisibility( PROGRESSBAR );
	ToggleChildGroupVisibility( HUDCLOCK );
	ToggleChildGroupVisibility( HEALTHGAUGE );
	ToggleChildGroupVisibility( NVGGAUGE );
	ToggleChildGroupVisibility( TOUCHEDWEAPON );

	if ( !HasViewtarget() )
	{
		EnableChildGroupVisibility( HUDSceneInstance, THROWABLEWEAPON, false );
		EnableChildGroupVisibility( HUDSceneInstance, HEALTHGAUGE, false );
	}
}

exec event function ToggleLargeMap()
{
	if ( LargeMapSceneInstance.IsVisible() )
		LargeMapSceneInstance.SetVisibility( false );
	else
		LargeMapSceneInstance.SetVisibility( true );
}

exec event function ToggleSeeThrough()
{
	bSeeThrough = !bSeeThrough;
}

exec event function ToggleMemberInfo()
{
	ToggleChildGroupVisibility( MEMBERINFO );
}

exec event simulated function ToggleConsoleUI()
{
	ToggleChildGroupVisibility( CONSOLEMESSAGE );
}

exec event function ToggleViewtargetName()
{
	ToggleChildGroupVisibility( OBSERVE_SPECTATORINFO );
}

exec event function ToggleSpectatorHelp()
{
	ToggleChildGroupVisibility( OBSERVE_GAMEHELP );
}

function ToggleChildGroupVisibility( int nGroupID )
{
	SetChildGroupVisibility( nGroupID, bool(ControlToggleInfo[nGroupID]) );
}

function SetChildGroupVisibility( int nGroupID, bool bEnable, optional bool bSave = true )
{
	local int i;
	for ( i = 0 ; i < SceneInstances.length ; ++ i )
	{
		EnableChildGroupVisibility( SceneInstances[i], nGroupID, bEnable );
	}
	if ( bSave )
		ControlToggleInfo[nGroupID]	=	BYTE(!bEnable);
}

function EnableChildGroupVisibility( UIScene SceneInstance, int nGroupID, bool bEnable )
{
	SceneInstance.SetChildGroupVisibility( nGroupID, bEnable );
}

simulated function AddGrenadeForIndicating( avaProjectile projectile )
{
	local int index;
	index = GrenadeForIndicatingList.length;
	GrenadeForIndicatingList.length = index + 1;
	GrenadeForIndicatingList[index] = projectile;
}

simulated function RemoveGrenadeForIndicating( avaProjectile projectile )
{
	local int index;
	index = GrenadeForIndicatingList.Find( projectile );
	if ( index < 0 )	return;
	GrenadeForIndicatingList.Remove( index, 1 );
}

defaultproperties
{
	// State Scene
	HUDScene			= UIScene'avaHUD.InGame.Default'
	HUDSceneBroadcast	= UIScene'avaHUD.InGame.Default_Broadcast'


	SpectatorScene		= UIScene'avaHUD.Spectator'
	DeathScene			= UIScene'avaHUD.Death'

	// Common Scene
	//QuitMenuScene		= UIScene'avaHUD.QuitWindow'
	//QuitKickScene		= UIScene'avaHUD.InGame.QuitWindow_KickMessage'
	//QuitBlockScene		= UIScene'avaHUD.InGame.QuitWindow_WarnForHost'
	ClassSelectScene	= UIScene'avaHUD.InGameClassChange'
	
	// 미리로딩하기 위해 ChatScene DefaultObject는 그대로 남겨둠
	ChatScene			= UIScene'avaHUD.InGame.Chat'
	TeamChatScene		= UIScene'avaHUD.InGame.TeamChat'

	ScoreScene			= UIScene'avaHUD.GameStatus'
	ScoreScene_DM		= UIScene'avaHUD.GameStatus_DM'
	WarmupScene			= UIScene'avaHUD.InGame.Warmup'
	RespawnScene		= UIScene'avaHUD.ReInforcement'
	GameInfoScene		= UIScene'avaHUD.GameInfoMessage'
	ObserverScene		= UIScene'avaHUD.InGame.observe'
	ObserverSceneBroadCast = UIScene'avaHUD.InGame.observe_broadcast'
	InGameGlobalScene	= UISCene'avaHUD.InGameGlobalScene'
	LargeMapScene		= UIScene'avaHUD.Largemap'

	// Indicator


	// ColorCorrection Scene
	ColorCorrectionScene=UIScene'avaHUD.Adjust.ColorCorrection'
	HostBlockTime = 120.0

	ConsoleDefaultColor     =(R=249,G=249,B=239,A=255)
	FriendlyColor           =(R=249,G=249,B=239,A=255)
	EnemyColor              =(R=255,G=40,B=40,A=255)
	SpectatorColor          =(R=253,G=55,B=255,A=255)
	LocationColor           =(R=239,G=233,B=200,A=255)

	//TeamColor(0)
	//TeamColor(1)
	QuickChatIndicatorColor	=(R=0,G=255,B=0,A=200)
	QuickChatTauntColor		=(R=255,G=0,B=0,A=200)
	ShadowColor				=(R=0,G=0,B=0,A=255)
	ComandCenterMsgColor	=(R=128,G=0,B=0,A=255)
	ChatColor				=(R=255,G=255,B=255,A=255)

	BIAColor				=(R=86,G=223,B=36,A=255)

	// Slot Num 를 표시해주는 Icon
	SlotNumTexture		=	Texture2D'avaUICommon.SlotNo'

	// Level 을 표시해주는 Icon 입니다
	LvlTexture			=	Texture2D'avaUICommon.Level_Icon'
	ClanMarkTextures(0)	=	Texture2D'avaClanMark.Small01'
	// Leader 를 표시해주기 위한 정보들입니다.
	LeaderTexture		=	Texture2D'avaUISkinEx.ava_HUD_Image'	
	LeaderU				=	300
	LeaderV				=	492
	LeaderUL			=	19
	LeaderVL			=	19

	RadarTopTargetIcon	=	29		// Radar에 표시되는 최우선 목표 IconCode
	HUDTopTargetIcon	=	40		// HUD에 표시되는 최우선 목표 IconCode
	SignalIcon			=	21		//
	PrevScreenQuality	=	-1
	bShowHUD			=	true
}
