//=============================================================================
//  avaWeapon
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
// 
//	2006/04/06 by OZ
//		Sprint 관련 함수 및 State 추가
//=============================================================================
class avaWeapon extends Weapon
	native
	nativereplication
	dependson(avaPlayerController)
	config(game)
	abstract;

`include(avaGame/avaGame.uci)

cpptext
{
	#ifdef EnableHostMigration
	virtual void hmSerialize(FArchive& Ar); // 20061102 dEAthcURe|HM
	#endif
	
	virtual void TickSpecial(FLOAT DeltaTime);
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );

	virtual INT LagRot( INT NewValue, INT OldValue, INT MaxDiff);
	virtual FVector	GetWeaponBob(class AavaPawn* Holder);
}

/*********************************************************************************************
 Ammo / Pickups / Inventory
 ********************************************************************************************* */
// Weapon Type 이 추가되면 avaGame.uci 의 MAX_WEAPON_TYPE 의 갯수도 같이 늘려줄것!!!
enum WEAPON_TYPE
{
	WEAPON_KNIFE,
	WEAPON_PISTOL,
	WEAPON_GRENADE,
	WEAPON_SMG,
	WEAPON_RIFLE,
	WEAPON_SNIPER,
	WEAPON_SHOTGUN,
	WEAPON_ETC,
	WEAPON_RPG,
	WEAPON_MACHINEGUN,
};

var class<avaAmmoPickupFactory> AmmoPickupClass;

/** Current ammo count */
var hmserialize cipher Byte AmmoCount;	// [!] 20070323 dEAthcURe|HM 'hmserialize' // 20080214 test // var hmserialize BYTE AmmoCount;	// [!] 20070323 dEAthcURe|HM 'hmserialize'

/** Max ammo count */
var int MaxAmmoCount;

/** Holds the min. amount of reload time that has to pass before you can switch */
var float MinReloadPct;

/** View shake applied when a shot is fired */
var avaPlayerController.ViewShakeInfo FireShake;

var name EffectSockets;

/*********************************************************************************************
 Attachments
********************************************************************************************* */
var class<avaWeaponAttachment>		AttachmentClass;		// The class of the attachment to spawn
var avaWeaponAttachment 			WeaponAttachment;		// Holds a reference to the actual Attachment

/** If true, this weapon is a super weapon.  Super Weapons have longer respawn times a different
    pickup base and never respect weaponstay */
var bool bSuperWeapon;

/*********************************************************************************************
 Inventory Grouping/etc.
********************************************************************************************* */
var byte InventoryGroup;	// The weapon/inventory set, 0-9.
var float GroupWeight;		// position within inventory group. (used by prevweapon and nextweapon)
var float InventoryWeight;	// The final inventory weight.  It's calculated in PostBeginPlay()

/** If true, this will will never accept a forwarded pending fire */
var bool bNeverForwardPendingFire;

/*********************************************************************************************
 Animations and Sounds
********************************************************************************************* */

/** Animation to play when the weapon is fired */
var(Animations)	array<name>	WeaponFireAnim;

var(Animations) array<name> WeaponAltFireAnim;

/** Animation to play when the weapon is Put Down */
var(Animations) name		WeaponPutDownAnim;

/** Animation to play when the weapon is Equipped */
var(Animations) name		WeaponEquipAnim;

var(Animations) array<name>	WeaponIdleAnims;

var	int						IdleContinuousCnt;			//	기본 Idle Animation 이 연속으로 몇번 나왔는가?
var int						MinIdleContinuousCnt;		//	기본 Idle Animation 이 최소한 이정도는 나와야 Gesture 가 나올 기회가 주어진다...

/** Sound to play when the weapon is fired */
var(Sounds)	SoundCue	WeaponFireSnd;

/** Sound to play when the weapon is Put Down */
var(Sounds) SoundCue 	WeaponPutDownSnd;

/*********************************************************************************************
 Misc
********************************************************************************************* */

/** How much to damp view bob */
var() float	BobDamping;

/** How much to damp view bob in Dash Mode*/
var() float	BobDampingInDash;

/** How much to damp jump and land bob */
var() float	JumpDamping;




/** Limit for pitch lag */
var()	float MaxPitchLag;

/** Limit for pitch lag */
var()	float MaxYawLag;

/** Rotation lag convergence speed */
var()	float RotLagSpeed;

/** The Color used when drawing the Weapon's Name on the Hud */
var color WeaponColor;
var color WeaponColorInNVG;

/*********************************************************************************************
 Muzzle Flash
********************************************************************************************* */

/** dynamic light */
var bool bDynamicMuzzleFlashes;

var	class<avaGunMuzzleFlashLight> MuzzleFlashLightClass;
var avaGunMuzzleFlashLight	MuzzleFlashLight;

/** Holds the name of the socket to attach a muzzle flash too */
var name					MuzzleFlashSocket;

/** Muzzle flash staticmesh, attached to weapon mesh */
var	StaticMeshComponent		MuzzleFlashMesh;

/** Muzzle flash PSC and Templates*/
var ParticleSystemComponent	MuzzleFlashPSC;
var ParticleSystemComponent AbsMuzzleFlashPSC;
var ParticleSystem			MuzzleFlashPSCTemplate, MuzzleFlashAltPSCTemplate;
var ParticleSystem			AbsMuzzleFlashPSCTemplate, AbsMuzzleFlashAltPSCTemplate;
var color					MuzzleFlashColor;
var bool					bMuzzleFlashPSCLoops;

/** How long the Muzzle Flash should be there */
var() float					MuzzleFlashDuration;
/** Offset from view center */
var(FirstPerson) vector	PlayerViewOffset;
/*********************************************************************************************
 * WeaponStatsIndex
 ********************************************************************************************* */
var		int		OwnerStatsID;
//var 	int		WeaponStatsID;

/** if true, pressing fire activates firemode 1 and altfire activates firemode 0 (only affects human players) */
var config bool bSwapFireModes;

/** The Class of KProjectile to spawn */
//var	Array< class<avaKProjectile> >	WeaponKProjectiles;

/** 무기에 적용할 FOV 값 **/
var float					WeaponFOV;

var bool					FireImpulse/*, ClearImpulse*/;

// Mode 에 따라 Projectile 의 발사 위치가 틀리게 나가도록 하기 위해서
var()	array<vector>		FireOffsetEx;

// True 이면 Ammo 가 없더라도 BeginFire 가 성공한다. 카스 스타일
var()	bool				bBeginFireWithoutAmmo;

// 
var	class<avaPickUp>		PickUpClass;
var	bool					DropOnlyOneAmmo;	// 무기를 떨어뜨릴때 Ammo 를 하나만 남기고 떨어뜨린다.
var bool					PickUpAddAmmo;		// PickUp 시 Ammo 만 증가시켜준다.
var	hmserialize float		PickUpLifeTime;		// PickUp 의 Life Time 을 지정해준다. replication 안 해도 되지만 Host Migration 을 위해서는 해줘야 한다....
var hmserialize BYTE		PickUpTeamNum;		// PickUp 의 TeamNum 를 지정해준다.
var hmserialize	bool		bDrawInRadar;		// PickUp 의 DrawRadar 유무를 지정해준다.
var	int						IndicatorType;		//

var()	vector				ThrowOffset;

// Weapon 에 의한 Pawn 의 속도 제한
var()	float				BaseSpeed;				// 기본속도
var()	float				AimSpeedPct;			// 조준시 보정치
var()	float				WalkSpeedPct;			// 걷기시 보정치
var()	float				CrouchSpeedPct;			// 앉아이동시 보정치
var()	float				CrouchAimSpeedPct;		// 앉아서 조준 이동시 보정치
var()	float				SwimSpeedPct;			// 수영시 보정치
var()	float				SprintSpeedPct;			// 스프린트시 보정치
var()	float				CrouchSprintSpeedPct;	// 앉아서 스프린트시 보정치

// Dash 관련 Properties
var(Animations) name		WeaponDashStartAnims;
var(Animations) name		WeaponDashEndAnims;
var(Animations) name		WeaponDashIdleAnims;
var AnimNodeSequence		DashStartAnimNode;	// WeaponDashEndAnims Play 시 Start 위치를 계산하기 위하여

var(Animations)	array<name>	QuickVoiceAnim;

var		bool				bAlwaysDrawIcon;	// 소지 여부를 Icon 으로 표현한다.

var		WEAPON_TYPE			WeaponType;			// 어떤 Type 의 무기인가...
	 
// Modifier 를 위한 Properties

// Weapon Skin : 0(Body),1(Stock),2(Grip)
// 3인칭총, PickUp 등에서도 같이 처리해줘야 한다!!!
`define						MAX_WEAPON_SKIN	3 

var string					EUArmMeshName, NRFArmMeshName, DefaultHandMeshName;
var string					BaseSkelMeshName;
var string					BaseAnimSetName;

var	string					WeaponSkin[`MAX_WEAPON_SKIN];	
// Arm 용 Mesh 이름, Skin 이름, Component
var string					ArmMeshName;
var string					ArmSkinName;
var SkeletalMeshComponent	ArmComponent;
// Hand 용 Mesh 이름, Skin 이름, Component
var string					HandMeshName;
var string					HandSkinName;
var SkeletalMeshComponent	HandComponent;

var	bool					bInitializedMesh;
// Weapon 에 붙는 Item 관리용
struct native WeaponItemPart
{
	var name					SocketName;
	var string					MeshName;
	var StaticMeshComponent		ItemMesh;
	var	float					MaxVisibleDistance;
};

var array<WeaponItemPart>		ItemParts;

// Default Weapon Modifier...
var array< class<avaModifier> >	DefaultModifiers;


// Weapon Modifier...
var bool						bAdjustWeaponModifier;
var class<avaModifier>			WeaponModifiers[`MAX_WEAPON_MODIFIER];
var repnotify bool				ModifiersRepDone;
var	repnotify hmserialize float	MaintenanceRate;			// 정비도...

var bool						bRemoveReserved;		// inactive 상태로 가면 InventoryManager 에서 Remove 하도록 하자...

var bool						bSpecialInventory;		// bSpecialInventory 가 true 이면 소지하고 있는 Icon 이 표시된다.
var hmserialize int				RadarIconCode;			// bSpecialInventory 가 true 인 경우 이 Weapon 을 소지하고 있는 Pawn 은 Radar 에 등록된다. 이때 Radar 에 표현할 IconCode 를 넣어준다.
var bool						bHideWeaponMenu;		// bHideWeaponMenu 가 true 인 경우 WeaponMenu 에서 표시되지 않는다.
var bool						bNoSelectable;			// Select 가 불가능한 Weapon 이다. Mission Objective 를 Weapon 과 동일하게 처리하기 위해서 필요하다.
var color						RadarIconColor;			// RadarIconCode가 없을때 찍어줄 색상


var Texture2D					CrossHairMtrl;			// 이것이 지정되어 있다면 CrossHair 를 이것으로 찍는다...
var int							CrossHairSizeX, CrossHairSizeY;


