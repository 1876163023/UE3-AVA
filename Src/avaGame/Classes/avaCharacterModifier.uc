/*=============================================================================
  avaCharacterModifier
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2006/07/20 by OZ
		
		Character Part 등등을 위한 Modifier 이다...

=============================================================================*/
class avaCharacterModifier extends avaModifier
	abstract;

// Modifier 의 Physical 한 위치 Enum

//enum CHAR_SLOT
//{
//	CHAR_SLOT_HEAD,
//};


//! 슬롯정보 (2007/01/19 고광록).
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
	CHAR_SLOT_HEAD,				//!< Head(얼굴).

	CHAR_SLOT_MARK,				//!< Mark(마크).

//	CHAR_SLOT_M1,				//!< Mission Object1.
//	CHAR_SLOT_M2,				//!< Mission Object2.

	// 기획에 없는 부분 추가(서버에도 없어서 실제로 사용 못한다).
	CHAR_SLOT_U,				//!< Military Uniform(군복).

//	CHAR_SLOT_MAX
};

var ECharSlot			Slot;					// 캐릭터 슬롯 정보.

//var array<CHAR_SLOT>	CharacterSlots;			// Modifier 의 위치를 지정하는 array

var array<AttachedItem>	EURifleAttachedItems;	// EU RifleMan 용 items
var array<AttachedItem> EUPointAttachedItems;	// EU PintMan 용 Items
var array<AttachedItem> EUSniperAttachedItems;	// EU Sniper 용 Items

var array<AttachedItem>	NRFRifleAttachedItems;	// NRF RifleMan 용 Items
var array<AttachedItem> NRFPointAttachedItems;	// NRF PintMan 용 Items
var array<AttachedItem> NRFSniperAttachedItems;	// NRF Sniper 용 Items

var array<ExtraMesh>	EURifleExtraMeshes;		// EU RifleMan 용 Extra Meshes
var array<ExtraMesh>	EUPointExtraMeshes;		// EU PintMan 용 Extra Meshes
var array<ExtraMesh>	EUSniperExtraMeshes;	// EU Sniper 용 Extra Meshes
												
var array<ExtraMesh>	NRFRifleExtraMeshes;	// NRF RifleMan 용 Extra Meshes
var array<ExtraMesh>	NRFPointExtraMeshes;	// NRF PintMan 용 Extra Meshes
var array<ExtraMesh>	NRFSniperExtraMeshes;	// NRF Sniper 용 Extra Meshes

var ExtraMesh			EURifleHelmetMesh;	// EU RifleMan 용 Helmet Meshes
var ExtraMesh			EUPointHelmetMesh;	// EU PintMan 용 Helmet Meshes
var ExtraMesh			EUSniperHelmetMesh;	// EU Sniper 용 Helmet Meshes
									
var ExtraMesh			NRFRifleHelmetMesh;	// NRF RifleMan 용 Helmet Meshes
var ExtraMesh			NRFPointHelmetMesh;	// NRF PintMan 용 Helmet Meshes
var ExtraMesh			NRFSniperHelmetMesh;// NRF Sniper 용 Helmet Meshes

var array<AttachedItem>	EURifleHelmetAttachedItems;	// EU RifleMan 용 items
var array<AttachedItem> EUPointHelmetAttachedItems;	// EU PintMan 용 Items
var array<AttachedItem> EUSniperHelmetAttachedItems;	// EU Sniper 용 Items

var array<AttachedItem>	NRFRifleHelmetAttachedItems;	// NRF RifleMan 용 Items
var array<AttachedItem> NRFPointHelmetAttachedItems;	// NRF PintMan 용 Items
var array<AttachedItem> NRFSniperHelmetAttachedItems;	// NRF Sniper 용 Items
	
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

