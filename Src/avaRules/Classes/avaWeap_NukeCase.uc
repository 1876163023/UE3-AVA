class avaWeap_NukeCase extends avaWeap_BaseMissionObject;

function bool DenyPickupQuery(class<Inventory> Inv, Actor Pickup)
{
	// Light Stick �� �ϳ��� ������ �Ѵ�....
	if ( class<avaWeap_NukeCase>(Inv) != None )
		return true;
	return false;
}

defaultproperties
{
	BaseSpeed		= 220	// �⺻�ӵ�
	AimSpeedPct		= 0.8	// ���ؽ� ����ġ
	WalkSpeedPct		= 0.42	// �ȱ�� ����ġ
	CrouchSpeedPct		= 0.3	// �ɾ��̵��� ����ġ
	CrouchAimSpeedPct	= 0.2	// �ɾƼ� ���� �̵��� ����ġ
	SwimSpeedPct		= 0.7	// ������ ����ġ
	SprintSpeedPct		= 1.3	// ������Ʈ�� ����ġ
	CrouchSprintSpeedPct	= 1	// �ɾƼ� ������Ʈ�� ����ġ

	// �ϴ��� C4 �� ����Ѵ�...
	BaseSkelMeshName	=	"Wp_NUCpack.MS_NUCpack"
	BaseAnimSetName		=	"Wp_NUCpack.Ani_NCPack"

	AttachmentClass		=	class'avaAttachment_NukeCase'
	PickupSound=SoundCue'avaItemSounds.Item_Get2_Cue'
	bSpecialInventory	=	true
	bAvailableAbandonWeapon = true
}