/**
 * Copyright 2006 AVA.

 */

class avaWeap_BasePistol extends avaWeap_BaseGun;

var(Pistol) float MinAccuracy, MaxAccuracy, AccuracyRefDeltaTime, AccuracyTimeMultiplier;

simulated function PostBeginPlay()
{
	super.PostBeginPlay();

	Accuracy = MaxAccuracy;
}

//simulated function ReloadDone()
//{
//	super.ReloadDone();
//
//	Accuracy = MaxAccuracy;
//}


simulated function ApplyKickback()
{
	Kickback( Kickback_WhenSteady );
}

simulated function float CalcAccuracy( int ShotsFired, float DeltaTime )
{	
	local float result;

	result = Accuracy - AccuracyTimeMultiplier*(AccuracyRefDeltaTime - ( DeltaTime ) );

	return result;
}

simulated function ClampAccuracy( out float result )
{
	result = FClamp( result, MinAccuracy, MaxAccuracy );	
}


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
	MinAccuracy = 0.55
	MaxAccuracy = 0.9
	AccuracyRefDeltaTime = 0.4
	AccuracyTimeMultiplier = 0.35

	InventoryGroup=2  // in cass, pistol is 2
	GroupWeight=0.5

	InstantHitDamageTypes(0)=class'avaDmgType_Gun'

	// Weapon SkeletalMesh
	
	WeaponFireAnim(0)	=Fire
 	WeaponPutDownAnim	=Down
	WeaponEquipAnim		=BringUp
	WeaponReloadAnim	=Reload
	EquipTime			=0.84
	PutDownTime			=0.0333
	ReloadTime			=2.2333
	WeaponIdleAnims(0)	=Idle

	MuzzleFlashSocket=MuzzleFlashSocket
	MuzzleFlashPSCTemplate=ParticleSystem'avaEffect.Particles.P_WP_Pistol_MuzzleFlash_1p'
	MuzzleFlashDuration=0.05
	// Pickup staticmesh

	// Weapon 에 의한 Pawn 의 속도 제한
	BaseSpeed			= 285	// 기본속도
	AimSpeedPct			= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.4	// 걷기시 보정치
	CrouchSpeedPct		= 0.25	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.25	// 스프린트시 보정치
	CrouchSprintSpeedPct= 1	// 앉아서 스프린트시 보정치

	BulletTemplete		= ParticleSystem'avaEffect.Gun_Effect.Ps_Wp_Pistol_cartridge'

	WeaponType			= WEAPON_PISTOL
	PickUpClass			= None				// Pistol 은 던질 수 없다...
	bCanThrow			= false		

	Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0
		bUseAsOccluder = FALSE
		DepthPriorityGroup=SDPG_Foreground
		bOnlyOwnerSee=true
		CollideActors=false
		Rotation=(Yaw=-16384,Pitch=0,Roll=0)
	End Object
	SilencerMesh=StaticMeshComponent0		

	bDropWhenDead		=	false
}
