class avaWeap_BaseLightStick extends avaThrowableWeapon;

var float	LightStickUseTime;
var string	LightOnMaterial;
var Material LightOnMtrl;

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();
	LightOnMtrl = Material( DynamicLoadObject( LightOnMaterial, class'Material' ) );
}

function bool DenyPickupQuery(class<Inventory> Inv, Actor Pickup)
{
	// Light Stick 은 하나만 가져야 한다....
	if ( class<avaWeap_BaseLightStick>(Inv) != None )
		return true;
	return false;
}

simulated event Notify_TurnOnLight()
{
	if ( Instigator.IsLocallyControlled() )
	{
		avaAttachment_BaseLightStick( WeaponAttachment ).EnableLight();
		// Material Change
		if ( LightOnMtrl != None )	Mesh.SetMaterial( 0, LightOnMtrl );
	}
}

simulated state WeaponPreparing
{
	simulated function BeginState( Name PreviousStateName )
	{
		Super.BeginState( PreviousStateName );
		if ( Role==ROLE_Authority )
			avaPawn(Instigator).PlayPullPinAnimation();
	}
}

simulated state WeaponReady
{
	simulated function BeginState( Name PreviousStateName )
	{
		Super.BeginState( PreviousStateName );
		if ( Role == ROLE_Authority )
		{
			LightStickUseTime	= WorldInfo.TimeSeconds;
			avaPawn(Instigator).ChangeWeaponState( 1 );
		}
	}
}

function SpawnProjectile()
{
	local float			UsedTime;
	local vector		RealStartLoc;
	local Projectile	SpawnedProjectile;
	
	UsedTime = WorldInfo.TimeSeconds - LightStickUseTime;

	// this is the location where the projectile is spawned.
	RealStartLoc = GetPhysicalFireStartLoc();

	// Spawn projectile
	if ( WeaponProjectiles[CurrentFireMode] != None )
	{
		SpawnedProjectile = Spawn( WeaponProjectiles[CurrentFireMode], Self,, RealStartLoc);
		avaProj_BaseLightStick( SpawnedProjectile ).SetUsedTime( UsedTime );
		if( SpawnedProjectile != None && !SpawnedProjectile.bDeleteMe )
		{
			SpawnedProjectile.Init( Vector(GetAdjustedAim( RealStartLoc )) );
			if ( avaProjectile(SpawnedProjectile) != none )
			{
				avaProjectile(SpawnedProjectile).InitStats(self);
			}
		}
	}

	if ( Instigator.IsLocallyControlled() )
		avaAttachment_BaseLightStick( WeaponAttachment ).DisableLight();
}

static simulated function Precache( out array<object> outList )
{
	Super.Precache( outList );
	DLO( default.LightOnMaterial, outList );
}

static event LoadDLOs()
{
	
	Super.LoadDLOs();
}

defaultproperties
{
	BaseSpeed				=	265		// 기본속도
	AimSpeedPct				=	0.8		// 조준시 보정치
	WalkSpeedPct			=	0.42	// 걷기시 보정치
	CrouchSpeedPct			=	0.3		// 앉아이동시 보정치
	CrouchAimSpeedPct		=	0.2		// 앉아서 조준 이동시 보정치
	SwimSpeedPct			=	0.7		// 수영시 보정치
	SprintSpeedPct			=	1.3		// 스프린트시 보정치
	CrouchSprintSpeedPct	=	1		// 앉아서 스프린트시 보정치

	BaseSkelMeshName		=	"Wp_LightStick.MS_LightStick_EU"
	BaseAnimSetName			=	"Wp_LightStick.Ani_LighitSick"

	AmmoCount				=	1
	MaxAmmoCount			=	1
	WeaponRange				=	300
	
	InventoryGroup			=	4
	GroupWeight				=	0.4

	WeaponColor				=	(R=255,G=0,B=0,A=255)

	AttachmentClass			=	class'avaAttachment_BaseLightStick'
	WeaponFireSnd			=	None

	FireShake=(OffsetMag=(X=-5.0,Y=-5.00,Z=-5.00),OffsetRate=(X=-1000.0,Y=-1000.0,Z=-1000.0),OffsetTime=2,RotMag=(X=0.0,Y=0.0,Z=0.0),RotRate=(X=0.0,Y=0.0,Z=0.0),RotTime=2)	

	WeaponProjectiles(0)	=	class'avaProj_BaseLightStick'
	WeaponProjectiles(1)	=	class'avaProj_BaseLightStick'

	WeaponFireAnim(0)		=	None
	WeaponFireAnim(1)		=	None
 	WeaponPutDownAnim		=	None
	WeaponEquipAnim			=	BringUp
	EquipTime				=	0.7
	PutDownTime				=	0
	WeaponIdleAnims(0)		=	Idle
	WeaponIdleAnims(1)		=	Idle2
	MinIdleContinuousCnt	=	3

	PrepareAnim(0)			=	Pullpin
	PrepareTime(0)			=	0.8333
	PrepareSnd(0)			=	None
	PrepareAnim(1)			=	Pullpin
	PrepareTime(1)			=	0.8333
	PrepareSnd(1)			=	None
	ThrowAnim(0)			=	Fire1
	ThrowTime(0)			=	0.6333
	ThrowAnim(1)			=	Fire2
	ThrowTime(1)			=	0.6333
	ThrowEndTime(0)			=	0.0
	ThrowEndTime(1)			=	0.0
	FireTime(0)				=	0.08		//  던지는 Animation 으로 전환후 몇초 있다 Projectile 을 생성할 것인가?
	FireTime(1)				=	0.14		//  던지는 Animation 으로 전환후 몇초 있다 Projectile 을 생성할 것인가?

	FiringStatesArray(0)	=	WeaponPreparing
	FiringStatesArray(1)	=	WeaponPreparing

	WeaponFireTypes(0)		=	EWFT_Projectile
	WeaponFireTypes(1)		=	EWFT_Projectile

	FireOffsetEx(0)			=	(X=20,Y=10,Z=10)
	FireOffsetEx(1)			=	(X=17,Y=12,Z=-5)

	ProjZAng(0)				=	0.28
	ProjZAng(1)				=	0.05	

	ThrowEndRadioSoundIndex(0)	=	-1
	ThrowEndRadioSoundIndex(1)	=	-1

	bCancelPrepare(0)		=	false
	bCancelPrepare(1)		=	false
	bSkipReady(0)			=	false
	bSkipReady(1)			=	false

	ProjStartVel(0)			=	650
	ProjStartVel(1)			=	400

	ProjTossZ(0)			=	130
	ProjTossZ(1)			=	30
	
	DropOnlyOneAmmo			=	true
	bAlwaysDrawIcon			=	true

	WeaponType				=	WEAPON_GRENADE
	PickupSound				=	SoundCue'avaItemSounds.Item_Get_Cue'

	MaxPitchLag				=	700
	MaxYawLag				=	1000
	RotLagSpeed				=	0.82
	JumpDamping				=	0.3
	BobDamping				=	0.6
	BobDampingInDash		=	0.4
	bDisableChangeState		=	true

	LightOnMaterial			=	"Wp_LightStick.MT_LightStick_EU_on"
}