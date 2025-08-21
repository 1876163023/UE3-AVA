class avaWeap_FlashBang extends avaWeap_Grenade;

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
	FireTime(0)			=	0.08		//  던지는 Animation 으로 전환후 몇초 있다 Projectile 을 생성할 것인가?
	FireTime(1)			=	0.06		//  던지는 Animation 으로 전환후 몇초 있다 Projectile 을 생성할 것인가?
}