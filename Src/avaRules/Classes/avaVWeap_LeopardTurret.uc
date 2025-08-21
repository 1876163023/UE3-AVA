/*
	Leopard의 대포.

	2007/10/24	고광록
*/
class avaVWeap_LeopardTurret extends avaVehicleWeapon
	HideDropDown;

`include(avaGame/avaGame.uci)

//! 대포를 쏠 때의 충격이 얼마나 될지 정할 수 있다.
var float FireImpulseFactor;

/*
`devexec simulated function TestImpulse()
{
	`log("avaVWeap.TestImpulse");
	MyVehicle.Mesh.AddImpulse(Vect(0,100000,0), Vect(0,0,0), 'Bone01');
}
*/

simulated function Projectile ProjectileFire()
{
	local Projectile P;
	local vector ForceLoc;

	P = Super.ProjectileFire();
	if ( (Role==ROLE_Authority) && (P != None) )
	{
		// apply force to vehicle
		MyVehicle.GetBarrelLocationAndRotation(0, ForceLoc);
		ForceLoc.Z += 150;
		`log("avaVWeap_LeopardTurret - P.Velocity=" @P.Velocity @"ForceLoc=" @ForceLoc @MyVehicle);
		// 발사되는 반대방향으로 충격을 준다.
		MyVehicle.Mesh.AddImpulse(FireImpulseFactor * P.Velocity * -1, ForceLoc, 'Bone01');
	}
	return P;
}

simulated event function RifleFire( int ShotNum )
{
	ProjectileFire();
	PlayFireEffects(0);
}

DefaultProperties
{
	//! avaWeap_BaseBazooka
	WeaponFireTypes(0)		=	EWFT_InstantHit
	WeaponProjectiles(0)		=	class'avaProj_RPGRocket'
	FireOffset			=	(X=40,Y=10)  // Projectile 이 나가는 위치
	AttachmentClass=class'avaAttachment_RPG7'
	WeaponFireSnd=SoundCue'avaWeaponSounds.Heavy_RPG7.Heavy_RPG7_fire'

	// Velocity Limit Parameter of Pawn
	BaseSpeed			=	182		// 기본속도
	AimSpeedPct			=	0.4		// 조준시 보정치
	WalkSpeedPct		=	0.3		// 걷기시 보정치
	CrouchSpeedPct		=	0.25	// 앉아이동시 보정치
	CrouchAimSpeedPct	=	0.15	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		=	0.7		// 수영시 보정치
	SprintSpeedPct		=	1.25	// 스프린트시 보정치
	CrouchSprintSpeedPct=	1		// 앉아서 스프린트시 보정치

	FireInterval(0)		=	1.2
	EquipTime			=	1.4333
	PutDownTime			=	1.0333
	ReloadTime			=	4.0333

	ClipCnt				=	100
	AmmoCount			=	100
	MaxAmmoCount		=	200

	SightInfos(0)		= (FOV=90,ChangeTime=0.2)

	bHideWeaponInSightMode = false
	bHideCursorInSightMode = false
	ScopeMeshName          = ""
	SightInAnim            = ""
	SightOutAnim           = ""
	bReleaseZoomAfterFire = false

	Kickback_WhenMoving = (UpBase=14.5,LateralBase=1.8,UpModifier=1.5,LateralModifier=0.9,UpMax=30,LateralMax=22.5,DirectionChange=3)
	Kickback_WhenFalling = (UpBase=14.5,LateralBase=1.8,UpModifier=1.5,LateralModifier=0.9,UpMax=30,LateralMax=22.5,DirectionChange=3)
	Kickback_WhenDucking = (UpBase=12.4,LateralBase=0.96,UpModifier=0.8,LateralModifier=0.48,UpMax=16,LateralMax=12,DirectionChange=3)
	Kickback_WhenSteady = (UpBase=13,LateralBase=1.2,UpModifier=1,LateralModifier=0.6,UpMax=20,LateralMax=15,DirectionChange=3)
	
	Spread_WhenFalling = (param1=-0.031,param2=0.8)
	Spread_WhenMoving = (param1=-0.1178,param2=0.24)
	Spread_WhenDucking = (param1=-0.031,param2=0.072)
	Spread_WhenSteady = (param1=-0.031,param2=0.08)
	
	AccuracyDivisor  =  200
	AccuracyOffset  =  1.5
	MaxInaccuracy  =  5
	
	Kickback_UpLimit = 10
	Kickback_LateralLimit = 3

	Kickback_WhenMovingA = (UpBase=14.5,LateralBase=1.8,UpModifier=1.5,LateralModifier=0.9,UpMax=30,LateralMax=22.5,DirectionChange=3)
	Kickback_WhenFallingA = (UpBase=14.5,LateralBase=1.8,UpModifier=1.5,LateralModifier=0.9,UpMax=30,LateralMax=22.5,DirectionChange=3)
	Kickback_WhenDuckingA = (UpBase=12.4,LateralBase=0.96,UpModifier=0.8,LateralModifier=0.48,UpMax=16,LateralMax=12,DirectionChange=3)
	Kickback_WhenSteadyA = (UpBase=13,LateralBase=1.2,UpModifier=1,LateralModifier=0.6,UpMax=20,LateralMax=15,DirectionChange=3)
	
	Spread_WhenFallingA = (param1=-0.031,param2=0.8)
	Spread_WhenMovingA = (param1=-0.1178,param2=0.24)
	Spread_WhenDuckingA = (param1=-0.031,param2=0.072)
	Spread_WhenSteadyA = (param1=-0.031,param2=0.08)
	
	AccuracyDivisorA  =  200
	AccuracyOffsetA  =  0.55
	MaxInaccuracyA  =  2.5
	
	DirectionHold = 0
	FireIntervalMultiplierA = 1


	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.6
	BobDamping		=	0.7
	BobDampingInDash		=	0.0

 //	WeaponFireTypes(0)=EWFT_Projectile
//	WeaponProjectiles(0)=class'UTProj_TankShell'
 //	WeaponFireTypes(1)=EWFT_None

//	WeaponFireSnd(0)=SoundCue'A_Vehicle_Goliath.SoundCues.A_Vehicle_Goliath_Fire'

//	FireInterval(0)=+2.5
//	FireInterval(1)=+2.5
//	ShotCost(0)=0
//	ShotCost(1)=0

//	FireTriggerTags=(GoliathTurret)

//	FireCameraAnim[0]=CameraAnim'VH_Goliath.PrimaryFireViewShake'

	Spread[0]=0.015
	Spread[1]=0.015

//	bZoomedFireMode(1)=1

//	ZoomedTargetFOV=40.0
//	ZoomedRate=60.0

	// 임시로 일단(이거땜에 Turret의 방향이 엉망이 되었다)
	// Weapon.MaxRange() -> avaVehicleWeapon.PostBeginPlay()에서 AimTraceRange에 저장후
	// avaVehicle.ApplyWeaponRotation()함수에서 참조해서 사용되어 진다.
	bInstantHit=true

	//! avaWeap_BaseBazooka
//	bCanThrow			=	true
	WeaponType			=	WEAPON_RPG

	DamageCode=Gun

	bAutoFire			=	false	// fire 1.
	bInfinityAmmo		=	true
	bEjectBulletWhenFire= false

	FireImpulseFactor = 27000
}