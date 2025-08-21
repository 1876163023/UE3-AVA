// Binocular 는 분대장 아이템이다....
class avaWeap_Binocular extends avaWeap_BaseGun
	native;

cpptext
{
	virtual VOID	TickSpecial(FLOAT DeltaTime);
	virtual VOID	TestTrace();
}

var float						PrvDistance;
var	MaterialInstanceConstant	DistanceMIC[3];	// 거리를 표시해 주는 MIC
var MaterialInstanceConstant	DirectionMIC;	// 방향을 표시해 주는 MIC
var MaterialInstanceConstant	TargetMIC;

var array<avaPawn>				TargettedList;

var	float						CoolTime;
var	bool						bCanFire;

simulated function AttachItems()
{
	Super.AttachItems();
	AttachScope();
	if ( ScopeComp != None )
	{
		DistanceMIC[0]	= ScopeComp.CreateMaterialInstance( 4 );
		DistanceMIC[1]	= ScopeComp.CreateMaterialInstance( 2 );
		DistanceMIC[2]	= ScopeComp.CreateMaterialInstance( 3 );
		DirectionMIC	= ScopeComp.CreateMaterialInstance( 5 );
		TargetMIC		= ScopeComp.CreateMaterialInstance( 6 );
	}
}

simulated function AttachScope()
{
	local StaticMesh	tempStaticMesh;
	local vector		translation;
	if ( ScopeMeshName != "" )
	{
		tempStaticMesh = StaticMesh( DynamicLoadObject( ScopeMeshName, class'StaticMesh' ) );
		translation		= ScopeComp.Translation;
		Translation.x	= 25.0; 
		translation.y	= -PlayerViewOffset.y;
		translation.z	= -PlayerViewOffset.z;
		ScopeComp.SetTranslation( translation );
		StaticMeshComponent(ScopeComp).SetStaticMesh( tempStaticMesh );
		ScopeComp.SetShadowParent( Mesh );
		ScopeComp.SetOcclusionGroup( Mesh );		
		ScopeComp.SetHidden( true );
	}
}

simulated function DetachWeapon()
{
	avaPawn( Instigator ).bUseBinocular = false;
	Super.DetachWeapon();
}

simulated function EndSightInState()
{
	Super.EndSightInState();
	avaPawn( Instigator ).bUseBinocular = true;
}

simulated function bool SwitchSightMode( int requestMode, float transitionTime, optional bool bPlayAnim = true )
{
	local bool bResult;
	bResult = Super.SwitchSightMode( requestMode, transitionTime, bPlayAnim );
	if ( bResult == true )
	{
		if ( SightMode == 0 )	avaPawn( Instigator ).bUseBinocular = false;
		else					avaPlayerController( Instigator.Controller ).ShowGIM( 109, 1 );
	}
	return bResult;
}

simulated function ReleaseCoolTime()
{
	bCanFire = true;
}

// Host 에 정찰이 가능한지 요청한다. 정찰이 가능하다면 Client 에 AllowRecon 함수를 호출해 준다..
reliable server function RequestRecon()
{
	if ( SightMode == 1 && bCanFire == true )
	{
		AllowRecon();
		bCanFire = false;
		SetTimer( CoolTime, false, 'ReleaseCoolTime' );
	}
}

reliable client function AllowRecon()
{
	local avaPawn				p;
	if ( Instigator.IsLocallyControlled() )
	{
		TargettedList.length = 0;
		foreach DynamicActors( class'avaPawn', p )
		{
			if ( p.GetTeamNum() == Instigator.GetTeamNum() )	continue;
			if ( p.bIsInScreen == true )	TargettedList[TargettedList.length] = p;
		}
		ClearTimer( 'TargetAccepted' );
		SetTimer( 0.5, false, 'TargetAccepted' );
		Instigator.ReceiveLocalizedMessage( class'avaRadioAutoMessage', AUTOMESSAGE_ReconRequest, Instigator.PlayerReplicationInfo, Instigator.PlayerReplicationInfo );
		avaPlayerController( Instigator.Controller ).ClientPlaySound( SoundCue'avaGameSound.Screen_shot_sound' );;

		if ( Role != ROLE_Authority )
		{
			bCanFire = false;
			SetTimer( CoolTime, false, 'ReleaseCoolTime' );
		}
	}
}