var	databinding	localized string	StatusStr;			// 소지하고 있을 경우에 상태창에 표시되는 String
var	int								StatusPriority;		// 동시에 2개 이상을 소유하고 있을 때 Priority 가 높은 것이 표시된다.

var	float							DropVelocity;
var bool							bDisplayAmmoCount;
var	bool							bDropWhenDead;
var bool							bDropIfHasAmmo;		// Ammo 가 있을때만 떨어뜨린다.

var	databinding	localized string	ItemShortName;

var int								GIMIndexWhenPickUp;	// Game Info Message Index When Pick Up
var int								GIMIndexWhenEquip;	// Game Info Message Index When equip

var bool							bAvailableAbandonWeapon;	// G 키를 이용해 버릴 수 있는 Weapon 이다....

var bool							bInfinityAmmo;				// 해당 총은 Ammo 를 무한대로 가진다...

var	hmserialize	avaPickupProvider	PickUpProvider;				// PickUpProvider...

// true 이면 총알이 떨어진 경우 무기를 삭제한다.
var	bool	DestroyWeaponEmpty;

replication
{
	// Server->Client properties
	if (Role == ROLE_Authority)
		AmmoCount;

	if (Role == ROLE_Authority && bNetDirty )
		WeaponModifiers, ModifiersRepDone, PickUpLifeTime, MaintenanceRate, PickUpTeamNum, bDrawInRadar, RadarIconCode, PickUpProvider;
}

native simulated function SetPositionEx( avaPawn Holder );

simulated function PostBeginPlay()
{
	local int i,j;
	Super.PostBeginPlay();
	CalcInventoryWeight();	
	// StandAlone 에서만 Default Modifier 를 적용한다....
	if ( WorldInfo.NetMode == NM_StandAlone )
	{
		for ( i = 0 ; i < DefaultModifiers.length ; ++ i )
		{
			DefaultModifiers[i].static.ApplyToWeapon_Client( self );

			for ( j = 0 ; j < DefaultModifiers[i].default.CommonAttachedItems.length ; ++ j )
			{
				AddItemMesh( DefaultModifiers[i].default.CommonAttachedItems[j].MeshName,
							 DefaultModifiers[i].default.CommonAttachedItems[j].PrimarySocket,
							 DefaultModifiers[i].default.CommonAttachedItems[i].MaxVisibleDistance );	
			}

			if ( Role == ROLE_Authority )
					DefaultModifiers[i].static.ApplyToWeapon_Server( self );
		}
	}
}

simulated event ReplicatedEvent(name VarName)
{
	if ( VarName == 'ModifiersRepDone' )
		AdjustWeaponModifiers();
	else if ( VarName == 'MaintenanceRate' )
		AdjustMaintenanceRate();
	Super.ReplicatedEvent( VarName );
}

function SetMaintenanceRate( float Rate )
{
	MaintenanceRate = Rate;
	AdjustMaintenanceRate();
}

simulated function AdjustMaintenanceRate()
{
	class'avaMod_Weapon'.static.ApplyMaintenanceRate( self, MaintenanceRate );
}

simulated function AdjustWeaponModifiers(optional pawn PlayerPawn)
{
	local int i,j;
	if ( bAdjustWeaponModifier == true )	return;	// Modifier 는 한번만 적용해야 된다....
	// Weapon Modifier 를 적용한다....
	for ( i = 0 ; i < `MAX_WEAPON_MODIFIER ; ++ i )
	{
		if ( WeaponModifiers[i] != None )
		{
			WeaponModifiers[i].static.ApplyToWeapon_Client( self );
			// Weapon 에도 Primary 와 Secondary Socket 을 구분할 필요가 있을까?
			for ( j = 0 ; j < WeaponModifiers[i].default.CommonAttachedItems.length ; ++ j )
			{
				AddItemMesh( WeaponModifiers[i].default.CommonAttachedItems[j].MeshName,
							 WeaponModifiers[i].default.CommonAttachedItems[j].PrimarySocket,
							 WeaponModifiers[i].default.CommonAttachedItems[j].MaxVisibleDistance );	
			}
			if ( Role == ROLE_Authority )
				WeaponModifiers[i].static.ApplyToWeapon_Server( self );
		}
	}
	if ( avaWeap_BaseGun( self ) != None )
	{		
		if(PlayerPawn != None) {
			MaxAmmoCount = Clamp( MaxAmmoCount + int( avaWeap_BaseGun(self).ClipCnt * avaPawn(PlayerPawn).WeapTypeAmp[WeaponType].AmmoAmp ), 0, 250 );
		}
		else {
			MaxAmmoCount = Clamp( MaxAmmoCount + int( avaWeap_BaseGun(self).ClipCnt * avaPawn(Instigator).WeapTypeAmp[WeaponType].AmmoAmp ), 0, 250 );
		}		
	}
	bAdjustWeaponModifier = true;
}



function bool AddWeaponModifier( class<avaModifier> mod ) // [!] 20070212 dEAthcURe|HM return bool
{
	local int i;
	
	// {{ 20070212 dEAthcURe|HM
	for ( i = 0 ; i < `MAX_WEAPON_MODIFIER ; ++ i )
	{
		if(WeaponModifiers[i] != None && WeaponModifiers[i].default.Id == mod.default.Id) {
			//`log("[dEAthcURe|AddWeaponModifier] already exist " @ mod.default.Id);
			return false;
		}
	}
	// }} 20070212 dEAthcURe|HM
	
	for ( i = 0 ; i < `MAX_WEAPON_MODIFIER ; ++ i )
	{
		if ( WeaponModifiers[i] == None )
		{
			WeaponModifiers[i] = mod;
			return true; // [!]
		}
	}
	return false; // [!]
}

// {{ 20070212 dEAthcURe|HM
native final function WorldInfo GetWorldInfo();

event function bool HmAddWeaponModifier(int idItem)
{	
	local class<avaModifier> modifier;	

	modifier = class'avaMod_TranslatorBase'.static.GetModifier(class'avaNetHandler'.static.GetAvaNetHandler().WeaponModifierList, idItem);	
	if(modifier != None) {
		`log("[dEAthcURe|HmAddWeaponModifier] found"@ idItem);
		return AddWeaponModifier(modifier);
	}
	else {
		`log("[dEAthcURe|HmAddWeaponModifier] Not found");
	}
	return false;
}
// }} 20070212 dEAthcURe|HM

function WeaponModifierDone(optional pawn PlayerPawn)
{
	ModifiersRepDone = true;
	AdjustWeaponModifiers(PlayerPawn);
}

// {{ 20070212 dEAthcURe|HM
event function HmWeaponModifierDone(Pawn pawn) // [!] 20070905 dEAthcURe|HM // event function HmWeaponModifierDone(avaPawn pawn)
{
	bAdjustWeaponModifier = false;
	WeaponModifierDone(pawn);

	// Host Migration 이후 정비도 반영...
	SetMaintenanceRate( MaintenanceRate );
}
// }} 20070212 dEAthcURe|HM

//=================================================================================================
// Modifier 적용을 위한 function...
//=================================================================================================
simulated function ChagneBodySkin( string SkinName )	{	WeaponSkin[0] = SkinName;	}
simulated function ChangeStockSkin( string SkinName )	{	WeaponSkin[1] = SkinName;	}
simulated function ChangeGripSkin( string SkinName )	{	WeaponSkin[2] = SkinName;	}

simulated function AddItemMesh( string MeshName, name SocketName, float MaxVisibleDistance )
{
	local int id;
	id									= ItemParts.length;
	ItemParts.length					= id + 1;
	ItemParts[id].MeshName				= MeshName;
	ItemParts[id].SocketName			= SocketName;
	ItemParts[id].MaxVisibleDistance	= MaxVisibleDistance;
}

// Modified 된 것들을 종합하여 Component 를 만들고 Material 등을 적용해 준다
simulated function CreateComponent()
{
	//if ( bAdjustWeaponModifier == false )
	//{
	//	// Warning Message -> Modifier 가 적용되기 전에 CreateComponent 가 호출 되었음 
	//	`Warn( "###################################################################################################" );
	//	`Warn( "avaWeapon CreateCompnent before Adjust Modifiers" @self );
	//	`Warn( "###################################################################################################" );
	//}
	//else
	//{
	//	`Log( "avaWeapon CreateCompnent" @self );
	//}
	CreateBaseMesh();
	ArmComponent	= CreateExtraMesh( ArmMeshName, ArmSkinName );
	HandComponent	= CreateExtraMesh( HandMeshName, HandSkinName );
	AttachItems();	
	ChangeSkin();
	ChangeVisibility(true);
}

simulated function CreateBaseMesh()
{
	local SkeletalMesh	tmpSkelMesh;
	local AnimSet		tmpAnimSet;
	local SkeletalMeshComponent	BaseSkelMesh;

	BaseSkelMesh = SkeletalMeshComponent( Mesh );

	if ( BaseSkelMeshName != "" )
	{
		tmpSkelMesh = SkeletalMesh( DynamicLoadObject( BaseSkelMeshName, class'SkeletalMesh' ) );
		if ( tmpSkelMesh != None )	BaseSkelMesh.SetSkeletalMesh( tmpSkelMesh );
		else						`warn( "avaWeapon.CreateBaseMesh Cannot Load SkeletalMesh" @BaseSkelMeshName );
	}

	if ( BaseAnimSetName != "" )
	{
		tmpAnimSet	= AnimSet( DynamicLoadObject( BaseAnimSetName, class'AnimSet' ) );
		if ( tmpAnimSet != None )
		{
			BaseSkelMesh.AnimSets.length = 1;
			BaseSkelMesh.AnimSets[0] = tmpAnimSet;
		}
		else
		{
			`warn( "avaWeapon.CreateBaseMesh Cannot Load AnimSet" @BaseAnimSetName );
		}
	}
}

static function DLO( string resource, out array< object > outList )
{
	local object obj;
	if (resource != "")
	{		
		obj = DynamicLoadObject( resource, class'Object' );
		if ( obj != None )
		{
			outList.length = outList.length + 1;
			outList[ outList.length - 1 ] = obj;
		}
	}
}

