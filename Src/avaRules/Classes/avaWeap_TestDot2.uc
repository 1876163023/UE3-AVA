/** 
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaWeap_TestDot2 extends avaWeap_BaseRifle;

var() array< string >					TestDotName;
var() SkeletalMeshComponent				BaseDotComp;
var() array< SkeletalMeshComponent >	TestDotComp;

simulated exec function ResetTestDot()
{
	AttachItems();
}

simulated function AttachItems()
{
	local SkeletalMeshComponent	ExtraMesh;
	local int					i;
	local vector				translation;
	local rotator				rot;

	if ( TestDotComp.length > 0 )
	{
		DetachComponent( TestDotComp[0] );
		TestDotComp.length = 0;
	}

	for ( i = 0 ; i < TestDotName.length ; ++  i )
	{
		ExtraMesh = new(outer) class'avaSkeletalMeshComponent';
		ExtraMesh.bUseAsOccluder = FALSE;
		ExtraMesh.SetShadowParent( Mesh );
		ExtraMesh.SetDepthPriorityGroup( SDPG_Foreground );
		ExtraMesh.SetOnlyOwnerSee( true );
		ExtraMesh.bUpdateSkelWhenNotRendered = false;
		ExtraMesh.bCastDynamicShadow = false;
		ExtraMesh.SetSkeletalMesh( SkeletalMesh( DynamicLoadObject( TestDotName[i], class'SkeletalMesh' ) ) );
		
		TestDotComp[TestDotComp.length] = ExtraMesh;

		if ( i == 0 )	
		{
			translation.x	= 5.0; 
			translation.y	= -PlayerViewOffset.y;
			translation.z	= -3.0;
			rot.Yaw			= -90;
			rot.Pitch		= 0;
			rot.Roll		= 0;
			ExtraMesh.SetTranslation( translation );
			AttachComponent( ExtraMesh );
			ExtraMesh.SetRotation( rot );
		}
		else			
		{
			ExtraMesh.SetParentAnimComponent( TestDotComp[0] );
			TestDotComp[0].AttachComponent( ExtraMesh, 'Bone01' );

		}
	}
}

simulated function ChangeVisibility(bool bIsVisible)
{
	Super.ChangeVisibility( bIsVisible );
	ArmComponent.SetHidden( true );
	HandComponent.SetHidden( true );
	Mesh.SetHidden( true );
}

simulated function SetLightEnvironment( LightEnvironmentComponent env )
{
	local int i;	
	for ( i = 0 ; i < TestDotComp.Length ; ++ i )
	{
		if ( TestDotComp[i] != None )
			TestDotComp[i].SetLightEnvironment( env );				
	}
	Super.SetLightEnvironment( env );
}


defaultproperties
{

	BulletType				=	class'avaBullet_556NATO'	

	BaseSpeed			= 260	// 기본속도
	AimSpeedPct			= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.4	// 걷기시 보정치
	CrouchSpeedPct		= 0.25	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.25	// 스프린트시 보정치
	CrouchSprintSpeedPct= 1	// 앉아서 스프린트시 보정치

	HitDamage			=	33
	FireInterval(0)			=	0.085
	
	ReloadCnt			=	30
	AmmoCount			=	30
	MaxAmmoCount			=	90

	Penetration			=	2
	RangeModifier			=	0.9

	AccuracyDivisor			=	800
	AccuracyOffset			=	0.25
	MaxInaccuracy			=	3

	SpreadDecayTime = 0.5

	Kickback_WhenMoving = (UpBase=0.66,LateralBase=0.55,UpModifier=0.22,LateralModifier=0.11,UpMax=18.7,LateralMax=4.4,DirectionChange=5)
	Kickback_WhenFalling = (UpBase=1.2,LateralBase=1,UpModifier=0.4,LateralModifier=0.2,UpMax=34,LateralMax=8,DirectionChange=5)
	Kickback_WhenDucking = (UpBase=0.42,LateralBase=0.35,UpModifier=0.14,LateralModifier=0.07,UpMax=11.9,LateralMax=2.8,DirectionChange=5)
	Kickback_WhenSteady = (UpBase=0.6,LateralBase=0.5,UpModifier=0.2,LateralModifier=0.1,UpMax=17,LateralMax=4,DirectionChange=5)
	
	Spread_WhenFalling = (param1=0.03,param2=0.12)
	Spread_WhenMoving = (param1=0.011,param2=0.044)
	Spread_WhenDucking = (param1=0.0025,param2=0.01)
	Spread_WhenSteady = (param1=0.005,param2=0.02)

	MaxPitchLag					=	700
	MaxYawLag					=	1000
	RotLagSpeed					=	0.82
	JumpDamping					=	0.3
	BobDamping					=	0.7
	BobDampingInDash			=	0.45

	EquipTime					=	1.0333
	PutDownTime					=	0.1
	ReloadTime					=	2.967
	SightInfos(0)				=	(FOV=90,ChangeTime=0.2)

	AttachmentClass				=	class'avaAttachment_M4A1'

	BaseSkelMeshName			=	"Wp_Rif_M4.Wp_Rif_M4_Rail.MS_M4_Rail"
	BaseAnimSetName				=	"Wp_Rif_M4.Ani_M4A1_1P"

	FireAnimInfos(0)			=	(AnimName=Fire3,FirstShotRate=0.2,OtherShotRate=0.05)
	FireAnimInfos(1)			=	(AnimName=Fire2,FirstShotRate=0.8,OtherShotRate=0.3)
	FireAnimInfos(2)			=	(AnimName=Fire1,FirstShotRate=0.0,OtherShotRate=0.65)
	WeaponFireAnim(0)			=	Fire3
	WeaponFireAnim(1)			=	Fire2
	WeaponFireAnim(2)			=	Fire1
 	WeaponPutDownAnim			=	Down
	WeaponEquipAnim				=	BringUp
	WeaponReloadAnim			=	Reload

	WeaponIdleAnims(0)			=	Idle
	HudMaterial					=	Texture2D'avaDotSightUI.Textures.M4_dotsight'
	PickupSound					=	SoundCue'avaItemSounds.Item_Get_Cue'

//	DefaultModifiers(0)			=	class'avaRules.avaMod_M4A1_M_Dot'
//임시로 홀로사이트 장비
	InstantHitDamageTypes(0)	=	class'avaDmgType_Rifle'

	TestDotName(0)				=	"Highmesh.Holosight.1"
	TestDotName(1)				=	"Highmesh.Holosight.2"
	TestDotName(2)				=	"Highmesh.Holosight.3"
	TestDotName(3)				=	"Highmesh.Holosight.4"
	TestDotName(4)				=	"Highmesh.Holosight.5"
	TestDotName(5)				=	"Highmesh.Holosight.6"
	TestDotName(6)				=	"Highmesh.Holosight.7"
}
