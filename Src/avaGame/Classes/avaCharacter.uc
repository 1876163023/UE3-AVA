class avaCharacter extends avaPawn
	Placeable
	dependson(avaPhysicalMaterialProperty)
	dependson(avaModifier)
	dependson(avaCharacterModifier)
	;

`include(avaGame/avaGame.uci)

// Body를 구성하는 Part Struct
struct BodyPart
{
	var		name					Id;
	var		string					MeshName;
	var()	SkeletalMeshComponent	BodyMesh;
	var		float					MaxVisibleDistance;
	var		ECharSlot				Slot;		//!< 슬롯정보 추가(2007/01/22).
	var		int						Priority;
};

var() array<BodyPart>						BodyParts;
// Character 소지 Item 을 위한 Variable
var array<ItemPart>							ItemParts;
var avaSkelControl_Twist 					TwistControl;

//=========================================================================
//	Look At Controller (Pawn 의 시점이 나를 바라보도록...)
//=========================================================================
var bool									bEnableLookAt;			// Manual Look At
var bool									bActiveLookAtControl;	
//=========================================================================
// Extra Mesh Initialize Flag
var bool									bExtraMeshInit;	
//=========================================================================
var SkeletalMesh							BaseSkeletalMesh;		// 기본 Skeletal Mesh
//=========================================================================
//	Helmet 관련 Properties 
//=========================================================================
var() SkeletalMeshComponent					HeadComp;				// 머리 Component -> Morph Target을 이용하기 위해서...
var() int									HeadModPriority;
var() string								HeadMeshName;
var() string								HeadMTSName;
var() string								EyeBallMeshName;

var MorphNodeWeight							MN_CloseEye;			// Morph Node For Close Eyes
var avaHelmet								Helmet;					// 착용하고 있는 Helmet Actor
var string									HelmetMeshName;			// 착용하고 있는 Helmet Mesh 
var string									HelmetSkinName;			// Helmet Skin
var bool									EnableTakeOffHelmet;	// Helmet 탈착 가능 여부, Helmet 의 Properties 에 들어갈 내용이다....
var array<ItemPart>							HelmetAccessory;		// Helmet 에 부착한 Accessory
var Name									PrvWeaponFiringMode;
var array< class< avaCharacterModifier> >	BaseModifier;			//  Network 과 상관없이 반드시 들고 나오는 Modifier 이다...
var	array< class< avaCharacterModifier> >	DefaultModifier;		//  Server 에서 받아오는 Modifier 정보가 없을 때 적용한다...

var	string									CharacterIcon;

var DynamicLightEnvironmentComponent		LightEnvironment;

var() Vector								ThirdPersonCamOffset;

//	Character Decal 관련 Properties...
var() SkeletalMeshComponent					DecalParentMesh;		// Decal Mesh
var MaterialInstanceConstant				DecalMIC;				// Decal Material Instance Constant
var avaTexture2DComposite					DecalTexture;			// Decal For Diffuse
var avaTexture2DComposite					DecalNormal;			// Decal For Normal
var avaTexture2DComposite					DecalSpec;				// Decal For Specular

var globalconfig	string					TeamDecal[2];
var globalconfig	string					EUClassDecal[3];
var globalconfig	string					NRFClassDecal[3];
var globalconfig	string					RankDecal[19];
/**************************************************************************************************
	Initialize Functions
**************************************************************************************************/
simulated function InitializeSkelControl()
{
	// Local Player Controller 를 찾는다.
	FindLocalController();
    InitializeTwist();	
	LookAtControl	= SkelControlLookAt(mesh.FindSkelControl('HeadController'));
	LeftEyeControl	= SkelControlLookAt(EyeBallComp.FindSkelControl('LeftEyeController'));
	RightEyeControl = SkelControlLookAt(EyeBallComp.FindSkelControl('RightEyeController'));
	if ( bEnableLookAt == false )
		EnableLookAtController(  false );

	if ( LookAtControl == none || LocalPC == None )	return;
	if ( LocalPC == PlayerController(Controller) || LODBias >= 1 )	
		EnableAutoLookAt( false );
}

simulated function FindLocalController()
{
	local PlayerController	PC;
	if ( LocalPC != None )	return;
	ForEach LocalPlayerControllers(PC)
	{
		LocalPC = PC;
		break;
	}
}

simulated function InitializeTwist()
{
    TwistControl = avaSkelControl_Twist(mesh.FindSkelControl('TwistControl'));
}

/**************************************************************************************************
	Initialize Mesh Component
**************************************************************************************************/

function PossessedBy(Controller C, bool bVehicleTransition)
{
	Super.PossessedBy(C, bVehicleTransition);
	// Team 이 바뀌었을 경우 PlayerReplicationInfo 의 Replicate event 에서  NotifyTeamChanged 를 호출해준다.
	// Host 인 경우에는 NotifyTeamChanged 를 호출해 주지 않기 때문에 Host 의 경우 여기서 호출해준다.
	NotifyTeamChanged();
}

simulated function NotifyTeamChanged()
{
	// Local Player Controller 를 찾는다.
	FindLocalController();
	ChangeMesh( DefaultTeam );
	Super.NotifyTeamChanged();
}


// 현재 Team 과 현재 Class
simulated function ChangeMesh( int nTeam )
{
	local int									i;
	local avaPlayerReplicationInfo				avaPRI;
	local array< class<avaCharacterModifier> >	CharacterModifiers;

	if ( bExtraMeshInit )	return;

	avaPRI = avaPlayerReplicationInfo( PlayerReplicationInfo );

	if ( avaPRI != None && avaPRI.avaPMI != None && avaPRI.avaPMI.ClassTypeInfos[TypeID].CharMod.length > 0 )
		CharacterModifiers	= avaPRI.avaPMI.ClassTypeInfos[TypeID].CharMod;
	else
		CharacterModifiers	= DefaultModifier;	

	for ( i = 0 ; i < BaseModifier.length ; ++ i )
		ApplyCharacterModifier( BaseModifier[i], nTeam );

	for ( i = 0 ; i < CharacterModifiers.length ; ++ i )
	{
		CharacterModifiers[i].static.ApplyToCharacter_Client( self );
		ApplyCharacterModifier( CharacterModifiers[i], nTeam );
	}

	// Set Base Skeletal Mesh
	Mesh.bForceNotToDraw = true;

	Mesh.SetSkeletalMesh( BaseSkeletalMesh );
	
	//<@ add cascaded shadow ; 2008. 1. 7 changmin
	Mesh.bCastHiddenShadow = (Controller != None && Controller.IsLocalPlayerController());
	//>@ changmin

	CreateHead();

	CreateEyeBall();

	CreateBodyPart();

	if ( !bNoDetailMeshes )
		AttachItems();

	if ( bHasHelmet == true )	// Host Migration 의 경우 bHasHelmet 을 Check 해서 Helmet 을 만들어 주어야 한다.
		CreateHelmet();

	CreateCharacterDecal( DefaultTeam, avaPRI.Level, avaPRI.GetClanMarkID() );

	InitializeSkelControl();

	bExtraMeshInit = true;
	
	if (nTeam == 0 || nTeam == 1)
	{
		MarkSeeThroughGroupIndex( nTeam + 1 );	
	}
	else
	{
		MarkSeeThroughGroupIndex( 0 );	
	}
}

simulated function InitComponent( PrimitiveComponent Comp )
{
	Comp.SetShadowParent( Mesh );
	Comp.SetOcclusionGroup( Mesh );
	Comp.SetLightEnvironment( LightEnvironment );
	Comp.SetOwnerNoSee( Mesh.bOwnerNoSee );
	Comp.CastShadow  = true;	
}

simulated function MarkSeeThroughGroupIndex( int SeeThroughGroupIndex )
{
	local int i;
	
	Super.MarkSeeThroughGroupIndex( SeeThroughGroupIndex );	
	
	if (Helmet != None)
	{
		Helmet.MarkSeeThroughGroupIndex( SeeThroughGroupIndex );
	}
	
	HeadComp.SetSeeThroughGroupIndex( SeeThroughGroupIndex );
	Mesh.SetSeeThroughGroupIndex( SeeThroughGroupIndex );
	EyeBallComp.SetSeeThroughGroupIndex( SeeThroughGroupIndex );
	
	for ( i = 0 ; i < BodyParts.Length ; ++ i )
	{
		if (BodyParts[i].BodyMesh != None)
		{
			BodyParts[i].BodyMesh.SetSeeThroughGroupIndex( SeeThroughGroupIndex );
		}
	}
	
	for ( i = 0 ; i < ItemParts.Length ; ++ i )
	{
		if (ItemParts[i].ItemMesh != None)
		{
			ItemParts[i].ItemMesh.SetSeeThroughGroupIndex( SeeThroughGroupIndex );
		}
	}
}

simulated function CreateHead()
{
	local SkeletalMesh		TempSkelMesh;
	local MorphTargetSet	TempMTS;
	// Set Head 
	TempSkelMesh			=	SkeletalMesh( DynamicLoadObject( HeadMeshName, class'SkeletalMesh' ) );
	HeadComp.LODBias		=	LODBias;
	HeadComp.SetSkeletalMesh( TempSkelMesh );
	if ( HeadMTSName != "" )
	{
		TempMTS		 = MorphTargetSet( DynamicLoadObject( HeadMTSName, class'MorphTargetSet' ) );	
		if ( TempMTS != None )
		{
			HeadComp.MorphSets.length = 1;
			HeadComp.MorphSets[0] = TempMTS;
		}
	}

	HeadComp.SetAnimTreeTemplate( AnimTree(DynamicLoadObject( "'avaCharCommon.Head'", class'AnimTree')) );
	InitComponent( HeadComp );
	HeadComp.SetParentAnimComponent( Mesh );
	HeadComp.bUpdateSkelWhenNotRendered = false;
	Mesh.AttachComponent( HeadComp, 'Bip01' );

	MN_CloseEye = MorphNodeWeight( HeadComp.FindMorphNode('CloseEye') );
	if ( MN_CloseEye != None )
	{
		OpenEyes();
		// 자기 자신을 제외한 3인칭 캐릭터들의 (Dynamic용)MorphVertexBuffer를 미리 생성한다.
		if ( LocalPC != Controller )
			MN_CloseEye.SetNodeWeight( 0.01 );
	}
	
	//<@ add cascaded shadow ; 2008. 1. 7 changmin
	HeadComp.bCastHiddenShadow = ( Controller != None && Controller.IsLocalPlayerController());
	//> changmin
}

simulated function CreateEyeball()
{
	local SkeletalMesh				TempSkelMesh;
	if ( EyeBallMeshName != "" )
	{
		TempSkelMesh = SkeletalMesh( DynamicLoadObject( EyeBallMeshName, class'SkeletalMesh' ) );
		EyeBallComp.SetSkeletalMesh( TempSkelMesh );
		EyeBallComp.SetShadowParent( Mesh );
		EyeBallComp.SetOcclusionGroup( Mesh );
		EyeBallComp.SetLightEnvironment( LightEnvironment );
		Mesh.AttachComponentToSocket( EyeBallComp, 'eye' );
		//<@ add cascaded shadow ; 2008. 1. 7 changmin
		EyeBallComp.bCastHiddenShadow = ( Controller != None && Controller.IsLocalPlayerController());
		//>@ changmin
	}
}

simulated function CreateBodyPart()
{
	local int			i;
	local SkeletalMesh	TempSkelMesh;
	// Set Body Part
	for ( i = 0 ; i < BodyParts.Length ; ++ i )
	{
		if ( BodyParts[i].MeshName != "" )
		{	
			// Create the Body mesh if needed
			if ( BodyParts[i].BodyMesh == None )
			{
				TempSkelMesh = SkeletalMesh( DynamicLoadObject(BodyParts[i].MeshName, class'SkeletalMesh' ) );
				if ( TempSkelMesh != None )
				{
					BodyParts[i].BodyMesh = new(outer) class'SkeletalMeshComponent';
					BodyParts[i].BodyMesh.bUseAsOccluder = FALSE;
					BodyParts[i].BodyMesh.LODBias		 = LODBias;
					BodyParts[i].BodyMesh.SetSkeletalMesh( TempSkelMesh );
				}
				else
				{
					`warn( "avaCharacter.dMesh Cannot Load SkeletalMesh" @BodyParts[i].MeshName );
				}
			}

			// Apply the Body Part
			if ( BodyParts[i].BodyMesh != None )
			{
				InitComponent( BodyParts[i].BodyMesh );
				BodyParts[i].BodyMesh.SetParentAnimComponent( Mesh );
				BodyParts[i].BodyMesh.bUpdateSkelWhenNotRendered = false;
				
				//<@ add cascaded shadow ; 2008. 1. 7 changmin
				BodyParts[i].BodyMesh.bCastHiddenShadow = (Controller != None && Controller.IsLocalPlayerController());
				//>@ changmin

				if ( BodyParts[i].Slot == CHAR_SLOT_MARK )
				{
					Assert( DecalParentMesh == None );
					DecalParentMesh = BodyParts[i].BodyMesh;
				}
			}

			if ( BodyParts[i].MaxVisibleDistance > 0.0 )
			{
				SetFadeOut( BodyParts[i].BodyMesh, BodyParts[i].MaxVisibleDistance );
			}

			// add it to the components array if it's not there already
			// Mesh 에 대하여 AttachComponent 를 하는 이유는 ChangeVisibility 시에 자동으로 해주기 위해서이다.
			if ( !BodyParts[i].BodyMesh.bAttached )
				Mesh.AttachComponent( BodyParts[i].BodyMesh,'Bip01');
		}
	}
}

