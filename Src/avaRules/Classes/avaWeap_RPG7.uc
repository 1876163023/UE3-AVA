class avaWeap_RPG7 extends avaWeap_BaseBazooka;

defaultproperties
{
	WeaponProjectiles(0)		=	class'avaProj_RPGRocket'
	FireOffset					=	(X=0,Y=0,Z=0)  // Projectile �� ������ ��ġ
	AttachmentClass=class'avaAttachment_RPG7'
	WeaponFireSnd=SoundCue'avaWeaponSounds.Heavy_RPG7.Heavy_RPG7_fire'

	// Velocity Limit Parameter of Pawn
	BaseSpeed			=	182		// �⺻�ӵ�
	AimSpeedPct			=	0.4		// ���ؽ� ����ġ
	WalkSpeedPct			=	0.3		// �ȱ�� ����ġ
	CrouchSpeedPct			=	0.25		// �ɾ��̵��� ����ġ
	CrouchAimSpeedPct		=	0.15		// �ɾƼ� ���� �̵��� ����ġ
	SwimSpeedPct			=	0.7		// ������ ����ġ
	SprintSpeedPct			=	1.25	// ������Ʈ�� ����ġ
	CrouchSprintSpeedPct		=	1		// �ɾƼ� ������Ʈ�� ����ġ

	FireInterval(0)			=	1.2
	EquipTime			=	1.4333
	PutDownTime			=	1.0333
	ReloadTime			=	4.0333

	ClipCnt					=	1
	AmmoCount				=	1
	MaxAmmoCount				=	3

	SightInfos(0) 		= (FOV=90,ChangeTime=0.5)
	SightInfos(1) 		= (FOV=85,ChangeTime=0.5)

	ScopeMeshName			= "avaScopeUI.Distortion.MS_RPG7_Scope_Mesh"

	bReleaseZoomAfterFire = true
	fReleaseZoomAfterFireInterval = 0.3

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
}