/*=============================================================================
  avaMod_Weapon
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2006/07/24 by OZ
		
		Weapon Modifier 이다.

=============================================================================*/
class avaMod_Weapon extends avaModifier
	abstract;

/*! @brief 무기 병과별 슬롯.
	@note
		DefType.h 에 정의되어 있다.

		_EP_R1 = (1 << 1),
		_EP_R2 = (1 << 2),
		_EP_R3 = (1 << 3),
		_EP_R4 = (1 << 4),
		_EP_P1 = (1 << 5),
		_EP_P2 = (1 << 6),
		_EP_P3 = (1 << 7),
		_EP_P4 = (1 << 8),
		_EP_S1 = (1 << 9),
		_EP_S2 = (1 << 10),
		_EP_S3 = (1 << 11),
		_EP_S4 = (1 << 12),

		단, 여기서 재정의 되는 값은 shift되는 값이다.
		(실제로 R2=P2=S2, R3=P3=S3, R4=P4=S4가 같은 무기며 동일하게 장착된다)
*/
enum EWeaponClassSlot
{
	WEAPON_CLASS_SLOT_NONE,

	WEAPON_CLASS_SLOT_P1,		//!< PointMan 주무기.
	WEAPON_CLASS_SLOT_P2,		//!< PointMan 보조무기(권총류).
	WEAPON_CLASS_SLOT_P3,		//!< PointMan 근접무기.
	WEAPON_CLASS_SLOT_P4,		//!< PointMan 투척무기.

	WEAPON_CLASS_SLOT_R1,		//!< RilfeMan 주무기.
	WEAPON_CLASS_SLOT_R2,		//!< RilfeMan 보조무기(권총류).
	WEAPON_CLASS_SLOT_R3,		//!< RilfeMan 근접무기.
	WEAPON_CLASS_SLOT_R4,		//!< RilfeMan 투척무기.

	WEAPON_CLASS_SLOT_S1,		//!< Sniper 주무기.
	WEAPON_CLASS_SLOT_S2,		//!< Sniper 보조무기(권총류).
	WEAPON_CLASS_SLOT_S3,		//!< Sniper 근접무기.
	WEAPON_CLASS_SLOT_S4,		//!< Sniper 투척무기.
};


/*! @brief 슬롯 정보 추가(2007/01/19 고광록).
	@note
		DefType.h - enum CUSTOM_SLOT_IDX에서 가져옴.

		_CSI_FRONT = 0,
		_CSI_MOUNT = 1,
		_CSI_BARREL = 2,
		_CSI_TRIGGER = 3,
		_CSI_GRIP = 4,
		_CSI_STOCK = 5,
*/
enum EWeaponSlot
{
	// 장착 아이템.
	WEAPON_SLOT_Front,			//!< 
	WEAPON_SLOT_Mount,			//!< 
	WEAPON_SLOT_Barrel,			//!< 
	WEAPON_SLOT_Trigger,		//!< 
	WEAPON_SLOT_Grip,			//!< 
	WEAPON_SLOT_Stock,			//!< 

	//{ 아래는 사용하지 않는다.
//	WEAPON_SLOT_SMG,			//!< 주무기(PointMan).
//	WEAPON_SLOT_Rifle,			//!< 주무기(RifleMan).
//	WEAPON_SLOT_SniperRifle,	//!< 주무기(Sniper).
//	WEAPON_SLOT_Pistol,			//!< 보조무기(권총류).
//	WEAPON_SLOT_Melee,			//!< 근접무기.
//	WEAPON_SLOT_Grenade,		//!< 투척무기.
	//}

	WEAPON_SLOT_None,			//!< 기본값.

//	WEAPON_SLOT_MAX
};

//! 무기 슬롯.
var EWeaponSlot			Slot;

var class<avaWeapon>	WeaponClass;	// WeaponClass 가 None 이 아니라면 Modifier가 아니라 Weapon 자체를 가르킨다...


var string				ScopeMeshName;