simulated function CreateHelmet()
{
	local int i;
	// 여기 아래는 후에 수정이 필요할것 같다.
	if ( Helmet == None && HelmetMeshName != "" )
	{
		Helmet = Spawn(class'avaHelmet',);
		Helmet.Instigator = self;
		Helmet.SetMesh( HelmetMeshName );
		if ( HelmetSkinName != "" )	Helmet.ChangeSkin( HelmetSkinName );
		Helmet.AttachTo( Mesh, 'H1' );										// Helmet Socket 이름이 H1 이다...
		
		// Helmet Mesh 에 Night Vision 을 붙인다.
		if ( !bNoDetailMeshes )
		{
			for ( i = 0 ; i < HelmetAccessory.length ; ++ i )
				Helmet.AddAccessory( HelmetAccessory[i].MeshName, HelmetAccessory[i].SocketName, HelmetAccessory[i].MaxVisibleDistance );
			Helmet.AttachAccessory();
		}
		Helmet.SetLightEnvironment( LightEnvironment );
	}	
}

simulated function AttachItems()
{
	local int i;
	local StaticMesh	TempStaticMesh;

	// Set Item Part
	for ( i = 0 ; i < ItemParts.Length ; ++ i )
	{
		if ( ItemParts[i].MeshName != "" )
		{
			// Create the Item Mesh if needed
			if ( ItemParts[i].ItemMesh == None )
			{
				TempStaticMesh = StaticMesh( DynamicLoadObject(ItemParts[i].MeshName, class'StaticMesh') );

				if ( TempStaticMesh != None )
				{
					ItemParts[i].ItemMesh = new(outer) class'StaticMeshComponent';

					if ( ItemParts[i].MaxVisibleDistance > 0.0 )
						SetFadeOut( ItemParts[i].ItemMesh, ItemParts[i].MaxVisibleDistance );

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
				InitComponent( ItemParts[i].ItemMesh );
				
			//<@ add cascaded shadow ; 2008. 1. 7 changmin
			if( ItemParts[i].ItemMesh != None )
				ItemParts[i].ItemMesh.bCastHiddenShadow = (Controller != None && Controller.IsLocalPlayerController() );
			//>@ changmin

			// add it to the components array if it's not there already
			if ( !ItemParts[i].ItemMesh.bAttached )
				Mesh.AttachComponentToSocket( ItemParts[i].ItemMesh, ItemParts[i].SocketName );
		}
	}
}

simulated function SetFadeOut( PrimitiveComponent Comp, optional float MaxVisibleDistance = 300.0 )
{	
	Comp.bFadingOut = true;
	Comp.FadeStart	= MaxVisibleDistance;
	Comp.FadeEnd	= Comp.FadeStart;	
}

/**************************************************************************************************
	Function For Mesh Visibility
**************************************************************************************************/
simulated function SetMeshVisibility(bool bVisible)
{
	// bVisible 이 false 인 경우는 1인칭인 경우인데.... 1인칭 Spectator 가 없으므로 이런 일은 있을 수 없다...
	if ( bVisible == false )
	{
		if ( Controller == None || !Controller.IsLocalPlayerController() )
			return;
	}
	Super.SetMeshVisibility( bVisible );
	if ( Helmet != None )
		Helmet.ChangeVisibility( bForceHidden ? false : bVisible );
}

/**************************************************************************************************
	Twinkle Eyes
**************************************************************************************************/
simulated function OpenEyes()
{
	if ( MN_CloseEye == None )	return;
	MN_CloseEye.SetNodeWeight( 0.0 );
	SetTimer( ( FRand() + 0.1 ) * 5 , false, 'CloseEyes' );
}

simulated function CloseEyes()
{
	if ( MN_CloseEye == None )	return;
	MN_CloseEye.SetNodeWeight( 1.0 );
	SetTimer( 0.05, false, 'OpenEyes' );
}

simulated State Dying
{
	simulated function BeginState(Name PreviousStateName)
	{
		Super.BeginState( PreviousStateName );
		ClearTimer( 'OpenEyes' );
		ClearTimer( 'CloseEyes' );
		MN_CloseEye.SetNodeWeight( 1.0 );
		EyeBallComp.bNoSkeletonUpdate = 1;
	}
}

/**************************************************************************************************
	Adjust Modifier Functions
**************************************************************************************************/
simulated function ApplyCharacterModifier( class<avaCharacterModifier> Mod, int TeamNum )
{
	local int					i;
	local ExtraMesh				HelmetMesh;
	local array< ExtraMesh >	ExtraMeshes;
	local array< AttachedItem >	AttachedItems;

	if ( Mod == None )	return;
	
	for ( i = 0 ; i < Mod.default.CommonExtraMeshes.length ; ++ i )
		AddExtraMesh( Mod.default.CommonExtraMeshes[i].MeshName, Mod.default.Slot, Mod.default.Priority, Mod.default.CommonExtraMeshes[i].MaxVisibleDistance );

	ExtraMeshes = Mod.static.GetExtraMeshes( TeamNum, TypeID );
	for ( i = 0 ; i < ExtraMeshes.length ; ++i )
		AddExtraMesh( ExtraMeshes[i].MeshName, Mod.default.Slot, Mod.default.Priority, ExtraMeshes[i].MaxVisibleDistance );

	for ( i = 0 ; i < Mod.default.CommonAttachedItems.length ; ++ i )
		AddItemMesh( Mod.default.CommonAttachedItems[i].MeshName, Mod.default.CommonAttachedItems[i].PrimarySocket );

	AttachedItems = Mod.static.GetAttachedItems( TeamNum, TypeID );
	for ( i = 0 ; i < AttachedItems.length ; ++ i )
		AddItemMesh( AttachedItems[i].MeshName , AttachedItems[i].PrimarySocket );

	HelmetMesh = Mod.static.GetHelmetMesh( TeamNum, TypeID );
	if ( HelmetMesh.MeshName != "" )	SetHelmet( HelmetMesh.MeshName );

	AttachedItems = Mod.static.GetHelmetAttachedItems( TeamNum, TypeID );
	for ( i = 0 ; i < AttachedItems.length ; ++ i )
		AddHelmetAccessory( AttachedItems[i].MeshName , AttachedItems[i].PrimarySocket, AttachedItems[i].MaxVisibleDistance );

	AddHeadMesh( Mod );
}

simulated function AddHeadMesh( class<avaCharacterModifier> Mod )
{
	if ( HeadModPriority > Mod.default.Priority )	return;
	HeadModPriority	=	Mod.default.Priority;
	if ( Mod.default.HeadMeshName != "" )			HeadMeshName		= Mod.default.HeadMeshName;
	if ( Mod.default.HeadMTSName != "" )			HeadMTSName			= Mod.default.HeadMTSName;
	if ( Mod.default.EyeBallMeshName != "" )		EyeBallMeshName		= Mod.default.EyeBallMeshName;
}

simulated function AddItemMesh( string MeshName, name SocketName, optional float MaxVisibleDistance )
{
	local int id;
	id										= ItemParts.length;
	ItemParts.length						= id + 1;
	ItemParts[id].MeshName					= MeshName;
	ItemParts[id].SocketName				= SocketName;
	ItemParts[id].MaxVisibleDistance		= MaxVisibleDistance;	
}

simulated function AddExtraMesh( string MeshName, optional ECharSlot Slot, optional int Priority, optional float MaxVisibleDistance )
{
	local int	id;
	local int	index;
	index	=	BodyParts.Find( 'Slot', Slot );
	if ( index != INDEX_NONE )
	{
		if ( BodyParts[index].Priority > Priority )
			return;
	}
	id										= BodyParts.length;
	BodyParts.length						= id + 1;
	BodyParts[id].MeshName					= MeshName;
	BodyParts[id].MaxVisibleDistance		= MaxVisibleDistance;
	BodyParts[id].Slot						= Slot;
	BodyParts[id].Priority					= Priority;
}

simulated function SetHelmet( string MeshName, optional string SkinName )
{
	HelmetMeshName							= MeshName;
	HelmetSkinName							= SkinName;
}

simulated function AddHelmetAccessory( string MeshName, name SocketName, float MaxVisibleDistance )
{
	local int id;
	id										= HelmetAccessory.length;
	HelmetAccessory.length					= id + 1;
	HelmetAccessory[id].MeshName			= MeshName;
	HelmetAccessory[id].SocketName			= SocketName;
	HelmetAccessory[id].MaxVisibleDistance	= MaxVisibleDistance;
}

simulated function DetachItems()
{
	local int i;
	for ( i = 0 ; i < ItemParts.Length ; ++ i )
	{
		if ( ItemParts[i].ItemMesh != None )
		{
			Mesh.DetachComponent( ItemParts[i].ItemMesh );
			ItemParts[i].ItemMesh = None;
		}
	}
}

/**************************************************************************************************
	Helmet TakeOff Functions
**************************************************************************************************/
simulated function Client_TakeOffHelmet()
{
	if ( Helmet == None )	return;
	Helmet.TakeOff( Mesh, TOH_Momentum );	
}

simulated function TakeOffHelmet( vector Momentum )
{
	bHasHelmet		= false;
	// Helmet 이 벗겨질 수 있는 경우에만 Super 를 호출한다.
	if ( EnableTakeOffHelmet )	Super.TakeOffHelmet( Momentum );
}

function bool HitHelmet( avaPlayerReplicationInfo InstigatedByPRI, vector Momentum, int AbsorbDamage )
{
	if ( !bHasHelmet )	
		return false;
	if ( AbsorbDamage > Helmet_DamageThreshold && FRand() > HeadDefenceRate )
	{
		TakeOffHelmet( Momentum );
		if ( InstigatedByPRI != None )	InstigatedByPRI.AddHelmetDropCnt();
		if ( Health > 0 )				avaPlayerController(Controller).RaiseAutoMessage( AUTOMESSAGE_ImHit, true );
		return true;
	}
	return false;
}

/**************************************************************************************************
	Play Hit Effect Functions
**************************************************************************************************/
function PlayHitEx(float Damage, Controller InstigatedBy, vector HitLocation, class<DamageType> damageType, vector Momentum, TraceHitInfo HitInfo, bool bReducedByArmor, class<Weapon> weaponBy, EShotInfo shotInfo, int AbsorbDamage )
{
	local vector					angle;
	local bool						bTakeOffHelmet;
	local class<avaDamageType>		avaDamage;
	local float						damagedKickBackX, damagedKickBackZ;
	local avaPlayerReplicationInfo	InstigatedByPRI, InstigatorPRI;	
	local int						WeaponTypeID;

	super.PlayHit(Damage, InstigatedBy, HitLocation, DamageType, Momentum, HitInfo);

	if ( Damage <= 0 || Controller == None || Controller.bGodMode )	return;

	avaDamage		=	class<avaDamageType>(DamageType);
	bDamageHead		=	false;	
	angle			=	GetPunchAngle();
	InstigatedByPRI	=	InstigatedBy != None ? avaPlayerReplicationInfo( InstigatedBy.PlayerReplicationInfo ) : None;
	InstigatorPRI	=	Controller	 != None ? avaPlayerReplicationInfo( Controller.PlayerReplicationInfo ) : None;

	if ( shotInfo == SI_Head )
	{
		bDamageHead		=	true;
		LastTakeHitInfo.HitEffect = bHasHelmet ? HET_HelmetHit : HET_HeadShot;
		bTakeOffHelmet	=	HitHelmet( 	InstigatedByPRI, Momentum, AbsorbDamage );
	}
	else
	{
		LastTakeHitInfo.HitEffect = bReducedByArmor ? HET_KevlarHit : HET_Default;
	}

	damagedKickBackX = Clamp( Damage * DamagedKickBackInfo[shotInfo].DamageAmpX, DamagedKickBackInfo[shotInfo].MinAngleX, DamagedKickBackInfo[shotInfo].MaxAngleX );
	damagedKickBackZ = Clamp( Damage * DamagedKickBackInfo[shotInfo].DamageAmpZ, DamagedKickBackInfo[shotInfo].MinAngleZ, DamagedKickBackInfo[shotInfo].MaxAngleZ );
	angle.x +=	DamagedKickBackInfo[shotInfo].DirectionRandomX >= FRand() ? damagedKickBackX : -damagedKickBackX;
	angle.z +=	DamagedKickBackInfo[shotInfo].DirectionRandomZ >= FRand() ?	damagedKickBackZ : -damagedKickBackZ;

	if ( InstigatedByPRI != None )	
	{
		if ( class<avaWeapon>(weaponBy) != None )	WeaponTypeID	=	class<avaWeapon>(weaponBy).default.WeaponType;
		else										WeaponTypeID	=	-1;
		InstigatedByPRI.AddHitCount( WeaponTypeID, Damage, bDamageHead );
	}
	if ( InstigatorPRI != None )	InstigatorPRI.DamageTaken( Damage );

	if (InstigatedBy != none && avaGame(WorldInfo.Game).GameStats != none && InstigatedBy.Pawn != None)
	{
		avaGame(WorldInfo.Game).GameStats.HitEvent( InstigatedByPRI, InstigatorPRI, WeaponBy, shotInfo, 
													VSize(HitLocation-InstigatedBy.Pawn.Location), 
													Damage, Health <= 0, bTakeOffHelmet,
													InstigatedBy.Pawn.Location, HitLocation );
	}

	LastTakeHitInfo.bDamage			= Damage > 0 ? true : false;
	LastTakeHitInfo.HitLocation		= HitLocation;
	LastTakeHitInfo.Momentum		= Momentum;
	LastTakeHitInfo.DamageType		= DamageType;
	LastTakeHitInfo.HitBoneIndex	= Mesh.MatchRefBone( HitInfo.BoneName );
	LastTakeHitInfo.PunchAngle		= angle;			// Angle 에 1.5 를 곱하는 이유는 실제 Camera에 적용되는 값은 2/3 밖에 적용이 안되기 때문....
	LastTakeHitInfo.DamagedBy		= InstigatedByPRI != None ?	InstigatedByPRI.PlayerID : -1;
	LastTakeHitTimeout = WorldInfo.TimeSeconds + ( (avaDamage != None) ? avaDamage.static.GetHitEffectDuration(self, Damage)
																	   : class'avaDamageType'.static.GetHitEffectDuration(self, Damage) );
	// play clientside effects
	PlayTakeHitEffects( !InstigatedBy.IsLocalPlayerController() );
}


simulated function PlayTakeHitEffects( optional bool bCheckRelevant = true )
{
	local vector						BloodMomentum;	
	local class<avaDamageType>			avaDamage;	
	local EShotInfo						shotInfo;
	local int							bloodsize;
	local name							BoneName;
	local ParticleSystem				EmitterTemplate;
	/// Punch angle set
	SetPunchAngle( LastTakeHitInfo.PunchAngle );
	if ( bCheckRelevant && !EffectIsRelevant( LastTakeHitInfo.HitLocation, false, 800 ) )	return;
	PlayTakeHitRagDoll( LastTakeHitInfo.Momentum, LastTakeHitInfo.HitLocation, LastTakeHitInfo.HitBoneIndex );
	avaDamage = class<avaDamageType>(LastTakeHitInfo.DamageType);
	//	Play Damaged Sound in Clients...
	if ( LastTakeHitInfo.bDamage )
	{
		if ( WorldInfo.TimeSeconds - LastPainSound >= MinTimeBetweenPainSounds )
		{
			LastPainSound = WorldInfo.TimeSeconds;
			SoundGroupClass.static.PlayPainSound( self, LastTakeHitInfo.HitEffect, avaDamage );
		}
	}
	// 떨어져서 입은 Damage 의 경우에는 Blood 를 찍어주지 말자...
	if (LastTakeHitInfo.DamageType == class'DmgType_Fell')	return;
	// 피!
	BloodMomentum				=	LastTakeHitInfo.Momentum;
	if ( BloodMomentum.Z > 0 )		BloodMomentum.Z *= 0.5;
	BoneName					=	Mesh.GetBoneNameByIndex( LastTakeHitInfo.HitBoneIndex );
	if ( LastTakeHitInfo.HitEffect == HET_HelmetHit )
	{
		EmitterTemplate =	SparkEmitterClass.default.EmitterTemplate;
	}
	else
	{
		shotInfo = Check_ShotInfo( BoneName );
		if ( shotInfo == SI_Head )									bloodsize	= 0;
		else if ( shotInfo == SI_Stomach || shotInfo == SI_Chest )	bloodsize	= 1;
		else														bloodsize	= 2;
		if ( class'avaNetHandler'.static.GetAvaNetHandler().IsPlayerAdult() )	EmitterTemplate = BloodEmitterClass.default.BloodEmitterTemplate[bloodsize];
		else																	EmitterTemplate = BloodEmitterClassTeen.default.BloodEmitterTemplate[bloodsize];
	}
	
	SpawnParticleEffectOnPawn( EmitterTemplate, None, BoneName, LastTakeHitInfo.HitLocation, rotator(-BloodMomentum) );	

	// 자신이 쏜 경우에는 Additional 한 Effect 와 Sound 가 추가된다...
	if ( !avaGameReplicationInfo(WorldInfo.GRI).UseFirstPersonEffect )	
		return;

	if ( LastTakeHitInfo.DamagedBy == avaPlayerReplicationInfo( LocalPC.PlayerReplicationInfo ).PlayerID )
	{
		SoundGroupClass.static.PlayPainSound( LocalPC.Pawn, LastTakeHitInfo.HitEffect, avaDamage, true );

		if ( class'avaNetHandler'.static.GetAvaNetHandler().IsPlayerAdult() )	EmitterTemplate = BloodEmitterClass.default.BloodEmitterTemplateSelf[bloodsize];
		else																	EmitterTemplate = BloodEmitterClassTeen.default.BloodEmitterTemplateSelf[bloodsize];

		SpawnParticleEffectOnPawn( EmitterTemplate, None, BoneName, LastTakeHitInfo.HitLocation, rotator(-BloodMomentum) );	
	}
}

simulated function bool CheckMaxEffectDistance(PlayerController P, vector SpawnLocation, optional float CullDistance)
{
	local float Dist;
	if ( P.ViewTarget == None )		return true;
	if ( (Vector(P.Rotation) Dot (SpawnLocation - P.ViewTarget.Location)) < 0.0 )
		return false;
	Dist = VSize(SpawnLocation - P.ViewTarget.Location);
	if (CullDistance > 0 && CullDistance < Dist * P.LODDistanceFactor)
		return false;
	return !P.BeyondFogDistance(P.ViewTarget.Location,SpawnLocation);
}

/**************************************************************************************************
	Look At Functions
**************************************************************************************************/
simulated function EnableAutoLookAt( bool bEnable )
{
	bEnableAutoLookAtControl = bEnable;
	EnableLookAtController( bEnable );
}

simulated function EnableLookAtController( bool bEnable )
{
	LookAtControl.SetSkelControlActive( bEnable );
	if ( LeftEyeControl != None )	
	{
		LeftEyeControl.SetSkelControlActive( bEnable );
		RightEyeControl.SetSkelControlActive( bEnable );
	}
}

simulated function LookAt( vector at, bool bEnable )
{
	if ( LookAtControl == None )	return;
	bEnableLookAt		= bEnable;
	EnableLookAtController( bEnable );
	if ( bEnable )
	{
		LookAtControl.TargetLocation	= at;
		if ( LeftEyeControl != None )	
		{
			LeftEyeControl.TargetLocation	= at;
			RightEyeControl.TargetLocation	= at; 
		}
	}
}

/**************************************************************************************************
	Player Decal Functions
**************************************************************************************************/
simulated function int ConvertLevelToDecalGrade( int nLevel )
{
	if ( nLevel <= 5 )			nLevel = nLevel - 1;				// nLevel 이 5 이하인 경우는 병장까지를 의미한다. 병장까지는 호봉의 개념 없음
	else if ( nLevel <= 50 )	nLevel = 4 + ( nLevel - 1 )/ 5;		// nLevel 이 50 이하인 경우는 장성이하를 의미한다. 각 계급별 호봉수 5
	else						nLevel = 14 + nLevel - 50;			// nLevel 이 50 초과인 경우는 장성을 의미한다. 장성은 호봉가 없음
	return max( nLevel, 0 );
}

simulated function CreateCharacterDecal( int nTeam, int nLevel, int nClanMarkID, optional bool bNoSpecular, optional bool bNoNormal, optional bool bShowDefaultClanMark = false )
{
	// create the composite texture
	local string	TeamDecalStr;
	local string	ClassDecalStr;
	local string	RankDecalStr;
	local string	ClanDecalStr;
	local int		i;

	if ( DecalParentMesh == None )	
		return;

	DecalMIC = DecalParentMesh.CreateMaterialInstance( 0 );
	if ( DecalTexture != None )		DecalTexture = None;
	if ( DecalNormal  != None )		DecalNormal  = None;
	if ( DecalSpec	  != None )		DecalSpec	 = None;	
	DecalTexture	= new(self) class'avaTexture2DComposite';
	DecalNormal		= new(self) class'avaTexture2DComposite';
	DecalSpec		= new(self) class'avaTexture2DComposite';

	TeamDecalStr	= TeamDecal[nTeam];
	ClassDecalStr	= ( DefaultTeam == 0 ) ? EUClassDecal[TypeID] : NRFClassDecal[TypeID];
	RankDecalStr	= RankDecal[ConvertLevelToDecalGrade( nLevel )];

	DecalTexture.SourceRegions.Add(4);
	DecalNormal.SourceRegions.Add(4);
	DecalSpec.SourceRegions.Add(4);

	CreateSubDecal( TeamDecalStr, 0, 0, 0, bNoSpecular, bNoNormal );		// 진영 : EU or NRF
	CreateSubDecal( RankDecalStr, 1, 0, 128, bNoSpecular, bNoNormal );		// 계급 :
	CreateSubDecal( ClassDecalStr, 2, 128, 128, bNoSpecular, bNoNormal );	// 병과 :

`if( `isdefined(FINAL_RELEASE) )
	if ( nClanMarkID == 0 && bShowDefaultClanMark == false)
		nClanMarkID	= -1;
`endif

	if ( nClanMarkID >= 0 )
	{
		ClanDecalStr	=	class'avaNetHandler'.static.GetClanMarkPkgNameFromID( nClanMarkID );
		`log( "avaCharacter.ClanDecalStr" @ClanDecalStr );
		CreateSubDecal( ClanDecalStr, 3, 128, 0, bNoSpecular, bNoNormal );	// 클랜 : 
	}
	
	for ( i = 0 ; i < 4 ; ++ i )
	{
		DecalTexture.SourceRegions[i].Texture2D.bForceMiplevelsToBeResident	=	true;
		if ( DecalNormal.SourceRegions[i].Texture2D != None )	DecalNormal.SourceRegions[i].Texture2D.bForceMiplevelsToBeResident	=	true;
		if ( DecalSpec.SourceRegions[i].Texture2D != None )		DecalSpec.SourceRegions[i].Texture2D.bForceMiplevelsToBeResident	= true;
	}
	DecalTexture.UpdateCompositeTextueEx(256,256,0);
	DecalNormal.UpdateCompositeTextueEx(256,256,0);
	DecalSpec.UpdateCompositeTextueEx(256,256,0);
	for ( i = 0 ; i < 4 ; ++ i )
	{
		DecalTexture.SourceRegions[i].Texture2D.bForceMiplevelsToBeResident	=	false;
		if ( DecalNormal.SourceRegions[i].Texture2D != None )	DecalNormal.SourceRegions[i].Texture2D.bForceMiplevelsToBeResident	=	false;
		if ( DecalSpec.SourceRegions[i].Texture2D != None )		DecalSpec.SourceRegions[i].Texture2D.bForceMiplevelsToBeResident	=	false;
	}
	DecalMIC.SetTextureParameterValue( 'Diffuse1', DecalTexture );
	if ( bNoNormal == false )	DecalMIC.SetTextureParameterValue( 'Normal', DecalNormal );
	else						DecalMIC.SetTextureParameterValue( 'Normal', None );
	if ( bNoSpecular == false )	DecalMIC.SetTextureParameterValue( 'Specular', DecalSpec );
	else						DecalMIC.SetTextureParameterValue( 'Specular', None );
}

