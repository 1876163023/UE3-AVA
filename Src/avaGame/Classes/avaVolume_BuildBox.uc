//=============================================================================
//  avaVolume_BuildBox
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
//  
//	2006/05/11 by OZ
//		BuildBox �� ���� Volume �̴�.
//		avaSeqEvent_BuildBox �� �����Ѵ�.
//
//	ToDo.
//		Volume �� ���������� EntryMessage�� ����ֵ��� �Ѵ�. (UI�κ��� Ȯ���Ǹ�....)
//		ET �� Stamina ������ ���ٸ� ó���� �ֵ��� �Ѵ�.
//
//=============================================================================
class avaVolume_BuildBox extends TriggerVolume;

struct IconInfo
{
	var()	Texture2D	IconTexture;			// Icon ���� ����� Texture
	var()	float		u,v,w,h;				// Texture �� uv ��ǥ
	var()	float		width, height;			// ȭ��� ǥ���� �� Icon �� width, height
};

// Editable 
var()	repnotify	bool	bEnableBuildBox;	// Enable/Disable?
var()	ETeamType			TeamIdx;			// Build Box �� ����� �� �ִ� Team Index �̴�.
var()	float				TotalBuildTime;		// Build Box �� �ϼ��� �� ���� �ɸ��� Total �ð�.
var()	float				BuildCancelTime;	// Build Stop �� Cancel �� �Ǳ� ���� �ɸ��� �ð�.

var()	float				DecTimeRatePerUser;	// User�� 2�� �̻��� �� User �� �þ���� ���ҵǴ� Build Time Rate
var()	float				MinTotalBuildTime;	// User�� �ƹ��� �þ�� �� �̻��� BuildTime �� ���ҵ� �� ����.

var()	string				EntryMessage;		// BuildVolume �ȿ� �������� �� ���� �� Message
var()	IconInfo			BuildIcon;			// ȭ��� ǥ���� Icon �� ���� ����
var()	bool				bAlwaysShowIcon;	// true �̸� volume �� �������� �ʾƵ� ȭ��� Icon �� ǥ���� �ش�.

var()	Vector				Pivot;				// Pivot For Real Location
var() const editconst	StaticMeshComponent	StaticMeshComponent;	// ��üȭ�� BuildBox

// Non Editable
var		float				Progress;			// Build Complete Percentage. Replication �Ǿ�� �Ѵ�.
var		array<Pawn>			UserList;			// ���� Build Volume �� ����ϰ� �ִ� Pawn 
var		array<Pawn>			TouchList;			// ���� Build Volume �ȿ� ���� �ִ� Pawn

var		PlayerController	LocalPC;			// HUD �� ����ϱ�
var		bool				bIsInLocalPC;		// Volume �� Local PC �� ���Դ����� ���� Flag

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
	
	// �ʱ� ���� ����
	EnableBuildBox( bEnableBuildBox );

	// Icon �� Progress �� ǥ���� �ֱ� ���ؼ� HUD �� PostRenderedActor �� ����� �ؾ� ������,
	// �� ���������� LocalPlayerController �� ã�� �� ���� ������ ��Ͽ� ������ �� ���� Timer �� �ɾ�д�.
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

// Build Progress �� Build Icon �� ǥ���� �ش�.
// Bug  : ���� ������ Location �� HUD ���� Filtering �ϴ� Location �� ���̰� ���� ���̴� �Ⱥ��̴ٰ� �Ѵ�...
simulated function PostRenderFor(PlayerController PC, Canvas Canvas, vector CameraPosition, vector CameraDir)
{
	local float XL, YL;
	local vector ScreenLoc;
	local string str;

	// �����ܰ� Progress �� ���� Team ���Ը� ���δ�.
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

// ToDO : ������ Use �� �������� Check �Ѵ�. Team Check, ���� Check ����...
simulated function bool IsUsable( Pawn user )
{
	local Actor HitActor;
	local Vector StartTrace, EndTrace, HitLocation, HitNormal;
	// ���� Team ���� Use �� �� �ִ�.
	if ( TeamIdx != TEAM_Unknown && TeamIdx != user.GetTeamNum() )	return false;

	// Trace �� ã�ƺ���.
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

	// �ϴ� Test �� ���⼭ Start....
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

// Build �� �����Ѵ�.
function StartBuild( Pawn user )
{
	local int nIdx;
	// ����� �� ���ٸ� �����Ѵ�.
	if ( !IsUsable( user ) )	return;
	
	if ( IsTimerActive( 'OnCancel' ) )			ClearTimer( 'OnCancel' );

	nIdx = UserList.Find( user );
	if ( nIdx >= 0 )	return;					// �̹� User List �� �ִ� Pawn �̴�.

	UserList[ UserList.Length ] = user;
	if ( UserList.Length == 1 )					// ���� ������ ��쿡�� Event �� �߻���Ű����...
	{
		if ( Progress == 0.0 )	OnStart();
		else					OnRestart();
	}
}

// Build �� �����.
function StopBuild( Pawn user )
{
	local int nIdx;
	nIdx = UserList.Find( user );
	if ( nIdx < 0 )	return;						// User List �� ���� Pawn �̴�.
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
function OnComplete()	{	ActivateEvent( 'Complete' );	}	// Build �� �Ϸ�����.
// Build �� ����Ѵ�.
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

// Icon �̳� Progress Percentage �� ���������� �����Ѵ�.
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
	else	// Disable �̸� ������ �ʿ䰡 ����.
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

	//�����ܰ� Progress Rate �� Alpha ���� ���� �Ѵٸ� ���⿡��~
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

	// ET �� Stamina ������ ���ٸ� ���⼭ ����?
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