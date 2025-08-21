// avaWeap_Grenade
//	
//	1. FIRE 만 존재한다.
//	2. FIRE의 사출물은 avaProj_Grenade 이다.
//	3. 누르면 핀을 뽑고 준비하고, 떼는 순간 던진다.
//	4. 준비가 덜 된 상태에서 떼면 준비 동작이 완료 된 후 던진다.
//	5. 사용하지 않고 죽으면 땅에 무기가 떨어진다.

class avaWeap_Grenade extends avaThrowableWeapon;

defaultproperties
{
	BaseSpeed		= 265	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치

	BaseSkelMeshName	=	"Wp_M67.MS_M67"
	BaseAnimSetName		=	"Wp_M67.Ani_M67"

	AmmoCount=1
	MaxAmmoCount=1

	WeaponRange=300
	
	InventoryGroup=4
	GroupWeight=0.4

	WeaponColor=(R=255,G=0,B=0,A=255)

	AttachmentClass=class'avaAttachment_Grenade'
	WeaponFireSnd=None
	FireShake=(OffsetMag=(X=-5.0,Y=-5.00,Z=-5.00),OffsetRate=(X=-1000.0,Y=-1000.0,Z=-1000.0),OffsetTime=2,RotMag=(X=0.0,Y=0.0,Z=0.0),RotRate=(X=0.0,Y=0.0,Z=0.0),RotTime=2)	

	WeaponProjectiles(0)=class'avaProj_GrenadeM67'
	WeaponProjectiles(1)=class'avaProj_GrenadeM67'

	WeaponFireAnim(0)	=	None
	WeaponFireAnim(1)	=	None
 	WeaponPutDownAnim	=	None
	WeaponEquipAnim		=	BringUp
	EquipTime			=	0.8333
	PutDownTime			=	0
	WeaponIdleAnims(0)	=	Idle
	WeaponIdleAnims(1)	=	M67_Gesture
	MinIdleContinuousCnt=	3

	
	// 수류탄의 기본값들임
	PrepareAnim(0)		=	PinUp
	PrepareTime(0)		=	0.5667
	PrepareSnd(0)		=	None

	PrepareAnim(1)		=	PinUp
	PrepareTime(1)		=	0.5667
	PrepareSnd(1)		=	None

	//CancelAnim		=	WeaponAltFireLaunch1End
	//CancelTime=   0.3
	//CancelSnd=

	ThrowAnim(0)		=	Fire1
	ThrowTime(0)		=	0.60

	ThrowAnim(1)		=	Fire2
	ThrowTime(1)		=	0.60
	//ThrowSnd=
	//ThrowEndAnim		=	BringUp
	ThrowEndTime(0)		=	0.0
	ThrowEndTime(1)		=	0.0
	//ThrowEndSnd=
	FireTime(0)			=	0.08		//  던지는 Animation 으로 전환후 몇초 있다 Projectile 을 생성할 것인가?
	FireTime(1)			=	0.14		//  던지는 Animation 으로 전환후 몇초 있다 Projectile 을 생성할 것인가?

	FiringStatesArray(0)=	WeaponPreparing
	FiringStatesArray(1)=	WeaponPreparing

	WeaponFireTypes(0)	=	EWFT_Projectile
	WeaponFireTypes(1)	=	EWFT_Projectile

	FireOffsetEx(0)		=	(X=20,Y=10,Z=10)
	FireOffsetEx(1)		=	(X=17,Y=12,Z=-5)

	ProjZAng(0)		=	0.28
	ProjZAng(1)		=	0.05	

	ThrowEndRadioSoundIndex(0)	=	AUTOMESSAGE_FireInTheHole
	ThrowEndRadioSoundIndex(1)	=	-1

	/// cass 는 누르는 동안에 취소할 수 없다.
	bCancelPrepare(0)	=false
	bCancelPrepare(1)	=false
	/// 준비가 다 되면 들고 있는다.
	bSkipReady(0)		=false
	bSkipReady(1)		=false

	ProjStartVel(0)			=	900
	ProjStartVel(1)			=	400

	// Projectile 의 윗방향으로의 Speed
	ProjTossZ(0)			=	150
	ProjTossZ(1)			=	50
	
	DropOnlyOneAmmo		=	true
	bAlwaysDrawIcon		=	true

	WeaponType			=	WEAPON_GRENADE
	PickupSound=SoundCue'avaItemSounds.Item_Get_Cue'

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.6
	BobDampingInDash	=	0.4
}


