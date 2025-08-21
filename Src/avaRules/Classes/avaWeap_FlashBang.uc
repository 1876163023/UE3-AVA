class avaWeap_FlashBang extends avaWeap_Grenade;

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

	BaseSkelMeshName	=	"Wp_Grenade_MK3A2.MS_MK3A2_1p"
	BaseAnimSetName		=	"Wp_Grenade_MK3A2.MK3A2_Ani"

	AttachmentClass=class'avaAttachment_FlashBang'

	WeaponProjectiles(0)=class'avaProj_FlashBang'
	WeaponProjectiles(1)=class'avaProj_FlashBang'

	WeaponIdleAnims(1)	=	MK3A2_Gesture

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.6
	BobDampingInDash	=	0.4

	ThrowAnim(0)		=	Fire1
	ThrowTime(0)		=	0.60
	ThrowAnim(1)		=	Fire2
	ThrowTime(1)		=	0.60
	FireTime(0)			=	0.08		//  ������ Animation ���� ��ȯ�� ���� �ִ� Projectile �� ������ ���ΰ�?
	FireTime(1)			=	0.06		//  ������ Animation ���� ��ȯ�� ���� �ִ� Projectile �� ������ ���ΰ�?
}