static simulated function PreCache( out array<object> outList )
{
	local int i;	
	Super.PreCache( outList );
	
	DLO( default.BaseSkelMeshName, outList );
	DLO( default.BaseAnimSetName, outList );
	DLO( default.ArmSkinName, outList );
	DLO( default.HandSkinName, outList );

	for ( i = 0 ; i < default.ItemParts.Length ; ++ i )
	{
		DLO( default.ItemParts[i].MeshName, outList );
	}

	for ( i = 0 ; i < `MAX_WEAPON_SKIN ; ++ i )
	{
		if ( default.WeaponSkin[i] == "" )	continue;	
		DLO( default.WeaponSkin[i], outList );
	}

	DLO( default.ArmMeshName, outList );
	DLO( default.HandMeshName, outList );

	DLO( default.EUArmMeshName, outList );
	DLO( default.NRFArmMeshName, outList );
	DLO( default.DefaultHandMeshName, outList );
}

static event LoadDLOs()
{
	local array< object > outList;
	Precache( outList );
}

simulated function SkeletalMeshComponent CreateExtraMesh( string MeshName, string SkinName )
{
	local Material					tmpMaterial;
	local SkeletalMeshComponent		BaseMesh;
	local SkeletalMeshComponent		ExtraMesh;
	
	BaseMesh = SkeletalMeshComponent( Mesh );

	ExtraMesh = new(outer) class'avaSkeletalMeshComponent';
	ExtraMesh.bUseAsOccluder = FALSE;
	ExtraMesh.SetShadowParent( baseMesh );
	ExtraMesh.SetOcclusionGroup( baseMesh );
	ExtraMesh.SetParentAnimComponent( baseMesh );
	ExtraMesh.SetDepthPriorityGroup( SDPG_Foreground );
	ExtraMesh.SetOnlyOwnerSee( true );
	ExtraMesh.bUpdateSkelWhenNotRendered = false;
	ExtraMesh.bCastDynamicShadow			= false;
	baseMesh.AttachComponent( ExtraMesh, 'Bip01' );

	if ( SkinName != "" )
	{
		tmpMaterial = Material( DynamicLoadObject( SkinName, class'Material' ) );
		if ( tmpMaterial != None )	ExtraMesh.SetMaterial( 0, tmpMaterial );
		else	`warn( "avaWeapon.CreateExtraMesh Cannot Load Material" @SkinName );
	}	

	return ExtraMesh;
}

// DotSight 등 필요한 Item 들이 있다면 Weapon 에 붙이도록 한다. 
// 단 Silencer 와 같이 게임중의 visibility 가 바뀌는 것들은 별개로 처리하도록 한다.
simulated function AttachItems()
{
	local int i;
	local StaticMesh			tempStaticMesh;
	local SkeletalMeshComponent	BaseMesh;
	BaseMesh = SkeletalMeshComponent( Mesh );
	for ( i = 0 ; i < ItemParts.Length ; ++ i )
	{
		if ( ItemParts[i].MeshName != "" )
		{
			// Create the Item Mesh if needed
			if ( ItemParts[i].ItemMesh == None )
			{
				TempStaticMesh = StaticMesh( DynamicLoadObject( ItemParts[i].MeshName, class'StaticMesh') );
				ItemParts[i].ItemMesh = new(outer) class'StaticMeshComponent';
				ItemParts[i].ItemMesh.bUseAsOccluder = FALSE;
				ItemParts[i].ItemMesh.SetStaticMesh( TempStaticMesh );
			}

			// Apply Item Part
			if ( ItemParts[i].ItemMesh != None )
			{
				ItemParts[i].ItemMesh.SetShadowParent( Mesh );
				ItemParts[i].ItemMesh.SetOcclusionGroup( Mesh );				
				ItemParts[i].ItemMesh.SetDepthPriorityGroup( SDPG_Foreground );
				ItemParts[i].ItemMesh.SetOnlyOwnerSee( true );
			}

			// add it to the components array if it's not there already
			if ( !ItemParts[i].ItemMesh.bAttached )
				BaseMesh.AttachComponentToSocket( ItemParts[i].ItemMesh, ItemParts[i].SocketName );
		}
	}
}

simulated function SetLightEnvironment( LightEnvironmentComponent env )
{
	local int i;	

	for ( i = 0 ; i < ItemParts.Length ; ++ i )
	{
		// Apply Item Part
		if ( ItemParts[i].ItemMesh != None )
		{			
			ItemParts[i].ItemMesh.SetLightEnvironment( env );				
		}					
	}

	if (ArmComponent != none)
		ArmComponent.SetLightEnvironment( env );

	if (HandComponent != none)
		HandComponent.SetLightEnvironment( env );

	if (Mesh != none)
		Mesh.SetLightEnvironment( env );
}




/*********************************************************************************************
 * Initialization / System Messages / Utility
 *********************************************************************************************/

/**
 * Initialize the weapon
 */
// GIveTo 함수 가 inventory 에서 final 로 선언되어 있다.
// bDoNotActivate 를 사용할 수가 없다
// GiveToEx 를 대체해서 쓴다.
function GiveToEx( Pawn Other, optional bool bDoNotActivate )
{
	if ( Other != None && Other.InvManager != None )
	{
		Other.InvManager.AddInventory( Self, bDoNotActivate );
	}
}

/**
 * Each Weapon needs to have a unique InventoryWeight in order for weapon switching to
 * work correctly.  This function calculates that weight using the various inventory values
 * and the item name.
 */
simulated function CalcInventoryWeight()
{
	local int i;

	// Figure out the weight

	InventoryWeight = ((InventoryGroup+1) * 1000) + (GroupWeight * 100);
	for (i=0;i<Len(ItemName);i++)
	{
		InventoryWeight += float(asc(mid(ItemName,i,1))) / 1000;
	}

	if ( Priority < 0 )
		Priority = InventoryWeight;
}

/**
 * Material control
 *
 * @Param 	NewMaterial		The new material to apply or none to clear it
 */

simulated function SetSkin(Material NewMaterial)
{
	//local int i,Cnt;

	//if ( NewMaterial == None )	// Clear the materials
	//{
	//	if ( default.Mesh.Materials.Length > 0 )
	//	{
	//		Cnt = Default.Mesh.Materials.Length;
	//		for (i=0;i<Cnt;i++)
	//		{
	//			Mesh.SetMaterial( i, Default.Mesh.GetMaterial(i) );
	//		}
	//	}
	//	else if (Mesh.Materials.Length > 0)
	//	{
	//		Cnt = Mesh.Materials.Length;
	//		for ( i=0; i < Cnt; i++ )
	//		{
	//			Mesh.SetMaterial(i,none);
	//		}
	//	}
	//}
	//else
	//{
	//	if ( default.Mesh.Materials.Length > 0 || mesh.GetDefaultMaterialCount() > 0 )
	//	{
	//		Cnt = default.Mesh.Materials.Length > 0 ? default.Mesh.Materials.Length : mesh.GetDefaultMaterialCount();
	//		for ( i=0; i < Cnt; i++ )
	//		{
	//			Mesh.SetMaterial(i,NewMaterial);
	//		}
	//	}
	//}
}

/*********************************************************************************************
 * Weapon Adjustment Functions
 *********************************************************************************************/

