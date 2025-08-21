//=============================================================================
//  avaVolume_BuildBox
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
//  
//	2006/05/11 by OZ
//		BuildBox 를 위한 Volume 이다.
//		avaSeqEvent_BuildBox 와 연동한다.
//
//	ToDo.
//		Volume 에 진입했을때 EntryMessage를 찍어주도록 한다. (UI부분이 확정되면....)
//		ET 의 Stamina 개념이 들어간다면 처리해 주도록 한다.
//
//=============================================================================
class avaVolume_BuildBox extends TriggerVolume;

struct IconInfo
{
	var()	Texture2D	IconTexture;			// Icon 으로 사용할 Texture
	var()	float		u,v,w,h;				// Texture 의 uv 좌표
	var()	float		width, height;			// 화면상에 표시해 줄 Icon 의 width, height
};

// Editable 
var()	repnotify	bool	bEnableBuildBox;	// Enable/Disable?
var()	ETeamType			TeamIdx;			// Build Box 를 사용할 수 있는 Team Index 이다.
var()	float				TotalBuildTime;		// Build Box 를 완성할 때 까지 걸리는 Total 시간.
var()	float				BuildCancelTime;	// Build Stop 후 Cancel 이 되기 까지 걸리는 시간.

var()	float				DecTimeRatePerUser;	// User가 2명 이상일 때 User 가 늘어날수록 감소되는 Build Time Rate
var()	float				MinTotalBuildTime;	// User가 아무리 늘어나도 이 이상은 BuildTime 이 감소될 수 없다.

var()	string				EntryMessage;		// BuildVolume 안에 진입했을 때 보여 줄 Message
var()	IconInfo			BuildIcon;			// 화면상에 표시할 Icon 에 대한 정보
var()	bool				bAlwaysShowIcon;	// true 이면 volume 에 진입하지 않아도 화면상에 Icon 을 표시해 준다.

var()	Vector				Pivot;				// Pivot For Real Location
var() const editconst	StaticMeshComponent	StaticMeshComponent;	// 실체화된 BuildBox

// Non Editable
var		float				Progress;			// Build Complete Percentage. Replication 되어야 한다.
var		array<Pawn>			UserList;			// 현재 Build Volume 을 사용하고 있는 Pawn 
var		array<Pawn>			TouchList;			// 현재 Build Volume 안에 들어와 있는 Pawn

var		PlayerController	LocalPC;			// HUD 에 등록하기
var		bool				bIsInLocalPC;		// Volume 에 Local PC 가 들어왔는지에 대한 Flag

var		bool				bShowIcon;			// 
var		bool				bShowProgress;		//
var		float				IconAlpha;			//
var		float				ProgressAlpha;		//

replication
{
	// replicated properties
	if ( ( bNetInitial || bNetDirty ) && Role == ROLE_Authority)
		Progress, bEnableBuildBox;
}

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();
	
	// 초기 상태 설정
	EnableBuildBox( bEnableBuildBox );

	// Icon 과 Progress 를 표시해 주기 위해서 HUD 의 PostRenderedActor 에 등록을 해야 하지만,
	// 이 시점에서는 LocalPlayerController 를 찾을 수 없기 때문에 등록에 성공할 때 까지 Timer 를 걸어둔다.
	if ( WorldInfo.NetMode != NM_DedicatedServer )
		SetTimer( 1.0, true, 'RegisterHUD' );
}

simulated event ReplicatedEvent(name VarName)
{
	if ( VarName == 'bEnableBuildBox' )
	{
		Client_EnableBuildBox( bEnableBuildBox );
	}
	else Super.ReplicatedEvent( VarName );
}

simulated function RegisterHUD()
{
	ForEach LocalPlayerControllers(LocalPC)
	{
		if ( avaHUD(LocalPC.MyHUD) != None )
		{
			Client_EnableBuildBox( bEnableBuildBox );
			ClearTimer( 'RegisterHUD' );
			return;
		}
	}
}

