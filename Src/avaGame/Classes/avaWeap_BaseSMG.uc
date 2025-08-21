//=============================================================================
//  avaWeap_BaseSMG
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
// 
//	2006/04/05 by OZ
//
//		Alt-Fire �� ������ Ż���� �ڵ��߰�
//		BurstMode �ڵ� ����
//=============================================================================
class avaWeap_BaseSMG extends avaWeap_BaseGun;

simulated function AttachItems()
{
	local SkeletalMesh			tempMesh;
	local vector				translation;
	Super.AttachItems();
	if ( ScopeMeshName != "" )
	{
		tempMesh = SkeletalMesh( DynamicLoadObject( ScopeMeshName, class'SkeletalMesh' ) );

		translation		= ScopeComp.Translation;
		Translation.x	= -8.0; 
		translation.y	= -PlayerViewOffset.y;
		translation.z	= -PlayerViewOffset.z;
		ScopeComp.SetTranslation( translation );
		SkeletalMeshComponent(ScopeComp).SetSkeletalMesh( tempMesh );
		ScopeComp.SetShadowParent( Mesh );
		ScopeComp.SetOcclusionGroup( Mesh );		
		ScopeComp.SetHidden( true );
		SkeletalMeshComponent(ScopeComp).PlayAnim( 'Idle',, false );
		ScopeMIC = ScopeComp.CreateMaterialInstance( tempMesh.Materials.Length-1 );
	}
}

simulated function PlayFireEffects( byte FireModeNum, optional vector HitLocation )
{
	Super.PlayFireEffects( FireModeNum, HitLocation );
	if ( ScopeComp != None )
		SkeletalMeshComponent( ScopeComp ).PlayAnim( 'Fire',, false );
}

defaultproperties
{
	InventoryGroup=1

	bAutoFire=True

	InstantHitDamageTypes(0)=class'avaDmgType_Gun'

	// Weapon SkeletalMesh
	WeaponFireAnim(0)	=Fire
 	WeaponPutDownAnim	=Down
	WeaponEquipAnim		=BringUp
	WeaponReloadAnim	=Reload
	EquipTime			=1.0333
	PutDownTime			=0.0333
	ReloadTime			=2.6333
	WeaponIdleAnims(0)	=Idle

	MuzzleFlashSocket=MuzzleFlashSocket
	MuzzleFlashPSCTemplate=ParticleSystem'avaEffect.Particles.P_WP_SMG_MuzzleFlash_1P'
	MuzzleFlashDuration=0.05	

	Begin Object Name=BulletTrailComponent0
		HalfLife	=	0.03
		Intensity	=	10.35
		Size		=	0.55
		Speed		=	3000
	End Object
	TrailInterval	=	2


	// Weapon �� ���� Pawn �� �ӵ� ����
	BaseSpeed			= 285	// �⺻�ӵ�
	AimSpeedPct			= 0.8	// ���ؽ� ����ġ
	WalkSpeedPct		= 0.4	// �ȱ�� ����ġ
	CrouchSpeedPct		= 0.25	// �ɾ��̵��� ����ġ
	CrouchAimSpeedPct	= 0.2	// �ɾƼ� ���� �̵��� ����ġ
	SwimSpeedPct		= 0.7	// ������ ����ġ
	SprintSpeedPct		= 1.25	// ������Ʈ�� ����ġ
	CrouchSprintSpeedPct= 1	// �ɾƼ� ������Ʈ�� ����ġ

	// ������ ���� Propertie ��....
	Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0
		bUseAsOccluder = FALSE
		DepthPriorityGroup=SDPG_Foreground
		bOnlyOwnerSee=true
		CollideActors=false
		Rotation=(Yaw=-16384,Pitch=0,Roll=0)
	End Object
	SilencerMesh=StaticMeshComponent0

	bEnableSilencer			= true
	MountSilencerAnim		= Sil_In	// ������ ���� Animation �̸�
	UnMountSilencerAnim		= Sil_Out	// ������ Ż�� Animation �̸�
	MountSilencerTime		= 2.5333	// ������ ������ �ɸ��� �ð�
	UnMountSilencerTime		= 2.5333	// ������ Ż���� �ɸ��� �ð�

	WeaponSilencerFireSnd	= SoundCue'avaWeaponSounds.Common.Silencer.Silencer_Fire_SMG'	

	HitDamageS=26

	PenetrationS = 1
	RangeModifierS=0.85

	AccuracyDivisorA=200
	AccuracyOffsetA=0.35
	MaxInaccuracyA=1.25

	Kickback_WhenMovingA	= (UpBase=0.9,LateralBase=0.45,UpModifier=0.35,LateralModifier=0.04,UpMax=5.25,LateralMax=3,DirectionChange=4)
	Kickback_WhenFallingA	= (UpBase=0.45,LateralBase=0.3,UpModifier=0.2,LateralModifier=0.0275,UpMax=4,LateralMax=2.25,DirectionChange=7)
	Kickback_WhenDuckingA	= (UpBase=0.275,LateralBase=0.2,UpModifier=0.125,LateralModifier=0.02,UpMax=3,LateralMax=1,DirectionChange=9)
	Kickback_WhenSteadyA	= (UpBase=0.3,LateralBase=0.225,UpModifier=0.125,LateralModifier=0.02,UpMax=3.25,LateralMax=1.25,DirectionChange=8)

	Spread_WhenFallingA			= (param1=0,param2=0.3)
	Spread_WhenMovingA		= (param1=0,param2=0.115)
	Spread_WhenDuckingA		= (param1=0,param2=0.045)
	Spread_WhenSteadyA		= (param1=0,param2=0.045)	

	BulletTemplete		= ParticleSystem'avaEffect.Gun_Effect.Ps_Wp_SMG_cartridge'

	Begin Object class=AnimNodeSequence Name=SequenceA
	End Object

	Begin Object Class=SkeletalMeshComponent Name=ScopeComponent0
		bOnlyOwnerSee=true
		DepthPriorityGroup=SDPG_Foreground
		Rotation=(Yaw=-16384)
		Translation=(X=-8.0,Y=4.0,Z=-1.0)
		Animations=SequenceA
		PhysicsAsset=None
		AnimSets(0)=AnimSet'Wp_Scope.Com_Scope.dot_ani'
		bCastDynamicShadow=false
	End Object
	ScopeComp=ScopeComponent0
	Components.Add(ScopeComponent0)

	WeaponType		=	WEAPON_SMG

	bAvailableInstantZoomFire = true
}