simulated function CreateSubDecal( string srcName, int subIdx,int offsetx, int offsety, optional bool bNoSpecular, optional bool bNoNormal )
{
	local Texture2D	Diffuse, Normal, Spec;
	local string	srcBaseName;
	srcBaseName		= Left( srcName, InStr(srcName,"_d") );	
	Diffuse			= Texture2D( DynamicLoadObject( srcName, class'Texture2D' ) );
	Normal			= Texture2D( DynamicLoadObject( srcBaseName $"_b", class'Texture2D' ) );
	Spec			= Texture2D( DynamicLoadObject( srcBaseName $"_s", class'Texture2D' ) );
	if ( Diffuse != None )
	{
		DecalTexture.SourceRegions[subIdx].Texture2D	= Diffuse;
		DecalTexture.SourceRegions[subIdx].OffsetX		= offsetx;
		DecalTexture.SourceRegions[subIdx].OffsetY		= offsety;
		DecalTexture.SourceRegions[subIdx].SizeX		= 128;
		DecalTexture.SourceRegions[subIdx].SizeY		= 128;
	}
	if ( Normal != None && bNoNormal == false )
	{
		DecalNormal.SourceRegions[subIdx].Texture2D		= Normal;
		DecalNormal.SourceRegions[subIdx].OffsetX		= offsetx;
		DecalNormal.SourceRegions[subIdx].OffsetY		= offsety;
		DecalNormal.SourceRegions[subIdx].SizeX			= 128;
		DecalNormal.SourceRegions[subIdx].SizeY			= 128;
	}
	if ( Spec != None && bNoSpecular == false )
	{
		DecalSpec.SourceRegions[subIdx].Texture2D		= Spec;
		DecalSpec.SourceRegions[subIdx].OffsetX			= offsetx;
		DecalSpec.SourceRegions[subIdx].OffsetY			= offsety;
		DecalSpec.SourceRegions[subIdx].SizeX			= 128;
		DecalSpec.SourceRegions[subIdx].SizeY			= 128;
	}
}

