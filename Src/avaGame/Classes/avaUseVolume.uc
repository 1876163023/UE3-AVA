
class avaUseVolume extends TriggerVolume native;

`include(avaGame/avaGame.uci)

var()	int				IconCode;
var()	ETeamType		Team;
var()	Actor			UseActor;								// 사용할 Actor : 특별히 지정을 하지 않아도 된다. 단지 시야를 Check 하기 위한 용도이다.
//var()	bool			bUseActorWhenBaseChanged;

var()	int				GameInfoMsgIndex;
var		int				GameInfoMsgType;

var()	string			UseMessage;


var()	float			MaxUseTime;								// E Key 를 누르고 있어야만 하는 시간. 0 이라면 바로 Complete!

var()	bool			bLeaderOnly;							// Check 되어 있으면 분대장만 사용가능함
var()	int				bRestrictionClass[`MAX_PLAYER_CLASS];	// true 이면 해당 병과 사용 불가능

var()	int				ProgressRate;							// Use Progress
var()	int				MaxUserCnt;								// 이 Use Volume 을 이용할 수 있는 User 의 Count....
var()	float			UseEfficiency;							// 2명 이상이서 사용할 경우, User 가 추가될때마다 증가하는 효율....

var		bool			bUsed;									// 현재 사용중임
var		BYTE			UserCnt;								

var		array<Pawn>		UserList;
var		float			UseTime;				

var()	SoundCue		UseSoundCue;

var()	EUseActionType	ActionType;

replication
{
	// replicated properties
	if ( ( bNetInitial || bNetDirty ) && Role == ROLE_Authority)
		bUsed, UserCnt, UseActor;
}

function OnStart()						{	ActivateEvent( 'Start' );				}
function OnCancel()						{	ActivateEvent( 'Cancel' );				}
function OnComplete()					{	ActivateEvent( 'Complete' );			}
function OnProgress(int Progress )		{	ActivateEvent( 'Progress', Progress );	}	

function bool FindUser( Pawn P )
{
	local int i;
	for ( i = 0 ; i < UserList.length ; ++ i )
	{
		if ( UserList[i] == P )	
			return true;
	}
	return false;
}

function StartUse( Pawn p )
{
	if ( p.Controller == None )	return;
	if ( FindUser( P ) )		return;

	if ( UserList.length == 0 )
	{
		UseTime	=	0;
	}
	UpdateProgressBar( UseTime, MaxUseTime );
	AddUser( p );
	if ( bUsed == FALSE )
		OnStart();
	bUsed	=	TRUE;

	avaPlayerController( p.Controller ).NotifyStartUse( self );
}

function CancelUse( Pawn p )
{
	local int i;
	if ( !FindUser( p ) )	return;

	for ( i = 0 ; i < UserList.length ; ++ i )
	{
		if ( UserList[i] == p )
		{
			RemoveUser( i );
			break;
		}
	}
	
	if ( UserList.length <=  0 )
	{
		OnEmptyUser();
	}

	avaPlayerController( p.Controller ).NotifyStopUse( self );
}

function OnEmptyUser()
{
	UpdateProgressBar( 0, 0 );
	OnCancel();
	bUsed			= FALSE;
	UseTime			= 0;
	ClearUserList();
}

function UseComplete()
{
	local int i;
	for ( i = 0 ; i < UserList.length ; ++ i )
	{
		avaPlayerController( UserList[i].Controller ).NotifyStopUse( self );
	}


	UpdateProgressBar( 0, 0 );
	OnComplete();
	bUsed			= FALSE;
	UseTime			= 0;
	ClearUserList();
}

function ActivateEvent( name EventName, optional int Progress )
{
	local	avaSeqEvent_Use	useEvent;
	local	array<Controller>	InUserList;
	local	int i;

	InUserList.length = UserList.length;
	for ( i = 0 ; i < UserList.length ; ++ i )
	{
		InUserList[i] = UserList[i].Controller;
	}

	for ( i=0 ; i<GeneratedEvents.Length; ++i )
	{
		useEvent = avaSeqEvent_Use( GeneratedEvents[i] );
		if ( useEvent != none )	
		{
			useEvent.Trigger( EventName, InUserList, Progress );
		}
	} 
}

