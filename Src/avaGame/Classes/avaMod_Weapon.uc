/*=============================================================================
  avaMod_Weapon
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2006/07/24 by OZ
		
		Weapon Modifier �̴�.

=============================================================================*/
class avaMod_Weapon extends avaModifier
	abstract;

/*! @brief ���� ������ ����.
	@note
		DefType.h �� ���ǵǾ� �ִ�.

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

		��, ���⼭ ������ �Ǵ� ���� shift�Ǵ� ���̴�.
		(������ R2=P2=S2, R3=P3=S3, R4=P4=S4�� ���� ����� �����ϰ� �����ȴ�)
*/
enum EWeaponClassSlot
{
	WEAPON_CLASS_SLOT_NONE,

	WEAPON_CLASS_SLOT_P1,		//!< PointMan �ֹ���.
	WEAPON_CLASS_SLOT_P2,		//!< PointMan ��������(���ѷ�).
	WEAPON_CLASS_SLOT_P3,		//!< PointMan ��������.
	WEAPON_CLASS_SLOT_P4,		//!< PointMan ��ô����.

	WEAPON_CLASS_SLOT_R1,		//!< RilfeMan �ֹ���.
	WEAPON_CLASS_SLOT_R2,		//!< RilfeMan ��������(���ѷ�).
	WEAPON_CLASS_SLOT_R3,		//!< RilfeMan ��������.
	WEAPON_CLASS_SLOT_R4,		//!< RilfeMan ��ô����.

	WEAPON_CLASS_SLOT_S1,		//!< Sniper �ֹ���.
	WEAPON_CLASS_SLOT_S2,		//!< Sniper ��������(���ѷ�).
	WEAPON_CLASS_SLOT_S3,		//!< Sniper ��������.
	WEAPON_CLASS_SLOT_S4,		//!< Sniper ��ô����.
};


/*! @brief ���� ���� �߰�(2007/01/19 ����).
	@note
		DefType.h - enum CUSTOM_SLOT_IDX���� ������.

		_CSI_FRONT = 0,
		_CSI_MOUNT = 1,
		_CSI_BARREL = 2,
		_CSI_TRIGGER = 3,
		_CSI_GRIP = 4,
		_CSI_STOCK = 5,
*/
enum EWeaponSlot
{
	// ���� ������.
	WEAPON_SLOT_Front,			//!< 
	WEAPON_SLOT_Mount,			//!< 
	WEAPON_SLOT_Barrel,			//!< 
	WEAPON_SLOT_Trigger,		//!< 
	WEAPON_SLOT_Grip,			//!< 
	WEAPON_SLOT_Stock,			//!< 

	//{ �Ʒ��� ������� �ʴ´�.
//	WEAPON_SLOT_SMG,			//!< �ֹ���(PointMan).
//	WEAPON_SLOT_Rifle,			//!< �ֹ���(RifleMan).
//	WEAPON_SLOT_SniperRifle,	//!< �ֹ���(Sniper).
//	WEAPON_SLOT_Pistol,			//!< ��������(���ѷ�).
//	WEAPON_SLOT_Melee,			//!< ��������.
//	WEAPON_SLOT_Grenade,		//!< ��ô����.
	//}

	WEAPON_SLOT_None,			//!< �⺻��.

//	WEAPON_SLOT_MAX
};

//! ���� ����.
var EWeaponSlot			Slot;

var class<avaWeapon>	WeaponClass;	// WeaponClass �� None �� �ƴ϶�� Modifier�� �ƴ϶� Weapon ��ü�� ����Ų��...


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

// ���� ������ ������ �����Ѵ�.
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
// Server Side ���� �ؾ� �ϴ� �Ͱ� Client Side ���� �ؾ� �ϴ� �͵��� �����ؾ� �Ѵ�...

1.	ChagneBodySkin( name )
	@	name	: Material �̸�
	
	- Client Side
		Weapon �� Body Skin �� �ٲ��ش�.

2.	ChagneStockSkin( name )
	@	name	: Material �̸�
	
	- Client Side
		Weapon �� Stock Skin �� �ٲ��ش�.

3.	ChagneGripSkin( name )
	@	name	: Material �̸�
	
	- Client Side
		Weapon �� Grip Skin �� �ٲ��ش�.


4.	AddItemMesh( name, name )
	@	name	: Static Mesh Name
		name	: Socket Name

	- Client Side
		Weapon �� Item �� Visual������ �ٿ��ش�.
		(DotSight, Laser Point ����....)
=============================================================================*/


