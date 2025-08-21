class avaWeap_Knife extends avaWeap_BaseKnife;

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

	AttachmentClass		=	class'avaAttachment_Knife'


	MinIdleContinuousCnt	=	3
	WeaponIdleAnims.Add( Knife_Gesture )

	InstantHitDamage(0)			=	35.0
	InstantHitDamage(1)			=	70.0

	InstantHitDamageTypes(0)	=	class'avaDmgType_Knife'
	InstantHitDamageTypes(1)	=	class'avaDmgType_KnifeStab'

	HitDecisionTime(0)			=	0.12
	HitDecisionTime(1)			=	0.34

	FireInterval(0)				=	0.5
	FireInterval(1)				=	1.0

	MaxPitchLag		=	700
	MaxYawLag		=	1000
	RotLagSpeed		=	0.82
	JumpDamping		=	0.3
	BobDamping		=	0.6
	BobDampingInDash	=	0.4
}