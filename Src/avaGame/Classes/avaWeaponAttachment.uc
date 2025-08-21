/*=============================================================================
	avaWeaponAttachment
 
	Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2006/07/04 by OZ
	
		등에 붙이기 위하여 다시 만듬...
		Customize 를 고려해서 Design 해야함...

=============================================================================*/
class avaWeaponAttachment extends Actor
	abstract
	dependson(avaPhysicalMaterialProperty)
	native;


var array<MaterialInstance> BloodSpurtDecalBig, BloodSpurtDecalMid, BloodSpurtDecalSmall;

var()	MeshComponent			Mesh;					// 기본 Position 을 잡기 위한 SkeletalMeshComponent...
var()	SkeletalMeshComponent	SocMesh;				// Socket 위치를 잡기 위한 SkeletalMeshComponent, 필요 없을 수도 있다....
var()	StaticMeshComponent		BasicMesh;				// Rendering 을 위한 Basic StaticMeshComponent
/* Bullet trail */
var()	avaBulletTrailComponent	BulletTrailComponent;
var()	int						TrailInterval;

var		array< StaticMeshComponent >	Items;			// 부가적으로 붙는 Item 들...

var		bool					bMeshIsSkeletal;		// 기본 Mesh 가 Skeletal 인지 Static 인지 구분하기 위하여
var		string					MeshName;
var		string					SocMeshName;
var		string					BasicMeshName;

var		Name					PosRootBoneName;
var		Name					SocRootBonename;

var		Name					AttachmentBoneName;		// Character 의 손에 해당하는 Bone Name
var		Name					CarriedSocketName;		// 등뒤에 붙일 Socket 위치 Name

// (2007/01/26 고광록)
var		Name					UIAttachmentBoneName;	//!< UI Character 의 손에 해당하는 Bone Name
var		OcclusionGroupComponent	HitEffectOcclusionGroup;


//var MeshComponent	CarriedMesh;

/** 
	3인칭에서 표현될 Animation 의 Weapon Type 이다. 
	3인칭 시점에서 Animation 이 제대로 나오지 않는다면 하위 Attachment Class 에서 
	WeaponType 이 제대로 Setting 이 되었는지를 확인한다.

	EBlendWeaponType 은 avaAnimBlendByWeaponType 에 정의되어 있음
**/

var	EBlendWeaponType		AttachmentWeaponType;
var Name					AnimPrefix;
var int						TrailCount;

/*********************************************************************************************
 Muzzle Flash
********************************************************************************************* */

/** Holds the name of the socket to attach a muzzle flash too */
var name					MuzzleFlashSocket;

/** Muzzle flash staticmesh, attached to weapon mesh */
var	StaticMeshComponent		MuzzleFlashMesh;

/** Muzzle flash PSC and Templates*/
var ParticleSystemComponent	BulletPSC;
var bool					TryAttachBulletPSC;

var ParticleSystemComponent	MuzzleFlashPSC;
var ParticleSystemComponent AbsMuzzleFlashPSC;
var ParticleSystem			MuzzleFlashPSCTemplate, MuzzleFlashAltPSCTemplate;
var ParticleSystem			AbsMuzzleFlashPSCTemplate, AbsMuzzleFlashAltPSCTemplate;

var color					MuzzleFlashColor;
var bool					bMuzzleFlashPSCLoops;

/** dynamic light */
var	class<avaGunMuzzleFlashLight> MuzzleFlashLightClass;
var avaGunMuzzleFlashLight	MuzzleFlashLight;

/** How long the Muzzle Flash should be there */
var float					MuzzleFlashDuration;

/** TEMP for guns with no muzzleflash socket */
var SkeletalMeshComponent OwnerMesh;


/*********************************************************************************************
Effects
********************************************************************************************* */

/** impact effects by material type */
var name DamageCode;
/** default impact effect to use if a material specific one isn't found */
var PhysicalMaterial DefaultPhysicalMaterial;
var MaterialImpactEffect DefaultImpactEffect;
/** sound that is played when the bullets go whizzing past your head */
var SoundCue BulletWhip;

var float MaxImpactEffectDistance;
var float MaxFireEffectDistance;

var class<avaWeapon> WeaponClass;

var avaWeapon	LinkedWeapon;		// 이 Attachment 에 연결되어 있는 실제 Weapon 객체....

// Death Icons
var string		DeathIconStr;
var bool		bRightHandedWeapon;
var	int			WeaponState;

var bool		bAttachmentState;