`devexec simulated function AdjustMesh(string cmd)
{
	local string c,v;
	local vector t,s,o,k;
	local rotator r;
	local float sc;

	c = left(Cmd,InStr(Cmd,"="));
	v = mid(Cmd,InStr(Cmd,"=")+1);

	t  = PlayerViewOffset;//Mesh.Translation;
	r  = Mesh.Rotation;
	s  = Mesh.Scale3D;
	o  = FireOffset;
	sc = Mesh.Scale;
	k  = Mesh.Translation;

	if (c~="x")  t.X += float(v);
	if (c~="ax") t.X =  float(v);
	if (c~="y")  t.Y += float(v);
	if (c~="ay") t.Y =  float(v);
	if (c~="z")  t.Z += float(v);
	if (c~="az") t.Z =  float(v);

	if (c~="kx")  k.X += float(v);
	if (c~="kax") k.X =  float(v);
	if (c~="ky")  k.Y += float(v);
	if (c~="kay") k.Y =  float(v);
	if (c~="kz")  k.Z += float(v);
	if (c~="kaz") k.Z =  float(v);


	if (c~="ox")  o.X += float(v);
	if (c~="oax") o.X =  float(v);
	if (c~="oy")  o.Y += float(v);
	if (c~="oay") o.Y =  float(v);
	if (c~="oz")  o.Z += float(v);
	if (c~="oaz") o.Z =  float(v);


	if (c~="r")   R.Roll  += int(v);
	if (c~="ar")  R.Roll  =  int(v);
	if (c~="p")   R.Pitch += int(v);
	if (c~="ap")  R.Pitch =  int(v);
	if (c~="w")   R.Yaw   += int(v);
	if (c~="aw")  R.Yaw   =  int(v);

	if (c~="scalex") s.X = float(v);
	if (c~="scaley") s.Y = float(v);
	if (c~="scalez") s.Z = float(v);

	if (c~="scale") sc = float(v);

	if (c~="resetscale")
	{
		sc = 1.0;
		s.X = 1.0;
		s.Y = 1.0;
		s.Z = 1.0;
	}

	//Mesh.SetTranslation(t);
	PlayerViewOffset = t;
	FireOffset = o;
	Mesh.SetTranslation(k);
	Mesh.SetRotation(r);
	Mesh.SetScale(sc);
	Mesh.SetScale3D(s);	


	`log("#### AdjustMesh ####");
	`log("####    View Offset :"@PlayerViewOffset);
	`log("####    Translation :"@Mesh.Translation);
	`log("####    Effects     :"@FireOffset);
	`log("####    Rotation    :"@Mesh.Rotation);
	`log("####    Scale3D     :"@Mesh.Scale3D);
	`log("####    scale       :"@Mesh.Scale);
	`log("####    FireOffset	 :"@FireOffset);	
}

`devexec function AdjustFire(string cmd)
{
	local string c,v;
	local vector t;

	c = left(Cmd,InStr(Cmd,"="));
	v = mid(Cmd,InStr(Cmd,"=")+1);

	`log( "FireOffset : " $FireOffset );
	t  = FireOffset;

	if (c~="x")  t.X += float(v);
	if (c~="ax") t.X =  float(v);
	if (c~="y")  t.Y += float(v);
	if (c~="ay") t.Y =  float(v);
	if (c~="z")  t.Z += float(v);
	if (c~="az") t.Z =  float(v);

	FireOffset = t;
	`log("#### FireOffset ####");
	`log( "FireOffset : " $FireOffset );

}


/*********************************************************************************************
 * Hud/Crosshairs
 *********************************************************************************************/

/**
 * Access to HUD and Canvas.
 * Event always called when the InventoryManager considers this Inventory Item currently "Active"
 * (for example active weapon)
 *
 * @param	HUD			- HUD with canvas to draw on
 */
simulated function ActiveRenderOverlays( HUD H )
{
	if ( Instigator == None )	return;
	if ( H.bShowHUD == false )	return;
//	if ( !avaPawn(Instigator).bIsDash )	DrawWeaponCrosshair( H );
}

/**
 * Draw the Crosshairs
 */
simulated function DrawWeaponCrosshair( Hud HUD )
{
	local	float	CrosshairSize;
	local avaHUD		H;

	H = avaHUD(HUD);
	if ( H == None )
		return;

	if ( CrossHairMtrl == None )
	{
		CrosshairSize = 4;
		if ( H.LastPickupTime > WorldInfo.TimeSeconds - 0.4 )
		{
			if ( H.LastPickupTime > WorldInfo.TimeSeconds - 0.2 )
				CrosshairSize *= (1 + 5 * (WorldInfo.TimeSeconds - H.LastPickupTime));
			else
				CrosshairSize *= (1 + 5 * (H.LastPickupTime + 0.4 - WorldInfo.TimeSeconds));
		}

		// Draw Temporary Crosshair
		H.Canvas.DrawColor	= WeaponColor;
		H.Canvas.SetPos((0.5 * H.Canvas.ClipX) - CrosshairSize, 0.5 * H.Canvas.ClipY);
		H.Canvas.DrawRect(2*CrosshairSize + 1, 1);

		H.Canvas.SetPos(0.5 * H.Canvas.ClipX, (0.5 * H.Canvas.ClipY) - CrosshairSize);
		H.Canvas.DrawRect(1, 2*CrosshairSize + 1);
	}
	else
	{
		H.Canvas.SetDrawColor( 255,255,255,255 );
		H.Canvas.SetPos((0.5 * H.Canvas.ClipX) - CrossHairSizeX/2, 0.5 * H.Canvas.ClipY - CrossHairSizeY/2 );
		H.Canvas.DrawTile(CrossHairMtrl, CrossHairSizeX, CrossHairSizeY, 0, 0, CrossHairMtrl.SizeX, CrossHairMtrl.SizeY);	
	}
}

simulated function int GetAmmoCount()
{
	return AmmoCount;
}


/**
 * list important Weapon variables on canvas.  HUD will call DisplayDebug() on the current ViewTarget when
 * the ShowDebug exec is used
 *
 * @param	HUD			- HUD with canvas to draw on
 * @input	out_YL		- Height of the current font
 * @input	out_YPos	- Y position on Canvas. out_YPos += out_YL, gives position to draw text for next debug line.
 */
simulated function DisplayDebug(HUD HUD, out float out_YL, out float out_YPos)
{
	//local string T;

	super.DisplayDebug(HUD, out_YL, out_YPos);

	//if ( Instigator == None ) return;

	//T = "Eyeheight "$Instigator.EyeHeight$" base "$Instigator.BaseEyeheight$" landbob "$avaPawn(Instigator).Landbob$" just landed "$avaPawn(Instigator).bJustLanded$" land recover "$avaPawn(Instigator).bLandRecovery;
 //   HUD.Canvas.DrawText(T, false);
 //   out_YPos += out_YL;
}


/*********************************************************************************************
 * Attachments / Effects / etc
 *********************************************************************************************/
/**
 * Returns interval in seconds between each shot, for the firing state of FireModeNum firing mode.
 *
 * @param	FireModeNum	fire mode
 * @return	Period in seconds of firing mode
 */
simulated function float GetFireInterval( byte FireModeNum )
{
	return FireInterval[FireModeNum] * ((avaPawn(Owner)!= None) ? avaPawn(Owner).FireRateMultiplier : 1.0);
}

/**
 * PlayFireEffects Is the root function that handles all of the effects associated with
 * a weapon.  This function creates the 1st person effects.  It should only be called
 * on a locally controlled player.
 */
simulated function PlayFireEffects( byte FireModeNum, optional vector HitLocation )
{
	// Play Weapon fire animation
	if ( FireModeNum == 0 )
	{
		if ( WeaponFireAnim.length > 0 )
			PlayWeaponAnimation( WeaponFireAnim[Rand(WeaponFireAnim.length)], 0.0 );
	}
	else if ( FireModeNum == 1 )
	{
		if ( WeaponAltFireAnim.length > 0 )
			PlayWeaponAnimation( WeaponAltFireAnim[Rand(WeaponAltFireAnim.length)], 0.0 );
	}
	//PlayWeaponAnimation( WeaponFireAnim, GetFireInterval(FireModeNum) );

	// Start muzzle flash effect
	CauseMuzzleFlash();

	ShakeView();
}

simulated function StopFireEffects(byte FireModeNum)
{
	StopMuzzleFLash();
}

/** plays view shake on the owning client only */
simulated function ShakeView()
{
	local avaPlayerController PC;

	PC = avaPlayerController(Instigator.Controller);
	if (PC != None && LocalPlayer(PC.Player) != None)
	{
		PC.ShakeView(FireShake);
	}
}

/**
 * Tells the weapon to play a firing sound (uses CurrentFireMode)
 */
simulated function PlayFiringSound()
{
	// play weapon fire sound
	if ( WeaponFireSnd != None )
	{
		MakeNoise(1.0);
		
		WeaponPlaySound( WeaponFireSnd,,,,true );
	}

	InvManager.OwnerEvent('WeaponFiringSound');
}

/**
 * Turns the MuzzleFlashlight off
 */
simulated event MuzzleFlashTimer()
{
	if ( MuzzleFlashLight != None )
	{
		MuzzleFlashLight.SetEnabled(FALSE);
	}
	if (MuzzleFlashMesh != none)
	{
		MuzzleFlashMesh.SetHidden(true);
	}

	if ( !bMuzzleFlashPSCLoops )
	{
		if ( MuzzleFlashPSC != none )
			MuzzleFlashPSC.DeactivateSystem();
		if ( AbsMuzzleFlashPSC != none )
			AbsMuzzleFlashPSC.DeactivateSystem();
	}
}

simulated function CauseMuzzleFlashLight()
{
	local SkeletalMeshComponent SKMesh;
	SKMesh = SkeletalMeshComponent(Mesh);
	MuzzleFlashLight = new(self) MuzzleFlashLightClass;
	if ( SKMesh != None && SKMesh.GetSocketByName(MuzzleFlashSocket) != None )
	{
		SKMesh.AttachComponentToSocket(MuzzleFlashLight, MuzzleFlashSocket);
	}	
}

/** Returns TRUE, if muzzle flash effects should be played. */
simulated function bool IsMuzzleFlashRelevant()
{
	local float MuzzleFlashRadius;

	// Always should muzzle flashes on the player we're controlling or if the instigator is none (dummy fire)
	if( Instigator == None || (Instigator.IsHumanControlled() && Instigator.IsLocallyControlled()) )
	{
		return TRUE;
	}

	// If we have a muzzle flash light, use its radius as an indication
	MuzzleFlashRadius = MuzzleFlashLight == None ? 256.f : MuzzleFlashLight.Radius + 60.f;

	// if frame rate is really bad and Pawn hasn't been rendered since last second, then don't display effects
	if( WorldInfo.bAggressiveLOD && TimeSince(Instigator.LastRenderTime) > 1.f )
	{
		return FALSE;
	}

	// If Instigator hasn't been rendered for 2 seconds and camera isn't really close to the instigator, then don't play muzzle flash effects
	if( TimeSince(Instigator.LastRenderTime) > 2.f && !IsCameraWithinRadius(Instigator.Location, MuzzleFlashRadius) )
	{
		return FALSE;
	}

	return TRUE;
}

simulated function bool IsCameraWithinRadius(Vector TestLocation, float Radius)
{
	local PlayerController		PC;
	local Vector	CamLoc;
	local Rotator	CamRot;

	if( WorldInfo.NetMode == NM_DedicatedServer )
	{
		return FALSE;
	}

	ForEach LocalPlayerControllers(PC)
	{
		avaPlayerController(PC).GetPlayerViewPoint(CamLoc, CamRot);

		if( VSize(TestLocation - CamLoc) <= Radius )
		{
			return TRUE;
		}
	}

	return FALSE;
}


/**
 * Causes the muzzle flashlight to turn on and setup a time to
 * turn it back off again.
 */

simulated event CauseMuzzleFlash()
{
	local vector	l;
	local rotator	r;
	if (IsMuzzleFlashRelevant())
	{
		if ( MuzzleFlashLight == None )
		{
			if ( MuzzleFlashLightClass != None )
				CauseMuzzleFlashLight();
		}
		else
		{
			MuzzleFlashLight.ResetLight();
			MuzzleFlashLight.CastDynamicShadows = bDynamicMuzzleFlashes;
		}

		if ( !bMuzzleFlashPSCLoops || MuzzleFlashPSC.bWasDeactivated )
		{
			if (Instigator != None && Instigator.FiringMode == 1 )
			{
				if ( MuzzleFlashPSC != none && MuzzleFlashAltPSCTemplate != None )
					MuzzleFlashPSC.SetTemplate(MuzzleFlashAltPSCTemplate);
				if ( AbsMuzzleFlashPSC != none && AbsMuzzleFlashAltPSCTemplate != None )
					AbsMuzzleFlashPSC.SetTemplate(AbsMuzzleFlashAltPSCTemplate);
			}
			else
			{
				if ( MuzzleFlashPSC != none && MuzzleFlashPSCTemplate != None )
					MuzzleFlashPSC.SetTemplate(MuzzleFlashPSCTemplate);
				if ( AbsMuzzleFlashPSC != none && AbsMuzzleFlashPSCTemplate != None )
					AbsMuzzleFlashPSC.SetTemplate(AbsMuzzleFlashPSCTemplate);
			}
			
			if ( MuzzleFlashPSC != None )
			{
				MuzzleFlashPSC.SetVectorParameter('MFlashScale',Vect(0.5,0.5,0.5));
				MuzzleFlashPSC.ActivateSystem();
			}

			if ( AbsMuzzleFlashPSC != None )
			{
				if ( SkeletalMeshComponent(Mesh) != None && SkeletalMeshComponent(Mesh).GetSocketWorldLocationAndRotation( MuzzleFlashSocket, l, r ) )
				{
					`log( "avaWeapon.CauseMuzzleFlash Activate AbsMuzzleFlashPSC" @l @r );
					AbsMuzzleFlashPSC.SetTranslation( l );
					AbsMuzzleFlashPSC.SetRotation( r );
					AbsMuzzleFlashPSC.ActivateSystem();
				}
			}
		}

		if (MuzzleFlashMesh != none)
			MuzzleFlashMesh.SetHidden(false);

		// Set when to turn it off.
		if ( MuzzleFlashDuration != 0.0 )
			SetTimer(MuzzleFlashDuration,false,'MuzzleFlashTimer');
	}
}

simulated event StopMuzzleFlash()
{
	ClearTimer('MuzzleFlashTimer');
	MuzzleFlashTimer();

	if ( MuzzleFlashPSC != none )
	{
		MuzzleFlashPSC.DeactivateSystem();
	}
	if ( AbsMuzzleFlashPSC != none )
	{
		AbsMuzzleFlashPSC.DeactivateSystem();
	}
}


/**
 * Sets the timing for putting a weapon down.  The WeaponIsDown event is trigged when expired
*/
simulated function TimeWeaponPutDown()
{
	if( Instigator.IsFirstPerson() )
	{
		PlayWeaponPutDown();
	}

	if ( PutDownTime > 0 )
		SetTimer( PutDownTime , false, 'WeaponIsDown' );
	else
		WeaponIsDown();
	//super.TimeWeaponPutDown();
}

/**
 * Show the weapon being put away
 */
simulated function PlayWeaponPutDown()
{
	// Play the animation for the weapon being put down

	if ( WeaponPutDownAnim != '' )
		PlayWeaponAnimation( WeaponPutDownAnim, PutDownTime );

	// play any associated sound
	if ( WeaponPutDownSnd != None )
		WeaponPlaySound( WeaponPutDownSnd );
}