// 
function bool CanUseContinue( Pawn p )
{
	local vector HitLocation, HitNormal;
	local vector TraceEnd, TraceStart;
	local Actor	 HitActor;
	if ( !FindUser( p ) )	return FALSE;
	if ( UseActor != None )
	{
		TraceStart	=	p.GetPawnViewLocation();
		TraceEnd	=	TraceStart + vector( p.GetViewRotation() ) * 1000;
		HitActor	=	p.Trace( HitLocation, HitNormal, TraceEnd, TraceStart, true, vect(0,0,0) );
		if ( HitActor != UseActor )
			return FALSE;
	}
	return TRUE;
}

simulated event bool ClientIsUseable( Pawn p )
{
	local vector HitLocation, HitNormal;
	local vector TraceEnd, TraceStart;
	local Actor	 HitActor;

	if ( p == None )
		return FALSE;

	// 더 이상 사용할 수 없음....
	if ( UserCnt >= MaxUserCnt )
		return FALSE;

	if ( Team != TEAM_Unknown && Team != p.GetTeamNum() )
		return FALSE;

	// Leader 만 사용가능
	if ( bLeaderOnly == TRUE && !avaPlayerReplicationInfo( p.PlayerReplicationInfo ).IsSquadLeader() )
		return FALSE;

	// 병과제한에 의해서 사용불가능
	if ( bRestrictionClass[avaCharacter(p).TypeID] != 0 )
		return FALSE;

	// Trace 를 해서 UseActor 가 보이는지 Check 한다...
	if ( UseActor != None )
	{
		TraceStart	=	p.GetPawnViewLocation();
		TraceEnd	=	TraceStart + vector( p.GetViewRotation() ) * 1000;
		HitActor	=	p.Trace( HitLocation, HitNormal, TraceEnd, TraceStart, true, vect(0,0,0) );
		if ( HitActor != UseActor )
			return FALSE;
	}
	return TRUE;
}

simulated event bool IsUseable( Pawn p )
{
	return ClientIsUseable( p );
}

function bool UsedBy( Pawn p )
{
	if ( !ClientIsUseable( p ) )	
		return FALSE;

	if ( MaxUseTime <= 0 )					// MaxUseTime 을 지정하지 않았다면 바로 Complete 가 된다.
	{
		UseComplete();
	}
	else
	{
		StartUse( p );
	}
	return TRUE;
}

function UnUse( Pawn p )
{
	if ( p != None )
	{
		CancelUse( p );
	}
}

simulated event Touch( Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal )
{
	if ( WorldInfo.GRI == None ) return;
	if ( avaGameReplicationInfo( WorldInfo.GRI ).bWarmupRound == true )	return;

	Super.Touch( Other, OtherComp, HitLocation, HitNormal );
	if ( Pawn( Other ) == None  )	return;
	avaPawn( Other ).TouchUseVolume( self );
}

simulated event UnTouch( Actor Other )
{
	if ( avaGameReplicationInfo( WorldInfo.GRI ).bWarmupRound == true )	return;

	if ( Pawn( Other ) == None  )	return;
	avaPawn( Other ).UnTouchUseVolume( self );
	UnUse( Pawn( Other ) );
}

function UpdateUserProgressBar( Pawn p, float Cur, float Max )
{
	avaPlayerReplicationInfo( p.Controller.PlayerReplicationInfo ).SetGauge( Cur * 10, Max * 10 );
}

function UpdateProgressBar(float Cur, float Max)
{
	local int i;
	for ( i = 0 ; i < UserList.length ; ++ i )
	{
		UpdateUserProgressBar( UserList[i], Cur * 10, Max * 10 );
	}
}

function AddUser( Pawn P )
{
	if ( ActionType != UAT_None )
		avaPlayerReplicationInfo( P.PlayerReplicationInfo ).SetUseActionType( ActionType );
	UserList[UserList.Length] = P;
	UserCnt = UserList.length;
	NetUpdateTime = WorldInfo.TimeSeconds - 1.0f;
}

