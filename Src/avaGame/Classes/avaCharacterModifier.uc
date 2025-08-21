/*=============================================================================
  avaCharacterModifier
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2006/07/20 by OZ
		
		Character Part ����� ���� Modifier �̴�...

=============================================================================*/
class avaCharacterModifier extends avaModifier
	abstract;

// Modifier �� Physical �� ��ġ Enum

//enum CHAR_SLOT
//{
//	CHAR_SLOT_HEAD,
//};


//! �������� (2007/01/19 ����).
enum ECharSlot
{
	CHAR_SLOT_NONE,				//!< None.

	CHAR_SLOT_H1,				//!< Helmet.
	CHAR_SLOT_H1_1,				//!< Helmet NightVision Goggles.
	CHAR_SLOT_H1_2,				//!< Helmet Accessory.
	CHAR_SLOT_H2,				//!< Glass.
	CHAR_SLOT_H3,				//!< Mask.

	CHAR_SLOT_C1,				//!< Chest Upper Right.
	CHAR_SLOT_C2,				//!< Chest Upper Left.
//	CHAR_SLOT_C3,				//!< Chest Lower Right.
//	CHAR_SLOT_C4,				//!< Chest Lower Middle.
//	CHAR_SLOT_C5,				//!< Chest Lower Left.

	CHAR_SLOT_A1,				//!< Abdomen Right.
	CHAR_SLOT_A2,				//!< Abdomen Left.
	CHAR_SLOT_B1,				//!< Back Left.
	CHAR_SLOT_B3,				//!< Back Right.
	CHAR_SLOT_W1,				//!< Waist Left.
	CHAR_SLOT_W2,				//!< Waist Middle.
	CHAR_SLOT_W3,				//!< Waist Right.
	CHAR_SLOT_T1,				//!< Thigh Right.
	CHAR_SLOT_T2,				//!< Thigh Left.
	CHAR_SLOT_E,				//!< Elbow.

	CHAR_SLOT_G,				//!< Glove.			// Do not use for Rendering
	CHAR_SLOT_K,				//!< Knee.			// Do not use for Rendering
	CHAR_SLOT_BT,				//!< Boots.			// Do not use for Rendering
	CHAR_SLOT_BD,				//!< Body.
	CHAR_SLOT_HEAD,				//!< Head(��).

	CHAR_SLOT_MARK,				//!< Mark(��ũ).

//	CHAR_SLOT_M1,				//!< Mission Object1.
//	CHAR_SLOT_M2,				//!< Mission Object2.

	// ��ȹ�� ���� �κ� �߰�(�������� ��� ������ ��� ���Ѵ�).
	CHAR_SLOT_U,				//!< Military Uniform(����).

//	CHAR_SLOT_MAX
};

var ECharSlot			Slot;					// ĳ���� ���� ����.

//var array<CHAR_SLOT>	CharacterSlots;			// Modifier �� ��ġ�� �����ϴ� array

var array<AttachedItem>	EURifleAttachedItems;	// EU RifleMan �� items
var array<AttachedItem> EUPointAttachedItems;	// EU PintMan �� Items
var array<AttachedItem> EUSniperAttachedItems;	// EU Sniper �� Items

var array<AttachedItem>	NRFRifleAttachedItems;	// NRF RifleMan �� Items
var array<AttachedItem> NRFPointAttachedItems;	// NRF PintMan �� Items
var array<AttachedItem> NRFSniperAttachedItems;	// NRF Sniper �� Items

var array<ExtraMesh>	EURifleExtraMeshes;		// EU RifleMan �� Extra Meshes
var array<ExtraMesh>	EUPointExtraMeshes;		// EU PintMan �� Extra Meshes
var array<ExtraMesh>	EUSniperExtraMeshes;	// EU Sniper �� Extra Meshes
												
var array<ExtraMesh>	NRFRifleExtraMeshes;	// NRF RifleMan �� Extra Meshes
var array<ExtraMesh>	NRFPointExtraMeshes;	// NRF PintMan �� Extra Meshes
var array<ExtraMesh>	NRFSniperExtraMeshes;	// NRF Sniper �� Extra Meshes

var ExtraMesh			EURifleHelmetMesh;	// EU RifleMan �� Helmet Meshes
var ExtraMesh			EUPointHelmetMesh;	// EU PintMan �� Helmet Meshes
var ExtraMesh			EUSniperHelmetMesh;	// EU Sniper �� Helmet Meshes
									
