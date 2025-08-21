class avaWeap_Knife extends avaWeap_BaseKnife;

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