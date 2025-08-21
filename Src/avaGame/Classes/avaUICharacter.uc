/*
	UI Scene�� ������� ���Ǵ� ĳ���� Ŭ����.

	2007/05/14	����
		�Ǽ�����(AttachedItem)���� ��� ������� ��ȹ�� ������ DB������ �����ϸ�
		3D������ ��ǥ �޽��� ����Ͽ� ����Ѵ�.
		(��޽��� Helmet, Body, Armor, Item, Head+Eye �̷��� ���еȴ�)

		@note
			bUpdateSkelWhenNotRendered = true�� �����ָ� ù �����ӿ��� �ִϸ��̼��� 
			������ �ȵǴ���.

	2007/02/08	����
		���⸸ �ٸ� ���� ������ �ִϸ��̼��� �ٲ���� Playtime�� �����ؼ� �����ؼ�
		������ Ƣ�� �ʰ� �ε巴�� ����ǵ��� ����.

	2007/02/01	����
		LocalPlayerController���� PRI, PMI�� �����鼭 Server���������� �� �� ���� �Ǿ�
		PRI.bInitLastInfo�� ���� �˻��ϵ��� ����.

	2007/01/18	����
		avaCharacter.uc, avaUICharacterPIP.uc������ �����ؼ� ����.
*/
class avaUICharacter extends avaCharacter
	dependson(avaCharacterModifier)
	dependson(avaGame);

`include(avaGame/avaGame.uci)


//! ĳ������ �ִϸ��̼� Ʈ���� ������ ����.
enum EUICharAnimTree
{
	UICA_Channel,
	UICA_Ready,

	UICA_None,
//	UICA_Test,
//	UICA_Max
};

/*! @brief �� ������ �����Ѵ�.
	@note
		���� avaPawn.uc���� �̹� TypeID�� ������ ������ �����ϰ� �ִ�.
		(ChangeTeam, ChangeClass���� ��ȯ �����ϰ� �� �����̴�.)
*/
var ETeamType								TeamID;

//! �������� ���� ���� ������ �����Ѵ�.
var bool									bUpdateTeamClass;

//! ������ �ִϸ��̼� ����.
var() EUICharAnimTree						AnimTreeType;
//! ���⺰�� �ִϸ��̼��� ������ �� �ð��� �����ϴ��� ����.
var bool									bSaveAnimSeqTime;
//! ���� �������� ����� �ִϸ��̼� �ð�.
var float									LastAnimTime;
//! ���������� ����� �ִϸ��̼� ���̾�(ex. _Idle, _BackTurn ... )
var string									LastAnimSuffix;
//!
var int										ActiveChildIndex;
//! �ϵ��� ������ġ.
var vector									TurnAnimStartPos;
//! �ϵ��� ����ȸ����.
var rotator									TurnAnimStartRot;
//! �ϵ��� ��� Ƚ��.
var int										TurnAnimCount;
//! �ϵ������ΰ�?
var bool									bPlayingTurnAnim;
//! 
var bool									bNeedInitTurnAnim;
//!
var bool									bLastLookAtEnabled;
//!
var vector									TargetLocation;
//! 
var string									CurAnimSuffix;


//! ���� ĳ���� Ŭ����.
var() class<avaCharacter>					CharClass;
//! ��, ������ ĳ���� Ŭ����.(6 = `MAX_PLAYER_CLASS * `MAX_TEAM).
var array< class<avaCharacter> >			CharClasses;
//! �����ϰ� �ִ� ĳ���� Modifier���� ����.
var() array< class<avaCharacterModifier> >	Modifiers;


//! ���� ����.
var class< avaWeapon >						WeaponClass;
//! ������ ���� ������ ����.
var array< class<avaMod_Weapon> >			WeaponModifiers;


/*
	@note
		������ ���� ���������� ������ �ȴ�.

		Mesh									=> �⺻ ����.
			- BodyMeshes(bone)					=> ��ü �޽��� �߰� ���� ��Ų�޽�.
			- BodyItemMeshes(socket)			=> ���� �߰� ���� �޽�.
			- HelmetMesh(socket)				=> ��� �޽�.
				- HelmetItemMeshes				=> ����� �߰� ���� �޽�.
			- HeadMesh(bone)					=> �� �޽�.
			- EyeMesh(socket)					=> �� �޽�.
			- WeaponSkeleton(bone)				=> ������ ��ġ ������ ����.
				- WeaponSocketSkeleton(bone)	=> ������ ������ ���� ���Ͽ� ����.
					- WeaponBasicMesh(bone)		=> ������ �⺻ �޽�.
					- WeaponItemMeshes(socket)	=> ������ �߰� ���� �޽�.
*/

//! ����� �ܼ��� Ż���� ���ؼ� �߰��� ����.
var SkeletalMeshComponent					HelmetComp;


//! ���� ���� ����.
struct WeaponPart
{
	var string					MeshName;		//!< ���� ������ �̸�.
	var name					SocketName;		//!< ���� ���� �̸�.
	var StaticMeshComponent		ItemMesh;		//!< ���� ������ �޽�.
	var EWeaponSlot				Slot;			//!< ���� ����.
};


//! ������ ��ġ ������ �޽�(only Skeleton).
var MeshComponent							WeaponMesh;
//! ������ ������ġ ������ �޽�.
var SkeletalMeshComponent					WeaponSocMesh;
//! ������ �⺻ �޽�.
var StaticMeshComponent						WeaponBasicMesh;
//! ������ ���� �޽�.
var array<WeaponPart>						WeaponItemParts;

//! Kismet���� ���޵� ���� ��鿡���� LookAt����.
var bool									bUISceneLookAtEnable;
//! �ִϸ��̼��� LookAtó���� �ȵǵ��� ������ ����.
var bool									bLockedLookAtEnabled;


var array< string >							BaseModifierName;

//-----------------------------------------------------------------------------
//	functions
//-----------------------------------------------------------------------------

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	// '��������'�� ���߰� �;�~!!
	if ( Mesh != None )
		Mesh.SkelVisibleTime = 1.0f;

	TurnAnimStartPos = Location;
	TurnAnimStartRot = Rotation;


	// �����Ϳ����� �� �� �ֵ��� ����.(â������ ī�޶� ���� �� �ʿ��ϼż�...)
//	`if(`notdefined(FINAL_RELEASE))
	ApplyAllModifiers();
	InitAnim(AnimTreeType);
//	`endif
}

/*! @brief SkeletalMeshComponent�� ������ �ش�.
	@param ThisMeshName
		������ �޽� �̸�.
	@copy avaUICharacterPIP
*/
function SkeletalMeshComponent CreateSkeletalMesh( string ThisMeshName )
{
	local SkeletalMeshComponent NewMesh;
	local SkeletalMesh tmpSkelMesh;
	
	NewMesh = new(self) class'avaSkeletalMeshComponent';
	NewMesh.bUseAsOccluder = false;

	if ( ThisMeshName != "" )
	{
		tmpSkelMesh = SkeletalMesh( DynamicLoadObject( ThisMeshName, class'SkeletalMesh' ) );
		if ( tmpSkelMesh != None )
			NewMesh.SetSkeletalMesh( tmpSkelMesh );
		else
			`warn( "Load SkeletalMesh" @ThisMeshName );
	}
	else
	{
		`warn( "Not Initialize Mesh" );
	}

	NewMesh.bUpdateSkelWhenNotRendered=true;
	NewMesh.bCastDynamicShadow=true;
	NewMesh.CastShadow  = true;
	NewMesh.ForcedLodModel = 1;

	NewMesh.SetShadowParent( Mesh );

	return NewMesh;
}

/*! @brief StaticMeshComponent�� ������ �ش�.(copy avaUICharacterPIP).
	@param ThisMeshName
		������ �޽� �̸�.
	@copy avaUICharacterPIP
*/
function StaticMeshComponent CreateStaticMesh( string ThisMeshName )
{
	local StaticMeshComponent NewMesh;
	local StaticMesh tmpStaticMesh;

	NewMesh = new(self) class'StaticMeshComponent';
	NewMesh.bUseAsOccluder = FALSE;
	if ( ThisMeshName != "" )
	{
		tmpStaticMesh = StaticMesh( DynamicLoadObject( ThisMeshName, class'StaticMesh' ) );
		if ( tmpStaticMesh != None )
			NewMesh.SetStaticMesh( tmpStaticMesh );
		else
			`warn( "Cannot Load StaticMesh" @ThisMeshName );
	}
	else
	{
		`warn( "Not Initialize Mesh" );
	}

	NewMesh.bCastDynamicShadow=true;
	NewMesh.SetShadowParent( Mesh );
	NewMesh.ForcedLodModel = 1;

	return NewMesh;
}

/*! @brief ExtraMesh�� �߰�(Slot�߰���).
	@note
		CollectCharacterMesh���� ����.
	@copy
		AddExtraMesh (avaCharacter.uc)
*/
//simulated function AddExtraMeshSlot( string MeshName, ECharSlot Slot )
//{
//	local int id;
//	id							= BodyParts.length;
//	BodyParts.length			= id + 1;
//	BodyParts[id].MeshName		= MeshName;
//	BodyParts[id].Slot			= Slot;
//}
//
///*! @brief ItemMesh�� �߰�(Slot�߰���).
//	@note
//		CollectCharacterMesh���� ����.
//	@copy
//		AddItemMesh (avaCharacter.uc)
//*/
//simulated function AddItemMeshSlot( string MeshName, name SocketName, ECharSlot Slot )
//{
//	local int id;
//	id							= ItemParts.length;
//	ItemParts.length			= id + 1;
//	ItemParts[id].MeshName		= MeshName;
//	ItemParts[id].SocketName	= SocketName;
//	ItemParts[id].Slot			= Slot;
//}
//
///*! @brief HelmetAccessory�� �߰�(Slot�߰���).
//	@note
//		CollectCharacterMesh���� ����.
//	@copy
//		AddHelmetAccessory (avaCharacter.uc)
//*/
//simulated function AddHelmetAccessorySlot( string MeshName, name SocketName, ECharSlot Slot )
//{
//	local int id;
//	id								= HelmetAccessory.length;
//	HelmetAccessory.length			= id + 1;
//	HelmetAccessory[id].MeshName	= MeshName;
//	HelmetAccessory[id].SocketName	= SocketName;
//	HelmetAccessory[id].Slot		= Slot;
//}

