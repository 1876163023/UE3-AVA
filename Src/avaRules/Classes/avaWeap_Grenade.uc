// avaWeap_Grenade
//	
//	1. FIRE �� �����Ѵ�.
//	2. FIRE�� ���⹰�� avaProj_Grenade �̴�.
//	3. ������ ���� �̰� �غ��ϰ�, ���� ���� ������.
//	4. �غ� �� �� ���¿��� ���� �غ� ������ �Ϸ� �� �� ������.
//	5. ������� �ʰ� ������ ���� ���Ⱑ ��������.

class avaWeap_Grenade extends avaThrowableWeapon;

defaultproperties
{
	BaseSpeed		= 265	// �⺻�ӵ�
	AimSpeedPct		= 0.8	// ���ؽ� ����ġ
	WalkSpeedPct		= 0.42	// �ȱ�� ����ġ
	CrouchSpeedPct		= 0.3	// �ɾ��̵��� ����ġ
	CrouchAimSpeedPct	= 0.2	// �ɾƼ� ���� �̵��� ����ġ
	SwimSpeedPct		= 0.7	// ������ ����ġ
	SprintSpeedPct		= 1.3	// ������Ʈ�� ����ġ
	CrouchSprintSpeedPct	= 1	// �ɾƼ� ������Ʈ�� ����ġ

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

	
	// ����ź�� �⺻������
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
	FireTime(0)			=	0.08		//  ������ Animation ���� ��ȯ�� ���� �ִ� Projectile �� ������ ���ΰ�?
	FireTime(1)			=	0.14		//  ������ Animation ���� ��ȯ�� ���� �ִ� Projectile �� ������ ���ΰ�?

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

	/// cass �� ������ ���ȿ� ����� �� ����.
	bCancelPrepare(0)	=false
	bCancelPrepare(1)	=false
	/// �غ� �� �Ǹ� ��� �ִ´�.
	bSkipReady(0)		=false
	bSkipReady(1)		=false

	ProjStartVel(0)			=	900
	ProjStartVel(1)			=	400

	// Projectile �� ������������ Speed
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