// Build Progress 와 Build Icon 을 표시해 준다.
// Bug  : 실제 찍히는 Location 과 HUD 에서 Filtering 하는 Location 이 차이가 나서 보이다 안보이다가 한다...
simulated function PostRenderFor(PlayerController PC, Canvas Canvas, vector CameraPosition, vector CameraDir)
{
	local float XL, YL;
	local vector ScreenLoc;
	local string str;

	// 아이콘과 Progress 는 같은 Team 에게만 보인다.
	if ( TeamIdx != TEAM_Unknown && TeamIdx != PC.GetTeamNum() )	return;

	if ( ProgressAlpha == 0.0 && IconAlpha == 0.0 )	return;

	screenLoc = Canvas.Project(GetRealLoc());

	if (screenLoc.X >= 0 &&	screenLoc.X < Canvas.ClipX && screenLoc.Y >= 0 && screenLoc.Y < Canvas.ClipY)
	{
		if ( BuildIcon.IconTexture != None && IconAlpha > 0 )
		{
			Canvas.SetPos(ScreenLoc.X-0.5*BuildIcon.Width,ScreenLoc.Y-0.5*BuildIcon.Height);
			Canvas.SetDrawColor(255,255,255,BYTE(IconAlpha));
			Canvas.DrawTile( BuildIcon.IconTexture, BuildIcon.Width, BuildIcon.Height, BuildIcon.u, BuildIcon.v, BuildIcon.w, BuildIcon.h);
			ScreenLoc.Y += 0.5*BuildIcon.Height;
		}

		if ( ProgressAlpha > 0 )
		{
			str = "("$int(Progress)$"%)";
			Canvas.StrLen(str, XL, YL);
			Canvas.SetDrawColor(255,255,255,BYTE(ProgressAlpha));
			Canvas.SetPos(ScreenLoc.X-0.5*XL,ScreenLoc.Y);
			Canvas.DrawTextClipped(str, true);
		}
	}
}

simulated function Vector GetRealLoc()
{
	return Location + StaticMeshComponent.Translation + Pivot;
}

// ToDO : 실제로 Use 가 가능한지 Check 한다. Team Check, 각도 Check 등등등...
simulated function bool IsUsable( Pawn user )
{
	local Actor HitActor;
	local Vector StartTrace, EndTrace, HitLocation, HitNormal;
	// 같은 Team 만이 Use 할 수 있다.
	if ( TeamIdx != TEAM_Unknown && TeamIdx != user.GetTeamNum() )	return false;

	// Trace 로 찾아본다.
	StartTrace	= user.GetWeaponStartTraceLocation();
	EndTrace	= StartTrace + vector(user.GetBaseAimRotation()) * 10000;
	HitActor = user.Trace(HitLocation, HitNormal, EndTrace, StartTrace, true );
	if ( HitActor != self )	return false;

	return true;
}

simulated event Touch(Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal)
{
	if ( Pawn( Other ) == None )	return;
	TouchList[ TouchList.Length ] = Pawn(Other);
	if ( Pawn( Other ).IsLocallyControlled() )	bIsInLocalPC = true;

	// 일단 Test 로 여기서 Start....
	StartBuild( Pawn(Other) );
}

simulated event UnTouch( Actor Other )
{
	local int nIdx;
	if ( Pawn( Other ) == None )	return;
	nIdx = TouchList.Find( Pawn( Other ) );
	if ( nIdx >= 0 )	TouchList.Remove( nIdx, 1 );
	StopBuild( Pawn(Other) );
	if ( Pawn( Other ).IsLocallyControlled() )	bIsInLocalPC = false;
}

// Build 를 시작한다.
function StartBuild( Pawn user )
{
	local int nIdx;
	// 사용할 수 없다면 무시한다.
	if ( !IsUsable( user ) )	return;
	
	if ( IsTimerActive( 'OnCancel' ) )			ClearTimer( 'OnCancel' );

	nIdx = UserList.Find( user );
	if ( nIdx >= 0 )	return;					// 이미 User List 에 있는 Pawn 이다.

	UserList[ UserList.Length ] = user;
	if ( UserList.Length == 1 )					// 최초 시작한 경우에만 Event 를 발생시키도록...
	{
		if ( Progress == 0.0 )	OnStart();
		else					OnRestart();
	}
}

// Build 를 멈춘다.
function StopBuild( Pawn user )
{
	local int nIdx;
	nIdx = UserList.Find( user );
	if ( nIdx < 0 )	return;						// User List 에 없는 Pawn 이다.
	UserList.Remove( nIdx, 1 );
	if ( UserList.Length == 0 )
	{
		SetTimer( BuildCancelTime, false, 'OnCancel' );
		OnStop();
	}
}

function OnStart()		{	ActivateEvent( 'Start' );		}
function OnRestart()	{	ActivateEvent( 'ReStart' );		}
function OnStop()		{	ActivateEvent( 'Stop' );		}
function OnComplete()	{	ActivateEvent( 'Complete' );	}	// Build 를 완료했음.
// Build 를 취소한다.
function OnCancel()		
{	
	Progress = 0.0;
	ActivateEvent( 'Cancel' );
}