simulated function bool CalcThirdPersonCam( float fDeltaTime, out vector out_CamLoc, out rotator out_CamRot, out float out_FOV )
{
	local vector CamStart, HitLocation, HitNormal, CamDir;
	local float SinPitch;	

	SinPitch = sin(out_CamRot.Pitch * 0.0000958738);

	CamStart = Location + ( ThirdPersonCamOffset >> out_CamRot );	
	CamStart.Z += 1.5 * 40 * ( 0.3 + 0.7 * square(SinPitch) );
	CamDir = Vector(out_CamRot) * GetCollisionRadius() * CurrentCameraScale;

	//if ( (Health <= 0) )
	//{
	//	// adjust camera position to make sure it's not clipping into world
	//	// @todo fixmesteve.  Note that you can still get clipping if FindSpot fails (happens rarely)
	//	FindSpot(GetCollisionExtent(),CamStart);
	//}

	if (CurrentCameraScale < CameraScale)
	{
		CameraScaleTime += fDeltaTime;
		CurrentCameraScale += ( CameraScale - CurrentCameraScale ) * fDeltaTime / 2.0;
		CurrentCameraScale = FMin( CurrentCameraScale, CameraScale );
	}
	else if (CurrentCameraScale > CameraScale)
	{
		CameraScaleTime += fDeltaTime;
		CurrentCameraScale = ( default.CurrentCameraScale - CameraScale ) * CameraScaleTime/2.0;
		CurrentCameraScale = FMax( CurrentCameraScale, CameraScale );
	}

	if (CamDir.Z > GetCollisionHeight())
	{
		CamDir *= square(cos(out_CamRot.Pitch * 0.0000958738)); // 0.0000958738 = 2*PI/65536
	}

	out_CamLoc = CamStart - CamDir;

	if (Trace(HitLocation, HitNormal, out_CamLoc, CamStart, false, vect(12,12,12)) != None)
	{
		out_CamLoc = HitLocation;				
	}	

	return true;
}

