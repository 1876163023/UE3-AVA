/*
	UI Scene의 배경으로 사용되는 캐릭터 클래스.

	2007/05/14	고광록
		악세서리(AttachedItem)들이 모두 사라지고 기획과 서버측 DB에서만 존재하며
		3D에서는 대표 메쉬를 사용하여 출력한다.
		(통메쉬로 Helmet, Body, Armor, Item, Head+Eye 이렇게 구분된다)

		@note
			bUpdateSkelWhenNotRendered = true로 안해주면 첫 프레임에서 애니메이션이 
			갱신이 안되더라.

	2007/02/08	고광록
		무기만 다른 같은 종류의 애니메이션이 바뀌더라도 Playtime을 저장해서 복구해서
		동작이 튀지 않고 부드럽게 연결되도록 수정.

	2007/02/01	고광록
		LocalPlayerController에서 PRI, PMI를 얻으면서 Server접속유무를 알 수 없게 되어
		PRI.bInitLastInfo의 값을 검사하도록 수정.

	2007/01/18	고광록
		avaCharacter.uc, avaUICharacterPIP.uc파일을 참고해서 만듬.
*/
class avaUICharacter extends avaCharacter
	dependson(avaCharacterModifier)
	dependson(avaGame);

`include(avaGame/avaGame.uci)


//! 캐릭터의 애니메이션 트리의 종류를 정의.
enum EUICharAnimTree
{
	UICA_Channel,
	UICA_Ready,

	UICA_None,
//	UICA_Test,
//	UICA_Max
};

/*! @brief 팀 정보를 보관한다.
	@note
		또한 avaPawn.uc에서 이미 TypeID란 값으로 병과를 저장하고 있다.
		(ChangeTeam, ChangeClass에서 변환 가능하게 할 예정이다.)
*/
var ETeamType								TeamID;

//! 서버에서 팀과 병과 정보를 갱신한다.
var bool									bUpdateTeamClass;

//! 병과별 애니메이션 유무.
var() EUICharAnimTree						AnimTreeType;
//! 무기별로 애니메이션이 설정될 때 시간을 저장하는지 유무.
var bool									bSaveAnimSeqTime;
//! 위의 변수에서 저장된 애니메이션 시간.
var float									LastAnimTime;
//! 마지막으로 재생된 애니메이션 접미어(ex. _Idle, _BackTurn ... )
var string									LastAnimSuffix;
//!
var int										ActiveChildIndex;
//! 턴동작 시작위치.
var vector									TurnAnimStartPos;
//! 턴동작 시작회전값.
var rotator									TurnAnimStartRot;
//! 턴동작 재생 횟수.
var int										TurnAnimCount;
//! 턴동작중인가?
var bool									bPlayingTurnAnim;
//! 
var bool									bNeedInitTurnAnim;
//!
var bool									bLastLookAtEnabled;
//!
var vector									TargetLocation;
//! 
var string									CurAnimSuffix;


//! 현재 캐릭터 클래스.
var() class<avaCharacter>					CharClass;
//! 팀, 병과별 캐릭터 클래스.(6 = `MAX_PLAYER_CLASS * `MAX_TEAM).
var array< class<avaCharacter> >			CharClasses;
//! 보관하고 있는 캐릭터 Modifier들의 정보.
var() array< class<avaCharacterModifier> >	Modifiers;


//! 무기 정보.
var class< avaWeapon >						WeaponClass;
//! 무기의 장착 아이템 정보.
var array< class<avaMod_Weapon> >			WeaponModifiers;


/*
	@note
		다음과 같은 계층구조를 가지게 된다.

		Mesh									=> 기본 뼈대.
			- BodyMeshes(bone)					=> 몸체 메쉬와 추가 장착 스킨메쉬.
			- BodyItemMeshes(socket)			=> 몸의 추가 장착 메쉬.
			- HelmetMesh(socket)				=> 헬멧 메쉬.
				- HelmetItemMeshes				=> 헬멧의 추가 장착 메쉬.
			- HeadMesh(bone)					=> 얼굴 메쉬.
			- EyeMesh(socket)					=> 눈 메쉬.
			- WeaponSkeleton(bone)				=> 무기의 위치 보정용 뼈대.
				- WeaponSocketSkeleton(bone)	=> 무기의 아이템 장착 소켓용 뼈대.
					- WeaponBasicMesh(bone)		=> 무기의 기본 메쉬.
					- WeaponItemMeshes(socket)	=> 무기의 추가 장착 메쉬.
*/

//! 헬멧의 단순한 탈착을 위해서 추가한 변수.
var SkeletalMeshComponent					HelmetComp;


//! 무기 정보 저장.
struct WeaponPart
{
	var string					MeshName;		//!< 무기 아이템 이름.
	var name					SocketName;		//!< 무기 소켓 이름.
	var StaticMeshComponent		ItemMesh;		//!< 무기 아이템 메쉬.
	var EWeaponSlot				Slot;			//!< 무기 슬롯.
};


//! 무기의 위치 설정용 메쉬(only Skeleton).
var MeshComponent							WeaponMesh;
//! 무기의 소켓위치 설정용 메쉬.
var SkeletalMeshComponent					WeaponSocMesh;
//! 무기의 기본 메쉬.
var StaticMeshComponent						WeaponBasicMesh;
//! 무기의 장착 메쉬.
var array<WeaponPart>						WeaponItemParts;

//! Kismet으로 전달된 현재 장면에서의 LookAt유무.
var bool									bUISceneLookAtEnable;
//! 애니메이션중 LookAt처리가 안되도록 막는지 유무.
var bool									bLockedLookAtEnabled;


var array< string >							BaseModifierName;

//-----------------------------------------------------------------------------
//	functions
//-----------------------------------------------------------------------------

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	// '도리도리'를 멈추고 싶어~!!
	if ( Mesh != None )
		Mesh.SkelVisibleTime = 1.0f;

	TurnAnimStartPos = Location;
	TurnAnimStartRot = Rotation;


	// 에디터에서도 볼 수 있도록 수정.(창기형이 카메라 잡을 때 필요하셔서...)
//	`if(`notdefined(FINAL_RELEASE))
	ApplyAllModifiers();
	InitAnim(AnimTreeType);
//	`endif
}

