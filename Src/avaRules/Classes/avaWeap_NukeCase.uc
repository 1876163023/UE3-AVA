class avaWeap_NukeCase extends avaWeap_BaseMissionObject;

function bool DenyPickupQuery(class<Inventory> Inv, Actor Pickup)
{
	// Light Stick 은 하나만 가져야 한다....
	if ( class<avaWeap_NukeCase>(Inv) != None )
		return true;
	return false;
}

defaultproperties
{
	BaseSpeed		= 220	// 기본속도
	AimSpeedPct		= 0.8	// 조준시 보정치
	WalkSpeedPct		= 0.42	// 걷기시 보정치
	CrouchSpeedPct		= 0.3	// 앉아이동시 보정치
	CrouchAimSpeedPct	= 0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		= 0.7	// 수영시 보정치
	SprintSpeedPct		= 1.3	// 스프린트시 보정치
	CrouchSprintSpeedPct	= 1	// 앉아서 스프린트시 보정치

	// 일단은 C4 로 대신한다...
	BaseSkelMeshName	=	"Wp_NUCpack.MS_NUCpack"
	BaseAnimSetName		=	"Wp_NUCpack.Ani_NCPack"

	AttachmentClass		=	class'avaAttachment_NukeCase'
	PickupSound=SoundCue'avaItemSounds.Item_Get2_Cue'
	bSpecialInventory	=	true
	bAvailableAbandonWeapon = true
}