function SkeletalMeshComponent CreateSkelMesh( string ThisMeshName )
{
	local SkeletalMeshComponent NewMesh;
	local SkeletalMesh tmpSkelMesh;	

	NewMesh = new(self) class'SkeletalMeshComponent';
	NewMesh.bUseAsOccluder = FALSE;
	NewMesh.SetBlockRigidBody(False);
	NewMesh.SetActorCollision( FALSE, FALSE );
	if ( ThisMeshName != "" )	
	{
		tmpSkelMesh = SkeletalMesh( DynamicLoadObject( ThisMeshName, class'SkeletalMesh' ) );
		if ( tmpSkelMesh != None )	NewMesh.SetSkeletalMesh( tmpSkelMesh );
		else						`warn( "avaWeaponAttachment.CreateBasicComponent Cannot Load SkeletalMesh" @ThisMeshName );
	}
	else							`warn( "avaWeaponAttachment.CreateBasicComponent Not Initialize Mesh" );
	NewMesh.bUpdateSkelWhenNotRendered=false;
	NewMesh.bCastDynamicShadow=false;

	return NewMesh;
}

function StaticMeshComponent CreateStaticMesh( string ThisMeshName )
{
	local StaticMeshComponent NewMesh;
	local StaticMesh tmpStaticMesh;

	NewMesh = new(self) class'StaticMeshComponent';
	NewMesh.bUseAsOccluder = FALSE;
	NewMesh.SetBlockRigidBody(False);
	NewMesh.SetActorCollision( FALSE, FALSE );
	if ( ThisMeshName != "" )	
	{
		tmpStaticMesh = StaticMesh( DynamicLoadObject( ThisMeshName, class'StaticMesh' ) );
		if ( tmpStaticMesh != None )	NewMesh.SetStaticMesh( tmpStaticMesh );
		else							`warn( "avaWeaponAttachment.CreateBasicComponent Cannot Load StaticMesh" @ThisMeshName );
	}
	else								`warn( "avaWeaponAttachment.CreateBasicComponent Not Initialize Mesh" );

	return NewMesh;
}

// 기본 Component 를 만든다...
function CreateBasicComponent()
{	
	local rotator		newRot;

	if ( bMeshIsSkeletal )
	{
		// Mesh 가 Skeletal 인 경우는 Carried 를 같이 붙이기 위해서 이다. 즉, SocMesh 와 BasicMesh 는 반드시 필요하다...
		Mesh = CreateSkelMesh( MeshName );		

		SkeletalMeshComponent( Mesh ).bPauseAnims = TRUE;
		SkeletalMeshComponent( Mesh ).bNoSkeletonUpdate = 1;

		SocMesh = CreateSkelMesh( SocMeshName );		
		SocMesh.bPauseAnims = TRUE;
		SocMesh.bNoSkeletonUpdate = 1;
		SocMesh.SetHidden(true);
		SocMesh.bForceNotToDraw = true;
		SocMesh.SetActorCollision(false,false);				

		BasicMesh = CreateStaticMesh( BasicMeshName );
		BasicMesh.SetActorCollision( false, false );
	}
	else
	{
		Mesh = CreateStaticMesh( MeshName );		
	}

	newRot.Roll = -16384;
	Mesh.SetHidden(true);
	Mesh.SetActorCollision(false,false);
	Mesh.SetRotation( newRot );
}

// component 를 만든다
function AttachTo( avaPawn p, avaWeapon w )
{
	Instigator		= p;
	LinkedWeapon	= w;

	CreateBasicComponent();

	// Skin 을 바꾸거나 Item 등을 Attach 해준다!!!
	p.Mesh.AttachComponent( Mesh, AttachmentBoneName );
	if ( SocMesh != None )
		SocMesh.AttachComponent( BasicMesh, SocRootBoneName );
	
	AttachComponent( BulletTrailComponent );
	AttachComponent( HitEffectOcclusionGroup );	

	//if ( !p.bNoDetailMeshes )
	AttachItems();

	SetSkins();

	ApplyShadowParent( p.Mesh );

	ChangeAttachmentState( false );

	AttachMuzzleFlash();

	//AttachBulletEjector();

}

simulated function AttachItems()
{
	local int			i;
	local int			id;
	local StaticMesh	tmpStaticMesh;
	local string		StaticMeshName;

	if ( SocMesh == None )	return;	// Socket Mesh 가 없다면 어떻게 붙일것인가...??
	for ( i = 0 ; i < LinkedWeapon.ItemParts.Length ; ++ i )
	{
		if ( LinkedWeapon.ItemParts[i].MeshName != "" )
		{
			id				= items.length;
			Items.Length	= id + 1;
			Items[id]		= new(self) class'StaticMeshComponent';
			Items[id].bUseAsOccluder = FALSE;
			Items[id].SetActorCollision( FALSE, FALSE );
			StaticMeshName	= LinkedWeapon.ItemParts[i].MeshName$"_3p";	// 3인칭용은 1인칭용 뒤에 _3p 를 붙여서 쓴다...
			tmpStaticMesh	= StaticMesh( DynamicLoadObject( StaticMeshName, class'StaticMesh' ) );
			if ( LinkedWeapon.ItemParts[i].MaxVisibleDistance > 0 )
				SetFadeOut( Items[id], LinkedWeapon.ItemParts[i].MaxVisibleDistance );
			if ( tmpStaticMesh != None )	Items[id].SetStaticMesh( tmpStaticMesh );
			else							`warn( "avaWeaponAttachment.AttachItems Cannot Load StaticMesh" @StaticMeshName );
			Items[id].SetActorCollision(false,false);
			Items[id].bCastDynamicShadow=false;
			SocMesh.AttachComponentToSocket( Items[id], LinkedWeapon.ItemParts[i].SocketName );
		}
	}
}

simulated function SetSkins()
{
	local int i;
	local Material		tmpMtrl;
	for ( i = 0 ; i < 3 ; ++ i )
	{
		if ( LinkedWeapon.WeaponSkin[i] == "" )	continue;	// 적용할 Material 이 없음
		tmpMtrl = Material( DynamicLoadObject( LinkedWeapon.WeaponSkin[i], class'Material' ) );
		if ( tmpMtrl != None )
		{
			if ( BasicMesh != None )	BasicMesh.SetMaterial( i, tmpMtrl );
			else						Mesh.SetMaterial( i, tmpMtrl );
		}
		else					`warn( "avaWeaponAttachment.SetSkins Cannot Load Material" @LinkedWeapon.WeaponSkin[i] );
	}
}

function DetachFrom( avaPawn p )
{
	// Weapon Mesh Shadow
	if ( Mesh != None )
	{		
		ApplyShadowParent( None );
		if ( SocMesh != None )
		{
			if ( p.Mesh.IsComponentAttached( SocMesh ) )
			{
				SkeletalMeshComponent( Mesh ).AttachComponent( SocMesh, PosRootBoneName );
			}
		}
		DetachComponent( BulletTrailComponent );		
		DetachComponent( HitEffectOcclusionGroup );		

		if ( p.Mesh != None )
		{
			p.Mesh.DetachComponent( mesh );
		}

		SocMesh = None;
		Mesh	= None;
	}
	DetachMuzzleFlash();
	DetachBulletEjector();
}

function ChangeAttachmentState( bool bAttachment )
{
	bAttachmentState = bAttachment;
	if ( bAttachmentState == true )
	{
		if ( SocMesh != None )	SkeletalMeshComponent( Mesh ).AttachComponent( SocMesh, PosRootBoneName );
	}
	else
	{
		if ( CarriedSocketName != '' && SocMesh != None )	avaPawn(Instigator).Mesh.AttachComponentToSocket( SocMesh, CarriedSocketName );
	}
	ChangeVisibility( !avaPawn( Instigator ).IsFirstPerson() );
}

simulated function MarkSeeThroughGroupIndex( int Index )
{
	local int i;
	
	Mesh.SetSeeThroughGroupIndex( Index );
	
	if ( SocMesh != None )		
	{
		SocMesh.SetSeeThroughGroupIndex( Index );
	}
	if ( BasicMesh != None )
	{
		BasicMesh.SetSeeThroughGroupIndex( Index );
	}

	for ( i = 0 ; i < Items.length ; ++i )
	{
		Items[i].SetSeeThroughGroupIndex( Index );
	}
}

simulated function ApplyShadowParent( SkeletalMeshComponent MeshCpnt )
{
	local int i;
	Mesh.SetShadowParent( MeshCpnt );
	Mesh.SetOcclusionGroup( MeshCpnt );	
	if ( SocMesh != None )		
	{
		SocMesh.SetShadowParent( MeshCpnt );		
		SocMesh.SetOcclusionGroup( MeshCpnt );
	}
	if ( BasicMesh != None )
	{
		BasicMesh.SetShadowParent( MeshCpnt );
		BasicMesh.SetOcclusionGroup( MeshCpnt );		
	
		if ( MeshCpnt != None )	BasicMesh.SetLightEnvironment( MeshCpnt.LightEnvironment );
	}
	else if ( MeshCpnt != None )
	{
		Mesh.SetLightEnvironment( MeshCpnt.LightEnvironment );
	}

	for ( i = 0 ; i < Items.length ; ++i )
	{
		Items[i].SetShadowParent( MeshCpnt );
		Items[i].SetOcclusionGroup( MeshCpnt );
		if ( MeshCpnt != None )	Items[i].SetLightEnvironment( MeshCpnt.LightEnvironment );
	}
}

// MuzzleFlash 의 위치는 SocMesh 에서 가지고 와야 한다....
simulated function AttachMuzzleFlash()
{
	local SkeletalMeshComponent SKMesh;
	if ( SocMesh == None )	return;
	SKMesh = SocMesh;
	if (SKMesh != none && MuzzleFlashSocket != '')
	{
		// Muzzle Flash mesh
		if ( MuzzleFlashMesh != None )
			SKMesh.AttachComponentToSocket(MuzzleFlashMesh, MuzzleFlashSocket);

		if (MuzzleFlashPSCTemplate != None || MuzzleFlashAltPSCTemplate != None)
		{
			MuzzleFlashPSC = new(self) class'avaParticleSystemComponent';
			SKMesh.AttachComponentToSocket(MuzzleFlashPSC, MuzzleFlashSocket);
			MuzzleFlashPSC.DeactivateSystem();
			MuzzleFlashPSC.SetColorParameter('MuzzleFlashColor', MuzzleFlashColor);
			
			MuzzleFlashPSC.bCastSunShadow = false;	// 2007. 1 .7 changmin ; add cascaded shadow
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

simulated function AttachBulletEjector()
{
	TryAttachBulletPSC = true;
	if ( SocMesh == None )	return;
	if ( WeaponClass == None ) return;
	if ( class<avaWeap_BaseGun>(WeaponClass).default.BulletTemplete == None )	return;
	if ( SocMesh.GetSocketByName( 'ejector' ) == None )						return;

	BulletPSC = new(self) class'avaParticleSystemComponent';
	SocMesh.AttachComponentToSocket( BulletPSC, 'ejector' );
	BulletPSC.DeactivateSystem();

}

simulated function DetachMuzzleFlash()
{
	local SkeletalMeshComponent SKMesh;
	if ( SocMesh == None )	return;
	SKMesh = SocMesh;
	if (SKMesh != none)
	{
		// Muzzle Flash Mesh
		if ( MuzzleFlashMesh != None )
			SKMesh.DetachComponent( MuzzleFlashMesh );

		if (MuzzleFlashPSC != none)
			SKMesh.DetachComponent( MuzzleFlashPSC );

		if (AbsMuzzleFlashPSC != none)
			SKMesh.DetachComponent( AbsMuzzleFlashPSC );

		// Muzzle Flash dynamic light
		if ( MuzzleFlashLight != None )
			SKMesh.DetachComponent( MuzzleFlashLight );
	}
}

simulated function DetachBulletEjector()
{
	TryAttachBulletPSC = false;
	if ( BulletPSC != None )
	{
		SocMesh.DetachComponent( BulletPSC );
	}
}

simulated function ChangeVisibility( bool bVisible )
{
	local bool bWeaponVisible;
	if ( Mesh == None )	return;
	bWeaponVisible = true;
	if ( bVisible == false )	bWeaponVisible = false;
	else
	{
		if ( bAttachmentState == true || CarriedSocketName != '' )
			bWeaponVisible = true;
		else
			bWeaponVisible = false;
	}

	ChangeMeshVisibility( bWeaponVisible, SocMesh != None ? SocMesh : Mesh );
}		

simulated function ChangeMeshVisibility(bool bIsVisible, MeshComponent C)
{
	local int i;
	local SkeletalMeshComponent AttachmentMesh;
	local PrimitiveComponent Primitive;

	//<@ add cascaded shadow ; 2008. 1. 7 changmin
	C.bCastHiddenShadow = bAttachmentState;
	//>@ changmin
	 
	C.SetHidden( !bIsVisible );
	AttachmentMesh = SkeletalMeshComponent(C);
	if (AttachmentMesh != None)
	{
		// set bOwnerNoSee on anything attached to weapon attachment as well
		// FIXME: shouldn't this be automatic?
		for (i = 0; i < AttachmentMesh.Attachments.Length; i++)
		{
			Primitive = PrimitiveComponent(AttachmentMesh.Attachments[i].Component);
			if (Primitive != None)
			{
				if (SkeletalMeshComponent(Primitive) == none)
				{
					//<@ add cascaded shadow ; 2008. 1. 7 changmin
					Primitive.bCastHiddenShadow = bAttachmentState;
					//>@ changmin
					Primitive.SetHidden( !bIsVisible );
				}
				else
				{
//					`log( "avaWeaponAttachment.ChangeMeshVisibility Recursive Call" );
					ChangeMeshVisibility( bIsVisible, SkeletalMeshComponent(Primitive) );				
				}
			}
		}
	}
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

/**
 * Causes the muzzle flashlight to turn on and setup a time to
 * turn it back off again.
 */

simulated event CauseMuzzleFlash( vector HitLocation )
{
	local vector	l;
	local rotator	r;
	// only enable muzzleflash light if performance is high enough
	//if ( !WorldInfo.bDropDetail )
	//{
		if ( MuzzleFlashLight == None )
		{
			if ( MuzzleFlashLightClass != None )
				CauseMuzzleFlashLight();
		}
		else
		{
			MuzzleFlashLight.ResetLight();
		}
	//}

	if (MuzzleFlashPSC != none)
	{
		if ( !bMuzzleFlashPSCLoops || MuzzleFlashPSC.bWasDeactivated )
		{
			if (Instigator != None && Instigator.FiringMode == 1 && MuzzleFlashAltPSCTemplate != None)
			{
				MuzzleFlashPSC.SetTemplate(MuzzleFlashAltPSCTemplate);
			}
			else
			{
				MuzzleFlashPSC.SetTemplate(MuzzleFlashPSCTemplate);
			}
			MuzzleFlashPSC.ActivateSystem();
			MuzzleFlashPSC.SetOcclusionGroup(Mesh.OcclusionGroup);
		}
	}

	if (AbsMuzzleFlashPSC != none)
	{
		if ( !bMuzzleFlashPSCLoops || AbsMuzzleFlashPSC.bWasDeactivated )
		{

			if ( SocMesh != None && SocMesh.GetSocketWorldLocationAndRotation( MuzzleFlashSocket, l, r ) )
			{
				if (Instigator != None && Instigator.FiringMode == 1 && AbsMuzzleFlashAltPSCTemplate != None)
				{
					AbsMuzzleFlashPSC.SetTemplate(AbsMuzzleFlashAltPSCTemplate);
				}
				else
				{
					AbsMuzzleFlashPSC.SetTemplate(AbsMuzzleFlashPSCTemplate);
				}
				AbsMuzzleFlashPSC.SetTranslation( l );
				AbsMuzzleFlashPSC.SetRotation( r );
				AbsMuzzleFlashPSC.ActivateSystem();
				AbsMuzzleFlashPSC.SetOcclusionGroup(Mesh.OcclusionGroup);
			}
		}
	}

	if (MuzzleFlashMesh != none)
	{
		MuzzleFlashMesh.SetHidden(false);
	}	

	// Hit 중 RagDoll Motion 이 진행중에는 Trail 을 찍지 않도록....
	if (TrailCount % TrailInterval == 0 && avaPawn(Instigator).bBlendOutTakeHitPhysics == false )
	{
		PlayTrailEffect( HitLocation );
	}

	TrailCount++;

	// Set when to turn it off.
	if ( MuzzleFlashDuration != 0.0 )
		SetTimer(MuzzleFlashDuration,false,'MuzzleFlashTimer');
}

simulated function CauseMuzzleFlashLight()
{
	local vector X,Y,Z;	
	MuzzleFlashLight = new(self) MuzzleFlashLightClass;
	if ( SocMesh != None && SocMesh.GetSocketByName(MuzzleFlashSocket) != None )
	{
		SocMesh.AttachComponentToSocket(MuzzleFlashLight, MuzzleFlashSocket);
	}
	else if (OwnerMesh != none)
	{
		GetAxes(Rotation,X,Y,Z);
		OwnerMesh.AttachComponent(MuzzleFlashLight, AttachmentBoneName);
	}
}

simulated function PlayTrailEffect( vector HitLocation )
{
	local vector	l;
	local rotator	r;
	local float		d;

	if ( SocMesh == None )	return;
	if ( SocMesh.GetSocketWorldLocationAndRotation( MuzzleFlashSocket, l, r ) )
	{
		d = VSize(HitLocation-l);
		if (d > 100)
		{
			BulletTrailComponent.Fire( l + (HitLocation-l) * 100 / d, HitLocation, Vect(0,0,0) );
		}			
	}
}

/**
 * Stops the muzzle flash
 */

simulated event StopMuzzleFlash()
{
	ClearTimer('MuzzleFlashTimer');
	MuzzleFlashTimer();
	if ( MuzzleFlashPSC != none )
		MuzzleFlashPSC.DeactivateSystem();

	if ( AbsMuzzleFlashPSC != none )
		AbsMuzzleFlashPSC.DeactivateSystem();
}

//simulated event StopBulletEjector()
//{
//	if ( BulletPSC != None )
//	{
//		//BulletPSC.DeactivateSystem();
//		ClearTimer( 'StopBulletEjector' );
//	}
//}

/**
 * The Weapon attachment, though hidden, is also responsible for controlling
 * the first person effects for a weapon.
 */

simulated function FirstPersonFireEffects(Weapon PawnWeapon, vector HitLocation)	// Should be subclassed
{
	//if (PawnWeapon!=None)
	//	PawnWeapon.PlayFireEffects( Pawn(Owner).FiringMode, HitLocation );
}

simulated function StopFirstPersonFireEffects(Weapon PawnWeapon)	// Should be subclassed
{
	if (PawnWeapon!=None)
		PawnWeapon.StopFireEffects( Pawn(Owner).FiringMode );
}

simulated function bool EffectIsRelevant(vector SpawnLocation, bool bForceDedicated, optional float CullDistance )
{
	if ( Instigator != None )
	{
		if ( SpawnLocation == Location )
			SpawnLocation = Instigator.Location;
		return Instigator.EffectIsRelevant(SpawnLocation, bForceDedicated, CullDistance);
	}
	return Super.EffectIsRelevant(SpawnLocation, bForceDedicated, CullDistance);
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
 * Spawn all of the effects that will be seen in behindview/remote clients.  This
 * function is called from the pawn, and should only be called when on a remote client or
 * if the local client is in a 3rd person mode.
*/
simulated event ThirdPersonFireEffects(vector HitLocation)
{
	if (IsMuzzleFlashRelevant() )
	{
		CauseMuzzleFlash(HitLocation);
		if ( class<avaWeap_BaseGun>(WeaponClass).default.bEjectBulletWhenFire == true )
			EjectBullet();
	}

	// Have pawn play firing anim
	if ( Instigator != none && avaPawn(Instigator)!=None )
		Instigator.PlayFiring(1.0,avaPawn(Instigator).FiringMode == 1 ? '1' : '0' );
}

simulated event StopThirdPersonFireEffects()
{
	StopMuzzleFlash();
	//StopBulletEjector();
	// Have pawn play firing anim
	if ( Instigator != none && avaPawn(Instigator)!=None )
		avaPawn(Instigator).StopPlayFiring();
}

/** returns the impact sound that should be used for hits on the given physical material */
simulated function MaterialImpactEffect GetImpactEffect(PhysicalMaterial HitMaterial)
{
	local int i;
	local avaPhysicalMaterialProperty PhysicalProperty;
	local ImpactDecalData IDD;

	if (HitMaterial == None)	HitMaterial = DefaultPhysicalMaterial;
	PhysicalProperty = avaPhysicalMaterialProperty(HitMaterial.GetPhysicalMaterialProperty(class'avaPhysicalMaterialProperty'));
	if (PhysicalProperty != None)
	{
		i = PhysicalProperty.ImpactEffects.Find('DamageCode', DamageCode);
		if (i != -1)	return PhysicalProperty.ImpactEffects[i];

		if (IsA( 'avaAttachment_BaseGun' ))
		{
			i = PhysicalProperty.ImpactEffects.Find('DamageCode', 'Gun');
			if (i != -1)	return PhysicalProperty.ImpactEffects[i];
		}
	}	
	`log( "Error invalid damage code"@DamageCode );
	if (DefaultImpactEffect.ImpactDecals.Length == 0)
	{		
		DefaultImpactEffect.ImpactDecals[0] = IDD;
		DefaultImpactEffect.ImpactDecals[0].DecalMaterial = DefaultImpactEffect.DecalMaterial;
		DefaultImpactEffect.ImpactDecals[0].DecalWidth = DefaultImpactEffect.DecalWidth;
		DefaultImpactEffect.ImpactDecals[0].DecalHeight = DefaultImpactEffect.DecalHeight;		
	}
	return DefaultImpactEffect;	
}

simulated function bool AllowImpactEffects(Actor HitActor, vector HitLocation, vector HitNormal)
{
	return true;
}

simulated function PlayImpactEffects(Weapon PawnWeapon,vector HitLocation)
{
	PlayImpactEffects2(PawnWeapon,HitLocation,true,false);
}

// Hit 된 위치, Normalize 된 Hit Direcction
simulated function TraceHitInfo PlayImpactFX(vector HitLocation,vector HitDir,bool bBloodSpurt,out Actor HittedActor)
{
	local vector					HitNormal,NewHitLoc;
	local TraceHitInfo				HitInfo;
	local Actor						HitActor;
	local MaterialImpactEffect		ImpactEffect;
	local avaDecalLifetimeDataRound LifetimeData;
	local Rotator					r,BloodRotator;
	local float						Distance;
	local MaterialInstance			Blood;
	local int						BloodSize;
	local int						DecalIndex;
	local Emitter					AnEmitter;
		 
	HitNormal = HitDir;
	HitActor = Trace(NewHitLoc, HitNormal, (HitLocation - (HitNormal * 32)), HitLocation + (HitNormal * 32), true,, HitInfo, TRACEFLAG_Bullet);
	HittedActor = HitActor;

	if ( HitActor == None || avaPawn(HitActor) != None )	return HitInfo;
	
	if ( !EffectIsRelevant(HitLocation, false, MaxImpactEffectDistance) || 
		 !AllowImpactEffects(HitActor, HitLocation, HitNormal) )
		return HitInfo;

	//if ( HitInfo.PhysMaterial == None )	return HitInfo;

	ImpactEffect = GetImpactEffect(HitInfo.PhysMaterial);	

	r = rotator( -HitNormal );
	if (ImpactEffect.decalAlignType == DECAL_ALIGN_VIEW )
	{
		if ( ( HitNormal cross vector( avaPawn(Instigator).Rotation ) ).z == 0.0 )
			r.Roll = avaPawn(Instigator).GetViewRotation().Yaw;
	}
	else if ( ImpactEffect.decalAlignType == DECAL_ALIGN_RANDOM )	
		r.Roll = FRand() * 65536.0f;

	if ( bBloodSpurt && class'avaNetHandler'.static.GetAvaNetHandler().IsPlayerAdult() )
	{
		// 같은 편이라면 핏자국을 찍지 않는다... 나중에 만약 Friendly Fire 가 켜진다면 핏자국도 찍어야 한다...
		//if ( !Pawn(HitActor).IsSameTeam( Instigator ) )
		//{
		Distance = VSize(avaPawn(Instigator).Location - HitLocation) / 16 * 0.3;

		if (Distance > 10)
		{
			bBloodSpurt = false;
		}
		else
		{
			LifetimeData		= new(HitActor.Outer) class'avaDecalLifetimeDataRound';
			LifetimeData.Round	= avaGameReplicationInfo(WorldInfo.GRI).CurrentRound;
			BloodRotator		= r;
			BloodRotator.Roll	= FRand() * 65536.0f;

			if (Distance > 5)
			{
				Blood	  = BloodSpurtDecalSmall[rand(3)];
				BloodSize = 60;
			}
			else if (Distance > 2)
			{
				Blood	  = BloodSpurtDecalMid[rand(3)];
				BloodSize = 80;
			}
			else
			{
				Blood	  = BloodSpurtDecalBig[rand(3)];
				BloodSize = 105;
			}

			HitActor.AddDecal( Blood, HitLocation, BloodRotator,0, //rotator(-HitNormal),
							BloodSize, BloodSize, 10.0, false, HitInfo.HitComponent, LifetimeData, false, false, HitInfo.BoneName, HitInfo.Item, HitInfo.LevelIndex );
		}
		//}
	}
	else
	{
		if (ImpactEffect.ImpactDecals.Length > 0 )
		{
			if (class'avaOptionSettings'.Default.DecalDetail == 0)
				DecalIndex = 0;
			else
				DecalIndex = Rand( ImpactEffect.ImpactDecals.Length );

			LifetimeData = new(HitActor.Outer) class'avaDecalLifetimeDataRoundAnother';
			LifetimeData.Round = avaGameReplicationInfo(WorldInfo.GRI).CurrentRound;
			HitActor.AddDecal( ImpactEffect.ImpactDecals[DecalIndex].DecalMaterial, HitLocation, r,0, //rotator(-HitNormal),
								ImpactEffect.ImpactDecals[DecalIndex].DecalWidth, ImpactEffect.ImpactDecals[DecalIndex].DecalHeight, 10.0, false, 
								HitInfo.HitComponent, LifetimeData, false, false, '', HitInfo.Item, HitInfo.LevelIndex );
		}

		if (ImpactEffect.ParticleTemplate != None && !(class'avaOptionSettings'.Default.bDisableImpactParticle) )
		{
			AnEmitter = avaGameReplicationInfo(WorldInfo.GRI).ObjectPool.GetImpactEmitter(ImpactEffect.ParticleTemplate, HitLocation, rotator(HitNormal));			
			AnEmitter.ParticleSystemComponent.ActivateSystem();
			AnEmitter.ParticleSystemComponent.SetOcclusionGroup(HitEffectOcclusionGroup);
			AnEmitter.SetTimer( 3, FALSE, 'HideSelf' );
		}
	}

	return HitInfo;
}

simulated function PlayImpactEffects2(Weapon PawnWeapon,vector HitLocation,bool bBulletWhipSound,bool bBackFaceImpact,optional bool bBloodSpurt,optional int eighthrandom )
{
	local vector				HitNormal, FireDir;
	local TraceHitInfo			HitInfo;
	local MaterialImpactEffect	ImpactEffect;	
	local Actor					HitActor;

	if (avaPawn(Owner) != None)
	{
		HitNormal = Normal(Owner.Location - HitLocation);

		HitInfo = PlayImpactFX( HitLocation, 
								bBackFaceImpact ? -HitNormal : HitNormal,
								bBloodSpurt, HitActor );

		ImpactEffect = GetImpactEffect(HitInfo.PhysMaterial);

		if ( ImpactEffect.Sound == None )	return;

		// ImpactEffect Sound 의 경우 처음 맞은 부분만 Play해주면 된다...
		// ImpactEffect Sound 의 경우 Pawn 이면 그리지 않는다...(Pawn 이 맞은 소리는 따로 처리해 준다....)

		if ( avaPawn( HitActor ) == None && !bBackFaceImpact )
		{
			PlaySound(ImpactEffect.Sound, true,,,HitLocation);
		}

		if ( !bBulletWhipSound )			return;
		if ( BulletWhip == None )			return;

		// BulletWhip Sound 를 Play 해준다...
		FireDir = Normal(HitLocation-Owner.Location);
		avaPlayerController( avaPawn(Owner).LocalPC ).CheckBulletWhip(BulletWhip, Owner.Location, FireDir, HitLocation);
	}
}

static function PhysicalMaterial GetDefaultPhysicalMaterial()
{
	return Default.DefaultPhysicalMaterial;
}

simulated function EjectBullet()
{
	local Emitter	AnEmitter;			// The Effects for the explosion 
	local vector	l;
	local rotator	r;

	if ( SocMesh == None )														return;
	// Character Detail 이 최상이 아니면 3인칭 탄피를 찍지 않는다....
	if ( avaPawn( Instigator ).LODBias >= 1 )									return;
	if ( class<avaWeap_BaseGun>(WeaponClass).default.BulletTemplete == None )	return;

	if ( SocMesh.GetSocketWorldLocationAndRotation( 'ejector', l, r ) )
	{
		r.Pitch = 0;
		r.Roll	= 0;
		AnEmitter = avaGameReplicationInfo(WorldInfo.GRI).ObjectPool.GetBulletEmitter(class<avaWeap_BaseGun>(WeaponClass).default.BulletTemplete, l, r);		
		AnEmitter.ParticleSystemComponent.ActivateSystem();
		AnEmitter.ParticleSystemComponent.SetOcclusionGroup(Mesh.OcclusionGroup);				
		AnEmitter.SetTimer( 3, FALSE, 'HideSelf' );
	}
}

simulated function SetFadeOut( PrimitiveComponent Comp, optional float MaxVisibleDistance = 300.0 )
{	
	Comp.bFadingOut = true;
	Comp.FadeStart	= MaxVisibleDistance;
	Comp.FadeEnd	= Comp.FadeStart;	
}

simulated function ChangeWeaponState( int NewState )
{
	WeaponState = NewState;
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

static simulated function PreCache( out array< object > outList )
{
	local int i;	
	Super.PreCache( outList );
	for ( i = 0 ; i < default.LinkedWeapon.ItemParts.Length ; ++ i )
	{
		DLO( default.LinkedWeapon.ItemParts[i].MeshName, outList );
	}	

	DLO( default.MeshName, outList );
	DLO( default.SocMeshName, outList );
	DLO( default.BasicMeshName, outList );
	DLO( default.MeshName$"_UI", outList );
}

static event LoadDLOs()
{
	local array< object > outList;
	PreCache( outList );
}

defaultproperties
{
	/* 고의적으로 Components에 추가하지 않은 것입니다 */
	Begin Object Class=avaBulletTrailComponent name=BulletTrailComponent0				
		AddDeltaTime=0.0
	End Object
	BulletTrailComponent=BulletTrailComponent0
	
	Begin Object Class=OcclusionGroupComponent	 name=HitEffectOcclusionGroup0		
	End Object
	HitEffectOcclusionGroup=HitEffectOcclusionGroup0	

	TrailInterval			=	3
	NetUpdateFrequency		=	10
	RemoteRole				=	ROLE_None
	bReplicateInstigator	=	true
	MaxImpactEffectDistance	=	4000.0
	MaxFireEffectDistance	=	10000.0
	MuzzleFlashDuration		=	0.3
	MuzzleFlashColor		=	(R=255,G=255,B=255,A=255)
	DefaultPhysicalMaterial	=	PhysicalMaterial'avaPhyMats.Default'
	DefaultImpactEffect		=	(Sound=SoundCue'avaPhySounds.Impact.ConcreteImpact',ParticleTemplate=ParticleSystem'avaEffect.Particles.P_WP_Enforcer_Impact',DecalMaterial=DecalMaterial'avaDecal.Metal.M_Metal_Hole01',DecalWidth=4.0,DecalHeight=4.0)	
	
	AttachmentBoneName		=	WPBone01	
	bRightHandedWeapon		=	true

	BloodSpurtDecalBig(0)	=	Material'avaDecal.Blood-Big.Blood-Big01'
	BloodSpurtDecalBig(1)	=	Material'avaDecal.Blood-Big.Blood-Big02'
	BloodSpurtDecalBig(2)	=	Material'avaDecal.Blood-Big.Blood-Big03'
	
	BloodSpurtDecalMid(0)	=	Material'avaDecal.Blood-Middle.Blood-Middle01'
	BloodSpurtDecalMid(1)	=	Material'avaDecal.Blood-Middle.Blood-Middle02'
	BloodSpurtDecalMid(2)	=	Material'avaDecal.Blood-Middle.Blood-Middle03'

	BloodSpurtDecalSmall(0)	=	Material'avaDecal.Blood-Small.Blood-Small01'	
	BloodSpurtDecalSmall(1)	=	Material'avaDecal.Blood-Small.Blood-Small02'	
	BloodSpurtDecalSmall(2)	=	Material'avaDecal.Blood-Small.Blood-Small03'	
	
	UIAttachmentBoneName	=	WPBone01
	AttachmentWeaponType	=	WBT_SMG01
}