function ActivateEvent( name EventName )
{
	local	avaSeqEvent_BuildBox	eventBuildBox;
	local	int i;
	for ( i=0 ; i<GeneratedEvents.Length; ++i )
	{
		eventBuildBox = avaSeqEvent_BuildBox( GeneratedEvents[i] );
		if ( eventBuildBox != none )	eventBuildBox.ActivateEvent( EventName );
	}
}

function Reset()
{
	Progress = 0.0;
	TouchList.Length = 0;
	UserList.Length	 = 0;
}

// Icon 이나 Progress Percentage 가 보여질지를 결정한다.
simulated function CheckPostRender( float DeltaTime )
{
	if ( bEnableBuildBox )
	{
		if ( bIsInLocalPC && LocalPC != None && LocalPC.Pawn != None && IsUsable( LocalPC.Pawn ) )	
		{
			bShowProgress	= true;
			bShowIcon		= true;
		}
		else		
		{
			bShowProgress	= false;
			bShowIcon		= false;
		}

		if ( bAlwaysShowIcon )
		{
			IconAlpha = 255;
		}
		else
		{
			if ( bShowIcon )	IconAlpha		= Min( 255, IconAlpha + DeltaTime * 255.0 );
			else				IconAlpha		= Max( 0,	IconAlpha - DeltaTime * 255.0 );
		}
		if ( bShowProgress )	ProgressAlpha	= Min( 255, ProgressAlpha + DeltaTime * 255.0 );
		else					ProgressAlpha	= Max( 0,	ProgressAlpha - DeltaTime * 255.0 );
	}
	else	// Disable 이면 보여줄 필요가 없다.
	{
		bShowIcon		= false;
		bShowProgress	= false;
		IconAlpha		= 0;
		ProgressAlpha	= 0;
	}
}

simulated event Tick( float DeltaTime )
{
	local int	i;
	local array<Pawn>	InvalidUsers;
	local float RealBuildTime;

	//아이콘과 Progress Rate 의 Alpha 값을 조절 한다면 여기에서~
	if ( WorldInfo.NetMode != NM_DedicatedServer )
	{	
		CheckPostRender( DeltaTime );
	}

	if ( Role != ROLE_Authority )	return;
	if ( bEnableBuildBox == false )	return;

	// Check Valid User Again.
	for ( i = 0 ; i < UserList.Length ; ++ i )	
		if ( !IsUsable( UserList[i] ) )	InvalidUsers[ InvalidUsers.Length ] = UserList[i];
	for ( i = 0 ; i < InvalidUsers.Length ; ++ i )		
		StopBuild( InvalidUsers[i] );

	if ( UserList.Length == 0 )		return;

	// ET 의 Stamina 개념이 들어간다면 여기서 감소?
	RealBuildTime = TotalBuildTime;
	if ( UserList.Length > 1 && DecTimeRatePerUser > 0.0 )
	{
		RealBuildTime = TotalBuildTime * ( 1.0 - DecTimeRatePerUser * ( UserList.Length - 1 ) / 100 );
		RealBuildTime = Max( RealBuildTime, MinTotalBuildTime );
	}

	Progress += 100 * DeltaTime/RealBuildTime;
	if ( Progress >= 100 )
	{
		Progress = 100;
		OnComplete();
	}
}

simulated function Client_EnableBuildBox( bool bFlag )
{
	//if ( LocalPC != None )
	//{
	//	if ( bFlag == true )	avaHUD(LocalPC.MyHUD).AddPostRenderedActor( self );
	//	else					avaHUD(LocalPC.MyHUD).RemovePostRenderedActor( self );
	//}
}

function EnableBuildBox( bool bFlag )
{
	Progress		= 0;
	bBlockActors	= bFlag ? true : false;
	SetHidden( !bFlag );
	bEnableBuildBox	= bFlag;
}

defaultproperties
{
	TickGroup=TG_PreAsyncWork
	bColored=true
	BrushColor=(R=255,G=0,B=0,A=255)
	SupportedEvents.Add(class'avaSeqEvent_BuildBox')

	bStatic=false
	bAlwaysRelevant=true
	bSkipActorPropertyReplication=false
	bUpdateSimulatedPosition=false
	bReplicateMovement=false
	RemoteRole=ROLE_SimulatedProxy

	Begin Object Name=BrushComponent0
		HiddenGame=true
	End Object

	bBlockActors=true
	Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0
		bUseAsOccluder = FALSE
	    BlockRigidBody=true
		bAcceptsLights=true
	End Object
	CollisionComponent=StaticMeshComponent0
	StaticMeshComponent=StaticMeshComponent0
	Components.Add(StaticMeshComponent0)
	bHidden=false

	TotalBuildTime	= 30.0
	BuildCancelTime	=  5.0
	MinTotalBuildTime = 1.0
}