/**
 * Sets the timing for equipping a weapon.
 * The WeaponEquipped event is trigged when expired
 */
simulated function TimeWeaponEquipping()
{
	// The weapon is equipped, attach it to the mesh.
	AttachWeaponTo( Instigator.Mesh );

	PlayWeaponEquip();	

	SetTimer( GetEquipTime() , false, 'WeaponEquipped');
}

simulated function float GetEquipTime()
{
	local float ETime;

	ETime = EquipTime>0 ? EquipTime : 0.01;
	if ( PendingFire(0) || PendingFire(1) )
	{
		ETime += 0.25;
	}
	return ETime * avaPawn( Instigator ).WeapTypeAmp[WeaponType].EquipAmp;
}

/**
 * Show the weapon begin equipped
 */
simulated function PlayWeaponEquip()
{
	/// 2006/3/14 ; deif
	/// 1인칭이 아닐때도 소리는 나야한다!!
	/// 따라서, animation에만 if문을 적용
	if ( WeaponEquipAnim != '' )
	{
		PlayWeaponAnimation( WeaponEquipAnim, GetEquipTime() );
		SkeletalMeshComponent( Mesh ).ForceSkelUpdate();
	}
}

/// 2005/3/14 ; deif
/// animNotify에서 replicate to all clients 하는 부분이 있기에 dedicated server에서도 playAnim을 불러야 한다
simulated function AnimNodeSequence PlayWeaponAnimation( Name Sequence, float fDesiredDuration, optional bool bLoop, optional SkeletalMeshComponent SkelMesh)
{
	if ( SkelMesh == None )
	{
		SkelMesh = SkeletalMeshComponent(Mesh);
	}
	
	// Check we have access to mesh and animations
	if( SkelMesh == None || GetWeaponAnimNodeSeq() == None )
	{
		return None;
	}

	// @todo - this should call GetWeaponAnimNodeSeq, move 'duration' code into AnimNodeSequence and use that.
	return SkelMesh.PlayAnim(Sequence, fDesiredDuration, bLoop);
}

simulated function ChangeSkin()
{
	local int			i;
	local Material		tmpMtrl;
	for ( i = 0 ; i < `MAX_WEAPON_SKIN ; ++ i )
	{
		if ( WeaponSkin[i] == "" )	continue;	// 적용할 Material 이 없음
		tmpMtrl = Material( DynamicLoadObject( WeaponSkin[i], class'Material' ) );
		if ( tmpMtrl != None )	Mesh.SetMaterial( i, tmpMtrl );
		else					`warn( "avaWeapon.ApplySkin Cannot Load Material" @WeaponSkin[i] );
	}	
}

 /**
 * Attach Weapon Mesh, Weapon MuzzleFlash and Muzzle Flash Dynamic Light to a SkeletalMesh
 *
 * @param	who is the pawn to attach to
 */
simulated function AttachWeaponTo( SkeletalMeshComponent MeshCpnt, optional Name SocketName )
{
	local string	PrvArmMeshName, PrvHandMeshName;

	PrvArmMeshName	= ArmMeshName;
	PrvHandMeshName = HandMeshName;

	SetSkin( None );

	HandMeshName	= DefaultHandMeshName;
	ArmMeshName		= avaCharacter(Instigator).DefaultTeam == 0 ? EUArmMeshName : NRFArmMeshName;

	// Component 의 생성은 한번만 하면 된다...
	if ( bInitializedMesh == false )
	{
		CreateComponent();
		bInitializedMesh = true;
	}

	// Arm 이나 Hand 는 잡은 사람에 따라 달라지므로 AttachWeaponTo 에서 매번 해줘야 한다.
	if ( ArmMeshName != PrvArmMeshName )	ArmComponent.SetSkeletalMesh( SkeletalMesh( DynamicLoadObject( ArmMeshName, class'SkeletalMesh' ) ) );
	if ( HandMeshName != PrvHandMeshName )	HandComponent.SetSkeletalMesh( SkeletalMesh( DynamicLoadObject( HandMeshName, class'SkeletalMesh' ) ) );

	if ( avaPawn(Instigator).LODBias >= 1 )
	{
		ArmComponent.ForcedLodModel						= 2;
		HandComponent.ForcedLodModel					= 2;
		SkeletalMeshComponent( Mesh ).ForcedLodModel	= 2;
	}

	
	// Spawn the 3rd Person Attachment
	if (Role==ROLE_Authority && avaPawn(Instigator) != None)
	{
		avaPawn(Instigator).ChangeWeaponAttachment( Self, true );
		avaPawn(Instigator).ChangeWeaponState( GetWeaponState() );
	}

	SetLightEnvironment( avaCharacter(Instigator).LightEnvironment );

	// Attach 1st Person Muzzle Flashes, etc,
	if ( Instigator.IsFirstPerson() )
	{
		SetHidden( false );
		AttachMuzzleFlash();
	}
	else
	{
		SetHidden( true );
	}
}

simulated function AttachMuzzleFlash()
{
	local SkeletalMeshComponent SKMesh;
	// Attach the Muzzle Flash
	SKMesh = SkeletalMeshComponent(Mesh);

	if ( SkMesh.GetSocketByName( MuzzleFlashSocket ) == None )	return;
	if (  SKMesh != none )
	{
		// Muzzle Flash mesh
		if ( MuzzleFlashMesh != None )
			SKMesh.AttachComponentToSocket(MuzzleFlashMesh, MuzzleFlashSocket);

		if (MuzzleFlashPSCTemplate != none)
		{
			MuzzleFlashPSC  = new(self) class'avaParticleSystemComponent';
			SKMesh.AttachComponentToSocket(MuzzleFlashPSC, MuzzleFlashSocket);
			MuzzleFlashPSC.SetDepthPriorityGroup(SDPG_Foreground);
			MuzzleFlashPSC.DeactivateSystem();
			MuzzleFlashPSC.SetColorParameter('MuzzleFlashColor', MuzzleFlashColor);
		}

		if (AbsMuzzleFlashPSCTemplate != None || AbsMuzzleFlashAltPSCTemplate != None)
		{
			AbsMuzzleFlashPSC = new(self) class'avaParticleSystemComponent';
			AttachComponent(AbsMuzzleFlashPSC);
			AbsMuzzleFlashPSC.SetAbsolute(true,true);
			AbsMuzzleFlashPSC.bAutoActivate = FALSE;  
		}

		// Muzzle Flash dynamic light
		if ( MuzzleFlashLight != None )
			SKMesh.AttachComponentToSocket(MuzzleFlashLight, MuzzleFlashSocket);
	}
}

function int GetWeaponState()
{
	return 0;
}

/**
 * Detach weapon from skeletal mesh
 *
 * @param	SkeletalMeshComponent weapon is attached to.
 */
simulated function DetachWeapon()
{
	local avaPawn P;
	StopWeaponAnimation();
	SetSkin(None);
	P = avaPawn(Instigator);
	if (Role == ROLE_Authority && P != None)
	{
		P.ChangeWeaponAttachment( self, false );
	}
	SetHidden(True);
	DetachMuzzleFlash();
}

simulated function DetachMuzzleFlash()
{
	local SkeletalMeshComponent SKMesh;
	SKMesh = SkeletalMeshComponent(Mesh);
	if (  SKMesh != none )
	{
		// Muzzle Flash Mesh
		if ( MuzzleFlashMesh != None )
			SKMesh.DetachComponent( MuzzleFlashMesh );

		if (MuzzleFlashPSC != none)
			SKMesh.DetachComponent( MuzzleFlashPSC );

		if (AbsMuzzleFlashPSC != none)
			DetachComponent( AbsMuzzleFlashPSC );

		// Muzzle Flash dynamic light
		if ( MuzzleFlashLight != None )
			SKMesh.DetachComponent( MuzzleFlashLight );
	}
}