simulated function InstantFire()
{
	local vector				StartTrace, EndTrace;
	local vector				HitLocation, HitNormal;

	// Host 에서 Cool Time Check 를 하도록 한다....
	if ( SightMode == 1 && bCanFire == TRUE && Instigator.IsLocallyControlled() )
		RequestRecon();

	if ( Role == ROLE_Authority && SightMode == 0 )
	{
		StartTrace	= Instigator.GetPawnViewLocation();
		EndTrace	= StartTrace + vector(Instigator.GetViewRotation()) * GetTraceRange();
		if ( Instigator.Trace( HitLocation, HitNormal, EndTrace, StartTrace, TRUE, vect(0,0,0),, TRACEFLAG_Bullet ) != None )
			avaPlayerController( Instigator.Controller ).UpdateSignalPos( HitLocation );
	}
}

simulated function TargetAccepted()
{
	local int i;
	if ( TargettedList.length <= 0 )	return;
	for  ( i = 0 ; i < TargettedList.length ; ++ i )
	{
		TargettedList[i].SetTargetted( true );
	}
	TargettedList.length = 0;
	avaPlayerController( Instigator.Controller ).BroadcastLocalizedTeamParam( class'avaRadioAutoMessage', AUTOMESSAGE_ReconAccept ); 
}

simulated function FireAmmunition()
{
	Super( avaWeapon ).FireAmmunition();
	HandleAutofire();
}

function bool DropWeapon(vector StartLocation, optional bool bDoNotSwitch )
{
	// 분대장을 포기한다...
	avaGame( WorldInfo.Game ).ResignSquadLeader( avaPlayerReplicationInfo( Instigator.PlayerReplicationInfo ) );
	PickUpTeamNum	= Instigator.GetTeamNum();
	PickUpLifeTime	= 0;
	IndicatorType	= 1;
	return Super.DropWeapon( StartLocation, bDoNotSwitch );
}

function GivenTo(Pawn NewOwner, bool bDoNotActivate)
{
	Super.GivenTo( NewOwner, bDoNotActivate );
	// 주운사람이 분대장이 된다...
	avaGame( WorldInfo.Game ).SetSquadLeader( avaPlayerReplicationInfo( Instigator.PlayerReplicationInfo ) );
}

function ConsumeAmmo( byte FireModeNum )
{
}