simulated event vector GetCameraOriginOffset()
{
	return vect(0,0,0);
}

/** Tick function */
simulated function Tick(float DeltaSeconds)
{
	if ( LocalPC == None )	
		FindLocalController();
	Super.Tick(DeltaSeconds);
}

simulated event Destroyed()
{
	if ( Helmet != None )	
		Helmet.Destroy();
	super.Destroyed();
}

simulated function PlayFiring(float Rate, name WeaponFiringMode)
{
	PrvWeaponFiringMode = WeaponFiringMode;
	PlayAnimByEvent( WeaponFiringMode == '0' ? EBT_Fire : EBT_AltFire );
}

simulated function StopPlayFiring()
{
	StopAnimByEvent( PrvWeaponFiringMode == '0' ? EBT_Fire : EBT_AltFire );
	fLastFireTime	= WorldInfo.TimeSeconds;
}

simulated event EndCrouch(float HeightAdjust)
{
	super.EndCrouch(HeightAdjust);
	if( Mesh != None )
	{
		Mesh.SetTranslation(Mesh.Translation - Vect(0,0,1)*14);
	}
}

simulated event StartCrouch(float HeightAdjust)
{
	super.StartCrouch(HeightAdjust);
	if( Mesh != None )
	{
		Mesh.SetTranslation(Mesh.Translation + Vect(0,0,1)*14);
	}
}

