class avaUICharacterPIP extends UIPIP native dependson(avaWeapon);

`include(avaGame/avaGame.uci)

var() class<avaCharacter>					Template;
var() class<avaWeaponAttachment>			WeaponTemplate;

var transient SkeletalMeshComponent			Mesh;				// 기본 뼈대 입니다.
var transient array<SkeletalMeshComponent>	BodyMeshes;			// 기본 뼈대에 붙는 Extar Mesh 들 입니다.
var transient array<StaticMeshComponent>	ItemMeshes;			// 기본 뼈대의 Socket 에 붙는 Static Mesh 들 입니다.
var transient SkeletalMeshComponent			HelmetMesh;			// Helmet Mesh 입니다. Item 을 붙이기 위해서 SMC 로 만들었습니다.
var transient array<StaticMeshComponent>	HelmetItems;		// Helmet 에 붙는 Item 입니다. Night Vision 등등등...
var transient SkeletalMeshComponent			HeadMesh;			// Head용 Mesh 입니다. Morph Target 을 이용하기 위해서 기존의 Extra Mesh 와는 다르게 별도로 관리합니다.
var transient MorphNodeWeight				HeadMN_Eye;			// Head용 Morph Node 입니다. Weight를 조절함으로써 눈을 깜빡입니다. MorphNode 가 많아지면 별도의 Class 로 관리를 해야 할 것 같습니다.

var transient MeshComponent					WeaponMesh;			// Weapon 의 위치를 잡아주기 위한 기본 Mesh 입니다. SocMesh 가 붙습니다.
var transient SkeletalMeshComponent			WeaponSocMesh;		// Weapon 에 기본 Mesh 와 Item 을 붙이기 위한 SMC 입니다.
var transient StaticMeshComponent			WeaponBasicMesh;	// Weapon 의 가장 기본이 되는 Mesh 입니다.
var transient array<StaticMeshComponent>	WeaponItems;		// Weapon 에 붙는 Item 들입니다.

var transient string						HelmetMeshName;		// Helmet용 Mesh 이름입니다.
var transient array<ItemPart>				HelmetAccessory;	// Helmet 에 붙는 Accessory들의 이름과 붙는 Socket 정보를 담고 있는 Array입니다.
var transient string						HeadMeshName;		// Head 용 Mesh 이름입니다.
var transient string						HeadMTSName;		// Head 용 Morph Target Set 이름입니다. Head Mesh 별로 따로 존재합니다.


function OnApplyModifier( avaUIAction_ApplyModifier action )
{
	//local avaPlayerReplicationInfo	avaPRI;
	//local avaPlayerController		avaPC;
	//local WorldInfo					WorldInfo;
	//local int						i;
	//if ( GetScene() == None )						return;

	//WorldInfo = GetScene().GetWorldInfo();
	//foreach WorldInfo.LocalPlayerControllers(avaPC)
	//{
	//	avaPRI = avaPlayerReplicationInfo( avaPC.PlayerReplicationInfo );
	//	for ( i = 0 ; i < action.Modifiers.length ; ++ i )
	//	{
	//		avaPRI.AddCharacterModifierByClass( action.Modifiers[i] );
	//	}
	//}
	Refresh();
}

function OnSetCharacter( avaUIAction_SetCharacter action )
{
	Template = action.Template;

	Refresh();
}

function OnSetWeapon( avaUIAction_SetWeapon action )
{
	WeaponTemplate = action.Template;

	Refresh();
}

function SetupScene()
{
	UpdateMeshes();

	Super.SetupScene();
}

function SkeletalMeshComponent CreateSkeletalMesh( string ThisMeshName  )
{
	local SkeletalMeshComponent NewMesh;
	local SkeletalMesh tmpSkelMesh;	
	
	NewMesh = new(self) class'avaSkeletalMeshComponent';		
	NewMesh.bUseAsOccluder = FALSE;
	if ( ThisMeshName != "" )	
	{
		tmpSkelMesh = SkeletalMesh( DynamicLoadObject( ThisMeshName, class'SkeletalMesh' ) );
		if ( tmpSkelMesh != None )	NewMesh.SetSkeletalMesh( tmpSkelMesh );
		else						`warn( "Load SkeletalMesh" @ThisMeshName );
	}
	else							`warn( "Not Initialize Mesh" );
	NewMesh.bUpdateSkelWhenNotRendered=false;
	NewMesh.bCastDynamicShadow=true;
	NewMesh.CastShadow  = true;
	NewMesh.ForcedLodModel = 1;

	NewMesh.SetShadowParent( Mesh );

	return NewMesh;
}

function StaticMeshComponent CreateStaticMesh( string ThisMeshName )
{
	local StaticMeshComponent NewMesh;
	local StaticMesh tmpStaticMesh;

	NewMesh = new(self) class'StaticMeshComponent';	
	NewMesh.bUseAsOccluder = FALSE;
	if ( ThisMeshName != "" )	
	{
		tmpStaticMesh = StaticMesh( DynamicLoadObject( ThisMeshName, class'StaticMesh' ) );
		if ( tmpStaticMesh != None )	NewMesh.SetStaticMesh( tmpStaticMesh );
		else							`warn( "Cannot Load StaticMesh" @ThisMeshName );
	}
	else								`warn( "Not Initialize Mesh" );
	NewMesh.bCastDynamicShadow=true;
	NewMesh.SetShadowParent( Mesh );
	NewMesh.ForcedLodModel = 1;

	return NewMesh;
}

// Add ExtraMesh To Character
function AddExtraMesh( string MeshName )
{
	local SkeletalMeshComponent NewSkelComp;
	NewSkelComp = CreateSkeletalMesh( MeshName );
	if (NewSkelComp  != None )
	{				
		NewSkelComp.SetParentAnimComponent( Mesh );				
		Mesh.AttachComponent( NewSkelComp, 'Bip01' );
		BodyMeshes[BodyMeshes.Length] = NewSkelComp;
	}
}

// Add Item To Character
function AddItemMesh( string MeshName, name SocketName )
{
	local StaticMeshComponent	NewStaticComp;
	NewStaticComp = CreateStaticMesh( MeshName );
	if (NewStaticComp != None)
	{				
		Mesh.AttachComponentToSocket( NewStaticComp, SocketName  );
		ItemMeshes[ItemMeshes.Length] = NewStaticComp;
	}
}

function ApplyCharacterModifier( class<avaCharacterModifier> Mod )
{
	local int i;
	local array< ExtraMesh >	ExtraMeshes;
	local array< AttachedItem >	AttachedItems;
	local ExtraMesh				HMesh;
	local ItemPart				HelmetItem;
	

	if ( Mod == None )	return;
	for ( i = 0 ; i < Mod.default.CommonExtraMeshes.length ; ++ i )
		AddExtraMesh( Mod.default.CommonExtraMeshes[i].MeshName );

	ExtraMeshes = Mod.static.GetExtraMeshes( Template.default.DefaultTeam, Template.default.TypeID );
	for ( i = 0 ; i < ExtraMeshes.length ; ++ i )
		AddExtraMesh( ExtraMeshes[i].MeshName );

	for ( i = 0 ; i < Mod.default.CommonAttachedItems.length ; ++ i )
		AddItemMesh( Mod.default.CommonAttachedItems[i].MeshName, Mod.default.CommonAttachedItems[i].PrimarySocket );
	
	AttachedItems = Mod.static.GetAttachedItems( Template.default.DefaultTeam, Template.default.TypeID );
	for ( i = 0 ; i < AttachedItems.length ; ++ i )
		AddItemMesh( AttachedItems[i].MeshName, AttachedItems[i].PrimarySocket );

	HMesh = Mod.static.GetHelmetMesh( Template.default.DefaultTeam, Template.default.TypeID );
	if ( HMesh.MeshName != "" )		HelmetMeshName = HMesh.MeshName;

	AttachedItems = Mod.static.GetHelmetAttachedItems( Template.default.DefaultTeam, Template.default.TypeID );
	for ( i = 0 ; i < AttachedItems.length ; ++ i )
	{
		HelmetItem.MeshName		=	AttachedItems[i].MeshName; 
		HelmetItem.SocketName	=	AttachedItems[i].PrimarySocket;
		HelmetAccessory[ HelmetAccessory.length ] = HelmetItem;
	}

	// HeadMesh 와 Head Morph Target Set 이름을 가지고 옵니다...
	if ( Mod.default.HeadMeshName != "" )	HeadMeshName	= Mod.default.HeadMeshName;
	if ( Mod.default.HeadMTSName != "" )	HeadMTSName		= Mod.default.HeadMTSName; 
}

// 수정해야 한다. 일반적인 PMI 가 적용될 수 있도록...
// PlayerModifierInfo 를 Setting 할 수 있도록??
function ApplyCharacterModifiers()
{
	local int								i,TypeID;
	local avaPlayerReplicationInfo			avaPRI;
	local PlayerController					PC;
	local WorldInfo							WorldInfo;

	TypeID = Template.default.TypeID;

	if ( TypeID >= 0 && TypeID < `MAX_PLAYER_CLASS )
	{
		WorldInfo = GetScene().GetWorldInfo();
		foreach WorldInfo.LocalPlayerControllers(PC)
		{
			avaPRI = avaPlayerReplicationInfo( PC.PlayerReplicationInfo );

			if ( avaPRI != None && avaPRI.avaPMI != None && avaPRI.avaPMI.ClassTypeInfos[TypeID].CharMod.length > 0 )
			{
				for ( i = 0 ; i < avaPRI.avaPMI.ClassTypeInfos[TypeID].CharMod.length ; ++ i )
					ApplyCharacterModifier( avaPRI.avaPMI.ClassTypeInfos[TypeID].CharMod[i] );
				return;
			}
		}
	}

	for ( i = 0 ; i < Template.default.DefaultModifier.length ; ++ i )
		ApplyCharacterModifier( Template.default.DefaultModifier[i] );

}

