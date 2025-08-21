/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaGameViewportClient extends GameViewportClient
	native
	config(Game);

`include(avaGame/avaGame.uci)

/** The viewport's UI interaction. */
var avaUIInteraction	ViewportUI;
var avaQuickChatUI		QuickChatUI;
var avaVoteUI			VoteUI;

var	bool				bEnableQuickChat;
var bool				bEnableVoteUI;



var avaWaypointUI		WaypointUI;
var avaAdminUI			AdminUI;

var bool bCheckedEntry;

/** The background to use for the menu and transition screens. */
var Texture2D			LoadBackGround;

var localized string LevelActionMessages[6];

var MaterialEffect			NightVisionEffect, FlashEffect;
var DOFandBloomEffect		DOFandBloomEffect;
var MotionBlurEffect		MotionBlurEffect;
var MaterialInstanceConstant NightVisionMaterialInstance, FlashMaterialInstance;
var int						MapIndex;				// Index of  LoadBackground Image

struct native LoadingIconInfo
{
	var string					IconPath;
	var TextureCoordinates		FirstIconCoords;
	var IntPoint				IconDimension;
	var Vector2D				IconDrawXY;
	var float					AnimationPeriod;
	var int						AnimationImageCount;
};

var config array<LoadingIconInfo>		LoadingIconData;
var array<Texture2D>					LoadingIcons;
var localized string					NowLoading;


//! 로딩화면 효과를 위한.
struct native LoadingImageData
{
	var string			Resource;		//!< 이미지 리소스 이름.
	var float			U, V, UL, VL;	//!< 이미지 사용 영역.

	var Vector2D		DrawPos;		//!< 그려질 좌표.
	var Texture2D		Image;			//!< 그려질 이미지.

	structdefaultproperties
	{
		U=0
		V=0
		UL=0
		VL=0
		DrawPos=(X=0,Y=0)
	}
};

//! 로딩정보.
struct native LoadingInfo
{
	var string				MapFileName;	//!< 맵 이름.

	var LoadingImageData	Background;		//!< 배경.
	var LoadingImageData	PostImage1;		//!< 나중에 그려질 이미지1.
	var LoadingImageData	PostImage2;		//!< 나중에 그려질 이미지1.

	var bool				bProgress;		//!< 이미지(PostImage)를 프로그래스로 처리해야 하는가?

	structdefaultproperties
	{
		bProgress = false
	}
};

struct native SponsorInfo
{
	var string				Id;		// sponsor id
	var LoadingImageData	Data;
	var EChannelFlag		Channel;
};

var config array<LoadingInfo>			LoadingInfos;
var transient LoadingInfo				CurrentLoadingInfo;

var LoadingImageData					ScanLine;
var LoadingImageData					ProgressBar0;
var LoadingImageData					ProgressBar1;
var LoadingImageData					ProgressBar2;
var LoadingImageData					AVALogo;
var LoadingImageData					TipBar;
var LoadingImageData					HmAnim;
var config array<SponsorInfo>			SponsorInfos;
var array<LoadingImageData>				CurrentSponsorInfos;


`define MOTD_LENGTH 19
var localized string					MOTD[`MOTD_LENGTH];
var config int							MOTDIndex;

var localized string					HostMigrationMessage;
var bool								bIsHostMigrating;
var Texture2D							HostMigrationImage;

cpptext
{
	virtual void Draw(FViewport* Viewport,FCanvas* Canvas);
}

function OpenMapMenu()
{
	ViewportUI.bOpened = true;	
}

event bool Init(out string OutError)
{
	local int UIInteractionIndex;
	//Local avaNetHandler NetHandler;		/**< Admin인지 확인하기 위한 NetHandler  */
	//Local int ViewportConsoleIndex;

	if(!Super.Init(OutError))
	{
		return false;
	}

	//NetHandler = class'avaNetHandler'.static.GetAvaNetHandler();

	// Create the viewport's UI interaction.
	ViewportUI = new(Self) class'avaUIInteraction';
	QuickChatUI = new(Self) class'avaQuickChatUI';
	VoteUI = new(Self) class'avaVoteUI';
	//WaypointUI = new(Self) class'avaWaypointUI';
	AdminUI = new(Self) class'avaAdminUI';
	
	// Insert the UI interaction just after the console
	UIInteractionIndex = GlobalInteractions.Find(ViewportConsole) + 1;
	if ( InsertInteraction(ViewportUI, UIInteractionIndex) == -1 )
	{
		OutError = "Failed to add interaction to GlobalInteractions array:" @ ViewportUI.Class;
		return false;
	}

	if( InsertInteraction(QuickChatUI, UIInteractionIndex+1) == -1 )
	{
		outerror = "failed to add interaction to globalinteractions array:" @ quickchatui.class;
		return false;
	}
	if( InsertInteraction(VoteUI, UIInteractionIndex+1) == -1 )
	{
		OutError = "Failed to add interaction to GlobalInteractions array:" @ VoteUI.Class;
		return false;
	}
	//if( InsertInteraction(WaypointUI, UIInteractionIndex+1) == -1 )
	//{
	//	OutError = "Failed to add interaction to GlobalInteractions array:" @ WaypointUI.Class;
	//	return false;
	//}

	//ViewportConsoleIndex = GlobalInteractions.find(ViewportConsole);
	//if( ViewportConsoleIndex < 0 ) ViewportConsoleIndex = UIInteractionIndex + 1;
	//if( NetHandler != None && NetHandler.AmIAdmin() && InsertInteraction(AdminUI, ViewportConsoleIndex ) == -1 )
	//{
	//	OutError = "Failed to add interaction to GlobalInteractions array:" @ AdminUI.Class;
	//	return false;
	//}

	DOFandBloomEffect = DOFandBloomEffect( Outer.DefaultPostProcess.FindPostProcessEffect('DOFandBloom') );

	if (DOFandBloomEffect == none)
	{
		OutError = "No DOFandBloom effect found in DefaultPostProcess";
		return false;
	}

	MotionBlurEffect = MotionBlurEffect( Outer.DefaultPostProcess.FindPostProcessEffect('MotionBlur') );

	if (MotionBlurEffect == none)
	{
		OutError = "No MotionBlur effect found in DefaultPostProcess";
		return false;
	}	

	NightVisionEffect = MaterialEffect( Outer.DefaultPostProcess.FindPostProcessEffect('NightVision') );

	if (NightVisionEffect == none)
	{
		OutError = "No Nightvision effect found in DefaultPostProcess";
		return false;
	}


	NightVisionMaterialInstance = MaterialInstanceConstant( NightVisionEffect.Material );

	if (NightVisionMaterialInstance == none)
	{
		OutError = "Nightvision material isn't a material instance constant";
		return false;
	}	

	FlashEffect = MaterialEffect( Outer.DefaultPostProcess.FindPostProcessEffect('FlashEffect') );

	if (FlashEffect == none)
	{
		OutError = "No Flash effect found in DefaultPostProcess";
		return false;
	}


	FlashMaterialInstance = MaterialInstanceConstant( FlashEffect.Material );

	if (FlashMaterialInstance == none)
	{
		OutError = "Flash material isn't a material instance constant";
		return false;
	}	

	return true;
}

/**
 * Called when a new map is about to be loaded, just before the current map is GC'd.
 */
event GameSessionEnded()
{
	Super.GameSessionEnded();	

	ViewportUI.bOpened = false;
	bCheckedEntry = false;
}

event PostRender(Canvas Canvas)
{
	local avaPlayerController PC;

	if( !bCheckedEntry && TransitionType == TT_None )
	{
		if(left(Pathname(Outer.GetCurrentWorldInfo()),10)~= "EnvyEntry.")
		{
			OpenMapMenu();
		}
		bCheckedEntry = true;
	}

	if(ViewportUI.bOpened)
		ViewportUI.RenderMenu(Canvas);

	//TODO: 죽거나, Spectator일 때는 그리지 말고, 닫아야죠?
//	if( QuickVoicesIR.bOpened ) 
//		QuickVoicesIR.RenderMenu( Canvas );

	if( VoteUI.bOpened)
		VoteUI.RenderMenu( Canvas );

//	if( StrategyUI.bOpened )
//		StrategyUI.RenderMenu( Canvas );
	//if( QuickChatUI.bOpened )
	//	QuickChatUI.RenderMenu( Canvas );
	//if( VoteUI.bOpened)
	//	VoteUI.RenderMenu( Canvas );
	//if( WaypointUI.bOpened)
	//	WaypointUI.RenderMenu( Canvas );

	
	PC = avaPlayerController(GamePlayers[0].Actor);
	if ( PC != None )
	{
		PC.RestorePawnOffset();
	}

	super.PostRender(Canvas);
}

event Initialize(string MapDesc)
{
	local string MapDescLower, MapFileNameLower;
	local int i;
	local EChannelFlag CurrentChannel;

	CurrentChannel = class'avaNetHandler'.static.GetAvaNetHandler().GetCurrentChannelFlag();

	//local String LoadBackGroundString;
	
	MapDescLower = Locs(MapDesc);

	MapIndex = -1;
	for( i = 0 ; i < LoadingInfos.Length ; i++)
	{
		MapFileNameLower = Locs(LoadingInfos[i].MapFileName);
		if( InStr(MapFileNameLower, MapDescLower) >= 0) 
		{
			MapIndex = i;
			break;
		}
	}

	// sponsor
	for (i=0; i<SponsorInfos.Length; i++)
	{
		// Channel로 filtering
		if (SponsorInfos[i].Channel == CurrentChannel)
		{
			CurrentSponsorInfos[CurrentSponsorInfos.Length] = SponsorInfos[i].Data;
		}
	}

	for (i=0; i<CurrentSponsorInfos.Length; i++)
	{
		CurrentSponsorInfos[i].Image = Texture2D(DynamicLoadObject(CurrentSponsorInfos[i].Resource,class'Texture2D'));
	}

	// 로딩화면이 없는 맵은 기본 화면으로...
	if( MapIndex < 0 )
		MapIndex = LoadingInfos.find('MapFileName',"");

	if( MapIndex < 0 )
	{
		`warn("There's no default LoadBackground. Check the item [Engine.WorldInfo] of DefaultGame.ini");
		return;
	}

	// 해당 맵의 로딩화면 정보를 얻는다.
	if ( LoadingInfos[MapIndex].Background.Resource != "" )
		LoadingInfos[MapIndex].Background.Image = Texture2D(DynamicLoadObject(LoadingInfos[MapIndex].Background.Resource,class'Texture2D'));
	if ( LoadingInfos[MapIndex].PostImage1.Resource != "" )
		LoadingInfos[MapIndex].PostImage1.Image = Texture2D(DynamicLoadObject(LoadingInfos[MapIndex].PostImage1.Resource,class'Texture2D'));
	if ( LoadingInfos[MapIndex].PostImage2.Resource != "" )
		LoadingInfos[MapIndex].PostImage2.Image = Texture2D(DynamicLoadObject(LoadingInfos[MapIndex].PostImage2.Resource,class'Texture2D'));

	CurrentLoadingInfo = LoadingInfos[MapIndex];

	if ( !LoadingInfos[MapIndex].bProgress )
	{
		AVALogo.Image = Texture2D(DynamicLoadObject(AVALogo.Resource, class'Texture2D'));

		// Scanline
		ScanLine.Image = Texture2D(DynamicLoadObject(ScanLine.Resource, class'Texture2D'));

		ProgressBar0.Image = Texture2D(DynamicLoadObject(ProgressBar0.Resource,class'Texture2D'));
		ProgressBar1.Image = Texture2D(DynamicLoadObject(ProgressBar1.Resource,class'Texture2D'));
		ProgressBar2.Image = Texture2D(DynamicLoadObject(ProgressBar2.Resource,class'Texture2D'));
	}
	else
	{
		TipBar.Image = Texture2D(DynamicLoadObject(TipBar.Resource, class'Texture2D'));
	}

	if ( HmAnim.Resource != "" )
		HmAnim.Image = Texture2D(DynamicLoadObject(HmAnim.Resource, class'Texture2D'));

	LoadingIcons.Length = LoadingIconData.Length;
	for( i = 0 ; i < LoadingIconData.Length ; i++ )
	{
		LoadingIcons[i] = Texture2D(DynamicLoadObject( LoadingIconData[i].IconPath, class'Texture2D'));
	}

	MOTDIndex = ( MOTDIndex + 1 ) % `MOTD_LENGTH;
}

event Uninitialize()
{
	local int i;

	ScanLine.Image = None;

	for (i=0; i<CurrentSponsorInfos.Length; i++)
	{
		CurrentSponsorInfos[i].Image = None;
	}

	CurrentSponsorInfos.Length = 0;

	CurrentLoadingInfo.Background.Image = None;
	CurrentLoadingInfo.PostImage1.Image = None;
	CurrentLoadingInfo.PostImage2.Image = None;

	LoadingInfos[MapIndex].Background.Image = None;
	LoadingInfos[MapIndex].PostImage1.Image = None;
	LoadingInfos[MapIndex].PostImage2.Image = None;

	ProgressBar0.Image = None;
	ProgressBar1.Image = None;
	ProgressBar2.Image = None;

	AVALogo.Image = None;
	TipBar.Image = None;

	LoadBackground = none;
}

// avaLoadingScreen에서 담당합니다.
function DrawTransition(Canvas Canvas);

function NotifyPawnDied()
{
	VoteUI.NotifyPawnDied(  );
	QuickChatUI.NotifyPawnDied(  );
}

function int GetNumberKey( name Key )
{
	if( key == 'One' )
		return 1;
	else if( key == 'Two' )
		return 2;
	else if( key == 'Three' )
		return 3;
	else if( key == 'Four' )
		return 4;
	else if( key == 'Five' )
		return 5;
	else if( key == 'Six' )
		return 6;
	else if( key == 'Seven' )
		return 7;
	else if( key == 'Eight' )
		return 8;
	else if( key == 'Nine' )
		return 9;
	else if( key == 'Zero' )
		return 0;
	else
		return -1;
}

// {{ 20070108 dEAthcURe|HM
event HmReinit()
{
	local string OutError;
	
	`log("[dEAthcURe] avaGameViewportClient:HmReinit");	
	
	Init(OutError);
	`log("[dEAthcURe] " @ OutError);	
}
// }} 20070108 dEAthcURe|HM

function SetHostMigration( bool bSetHostMigration )
{
	bIsHostMigrating = bSetHostMigration;
}

// A.V.A 에서는 Split Screen을 지원하지 않는다....
event LayoutPlayers()
{
	GamePlayers[0].Size.X	=	1.0;
	GamePlayers[0].Size.Y	=	1.0;
	GamePlayers[0].Origin.X =	0.0;
	GamePlayers[0].Origin.Y =	0.0;
}

event bool InputKey(int ControllerId,name Key,EInputEvent Event, float AmountDepressed, bool bGamepad )
{
	Local int FindIndex;
	Local string CommandStr;

	FindIndex = class'avaPlayerInput'.default.Bindings.Find('Name', Key);
	if( FindIndex != INDEX_NONE  )
	{
		CommandStr = Caps(class'avaPlayerInput'.default.Bindings[FindIndex].Command);
		if( CommandStr == "SHOT" || CommandStr == "SCREENSHOT" || CommandStr == "TILEDSHOT")
		{
			ViewportConsole.ConsoleCommand(CommandStr);
			return true;
		}
	}
	return Super.InputKey( ControllerId, Key, Event, AmountDepressed, bGamepad );
}

defaultproperties
{
	bEnableQuickChat	=	true
	bEnableVoteUI		=	true

	ScanLine=(Resource="avaUI.loading_scanbar",U=0,V=0,UL=128,VL=256)

//	LoadingInfos.Add( (MapFileName="SW-BO.ut3",Background=(Resource="avaUI.loading_BO",U=0,V=0,UL=1024,VL=768)) )//,PostImage1=(Resource="avaUI.loading_BO_ammo1",U=0,V=0,UL=256,VL=256,DrawPos=(X=0,Y=407)),PostImage2=(Resource="avaUI.loading_BO_ammo2",U=0,V=0,UL=512,VL=256,DrawPos=(X=283,Y=0))) )
//	LoadingInfos.Add( (MapFileName="SW-BO2.ut3",Background=(Resource="avaUI.loading_BO_night",U=0,V=0,UL=1024,VL=768)) )//,PostImage1=(Resource="avaUI.loading_BO_night_ammo",U=0,V=0,UL=256,VL=256,DrawPos=(X=768,Y=512)),PostImage2=(Resource="avaUI.loading_BO_dogtac",U=0,V=0,UL=256,VL=512,DrawPos=(X=0,Y=100))) )
//	LoadingInfos.Add( (MapFileName="SW-kkado.ut3",Background=(Resource="avaUI.loading_CH",U=0,V=0,UL=1024,VL=768)) )//,PostImage1=(Resource="avaUI.loading_CH_ammo",U=0,V=0,UL=256,VL=512,DrawPos=(X=0,Y=269)),PostImage2=(Resource="avaUI.loading_CH_dogtac",U=0,V=0,UL=512,VL=256,DrawPos=(X=249,Y=0))) )
//	LoadingInfos.Add( (MapFileName="SW-FH_re.ut3",Background=(Resource="avaUI.loading_FH",U=0,V=0,UL=1024,VL=768)) )//,PostImage1=(Resource="avaUI.loading_FH_ammo",U=0,V=0,UL=512,VL=512,DrawPos=(X=0,Y=227))) )
//	LoadingInfos.Add( (MapFileName="SW-HammerRE.ut3",Background=(Resource="avaUI.loading_HB",U=0,V=0,UL=1024,VL=768)) )//,PostImage1=(Resource="avaUI.loading_HB_ammo1",U=0,V=0,UL=256,VL=256,DrawPos=(X=0,Y=304)),PostImage2=(Resource="avaUI.loading_HB_ammo2",U=0,V=0,UL=512,VL=256,DrawPos=(X=274,Y=0))) )
//	LoadingInfos.Add( (MapFileName="SW-SnakeEye.ut3",Background=(Resource="avaUI.loading_SE",U=0,V=0,UL=1024,VL=768)) )//,PostImage1=(Resource="avaUI.loading_SE_ammo",U=0,V=0,UL=512,VL=256,DrawPos=(X=314,Y=0)),PostImage2=(Resource="avaUI.loading_SE_dogtac",U=0,V=0,UL=512,VL=512,DrawPos=(X=0,Y=173))) )
//	LoadingInfos.Add( (MapFileName="",Background=(Resource="avaUI.loading_ava",U=0,V=0,UL=480,VL=160),PostImage1=(Resource="avaUI.loading_ava_bar",U=0,V=0,UL=326,VL=15,DrawPos=(X=153,Y=146)),bProgress=true) )

	ProgressBar0=(Resource="avaUI.loading_bar_left",U=0,V=0,UL=128,VL=128,DrawPos=(X=48,Y=80))
	ProgressBar1=(Resource="avaUI.loading_bar_center",U=0,V=0,UL=128,VL=128,DrawPos=(X=0,Y=0))
	ProgressBar2=(Resource="avaUI.loading_bar_right",U=0,V=0,UL=128,VL=128,DrawPos=(X=0,Y=80))

	AVALogo=(Resource="avaUI.loading_avalogo",U=0,V=0,UL=181,VL=59,DrawPos=(X=13,Y=8))
	TipBar=(Resource="avaUI.loading_ava_tip",U=0,V=0,UL=512,VL=128,DrawPos=(X=0,Y=638))
	HmAnim=(Resource="avaUICommon.HM_Ani",U=0,V=0,UL=172,VL=107,DrawPos=(X=462,Y=330))
}