/**************************************************************************************************
	Functons For Dynamic Load Object
**************************************************************************************************/
static function StaticPrecache( out array<Object> list )
{
	list[list.Length] = default.BaseSkeletalMesh;
	list[list.Length] = default.Mesh.SkeletalMesh;
}

static function DLO( string resource )
{
	if (resource != "")
	{		
		DynamicLoadObject( resource, class'Object' );
	}
}

static event LoadDLOs()
{
	local int		i;	
	local string	str;

	DLO( default.HeadMeshName );
	DLO( default.EyeballMeshName );
	DLO( default.HelmetMeshName );
	DLO( default.HelmetSkinName );
	DLO( default.HeadMTSName );

	// Set Body Part
	for ( i = 0 ; i < default.BodyParts.Length ; ++ i )
	{
		DLO( default.BodyParts[i].MeshName );
	}

	for ( i = 0 ; i < default.ItemParts.Length ; ++ i )
	{
		DLO( default.ItemParts[i].MeshName );
	}

	DLO( "avaCharCommon.Head" );

	// Decal DLO
	for (i = 0 ; i < 2 ; ++ i )
	{
		DLO( default.TeamDecal[i] );
		str	= Left( default.TeamDecal[i], InStr(default.TeamDecal[i],"_d") );
		DLO( str $"_b" );
		DLO( str $"_s" );
	}

	for (i = 0 ; i < 3 ; ++ i )
	{
		DLO( default.EUClassDecal[i] );
		DLO( default.NRFClassDecal[i] );
		str = Left( default.EUClassDecal[i], InStr(default.EUClassDecal[i],"_d") );
		DLO( str $"_b" );
		DLO( str $"_s" );
	}

	for (i = 0 ; i < 19 ; ++ i )
	{
		DLO( default.RankDecal[i] );
		str = Left( default.RankDecal[i], InStr(default.RankDecal[i],"_d") );
		DLO( str $"_b" );
		DLO( str $"_s" );
	}
}

