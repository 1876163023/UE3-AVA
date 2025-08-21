/*
	ĳ���� ���ҽ��� �̸� �ε��صδ� Ŭ����.

	2007/04/13	����
*/
class avaUICharacterCache extends avaUICharacter;

`include(avaGame/avaGame.uci)


//! �̸� �Ӹ� ������ ������ �д�.
var array< class<avaCharacterModifier> >	HeadMods;

//! �ϴ� ���� �־� ����.
var array<SoundCue>							SoundCues;

//! avaCache�� �Լ��� ȣ������ �Լ�.(�����ϰ�....?)
function AddObject(string ObjName, class ObjClass)
{
	class'avaCache'.static.RegisterObject(ObjName, ObjClass);
}

function ApplyHeadMesh()
{
	// Set Head 
	AddObject(HeadMeshName, class'SkeletalMesh');

	// Set MorphTargetSet(�� ������)
	AddObject(HeadMTSName, class'MorphTargetSet');

	AddObject("avaCharCommon.Head", class'AnimTree');

	// Set EyeBall(������)
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

	// �̸� ��� �ִٸ�?
	if ( WeaponClass.default.PickupSound != None )
		SoundCues[SoundCues.Length] = WeaponClass.default.PickupSound;
	if ( WeaponClass.default.WeaponFireSnd != None )
		SoundCues[SoundCues.Length] = WeaponClass.default.WeaponFireSnd;

	// ���ӿ��� ���Ǵ� �޽� �߰�.(avaUICharacter�� ����)
	AddObject(WeaponClass.default.EUArmMeshName, class'SkeletalMesh');
	AddObject(WeaponClass.default.NRFArmMeshName, class'SkeletalMesh');
	AddObject(WeaponClass.default.DefaultHandMeshName, class'SkeletalMesh');

	// ���ӿ��� ���Ǵ� �޽� �߰�.(�̰� �� ����??)
	if ( WeaponClass.default.BaseSkelMeshName != "" )
	{
		AddObject(WeaponClass.default.BaseSkelMeshName, class'SkeletalMesh');

		// �� �ӿ� SoundNodeWave�� ������ ���̾�... @_@a
		AddObject(WeaponClass.default.BaseAnimSetName, class'AnimSet');
	}

	GunClass1p = class<avaWeap_BaseGun>(WeaponClass);

	// 1��Ī�� ������.(�� �ƴ� �ٸ� �����ε� �̰� �����ϴ� �� ����... �� ������ ����?)
	// (��ź, Į ���� avaWeap_BaseGun�� ��ӹ��� �ʴ´�)
	if ( GunClass1p != None )
	{
		if ( GunClass1p.default.SilencerMeshName != "" )
			AddObject(GunClass1p.default.SilencerMeshName, class'StaticMesh');

		if ( GunClass1p.default.ScopeMeshName != "" )
			AddObject(GunClass1p.default.ScopeMeshName, class'SkeletalMesh');
	}

	// 3��Ī�� ���� Ŭ����.	
	WeaponClass3p = WeaponClass.default.AttachmentClass;

	// ���� �����۸� �����ؾ� �ϴ� ��찡 �ִ�.
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

	// 3��Ī ������.
	GunClass3p = class<avaAttachment_BaseGun>(WeaponClass3p);
	if ( GunClass3p != None )
	{
		if ( GunClass3p.default.SilencerMeshName != "" )
			AddObject(GunClass3p.default.SilencerMeshName, class'StaticMesh');
	}

	if ( WeaponSocMesh != None )
	{
		// ���� ���� ������ ����.
		for( i = 0; i < WeaponModifiers.Length; ++ i )
		{
			WeaponItems = WeaponModifiers[i].default.CommonAttachedItems;

			// ���� �������� �ٿ��ش�.
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

	// ���� ĳ���� Ŭ�������� ���ٸ�.
	if ( CharClasses.Length == 0 )
		InitCharacterClasses();


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

	
	// avaWeapon���� class<avaModifier>�ڷ����� �����迭 ����...
	WeaponModifiers.Length = 0;

	// ĳ������ ��� ���⸦ ���´�.
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

	// ��� ������� ����.
	RemoveAllMeshes();


	// ��� Modifiers������ ���´�.
	CollectAllModifiers(bServer);

	// Modifiers���� �޽������� ���´�.
	for ( i = 0; i < Modifiers.Length; ++ i )
		CollectCharacterMesh( Modifiers[i] );


	// ��� �޽� ������ �����Ѵ�.
	ApplyAllMeshes();



	// ��� ������� ����.
	RemoveAllMeshes();

	// �Ӹ� Modifier�߰�.
	// (���������� Modifier���� head������ 1���� ������ �־ �̷� ����� ��... )
	AddHeadModifiers(head);

	// Modifiers���� �޽������� ���´�.
	for ( i = 0; i < Modifiers.Length; ++ i )
		CollectCharacterMesh( Modifiers[i] );

	// ��� �޽� ������ �����Ѵ�.
	ApplyAllMeshes();


	// CB������ ������ ������ Overwrite�Ǿ �޽��� 2�� �о����� �ϴ���.

	// ��� ������� ����.
	RemoveAllMeshes();

	// Modifiers���� �޽������� ���´�.
	for ( i = 0; i < Modifiers.Length; ++ i )
		CollectCharacterMesh( Modifiers[i] );

	// ��� �޽� ������ �����Ѵ�.
	ApplyAllMeshes();
}

//! �Ӹ� Modifier�߰�.
simulated function AddHeadModifiers(int index)
{
	// DefaultProperties������ �Լ� ȣ���� �ȵǴµ�... �� ������ ������ �ȳ�����....;;
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

	// �ֹ��� Modifier���� ���� Ŭ������ ��.
	WeaponClass = WeapClass;

	// avaWeapon���� class<avaModifier>�ڷ����� �����迭 ����...
	// (���� Modifier���� �����Ѵ�)
	WeaponModifiers.Length = 0;
	for ( i = 0; i < WeaponClass.default.DefaultModifiers.Length; ++ i )
	{
		if ( WeaponClass.default.DefaultModifiers[i] == None )
			continue;

		WeaponModifiers[WeaponModifiers.Length] = class<avaMod_Weapon>(WeaponClass.default.DefaultModifiers[i]);
	}

	// ���� �����ϰ�
	ApplyWeaponMesh();
}

/*! @brief ĳ������ �⺻���� Mesh & Texture �ε�.
	@note
		���� �ϸ� 64MB, �⺻���⸸ �ϸ� 50MB���� ������.
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

			// ���������� �ƴ� �⺻ ���� ���.
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

	// �������⵵?
	ChangeWeapon( class<avaMod_Weapon>(DynamicLoadObject( "avaRules.avaMod_Glock", class'class' )) );
	ChangeWeapon( class<avaMod_Weapon>(DynamicLoadObject( "avaRules.avaMod_P226", class'class' )) );
	ChangeWeapon( class<avaMod_Weapon>(DynamicLoadObject( "avaRules.avaMod_Knife", class'class' )) );
	ChangeWeapon( class<avaMod_Weapon>(DynamicLoadObject( "avaRules.avaMod_Grenade", class'class' )) );
	ChangeWeapon( class<avaMod_Weapon>(DynamicLoadObject( "avaRules.avaMod_SmokeBomb", class'class' )) );

	// ������.
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