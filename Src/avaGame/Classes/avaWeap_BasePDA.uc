// PDA Expired

/*
	PDA System �� ���� Weapon Class �̴�.
	��Ŭ������ PDA Mode ���� ��ȯ�� ��û�Ѵ�.

	1. ��Ŭ���� PDASightIn State �� ��ȯ
	2. PDASightIn State ���� ��ȯ Animation Play
	3. ��ȯ Animation �� ������ PDAState �� ��ȯ
	4. PDAState ������ Fire �� ���� ��ü�� �Ұ��� �ϴ�.
*/

class avaWeap_BasePDA extends avaWeapon;

//var float					SightInTime;
//var(Animations) name		SightInAnim;
//
//var float					SightOutTime;
//var(Animations)	name		SightOutAnim;
//
//simulated function ClosePDA();
//
//simulated state PDASightIn
//{
//	simulated function BeginState( Name PreviousStateName )
//	{
//		SetTimer( SightInTime, false, 'SightInDone');
//		PlayWeaponAnimation( SightInAnim, SightInTime );
//	}
//
//	simulated function EndState(Name NextStateName)
//	{
//		ClearTimer('SightInDone');
//	}
//
//	simulated function SightInDone()
//	{
//		GotoState('PDAState');
//	}
//
//	simulated function bool TryPutDown()
//	{
//		return false;
//	}
//
//	simulated function bool DenyClientWeaponSet()
//	{
//		return true;
//	}
//
//	simulated function BeginFire( Byte FireModeNum )
//	{
//		
//	}
//
//	simulated event bool IsFiring()
//	{
//		return true;
//	}
//}
//
//simulated state PDASightOut
//{
//	simulated function BeginState( Name PreviousStateName )
//	{
//		SetTimer( SightOutTime, false, 'SightOutDone');
//		PlayWeaponAnimation( SightOutAnim, SightOutTime );
//	}
//
//	simulated function EndState(Name NextStateName)
//	{
//		ClearTimer('SightOutDone');
//	}
//
//	simulated function SightOutDone()
//	{
//		GotoState('Active');
//	}
//
//	simulated function bool TryPutDown()
//	{
//		return false;
//	}
//
//	simulated function bool DenyClientWeaponSet()
//	{
//		return true;
//	}
//
//	simulated function BeginFire( Byte FireModeNum )
//	{
//		
//	}
//
//	simulated event bool IsFiring()
//	{
//		return true;
//	}
//}
//
//
//simulated state PDAState
//{
//	simulated function BeginState( Name PreviousStateName )
//	{
//		if ( Instigator.Controller != None && Instigator.Controller.IsLocalPlayerController())
//			avaPlayerController( Instigator.Controller ).TogglePDAMode();
//	}
//
//	simulated function ClosePDA()
//	{
//		GotoState( 'Active' );
//	}
//
//	simulated function bool DenyClientWeaponSet()
//	{
//		return false;
//	}
//
//	simulated function BeginFire( Byte FireModeNum )
//	{
//	}
//}
//
//function ConsumeAmmo( byte FireModeNum )
//{
//}
//
//defaultproperties
//{
//
//	FiringStatesArray(0)=	PDASightIn
//
//	BaseSkelMeshName	=	"Wp_PDA.MS_PDA"
//	BaseAnimSetName		=	"Wp_PDA.Ani_PDA"
//
//	WeaponFireTypes(0)	=	EWFT_Custom
//
//	InventoryGroup		=	9
//	GroupWeight			=	0.5
//
//	AmmoCount			=	1
//	MaxAmmoCount		=	1
//
//	bCanThrow			=	false
//
//	WeaponEquipAnim		=	BringUp
//	EquipTime			=	0.8333
//	PutDownTime			=	0.0
//	WeaponIdleAnims(0)	=	Idle
//
//	BaseSpeed			=	285	// �⺻�ӵ�
//	AimSpeedPct			=	0.8	// ���ؽ� ����ġ
//	WalkSpeedPct		=	0.4	// �ȱ�� ����ġ
//	CrouchSpeedPct		=	0.25	// �ɾ��̵��� ����ġ
//	CrouchAimSpeedPct	=	0.2	// �ɾƼ� ���� �̵��� ����ġ
//	SwimSpeedPct		=	0.7	// ������ ����ġ
//	SprintSpeedPct		=	1.6	// ������Ʈ�� ����ġ
//	CrouchSprintSpeedPct=	1.1	// �ɾƼ� ������Ʈ�� ����ġ
//
//	bHideWeaponMenu		=	true
//
//	SightInTime			=	0.2
//	SightInAnim			=	ZoomIn
//
//	SightOutTime		=	0.2
//	SightOutAnim		=	ZoomOut
//}
