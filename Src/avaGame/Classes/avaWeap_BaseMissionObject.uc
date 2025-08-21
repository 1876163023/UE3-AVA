/*
	���� �̼��� ���� Class �̴�.
	Mouse ��Ŭ���� �ƹ��͵� Binding �Ǿ� ���� �ʴ´�...
	Mouse ��Ŭ������ ���� �� �ִ�.
*/

class avaWeap_BaseMissionObject extends avaWeapon
	native;

/// ���� �ʰ� ���� ��� ���� ������.
simulated function OwnerEvent( name eventName )
{
	if ( eventName == 'died' )
	{
		ThrowWeapon(true);
	}
}

simulated function bool CanThrow()
{
	return true;
}

simulated function StartFire(byte FireModeNum)
{

}
//simulated state Active
//{
//	simulated function BeginFire( byte FireModeNum )
//	{
//		if ( FireModeNum == 1 )
//		{
//			ThrowWeapon();
//			ClearPendingFire( FireModeNum );
//		}
//	}
//}

simulated function DrawWeaponCrosshair( Hud HUD )
{

}

defaultproperties
{
	InventoryGroup		=	5
	GroupWeight			=	0.5

	AmmoCount			=	1
	MaxAmmoCount		=	1

	bCanThrow			=	true
	PickUpClass			=	class'avaPickUp'
	ThrowOffset			=	(X=0,Y=0,Z=-12)

	WeaponEquipAnim		=	BringUp
	EquipTime			=	0.8333
	PutDownTime			=	0.0
	WeaponIdleAnims(0)	=	Idle

	BaseSpeed			=	285	// �⺻�ӵ�
	AimSpeedPct			=	0.8	// ���ؽ� ����ġ
	WalkSpeedPct		=	0.4	// �ȱ�� ����ġ
	CrouchSpeedPct		=	0.25	// �ɾ��̵��� ����ġ
	CrouchAimSpeedPct	=	0.2	// �ɾƼ� ���� �̵��� ����ġ
	SwimSpeedPct		=	0.7	// ������ ����ġ
	SprintSpeedPct		=	1.6	// ������Ʈ�� ����ġ
	CrouchSprintSpeedPct=	1.1	// �ɾƼ� ������Ʈ�� ����ġ
}


