// PDA Expired

/*
	PDA System 을 위한 Weapon Class 이다.
	좌클릭으로 PDA Mode 로의 전환을 요청한다.

	1. 좌클릭시 PDASightIn State 로 전환
	2. PDASightIn State 에서 전환 Animation Play
	3. 전환 Animation 이 끝나면 PDAState 로 전환
	4. PDAState 에서는 Fire 및 무기 교체가 불가능 하다.
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
//	BaseSpeed			=	285	// 기본속도
//	AimSpeedPct			=	0.8	// 조준시 보정치
//	WalkSpeedPct		=	0.4	// 걷기시 보정치
//	CrouchSpeedPct		=	0.25	// 앉아이동시 보정치
//	CrouchAimSpeedPct	=	0.2	// 앉아서 조준 이동시 보정치
//	SwimSpeedPct		=	0.7	// 수영시 보정치
//	SprintSpeedPct		=	1.6	// 스프린트시 보정치
//	CrouchSprintSpeedPct=	1.1	// 앉아서 스프린트시 보정치
//
//	bHideWeaponMenu		=	true
//
//	SightInTime			=	0.2
//	SightInAnim			=	ZoomIn
//
//	SightOutTime		=	0.2
//	SightOutAnim		=	ZoomOut
//}
