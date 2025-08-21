/*
	수송 미션을 위한 Class 이다.
	Mouse 좌클릭은 아무것도 Binding 되어 있지 않는다...
	Mouse 우클릭으로 버릴 수 있다.
*/

class avaWeap_BaseMissionObject extends avaWeapon
	native;

/// 쓰지 않고 죽은 경우 땅에 떨군다.
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

	BaseSpeed			=	285	// 기본속도
	AimSpeedPct			=	0.8	// 조준시 보정치
	WalkSpeedPct		=	0.4	// 걷기시 보정치
	CrouchSpeedPct		=	0.25	// 앉아이동시 보정치
	CrouchAimSpeedPct	=	0.2	// 앉아서 조준 이동시 보정치
	SwimSpeedPct		=	0.7	// 수영시 보정치
	SprintSpeedPct		=	1.6	// 스프린트시 보정치
	CrouchSprintSpeedPct=	1.1	// 앉아서 스프린트시 보정치
}