// 해당 Modifier 와 Slot 이 중복되는지 Check 한다.
// 중복되는 Slot 이 하나라도 있으면 true 이다.
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
	DLO_AttachedItems( default.EURifleAttachedItems, outList );	// EU RifleMan 용 items
	DLO_AttachedItems( default.EUPointAttachedItems, outList );	// EU PintMan 용 Items
	DLO_AttachedItems( default.EUSniperAttachedItems, outList );	// EU Sniper 용 Items

	DLO_AttachedItems( default.NRFRifleAttachedItems, outList );	// NRF RifleMan 용 Items
	DLO_AttachedItems( default.NRFPointAttachedItems, outList );	// NRF PintMan 용 Items
	DLO_AttachedItems( default.NRFSniperAttachedItems, outList );	// NRF Sniper 용 Items

	DLO_ExtraMeshes( default.EURifleExtraMeshes, outList );		// EU RifleMan 용 Extra Meshes
	DLO_ExtraMeshes( default.EUPointExtraMeshes, outList );		// EU PintMan 용 Extra Meshes
	DLO_ExtraMeshes( default.EUSniperExtraMeshes, outList );	// EU Sniper 용 Extra Meshes
							
	DLO_ExtraMeshes( default.NRFRifleExtraMeshes, outList );	// NRF RifleMan 용 Extra Meshes
	DLO_ExtraMeshes( default.NRFPointExtraMeshes, outList );	// NRF PintMan 용 Extra Meshes
	DLO_ExtraMeshes( default.NRFSniperExtraMeshes, outList );	// NRF Sniper 용 Extra Meshes	

	DLO( default.EURifleHelmetMesh.MeshName, outList );
	DLO( default.EUPointHelmetMesh.MeshName, outList );
	DLO( default.EUSniperHelmetMesh.MeshName, outList );

	DLO( default.NRFRifleHelmetMesh.MeshName, outList );
	DLO( default.NRFPointHelmetMesh.MeshName, outList );
	DLO( default.NRFSniperHelmetMesh.MeshName, outList );

	DLO_AttachedItems( default.EURifleHelmetAttachedItems, outList );	// EU RifleMan 용 items
	DLO_AttachedItems( default.EUPointHelmetAttachedItems, outList );	// EU PintMan 용 Items
	DLO_AttachedItems( default.EUSniperHelmetAttachedItems, outList );	// EU Sniper 용 Items

	DLO_AttachedItems( default.NRFRifleHelmetAttachedItems, outList );	// NRF RifleMan 용 Items
	DLO_AttachedItems( default.NRFPointHelmetAttachedItems, outList );	// NRF PintMan 용 Items
	DLO_AttachedItems( default.NRFSniperHelmetAttachedItems, outList );	// NRF Sniper 용 Items

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

// Server Side 에서 해야 하는 것과 Client Side 에서 해야 하는 것들을 구분해야 한다...


//================================ Visual Modifier ================================
1.	AddItemMesh( string, name )
	@	string	: StaticMesh 이름
		name	: Socket 이름
		
	- Client Side
	Pawn 의 Socket 에 StaticMesh 를 부착한다.
	각종 아이템을 visual 적으로 부착하기 위해서 사용한다.

2.	AddExtraMesh( string )
	@	string	: SkeletalMesh 이름
	
	- Client Side
	Pawn 에 Body Part 를 추가한다. 

3.	SetHelmet( string, bool, optional string )
	@	string	: SkeletalMesh 이름
		bool	: Damage 를 받았을때 Helmet 이 벗겨질수 있는가
		string	: Helmet Skin Material 이름 (지정한 Skin 으로 Helmet Skin 을 변경한다. 지정하지 않으면 Default Material 로 Setting 된다.)

	- Client Side
	Pawn 에 Helmet 을 부착한다.

4.	AddHelmetAccessory( string, name )
	@	string	: StaticMesh 이름
		name	: Socket 이름

	- Client Side
	Pawn 의 Helmet 에 Accessory 를 부착한다.
	

=============================================================================*/