// Test Console 
`devexec simulated function ToggleVisibility()
{
	if ( Mesh != None )
	{
		if ( Mesh.HiddenGame )	ChangeVisibility( true );
		else					ChangeVisibility( false );
	}
}

/// AVA는 Mesh아래 붙어있는 것들이 더 있다. Show/Hide recursive가 안되는 관계로!!
simulated function ChangeVisibility(bool bIsVisible)
{
	local int i;
	local PrimitiveComponent primitivecomponent;
	if (Mesh != None)
	{
		Mesh.SetHidden(!bIsVisible);
		for (i = 0; i < SkeletalMeshComponent(Mesh).Attachments.Length; i++)
		{
			primitivecomponent = PrimitiveComponent( SkeletalMeshComponent(Mesh).Attachments[i].Component );
			if ( primitivecomponent == None ) continue;
			primitivecomponent.SetHidden( !bIsVisible );
		}
	}
}

/*********************************************************************************************
 * Pawn/Controller/View functions
 *********************************************************************************************/


simulated function GetViewAxes( out vector xaxis, out vector yaxis, out vector zaxis )
{
	if ( Instigator.Controller == None )
	{
	GetAxes( Instigator.Rotation, xaxis, yaxis, zaxis );
    }
    else
    {
	GetAxes( Instigator.Controller.Rotation, xaxis, yaxis, zaxis );
    }
}

/**
 * This function is called whenever you attempt to reselect the same weapon
 */
reliable server function ServerReselectWeapon();


/**
 * Returns true if this item can be thrown out.
 */
simulated function bool CanThrow()
{
	// cass 는 ammo 가 없어도 던질 수 있다.
	return bCanThrow;
	//return bCanThrow && HasAnyAmmo();
}


/**
 * Called from pawn to tell the weapon it's being held in hand NewWeaponHand
 */
simulated function SetHand(avaPawn.EWeaponHand NewWeaponHand)
{
	// FIXME:: Invert the mesh
}

/**
 * Returns the current Weapon Hand
 */
simulated function avaPawn.EWeaponHand GetHand()
{
	// Get the Weapon Hand from the pawn or default to HAND_Right
	if ( avaPawn(Instigator)!=none )
	{
		return avaPawn(Instigator).WeaponHand;
	}
	else
		return HAND_Right;
}
/**
 * This function aligns the gun model in the world
 */
simulated function SetPosition(avaPawn Holder)
{
	local vector DrawOffset, ViewOffset;
	local avaPawn.EWeaponHand CurrentHand;
	local rotator NewRotation;
	if ( Instigator == None || Holder == None )	return;
	if ( !Instigator.IsFirstPerson() )
		return;
	// Hide the weapon if hidden
	CurrentHand = GetHand();
	if ( CurrentHand == HAND_Hidden)
	{
		SetHidden( true );
		return;
	}
	SetHidden( false );
	// Adjust for the current hand
	ViewOffset = PlayerViewOffset;
	switch ( CurrentHand )
	{
		case HAND_Left:
			ViewOffset.Y *= -1;
			break;

		case HAND_Centered:
			ViewOffset.Y = 0;
			break;
	}
	// Calculate the draw offset
	if ( Holder.Controller == None )
	{
		DrawOffset = (ViewOffset >> Rotation) + Holder.GetEyeHeight() * vect(0,0,1);
	}
	else
	{
		DrawOffset.Z = Holder.GetEyeHeight();
		if ( Holder.bWeaponBob )
		{
			if ( IsInState( 'WeaponDash' ) )
				DrawOffset += Holder.WeaponBob(BobDampingInDash,JumpDamping);
			else
				DrawOffset += Holder.WeaponBob(BobDamping,JumpDamping);
		}
		if ( avaPlayerController(Holder.Controller) != None )
		{
			DrawOffset += avaPlayerController(Holder.Controller).ShakeOffset >> Holder.Controller.Rotation;
		}
		DrawOffset = DrawOffset + ( ViewOffset >> Holder.Controller.Rotation );
	}
	// Adjust it in the world
	SetLocation( Holder.Location + DrawOffset );
	// 2006/7/20  아래처럼 하면 PunchAngle 이 적용이 안되기 때문에 수정하였음... by OZ
	Holder.Controller.GetPlayerViewPoint( ViewOffset, NewRotation );
	SetRotation(NewRotation);
}

simulated function int LagRot(int NewValue, int OldValue, int MaxDiff )
{
	if ( NewValue ClockWiseFrom OldValue )
	{
		if ( OldValue > NewValue )	OldValue -= 65536;
		// NewValue 의 값이 항상 큼
		if ( NewValue - OldValue > MaxDiff )
			OldValue = NewValue - MaxDiff;
	}
	else
	{
		if ( NewValue > OldValue )	NewValue -= 65536;
		// OldValue 의 값이 항상 큼
		if ( OldValue - NewValue > MaxDiff )
			OldValue = NewValue + MaxDiff;
	}
	return OldValue + RotLagSpeed * (NewValue - OldValue);
}

/**
 * called every time owner takes damage while holding this weapon - used by shield gun
 */
function AdjustPlayerDamage( out int Damage, Controller InstigatedBy, Vector HitLocation,
			     out Vector Momentum, class<DamageType> DamageType)
{
}

/**
 * BestMode()
 * choose between regular or alt-fire
 */
function byte BestMode()
{
	local byte Best;
	if ( IsFiring() )
		return CurrentFireMode;

	if ( FRand() < 0.5 )
		Best = 1;

//	if ( Best < bZoomedFireMode.Length && bZoomedFireMode[Best] != 0 )
//		return 0;
//	else
		return Best;
}

/*********************************************************************************************
 * Ammunition / Inventory
 *********************************************************************************************/

/**
 * Consumes some of the ammo
 */
function ConsumeAmmo( byte FireModeNum )
{
	local avaInventoryManager	avaInvManager;
	avaInvManager = avaInventoryManager( InvManager );
	// 연습채널에서는 Ammo 를 소모하지 않는다....
	if ( ( avaInvManager == None || !avaInvManager.bInfiniteAmmo ) && bInfinityAmmo == false && ( !avaGame(WorldInfo.Game).IsPracticeMode() || ConsumeAmmoWhenPracticeMode() ) )
	{
		AddAmmo(-1);
	}
}

function bool ConsumeAmmoWhenPracticeMode()
{
	return false;
}

/**
 * This function is used to add ammo back to a weapon.  It's called from the Inventory Manager
 */
function int AddAmmo( int Amount )
{
	AmmoCount = Clamp(AmmoCount + Amount,0,MaxAmmoCount);
	return AmmoCount;
}

/**
 * This function will fill the weapon up to it's maximum amount of ammo
 */
simulated function FillToInitialAmmo()
{
	AmmoCount = Max(AmmoCount,Default.AmmoCount);
}

/**
 * Retrusn true if the ammo is maxed out
 */
simulated function bool AmmoMaxed(int mode)
{
	return (AmmoCount >= MaxAmmoCount);
}

/**
 * This function checks to see if the weapon has any ammo availabel for a given fire mode.
 *
 * @param	FireModeNum		- The Fire Mode to Test For
 * @param	Amount			- [Optional] Check to see if this amount is available.  If 0 it will default to checking
 */
simulated function bool HasAmmo( byte FireModeNum, optional int Amount )
{
	if (Amount==0)
		return (AmmoCount >= 1);
	else
		return ( AmmoCount >= Amount );
}

/**
 * returns true if this weapon has any ammo
 */
simulated function bool HasAnyAmmo()
{
	return ( ( AmmoCount > 0 ) );
}
/**
 * This function retuns how much of the clip is empty.
 */
simulated function float DesireAmmo(bool bDetour)
{
	return (1.f - float(AmmoCount)/MaxAmmoCount);
}

/**
 * Returns true if the current ammo count is less than the default ammo count
 */
simulated function bool NeedAmmo()
{
	return ( AmmoCount < Default.AmmoCount );
}

/**
 * Cheat Help function the loads out the weapon
 *
 * @param 	bUseWeaponMax 	- [Optional] If true, this function will load out the weapon
 *							  with the actual maximum, not 999
 */
simulated function Loaded(optional bool bUseWeaponMax)
{
	if (bUseWeaponMax)
		AmmoCount = MaxAmmoCount;
	else
		AmmoCount = 999;
}

/**
 * Called when the weapon runs out of ammo during firing
 */

simulated function WeaponEmpty()
{
	// If we were firing, stop
	if (IsFiring())
	{
		GotoState('Active');
	}

	// make sure the client knows
	ClientWeaponEmpty();
}

function EquipWeapon()
{
	ClientEquipWeapon();
	ForceEquipWeapon();
}

reliable client simulated function ClientEquipWeapon()
{
	if ( Role < ROLE_Authority )
		ForceEquipWeapon();
}

simulated function ForceEquipWeapon()
{
	GotoState('WeaponEquipping');
}

function ResetWeapon()
{
	ClientResetWeapon();
	ForceResetWeapon();
}

reliable client simulated function ClientResetWeapon()
{
	if ( Role < ROLE_Authority )
		ForceResetWeapon();
}

simulated function ForceResetWeapon()
{
	GotoState( 'Active' );	
}

reliable client function ClientWeaponEmpty()
{
	// if we were firing, stop
	if (IsFiring())
	{
		GotoState('Active');
	}

	// 총알이 떨어진 경우 Weapon 을 Change 한다.
	
	//// Switch to best weapon
	//if( Pawn(Owner) != None )
	//{
	//	Instigator.InvManager.SwitchToBestWeapon( true );
	//}
}

/*********************************************************************************************
 * Firing
 *********************************************************************************************/

/*
   These 3 functions are overriden here to allow for stats collection.  If your custom weapon
   does not use these 3 base firing functions, you need to implement the stats tracking on
   your own by calling UpdateFireStats()
*/


simulated function GetFireLocAndRot( out vector loc, out rotator rot )
{
	loc = Instigator.GetPawnViewLocation();
	rot = Instigator.GetViewRotation();
}
/**
 * Fires a projectile.
 * Spawns the projectile, but also increment the flash count for remote client effects.
 * Network: Local Player and Server
 *
 * avaKProjectile 을 처리하기 위한 코드 추가 2006/02/09 by OZ
 */
simulated function Projectile ProjectileFire()
{
	local vector			RealStartLoc;
	local Projectile		SpawnedProjectile;

	// tell remote clients that we fired, to trigger effects
	IncrementFlashCount();

	if( Role == ROLE_Authority )
	{
		// this is the location where the projectile is spawned.
		RealStartLoc = GetPhysicalFireStartLoc();

		// Spawn projectile
		if ( WeaponProjectiles[CurrentFireMode] != None )
		{
			SpawnedProjectile = Spawn( WeaponProjectiles[CurrentFireMode], Self,, RealStartLoc,,,true);

			if( SpawnedProjectile != None && !SpawnedProjectile.bDeleteMe )
			{
				SpawnedProjectile.Init( Vector(GetAdjustedAim( RealStartLoc )) );
				if ( avaProjectile(SpawnedProjectile) != none )
				{
					avaProjectile(SpawnedProjectile).InitStats(self);
				}
			}
		}
		UpdateFiredStats(1);
	}
	return None;
}

simulated function CustomFire()
{
	UpdateFiredStats(1);
	Super.CustomFire();
}

/**
 * Update the stats for this weapon
 */

function UpdateFiredStats(int NoShots)
{
	local int i;
	local avaGame GI;
	GI = avaGame(WorldInfo.Game);
	if (GI != none && GI.GameStats != none )
	{
		for (i=0;i<NoShots;i++)
		{
			GI.GameStats.WeaponEvent(OwnerStatsID, class, CurrentFireMode, Instigator.PlayerReplicationInfo,'fired');
		}
	}
}

function UpdateHitStats(bool bDirectHit,bool bFriendlyHit,float distance)
{
	local avaGame GI;
	GI = avaGame(WorldInfo.Game);
	if (GI != none && GI.GameStats != none )
	{
		if ( bFriendlyHit == true )
		{
			GI.GameStats.WeaponEvent(OwnerStatsID, class, CurrentFireMode, Instigator.PlayerReplicationInfo,'FriendlyHit', distance);
		}
		else
		{
			GI.GameStats.WeaponEvent(OwnerStatsID, class, CurrentFireMode, Instigator.PlayerReplicationInfo,'hit', distance);
		}
	}
}



client reliable simulated function ClientEndFire(byte FireModeNum)
{
	if (Role != ROLE_Authority)
	{
		ClearPendingFire(FireModeNum);
		EndFire(FireModeNum);
	}
}

auto simulated state Inactive
{
	ignores OnAnimEnd;

	simulated function BeginState( Name PreviousStateName )
	{
		//NetUpdateFrequency = 0.1;
		Super.BeginState( PreviousStateName );

		if ( DestroyWeaponEmpty && !HasAnyAmmo() )
		{
			avaInventoryManager( Instigator.InvManager ).RequestRemoveFromInventory(self);
			LifeSpan = 1.0;
		}
	}
}

/*********************************************************************************************
 * State WeaponFiring
 * This is the default Firing State.  It's performed on both the client and the server.
 *********************************************************************************************/
simulated state WeaponFiring
{
	/**
	 * We override BeginFire() so that we can check for zooming and/or empty weapons
	 */

	simulated function BeginFire( Byte FireModeNum )
	{
		Global.BeginFire(FireModeNum);

		// No Ammo, then do a quick exit.
		if( !HasAmmo(FireModeNum) )
		{
			WeaponEmpty();
			return;
		}
	}

	/**
	 * When we are in the firing state, don't allow for a pickup to switch the weapon
	 */

	simulated function bool DenyClientWeaponSet()
	{
		return false;
	}

	simulated function BeginState( Name PreviousStateName )
	{
		Super.BeginState( PreviousStateName );
	}


}

/*********************************************************************************************
 * state WeaponEquipping
 * This state is entered when a weapon is becomeing active (ie: Being brought up)
 *********************************************************************************************/

simulated state WeaponEquipping
{
	/**
	 * We want to being this state by setting up the timing and then notifying the pawn
	 * that the weapon has changed.
	 */
	//ignores	BeginFire;

	simulated function BeginState(Name PreviousStateName)
	{
		//Log( "avaWeapon.Equip.BeginState" @Self );
		super.BeginState(PreviousStateName);

		// Notify the pawn that it's weapon has changed.

		if (Instigator.IsLocallyControlled() && avaPawn(Instigator)!=None)
		{
			avaPawn(Instigator).WeaponChanged(self);
			if ( GIMIndexWhenEquip >= 0 )
				avaPlayerController( Instigator.Controller ).ShowGIM( GIMIndexWhenEquip, 1 );
		}
	}

	//simulated function Tick( float DeltaTime )
	//{
	//	`log( SkeletalMeshComponent(Mesh).SpaceBases[0].XPlane.X @SkeletalMeshComponent(Mesh).SpaceBases[0].XPlane.Y @SkeletalMeshComponent(Mesh).SpaceBases[0].XPlane.Z );
	//	`log( SkeletalMeshComponent(Mesh).SpaceBases[0].YPlane.X @SkeletalMeshComponent(Mesh).SpaceBases[0].YPlane.Y @SkeletalMeshComponent(Mesh).SpaceBases[0].YPlane.Z );
	//	`log( SkeletalMeshComponent(Mesh).SpaceBases[0].ZPlane.X @SkeletalMeshComponent(Mesh).SpaceBases[0].ZPlane.Y @SkeletalMeshComponent(Mesh).SpaceBases[0].ZPlane.Z );
	//	`log( " " );

	//	//@SkeletalMeshComponent(Mesh).SpaceBases[0].YPlane @SkeletalMeshComponent(Mesh).SpaceBases[0].ZPlane @SkeletalMeshComponent(Mesh).SpaceBases[0].WPlane 
	//}

	simulated function bool TryPutDown()
	{
		PutDownWeapon();
		return TRUE;
	}
}

