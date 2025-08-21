/*
	캐릭터 리소스를 미리 로딩해두는 클래스.

	2007/04/13	고광록
*/
class avaUICharacterCache extends avaUICharacter;

`include(avaGame/avaGame.uci)


//! 미리 머리 정보를 생성해 둔다.
var array< class<avaCharacterModifier> >	HeadMods;

//! 일단 물고 있어 보자.
var array<SoundCue>							SoundCues;

//! avaCache의 함수를 호출해줄 함수.(간단하게....?)
function AddObject(string ObjName, class ObjClass)
{
	class'avaCache'.static.RegisterObject(ObjName, ObjClass);
}

function ApplyHeadMesh()
{
	// Set Head 
	AddObject(HeadMeshName, class'SkeletalMesh');

	// Set MorphTargetSet(눈 깜빡임)
	AddObject(HeadMTSName, class'MorphTargetSet');

	AddObject("avaCharCommon.Head", class'AnimTree');

	// Set EyeBall(눈동자)
	AddObject(EyeBallMeshName, class'SkeletalMesh');
	AddObject("avaCharCommon.EyeBall", class'AnimTree');
}

function ApplyHelmetMesh()
{
	local int i;

	AddObject(HelmetMeshName, class'SkeletalMesh');

	for ( i = 0; i < HelmetAccessory.Length; ++i)
	{
		if( HelmetAccessory[i].MeshName != "" )
		{
			if ( HelmetComp.GetSocketByName( HelmetAccessory[i].SocketName ) == None )
			{
				`warn( "avaUICharacter.ApplyHelmet Cannot Find Socket in" @HelmetAccessory[i].SocketName );
				continue;
			}

			AddObject(HelmetAccessory[i].MeshName, class'StaticMesh');
		}
	}
}

function ApplyBodyMesh()
{
	local int i;

	// Set Body Part
	for ( i = 0 ; i < BodyParts.Length ; ++ i )
		AddObject(BodyParts[i].MeshName, class'SkeletalMesh');

	// Set Item Part
	for ( i = 0 ; i < ItemParts.Length ; ++ i )
		AddObject(ItemParts[i].MeshName, class'StaticMesh');
}