function RemoveUser( int i )
{
	UpdateUserProgressBar( UserList[i], 0, 0 );
	UserList.Remove(i, 1);
	UserCnt = UserList.length;
	NetUpdateTime = WorldInfo.TimeSeconds - 1.0f;
}

function ClearUserList()
{
	UserList.length = 0;
	UserCnt = UserList.length;
	NetUpdateTime = WorldInfo.TimeSeconds - 1.0f;
}

event Tick( float DeltaTime )
{
	local float	PrevUseTime;
	local float	Progress;
	local int	PrvRate;
	local int	CurRate;
	local int	i;

	if ( bUsed == FALSE )	return;

	for ( i = 0 ; i < UserList.length ; ++i )
	{
		if ( !CanUseContinue( UserList[i] ) )
		{
			RemoveUser( i );
			--i;
		}
	}

	if ( UserList.length == 0 )
	{
		OnEmptyUser();
	}

	PrevUseTime =	UseTime;
	UseTime		+=	DeltaTime * ( 1 + ( ( UserList.length - 1 ) * UseEfficiency ) );

	if ( UseTime >= MaxUseTime )
	{
		UseTime = MaxUseTime;
		UseComplete();
	}
	else
	{
		if ( ProgressRate > 0 )
		{
			Progress	= MaxUseTime / (ProgressRate + 1 );
			PrvRate		= int( PrevUseTime / Progress );
			CurRate		= int( UseTime / Progress );
		
			if ( PrvRate < CurRate )
			{
				OnProgress( CurRate );
			}
		}

		UpdateProgressBar( UseTime, MaxUseTime );
	}
}

/**	Handling Toggle event from Kismet. */
simulated function OnToggle(SeqAct_Toggle Action)
{
	// Turn ON
	if (Action.InputLinks[0].bHasImpulse)
	{
		if(!bCollideActors)
		{
			SetCollision(true, bBlockActors);
		}

		if ( bIgnoreBlockRigidBody )
			CollisionComponent.SetBlockRigidBody( FALSE );
		else
			CollisionComponent.SetBlockRigidBody( TRUE );
	}
	// Turn OFF
	else if (Action.InputLinks[1].bHasImpulse)
	{
		if(bCollideActors)
		{
			SetCollision(false, bBlockActors);
		}

		CollisionComponent.SetBlockRigidBody( FALSE );
	}
	// Toggle
	else if (Action.InputLinks[2].bHasImpulse)
	{
		SetCollision(!bCollideActors, bBlockActors);

		if ( bIgnoreBlockRigidBody )
			CollisionComponent.SetBlockRigidBody( FALSE );
		else
			CollisionComponent.SetBlockRigidBody( !CollisionComponent.BlockRigidBody );
	}

	ForceNetRelevant();

	SetForcedInitialReplicatedProperty(Property'Engine.Actor.bCollideActors', (bCollideActors == default.bCollideActors));
}

/*! @brief Base가 바뀔 때 UseActor도 같이 바뀌게 해준다.(2007/07/27 고광록)
	@note
		Kismet에서 AttachToActor를 이용해서 붙일 경우에도 UseActor를 설정하고 싶어서 추가함.
*/
//event BaseChange()
//{
//	if ( bUseActorWhenBaseChanged )
//		UseActor = Base;
//}

defaultproperties
{	
	bColored			=	TRUE
	BrushColor			=	(R=100,G=100,B=255,A=255)
	bCollideActors		=	TRUE
	bProjTarget			=	FALSE
	bStatic				=	FALSE
	SupportedEvents(3)	=	class'avaSeqEvent_Use'
	UseMessage			=	"Press USE key to interact this"
	Team				=	TEAM_Unknown

	bUsed				=	FALSE
	MaxUserCnt			=	1
	UseEfficiency		=	0.2

	GameInfoMsgIndex	=	-1
	GameInfoMsgType		=	2
}