function DropDogTag()
{
	local avaPickupDogTag	dogTag;
	local vector			DropLocation;
	local rotator			ViewRotation;
	local vector			X, Y, Z;
	if ( avaGameReplicationInfo(WorldInfo.GRI).DogTagPackCnt <= 0 )	return;
	DogTagCnt		= 0;;
	ViewRotation	= GetViewRotation();
	DropLocation	= GetPawnViewLocation() + (Vect(0,0,-12) >> ViewRotation);
	dogTag			= Spawn( class'avaPickupDogTag',,,DropLocation);
	dogTag.SetTeam( GetTeamNum() );
	GetAxes( ViewRotation, X, Y, Z );
	dogTag.Throw( TearOffMomentum/2.0 * X + Velocity );
}

/**************************************************************************************************
	Functons For Debugging
**************************************************************************************************/
simulated function DisplayDebug(HUD HUD, out float out_YL, out float out_YPos)
{
	Super.DisplayDebug(Hud, out_YL, out_YPos);
}

`devexec function sv_TakeOffHelmet()
{
	TakeOffHelmet( vect(5,0,0) );
}

`devexec function sv_PlayHitEx( int shot )
{
	local TraceHitInfo	HitInfo;
	PlayHitEx( 100, None, Vect(0,0,0), None, Vect(0,0,0), HitInfo, true, None, EShotInfo(shot), 0 );
}

`devexec simulated function sv_ChangePlayerClan( int ClanID, optional bool bNoSpecular, optional bool bNoNormal )
{
	if ( WorldInfo.NetMode == NM_StandAlone )
	{
		avaPlayerReplicationInfo( PlayerReplicationInfo ).ClanMarkID = ClanID;
		CreateCharacterDecal( DefaultTeam, avaPlayerReplicationInfo( PlayerReplicationInfo ).Level, avaPlayerReplicationInfo( PlayerReplicationInfo ).GetClanMarkID(), bNoSpecular, bNoNormal );
	}
}

`devexec simulated function sv_ChangePlayerLevel( int nLevel, optional bool bNoSpecular, optional bool bNoNormal )
{
	if ( WorldInfo.NetMode == NM_StandAlone )
	{
		avaPlayerReplicationInfo( PlayerReplicationInfo ).Level = nLevel;
		CreateCharacterDecal( DefaultTeam, avaPlayerReplicationInfo( PlayerReplicationInfo ).Level, avaPlayerReplicationInfo( PlayerReplicationInfo ).GetClanMarkID(), bNoSpecular, bNoNormal );
	}
}

`devexec function AddHealth( int nHealth )
{
	local int TmpHealth;
	TmpHealth = Health + nHealth;
	Health	  = clamp( TmpHealth, 1, HealthMax );
}

`devexec function sv_AddArmor( int nArmor )
{
	Armor_Stomach += nArmor;
}

`devexec simulated function sv_ChangeHead( string ModName )
{
	local class<avaCharacterModifier>	Mod;
	Mod = class<avaCharacterModifier>(DynamicLoadObject( ModName, class'class' ));
	if ( Mod == None )
	{
		ModName = "avaRules.avaMod_" $ModName;
		Mod = class<avaCharacterModifier>(DynamicLoadObject( ModName, class'class' ));
	}
	if ( Mod != None )
	{
		if ( Mod.default.HeadMeshName != "" )
		{
			HeadMeshName		=	Mod.default.HeadMeshName;
			HeadMTSName			=	Mod.default.HeadMTSName;
			EyeBallMeshName		=	Mod.default.EyeBallMeshName;
			CreateHead();
			CreateEyeball();
		}
	}
}

`devexec simulated function sv_ChangeBodyPart( int nSlot, string MeshName )
{
	local int	i;
	local SkeletalMesh	TempSkelMesh;
	for ( i = 0 ; i < BodyParts.length ; ++ i )
	{
		if ( BodyParts[i].Slot == nSlot && BodyParts[i].BodyMesh != None )
		{
			Mesh.DetachComponent( BodyParts[i].BodyMesh );
			BodyParts[i].BodyMesh = None;
			TempSkelMesh = SkeletalMesh( DynamicLoadObject( MeshName, class'SkeletalMesh' ) );
			BodyParts[i].BodyMesh = new(outer) class'SkeletalMeshComponent';
			BodyParts[i].BodyMesh.bUseAsOccluder = FALSE;
			BodyParts[i].BodyMesh.LODBias		 = LODBias;
			BodyParts[i].BodyMesh.SetSkeletalMesh( TempSkelMesh );
			BodyParts[i].BodyMesh.SetParentAnimComponent( Mesh );
			BodyParts[i].BodyMesh.bUpdateSkelWhenNotRendered = false;
			InitComponent( BodyParts[i].BodyMesh );
			Mesh.AttachComponent( BodyParts[i].BodyMesh,'Bip01');
		}
	}
}

