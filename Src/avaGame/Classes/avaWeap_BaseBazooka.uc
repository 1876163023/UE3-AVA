/*=============================================================================
  avaWeap_BaseBazooka
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2006/05/29 by OZ
		Bazooka 를 구현하기 위한 Base Class
		avaWeap_BaseGun 으로부터 상속받는다.

	ToDo.	1. 장전이 되어 있지 않은 경우 탄두가 보이 지 않으며, 이 내용은 Replication 되어야 한다.
				1-1. 1인칭 에서는 발사 후 탄두가 사라진다.
				1-2. 1인칭 에서는 Reload 가 시작될 때 탄두가 보인다.
				1-3. 3인칭 에서는 Reload 가 완료된 후 (ReloadCnt 가 있는 경우) 탄두가 보이기 시작하면 된다.
				-> 제일 좋은 방법은 Weapon 을 모두가 들고 있는 것이다....
				[완료]
			2. Attachment Class 를 만든다.
				[완료]
			3. DamageType 은 어떻게 할 것인가?
=============================================================================*/
class avaWeap_BaseBazooka extends avaWeap_BaseGun;

`include(avaGame/avaGame.uci)

var bool			bShowShell;				// 탄두를 보여줄지에 대한 Flag
var	MeshComponent	ShellMesh;				// RPG 탄두 MeshComponent
var string			ShellMeshName;
var name			ShellBoneName;

simulated state WeaponPuttingDown
{
	simulated function BeginState( name prvstate )
	{
		Super.BeginState( prvstate );
		ShowShell( ( ReloadCnt > 0 ) );
	}
}

simulated function ConfirmFire( byte FireModeNum )
{
	// set current fire mode
	SetCurrentFireMode(FireModeNum);
	// transition to firing mode state
	GotoState(FiringStatesArray[FireModeNum]);	
}

reliable client simulated function ClientConfirmFire( byte FireModeNum )
{
	if ( Role == ROLE_Authority )	return;
	ConfirmFire( FireModeNum );
}

simulated function SendToFiringState(byte FireModeNum)
{
	if ( Role < ROLE_Authority )
		return;
	if ( FireModeNum == 0 && SightMode > 0 && HasAmmo(FireModeNum) )
	{
		ClientConfirmFire( FireModeNum );
		ConfirmFire( FireModeNum );
	}
}

simulated state WeaponReloading
{
	simulated function BeginState(name prvstate)
	{
		Super.BeginState( prvstate );
		ShowShell( true );
	}
}

// 바주카의 탄환을 보여줄것인가?
simulated function ShowShell( bool bShow )
{
	bShowShell = bShow;
	ChangeShellVisibility( bShowShell );
}

simulated function ChangeShellVisibility( bool bVisible )
{
	if ( !bVisible )				ShellMesh.SetHidden( true );
	else if ( !Mesh.HiddenGame )	ShellMesh.SetHidden( false );
}

simulated function ChangeVisibility(bool bIsVisible)
{
	Super.ChangeVisibility(bIsVisible);
	ChangeShellVisibility(bShowShell);
}

simulated state WeaponEquipping
{
	simulated function BeginState( name prvstate )
	{
		Super.BeginState( prvstate );
		ShowShell( ( ReloadCnt > 0 ) );	// 장전이 되어 있으면 탄두를 보여준다.
	}
}

simulated function AttachItems()
{
	local StaticMesh	tempStaticMesh;
	local vector		translation;
	Super.AttachItems();
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

	if ( ShellMeshName != "" )
	{
		tempStaticMesh = StaticMesh( DynamicLoadObject( ShellMeshName, class'StaticMesh' ) );
		StaticMeshComponent( ShellMesh ).SetStaticMesh( tempStaticMesh );
		ShellMesh.SetShadowParent( Mesh );
		ShellMesh.SetOcclusionGroup( Mesh );		
		SkeletalMeshComponent(Mesh).AttachComponent(ShellMesh, ShellBoneName );
	}
}

simulated function SetLightEnvironment( LightEnvironmentComponent env )
{
	Super.SetLightEnvironment( env );
	if (ShellMesh != none)
		ShellMesh.SetLightEnvironment( env );
}

simulated event function RifleFire( int ShotNum )
{
	ShowShell( false );
	ProjectileFire();
	PlayFireEffects(0);
}

defaultproperties
{
	BaseSkelMeshName		=	"Wp_Heavy_RPG7.MS_Heavy_RPG7"
	BaseAnimSetName			=	"Wp_Heavy_RPG7.Ani_RPG7"
	InventoryGroup			=	1

	// Projectile 관련 Properties
	WeaponFireTypes(0)		=	EWFT_InstantHit
	WeaponProjectiles(0)	=	class'avaProj_Bazooka'
	FireOffset				=	(X=40,Y=0)					// Projectile 이 나가는 위치

	WeaponFireAnim(0)		=	Fire_A
 	WeaponPutDownAnim		=	Down
	WeaponEquipAnim			=	BringUp
	WeaponReloadAnim		=	Reload
	WeaponIdleAnims(0)		=	Idle
	SightInAnim				=	Zoom_In 
	SightOutAnim			=	Zoom_Out

	FireInterval(0)			=	2.0
	EquipTime				=	1.4333
	PutDownTime				=	0.0333
	ReloadTime				=	4.0333

	// Ammo 관련
	ClipCnt					=	1
	AmmoCount				=	2
	MaxAmmoCount			=	5

	// SightMode 관련 Parameter
	SightInfos(0)			=	(FOV=90,ChangeTime=0.1)
	SightInfos(1)			=	(FOV=85,ChangeTime=0.1)

	// Velocity Limit Parameter of Pawn
	BaseSpeed				=	182		// 기본속도
	AimSpeedPct				=	0.6		// 조준시 보정치
	WalkSpeedPct			=	0.4		// 걷기시 보정치
	CrouchSpeedPct			=	0.3		// 앉아이동시 보정치
	CrouchAimSpeedPct		=	0.2		// 앉아서 조준 이동시 보정치
	SwimSpeedPct			=	0.7		// 수영시 보정치
	SprintSpeedPct			=	1.25	// 스프린트시 보정치
	CrouchSprintSpeedPct	=	1		// 앉아서 스프린트시 보정치

	bReleaseZoomAfterFire			=	true
	fReleaseZoomAfterFireInterval	=	0.3

	Kickback_WhenMoving		=  (UpBase=30.0,LateralBase=5.0,UpModifier=0.25,LateralModifier=0.015,UpMax=40.0,LateralMax=20.0,DirectionChange=7)
	Kickback_WhenFalling	=  (UpBase=30.0,LateralBase=5.0,UpModifier=0.25,LateralModifier=0.015,UpMax=40.0,LateralMax=20.0,DirectionChange=7)
	Kickback_WhenDucking	=  (UpBase=30.0,LateralBase=5.0,UpModifier=0.25,LateralModifier=0.015,UpMax=40.0,LateralMax=20.0,DirectionChange=7)
	Kickback_WhenSteady		= (UpBase=30.0,LateralBase=5.0,UpModifier=0.25,LateralModifier=0.015,UpMax=40.0,LateralMax=20.0,DirectionChange=7)

	Kickback_WhenMovingA	=  (UpBase=30.0,LateralBase=5.0,UpModifier=0.25,LateralModifier=0.015,UpMax=40.0,LateralMax=20.0,DirectionChange=7)
	Kickback_WhenFallingA	=  (UpBase=30.0,LateralBase=5.0,UpModifier=0.25,LateralModifier=0.015,UpMax=40.0,LateralMax=20.0,DirectionChange=7)
	Kickback_WhenDuckingA	=  (UpBase=30.0,LateralBase=5.0,UpModifier=0.25,LateralModifier=0.015,UpMax=40.0,LateralMax=20.0,DirectionChange=7)
	Kickback_WhenSteadyA	= (UpBase=30.0,LateralBase=5.0,UpModifier=0.25,LateralModifier=0.015,UpMax=40.0,LateralMax=20.0,DirectionChange=7)

	ScopeMeshName			= "avaScopeUI.Distortion.MS_TPGSniper_Scope_Mesh"

	Begin Object Class=StaticMeshComponent Name=ScopeComponent0
		bOnlyOwnerSee=true
		DepthPriorityGroup=SDPG_Foreground
		Rotation=(Yaw=16384)
		Translation=(X=25.0,Y=4.0,Z=-1.0)
		bCastDynamicShadow=false
	End Object
	ScopeComp=ScopeComponent0
	Components.Add(ScopeComponent0)


	// RPG 탄관련 Mesh Component
	Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0
		bUseAsOccluder = FALSE
		DepthPriorityGroup=SDPG_Foreground
		bOnlyOwnerSee=true
		CollideActors=false
		Rotation=(Yaw=-16384,Pitch=0,Roll=0)
	End Object
	ShellMesh=StaticMeshComponent0

	ShellMeshName		=	"Wp_Heavy_RPG7.MS_Heavy_RPG7_missile"
	ShellBoneName		=	Bone01
	bCanThrow			=	true
	PickupClass			=	class'avaSwappedPickUp'
	WeaponType			=	WEAPON_RPG

	bSyncSightMode		=	true
}