defaultproperties
{
	AmmoCount				=	1
	MaxAmmoCount			=	1	
	BaseSpeed				=	265	// 기본속도
	AimSpeedPct				=	0.8	// 조준시 보정치
	WalkSpeedPct			=	0.42	// 걷기시 보정치
	CrouchSpeedPct			=	0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct		=	0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct			=	0.7	// 수영시 보정치
	SprintSpeedPct			=	1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct	=	1	// 앉아서 스프린트시 보정치

	CoolTime				=	5
	FireInterval(0)			=	0.55

	bAutoFire				=	FALSE			// 누르고 있어도 한발 밖에 발사되지 않는다...
	InventoryGroup			=	5
	bSpecialInventory		=	false
	AttachmentClass			=	class'avaAttachment_Binocular'
	PickupSound				=	SoundCue'avaItemSounds.Item_Get2_Cue'
	BaseSkelMeshName		=	"Wp_Telescope.MS_Telescope"
	BaseAnimSetName			=	"Wp_Telescope.Ani_Telescope"
	SightInfos(0)			=	(FOV=90,ChangeTime=0.3)
	SightInfos(1)			=	(FOV=60,ChangeTime=0.15)
	ScopeMeshName			=	"Wp_Telescope.MS_Telescope_HUD"
	bDisplayAmmoCount		=	false
	bDropWhenDead			=	false
	PickUpClass				=	class'avaPickUp_Binocular'
	MaxPitchLag				=	700
	MaxYawLag				=	1000
	RotLagSpeed				=	0.82
	JumpDamping				=	0.3
	BobDamping				=	0.6
	BobDampingInDash		=	0.4

	WeaponFireAnim(0)		=	Fire
 	WeaponPutDownAnim		=	Down
	WeaponEquipAnim			=	BringUp
	WeaponReloadAnim		=	Reload
	EquipTime				=	1.2
	PutDownTime				=	0.0333
	ReloadTime				=	3.0333
	WeaponIdleAnims(0)		=	Idle

	Kickback_WhenMoving		=	(UpBase=0,LateralBase=0,UpModifier=0,LateralModifier=0,UpMax=0,LateralMax=0,DirectionChange=0)
	Kickback_WhenFalling	=	(UpBase=0,LateralBase=0,UpModifier=0,LateralModifier=0,UpMax=0,LateralMax=0,DirectionChange=0)
	Kickback_WhenDucking	=	(UpBase=0,LateralBase=0,UpModifier=0,LateralModifier=0,UpMax=0,LateralMax=0,DirectionChange=0)
	Kickback_WhenSteady		=	(UpBase=0,LateralBase=0,UpModifier=0,LateralModifier=0,UpMax=0,LateralMax=0,DirectionChange=0)
	
	Spread_WhenFalling		=	(param1=0.01,param2=0)
	Spread_WhenMoving		=	(param1=0.01,param2=0)
	Spread_WhenDucking		=	(param1=0.01,param2=0)
	Spread_WhenSteady		=	(param1=0.01,param2=0)
	
	AccuracyDivisor			=	1000
	AccuracyOffset			=	0
	MaxInaccuracy			=	0
	
	Kickback_UpLimit		=	10
	Kickback_LateralLimit	=	0

	Kickback_WhenMovingA	=	(UpBase=0,LateralBase=0,UpModifier=0,LateralModifier=0,UpMax=0,LateralMax=0,DirectionChange=0)
	Kickback_WhenFallingA	=	(UpBase=0,LateralBase=0,UpModifier=0,LateralModifier=0,UpMax=0,LateralMax=0,DirectionChange=0)
	Kickback_WhenDuckingA	=	(UpBase=0,LateralBase=0,UpModifier=0,LateralModifier=0,UpMax=0,LateralMax=0,DirectionChange=0)
	Kickback_WhenSteadyA	=	(UpBase=0,LateralBase=0,UpModifier=0,LateralModifier=0,UpMax=0,LateralMax=0,DirectionChange=0)
	
	Spread_WhenFallingA		=	(param1=0.01,param2=0)
	Spread_WhenMovingA		=	(param1=0.01,param2=0)
	Spread_WhenDuckingA		=	(param1=0.01,param2=0)
	Spread_WhenSteadyA		=	(param1=0.01,param2=0)
	
	AccuracyDivisorA		=	1000
	AccuracyOffsetA			=	0
	MaxInaccuracyA			=	0
	
	DirectionHold			=	0
	FireIntervalMultiplierA =	1

	Begin Object Class=StaticMeshComponent Name=ScopeComponent0
		bOnlyOwnerSee=true
		DepthPriorityGroup=SDPG_Foreground
		Rotation=(Yaw=16384)
		Translation=(X=25.0,Y=4.0,Z=-1.0)
		bCastDynamicShadow=false
	End Object
	ScopeComp=ScopeComponent0
	Components.Add(ScopeComponent0)

	GIMIndexWhenPickUp		=	105
	GIMIndexWhenEquip		=	110
	bCanFire				=	true
	bAvailableAbandonWeapon =	true
	DropVelocity			=	25

	bDrawInRadar			=	true
	RadarIconCode			=	58

	bSyncSightMode			=	true	
}