/*! @brief Character Modifier���� ĳ������ �޽� ������ ��´�.
	@note
		�����ϱ� ���ؼ��� ApplyCharacterModifier�Լ��� ȣ��Ǿ�� �Ѵ�.
*/
simulated function CollectCharacterMesh( class<avaCharacterModifier> Mod )
{
	HeadModPriority	=	-1;
	ApplyCharacterModifier( Mod, TeamID );
	//local int i;
	//local ExtraMesh				HelmetMesh;
	//local array< ExtraMesh >	ExtraMeshes;
	//local array< AttachedItem >	AttachedItems;
	//local ECharSlot				Slot;

	//if ( Mod == None )
	//{
	//	`log("CollectCharacterMesh - (Mod == None)");
	//	return;
	//}

	//// ���õ� ĳ���� Ŭ�������� ���� ���� ������
	//// ����� �Լ����� TeamID, TypeID�� �����Ǿ� �־�� �Ѵ�.

	//// Modifier���� ���� ������ ���´�.
	//Slot = Mod.default.Slot;

	//// Body�� ���� �޽��� ���´�.
	//for ( i = 0 ; i < Mod.default.CommonExtraMeshes.length ; ++ i )
	//	AddExtraMeshSlot( Mod.default.CommonExtraMeshes[i].MeshName, Slot );

	//// Body�� ��, ������ �޽��� ���´�.
	//ExtraMeshes = Mod.static.GetExtraMeshes( TeamID, TypeID );
	//for ( i = 0 ; i < ExtraMeshes.length ; ++i )
	//	AddExtraMeshSlot( ExtraMeshes[i].MeshName, Slot );

	//// BodyItem�� ���� �޽��� ���´�.
	//for ( i = 0 ; i < Mod.default.CommonAttachedItems.length ; ++ i )
	//	AddItemMeshSlot( Mod.default.CommonAttachedItems[i].MeshName, Mod.default.CommonAttachedItems[i].PrimarySocket, Slot );

	//// BodyItem�� ��, ������ �޽��� ���´�.
	//AttachedItems = Mod.static.GetAttachedItems( TeamID, TypeID );
	//for ( i = 0 ; i < AttachedItems.length ; ++ i )
	//	AddItemMeshSlot( AttachedItems[i].MeshName , AttachedItems[i].PrimarySocket, Slot );

	//// Helmet�� ��, ������ �޽��� ���´�.
	//HelmetMesh = Mod.static.GetHelmetMesh( TeamID, TypeID );
	//if ( HelmetMesh.MeshName != "" )
	//	SetHelmet( HelmetMesh.MeshName );

	//// HelmetItem�� ��, ������ �޽��� ���´�.
	//AttachedItems = Mod.static.GetHelmetAttachedItems( TeamID, TypeID );
	//for ( i = 0 ; i < AttachedItems.length ; ++ i )
	//	AddHelmetAccessorySlot( AttachedItems[i].MeshName , AttachedItems[i].PrimarySocket, Slot );

	//// �Ӹ��� ���� �޽��̸� ���.
	//if ( Mod.default.HeadMeshName != "" )
	//	HeadMeshName = Mod.default.HeadMeshName;

	//// ����Ǯ(Morph��)�� �޽� �̸� ���.
	//if ( Mod.default.HeadMTSName != "" )
	//	HeadMTSName = Mod.default.HeadMTSName;

	//// ������(LookAt��)�� �޽� �̸� ���.
	//if ( Mod.default.EyeBallMeshName != "" )
	//	EyeBallMeshName = Mod.default.EyeBallMeshName;
}

//! Modifier Slot���� �ش��ϴ� �޽��� �����Ѵ�.
simulated function RemoveCharacterMesh( ECharSlot Slot )
{
	local int i;

//	`log("### RemoveCharacterMesh - " @Slot);

	// ���� �޽� ����.
	i = 0;
	while ( i < BodyParts.Length )
	{
		if ( BodyParts[i].Slot == Slot )
		{
			Mesh.DetachComponent( BodyParts[i].BodyMesh );
			BodyParts.Remove(i, 1);
		}
		else
		{
			++i;
		}
	}

	// ���� ������ �޽� ����.
	i = 0;
	while ( i < ItemParts.Length )
	{
		if ( ItemParts[i].Slot == Slot )
		{
			Mesh.DetachComponent( ItemParts[i].ItemMesh );
			ItemParts.Remove(i, 1);
		}
		else
		{
			++i;
		}
	}

	// ��� ������ �޽� ����.
	if ( Slot == CHAR_SLOT_H1 )
	{
		if( HelmetComp != None )
			Mesh.DetachComponent( HelmetComp );

		HelmetComp     = None;
		HelmetMeshName = "";
		HelmetSkinName = "";

		// ��信 ���� �ִ� ������ �޽��� ����.
		// (Collect�� ���·θ� ���־ ���� �����Ǵ� Helmet�� �����ǵ��� �Ѵ�)
		for ( i = 0; i < HelmetAccessory.Length; ++ i)
		{
			Mesh.DetachComponent( HelmetAccessory[i].ItemMesh );
			HelmetAccessory[i].ItemMesh = None;
		}
	}

	// ��� ������ �޽� ����.
	i = 0;
	while ( i < HelmetAccessory.Length )
	{
		if ( HelmetAccessory[i].Slot == Slot )
		{
			Mesh.DetachComponent( HelmetAccessory[i].ItemMesh );
			HelmetAccessory.Remove(i, 1);
		}
		else
		{
			++i;
		}
	}

	// �� ����.
	if ( Slot == CHAR_SLOT_HEAD )
	{
		if ( HeadComp != None )
		{
			Mesh.DetachComponent( HeadComp );
			HeadComp = None;
		}
		if ( EyeBallComp != None )
		{
			Mesh.DetachComponent( EyeBallComp );
			EyeBallComp = None;
		}

		HeadMeshName    = "";
		HeadMTSName     = "";
		EyeBallMeshName = "";
	}
}

//! ���� �޽��� ����.
simulated function RemoveWeaponMesh()
{
	local int i;

	if ( WeaponMesh != None )
	{
		if ( WeaponSocMesh != None )
		{
			// ���� ���� ����.
			for ( i = 0; i < WeaponItemParts.Length; ++i)
				WeaponSocMesh.DetachComponent( WeaponItemParts[i].ItemMesh );

			WeaponItemParts.Length = 0;

			// �ֹ��� ����.
			if( WeaponBasicMesh != None )
			{
				WeaponSocMesh.DetachComponent( WeaponBasicMesh );
				WeaponBasicMesh = None;
			}

			// ���� �޽� ����.
			SkeletalMeshComponent(WeaponMesh).DetachComponent( WeaponSocMesh );
			WeaponSocMesh = None;
		}

		// ���� ���� �޽� ����.
		Mesh.DetachComponent( WeaponMesh );
		WeaponMesh = None;
	}

	WeaponModifiers.Length = 0;
	WeaponClass            = None;
}

//! ��� �޽� ����.
simulated function RemoveAllMeshes()
{
	local int i;

	// ���� �޽� ����.
	for ( i = 0; i < BodyParts.Length; ++ i )
		if ( BodyParts[i].BodyMesh != None )
			Mesh.DetachComponent( BodyParts[i].BodyMesh );

	BodyParts.Length = 0;

	// ���� ������ �޽� ����.
	for ( i = 0; i < ItemParts.Length; ++ i )
		if ( ItemParts[i].ItemMesh != None )
			Mesh.DetachComponent( ItemParts[i].ItemMesh );

	ItemParts.Length = 0;

	// ��� ������ �޽� ����.
	if ( HelmetComp != None )
	{
		Mesh.DetachComponent( HelmetComp );
		HelmetComp = None;
	}

	HelmetMeshName = "";
	HelmetSkinName = "";

	// ��� ������ �޽� ����.
	for ( i = 0; i < HelmetAccessory.Length; ++ i )
		if ( HelmetAccessory[i].ItemMesh != None )
			Mesh.DetachComponent( HelmetAccessory[i].ItemMesh );

	HelmetAccessory.Length = 0;

	// �� ����.
	if ( HeadComp != None )
	{
		Mesh.DetachComponent( HeadComp );
		HeadComp = None;
	}
	if ( EyeBallComp != None )
	{
		Mesh.DetachComponent( EyeBallComp );
		EyeBallComp = None;
	}

	HeadMeshName    = "";
	HeadMTSName     = "";
	EyeBallMeshName = "";

	// ���� �޽��� ����.
	RemoveWeaponMesh();
}

/*! @brief �Ӹ������� �̿��Ͽ� �޽��� �����Ѵ�.
	@note
		CollectCharacterMesh�Լ��� ����Ǿ� MeshName���� �־�� �Ѵ�.
*/
function ApplyHeadMesh()
{
	local MorphTargetSet TempMTS;

	if ( HeadMeshName != "" )
	{
		if ( HeadComp == None )
		{
			// Set Head 
			HeadComp = CreateSkeletalMesh( HeadMeshName );
			
			// Set MorphTargetSet(�� ������)
			if ( HeadMTSName != "" )
			{
				TempMTS = MorphTargetSet( DynamicLoadObject( HeadMTSName, class'MorphTargetSet' ) );	
				HeadComp.MorphSets.length = 1;
				HeadComp.MorphSets[0] = TempMTS;
			}

			HeadComp.SetAnimTreeTemplate( AnimTree(DynamicLoadObject( "'avaCharCommon.Head'", class'AnimTree')) );
			HeadComp.SetParentAnimComponent( Mesh );
			HeadComp.SetLightEnvironment( LightEnvironment );
			Mesh.AttachComponent( HeadComp, 'Bip01' );

			MN_CloseEye = MorphNodeWeight( HeadComp.FindMorphNode('CloseEye') );
			if ( MN_CloseEye != None )
				OpenEyes();

			// Set EyeBall(������)
			if ( EyeBallMeshName != "" )
			{
				if ( EyeBallComp == None )
				{
					EyeBallComp = CreateSkeletalMesh( EyeBallMeshName );
					EyeBallComp.SetAnimTreeTemplate( AnimTree(DynamicLoadObject( "'avaCharCommon.EyeBall'", class'AnimTree')) );
					EyeBallComp.SetLightEnvironment( LightEnvironment );

					Mesh.AttachComponentToSocket( EyeBallComp, 'eye' );
				}
			}
		}
	}
}