simulated function ApplyWeaponMesh()
{
	local class<avaWeaponAttachment>	WeaponClass3p;
	local class<avaWeap_BaseGun>		GunClass1p;
	local class<avaAttachment_BaseGun>	GunClass3p;
	local array<AttachedItem>			WeaponItems;
	local int							i, j;

	if( WeaponClass == None )
	{
		`log("### ApplyWeaponMesh() - (WeaponClass == None) ###");
		return ;
	}

	// 미리 들고 있다면?
	if ( WeaponClass.default.PickupSound != None )
		SoundCues[SoundCues.Length] = WeaponClass.default.PickupSound;
	if ( WeaponClass.default.WeaponFireSnd != None )
		SoundCues[SoundCues.Length] = WeaponClass.default.WeaponFireSnd;

	// 게임에서 사용되는 메쉬 추가.(avaUICharacter에 없음)
	AddObject(WeaponClass.default.EUArmMeshName, class'SkeletalMesh');
	AddObject(WeaponClass.default.NRFArmMeshName, class'SkeletalMesh');
	AddObject(WeaponClass.default.DefaultHandMeshName, class'SkeletalMesh');

	// 게임에서 사용되는 메쉬 추가.(이건 또 모지??)
	if ( WeaponClass.default.BaseSkelMeshName != "" )
	{
		AddObject(WeaponClass.default.BaseSkelMeshName, class'SkeletalMesh');

		// 이 속에 SoundNodeWave가 생성될 줄이야... @_@a
		AddObject(WeaponClass.default.BaseAnimSetName, class'AnimSet');
	}

	GunClass1p = class<avaWeap_BaseGun>(WeaponClass);

	// 1인칭용 소음기.(나 아닌 다른 유저인데 이걸 생성하는 걸 보면... 곧 지원할 예정?)
	// (폭탄, 칼 등은 avaWeap_BaseGun를 상속받지 않는다)
	if ( GunClass1p != None )
	{
		if ( GunClass1p.default.SilencerMeshName != "" )
			AddObject(GunClass1p.default.SilencerMeshName, class'StaticMesh');

		if ( GunClass1p.default.ScopeMeshName != "" )
			AddObject(GunClass1p.default.ScopeMeshName, class'SkeletalMesh');
	}

	// 3인칭용 무기 클래스.	
	WeaponClass3p = WeaponClass.default.AttachmentClass;

	// 무기 아이템만 생성해야 하는 경우가 있다.
	if ( WeaponClass3p.default.bMeshIsSkeletal )
	{
		AddObject(WeaponClass3p.default.MeshName$"_UI", class'SkeletalMesh');

		AddObject(WeaponClass3p.default.MeshName, class'SkeletalMesh');
		AddObject(WeaponClass3p.default.SocMeshName, class'SkeletalMesh');
		AddObject(WeaponClass3p.default.BasicMeshName, class'StaticMesh');
	}
	else
	{
		AddObject(WeaponClass3p.default.MeshName, class'StaticMesh');
	}

	// 3인칭 소음기.
	GunClass3p = class<avaAttachment_BaseGun>(WeaponClass3p);
	if ( GunClass3p != None )
	{
		if ( GunClass3p.default.SilencerMeshName != "" )
			AddObject(GunClass3p.default.SilencerMeshName, class'StaticMesh');
	}

	if ( WeaponSocMesh != None )
	{
		// 무기 장착 아이템 생성.
		for( i = 0; i < WeaponModifiers.Length; ++ i )
		{
			WeaponItems = WeaponModifiers[i].default.CommonAttachedItems;

			// 장착 아이템을 붙여준다.
			for( j = 0; j < WeaponItems.Length; ++ j )
				AddObject(WeaponItems[j].MeshName$"_3p", class'StaticMesh');
		}
	}
}

simulated function RemoveCharacterMesh( ECharSlot Slot )
{
}

simulated function RemoveWeaponMesh()
{
	WeaponItemParts.Length = 0;
}

simulated function RemoveAllMeshes()
{
	HeadMeshName = "";
	HeadMTSName = "";
	EyeBallMeshName = "";
	HelmetMeshName = "";

	BodyParts.Length = 0;
	WeaponItemParts.Length = 0;
	ItemParts.Length = 0;
	HelmetAccessory.Length = 0;

	Modifiers.Length = 0;
	WeaponModifiers.Length = 0;
}

simulated function CollectAllModifiers(optional bool bServer = false, 
									   optional bool bCharacter = true, 
									   optional bool bWeapon = true)
{
	local int i, j;

	// 만약 캐릭터 클래스들이 없다면.
	if ( CharClasses.Length == 0 )
		InitCharacterClasses();


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

	
	// avaWeapon에서 class<avaModifier>자료형의 정적배열 복사...
	WeaponModifiers.Length = 0;

	// 캐릭터의 모든 무기를 얻어온다.
	for ( i = 0; i < CharClass.default.DefaultWeapons.Length; ++i)
	{
		WeaponClass = CharClass.default.DefaultWeapons[i];

		for ( j = 0; j < WeaponClass.default.DefaultModifiers.Length; ++ j )
		{
			if ( WeaponClass.default.DefaultModifiers[j] == None )
				continue;

			WeaponModifiers[WeaponModifiers.Length] = class<avaMod_Weapon>(WeaponClass.default.DefaultModifiers[j]);
		}
	}
}

simulated function ApplyAllModifiersEx(optional bool bServer = false, optional int head = 0)
{
	local int i;

	// 모든 등록정보 제거.
	RemoveAllMeshes();


	// 모든 Modifiers정보를 얻어온다.
	CollectAllModifiers(bServer);

	// Modifiers에서 메쉬정보를 얻어온다.
	for ( i = 0; i < Modifiers.Length; ++ i )
		CollectCharacterMesh( Modifiers[i] );


	// 모든 메쉬 정보를 적용한다.
	ApplyAllMeshes();



	// 모든 등록정보 제거.
	RemoveAllMeshes();

	// 머리 Modifier추가.
	// (내부적으로 Modifier개당 head정보를 1개만 가지고 있어서 이런 편법을 씀... )
	AddHeadModifiers(head);

	// Modifiers에서 메쉬정보를 얻어온다.
	for ( i = 0; i < Modifiers.Length; ++ i )
		CollectCharacterMesh( Modifiers[i] );

	// 모든 메쉬 정보를 적용한다.
	ApplyAllMeshes();


	// CB용으로 기존의 정보에 Overwrite되어서 메쉬가 2번 읽어져야 하더라.

	// 모든 등록정보 제거.
	RemoveAllMeshes();

	// Modifiers에서 메쉬정보를 얻어온다.
	for ( i = 0; i < Modifiers.Length; ++ i )
		CollectCharacterMesh( Modifiers[i] );

	// 모든 메쉬 정보를 적용한다.
	ApplyAllMeshes();
}

//! 머리 Modifier추가.
simulated function AddHeadModifiers(int index)
{
	// DefaultProperties에서는 함수 호출이 안되는데... 왜 컴파일 에러가 안난거지....;;
	if ( HeadMods.Length == 0 )
	{
		HeadMods[HeadMods.Length] = class<avaCharacterModifier>(DynamicLoadObject( "avaRules.avaMod_CharHeadA", class'class' ));
		HeadMods[HeadMods.Length] = class<avaCharacterModifier>(DynamicLoadObject( "avaRules.avaMod_CharHeadB", class'class' ));
		HeadMods[HeadMods.Length] = class<avaCharacterModifier>(DynamicLoadObject( "avaRules.avaMod_CharHeadC", class'class' ));
		HeadMods[HeadMods.Length] = class<avaCharacterModifier>(DynamicLoadObject( "avaRules.avaMod_CharHeadD", class'class' ));
		HeadMods[HeadMods.Length] = class<avaCharacterModifier>(DynamicLoadObject( "avaRules.avaMod_CharHeadE", class'class' ));
	}

	if ( index < HeadMods.Length )
		Modifiers[Modifiers.Length] = HeadMods[index];
	else
		Modifiers[Modifiers.Length] = HeadMods[0];
}

simulated function ChangeWeapon( class<avaMod_Weapon> Mod )
{
	if ( Mod != None )
		ChangeWeaponClass( Mod.default.WeaponClass );
}

simulated function ChangeWeaponClass( class<avaWeapon> WeapClass )
{
	local int i;

	if ( WeapClass == None )
		return ;

	RemoveWeaponMesh();

	// 주무기 Modifier에서 무기 클래스를 얻어서.
	WeaponClass = WeapClass;

	// avaWeapon에서 class<avaModifier>자료형의 정적배열 복사...
	// (장착 Modifier들을 복사한다)
	WeaponModifiers.Length = 0;
	for ( i = 0; i < WeaponClass.default.DefaultModifiers.Length; ++ i )
	{
		if ( WeaponClass.default.DefaultModifiers[i] == None )
			continue;

		WeaponModifiers[WeaponModifiers.Length] = class<avaMod_Weapon>(WeaponClass.default.DefaultModifiers[i]);
	}

	// 무기 적용하고
	ApplyWeaponMesh();
}

/*! @brief 캐릭터의 기본적인 Mesh & Texture 로딩.
	@note
		전부 하면 64MB, 기본무기만 하면 50MB정도 차지함.
*/
function LoadObjects()
{
	`log("### Start the preload");
/*
	for ( Team = 0; Team < 2; ++Team )
		for ( ClassType = 0; ClassType < 3; ++ClassType )
		{
			TeamID    = ETeamType(Team);
			TypeID    = ClassType;
			CharClass = None;

			// 서버정보가 아닌 기본 값을 사용.
			ApplyAllModifiersEx(false, ClassType + Team * 3);
		}
*/
	ChangeWeapon( class<avaMod_Weapon>(DynamicLoadObject( "avaRules.avaMod_MP5", class'class' )) );
	ChangeWeapon( class<avaMod_Weapon>(DynamicLoadObject( "avaRules.avaMod_MP5K", class'class' )) );
	ChangeWeapon( class<avaMod_Weapon>(DynamicLoadObject( "avaRules.avaMod_MiniUzi", class'class' )) );
//	ChangeWeapon( class<avaMod_Weapon>(DynamicLoadObject( "avaRules.avaMod_BizonPP19", class'class' )) );

	ChangeWeapon( class<avaMod_Weapon>(DynamicLoadObject( "avaRules.avaMod_G36", class'class' )) );
	ChangeWeapon( class<avaMod_Weapon>(DynamicLoadObject( "avaRules.avaMod_G36Rail", class'class' )) );
	ChangeWeapon( class<avaMod_Weapon>(DynamicLoadObject( "avaRules.avaMod_AK47", class'class' )) );
//	ChangeWeapon( class<avaMod_Weapon>(DynamicLoadObject( "avaRules.avaMod_M16A4", class'class' )) );
//	ChangeWeapon( class<avaMod_Weapon>(DynamicLoadObject( "avaRules.avaMod_M4A1", class'class' )) );

	ChangeWeapon( class<avaMod_Weapon>(DynamicLoadObject( "avaRules.avaMod_M24", class'class' )) );
	ChangeWeapon( class<avaMod_Weapon>(DynamicLoadObject( "avaRules.avaMod_GalilSniper", class'class' )) );
//	ChangeWeapon( class<avaMod_Weapon>(DynamicLoadObject( "avaRules.avaMod_TPG1", class'class' )) );
	ChangeWeapon( class<avaMod_Weapon>(DynamicLoadObject( "avaRules.avaMod_SV98", class'class' )) );
//	ChangeWeapon( class<avaMod_Weapon>(DynamicLoadObject( "avaRules.avaMod_MSG90A1", class'class' )) );

	// 보조무기도?
	ChangeWeapon( class<avaMod_Weapon>(DynamicLoadObject( "avaRules.avaMod_Glock", class'class' )) );
	ChangeWeapon( class<avaMod_Weapon>(DynamicLoadObject( "avaRules.avaMod_P226", class'class' )) );
	ChangeWeapon( class<avaMod_Weapon>(DynamicLoadObject( "avaRules.avaMod_Knife", class'class' )) );
	ChangeWeapon( class<avaMod_Weapon>(DynamicLoadObject( "avaRules.avaMod_Grenade", class'class' )) );
	ChangeWeapon( class<avaMod_Weapon>(DynamicLoadObject( "avaRules.avaMod_SmokeBomb", class'class' )) );

	// 망원경.
//	ChangeWeaponClass( class<avaWeapon>(DynamicLoadObject( "avaRules.avaWeap_Binocular", class'class' )) );
	ChangeWeaponClass( class<avaWeapon>(DynamicLoadObject( "avaGame.avaWeap_Binocular", class'class' )) );

	// DogTag
	AddObject(class'avaPickupDogTag'.default.EUDogTagName, class'StaticMesh');
	AddObject(class'avaPickupDogTag'.default.NRFDogTagName, class'StaticMesh');
/*
	if ( class'avaAttachment_BaseGun'.default.MuzzleFlashPSCTemplate != None )
	{
		for ( i = 0; i < class'avaAttachment_BaseGun'.default.MuzzleFlashPSCTemplate.Emitters.Length; ++i )
		{
			SprEmitter = ParticleSpriteEmitter(class'avaAttachment_BaseGun'.default.MuzzleFlashPSCTemplate.Emitters[i]);
			if ( SprEmitter != None )
			{
				class'avaCache'.static.GetInstance().UpdateMaterial(SprEmitter.Material);
			}
		}

		`log("### avaAttachment_BaseGun : MuzzleFlashPSCTemplate != None");
	}
*/
	`log("### End the preload");
}

defaultproperties
{
	bHidden = true
}