event UpdateCharacter()
{
	local SkeletalMeshComponent NewSkelComp;
	local StaticMeshComponent NewStaticComp;
	local int i;
	local BodyPart BodyPart;
	local ItemPart ItemPart;	
	local SkelControlLookAt		LookAtControl;
	local MorphTargetSet		NewMorphTargetSet;

	if (HelmetMesh != None)
	{
		for (i=0; i<HelmetItems.Length; ++i)
		{
			HelmetMesh.DetachComponent( HelmetItems[i] );
		}

		HelmetItems.Length = 0;
		
		if (Mesh != None)
		{
			Mesh.DetachComponent( HelmetMesh );
		}

		HelmetMesh = None;
	}	

	if ( HeadMesh != None )
	{
		if ( Mesh != None )
			Mesh.DetachComponent( HeadMesh );
		HeadMesh = None;
	}

	for (i=0; i<BodyMeshes.Length; ++i)
	{
		Mesh.DetachComponent( BodyMeshes[i] );		
	}

	for (i=0; i<ItemMeshes.Length; ++i)
	{
		Mesh.DetachComponent( ItemMeshes[i] );		
	}

	BodyMeshes.Length		= 0;
	ItemMeshes.Length		= 0;	

	HelmetMeshName			= "";
	HelmetAccessory.length	= 0;

	HeadMeshName			= "";
	HeadMTSName				= "";

	`log( "avaUICharacterPIP.UpdateCharacter" @Template );

	if (Template == none) return;


//	if ( GetOwnerScene().PlayerOwner.Actor == None )	return;

	Mesh.SetSkeletalMesh( Template.default.BaseSkeletalMesh );

	ApplyCharacterModifiers();

	if ( HeadMeshName != "" )
	{
		HeadMesh = CreateSkeletalMesh( HeadMeshName );
		HeadMesh.SetAnimTreeTemplate( AnimTree(DynamicLoadObject( "'avaCharCommon.Head'", class'AnimTree') ) );
		HeadMesh.SetParentAnimComponent( Mesh );
		Mesh.AttachComponent( HeadMesh, 'Bip01' );

		if ( HeadMTSName != "" )
		{
			NewMorphTargetSet = MorphTargetSet( DynamicLoadObject( HeadMTSName, class'MorphTargetSet' ) );	
			if ( NewMorphTargetSet != None )
			{
				HeadMesh.MorphSets.length = 1;
				HeadMesh.MorphSets[0]	  = NewMorphTargetSet;
				HeadMN_Eye = MorphNodeWeight( HeadMesh.FindMorphNode('CloseEye') );
				//if ( HeadMN_Eye != None )	OpenYourEyes();
			}
		}
	}

	// Set Body Part
	for ( i = 0 ; i < Template.default.BodyParts.Length ; ++ i )
	{
		BodyPart = Template.default.BodyParts[i];

		if ( BodyPart.MeshName != "" )
		{	
			NewSkelComp = CreateSkeletalMesh( BodyPart.MeshName );			
			if (NewSkelComp  != None )
			{				
				NewSkelComp.SetParentAnimComponent( Mesh );				

				Mesh.AttachComponent( NewSkelComp, 'Bip01' );

				BodyMeshes[BodyMeshes.Length] = NewSkelComp;				
			}
			else
			{
				`warn( "avaCharacter.ChangeMesh Cannot Load SkeletalMesh" @BodyPart.MeshName );
			}						
		}
	}

	// Set Item Part
	for ( i = 0 ; i < Template.default.ItemParts.Length ; ++ i )
	{
		ItemPart = Template.default.ItemParts[i];

		if ( ItemPart.MeshName != "" )
		{
			NewStaticComp = CreateStaticMesh( ItemPart.MeshName );

			if (NewStaticComp != None)
			{				
				Mesh.AttachComponentToSocket( NewStaticComp, ItemPart.SocketName );

				ItemMeshes[ItemMeshes.Length] = NewStaticComp;
			}
			else
			{
				`warn( "avaCharacter.AttachItems Cannot Load StaticMesh" @ItemPart.MeshName );
			}		
		}
	}

	if ( HelmetMeshName != "")
	{
		HelmetMesh = CreateSkeletalMesh( HelmetMeshName );
		Mesh.AttachComponentToSocket( HelmetMesh, 'H1' );
		for (i=0; i< HelmetAccessory.Length; ++i)
		{
			NewStaticComp = CreateStaticMesh( HelmetAccessory[i].MeshName );
			if (NewStaticComp != None)
			{
				HelmetItems[HelmetItems.Length] = NewStaticComp;
				HelmetMesh.AttachComponentToSocket( NewStaticComp, HelmetAccessory[i].SocketName );
			}
		}
	}

	LookAtControl = SkelControlLookAt(mesh.FindSkelControl('HeadController'));
	LookAtControl.SetSkelControlActive( false );
}

// 기본 Component 를 만든다...
function CreateBasicComponent()
{
	local rotator		newRot;
	local SkeletalMeshComponent skelComp;

	if (WeaponTemplate.default.bMeshIsSkeletal)
	{		
		// Mesh 가 Skeletal 인 경우는 Carried 를 같이 붙이기 위해서 이다. 즉, SocMesh 와 BasicMesh 는 반드시 필요하다...
		skelComp = CreateSkeletalMesh( WeaponTemplate.default.MeshName );				
		WeaponMesh = skelComp;
		WeaponSocMesh = CreateSkeletalMesh( WeaponTemplate.default.SocMeshName );				
		WeaponSocMesh.bForceNotToDraw = true;
		//WeaponSocMesh.SetHidden(true);		
		WeaponBasicMesh = CreateStaticMesh( WeaponTemplate.default.BasicMeshName );					

		`log( "WeaponMesh = "@WeaponMesh@skelComp.SkeletalMesh@WeaponTemplate.default.MeshName );
		`log( "WeaponSocMesh= "@WeaponSocMesh@WeaponSocMesh.SkeletalMesh@WeaponTemplate.default.SocMeshName );
		`log( "WeaponBasicMesh = "@WeaponBasicMesh@WeaponBasicMesh.StaticMesh@WeaponTemplate.default.BasicMeshName );		
	}
	else
	{
		WeaponMesh = CreateStaticMesh( WeaponTemplate.default.MeshName );				
	}

	newRot.Roll = -16384;	
	WeaponMesh.SetRotation( newRot );

	// Skin 을 바꾸거나 Item 등을 Attach 해준다!!!
	//`log( "AttachComp" @ WeaponMesh @ WeaponTemplate @ WeaponTemplate.default.AttachmentBoneName );
	`log( "Attach WeaponMesh to "@WeaponTemplate.default.AttachmentBoneName );		
	Mesh.AttachComponent( WeaponMesh, WeaponTemplate.default.AttachmentBoneName );	
	if ( WeaponSocMesh != None )
	{
		`log( "Attach SocMesh to WeaponMesh"@WeaponTemplate.default.PosRootBoneName );		
		SkeletalMeshComponent( WeaponMesh ).AttachComponent( WeaponSocMesh, WeaponTemplate.default.PosRootBoneName );

		`log( "Attach WeaponBasicMesh to SocMesh "@WeaponTemplate.default.SocRootBoneName );		
		WeaponSocMesh.AttachComponent( WeaponBasicMesh, WeaponTemplate.default.SocRootBoneName );
	}
}

function ApplyWeaponModifier( class<avaMod_Weapon> Mod )
{
	local int i;
	local StaticMeshComponent	NewItem;
	if ( WeaponSocMesh == None )	return;
	for ( i = 0 ; i < Mod.default.CommonAttachedItems.length ; ++ i )
	{
		NewItem = CreateStaticMesh( Mod.default.CommonAttachedItems[i].MeshName$"_3p" );
		WeaponSocMesh.AttachComponentToSocket( NewItem, Mod.default.CommonAttachedItems[i].PrimarySocket );
		WeaponItems[WeaponItems.Length]	= NewItem;
	}
}

function ApplyWeaponModifiers()
{
	local int i;
	for ( i = 0 ; i < WeaponTemplate.default.WeaponClass.default.DefaultModifiers.length ; ++ i )
	{
		ApplyWeaponModifier( class<avaMod_Weapon>(WeaponTemplate.default.WeaponClass.default.DefaultModifiers[i]) );
	}
}

event UpdateWeapon()
{
	local avaAnimBlendByWeaponType		weaponTypeBlend;	// Weapon Type 에 따른 Blend 객체 %%% Node Name 은 WeaponTypeNode 이어야만 한다. 
	local int							i;

	if (WeaponMesh != None )
	{			
		if ( WeaponSocMesh != None )
		{
			for (i=0; i<WeaponItems.Length; ++i)
			{
				WeaponSocMesh.DetachComponent( WeaponItems[i] );
			}

			WeaponItems.Length = 0;

			SkeletalMeshComponent( WeaponMesh ).DetachComponent( WeaponSocMesh );			
		}

		if (Mesh != None )
		{
			Mesh.DetachComponent( WeaponMesh );
		}

		WeaponSocMesh = None;
		WeaponMesh	= None;		
	}

	if (WeaponTemplate == none) return;

	CreateBasicComponent();	

	ApplyWeaponModifiers();

	weaponTypeBlend = avaAnimBlendByWeaponType( Mesh.Animations.FindAnimNode( 'WeaponTypeNode' ) );
	if ( weaponTypeBlend != None )	
	{
		if ( WeaponTemplate.default.AnimPrefix != '' )
			weaponTypeBlend.InitAnimSequence( WeaponTemplate.default.AnimPrefix );
	}
	else
		`warn( "Could not find weaponTypeBlend" );	
}

event UpdateMeshes()
{
	UpdateCharacter();
	UpdateWeapon();
}

cpptext
{
	/* === UObject interface === */
	/**
	* Called when a property value from a member struct or array has been changed in the editor.
	*/
	virtual void PostEditChange( FEditPropertyChain& PropertyThatChanged );

	/**
	* Called when a member property value has been changed in the editor.
	*/
	virtual void PostEditChange( UProperty* PropertyThatChanged );

	virtual void Initialize( UUIScene* inOwnerScene, UUIObject* inOwner );
}

function InitComponents()
{	
	Super.InitComponents();
	
	Mesh = CreateSkeletalMesh( "avaCharCommon.EU_PointMan_Bone01" );
	Mesh.SetOwnerNoSee( true );
	Mesh.AnimSets[0] = AnimSet(DynamicLoadObject( "avaCharCommon.New_animset", class'AnimSet' ) );
	Mesh.SetAnimTreeTemplate( AnimTree(DynamicLoadObject( "avaCharCommon.AnimTree", class'AnimTree') ) );

	Components[Components.Length] = Mesh;
}

defaultproperties
{	
}