/*! @brief ��������� �̿��Ͽ� �޽��� �����Ѵ�.
	@note
		CollectCharacterMesh�Լ��� ����Ǿ� MeshName���� �־�� �Ѵ�.
*/
function ApplyHelmetMesh()
{
	local int i;

	if ( HelmetMeshName != "" )
	{
		if( HelmetComp == None )
		{
			HelmetComp = CreateSkeletalMesh( HelmetMeshName );
			if ( HelmetComp == None)
				return ;

			HelmetComp.SetLightEnvironment( LightEnvironment );

			// H1 ���Ͽ� �ٿ��ش�.
			Mesh.AttachComponentToSocket( HelmetComp, 'H1' );
		}
	}

	for ( i = 0; i < HelmetAccessory.Length; ++i)
	{
		if( HelmetAccessory[i].MeshName != "" )
		{
			if ( HelmetComp.GetSocketByName( HelmetAccessory[i].SocketName ) == None )
			{
				`warn( "avaUICharacter.ApplyHelmet Cannot Find Socket in" @HelmetAccessory[i].SocketName @", HelmetMeshname:" @HelmetMeshName );
				continue;
			}

			if( HelmetAccessory[i].ItemMesh == None)
			{
				HelmetAccessory[i].ItemMesh = CreateStaticMesh( HelmetAccessory[i].MeshName );
				HelmetAccessory[i].ItemMesh.SetLightEnvironment( LightEnvironment );

				// ����� �ڽ����� �ش� ���Ͽ� �ٿ��ش�.
				HelmetComp.AttachComponentToSocket( HelmetAccessory[i].ItemMesh, HelmetAccessory[i].SocketName );
			}
		}
	}
}

/*! @brief ���������� �̿��Ͽ� �޽��� �����Ѵ�.
	@note
		CollectCharacterMesh�Լ��� ����Ǿ� MeshName���� �־�� �Ѵ�.
*/
function ApplyBodyMesh()
{
	local int			i;
	local SkeletalMesh	TempSkelMesh;
	local StaticMesh	TempStaticMesh;

	// Set Body Part
	for ( i = 0 ; i < BodyParts.Length ; ++ i )
	{
		`log("ApplyBodyMesh : BodyPart" @i @BodyParts[i].MeshName @BodyParts[i].Slot);

		if ( BodyParts[i].MeshName != "" )
		{	
			// Create the Body mesh if needed
			// (���� �߰��Ǿ� �̸� ������ �ִ� ��쿡 ���ؼ� �����Ѵ�)
			if ( BodyParts[i].BodyMesh == None )
			{
				TempSkelMesh = SkeletalMesh( DynamicLoadObject(BodyParts[i].MeshName, class'SkeletalMesh' ) );
				if ( TempSkelMesh != None )
				{
					BodyParts[i].BodyMesh = new(outer) class'SkeletalMeshComponent';
					BodyParts[i].BodyMesh.bUseAsOccluder = FALSE;
					BodyParts[i].BodyMesh.SetSkeletalMesh( TempSkelMesh );
				}
				else
				{
					`warn( "avaCharacter.ChangeMesh Cannot Load SkeletalMesh" @BodyParts[i].MeshName );
				}
			}

			// Apply the Body Part
			if ( BodyParts[i].BodyMesh != None )
			{
				`log( "avaUICharacter.ApplyBodyMesh" @LightEnvironment );
				BodyParts[i].BodyMesh.SetShadowParent( Mesh );
				BodyParts[i].BodyMesh.SetParentAnimComponent( Mesh );
				BodyParts[i].BodyMesh.SetLightEnvironment( LightEnvironment );
				BodyParts[i].BodyMesh.SetOwnerNoSee( true );
				BodyParts[i].BodyMesh.CastShadow  = true;
				// �����ָ� ù �����ӿ��� �ִϸ��̼��� ������ �ȵǴ���.(2007/05/14)
				BodyParts[i].BodyMesh.bUpdateSkelWhenNotRendered = true;

				if ( BodyParts[i].Slot == CHAR_SLOT_MARK )
				{
					Assert( DecalParentMesh == None );
					DecalParentMesh = BodyParts[i].BodyMesh;
				}
			}

			// add it to the components array if it's not there already
			// Mesh �� ���Ͽ� AttachComponent �� �ϴ� ������ ChangeVisibility �ÿ� �ڵ����� ���ֱ� ���ؼ��̴�.
//			if ( !BodyParts[i].BodyMesh.bAttached )
				Mesh.AttachComponent( BodyParts[i].BodyMesh,'Bip01' );
		}
	}

	// Set Item Part
	for ( i = 0 ; i < ItemParts.Length ; ++ i )
	{
		if ( ItemParts[i].MeshName != "" )
		{
			// Create the Item Mesh if needed
			// (���� �߰��Ǿ� �̸� ������ �ִ� ��쿡 ���ؼ� �����Ѵ�)
			if ( ItemParts[i].ItemMesh == None )
			{
				TempStaticMesh = StaticMesh( DynamicLoadObject(ItemParts[i].MeshName, class'StaticMesh') );

				if ( TempStaticMesh != None )
				{
					ItemParts[i].ItemMesh = new(outer) class'StaticMeshComponent';
					ItemParts[i].ItemMesh.bUseAsOccluder = FALSE;
					ItemParts[i].ItemMesh.SetStaticMesh( TempStaticMesh );
				}
				else
				{
					`warn( "avaCharacter.AttachItems Cannot Load StaticMesh" @ItemParts[i].MeshName );
				}
			}

			// Apply Item Part
			if ( ItemParts[i].ItemMesh != None )
			{
				ItemParts[i].ItemMesh.SetShadowParent( Mesh );
				ItemParts[i].ItemMesh.SetLightEnvironment( LightEnvironment );
				ItemParts[i].ItemMesh.SetOwnerNoSee( Mesh.bOwnerNoSee );
				ItemParts[i].ItemMesh.CastShadow  = true;
			}

			// add it to the components array if it's not there already
			if ( !ItemParts[i].ItemMesh.bAttached )
				Mesh.AttachComponentToSocket( ItemParts[i].ItemMesh, ItemParts[i].SocketName );
		}
	}
}

/*! @brief ���� �޽��� �����Ѵ�.
	@note
		CollectAllModifiers()�Լ��� ����Ǿ�� ����� ����ȴ�.
*/
simulated function ApplyWeaponMesh()
{
	local Rotator						newRot;
	local class<avaWeaponAttachment>	WeaponClass3p;
	local array<AttachedItem>			WeaponItems;
	local int							i, j, k;
	local string						SkinName;
	local string						TempName;

	if( WeaponClass == None )
	{
		`log("### ApplyWeaponMesh() - (WeaponClass == None) ###");
		return ;
	}

	// 3��Ī�� ���� Ŭ����.	
	WeaponClass3p = WeaponClass.default.AttachmentClass;

	// ���� �����۸� �����ؾ� �ϴ� ��찡 �ִ�.
	if( WeaponMesh == None )
	{
		if ( WeaponClass3p.default.bMeshIsSkeletal )
		{
			TempName = WeaponClass3p.default.MeshName$"_UI";
			// UI�� �޽��� ������.
			//if ( WeaponClass3p.default.UIMeshName != "" )
			//	TempName = WeaponClass3p.default.UIMeshName
			//else
			//	TempName = WeaponClass3p.default.MeshName;

			// Mesh �� Skeletal �� ���� Carried �� ���� ���̱� ���ؼ� �̴�. ��, SocMesh �� BasicMesh �� �ݵ�� �ʿ��ϴ�...
			WeaponMesh      = CreateSkeletalMesh( TempName );		
			WeaponSocMesh   = CreateSkeletalMesh( WeaponClass3p.default.SocMeshName );		
			WeaponBasicMesh = CreateStaticMesh( WeaponClass3p.default.BasicMeshName );		
			WeaponSocMesh.bForceNotToDraw = true;

			WeaponSocMesh.SetLightEnvironment( LightEnvironment );
			WeaponSocMesh.SetShadowParent( Mesh );
			WeaponBasicMesh.SetLightEnvironment( LightEnvironment );
			WeaponBasicMesh.SetShadowParent( Mesh );

			WeaponBasicMesh.SetActorCollision( false, false );

//			`log( "WeaponMesh = "@WeaponMesh@WeaponClass3p.default.MeshName );
//			`log( "WeaponSocMesh= "@WeaponSocMesh@WeaponClass3p.default.SocMeshName );
//			`log( "WeaponBasicMesh = "@WeaponBasicMesh@WeaponClass3p.default.BasicMeshName );		
		}
		else
		{
			WeaponMesh = CreateStaticMesh( WeaponClass3p.default.MeshName );				
		}

		newRot.Roll = -16384;
		WeaponMesh.SetRotation( newRot );
		WeaponMesh.SetHidden(true);
		WeaponMesh.SetActorCollision(false,false);
		WeaponMesh.SetLightEnvironment( LightEnvironment );
		WeaponMesh.SetShadowParent( Mesh );


		// �⺻ ���⸦ ���� ���δ�.
		if ( WeaponClass3p.default.UIAttachmentBoneName != '' )
		{
			Mesh.AttachComponent( WeaponMesh, WeaponClass3p.default.UIAttachmentBoneName );
//			`log( "Attach UIWeaponMesh to "@WeaponClass3p.default.UIAttachmentBoneName );
		}
		else
		{
			Mesh.AttachComponent( WeaponMesh, WeaponClass3p.default.AttachmentBoneName );
//			`log( "Attach WeaponMesh to "@WeaponClass3p.default.AttachmentBoneName );
		}

		if ( WeaponSocMesh != None )
		{
//			`log( "Attach SocMesh to WeaponMesh"@WeaponClass3p.default.PosRootBoneName );
			SkeletalMeshComponent(WeaponMesh).AttachComponent( WeaponSocMesh, WeaponClass3p.default.PosRootBoneName );

//			`log( "Attach WeaponBasicMesh to SocMesh "@WeaponClass3p.default.SocRootBoneName );
			WeaponSocMesh.AttachComponent( WeaponBasicMesh, WeaponClass3p.default.SocRootBoneName );
		}
	}

	if ( WeaponSocMesh != None )
	{
		// ���� ���� ������ ����.
		for( i = 0; i < WeaponModifiers.Length; ++ i )
		{
			WeaponItems = WeaponModifiers[i].default.CommonAttachedItems;

			// ��Ų�� ��ü�Ѵ�.
			if ( WeaponModifiers[i].default.WeaponClass != None )
			{
				for ( j = 0; j < 3; ++ j )
				{
					if ( WeaponModifiers[i].default.WeaponClass.default.WeaponSkin[j] != "" )
					{
						SkinName = WeaponModifiers[i].default.WeaponClass.default.WeaponSkin[j];
						Mesh.SetMaterial( i, Material( DynamicLoadObject(SkinName, class'Material') ) );
					}
				}
			}

			// ���� �������� �ٿ��ش�.
			for( j = 0; j < WeaponItems.Length; ++ j )
			{
				k = WeaponItemParts.Length;
				WeaponItemParts.Length = k + 1;

				WeaponItemParts[k].MeshName   = WeaponItems[j].MeshName$"_3p";
				WeaponItemParts[k].SocketName = WeaponItems[j].PrimarySocket;
				WeaponItemParts[k].ItemMesh   = CreateStaticMesh( WeaponItemParts[k].MeshName );
				WeaponItemParts[k].Slot       = WeaponModifiers[i].default.Slot;

				WeaponItemParts[k].ItemMesh.SetLightEnvironment( LightEnvironment );
				WeaponItemParts[k].ItemMesh.SetShadowParent( Mesh );

				// SocketMesh�� ���̱�.
				WeaponSocMesh.AttachComponentToSocket( WeaponItemParts[k].ItemMesh, WeaponItemParts[k].SocketName );
			}
		}
	}
}

/*! @brief ��� �޽��� �����Ѵ�.
	@note
		CollectCharacterMesh�Լ��� ����Ǿ� MeshName���� �־�� �Ѵ�.
*/
simulated function ApplyAllMeshes()
{
	local avaNetHandler NetHandler;
	local BYTE Level,LastClass,LastTeam;
	local int  Exp,SupplyPoint,Cash,Money,idClanMark;
	ApplyHeadMesh();
	ApplyHelmetMesh();
	ApplyBodyMesh();
	ApplyWeaponMesh();
	NetHandler = class'avaNetHandler'.static.GetAvaNetHandler();
	NetHandler.GetPlayerInfo( Level, LastClass, LastTeam, Exp, SupplyPoint, Cash, Money, idClanMark );
	CreateCharacterDecal( int(TeamID) , Level, idClanMark, , , true );
}


/*! @brief Local���� Character/Weapon Modifier���� ������ ���´�.
	@param bCharacter
		ĳ���������� ��������.
	@param bWeapon
		���������� ��������.
	@note
		�����ϱ� ���ؼ��� ApplyModifiers�Լ��� ȣ��Ǿ�� �Ѵ�.
*/
simulated function CollectAllModifiersFromLocal(optional bool bCharacter = true, optional bool bWeapon = true)
{
	local int i;

	// ���� ĳ���� Ŭ�������� ���ٸ�.
	if ( CharClasses.Length == 0 )
		InitCharacterClasses();

//	`log("### CollectAllModifiersFromLocal (bCharacter:" @bCharacter @",bWeapon:" @bWeapon @")"
//		 @"(TeamID:" @TeamID @"TypeID:" @TypeID @")");

	if ( bCharacter )
	{
		// ĳ���� ������ ���ٸ� ������ ���� �� ����.
		// ChangeCharacter�� ��쿡�� �����Ǿ �� �κп� ���� �� �ִ�.
		if ( CharClass == None )
			CharClass = CharClasses[TeamID * 3 + TypeID];

		// ���� ��´�.(from avaCharacter)
		if ( CharClass.default.DefaultTeam < 0 || CharClass.default.DefaultTeam > `MAX_TEAM )
			TeamID = TEAM_EU;
		else
			TeamID = ETeamType(CharClass.default.DefaultTeam);

		// ������ ��´�(from avaPawn).
		TypeID = CharClass.default.TypeID;
		if ( TypeID < 0 || TypeID > `MAX_PLAYER_CLASS )
			TypeID = 0;

		// �ʱ�ȭ.
		Modifiers.Length = 0;

		// ������ ����Ǿ� ���� ������� ó���ȴ�.
		Modifiers = CharClass.default.DefaultModifier;
	}

	if ( bWeapon )
	{
		// ĳ���� Ŭ������ ������ �⺻ ���������� ���� ���� ����.
		if ( CharClass == None )
		{
			`log("Cannont Collect Weapon Modifiers. ( CharClass == None )");
			return ;
		}

		// ĳ���Ϳ� �⺻���� ������ ���⸦ ����.
		if ( CharClass.default.DefaultWeapons.Length > 0 )
		{
			WeaponClass = CharClass.default.DefaultWeapons[0];

			// avaWeapon���� class<avaModifier>�ڷ����� �����迭 ����...
			WeaponModifiers.Length = 0;
			for ( i = 0; i < WeaponClass.default.DefaultModifiers.Length; ++ i )
			{
				if ( WeaponClass.default.DefaultModifiers[i] == None )
					continue;

				WeaponModifiers.AddItem( class<avaMod_Weapon>(WeaponClass.default.DefaultModifiers[i]) );
			}
		}
	}
}

/*! @brief Server���� ���ŵ� PlayerModifierInfo(PMI)���� Character/Weapon Modifier���� ������ ���´�.
	@param bCharacter
		ĳ���������� ��������.
	@param bWeapon
		���������� ��������.
	@note
		�����ϱ� ���ؼ��� ApplyModifiers�Լ��� ȣ��Ǿ�� �Ѵ�.
*/
simulated function CollectAllModifiersFromServer(optional bool bCharacter = true, optional bool bWeapon = true)
{
	local avaPlayerModifierInfo		avaPMI;
	local int						index;
	local int						i;

	// ���� ĳ���� Ŭ�������� ���ٸ�.
	if ( CharClasses.Length == 0 )
		InitCharacterClasses();

	// �������� ���� PMI������ ��´�.
	avaPMI = GetAvaPlayerModifierInfo();
	if ( avaPMI == None )
	{
		`log("CollectAllModifiersFromServer - (avaPMI == None)");
		return ;
	}

//	`log("### CollectAllModifiersFromServer (bCharacter:" @bCharacter @",bWeapon:" @bWeapon @")"
//		 @"(TeamID:" @TeamID @"TypeID:" @TypeID @")");

	if ( bCharacter )
	{
		// �ʱ�ȭ.
		Modifiers.Length = 0;

		// ���� �������� ĳ���� Ŭ������ ��´�.
		CharClass = CharClasses[TeamID * 3 + TypeID];

		// ���⸸ �����̶�� ����.
		// ���� PMI�� Modifiers�� ���´�.

		for ( i = 0 ; i < avaPMI.ClassTypeInfos[TypeID].CharMod.length ; ++ i )
		{
			index = Modifiers.length;
			Modifiers.length = index + 1;
			Modifiers[index] = avaPMI.ClassTypeInfos[TypeID].CharMod[i];
		}

		for ( i = 0 ; i < BaseModifierName.length ; ++ i )
		{
			index = Modifiers.length;
			Modifiers.length = index + 1;
			Modifiers[index] = class<avaCharacterModifier>(DynamicLoadObject(BaseModifierName[i],class'class'));
		}
	}

	if ( bWeapon )
	{
		// �������� ���� ���� [0]���� ���⸦ �ϴ� �⺻���� ����.
		// ([0]���� Primary Weapon�� �ȴٰ� �Ѵ�)
		if( avaPMI.ClassTypeInfos[TypeID].WeaponInfos.Length > 0 )
		{
			WeaponClass     = avaPMI.ClassTypeInfos[TypeID].WeaponInfos[0].Class;
			WeaponModifiers = avaPMI.ClassTypeInfos[TypeID].WeaponInfos[0].Mod;
		}
	}
}

/*! @brief Server or Local���� Character/Weapon Modifier���� ������ ���´�.
	@param bServer
		�������� ���� �������� �����Ѵ�.
	@param bCharacter
		ĳ���������� ��������.
	@param bWeapon
		���������� ��������.
	@note
		�����ϱ� ���ؼ��� ApplyModifiers�Լ��� ȣ��Ǿ�� �Ѵ�.
*/
simulated function CollectAllModifiers(optional bool bServer = false, 
									   optional bool bCharacter = true, 
									   optional bool bWeapon = true)
{
	if ( bServer )
		CollectAllModifiersFromServer(bCharacter, bWeapon);
	else
		CollectAllModifiersFromLocal(bCharacter, bWeapon);
}

/*! @brief Character Modifier���� ĳ������ �޽� ������ ��´�.
	@note
		�����ϱ� ���ؼ��� ApplyCharacterModifier�Լ��� ȣ��Ǿ�� �Ѵ�.
	@remark
		ĳ���� CharClass��ü�� None�� �ƴϿ��� �Ѵ�.
*/
simulated function ApplyAllModifiers(optional bool bServer = false)
{
	local int i;

	// ��� �޽��� �����.
	RemoveAllMeshes();


	// ��� Modifiers������ ���´�.
	CollectAllModifiers(bServer);

	// Modifiers���� �޽������� ���´�.
	for ( i = 0; i < Modifiers.Length; ++ i )
		CollectCharacterMesh( Modifiers[i] );

	// Set Base Skeletal Mesh(�� or ������ �ٲ� ������ ������ �ٽ� �Ǿ�� �Ѵ�)
	Mesh.SetSkeletalMesh( CharClass.default.BaseSkeletalMesh );


	// ��� �޽� ������ �����Ѵ�.
	ApplyAllMeshes();

	// ���� or ���⿡ ���� ���� �ʱ�ȭ.
	InitAnim();
}


//------------------------------------------------------------- ----------------
//	���� �Լ�.
//-----------------------------------------------------------------------------

//! ������ ������ ���ο� ĳ����/���⸦ ��ü�Ѵ�.
simulated function ChangeCharacter( class<avaCharacter> NewCharClass )
{
	if ( NewCharClass != None )
		CharClass = NewCharClass;

	ApplyAllModifiers();
}


/*! @brief ĳ���� ��� ��ü�Ѵ�.
	@param ���� ����� Modifier�� �־��ش�.
*/
simulated function ChangeCharacterItem( class<avaCharacterModifier> Mod )
{
	local int i;
	local avaNetHandler		NetHandler;
	local BYTE Level,LastClass,LastTeam;
	local int  Exp,SupplyPoint,Cash,Money,idClanMark;
	
	if ( Mod == None )
		return ;

	// Modifier������ ���ٸ� ���´�.
	if ( Modifiers.Length == 0 )
		CollectAllModifiersFromLocal();

	for ( i = 0; i < Modifiers.Length; ++ i )
	{
		// ���� 1���� 1���� Modifier�� ���ٰ� �Ѵ�.
		if ( Modifiers[i].default.Slot == Mod.default.Slot )
			break;
	}

	if( i == Modifiers.Length )
	{
		// 1. ���ο� Modifier�� �߰�.
		Modifiers.AddItem( Mod );

//		`log("ChangeCharacterItem : Add" @Mod.Name);
	}
	else
	{
//		`log("ChangeCharacterItem : Remove" @Modifiers[i].Name);

		// 1. ���� ���Կ� �ش��ϴ� �޽� ����(BodyPart or ItemPart, Mesh.DetachComponent).
		// (�ش� ������ ����ϴ� �޽��� ��� ����)
		RemoveCharacterMesh( Modifiers[i].default.Slot );

		// ���� Modifier�� ��ü.
		Modifiers[i] = Mod;

//		`log("ChangeCharacterItem : Swap" @Modifiers[i].Name);
	}

	// 2. ���ο� Modifier�� ����Ʈ(Midifiers)�� �߰�.
	// (MeshName�� ������ �ش�)
	CollectCharacterMesh( Mod );

	// 3. �ش� �޽� ���� ����.
	// (���ŵ� MeshName�� ���ؼ� (MeshComponent == None)�� ��츸 �����Ѵ�)
	ApplyHeadMesh();
	ApplyHelmetMesh();
	ApplyBodyMesh();

	NetHandler = class'avaNetHandler'.static.GetAvaNetHandler();
	NetHandler.GetPlayerInfo( Level, LastClass, LastTeam, Exp, SupplyPoint, Cash, Money, idClanMark );
	CreateCharacterDecal( int(TeamID) , Level, idClanMark, , , true );

	InitializeSkelControl();

	// ���� or ���⿡ ���� ���� �ʱ�ȭ.
//	InitAnim();
}


/*! @brief ���⸦ ��ü�Ѵ�.
	@param Mod
		���� ����� Modifier�� �־��ش�.
*/
simulated function ChangeWeapon( class<avaMod_Weapon> Mod )
{
	local int i;

	// ���� �����ϰ�
	RemoveWeaponMesh();


	if ( Mod == None )
		return ;

	// �ֹ��� Modifier���� ���� Ŭ������ ��.
	WeaponClass = Mod.default.WeaponClass;

	// avaWeapon���� class<avaModifier>�ڷ����� �����迭 ����...
	// (���� Modifier���� �����Ѵ�)
	WeaponModifiers.Length = 0;
	for ( i = 0; i < WeaponClass.default.DefaultModifiers.Length; ++ i )
	{
		if ( WeaponClass.default.DefaultModifiers[i] == None )
			continue;

		WeaponModifiers.AddItem( class<avaMod_Weapon>(WeaponClass.default.DefaultModifiers[i]) );
	}

	// ���� �����ϰ�
	ApplyWeaponMesh();

	// ���� or ���⿡ ���� ���� �ʱ�ȭ.
	InitAnim();
}


/*! @brief �κ��丮 ����� ��ü�Ѵ�.
	@param SN
		Item Serial Number�� 0�� �ƴ� ���� �ԷµǸ� ������ ���⿡�� ���� �������� ���´�.
*/
simulated function ChangeInventoryWeapon( string ItemSN )
{
	// ���� �����ϰ�
	RemoveWeaponMesh();


	// �����ϰ� �ִ� ���� �������� ���´�.
	WeaponModifiers.Length = 0;
	WeaponModifiers = class'avaNetHandler'.static.GetAvaNetHandler().GetWeaponModifiers( ItemSN );

	// �ֹ��⸦ ������, ����Ʈ���� �����Ѵ�.
	WeaponClass = WeaponModifiers[0].default.WeaponClass;
	WeaponModifiers.Remove( 0, 1 );


	// ���� �����ϰ�
	ApplyWeaponMesh();

	// ���� or ���⿡ ���� ���� �ʱ�ȭ.
	InitAnim();
}


/*! @brief ���⸦ ��ü�Ѵ�.
	@param ���� ����� Modifier�� �־��ش�.
*/
simulated function ChangeWeaponItem( class<avaMod_Weapon> Mod )
{
	local int i;

	if ( Mod == None )
		return ;

	// ( Mod.default.Slot == WEAPON_SLOT_None )�� ���� �����Ͱ� �Էµ��� ���� ����̴�.
//	`log("### ChangeWeaponItem :" @Mod @" ### SlotType:" @Mod.default.Slot);

	// ���� ���� ����.
	for ( i = 0; i < WeaponItemParts.Length; ++i)
		WeaponSocMesh.DetachComponent( WeaponItemParts[i].ItemMesh );
	WeaponItemParts.Length = 0;


	// �ش� ���� �������� �̹� ���� �Ǿ����� ���� ���������� �˻��Ѵ�.
	for ( i = 0; i < WeaponModifiers.Length; ++ i )
	{
		if ( WeaponModifiers[i].default.Slot == Mod.default.Slot )
			break;
	}

	// ���� ��쿡�� �߰�, �ִٸ� ��ü.
	if ( i == WeaponModifiers.Length )
		WeaponModifiers.AddItem( Mod );
	else
		WeaponModifiers[i] = Mod;

//	for ( i = 0; i < WeaponModifiers.Length; ++i )
//		`log("ApplyWeapon - WeaponModifiers[" @i @"] = " @WeaponModifiers[i]);

	// ���� �����ϰ�
	ApplyWeaponMesh();

	// ���� or ���⿡ ���� ���� �ʱ�ȭ.
//	InitAnim();
}

/*! @brief �������� �����ϰ� ĳ���͸� ���� �����Ѵ�.
	@note
		Local������ ����.
*/
simulated function ChangeTeam(ETeamType Team)
{
	if ( Team != TEAM_EU && Team != TEAM_USSR )
		Team = TEAM_EU;

	TeamID = Team;

	// None�� ��� TeamID, TypeID���� �̿��ؼ� ĳ���Ͱ� �����ȴ�.
	CharClass = None;

	ApplyAllModifiers();
}

/*! @brief ������ �����ϰ� ĳ���͸� ���� �����Ѵ�.
	@note
		Local������ ����.
*/
simulated function ChangeClassType(EPlayerClassType Type)
{
	// ������ ����� PointMan�� �⺻������.
	if( Type < 0 || Type > `MAX_PLAYER_CLASS )
		Type = PCT_PointMan;

	TypeID = Type;

	// None�� ��� TeamID, TypeID���� �̿��ؼ� ĳ���Ͱ� �����ȴ�.
	CharClass = None;

	ApplyAllModifiers();
}

//! �������� ���� PMI������ ĳ���ͷ� ��ü�Ѵ�.
simulated function ChangeServerCharacter()
{
	// �������� ���� ���� ������ �ٽ� ���´�.
	bUpdateTeamClass = true;

	// ���������� ��(TeamID)�� ����(TypeID)������ ������ �Լ�.
//	GetAvaPlayerModifierInfo();

	ApplyAllModifiers(true);
}

/*! @brief (�������� ���� ������) ���⸦ ��ü�Ѵ�.
*/
simulated function ChangeServerWeapon()
{
	// ���� �����ϰ�
	RemoveWeaponMesh();

	// �������� ���� ������ ���� ����(WeaponClass, WeaponModifiers�� �����Ѵ�).
	CollectAllModifiersFromServer(false, true);

	// ���� �����ϰ�
	ApplyWeaponMesh();

	// ���� or ���⿡ ���� ���� �ʱ�ȭ.
	InitAnim();
}

//! �������� �޾ƿ� ������ ���� ������ ���ؼ� �⺻ ĳ���ͷ� �����ش�.
simulated function ChangeServerTeamClass()
{
	local int i;

	// ��� �޽��� �����.
	RemoveAllMeshes();


	// �������� ���� ���� ������ �ٽ� ���´�.
	bUpdateTeamClass = true;

	// ĳ���� ������ ����, ���� ������ ���ÿ��� ���´�.
	CollectAllModifiersFromServer(true, false);
	CollectAllModifiersFromLocal(false, true);

	// Modifiers���� �޽������� ���´�.
	for ( i = 0; i < Modifiers.Length; ++ i )
		CollectCharacterMesh( Modifiers[i] );

	// Set Base Skeletal Mesh(�� or ������ �ٲ� ������ ������ �ٽ� �Ǿ�� �Ѵ�)
	Mesh.SetSkeletalMesh( CharClass.default.BaseSkeletalMesh );


	// ��� �޽� ������ �����Ѵ�.
	ApplyAllMeshes();

	// ���� or ���⿡ ���� ���� �ʱ�ȭ.
	InitAnim();
}


//------------------------------------------------------------- ----------------
//	Misc Function
//-----------------------------------------------------------------------------

/*! @brief �������� ���� PMI������ ����ִ� �Լ�.
	@return
		������ ������ �Ǿ� �ִ� ��찡 �ƴϸ� None�� ���ϵȴ�.
	@note
		������ ����� ������ 1ȸ�� ���ؼ� TeamID, TypeID�� ������ �ش�.
	@remark
		bUpdateTeamClass���� false�� ��쿡�� ���ŵȴ�.
		(�ٽ� �����ϰ� �ʹٸ� false�� �������ָ� �ȴ�)
*/
function avaPlayerModifierInfo GetAvaPlayerModifierInfo()
{
	local PlayerController			PC;
	local avaPlayerController		avaPC;
	local avaPlayerReplicationInfo	avaPRI;
	local avaPlayerModifierInfo		avaPMI;
	local ETeamType					oldTeamType;

//	`log("### GetAvaPlayerModifierInfo() ###");

	// LocalPlayerController�� ���´�(2007/01/31 ����).
	foreach WorldInfo.LocalPlayerControllers(PC)
	{
		avaPC = avaPlayerController(PC);
		break;
	}

	// PRI�� ����ȯ�ؼ� ava������ �� PMI�� ��´�.
	avaPRI = avaPlayerReplicationInfo( avaPC.PlayerReplicationInfo );
	if ( avaPRI == None )
	{
		`log("Failed GetAvaPlayerModifierInfo() - (avaPRI == None) : " @avaPRI);
		return None;
	}

	// ������ 1���� ���ؼ� �������� ���������� ����� ���� ������ ���´�.
	if ( avaPRI.bUpdate == true && bUpdateTeamClass == true )
	{
//		TeamID = ETeamType(avaPRI.LastTeam);
//		TypeID = avaPRI.LastClass;

		oldTeamType = TeamID;

		TeamID = ETeamType(int( class'avaNetHandler'.static.GetAvaNetHandler().GetMyURLString("Team") ));
		TypeID = int( class'avaNetHandler'.static.GetAvaNetHandler().GetMyURLString("LastClass") );

//		`log("bUpdateTeamClass : " @TeamID @". " @TypeID );

		// [2]�� ����ϴ� Spectator�� �־ ������ ó���� �ش�.
		if ( TeamID != Team_EU && TeamID != Team_USSR )
		{
			if ( oldTeamType != Team_EU && oldTeamType != Team_USSR )
				TeamID = Team_EU;
			else
				TeamID = oldTeamType;
		}

		// ���̻� �� �κ��� ó������ �ʵ��� false�� ����.
		bUpdateTeamClass = false;
	}

	avaPMI = avaPRI.avaPMI;
	if ( avaPMI == None )
	{
		`log("GetAvaPlayerModifierInfo() - (avaPMI == None) : " @avaPRI.avaPMI );
		return None;
	}

	// ���� Modifiers�� �ִٸ�, �������� ���������� ������ �����.
	if ( avaPMI.ClassTypeInfos[TypeID].CharMod.length > 0 )
		return avaPMI;

//	`log("GetAvaPlayerModifierInfo() - avaPMI.ClassTypeInfos[TypeID].CharMod.length : " 
//		  @avaPMI.ClassTypeInfos[TypeID].CharMod.length );

	// �������� ���� ������ ����.
	return None;
}

//! ��, ������ ĳ���� �����͸� �̸� ������ �д�.
function InitCharacterClasses()
{
	// �̸� 6���� ĳ���� ��, ������ ���ø��� ������ �д�.
	// team_id * 3(`MAX_PLAYER_CLASS) + class_type_id
	CharClasses.AddItem( class<avaCharacter>(DynamicLoadObject( "avaRules.avaCharacter_EUPointMan", class'class' )) );
	CharClasses.AddItem( class<avaCharacter>(DynamicLoadObject( "avaRules.avaCharacter_EURifleMan", class'class' )) );
	CharClasses.AddItem( class<avaCharacter>(DynamicLoadObject( "avaRules.avaCharacter_EUSniper", class'class' )) );
	CharClasses.AddItem( class<avaCharacter>(DynamicLoadObject( "avaRules.avaCharacter_USSRPointMan", class'class' )) );
	CharClasses.AddItem( class<avaCharacter>(DynamicLoadObject( "avaRules.avaCharacter_USSRRifleMan", class'class' )) );
	CharClasses.AddItem( class<avaCharacter>(DynamicLoadObject( "avaRules.avaCharacter_USSRSniper", class'class' )) );

}

/*! @biref ���⿡ ���� ĳ���� ������ �ٲ�� ���ִ� �Լ�.
	@note
		[UavaAnimNodes.cpp]
		void UavaAnimBlendByWeaponType::InitAnimSequence(FName NewPrefix)�Լ����� ȣ���.
		(�̰Ŷ��� ��ð��� �����Ѱ�..�Ф�)
	@original avaPawn.uc
		3��Ī �� Animation �� ���� Weapon Type �� Return �� �ش�. 2006/1/27 oz
	@var ����.
		���� ������ RebuildAnimSeqName�Լ��� ���Ͽ� �ʿ����(avaCharacter������ �ʿ��ϴ�).
*/
//simulated event function EBlendWeaponType GetWeaponAttachmentType()
//{
//	if ( WeaponClass != None )
//		return WeaponClass.default.AttachmentClass.default.AttachmentWeaponType;
//
//	return WBT_None;
//}

simulated function InitializeSkelControl()
{
    InitializeTwist();	

	LookAtControl = SkelControlLookAt(mesh.FindSkelControl('HeadController'));
	LeftEyeControl = SkelControlLookAt(EyeBallComp.FindSkelControl('LeftEyeController'));
	RightEyeControl = SkelControlLookAt(EyeBallComp.FindSkelControl('RightEyeController'));

	// ���� ������ LookAt �缳��.
	UICharacter_LookAtEnable( bUISceneLookAtEnable );
}


/*! @brief �÷��� �Ǵ� AnimNodeSequence��带 ã�Ƽ� ���� �ð��� ������ �ش�.
	@param NodeName
		Root�� �Ǵ� ����̸��̴�.
*/
simulated function SaveAnimSeqTime(name AnimNodeName)
{
	local avaAnimBlendBase			RootAnimBlend;
	local array<AnimNodeSequence>	AnimSeqs;
	local int						i, j;
	local float						DeltaTime;

	// �ش� ��带 ã�´�.
	RootAnimBlend = avaAnimBlendBase( Mesh.Animations.FindAnimNode( AnimNodeName ) );
	if ( RootAnimBlend == None )
	{
		`log("### SaveAnimSeqTime - Cannot found " @AnimNodeName);
		return ;
	}

	for ( i = 0; i < RootAnimBlend.Children.Length; ++ i )
	{
		// GetAnimSeqNodes�Լ����� �����Ǿ� ���ͼ� 0���� �ʱ�ȭ.
		// (��� �ڽĵ��� �Ѳ����� �����ؼ� ������ �� ������ UAnimNode::GetAnimSeqNodes����
		//  Reserve�ϴ� ������ �ڽĳ����� ������ �ϱ� ������ �Ҿȿ�Ҹ� ������ �ִ�)
		AnimSeqs.Length = 0;

		// �ڽ� ����� ��� Animation Node Sequence�� ���´�.
		RootAnimBlend.GetAnimSeqNodes(i, AnimSeqs);

		// �� Sequence�� �̸��� ���� ���⿡ �°� ������ ������ �ش�.
		for ( j = 0; j < AnimSeqs.Length; ++ j )
		{
			DeltaTime = AnimSeqs[j].CurrentTime - AnimSeqs[j].PreviousTime;

			// ���� �ִϸ��̼� ���̿��� �ð��� ��ȭ�� �ִ� ����� ���.
			if ( DeltaTime > 0.0 )
			{
				// ���� ������� �����...
				LastAnimSuffix   = Right(AnimSeqs[j].AnimSeqName, Len(AnimSeqs[j].AnimSeqName) - InStr(AnimSeqs[j].AnimSeqName, "_"));
				LastAnimTime     = AnimSeqs[j].CurrentTime;
				bSaveAnimSeqTime = true;

//				`log("Saved AnimSeq:" @AnimSeqs[j].AnimSeqName @"(Suffix:"@LastAnimSuffix@")" @" Time: " @AnimSeqs[j].CurrentTime);
				return;
			}
		}
	}
}

/*! @brief �ڽĳ�带 ã�Ƽ� ��� �ִϸ��̼� �̸����� ���ξ ��ü�� �ش�.
	@param NodeName
		Root�� �Ǵ� ����̸��̴�.
	@param WeaponAnimPrifix
		��ü�� ���ξ��̴�.
	@param bLastWeaponAnimPrefix
		WeaponAnimPrifix�� �̸��� ���ʿ� ��ġ�ϴ� ��� ���.(Channel������ �߰���)
	@note
		avaAnimBlendByWeaponType����� �Լ��� ���Դ�.<br><br>

		1. OldAnimName("G36_Ready_Idle"), AnimPrefix("M24")�� ���� ���.
		2. AnimSurfix("_Ready_Idle") = OldAnimName("G36_Ready_Idle")���� '_'�ڿ��κ� ���.
		3. AnimName("M24_Ready_Idle") = AnimPrefix("M24") + AnimSuffix("_Ready_Idle")
	@par ����
		avaAnimBlendBaseŬ������ ����Լ��� �ִ� ���� �����ҵ�...
*/
simulated function RebuildAnimSeqName(name AnimNodeName, string WeaponAnimPrefix, optional bool bLastWeaponAnimPrefix = false)
{
	local avaAnimBlendBase			RootAnimBlend;
	local array<AnimNodeSequence>	AnimSeqs;
	local AnimNodeSequence			LastAnimSeq;
	local string					AnimSuffix;
	local string					AnimPrefix;
	local name						AnimName;
	local int						i, j;

	// �ش� ��带 ã�´�.
	RootAnimBlend = avaAnimBlendBase( Mesh.Animations.FindAnimNode( AnimNodeName ) );
	if ( RootAnimBlend == None )
	{
		`log("### RebuildAnimSeqName - Cannot found " @AnimNodeName);
		return ;
	}

	for ( i = 0; i < RootAnimBlend.Children.Length; ++ i )
	{
		// GetAnimSeqNodes�Լ����� �����Ǿ� ���ͼ� 0���� �ʱ�ȭ.
		// (��� �ڽĵ��� �Ѳ����� �����ؼ� ������ �� ������ UAnimNode::GetAnimSeqNodes����
		//  Reserve�ϴ� ������ �ڽĳ����� ������ �ϱ� ������ �Ҿȿ�Ҹ� ������ �ִ�)
		AnimSeqs.Length = 0;

		// �ڽ� ����� ��� Animation Node Sequence�� ���´�.
		RootAnimBlend.GetAnimSeqNodes(i, AnimSeqs);

		// �� Sequence�� �̸��� ���� ���⿡ �°� ������ ������ �ش�.
		for ( j = 0; j < AnimSeqs.Length; ++ j )
		{
			// ������ WeaponAnimPrefix�� �ڿ� ���̴� ���.
			if ( bLastWeaponAnimPrefix )
			{
				// ù��° '_'�� ã�Ƽ� ���ξ� �κ��� �����´�.
				AnimPrefix = Left(AnimSeqs[j].AnimSeqName, InStr(AnimSeqs[j].AnimSeqName,"_",true) + 1);

				// ���ο� ("�⺻ �̸�" + "���� �̸�") ���� ������ �ش�.
				AnimName = name(AnimPrefix $ WeaponAnimPrefix);
			}
			else
			{
				// ù��° '_'�� ã�Ƽ� ���̾� �κ��� �����´�.
				AnimSuffix = Right(AnimSeqs[j].AnimSeqName, Len(AnimSeqs[j].AnimSeqName) - InStr(AnimSeqs[j].AnimSeqName,"_"));

				// ���������� ����� ��带 ã�´�.
				if ( LastAnimSuffix == AnimSuffix )
					LastAnimSeq = AnimSeqs[j];

				// ���ο� ("���� �̸�" + "�����̸�") ���� ������ �ش�.
				AnimName = name(WeaponAnimPrefix $ AnimSuffix);
			}

			// �ش� ���⿡ ���� �̸����� �缳�� ���ش�.
			if ( Mesh.FindAnimSequence( AnimName ) != None )
			{
				AnimSeqs[j].SetAnim( AnimName );

//				if ( bLastWeaponAnimPrefix )
//					`log("Child[" @i @"] AnimSequence Changed " 
//						 @AnimSeqs[j].AnimSeqName @" to " @AnimName @" ["@AnimPrefix@"+"@WeaponAnimPrefix@"]");
//				else
//					`log("Child[" @i @"] AnimSequence Changed " 
//						 @AnimSeqs[j].AnimSeqName @" to " @AnimName @" ["@WeaponAnimPrefix@"+"@AnimSuffix@"]");
			}
			else
				`log("Cannnot found AnimName - " @AnimName);
		}
	}

	// ����� ���� �ִٸ�, �÷��� �ð��� ������ �ش�.
	if ( bSaveAnimSeqTime )
	{
		LastAnimSeq.PlayAnim(false, 1.0f, LastAnimTime);

//		`log("Restore Last Animation Time - AnimNodeName = " @LastAnimSeq.AnimSeqName @", Time:" @LastAnimTime);

		// 1ȸ�� ����ȴ�.
		bSaveAnimSeqTime = false;
	}
}

/*! @brief ĳ������ �ִϸ��̼�(+����) �κ��� �ʱ�ȭ ���ش�.
*/
simulated function InitAnim( optional EUICharAnimTree NewAnimTreeType = UICA_None )
{
	local bool bUpdate;

	bUpdate = (NewAnimTreeType == UICA_None) ? true : false;

	// ������ �ִϸ��̼��� ���� ���⺰ �ִϸ��̼��� ��츦 ������.
	if ( NewAnimTreeType == UICA_Channel || (AnimTreeType == UICA_Channel && bUpdate) )
	{
//		`log("####### Channel InitAnim(" @NewAnimTreeType @") ######## Old =" @AnimTreeType);

		// ������ �ƴ� ��쿡��.
		if ( !bUpdate )
		{
			// AnimSet�� ���� �����ǰ�, AnimTree�� �����Ǿ�� �Ѵ�.
			Mesh.AnimSets.Length = 1;
			Mesh.AnimSets[0] = AnimSet(DynamicLoadObject("'avaCharCommon.UI_Animset'", class'AnimSet'));
			Mesh.SetAnimTreeTemplate( AnimTree(DynamicLoadObject("avaCharCommon.UI_ChannelAnimTree", class'AnimTree')) );
		}

		// 'ChannelTypeNode'��带 ��������
		// �����̸��� ���߼� �ִϸ��̼� �̸��� �籸�� ���ش�.
		if ( WeaponClass != None )
			RebuildAnimSeqName( 'ChannelTypeNode', string(WeaponClass.default.AttachmentClass.default.AnimPrefix), true );

		AnimTreeType = UICA_Channel;
	}
	else if ( NewAnimTreeType == UICA_Ready || (AnimTreeType == UICA_Ready && bUpdate) )
	{
//		`log("####### Ready InitAnim(" @NewAnimTreeType @") ######## Old =" @AnimTreeType);

		// ������ ��쿡 �ִϸ��̼� �ð��� �����Ѵ�.
		SaveAnimSeqTime('ReadyTypeNode');

		// ������ �ƴ� ��쿡�� ���� ������ �ش�.
		if ( !bUpdate )
		{
			// AnimSet�� ���� �����ǰ�, AnimTree�� �����Ǿ�� �Ѵ�.
			Mesh.AnimSets.Length = 1;
			Mesh.AnimSets[0] = AnimSet(DynamicLoadObject("'avaCharCommon.UI_Animset'", class'AnimSet'));
			Mesh.SetAnimTreeTemplate( AnimTree(DynamicLoadObject("'avaCharCommon.UI_ReadyAnimTree'", class'AnimTree')) );
		}
		else
		{
			// �ִϸ��̼� �̸��� �ٲ� ������ ����� ������ ������ �ʿ����.
			bSaveAnimSeqTime = false;
		}

		// 'ReadyTypeNode'��带 ��������
		// �����̸��� ���߼� �ִϸ��̼� �̸��� �籸�� ���ش�.
		// (��, ���� AnimTree�� ���ŵ��� �ʾ��� ��쿡�� �籸�� �� �ʿ䰡 ����)
		if ( WeaponClass != None )
			RebuildAnimSeqName( 'ReadyTypeNode', string(WeaponClass.default.AttachmentClass.default.AnimPrefix) );

		AnimTreeType = UICA_Ready;
	}
	else
	{
		// �ƹ��� �ִϸ��̼ǵ� �������� �ʴ´�.
	}

	// ���� ���� ��Ʈ�ѵ��� �ʱ�ȭ(?)
	InitializeSkelControl();
}

//! Kismet���� �ʱ�ȭ�� �ʿ��ϴٰ� ��û�ϸ�...
function InitTurnAnim()
{
	local avaAnimBlendBase ReadyTypeAnimBlend;

//	`log("avaUICharacter.InitTurnAnim" @TurnAnimCount @bNeedInitTurnAnim );

	if ( TurnAnimCount == 1 )
	{
		bNeedInitTurnAnim = true;
		bPlayingTurnAnim  = false;
	}
	else if ( TurnAnimCount == 2 )
	{
		ReadyTypeAnimBlend = avaAnimBlendBase( Mesh.Animations.FindAnimNode( 'ReadyTypeNode' ) );
		if ( ReadyTypeAnimBlend != None )
			ReadyTypeAnimBlend.SetActiveChild(0, 0.25);

		// Tell mesh to stop using root motion
		Mesh.RootMotionRotationMode = RMRM_Ignore;
		Mesh.RootMotionMode         = RMM_Ignore;

		bPlayingTurnAnim = false;

		TurnAnimCount = 0;

		SetLocation(TurnAnimStartPos);
		SetRotation(TurnAnimStartRot);
	}
}

/*! @brief �ڵ��� ������ �Ѵ�.
*/
simulated function PlayTurnAnim( optional bool bInit = false )
{
	local avaAnimBlendBase ReadyTypeAnimBlend;

	// ��ġ �ʱ�ȭ�� �ʿ��� ����.
	// ���� 1�� ��쿡�� �ִϸ��̼��� �ǵ��� �Ѵ�.
	if ( bInit )
	{
		if ( TurnAnimCount != 1 )
			return ;
		else
		{
			// �ڵ��� �ִٸ� ó�� ��ġ�� ������.
			if ( TurnAnimCount != 0 ) 
			{
				ReadyTypeAnimBlend = avaAnimBlendBase( Mesh.Animations.FindAnimNode( 'ReadyTypeNode' ) );
				if ( ReadyTypeAnimBlend != None )
					ReadyTypeAnimBlend.SetActiveChild(0, 0.25);

				// Tell mesh to stop using root motion
				Mesh.RootMotionRotationMode = RMRM_Ignore;
				Mesh.RootMotionMode         = RMM_Ignore;

				bPlayingTurnAnim = false;

				// (�ѹ��� ���� ����) ��ġ�� ���� �缳��.
				TurnAnimCount = 0;

//				SetLocation(TurnAnimStartPos);
//				SetRotation(TurnAnimStartRot);
			}
			return;
		}
	}

	// �ִϸ��̼� ���̸� ����.
	if ( bPlayingTurnAnim )
	{
//		`log("avaUICharacter.PlayTurnAnim - bPlayingTurnAnim=true");
		return ;
	}

	if ( AnimTreeType != UICA_Ready )
	{
//		`log("avaUICharacter.PlayTurnAnim - AnimTreeType != UICA_Ready");
		return ;
	}

	Mesh.RootMotionRotationMode = RMRM_RotateActor;
	Mesh.RootMotionMode         = RMM_Translate;

	ReadyTypeAnimBlend = avaAnimBlendBase( Mesh.Animations.FindAnimNode( 'ReadyTypeNode' ) );
	if ( ReadyTypeAnimBlend != None )
	{
		// �ణ�� BlendTime�� ����.(�ε巴��~)
		ReadyTypeAnimBlend.SetActiveChild(1, 0.25);
	}
/*
	if ( TurnAnimCount == 0 )
	{
		TurnAnimStartPos = Location;
		TurnAnimStartRot = Rotation;
	}
*/
	TurnAnimCount++;

	bPlayingTurnAnim = true;

//	`log("### PlayTurnAnim ###" @TurnAnimCount);
}


//
simulated function UICharacter_LookAtEnable( bool bEnable, bool bForce = false )
{
	// ������ ������ �����ϴ°�?
	if ( bForce )
	{
		LookAtControl.TargetLocationInterpSpeed = 1.0f;
		LookAtControl.TargetLocation = TargetLocation;
		if ( LeftEyeControl != None )	
		{
			LeftEyeControl.TargetLocation = TargetLocation;
			LeftEyeControl.TargetLocationInterpSpeed = 1.0f;
			RightEyeControl.TargetLocation = TargetLocation; 
			RightEyeControl.TargetLocationInterpSpeed = 1.0f;
		}

//		`log("LookAtEnable(Force) - " @bEnable @LookAtControl.StrengthTarget);
		EnableLookAtController( bEnable );
	}
	else
	{
		// ��� �ִ� ���� ����, �׷��� ���� ��쿡�� ����.
		if ( bLockedLookAtEnabled )
		{
//			`log("LookAtEnable(bLockedLookAtEnabled) - " @bEnable @LookAtControl.StrengthTarget);
			EnableLookAtController(false);
			return ;
		}

		LookAtControl.TargetLocationInterpSpeed = 1.0f;
		LookAtControl.TargetLocation = TargetLocation;
		if ( LeftEyeControl != None )	
		{
			LeftEyeControl.TargetLocation = TargetLocation;
			LeftEyeControl.TargetLocationInterpSpeed = 1.0f;
			RightEyeControl.TargetLocation = TargetLocation; 
			RightEyeControl.TargetLocationInterpSpeed = 1.0f;
		}

//		`log("LookAtEnable - " @bEnable @LookAtControl.StrengthTarget);
		EnableLookAtController( bEnable );
	}
}

//! Kismet���� ������ ��.
simulated function LookAt( vector at, bool bEnable )
{
	if ( LookAtControl == None )
		return;

	if ( bEnable )
		TargetLocation = at;

	// ������ ����.
	bUISceneLookAtEnable = bEnable;

//	`log("LookAt - at:" @at @"bEnable:" @bEnable);

	UICharacter_LookAtEnable( bEnable );
}

/*
*/
simulated event OnAnimPlay(AnimNodeSequence SeqNode)
{
	local string animSuffix;

//	`log("### OnAnimPlay : " @ SeqNode.AnimSeqName @","@Name);

	animSuffix = Right(SeqNode.AnimSeqName, Len(SeqNode.AnimSeqName) - InStr(SeqNode.AnimSeqName, "_"));
	// Ư�� ���ۿ��� ī�޶� �ٶ��� �ʵ��� �ϸ�,
	// ���Ⱑ ��ü�� ���
	if ( animSuffix == "_Idle2" || animSuffix == "_BackTurn" )
	{
		bLockedLookAtEnabled = true;

		UICharacter_LookAtEnable( false, true );
	}
	else
	{
		// AnimNodeRandom�� ��� ���� �ִϸ��̼� �ð��� �ʰ��ϴ� ���
		// OnAnimEnd�� �����ϴ� ��쵵 �ִ�.
		bLockedLookAtEnabled = false;

		if ( bUISceneLookAtEnable )
			UICharacter_LookAtEnable( bUISceneLookAtEnable );
	}

	CurAnimSuffix = animSuffix;
}



/*! @brief �ڵ��� ������ ���� �Ŀ� �ٽ� Idle�������� ������ �ش�.
*/
simulated event OnAnimEnd(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
	local avaAnimBlendBase	ReadyTypeAnimBlend;
	local avaAnimNodeRandom	RandomIdleNode;
	local string			animSuffix;

	animSuffix = Right(SeqNode.AnimSeqName, Len(SeqNode.AnimSeqName) - InStr(SeqNode.AnimSeqName, "_"));
	if( animSuffix == "_BackTurn" )
	{
		ReadyTypeAnimBlend = avaAnimBlendBase( Mesh.Animations.FindAnimNode( 'ReadyTypeNode' ) );
		if ( ReadyTypeAnimBlend != None )
		{
			ReadyTypeAnimBlend.SetActiveChild(0, 0);

			RandomIdleNode = avaAnimNodeRandom( Mesh.Animations.FindAnimNode( 'RandomIdleNode' ) );
			if ( RandomIdleNode != None )
				RandomIdleNode.ResetActiveChild(0, 0);
		}

		// Tell mesh to stop using root motion
		Mesh.RootMotionRotationMode = RMRM_Ignore;
		Mesh.RootMotionMode         = RMM_Ignore;

		bPlayingTurnAnim = false;

		// (�ѹ��� ���� ����) ��ġ�� ���� �缳��.
		if ( TurnAnimCount >= 2 )
		{
			TurnAnimCount = 0;

			SetLocation(TurnAnimStartPos);
			SetRotation(TurnAnimStartRot);
		}
	}

	// ��� ���� ������ ��� �־��ٸ� Ǯ���ش�.
	bLockedLookAtEnabled = false;

	// UIScene�� LookAt���������� ���� ����.
	if ( bUISceneLookAtEnable )
		UICharacter_LookAtEnable( true );


	// �ʿ信 ���� 1�� ��...
	if ( bNeedInitTurnAnim )
	{
		bNeedInitTurnAnim = false;

//		`log("avaUICharacter.OnAnimEnd");

		PlayTurnAnim();
	}

//	`log("### OnAnimEnd : " @ SeqNode.AnimSeqName @","@Name);
}

/*! @brief �α׿��� (avaPawn��)���� �޽��� ����� ���ؼ� �߰�.
*/
simulated event PostInitAnimTree(SkeletalMeshComponent SkelComp)
{
	if (SkelComp == Mesh)
	{
	}	
}


//------------------------------------------------------------- ----------------
//	Sequence
//-----------------------------------------------------------------------------

simulated function OnUICharacterEvent( avaSeqAct_UICharacterEvent action )
{
	`log("### "@action.Event@" - "@action.StrValue@" ### >>" @Name);

	switch(action.Event)
	{
		case UICE_ChangeCharacter:
			ChangeCharacter( class<avaCharacter>(DynamicLoadObject( action.StrValue, class'class' )) );
			break;
		case UICE_ChangeCharacterMod:
			ChangeCharacterItem( class<avaCharacterModifier>(DynamicLoadObject( action.StrValue, class'class' )) );
			break;
		case UICE_ChangeWeapon:
			ChangeWeapon( class<avaMod_Weapon>(DynamicLoadObject( action.StrValue, class'class' )) );
			break;
		case UICE_ChangeWeaponMod:
			ChangeWeaponItem( class<avaMod_Weapon>(DynamicLoadObject( action.StrValue, class'class' )) );
			break;

		case UICE_SetChannelAnimTree:
			InitAnim(UICA_Channel);
			break;
		case UICE_SetReadyAnimTree:
			InitAnim(UICA_Ready);
			break;

		case UICE_PlayReadyTurn:
			PlayTurnAnim();
			break;

		case UICE_ChangePointMan:
		case UICE_ChangeRifleMan:
		case UICE_ChangeSniper:
			ChangeClassType( EPlayerClassType(action.Event - UICE_ChangePointMan) );
			break;

		case UICE_ChangeTeamEU:
		case UICE_ChangeTeamNRF:
			ChangeTeam( ETeamType(action.Event - UICE_ChangeTeamEU) );
			break;

		case UICE_ChangeServerCharacter:
			ChangeServerCharacter();
			break;
		case UICE_ChangeServerWeapon:
			ChangeServerWeapon();
			break;
		case UICE_ChangeInventoryWeapon:
			ChangeInventoryWeapon( action.StrValue );
			break;

		case UICE_ChangeServerTeamClass:
			ChangeServerTeamClass();
			break;

		case UICE_InitReadyTurn:
			InitTurnAnim();
//			PlayTurnAnim( true );
			break;

		case UICE_RemoveCharacterHelmet:
			RemoveCharacterMesh(CHAR_SLOT_H1);
			break;
	}
}

simulated function OnUICharacterEventEx( avaSeqAct_UICharacterEventEx action )
{
	`log("### "@action.Event@" - "@action.ObjValue@" ### >>" @Name);

	switch(action.Event)
	{
		case UICE_ChangeCharacter:
			ChangeCharacter( class<avaCharacter>(action.ObjValue) );
			break;
		case UICE_ChangeCharacterMod:
			ChangeCharacterItem( class<avaCharacterModifier>(action.ObjValue) );
			break;
		case UICE_ChangeWeapon:
			ChangeWeapon( class<avaMod_Weapon>(action.ObjValue) );
			break;
		case UICE_ChangeWeaponMod:
			ChangeWeaponItem( class<avaMod_Weapon>(action.ObjValue) );
			break;

		case UICE_SetChannelAnimTree:
			InitAnim(UICA_Channel);
			break;
		case UICE_SetReadyAnimTree:
			InitAnim(UICA_Ready);
			break;

		case UICE_PlayReadyTurn:
			PlayTurnAnim();
			break;

		case UICE_ChangePointMan:
		case UICE_ChangeRifleMan:
		case UICE_ChangeSniper:
			ChangeClassType( EPlayerClassType(action.Event - UICE_ChangePointMan) );
			break;

		case UICE_ChangeTeamEU:
		case UICE_ChangeTeamNRF:
			ChangeTeam( ETeamType(action.Event - UICE_ChangeTeamEU) );
			break;

		case UICE_ChangeServerCharacter:
			ChangeServerCharacter();
			break;
		case UICE_ChangeServerWeapon:
			ChangeServerWeapon();
			break;
		case UICE_ChangeInventoryWeapon:
			//ChangeWeapon( class<avaMod_Weapon>(action.ObjValue), true );
			`log("Not Used!!");
			break;

		case UICE_ChangeServerTeamClass:
			ChangeServerTeamClass();
			break;

		case UICE_InitReadyTurn:
			InitTurnAnim();
//			PlayTurnAnim( true );
			break;

		case UICE_RemoveCharacterHelmet:
			RemoveCharacterMesh(CHAR_SLOT_H1);
			break;
	}
}

//! PostBeginPlay �Լ����� avaCharacter.ChangeMesh�� ȣ��Ǵ���
//! ���αװ� ���� �߻��ϴ� �� ���´�.
simulated function ChangeMesh( int nTeam )
{
}

static event LoadDLOs()
{
	Super.LoadDLOs();
	DLO( "avaCharCommon.UI_ChannelAnimTree" );
	DLO( "avaCharCommon.UI_ReadyAnimTree" );
}


defaultproperties
{
	AnimTreeType = UICA_Channel
	bSaveAnimSeqTime = false;

	Begin Object Name=WPawnSkeletalMeshComponent		
		Translation							= (X=0,Y=0,Z=0.0)		
		AnimSets(0)							= AnimSet'avaCharCommon.UI_Animset'
		AnimTreeTemplate					= AnimTree'avaCharCommon.UI_ChannelAnimTree'
		LODBias								= 0
		bUpdateSkelWhenNotRendered          = true
		bIgnoreControllersWhenNotRendered   = false
	End Object

	bExtraMeshInit							=	false
//	TypeID									=	-1
	DefaultTeam								=	-1
	
	Begin Object Name=HeadComponent
		LODBias								=	0
	End Object

	Begin Object Name=EyeBallComponent
		LODBias								=	0
	End Object

	HeadComp=None
	EyeBallComp=None

	HeadMeshName = ""
	HeadMTSName  = ""

	TeamID = TEAM_EU
	TypeID = 0

	bUpdateTeamClass = true

	bEnableAutoLookAtControl = false
	bUISceneLookAtEnable = false
	bLockedLookAtEnabled = false

	BaseModifierName(0)	=	"avaRules.avaMod_Mark_NRF"
	//BaseModifierName(1)	=	"avaRules.avaMod_TestHead"
}