defaultproperties
{
	Begin Object Class=DynamicLightEnvironmentComponent Name=ADynamicLightEnvironmentComponent
		Name=ADynamicLightEnvironmentComponent
	End Object
	Components.Add(ADynamicLightEnvironmentComponent)
	LightEnvironment=ADynamicLightEnvironmentComponent

	Begin Object Class=SkeletalMeshComponent Name=WPawnSkeletalMeshComponent		
		bUseAsOccluder						=	FALSE
		BlockZeroExtent						=	true
		BlockRigidBody						=	true
		CollideActors						=	true
		AlwaysLoadOnClient					=	true
		AlwaysLoadOnServer					=	true
		bOwnerNoSee							=	true
		CastShadow							=	true		
		bUpdateSkelWhenNotRendered			=	false		
		bUpdateKinematicBonesFromAnimation	=	false
		bCastDynamicShadow					=	true		
		bForceNotToDraw						=	true
		SkeletalMesh						=	SkeletalMesh'avaCharCommon.EU_Base_Bone01'
		AnimSets(0)							=	AnimSet'avaCharCommon.New_animset'
		AnimTreeTemplate					=	AnimTree'avaCharCommon.AnimTree'
		PhysicsAsset						=	PhysicsAsset'avaCharPhAT.Default.CharacterPhAT_Hit'
		PhysicsWeight						=	1.0
		Translation							=	(X=-6,Y=-4,Z=0.0)		
		LightEnvironment					=	ADynamicLightEnvironmentComponent
		//bUseHardwareScene					=	true
		RBChannel							=	RBCC_Pawn
		RBCollideWithChannels				=	(Default=true,Pawn=true)
		LODBias								=	0
		bDisableWarningWhenAnimNotFound		=	true
		bChartDistanceFactor				=	true
		MinDistFactorForKinematicUpdate		=	0.2
		bSyncActorLocationToRootRigidBody	=	false
		bForceNotToUseCPUSkinInGame			=	true
		bSkipAllUpdateWhenPhysicsAsleep		=	TRUE
	End Object
	Mesh=WPawnSkeletalMeshComponent
	Components.Add(WPawnSkeletalMeshComponent)

	RagdollPhysicsAsset						=	PhysicsAsset'avaCharPhAT.Default.DefaultCharacterPhAT'
	TakeHitPhysicsAsset						=	PhysicsAsset'avaCharPhAT.Default.CharacterPhAT_Hit'

	bExtraMeshInit							=	false
	TypeID									=	-1
	DefaultTeam								=	-1
	
	Begin Object Class=SkeletalMeshComponent Name=HeadComponent
		bOwnerNoSee							=	true
		bUpdateSkelWhenNotRendered			=	false
		CastShadow							=	true		
		AnimTreeTemplate					=	AnimTree'avaCharCommon.Head'
		bUseAsOccluder						=	FALSE
		bForceNotToUseCPUSkinInGame			=	true
		LODBias								=	LODBias
		bPauseAnims							=	true
		bSkipAllUpdateWhenPhysicsAsleep		=	TRUE
	End Object
	HeadComp=HeadComponent

	Begin Object Class=SkeletalMeshComponent Name=EyeBallComponent
		bOwnerNoSee							=	true
		bUpdateSkelWhenNotRendered			=	false
		CastShadow							=	false		
		AnimTreeTemplate					=	AnimTree'avaCharCommon.EyeBall'
		bUseAsOccluder						=	FALSE
		bForceNotToUseCPUSkinInGame			=	true
		LODBias								=	LODBias
		bPauseAnims							=	true
		bSkipAllUpdateWhenPhysicsAsleep		=	TRUE
	End Object
	EyeBallComp=EyeBallComponent

	HeadMeshName							=	"CH_HEAD_COM.Head_06.MS_Head_06"
	HeadMTSName								=	"avaCharCommon.Head_06_MTS"	
	bEnableAutoLookAtControl				=	true
	bEnableLookAt							=	true

	// Damage 받았을 때 적용되는 KickBack Data...
	DamagedKickBackInfo(SI_Generic)		= ( MinAngleX= 0.0,  MaxAngleX=0.0, DirectionRandomX=0.0, DamageAmpX=0.0, MinAngleZ=0,  MaxAngleZ=0, DirectionRandomZ=0.0, DamageAmpZ=0.0)
	DamagedKickBackInfo(SI_Head)		= ( MinAngleX=-12.0, MaxAngleX=0.0, DirectionRandomX=1.0, DamageAmpX=0.5, MinAngleZ=-9, MaxAngleZ=9, DirectionRandomZ=0.5, DamageAmpZ=1.0)
	DamagedKickBackInfo(SI_Stomach)		= ( MinAngleX=-4.0,  MaxAngleX=0.0, DirectionRandomX=1.0, DamageAmpX=0.1, MinAngleZ=0,  MaxAngleZ=0, DirectionRandomZ=0.0, DamageAmpZ=0.0)
	DamagedKickBackInfo(SI_Chest)		= ( MinAngleX=-4.0,  MaxAngleX=0.0, DirectionRandomX=1.0, DamageAmpX=0.1, MinAngleZ=0,  MaxAngleZ=0, DirectionRandomZ=0.0, DamageAmpZ=0.0)
	DamagedKickBackInfo(SI_LeftArm)		= ( MinAngleX=-4.0,  MaxAngleX=0.0, DirectionRandomX=1.0, DamageAmpX=0.1, MinAngleZ=0,  MaxAngleZ=0, DirectionRandomZ=0.0, DamageAmpZ=0.0)
	DamagedKickBackInfo(SI_RightArm)	= ( MinAngleX=-4.0,  MaxAngleX=0.0, DirectionRandomX=1.0, DamageAmpX=0.1, MinAngleZ=0,  MaxAngleZ=0, DirectionRandomZ=0.0, DamageAmpZ=0.0)
	DamagedKickBackInfo(SI_LeftLeg)		= ( MinAngleX= 0.0,  MaxAngleX=0.0, DirectionRandomX=0.0, DamageAmpX=0.0, MinAngleZ=0,  MaxAngleZ=0, DirectionRandomZ=0.0, DamageAmpZ=0.0)
	DamagedKickBackInfo(SI_RightLeg)	= ( MinAngleX= 0.0,  MaxAngleX=0.0, DirectionRandomX=0.0, DamageAmpX=0.0, MinAngleZ=0,  MaxAngleZ=0, DirectionRandomZ=0.0, DamageAmpZ=0.0)

	MeshInterpSpeedT	=	6
	MeshInterpSpeedR	=	6
}