/**
 * When attempting to put the weapon down, look to see if our MinReloadPct has been met.  If so just put it down
 */
simulated function bool TryPutDown()
{
	//local float MinTimerTarget;
	//local float TimerCount;
	//bWeaponPutDown = true;
	//MinTimerTarget = GetTimerRate('RefireCheckTimer') * MinReloadPct;
	//TimerCount = GetTimerCount('RefireCheckTimer');
	//{
	//	PutDownWeapon();
	//	return true;
	//}
	//else
	//{
	//	// Shorten the wait time
	//	SetTimer(TimerCount + (MinTimerTarget - TimerCount), false, 'RefireCheckTimer');
	//	return true;
	//}
	PutDownWeapon();
	return false;
}

simulated function Vector GetPhysicalFireStartLoc( optional vector AimDir )
{
	local avaPlayerController	PC;
	local vector				Offset;

	if( Instigator != none )
	{
		PC = avaPlayerController(Instigator.Controller);

		if ( FireOffsetEx.length > CurrentFireMode )
			Offset = FireOffsetEx[CurrentFireMode];
		else
			Offset = FireOffset;

		if ( PC!=none && PC.bCenteredWeaponFire )
		{
			return Instigator.GetPawnViewLocation() + (vector(Instigator.GetViewRotation()) * Offset.X);
		}
		else
		{
			return Instigator.GetPawnViewLocation() + (Offset >> Instigator.GetViewRotation());
		}
	}

	return Location;
}

/**
 * Returns the location + offset from which to spawn projectiles/effects/etc.
 */
simulated function vector GetEffectLocation()
{
	local vector SocketLocation;
	local Rotator SocketRotation;

	if (SkeletalMeshComponent(Mesh)!=none && EffectSockets!='')
	{
		if (!SkeletalMeshComponent(Mesh).GetSocketWorldLocationAndrotation(EffectSockets, SocketLocation, SocketRotation))
			SocketLocation = Location;
	}
	else if (Mesh!=none)
		SocketLocation = Mesh.Bounds.Origin + (vect(45,0,0) >> Rotation);
	else
		SocketLocation = Location;

 	return SocketLocation;
}


simulated function RefireCheckTimer();
simulated function WeaponIsDown();

simulated state WeaponPuttingDown
{
	ignores OnAnimEnd;

	simulated function BeginState( Name PreviousStateName )
	{
		WeaponIsDown();
		bWeaponPutDown = false;
	}

	simulated function WeaponIsDown()
	{
		if( InvManager.CancelWeaponChange() )
		{
			return;
		}

		// This weapon is down, remove it from the mesh
		DetachWeapon();

		// switch to pending weapon
		InvManager.ChangedWeapon();

		// Put weapon to sleep
		//@warning: must be before ChangedWeapon() because that can reactivate this weapon in some cases
		GotoState('Inactive');
	}
}

simulated function PlayIdleAnim( optional AnimNodeSequence SeqNode = None )
{
	local int IdleIndex;
	if ( bWeaponPutDown != true )
	{
		if ( WorldInfo.NetMode != NM_DedicatedServer && WeaponIdleAnims.Length > 0 )
		{
			if ( WeaponIdleAnims.Length > 1 )
			{
				if ( SeqNode != None && SeqNode.AnimSeqName == WeaponIdleAnims[0] )	
					++IdleContinuousCnt;
				else												
					IdleContinuousCnt=0;

				if ( IdleContinuousCnt < MinIdleContinuousCnt )
				{
					PlayWeaponAnimation(WeaponIdleAnims[0], 0.0);
				}
				else
				{
					IdleIndex = Rand( WeaponIdleAnims.Length );
					PlayWeaponAnimation(WeaponIdleAnims[IdleIndex], 0.0);
				}
			}
			else
			{
				PlayWeaponAnimation(WeaponIdleAnims[0], 0.0);
			}
		}
	}
}

simulated event OnAnimEnd(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
	PlayIdleAnim( SeqNode );
}

simulated function bool ShouldRefire()
{
	if( !HasAmmo( CurrentFireMode ) && !bBeginFireWithoutAmmo  )
	{
		return false;
	}

	// refire if owner is still willing to fire
	return StillFiring( CurrentFireMode );
}

simulated state Active
{
	/**
	 * We override BeginFire() so that we can check for zooming
	 */
	/** Initialize the weapon as being active and ready to go. */
	simulated function BeginState( Name PreviousStateName )
	{
		local int i;
		//NetUpdateFrequency	= default.NetUpdateFrequency;
		//NetUpdateTime		= WorldInfo.TimeSeconds - 1.0f;
		Super.BeginState( PreviousStateName );
		if (InvManager != none && InvManager.LastAttemptedSwitchToWeapon != none)
		{
			InvManager.LastAttemptedSwitchToWeapon.ClientWeaponSet( true );
			InvManager.LastAttemptedSwitchToWeapon = none;
		}

 		// Check to see if we need to go down
   		if( bWeaponPutDown )
		{
			PutDownWeapon();
		}
		else if ( !HasAnyAmmo() )
		{
			WeaponEmpty();
		}
		else
		{
	        // if either of the fire modes are pending, perform them
			for( i=0; i<InvManager.PendingFire.Length; i++ )
			{
				if( PendingFire(i) )
				{
					BeginFire(i);
					break;
				}
			}
		}
	}

	simulated function BeginFire(byte FireModeNum)
	{
		if( !bDeleteMe && Instigator != None )
		{
			Global.BeginFire(FireModeNum);

			// Ammo 가 없어도 Fire 할 수 있다.
			if( PendingFire(FireModeNum) )
			{
				if ( bBeginFireWithoutAmmo || HasAmmo(FireModeNum) )
					SendToFiringState(FireModeNum);
			}
		}
	}
}

// Knife 에서도 쓰기 때문에 avaWeap_BaseGun 에 있는 Code 를 avaWeapon 으로 옮겼음 2006/3/15 by OZ
simulated function SetImpactLocation( vector pos, optional bool bBackFaceImpact, optional bool bBloodSpurt )
{		
	avaPawn(Instigator).SetImpactLocation( pos, bBackFaceImpact, bBloodSpurt );
}

//{{ Foreground FOV
//!{ 2006. 3. 6		허 창 민
`devexec function TestForegroundFOV( float FOV )
{
	WeaponFOV = FOV;
}

simulated function float AdjustForegroundFOVAngle( float FOV )
{
	return  WeaponFOV;
}
//!} 2006. 3. 6		허 창 민
//}}

simulated function Activate()
{
	if (!IsFiring())
	{
		//`log( "avaWeapon::Activate Goto WeaponEquipping" $self );
		GotoState('WeaponEquipping');
	}
}

// DropFrom Override 
// DroppedPickUpClass 을 이용하는게 아니라 PickUpClass 를 이용한다!
//	2006/03/16 by OZ
//
// DenyPickupQuery Function 도 Override 해야 한다!
simulated function bool SwitchSightMode( int requestMode, float transitionTime, optional bool bPlayAnim = true );

/// 쓰지 않고 죽은 경우 땅에 떨군다.
simulated function OwnerEvent( name eventName )
{
	if ( eventName == 'died' && bDropWhenDead == true )
	{
		ThrowWeapon(true);
	}
}

simulated function bool ThrowWeapon( optional bool bDoNotSwitch )
{
	// Pawn.Toss Weapon 과 Toss Weapon을 호출하는 곳에서 계산하는 것을 옮겨옴.
	return DropWeapon( Instigator.GetPawnViewLocation() + (ThrowOffset >> Instigator.GetViewRotation()), bDoNotSwitch );
}

function DropFrom(vector StartLocation, vector StartVelocity)	
{
	ThrowWeapon( true );
}

function vector GetThrowVel()
{
	local vector X, Y, Z;
	if ( VSize( Instigator.TearOffMomentum ) != 0 )	return Instigator.TearOffMomentum/2.0;
	GetAxes( Instigator.GetViewRotation(), X, Y, Z );
	return DropVelocity * X + Instigator.Velocity;
}

function bool CheckDuplicatePickUp()
{
	local avaPickUp	PickUp;
	ForEach DynamicActors(class'avaPickUp', PickUp)
	{
		if ( PickUp.Inventory == self )
			return true;
	}
	return false;
}