static simulated function PreCache( out array< object > outList )
{
	Super.Precache( outList );
	DLO( default.ScopeMeshName, outList );
	DLO( default.ScopeMeshName$"_3p", outList );
}

static event LoadDLOs()
{	
	local array< object > outList;
	Super.LoadDLOs();
	PreCache( outList );
}

// 정비도 관련한 값들을 적용한다.
static simulated function ApplyMaintenanceRate( avaWeapon Weap, float rate )
{
	local avaWeap_BaseGun	BaseGun;
	local float				DmgAmp;
	local float				ZoomAmp;
	local float				UnZoomAmp;
	local float				RangeMod;

	DmgAmp		= 1.0;
	ZoomAmp		= 1.0;
	UnZoomAmp	= 1.0;
	RangeMod	= 0.0;

	BaseGun = avaWeap_BaseGun( Weap );

	if ( avaWeap_BaseSMG( Weap ) != None )
	{
		if ( rate <= 0 )
		{
			DmgAmp		= 0.5;
			ZoomAmp		= 1.6;
			UnZoomAmp	= 1.6;
		}
		else if ( rate < 20 )
		{
			DmgAmp		= 0.75;
			ZoomAmp		= 1.4;
			UnZoomAmp	= 1.4;
		}
		else if ( rate < 80 )
		{
			DmgAmp		= 0.9;
			ZoomAmp		= 1.2;
			UnZoomAmp	= 1.2;
		}
	}
	else if ( avaWeap_BaseRifle( Weap ) != None )
	{
		if ( rate <= 0 )
		{
			DmgAmp		= 0.5;
			ZoomAmp		= 1.4;
			UnZoomAmp	= 1.4;
			RangeMod	= -0.2;
		}
		else if ( rate < 20 )
		{
			DmgAmp		= 0.7;
			ZoomAmp		= 1.3;
			UnZoomAmp	= 1.3;
			RangeMod	= -0.2;
		}
		else if ( rate < 80 )
		{
			DmgAmp		= 0.9;
			ZoomAmp		= 1.1;
			UnZoomAmp	= 1.1;
			RangeMod	= -0.05;
		}
	}
	else if ( avaWeap_BaseSniperRifle( Weap ) != None )
	{
		if ( rate <= 0 )
		{
			DmgAmp		= 0.3;
			ZoomAmp		= 3.0;
		}
		else if ( rate < 20 )
		{
			DmgAmp		= 0.5;
			ZoomAmp		= 2.0;
		}
		else if ( rate < 80 )
		{
			DmgAmp		= 0.8;
			ZoomAmp		= 1.5;
		}
	}

	if ( BaseGun != None )
	{
		BaseGun.HitDamage		= BaseGun.HitDamage * DmgAmp;
		BaseGun.HitDamageS		= BaseGun.HitDamageS * DmgAmp;
		BaseGun.UnZoomSpreadAmp	= BaseGun.UnZoomSpreadAmp * UnZoomAmp;
		BaseGun.ZoomSpreadAmp	= BaseGun.ZoomSpreadAmp * ZoomAmp;
		BaseGun.RangeModifier	+= RangeMod;
		BaseGun.RangeModifierS	+= RangeMod;
	}
}

defaultproperties
{
	Slot = WEAPON_SLOT_None
}

/*=============================================================================
// Server Side 에서 해야 하는 것과 Client Side 에서 해야 하는 것들을 구분해야 한다...

1.	ChagneBodySkin( name )
	@	name	: Material 이름
	
	- Client Side
		Weapon 의 Body Skin 을 바꿔준다.

2.	ChagneStockSkin( name )
	@	name	: Material 이름
	
	- Client Side
		Weapon 의 Stock Skin 을 바꿔준다.

3.	ChagneGripSkin( name )
	@	name	: Material 이름
	
	- Client Side
		Weapon 의 Grip Skin 을 바꿔준다.


4.	AddItemMesh( name, name )
	@	name	: Static Mesh Name
		name	: Socket Name

	- Client Side
		Weapon 에 Item 을 Visual적으로 붙여준다.
		(DotSight, Laser Point 등등등....)
=============================================================================*/


