/*=============================================================================
  avaWeap_BaseBazooka
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2006/05/29 by OZ
		Bazooka �� �����ϱ� ���� Base Class
		avaWeap_BaseGun ���κ��� ��ӹ޴´�.

	ToDo.	1. ������ �Ǿ� ���� ���� ��� ź�ΰ� ���� �� ������, �� ������ Replication �Ǿ�� �Ѵ�.
				1-1. 1��Ī ������ �߻� �� ź�ΰ� �������.
				1-2. 1��Ī ������ Reload �� ���۵� �� ź�ΰ� ���δ�.
				1-3. 3��Ī ������ Reload �� �Ϸ�� �� (ReloadCnt �� �ִ� ���) ź�ΰ� ���̱� �����ϸ� �ȴ�.
				-> ���� ���� ����� Weapon �� ��ΰ� ��� �ִ� ���̴�....
				[�Ϸ�]
			2. Attachment Class �� �����.
				[�Ϸ�]
			3. DamageType �� ��� �� ���ΰ�?
=============================================================================*/
class avaWeap_BaseBazooka extends avaWeap_BaseGun;

`include(avaGame/avaGame.uci)

var bool			bShowShell;				// ź�θ� ���������� ���� Flag
var	MeshComponent	ShellMesh;				// RPG ź�� MeshComponent
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

// ����ī�� źȯ�� �����ٰ��ΰ�?
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
		ShowShell( ( ReloadCnt > 0 ) );	// ������ �Ǿ� ������ ź�θ� �����ش�.
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

	// Projectile ���� Properties
	WeaponFireTypes(0)		=	EWFT_InstantHit
	WeaponProjectiles(0)	=	class'avaProj_Bazooka'
	FireOffset				=	(X=40,Y=0)					// Projectile �� ������ ��ġ

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

	// Ammo ����
	ClipCnt					=	1
	AmmoCount				=	2
	MaxAmmoCount			=	5

	// SightMode ���� Parameter
	SightInfos(0)			=	(FOV=90,ChangeTime=0.1)
	SightInfos(1)			=	(FOV=85,ChangeTime=0.1)

	// Velocity Limit Parameter of Pawn
	BaseSpeed				=	182		// �⺻�ӵ�
	AimSpeedPct				=	0.6		// ���ؽ� ����ġ
	WalkSpeedPct			=	0.4		// �ȱ�� ����ġ
	CrouchSpeedPct			=	0.3		// �ɾ��̵��� ����ġ
	CrouchAimSpeedPct		=	0.2		// �ɾƼ� ���� �̵��� ����ġ
	SwimSpeedPct			=	0.7		// ������ ����ġ
	SprintSpeedPct			=	1.25	// ������Ʈ�� ����ġ
	CrouchSprintSpeedPct	=	1		// �ɾƼ� ������Ʈ�� ����ġ

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


	// RPG ź���� Mesh Component
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