var ExtraMesh			NRFRifleHelmetMesh;	// NRF RifleMan �� Helmet Meshes
var ExtraMesh			NRFPointHelmetMesh;	// NRF PintMan �� Helmet Meshes
var ExtraMesh			NRFSniperHelmetMesh;// NRF Sniper �� Helmet Meshes

var array<AttachedItem>	EURifleHelmetAttachedItems;	// EU RifleMan �� items
var array<AttachedItem> EUPointHelmetAttachedItems;	// EU PintMan �� Items
var array<AttachedItem> EUSniperHelmetAttachedItems;	// EU Sniper �� Items

var array<AttachedItem>	NRFRifleHelmetAttachedItems;	// NRF RifleMan �� Items
var array<AttachedItem> NRFPointHelmetAttachedItems;	// NRF PintMan �� Items
var array<AttachedItem> NRFSniperHelmetAttachedItems;	// NRF Sniper �� Items
	
var string				HeadMeshName;					// Head Mesh
var string				HeadMTSName;					// Head Morph Target Set Name
var string				EyeballMeshName;				// Eye Ball Mesh


static function ExtraMesh	GetHelmetMesh( int nTeam, int nClass )
{
	if ( nTeam == 0 )
	{
		switch( nClass )
		{
		case 0:		return default.EUPointHelmetMesh;
		case 1:		return default.EURifleHelmetMesh;
		case 2:		return default.EUSniperHelmetMesh;
		}
	}
	else if ( nTeam == 1 )
	{
		switch( nClass )
		{
		case 0:		return default.NRFPointHelmetMesh;
		case 1:		return default.NRFRifleHelmetMesh;
		case 2:		return default.NRFSniperHelmetMesh;
		}
	}
}

static function array<AttachedItem> GetHelmetAttachedItems( int nTeam, int nClass )
{
	if ( nTeam == 0 )
	{
		switch( nClass )
		{
		case 0:		return default.EUPointHelmetAttachedItems;	
		case 1:		return default.EURifleHelmetAttachedItems;
		case 2:		return default.EUSniperHelmetAttachedItems;
		}
	}
	else if ( nTeam == 1 )
	{
		switch( nClass )
		{
		case 0:		return default.NRFPointHelmetAttachedItems;
		case 1:		return default.NRFRifleHelmetAttachedItems;
		case 2:		return default.NRFSniperHelmetAttachedItems;
		}
	}
}

static function array<AttachedItem> GetAttachedItems( int nTeam, int nClass )	
{
	if ( nTeam == 0 )
	{
		switch( nClass )
		{
		case 0:		return default.EUPointAttachedItems;
		case 1:		return default.EURifleAttachedItems;
		case 2:		return default.EUSniperAttachedItems;
		}
	}
	else if ( nTeam == 1 )
	{
		switch( nClass )
		{
		case 0:		return default.NRFPointAttachedItems;
		case 1:		return default.NRFRifleAttachedItems;
		case 2:		return default.NRFSniperAttachedItems;
		}
	}
}

static function array<ExtraMesh>	GetExtraMeshes( int nTeam, int nClass )
{
	if ( nTeam == 0 )
	{
		switch( nClass )
		{
		case 0:		return default.EUPointExtraMeshes;
		case 1:		return default.EURifleExtraMeshes;
		case 2:		return default.EUSniperExtraMeshes;
		}
	}
	else if ( nTeam == 1 )
	{
		switch( nClass )
		{
		case 0:		return default.NRFPointExtraMeshes;
		case 1:		return default.NRFRifleExtraMeshes;
		case 2:		return default.NRFSniperExtraMeshes;
		}
	}
}

// �ش� Modifier �� Slot �� �ߺ��Ǵ��� Check �Ѵ�.
// �ߺ��Ǵ� Slot �� �ϳ��� ������ true �̴�.
//static function bool CheckSlotDup( class<avaCharacterModifier> Mod )
//{
//	local int i,j;
//	for( i = 0 ; i < default.CharacterSlots.length ; ++ i )
//	{
//		for ( j = 0 ; j < Mod.default.CharacterSlots.length ; ++ j )
//		{
//			if ( default.CharacterSlots[i] == Mod.default.CharacterSlots[j] )	return true;
//		}
//	}
//	return false;
//}

static function DLO_AttachedItems( array<AttachedItem> Items, out array<object> outList )
{
	local int i;
	for( i = 0 ; i < Items.Length; ++i )
		DLO( Items[i].MeshName, outList );
}