// Drop 에 실패 하더라도 Weapon 을 Destroy 하지는 않는다.
// Drop 의 실패 여부를 판단하기 위해서 inventory 의 DropFrom 함수 대신에 DropWeapon 함수를 쓴다.
function bool DropWeapon(vector StartLocation, optional bool bDoNotSwitch )
{
	local avaPickUp		P;

	if( !CanThrow() )				return false;
	if ( CheckDuplicatePickUp() )	return false;

	if ( Role != ROLE_Authority )	return false;

	if ( bDropIfHasAmmo == true && AmmoCount <= 0 )	return false;

	// if cannot spawn a pickup, then destroy and quit
	if( PickupClass == None )	return false;

	P = Spawn(PickUpClass,self,,StartLocation);
	if( P == None )
	{
		// 못던지더라도 Weapon 은 바꿔주도록 한다..
		// C4 때문에.. ㅡ.ㅡ;;
		if ( Role==ROLE_Authority )
			Instigator.InvManager.SwitchToBestWeapon( true );
		return false;
	}

	if ( DropOnlyOneAmmo )
		AmmoCount = 1;

	SwitchSightMode( 0, 0.0 );

	// Detach weapon components from instigator
	DetachWeapon();

	// Tell the client the weapon has been thrown
	ClientWeaponThrown();

	P.InventoryClass	= class;
	P.Instigator		= Instigator;
	P.bDynamicSpawned	= true;
	P.bJustAddAmmo		= PickUpAddAmmo;
	P.bDrawInRadar		= bDrawInRadar;
	P.LifeTime			= PickUpLifeTime;
	P.IndicatorType		= IndicatorType;
	P.IconCode			= RadarIconCode;
	P.PickupProvider	= PickupProvider;

	P.SetInventory( self );
	P.SetTeam( PickUpTeamNum );
	
	//if ( bSpecialInventory )
	//	P.SetTeam( Instigator.GetTeamNum() );

	P.Throw( GetThrowVel() );

	// Become inactive
	GotoState('Inactive');

	if( Instigator != None && Instigator.InvManager != None )
	{
		avaInventoryManager(Instigator.InvManager).RemoveFromInventoryEx(Self,!bDoNotSwitch);
	}

	Instigator = None;
	return true;
}

reliable client function ClientWeaponThrown()
{
	// Detach weapon components from instigator
	DetachWeapon();

	// Force it to turn off

	// Become inactive
	GotoState('Inactive');
}

function bool DenyPickupQuery(class<Inventory> ItemClass, Actor Pickup)
{
	return false;
}

function GivenTo(Pawn NewOwner, bool bDoNotActivate)
{
	if ( avaPawn(NewOwner) != None )
		avaPawn(NewOwner).AddWeapon( self );

	Super.GivenTo( NewOwner, bDoNotActivate);

	avaGame(WorldInfo.Game).PickupWeaponEvent(Self.Class,NewOwner.PlayerReplicationInfo, OwnerStatsID);
}

function ItemRemovedFromInvManager()
{
	if ( DestroyWeaponEmpty && !HasAnyAmmo() )
	{
		LifeSpan = 1.0f;
	}
	if ( avaPawn(owner) != None )
	{
		avaPawn(owner).RemoveWeapon( self );
	}
	super.ItemRemovedFromInvManager();
}

/************************************************************************************
	Dash 관련 Weapon Code
***********************************************************************************/
simulated function bool IsAvailableDash()
{
	return true;
}

reliable server function ServerDoDash( bool bDash )
{
	ClientDoDash( bDash );
	ForceDoDash( bDash );
}

reliable client simulated function ClientDoDash( bool bDash )
{
	ForceDoDash( bDash );
}

simulated function ForceDoDash( bool bDash )
{
	if ( bDash )	GotoState('WeaponDash');
	else			GotoState('Active');
}

simulated state WeaponDash
{
	simulated function BeginState(name PreviousStateName)
	{
		if ( WeaponDashStartAnims != '' )
			DashStartAnimNode = PlayWeaponAnimationEx( WeaponDashStartAnims );
	}

	simulated function EndState(Name NextStateName)
	{
		local float StartTime;
		if ( WeaponDashEndAnims != '' )
		{
			if ( DashStartAnimNode != None )
				StartTime = SkeletalMeshComponent(Mesh).GetAnimLength(WeaponDashEndAnims) - DashStartAnimNode.CurrentTime;

//			`log( "WeaponDash.EndState" @StartTime @DashStartAnimNode.CurrentTime );
			PlayWeaponAnimationEx( WeaponDashEndAnims, StartTime );
		}
	}

	simulated event OnAnimEnd(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
	{
		if ( WeaponDashIdleAnims != '' )
		{
			DashStartAnimNode = None;
			PlayWeaponAnimation( WeaponDashIdleAnims, 0.0 );
		}
		else global.OnAnimEnd( SeqNode, PlayedTime, ExcessTime );
	}

	simulated function AnimNodeSequence PlayWeaponAnimationEx(name AnimName, optional float StartTime )
	{
		local SkeletalMeshComponent SkelMesh;
		local AnimNodeSequence AnimNode;
		SkelMesh = SkeletalMeshComponent(Mesh);
		AnimNode = AnimNodeSequence(SkelMesh.Animations);
		if (AnimNode == None && SkelMesh.Animations.IsA('AnimTree'))
		{
			AnimNode = AnimNodeSequence(AnimTree(SkelMesh.Animations).Children[0].Anim);
		}
		AnimNode.SetAnim(AnimName);
		if (AnimNode.AnimSeq != None)
			AnimNode.PlayAnim(false,1.0,StartTime);
		return AnimNode;
	}

	simulated function bool DenyClientWeaponSet()
	{
		return true;
	}

	// Dash 중에는 무기를 버릴 수 없어야 한다.
	simulated function bool CanThrow()
	{
		return false;
	}

	simulated function bool TryPutDown()
	{
		return false;
	}
}

simulated function StartFire(byte FireModeNum)
{
	if ( PendingFire(FireModeNum) == true || PendingFire(1-FireModeNum) == true )	return;

	Super.StartFire( FireModeNum );
}

static function StaticPrecache( out array<Object> list )
{
}

// 'FiredWeapon' OwnerEvent 를 쓰는 놈이 없다. 
// 총을 쏜다고 Spawn Protection 이 풀리면 안될것 같다.
//simulated function FireAmmunition()
//{
//	Super.FireAmmunition();
//	if (avaPawn(Instigator) != None)
//		avaPawn(Instigator).DeactivateSpawnProtection();
//	InvManager.OwnerEvent('FiredWeapon');
//}

simulated function RaiseQuickVoice( int qv_index )
{
	if ( IsInState('Active') && WorldInfo.NetMode != NM_DedicatedServer && QuickVoiceAnim.Length > qv_index )
	{
		if ( SkeletalMeshComponent(Mesh).FindAnimSequence( QuickVoiceAnim[qv_index] ) != None )
			PlayWeaponAnimation(QuickVoiceAnim[qv_index], 0.0);
	}
}

simulated function float GetExposureCenterRegionScale()
{
	return 1.0;
}

// 가지고 있는 무기를 버린다....
function bool AbandonWeapon()
{
	if ( bAvailableAbandonWeapon )
	{
		return DropWeapon( Instigator.GetPawnViewLocation() + (ThrowOffset >> Instigator.GetViewRotation()), false );
	}
	return false;
}

simulated function bool CanEnterVehicle()
{
	return true;
}

defaultproperties
{
	MessageClass=class'avaPickupMessage'

	MaxAmmoCount=1

	FiringStatesArray(0)=WeaponFiring

	WeaponFireTypes(0)=EWFT_InstantHit
	WeaponFireTypes(1)=EWFT_InstantHit

	WeaponProjectiles(0)=none
	WeaponProjectiles(1)=none
	//WeaponKProjectiles(0)=none
	//WeaponKProjectiles(1)=none

	FireInterval(0)=+1.0
	FireInterval(1)=+1.0

	Spread(0)=0.0
	Spread(1)=0.0

	InstantHitDamage(0)=0.0
	InstantHitDamage(1)=0.0
	InstantHitMomentum(0)=0.0
	InstantHitMomentum(1)=0.0
	InstantHitDamageTypes(0)=class'DamageType'
	InstantHitDamageTypes(1)=class'DamageType'

    EffectSockets=MuzzleFlashSocket

	WeaponFireSnd=none

	MinReloadPct=0.5

	MuzzleFlashSocket=MuzzleFlashSocket


	ShouldFireOnRelease(0)=0
	ShouldFireOnRelease(1)=0

	WeaponColor=(R=255,G=255,B=255,A=255)
    BobDamping=0.85000
	BobDampingInDash=0.8500
	JumpDamping=1.0

 	WeaponFireAnim(0)=WeaponFire 	

 	WeaponPutDownAnim=None
	WeaponEquipAnim=None

	EquipTime=+0.45
	PutDownTime=+0.33

	MaxPitchLag	=	0
	MaxYawLag	=	5000
	RotLagSpeed	=	0.8

	Begin Object class=AnimNodeSequence Name=MeshSequenceA
		bCauseActorAnimEnd=True
	End Object

	// Weapon SkeletalMesh
	Begin Object Class=avaSkeletalMeshComponent Name=MeshComponent0
		Animations=MeshSequenceA
		bOnlyOwnerSee=true
		CastShadow=true
		DepthPriorityGroup=SDPG_Foreground
		CollideActors=false
		bUpdateSkelWhenNotRendered=false		
		bCastDynamicShadow=false
		Rotation=(Yaw=-16384)
		bUseAsOccluder=false
	End Object
	Mesh=MeshComponent0
	Components.Add(MeshComponent0)

	// Art 팀과 무기별로 상관없이 똑같이 적용하기로 하였기 때문에
	// 특별한 얘기가 없는한 하위Class 에서 이 값을 Setting 하지 말것
	PlayerViewOffset=(X=3,Y=-3.35,Z=1.0)
	//PlayerViewOffset=(X=0,Y=0,Z=0)
	WeaponFOV=50.0

	// true 이면 Ammo 가 없더라도 BeginFire 는 성공한다. 탄창이 비어있는 소리를 내기 위해서이다.
	bBeginFireWithoutAmmo = true

	// Weapon 을 떨어뜨릴지에 대한 Flag
	bCanThrow		=	false
	ThrowOffset		=	(X=30,Y=0,Z=1)
	
	bOnlyRelevantToOwner=	FALSE
	bAlwaysRelevant		=	true

	WeaponDashStartAnims	=	Dash_Start
	WeaponDashEndAnims		=	Dash_End
	WeaponDashIdleAnims		=	Dash_Idle

	WeaponType			=	WEAPON_ETC	
	
	WeaponPutDownSnd		=	none

	QuickVoiceAnim(0)	=	Sig_GoGo
	QuickVoiceAnim(1)	=	Sig_ISee
	QuickVoiceAnim(2)	=	Sig_Followme
	QuickVoiceAnim(3)	=	Sig_AllClear
	QuickVoiceAnim(4)	=	Sig_Enemy
	QuickVoiceAnim(5)	=	Sig_CoverMe
	QuickVoiceAnim(6)	=	Sig_Move
	QuickVoiceAnim(7)	=	Sig_No
	QuickVoiceAnim(8)	=	Sig_Nice

	CrossHairSizeX		=	64
	CrossHairSizeY		=	64
	DropVelocity		=	200

	EUArmMeshName = "CH_EU_Arms_1p.EU_Arms_01.MS_EU_Arms01_1p"
	NRFArmMeshName = "CH_NRF_Arms_1p.NRF_Arms_01.MS_NRF_Arms01_1p"
	DefaultHandMeshName = "CH_Glove_common.Glove_001.MS_hand01"

	PickUpLifeTime		=	16.0
	PickUpTeamNum		=	255
	MaintenanceRate		=	-1.0
	bDisplayAmmoCount	=	true
	bReceiveOwnerEvents	=	true

	GIMIndexWhenPickUp	=	-1
	GIMIndexWhenEquip	=	-1
}