/*! @brief SkeletalMeshComponent를 생성해 준다.
	@param ThisMeshName
		생성할 메쉬 이름.
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

/*! @brief StaticMeshComponent를 생성해 준다.(copy avaUICharacterPIP).
	@param ThisMeshName
		생성할 메쉬 이름.
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

/*! @brief ExtraMesh를 추가(Slot추가용).
	@note
		CollectCharacterMesh에서 사용됨.
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
///*! @brief ItemMesh를 추가(Slot추가용).
//	@note
//		CollectCharacterMesh에서 사용됨.
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
///*! @brief HelmetAccessory를 추가(Slot추가용).
//	@note
//		CollectCharacterMesh에서 사용됨.
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

/*! @brief Character Modifier에서 캐릭터의 메쉬 정보를 얻는다.
	@note
		적용하기 위해서는 ApplyCharacterModifier함수가 호출되어야 한다.
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

	//// 선택된 캐릭터 클래스에서 팀와 병과 정보는
	//// 선행된 함수에서 TeamID, TypeID가 설정되어 있어야 한다.

	//// Modifier에서 슬롯 정보를 얻어온다.
	//Slot = Mod.default.Slot;

	//// Body의 공통 메쉬를 얻어온다.
	//for ( i = 0 ; i < Mod.default.CommonExtraMeshes.length ; ++ i )
	//	AddExtraMeshSlot( Mod.default.CommonExtraMeshes[i].MeshName, Slot );

	//// Body의 팀, 병과별 메쉬를 얻어온다.
	//ExtraMeshes = Mod.static.GetExtraMeshes( TeamID, TypeID );
	//for ( i = 0 ; i < ExtraMeshes.length ; ++i )
	//	AddExtraMeshSlot( ExtraMeshes[i].MeshName, Slot );

	//// BodyItem의 공통 메쉬를 얻어온다.
	//for ( i = 0 ; i < Mod.default.CommonAttachedItems.length ; ++ i )
	//	AddItemMeshSlot( Mod.default.CommonAttachedItems[i].MeshName, Mod.default.CommonAttachedItems[i].PrimarySocket, Slot );

	//// BodyItem의 팀, 병과별 메쉬를 얻어온다.
	//AttachedItems = Mod.static.GetAttachedItems( TeamID, TypeID );
	//for ( i = 0 ; i < AttachedItems.length ; ++ i )
	//	AddItemMeshSlot( AttachedItems[i].MeshName , AttachedItems[i].PrimarySocket, Slot );

	//// Helmet의 팀, 병과별 메쉬를 얻어온다.
	//HelmetMesh = Mod.static.GetHelmetMesh( TeamID, TypeID );
	//if ( HelmetMesh.MeshName != "" )
	//	SetHelmet( HelmetMesh.MeshName );

	//// HelmetItem의 팀, 병과별 메쉬를 얻어온다.
	//AttachedItems = Mod.static.GetHelmetAttachedItems( TeamID, TypeID );
	//for ( i = 0 ; i < AttachedItems.length ; ++ i )
	//	AddHelmetAccessorySlot( AttachedItems[i].MeshName , AttachedItems[i].PrimarySocket, Slot );

	//// 머리에 대한 메쉬이름 얻기.
	//if ( Mod.default.HeadMeshName != "" )
	//	HeadMeshName = Mod.default.HeadMeshName;

	//// 눈꺼풀(Morph용)의 메쉬 이름 얻기.
	//if ( Mod.default.HeadMTSName != "" )
	//	HeadMTSName = Mod.default.HeadMTSName;

	//// 눈동자(LookAt용)의 메쉬 이름 얻기.
	//if ( Mod.default.EyeBallMeshName != "" )
	//	EyeBallMeshName = Mod.default.EyeBallMeshName;
}

//! Modifier Slot에서 해당하는 메쉬를 제거한다.
simulated function RemoveCharacterMesh( ECharSlot Slot )
{
	local int i;

//	`log("### RemoveCharacterMesh - " @Slot);

	// 몸통 메쉬 제거.
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

	// 몸통 아이템 메쉬 제거.
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

	// 헬멧 아이템 메쉬 제거.
	if ( Slot == CHAR_SLOT_H1 )
	{
		if( HelmetComp != None )
			Mesh.DetachComponent( HelmetComp );

		HelmetComp     = None;
		HelmetMeshName = "";
		HelmetSkinName = "";

		// 헬멧에 딸려 있는 아이템 메쉬도 제거.
		// (Collect한 상태로만 해주어서 새로 생성되는 Helmet에 부착되도록 한다)
		for ( i = 0; i < HelmetAccessory.Length; ++ i)
		{
			Mesh.DetachComponent( HelmetAccessory[i].ItemMesh );
			HelmetAccessory[i].ItemMesh = None;
		}
	}

	// 헬멧 아이템 메쉬 제거.
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

	// 얼굴 제거.
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

//! 무기 메쉬만 제거.
simulated function RemoveWeaponMesh()
{
	local int i;

	if ( WeaponMesh != None )
	{
		if ( WeaponSocMesh != None )
		{
			// 장착 무기 제거.
			for ( i = 0; i < WeaponItemParts.Length; ++i)
				WeaponSocMesh.DetachComponent( WeaponItemParts[i].ItemMesh );

			WeaponItemParts.Length = 0;

			// 주무기 제거.
			if( WeaponBasicMesh != None )
			{
				WeaponSocMesh.DetachComponent( WeaponBasicMesh );
				WeaponBasicMesh = None;
			}

			// 소켓 메쉬 제거.
			SkeletalMeshComponent(WeaponMesh).DetachComponent( WeaponSocMesh );
			WeaponSocMesh = None;
		}

		// 위지 보정 메쉬 제거.
		Mesh.DetachComponent( WeaponMesh );
		WeaponMesh = None;
	}

	WeaponModifiers.Length = 0;
	WeaponClass            = None;
}

//! 모든 메쉬 제거.
simulated function RemoveAllMeshes()
{
	local int i;

	// 몸통 메쉬 제거.
	for ( i = 0; i < BodyParts.Length; ++ i )
		if ( BodyParts[i].BodyMesh != None )
			Mesh.DetachComponent( BodyParts[i].BodyMesh );

	BodyParts.Length = 0;

	// 몸통 아이템 메쉬 제거.
	for ( i = 0; i < ItemParts.Length; ++ i )
		if ( ItemParts[i].ItemMesh != None )
			Mesh.DetachComponent( ItemParts[i].ItemMesh );

	ItemParts.Length = 0;

	// 헬멧 아이템 메쉬 제거.
	if ( HelmetComp != None )
	{
		Mesh.DetachComponent( HelmetComp );
		HelmetComp = None;
	}

	HelmetMeshName = "";
	HelmetSkinName = "";

	// 헬멧 아이템 메쉬 제거.
	for ( i = 0; i < HelmetAccessory.Length; ++ i )
		if ( HelmetAccessory[i].ItemMesh != None )
			Mesh.DetachComponent( HelmetAccessory[i].ItemMesh );

	HelmetAccessory.Length = 0;

	// 얼굴 제거.
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

	// 무기 메쉬도 제거.
	RemoveWeaponMesh();
}

/*! @brief 머리정보를 이용하여 메쉬를 적용한다.
	@note
		CollectCharacterMesh함수가 선행되어 MeshName값이 있어야 한다.
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
			
			// Set MorphTargetSet(눈 깜빡임)
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

			// Set EyeBall(눈동자)
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

/*! @brief 헬멧정보를 이용하여 메쉬를 적용한다.
	@note
		CollectCharacterMesh함수가 선행되어 MeshName값이 있어야 한다.
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

			// H1 소켓에 붙여준다.
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

				// 헬멧의 자식으로 해당 소켓에 붙여준다.
				HelmetComp.AttachComponentToSocket( HelmetAccessory[i].ItemMesh, HelmetAccessory[i].SocketName );
			}
		}
	}
}

/*! @brief 몸통정보를 이용하여 메쉬를 적용한다.
	@note
		CollectCharacterMesh함수가 선행되어 MeshName값이 있어야 한다.
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
			// (새로 추가되어 이름 정보만 있는 경우에 대해서 생성한다)
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
				// 안해주면 첫 프레임에서 애니메이션이 갱신이 안되더라.(2007/05/14)
				BodyParts[i].BodyMesh.bUpdateSkelWhenNotRendered = true;

				if ( BodyParts[i].Slot == CHAR_SLOT_MARK )
				{
					Assert( DecalParentMesh == None );
					DecalParentMesh = BodyParts[i].BodyMesh;
				}
			}

			// add it to the components array if it's not there already
			// Mesh 에 대하여 AttachComponent 를 하는 이유는 ChangeVisibility 시에 자동으로 해주기 위해서이다.
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
			// (새로 추가되어 이름 정보만 있는 경우에 대해서 생성한다)
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

/*! @brief 무기 메쉬를 적용한다.
	@note
		CollectAllModifiers()함수가 선행되어야 제대로 적용된다.
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

	// 3인칭용 무기 클래스.	
	WeaponClass3p = WeaponClass.default.AttachmentClass;

	// 무기 아이템만 생성해야 하는 경우가 있다.
	if( WeaponMesh == None )
	{
		if ( WeaponClass3p.default.bMeshIsSkeletal )
		{
			TempName = WeaponClass3p.default.MeshName$"_UI";
			// UI용 메쉬가 있으면.
			//if ( WeaponClass3p.default.UIMeshName != "" )
			//	TempName = WeaponClass3p.default.UIMeshName
			//else
			//	TempName = WeaponClass3p.default.MeshName;

			// Mesh 가 Skeletal 인 경우는 Carried 를 같이 붙이기 위해서 이다. 즉, SocMesh 와 BasicMesh 는 반드시 필요하다...
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


		// 기본 무기를 만들어서 붙인다.
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
		// 무기 장착 아이템 생성.
		for( i = 0; i < WeaponModifiers.Length; ++ i )
		{
			WeaponItems = WeaponModifiers[i].default.CommonAttachedItems;

			// 스킨을 교체한다.
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

			// 장착 아이템을 붙여준다.
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

				// SocketMesh에 붙이기.
				WeaponSocMesh.AttachComponentToSocket( WeaponItemParts[k].ItemMesh, WeaponItemParts[k].SocketName );
			}
		}
	}
}

/*! @brief 모든 메쉬를 적용한다.
	@note
		CollectCharacterMesh함수가 선행되어 MeshName값이 있어야 한다.
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


/*! @brief Local에서 Character/Weapon Modifier들의 정보를 얻어온다.
	@param bCharacter
		캐릭터정보의 갱신유무.
	@param bWeapon
		무기정보의 갱신유무.
	@note
		적용하기 위해서는 ApplyModifiers함수가 호출되어야 한다.
*/
simulated function CollectAllModifiersFromLocal(optional bool bCharacter = true, optional bool bWeapon = true)
{
	local int i;

	// 만약 캐릭터 클래스들이 없다면.
	if ( CharClasses.Length == 0 )
		InitCharacterClasses();

//	`log("### CollectAllModifiersFromLocal (bCharacter:" @bCharacter @",bWeapon:" @bWeapon @")"
//		 @"(TeamID:" @TeamID @"TypeID:" @TypeID @")");

	if ( bCharacter )
	{
		// 캐릭터 정보가 없다면 정보를 얻을 수 없다.
		// ChangeCharacter의 경우에는 설정되어서 이 부분에 들어올 수 있다.
		if ( CharClass == None )
			CharClass = CharClasses[TeamID * 3 + TypeID];

		// 팀을 얻는다.(from avaCharacter)
		if ( CharClass.default.DefaultTeam < 0 || CharClass.default.DefaultTeam > `MAX_TEAM )
			TeamID = TEAM_EU;
		else
			TeamID = ETeamType(CharClass.default.DefaultTeam);

		// 병과를 얻는다(from avaPawn).
		TypeID = CharClass.default.TypeID;
		if ( TypeID < 0 || TypeID > `MAX_PLAYER_CLASS )
			TypeID = 0;

		// 초기화.
		Modifiers.Length = 0;

		// 서버와 연결되어 있지 않은경우 처리된다.
		Modifiers = CharClass.default.DefaultModifier;
	}

	if ( bWeapon )
	{
		// 캐릭터 클래스가 없으면 기본 무기정보를 얻어올 수가 없다.
		if ( CharClass == None )
		{
			`log("Cannont Collect Weapon Modifiers. ( CharClass == None )");
			return ;
		}

		// 캐릭터에 기본으로 설정된 무기를 설정.
		if ( CharClass.default.DefaultWeapons.Length > 0 )
		{
			WeaponClass = CharClass.default.DefaultWeapons[0];

			// avaWeapon에서 class<avaModifier>자료형의 정적배열 복사...
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

/*! @brief Server에서 갱신된 PlayerModifierInfo(PMI)에서 Character/Weapon Modifier들의 정보를 얻어온다.
	@param bCharacter
		캐릭터정보의 갱신유무.
	@param bWeapon
		무기정보의 갱신유무.
	@note
		적용하기 위해서는 ApplyModifiers함수가 호출되어야 한다.
*/
simulated function CollectAllModifiersFromServer(optional bool bCharacter = true, optional bool bWeapon = true)
{
	local avaPlayerModifierInfo		avaPMI;
	local int						index;
	local int						i;

	// 만약 캐릭터 클래스들이 없다면.
	if ( CharClasses.Length == 0 )
		InitCharacterClasses();

	// 서버에서 받은 PMI정보를 얻는다.
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
		// 초기화.
		Modifiers.Length = 0;

		// 팀과 병과별로 캐릭터 클래스를 얻는다.
		CharClass = CharClasses[TeamID * 3 + TypeID];

		// 무기만 갱신이라면 제외.
		// 현재 PMI에 Modifiers를 얻어온다.

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
		// 서버에서 받은 값의 [0]번의 무기를 일단 기본으로 설정.
		// ([0]번이 Primary Weapon이 된다고 한다)
		if( avaPMI.ClassTypeInfos[TypeID].WeaponInfos.Length > 0 )
		{
			WeaponClass     = avaPMI.ClassTypeInfos[TypeID].WeaponInfos[0].Class;
			WeaponModifiers = avaPMI.ClassTypeInfos[TypeID].WeaponInfos[0].Mod;
		}
	}
}

/*! @brief Server or Local에서 Character/Weapon Modifier들의 정보를 얻어온다.
	@param bServer
		서버에서 받은 정보에서 갱신한다.
	@param bCharacter
		캐릭터정보의 갱신유무.
	@param bWeapon
		무기정보의 갱신유무.
	@note
		적용하기 위해서는 ApplyModifiers함수가 호출되어야 한다.
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

/*! @brief Character Modifier에서 캐릭터의 메쉬 정보를 얻는다.
	@note
		적용하기 위해서는 ApplyCharacterModifier함수가 호출되어야 한다.
	@remark
		캐릭터 CharClass객체가 None이 아니여야 한다.
*/
simulated function ApplyAllModifiers(optional bool bServer = false)
{
	local int i;

	// 모든 메쉬를 지운다.
	RemoveAllMeshes();


	// 모든 Modifiers정보를 얻어온다.
	CollectAllModifiers(bServer);

	// Modifiers에서 메쉬정보를 얻어온다.
	for ( i = 0; i < Modifiers.Length; ++ i )
		CollectCharacterMesh( Modifiers[i] );

	// Set Base Skeletal Mesh(팀 or 병과를 바꿀 때에는 설정이 다시 되어야 한다)
	Mesh.SetSkeletalMesh( CharClass.default.BaseSkeletalMesh );


	// 모든 메쉬 정보를 적용한다.
	ApplyAllMeshes();

	// 병과 or 무기에 따른 동작 초기화.
	InitAnim();
}


//------------------------------------------------------------- ----------------
//	접근 함수.
//-----------------------------------------------------------------------------

//! 로컬의 정보로 새로운 캐릭터/무기를 교체한다.
simulated function ChangeCharacter( class<avaCharacter> NewCharClass )
{
	if ( NewCharClass != None )
		CharClass = NewCharClass;

	ApplyAllModifiers();
}


/*! @brief 캐릭터 장비를 교체한다.
	@param 새로 장비할 Modifier를 넣어준다.
*/
simulated function ChangeCharacterItem( class<avaCharacterModifier> Mod )
{
	local int i;
	local avaNetHandler		NetHandler;
	local BYTE Level,LastClass,LastTeam;
	local int  Exp,SupplyPoint,Cash,Money,idClanMark;
	
	if ( Mod == None )
		return ;

	// Modifier정보가 없다면 얻어온다.
	if ( Modifiers.Length == 0 )
		CollectAllModifiersFromLocal();

	for ( i = 0; i < Modifiers.Length; ++ i )
	{
		// 슬롯 1개당 1개의 Modifier가 들어간다고 한다.
		if ( Modifiers[i].default.Slot == Mod.default.Slot )
			break;
	}

	if( i == Modifiers.Length )
	{
		// 1. 새로운 Modifier의 추가.
		Modifiers.AddItem( Mod );

//		`log("ChangeCharacterItem : Add" @Mod.Name);
	}
	else
	{
//		`log("ChangeCharacterItem : Remove" @Modifiers[i].Name);

		// 1. 이전 슬롯에 해당하는 메쉬 제거(BodyPart or ItemPart, Mesh.DetachComponent).
		// (해당 슬롯을 사용하는 메쉬를 모두 제거)
		RemoveCharacterMesh( Modifiers[i].default.Slot );

		// 이전 Modifier와 교체.
		Modifiers[i] = Mod;

//		`log("ChangeCharacterItem : Swap" @Modifiers[i].Name);
	}

	// 2. 새로운 Modifier를 리스트(Midifiers)에 추가.
	// (MeshName을 갱신해 준다)
	CollectCharacterMesh( Mod );

	// 3. 해당 메쉬 새로 적용.
	// (갱신된 MeshName에 대해서 (MeshComponent == None)인 경우만 생성한다)
	ApplyHeadMesh();
	ApplyHelmetMesh();
	ApplyBodyMesh();

	NetHandler = class'avaNetHandler'.static.GetAvaNetHandler();
	NetHandler.GetPlayerInfo( Level, LastClass, LastTeam, Exp, SupplyPoint, Cash, Money, idClanMark );
	CreateCharacterDecal( int(TeamID) , Level, idClanMark, , , true );

	InitializeSkelControl();

	// 병과 or 무기에 따른 동작 초기화.
//	InitAnim();
}


/*! @brief 무기를 교체한다.
	@param Mod
		새로 장비할 Modifier를 넣어준다.
*/
simulated function ChangeWeapon( class<avaMod_Weapon> Mod )
{
	local int i;

	// 무기 제거하고
	RemoveWeaponMesh();


	if ( Mod == None )
		return ;

	// 주무기 Modifier에서 무기 클래스를 얻어서.
	WeaponClass = Mod.default.WeaponClass;

	// avaWeapon에서 class<avaModifier>자료형의 정적배열 복사...
	// (장착 Modifier들을 복사한다)
	WeaponModifiers.Length = 0;
	for ( i = 0; i < WeaponClass.default.DefaultModifiers.Length; ++ i )
	{
		if ( WeaponClass.default.DefaultModifiers[i] == None )
			continue;

		WeaponModifiers.AddItem( class<avaMod_Weapon>(WeaponClass.default.DefaultModifiers[i]) );
	}

	// 무기 적용하고
	ApplyWeaponMesh();

	// 병과 or 무기에 따른 동작 초기화.
	InitAnim();
}


/*! @brief 인벤토리 무기로 교체한다.
	@param SN
		Item Serial Number가 0이 아닌 값이 입력되면 소유한 무기에서 장착 아이템을 얻어온다.
*/
simulated function ChangeInventoryWeapon( string ItemSN )
{
	// 무기 제거하고
	RemoveWeaponMesh();


	// 소지하고 있는 장착 아이템을 얻어온다.
	WeaponModifiers.Length = 0;
	WeaponModifiers = class'avaNetHandler'.static.GetAvaNetHandler().GetWeaponModifiers( ItemSN );

	// 주무기를 얻어오고, 리스트에서 제거한다.
	WeaponClass = WeaponModifiers[0].default.WeaponClass;
	WeaponModifiers.Remove( 0, 1 );


	// 무기 적용하고
	ApplyWeaponMesh();

	// 병과 or 무기에 따른 동작 초기화.
	InitAnim();
}


/*! @brief 무기를 교체한다.
	@param 새로 장비할 Modifier를 넣어준다.
*/
simulated function ChangeWeaponItem( class<avaMod_Weapon> Mod )
{
	local int i;

	if ( Mod == None )
		return ;

	// ( Mod.default.Slot == WEAPON_SLOT_None )의 경우는 데이터가 입력되지 않은 경우이다.
//	`log("### ChangeWeaponItem :" @Mod @" ### SlotType:" @Mod.default.Slot);

	// 장착 무기 제거.
	for ( i = 0; i < WeaponItemParts.Length; ++i)
		WeaponSocMesh.DetachComponent( WeaponItemParts[i].ItemMesh );
	WeaponItemParts.Length = 0;


	// 해당 무기 아이템이 이미 장착 되었는지 같은 슬롯유무를 검색한다.
	for ( i = 0; i < WeaponModifiers.Length; ++ i )
	{
		if ( WeaponModifiers[i].default.Slot == Mod.default.Slot )
			break;
	}

	// 없는 경우에는 추가, 있다면 교체.
	if ( i == WeaponModifiers.Length )
		WeaponModifiers.AddItem( Mod );
	else
		WeaponModifiers[i] = Mod;

//	for ( i = 0; i < WeaponModifiers.Length; ++i )
//		`log("ApplyWeapon - WeaponModifiers[" @i @"] = " @WeaponModifiers[i]);

	// 무기 적용하고
	ApplyWeaponMesh();

	// 병과 or 무기에 따른 동작 초기화.
//	InitAnim();
}

/*! @brief 팀정보를 변경하고 캐릭터를 새로 생성한다.
	@note
		Local정보로 갱신.
*/
simulated function ChangeTeam(ETeamType Team)
{
	if ( Team != TEAM_EU && Team != TEAM_USSR )
		Team = TEAM_EU;

	TeamID = Team;

	// None인 경우 TeamID, TypeID값을 이용해서 캐릭터가 생성된다.
	CharClass = None;

	ApplyAllModifiers();
}

/*! @brief 정보를 변경하고 캐릭터를 새로 생성한다.
	@note
		Local정보로 갱신.
*/
simulated function ChangeClassType(EPlayerClassType Type)
{
	// 범위를 벗어나면 PointMan을 기본값으로.
	if( Type < 0 || Type > `MAX_PLAYER_CLASS )
		Type = PCT_PointMan;

	TypeID = Type;

	// None인 경우 TeamID, TypeID값을 이용해서 캐릭터가 생성된다.
	CharClass = None;

	ApplyAllModifiers();
}

//! 서버에서 받은 PMI정보로 캐릭터로 교체한다.
simulated function ChangeServerCharacter()
{
	// 서버에서 팀과 병과 정보를 다시 얻어온다.
	bUpdateTeamClass = true;

	// 내부적을로 팀(TeamID)과 병과(TypeID)정보를 얻어오는 함수.
//	GetAvaPlayerModifierInfo();

	ApplyAllModifiers(true);
}

/*! @brief (서버에서 받은 정보로) 무기를 교체한다.
*/
simulated function ChangeServerWeapon()
{
	// 무기 제거하고
	RemoveWeaponMesh();

	// 서버에서 얻은 정보로 무기 정보(WeaponClass, WeaponModifiers만 갱신한다).
	CollectAllModifiersFromServer(false, true);

	// 무기 적용하고
	ApplyWeaponMesh();

	// 병과 or 무기에 따른 동작 초기화.
	InitAnim();
}

//! 서버에서 받아온 정보로 팀과 병과를 정해서 기본 캐릭터로 보여준다.
simulated function ChangeServerTeamClass()
{
	local int i;

	// 모든 메쉬를 지운다.
	RemoveAllMeshes();


	// 서버에서 팀과 병과 정보를 다시 얻어온다.
	bUpdateTeamClass = true;

	// 캐릭터 정보는 서버, 무기 정보는 로컬에서 얻어온다.
	CollectAllModifiersFromServer(true, false);
	CollectAllModifiersFromLocal(false, true);

	// Modifiers에서 메쉬정보를 얻어온다.
	for ( i = 0; i < Modifiers.Length; ++ i )
		CollectCharacterMesh( Modifiers[i] );

	// Set Base Skeletal Mesh(팀 or 병과를 바꿀 때에는 설정이 다시 되어야 한다)
	Mesh.SetSkeletalMesh( CharClass.default.BaseSkeletalMesh );


	// 모든 메쉬 정보를 적용한다.
	ApplyAllMeshes();

	// 병과 or 무기에 따른 동작 초기화.
	InitAnim();
}


//------------------------------------------------------------- ----------------
//	Misc Function
//-----------------------------------------------------------------------------

/*! @brief 서버에서 얻은 PMI정보를 얻어주는 함수.
	@return
		서버와 연결이 되어 있는 경우가 아니면 None이 리턴된다.
	@note
		서버와 연결된 최초의 1회에 한해서 TeamID, TypeID를 설정해 준다.
	@remark
		bUpdateTeamClass값이 false인 경우에만 갱신된다.
		(다시 갱신하고 싶다면 false로 설정해주면 된다)
*/
function avaPlayerModifierInfo GetAvaPlayerModifierInfo()
{
	local PlayerController			PC;
	local avaPlayerController		avaPC;
	local avaPlayerReplicationInfo	avaPRI;
	local avaPlayerModifierInfo		avaPMI;
	local ETeamType					oldTeamType;

//	`log("### GetAvaPlayerModifierInfo() ###");

	// LocalPlayerController를 얻어온다(2007/01/31 변경).
	foreach WorldInfo.LocalPlayerControllers(PC)
	{
		avaPC = avaPlayerController(PC);
		break;
	}

	// PRI를 형변환해서 ava용으로 얻어서 PMI를 얻는다.
	avaPRI = avaPlayerReplicationInfo( avaPC.PlayerReplicationInfo );
	if ( avaPRI == None )
	{
		`log("Failed GetAvaPlayerModifierInfo() - (avaPRI == None) : " @avaPRI);
		return None;
	}

	// 최초의 1번에 한해서 서버에서 마지막으로 저장된 팀과 병과를 얻어온다.
	if ( avaPRI.bUpdate == true && bUpdateTeamClass == true )
	{
//		TeamID = ETeamType(avaPRI.LastTeam);
//		TypeID = avaPRI.LastClass;

		oldTeamType = TeamID;

		TeamID = ETeamType(int( class'avaNetHandler'.static.GetAvaNetHandler().GetMyURLString("Team") ));
		TypeID = int( class'avaNetHandler'.static.GetAvaNetHandler().GetMyURLString("LastClass") );

//		`log("bUpdateTeamClass : " @TeamID @". " @TypeID );

		// [2]를 사용하는 Spectator가 있어서 범위를 처리해 준다.
		if ( TeamID != Team_EU && TeamID != Team_USSR )
		{
			if ( oldTeamType != Team_EU && oldTeamType != Team_USSR )
				TeamID = Team_EU;
			else
				TeamID = oldTeamType;
		}

		// 더이상 이 부분을 처리되지 않도록 false로 설정.
		bUpdateTeamClass = false;
	}

	avaPMI = avaPRI.avaPMI;
	if ( avaPMI == None )
	{
		`log("GetAvaPlayerModifierInfo() - (avaPMI == None) : " @avaPRI.avaPMI );
		return None;
	}

	// 만약 Modifiers가 있다면, 서버에서 정상적으로 정보를 얻었다.
	if ( avaPMI.ClassTypeInfos[TypeID].CharMod.length > 0 )
		return avaPMI;

//	`log("GetAvaPlayerModifierInfo() - avaPMI.ClassTypeInfos[TypeID].CharMod.length : " 
//		  @avaPMI.ClassTypeInfos[TypeID].CharMod.length );

	// 서버에서 얻은 정보가 없다.
	return None;
}

//! 팀, 병과별 캐릭터 데이터를 미리 생성해 둔다.
function InitCharacterClasses()
{
	// 미리 6개의 캐릭터 팀, 병과별 템플릿을 생성해 둔다.
	// team_id * 3(`MAX_PLAYER_CLASS) + class_type_id
	CharClasses.AddItem( class<avaCharacter>(DynamicLoadObject( "avaRules.avaCharacter_EUPointMan", class'class' )) );
	CharClasses.AddItem( class<avaCharacter>(DynamicLoadObject( "avaRules.avaCharacter_EURifleMan", class'class' )) );
	CharClasses.AddItem( class<avaCharacter>(DynamicLoadObject( "avaRules.avaCharacter_EUSniper", class'class' )) );
	CharClasses.AddItem( class<avaCharacter>(DynamicLoadObject( "avaRules.avaCharacter_USSRPointMan", class'class' )) );
	CharClasses.AddItem( class<avaCharacter>(DynamicLoadObject( "avaRules.avaCharacter_USSRRifleMan", class'class' )) );
	CharClasses.AddItem( class<avaCharacter>(DynamicLoadObject( "avaRules.avaCharacter_USSRSniper", class'class' )) );

}

/*! @biref 무기에 따라서 캐릭터 동작이 바뀌도록 해주는 함수.
	@note
		[UavaAnimNodes.cpp]
		void UavaAnimBlendByWeaponType::InitAnimSequence(FName NewPrefix)함수에서 호출됨.
		(이거땜에 몇시간을 삽질한겨..ㅠㅠ)
	@original avaPawn.uc
		3인칭 용 Animation 을 위한 Weapon Type 을 Return 해 준다. 2006/1/27 oz
	@var 제거.
		직접 구현한 RebuildAnimSeqName함수로 인하여 필요없다(avaCharacter에서만 필요하다).
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

	// 동작 설정후 LookAt 재설정.
	UICharacter_LookAtEnable( bUISceneLookAtEnable );
}


/*! @brief 플레이 되는 AnimNodeSequence노드를 찾아서 현재 시간을 리턴해 준다.
	@param NodeName
		Root가 되는 노드이름이다.
*/
simulated function SaveAnimSeqTime(name AnimNodeName)
{
	local avaAnimBlendBase			RootAnimBlend;
	local array<AnimNodeSequence>	AnimSeqs;
	local int						i, j;
	local float						DeltaTime;

	// 해당 노드를 찾는다.
	RootAnimBlend = avaAnimBlendBase( Mesh.Animations.FindAnimNode( AnimNodeName ) );
	if ( RootAnimBlend == None )
	{
		`log("### SaveAnimSeqTime - Cannot found " @AnimNodeName);
		return ;
	}

	for ( i = 0; i < RootAnimBlend.Children.Length; ++ i )
	{
		// GetAnimSeqNodes함수에서 누적되어 얻어와서 0으로 초기화.
		// (모든 자식들을 한꺼번에 누적해서 가져올 수 있지만 UAnimNode::GetAnimSeqNodes에서
		//  Reserve하는 개수가 자식노드들의 개수로 하기 때문에 불안요소를 가지고 있다)
		AnimSeqs.Length = 0;

		// 자식 노드의 모든 Animation Node Sequence를 얻어온다.
		RootAnimBlend.GetAnimSeqNodes(i, AnimSeqs);

		// 각 Sequence의 이름을 현재 무기에 맞게 적절해 수정해 준다.
		for ( j = 0; j < AnimSeqs.Length; ++ j )
		{
			DeltaTime = AnimSeqs[j].CurrentTime - AnimSeqs[j].PreviousTime;

			// 만약 애니메이션 중이여서 시간의 변화가 있는 노드인 경우.
			if ( DeltaTime > 0.0 )
			{
				// 만약 재생중인 노드라면...
				LastAnimSuffix   = Right(AnimSeqs[j].AnimSeqName, Len(AnimSeqs[j].AnimSeqName) - InStr(AnimSeqs[j].AnimSeqName, "_"));
				LastAnimTime     = AnimSeqs[j].CurrentTime;
				bSaveAnimSeqTime = true;

//				`log("Saved AnimSeq:" @AnimSeqs[j].AnimSeqName @"(Suffix:"@LastAnimSuffix@")" @" Time: " @AnimSeqs[j].CurrentTime);
				return;
			}
		}
	}
}

/*! @brief 자식노드를 찾아서 모든 애니메이션 이름명의 접두어를 교체해 준다.
	@param NodeName
		Root가 되는 노드이름이다.
	@param WeaponAnimPrifix
		교체될 접두어이다.
	@param bLastWeaponAnimPrefix
		WeaponAnimPrifix가 이름의 뒤쪽에 위치하는 경우 사용.(Channel때문에 추가됨)
	@note
		avaAnimBlendByWeaponType기능을 함수로 빼왔다.<br><br>

		1. OldAnimName("G36_Ready_Idle"), AnimPrefix("M24")가 있을 경우.
		2. AnimSurfix("_Ready_Idle") = OldAnimName("G36_Ready_Idle")에서 '_'뒤에부분 얻기.
		3. AnimName("M24_Ready_Idle") = AnimPrefix("M24") + AnimSuffix("_Ready_Idle")
	@par 개선
		avaAnimBlendBase클래스의 멤버함수로 있는 것이 적당할듯...
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

	// 해당 노드를 찾는다.
	RootAnimBlend = avaAnimBlendBase( Mesh.Animations.FindAnimNode( AnimNodeName ) );
	if ( RootAnimBlend == None )
	{
		`log("### RebuildAnimSeqName - Cannot found " @AnimNodeName);
		return ;
	}

	for ( i = 0; i < RootAnimBlend.Children.Length; ++ i )
	{
		// GetAnimSeqNodes함수에서 누적되어 얻어와서 0으로 초기화.
		// (모든 자식들을 한꺼번에 누적해서 가져올 수 있지만 UAnimNode::GetAnimSeqNodes에서
		//  Reserve하는 개수가 자식노드들의 개수로 하기 때문에 불안요소를 가지고 있다)
		AnimSeqs.Length = 0;

		// 자식 노드의 모든 Animation Node Sequence를 얻어온다.
		RootAnimBlend.GetAnimSeqNodes(i, AnimSeqs);

		// 각 Sequence의 이름을 현재 무기에 맞게 적절해 수정해 준다.
		for ( j = 0; j < AnimSeqs.Length; ++ j )
		{
			// 무기의 WeaponAnimPrefix를 뒤에 붙이는 경우.
			if ( bLastWeaponAnimPrefix )
			{
				// 첫번째 '_'를 찾아서 접두어 부분을 가져온다.
				AnimPrefix = Left(AnimSeqs[j].AnimSeqName, InStr(AnimSeqs[j].AnimSeqName,"_",true) + 1);

				// 새로운 ("기본 이름" + "무기 이름") 으로 생성해 준다.
				AnimName = name(AnimPrefix $ WeaponAnimPrefix);
			}
			else
			{
				// 첫번째 '_'를 찾아서 접미어 부분을 가져온다.
				AnimSuffix = Right(AnimSeqs[j].AnimSeqName, Len(AnimSeqs[j].AnimSeqName) - InStr(AnimSeqs[j].AnimSeqName,"_"));

				// 마지막으로 재생된 노드를 찾는다.
				if ( LastAnimSuffix == AnimSuffix )
					LastAnimSeq = AnimSeqs[j];

				// 새로운 ("무기 이름" + "이후이름") 으로 생성해 준다.
				AnimName = name(WeaponAnimPrefix $ AnimSuffix);
			}

			// 해당 무기에 대한 이름으로 재설정 해준다.
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

	// 저장된 값이 있다면, 플레이 시간도 적용해 준다.
	if ( bSaveAnimSeqTime )
	{
		LastAnimSeq.PlayAnim(false, 1.0f, LastAnimTime);

//		`log("Restore Last Animation Time - AnimNodeName = " @LastAnimSeq.AnimSeqName @", Time:" @LastAnimTime);

		// 1회만 적용된다.
		bSaveAnimSeqTime = false;
	}
}

/*! @brief 캐릭터의 애니메이션(+뼈대) 부분을 초기화 해준다.
*/
simulated function InitAnim( optional EUICharAnimTree NewAnimTreeType = UICA_None )
{
	local bool bUpdate;

	bUpdate = (NewAnimTreeType == UICA_None) ? true : false;

	// 병과별 애니메이션인 경우와 무기별 애니메이션인 경우를 나눈다.
	if ( NewAnimTreeType == UICA_Channel || (AnimTreeType == UICA_Channel && bUpdate) )
	{
//		`log("####### Channel InitAnim(" @NewAnimTreeType @") ######## Old =" @AnimTreeType);

		// 갱신이 아닌 경우에만.
		if ( !bUpdate )
		{
			// AnimSet이 먼저 설정되고, AnimTree가 설정되어야 한다.
			Mesh.AnimSets.Length = 1;
			Mesh.AnimSets[0] = AnimSet(DynamicLoadObject("'avaCharCommon.UI_Animset'", class'AnimSet'));
			Mesh.SetAnimTreeTemplate( AnimTree(DynamicLoadObject("avaCharCommon.UI_ChannelAnimTree", class'AnimTree')) );
		}

		// 'ChannelTypeNode'노드를 기준으로
		// 무기이름에 맞추서 애니메이션 이름을 재구축 해준다.
		if ( WeaponClass != None )
			RebuildAnimSeqName( 'ChannelTypeNode', string(WeaponClass.default.AttachmentClass.default.AnimPrefix), true );

		AnimTreeType = UICA_Channel;
	}
	else if ( NewAnimTreeType == UICA_Ready || (AnimTreeType == UICA_Ready && bUpdate) )
	{
//		`log("####### Ready InitAnim(" @NewAnimTreeType @") ######## Old =" @AnimTreeType);

		// 가능한 경우에 애니메이션 시간을 저장한다.
		SaveAnimSeqTime('ReadyTypeNode');

		// 갱신이 아닌 경우에만 새로 생성해 준다.
		if ( !bUpdate )
		{
			// AnimSet이 먼저 설정되고, AnimTree가 설정되어야 한다.
			Mesh.AnimSets.Length = 1;
			Mesh.AnimSets[0] = AnimSet(DynamicLoadObject("'avaCharCommon.UI_Animset'", class'AnimSet'));
			Mesh.SetAnimTreeTemplate( AnimTree(DynamicLoadObject("'avaCharCommon.UI_ReadyAnimTree'", class'AnimTree')) );
		}
		else
		{
			// 애니메이션 이름만 바꿀 때에는 저장된 동작을 갱신할 필요없다.
			bSaveAnimSeqTime = false;
		}

		// 'ReadyTypeNode'노드를 기준으로
		// 무기이름에 맞추서 애니메이션 이름을 재구축 해준다.
		// (단, 위의 AnimTree가 갱신되지 않았을 경우에는 재구축 할 필요가 없다)
		if ( WeaponClass != None )
			RebuildAnimSeqName( 'ReadyTypeNode', string(WeaponClass.default.AttachmentClass.default.AnimPrefix) );

		AnimTreeType = UICA_Ready;
	}
	else
	{
		// 아무런 애니메이션도 설정하지 않는다.
	}

	// 뼈대 관련 컨트롤들의 초기화(?)
	InitializeSkelControl();
}

//! Kismet으로 초기화가 필요하다고 요청하면...
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

/*! @brief 뒤돌기 동작을 한다.
*/
simulated function PlayTurnAnim( optional bool bInit = false )
{
	local avaAnimBlendBase ReadyTypeAnimBlend;

	// 위치 초기화가 필요할 때는.
	// 값이 1인 경우에만 애니메이션이 되도록 한다.
	if ( bInit )
	{
		if ( TurnAnimCount != 1 )
			return ;
		else
		{
			// 뒤돌아 있다면 처음 위치로 돌린다.
			if ( TurnAnimCount != 0 ) 
			{
				ReadyTypeAnimBlend = avaAnimBlendBase( Mesh.Animations.FindAnimNode( 'ReadyTypeNode' ) );
				if ( ReadyTypeAnimBlend != None )
					ReadyTypeAnimBlend.SetActiveChild(0, 0.25);

				// Tell mesh to stop using root motion
				Mesh.RootMotionRotationMode = RMRM_Ignore;
				Mesh.RootMotionMode         = RMM_Ignore;

				bPlayingTurnAnim = false;

				// (한바퀴 돌고 나서) 위치와 방향 재설정.
				TurnAnimCount = 0;

//				SetLocation(TurnAnimStartPos);
//				SetRotation(TurnAnimStartRot);
			}
			return;
		}
	}

	// 애니메이션 중이면 무시.
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
		// 약간의 BlendTime을 주자.(부드럽게~)
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
	// 강제로 무조건 적용하는가?
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
		// 잠겨 있는 경우는 리턴, 그렇지 않은 경우에만 적용.
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

//! Kismet에서 들어오는 값.
simulated function LookAt( vector at, bool bEnable )
{
	if ( LookAtControl == None )
		return;

	if ( bEnable )
		TargetLocation = at;

	// 무조건 저장.
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
	// 특정 동작에서 카메라를 바라보지 않도록 하며,
	// 무기가 교체될 경우
	if ( animSuffix == "_Idle2" || animSuffix == "_BackTurn" )
	{
		bLockedLookAtEnabled = true;

		UICharacter_LookAtEnable( false, true );
	}
	else
	{
		// AnimNodeRandom의 경우 현재 애니메이션 시간이 초과하는 경우
		// OnAnimEnd를 생략하는 경우도 있다.
		bLockedLookAtEnabled = false;

		if ( bUISceneLookAtEnable )
			UICharacter_LookAtEnable( bUISceneLookAtEnable );
	}

	CurAnimSuffix = animSuffix;
}



/*! @brief 뒤돌기 동작이 끝난 후에 다시 Idle동작으로 설정해 준다.
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

		// (한바퀴 돌고 나서) 위치와 방향 재설정.
		if ( TurnAnimCount >= 2 )
		{
			TurnAnimCount = 0;

			SetLocation(TurnAnimStartPos);
			SetRotation(TurnAnimStartRot);
		}
	}

	// 대상 보는 설정이 잠겨 있었다면 풀어준다.
	bLockedLookAtEnabled = false;

	// UIScene이 LookAt유무인지에 따라서 설정.
	if ( bUISceneLookAtEnable )
		UICharacter_LookAtEnable( true );


	// 필요에 따라서 1번 더...
	if ( bNeedInitTurnAnim )
	{
		bNeedInitTurnAnim = false;

//		`log("avaUICharacter.OnAnimEnd");

		PlayTurnAnim();
	}

//	`log("### OnAnimEnd : " @ SeqNode.AnimSeqName @","@Name);
}

/*! @brief 로그에서 (avaPawn의)에러 메시지 지우기 위해서 추가.
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

//! PostBeginPlay 함수에서 avaCharacter.ChangeMesh가 호출되는지
//! 경고로그가 마무 발생하는 걸 막는다.
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