static function DLO_ExtraMeshes( array<ExtraMesh> Items, out array<object> outList )
{
	local int i;
	for( i = 0 ; i < Items.Length; ++i )
		DLO( Items[i].MeshName, outList );
}

static simulated function PreCache( out array<object> outlist )
{
	DLO_AttachedItems( default.EURifleAttachedItems, outList );	// EU RifleMan �� items
	DLO_AttachedItems( default.EUPointAttachedItems, outList );	// EU PintMan �� Items
	DLO_AttachedItems( default.EUSniperAttachedItems, outList );	// EU Sniper �� Items

	DLO_AttachedItems( default.NRFRifleAttachedItems, outList );	// NRF RifleMan �� Items
	DLO_AttachedItems( default.NRFPointAttachedItems, outList );	// NRF PintMan �� Items
	DLO_AttachedItems( default.NRFSniperAttachedItems, outList );	// NRF Sniper �� Items

	DLO_ExtraMeshes( default.EURifleExtraMeshes, outList );		// EU RifleMan �� Extra Meshes
	DLO_ExtraMeshes( default.EUPointExtraMeshes, outList );		// EU PintMan �� Extra Meshes
	DLO_ExtraMeshes( default.EUSniperExtraMeshes, outList );	// EU Sniper �� Extra Meshes
							
	DLO_ExtraMeshes( default.NRFRifleExtraMeshes, outList );	// NRF RifleMan �� Extra Meshes
	DLO_ExtraMeshes( default.NRFPointExtraMeshes, outList );	// NRF PintMan �� Extra Meshes
	DLO_ExtraMeshes( default.NRFSniperExtraMeshes, outList );	// NRF Sniper �� Extra Meshes	

	DLO( default.EURifleHelmetMesh.MeshName, outList );
	DLO( default.EUPointHelmetMesh.MeshName, outList );
	DLO( default.EUSniperHelmetMesh.MeshName, outList );

	DLO( default.NRFRifleHelmetMesh.MeshName, outList );
	DLO( default.NRFPointHelmetMesh.MeshName, outList );
	DLO( default.NRFSniperHelmetMesh.MeshName, outList );

	DLO_AttachedItems( default.EURifleHelmetAttachedItems, outList );	// EU RifleMan �� items
	DLO_AttachedItems( default.EUPointHelmetAttachedItems, outList );	// EU PintMan �� Items
	DLO_AttachedItems( default.EUSniperHelmetAttachedItems, outList );	// EU Sniper �� Items

	DLO_AttachedItems( default.NRFRifleHelmetAttachedItems, outList );	// NRF RifleMan �� Items
	DLO_AttachedItems( default.NRFPointHelmetAttachedItems, outList );	// NRF PintMan �� Items
	DLO_AttachedItems( default.NRFSniperHelmetAttachedItems, outList );	// NRF Sniper �� Items

	DLO( default.HeadMeshName, outList );
	DLO( default.HeadMTSName, outList );
	DLO( default.EyeballMeshName, outList );
}

static event LoadDLOs()
{	
	local array<object> outList;
	Super.LoadDLOs();
	PreCache( outList );
}

/*=============================================================================

// Server Side ���� �ؾ� �ϴ� �Ͱ� Client Side ���� �ؾ� �ϴ� �͵��� �����ؾ� �Ѵ�...


//================================ Visual Modifier ================================
1.	AddItemMesh( string, name )
	@	string	: StaticMesh �̸�
		name	: Socket �̸�
		
	- Client Side
	Pawn �� Socket �� StaticMesh �� �����Ѵ�.
	���� �������� visual ������ �����ϱ� ���ؼ� ����Ѵ�.

2.	AddExtraMesh( string )
	@	string	: SkeletalMesh �̸�
	
	- Client Side
	Pawn �� Body Part �� �߰��Ѵ�. 

3.	SetHelmet( string, bool, optional string )
	@	string	: SkeletalMesh �̸�
		bool	: Damage �� �޾����� Helmet �� �������� �ִ°�
		string	: Helmet Skin Material �̸� (������ Skin ���� Helmet Skin �� �����Ѵ�. �������� ������ Default Material �� Setting �ȴ�.)

	- Client Side
	Pawn �� Helmet �� �����Ѵ�.

4.	AddHelmetAccessory( string, name )
	@	string	: StaticMesh �̸�
		name	: Socket �̸�

	- Client Side
	Pawn �� Helmet �� Accessory �� �����Ѵ�.
	

=============================================================================*/


