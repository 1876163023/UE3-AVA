/*=============================================================================
  avaHelmet
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.

	2006/02/28
	1. Helmet 착용 중 HeadShot 시 Helmet 이 날라가도록 하기 위하여 날가가는 것은 동기화 하지 않음(Actor 와 충돌 없음)

	2006/07/24
	1. Helmet 의 Mesh 와 Skin 및 Accessory 를 외부에서(Modifier에서) 수정 가능하도록 바꿈

=============================================================================*/
class avaHelmet extends KActor;

var bool						bAttached;			// Helmet 이 Instigator 에 붙어 있는지에 대한 Flag
var() SkeletalMeshComponent		Mesh;

// Helmet 에 붙는 Accessory...
struct HelmetAccessory
{
	var name					SocketName;
	var string					MeshName;
	var StaticMeshComponent		Mesh;
	var	float					MaxVisibleDistance;
};

var array<HelmetAccessory>		Accessory;

static function DLO( string resource )
{
	if (resource != "")
	{		
		DynamicLoadObject( resource, class'Object' );
	}
}

static event LoadDLOs()
{
	local int i;		

	for ( i = 0 ; i < default.Accessory.length ; ++ i )
	{
		DLO( default.Accessory[i].MeshName );
	}
}

simulated function SetMesh( string MeshName )
{
	local SkeletalMesh	tmpSkelMesh;
	tmpSkelMesh = SkeletalMesh( DynamicLoadObject( MeshName,class'SkeletalMesh') );
	if ( tmpSkelMesh != None )	Mesh.SetSkeletalMesh( tmpSkelMesh );
	else						`warn( "avaHelmet.SetMesh Cannot Load SkeletalMesh" @MeshName );
}

simulated function ChangeSkin( string SkinName )
{
	local Material		tmpMaterial;
	tmpMaterial = Material( DynamicLoadObject(SkinName,class'Material') );
	if ( tmpMaterial != None )	Mesh.SetMaterial( 0, tmpMaterial );
	else						`warn( "avaHelmet.ChangeSkin Cannot Load Material" @SkinName );
}

simulated function SetFadeOut( PrimitiveComponent Comp, optional float MaxVisibleDistance = 300.0 )
{	
	Comp.bFadingOut = true;
	Comp.FadeStart	= MaxVisibleDistance;
	Comp.FadeEnd	= Comp.FadeStart;	
}

simulated function AddAccessory( string MeshName, name SocketName, float MaxVisibleDistance )
{
	local int id;
	id								= Accessory.length;
	Accessory.length				= id + 1;
	Accessory[id].MeshName			= MeshName;
	Accessory[id].SocketName		= SocketName;
	Accessory[id].MaxVisibleDistance= MaxVisibleDistance;
}

simulated function AttachAccessory()
{
	local int i;
	local StaticMesh	staticmesh;
	for ( i = 0 ; i < Accessory.length ; ++ i )
	{
		if ( Mesh.GetSocketByName( Accessory[i].SocketName ) == None )
		{
			`warn( "avaHelmet.AttachAccessory Cannot Find Socket in" @Mesh );
			continue;
		}

		if ( Accessory[i].MeshName != "" )
		{
			if ( Accessory[i].Mesh == None )
			{
				staticmesh = StaticMesh( DynamicLoadObject( Accessory[i].MeshName, class'StaticMesh' ) );
				if ( staticMesh != None )
				{
					Accessory[i].Mesh = new(outer) class'StaticMeshComponent';
					Accessory[i].Mesh.bUseAsOccluder = FALSE;
					Accessory[i].Mesh.SetStaticMesh( staticmesh );
				}
				else
				{
					`warn( "avaHelmet.AttachAccessory Cannot Load StaticMesh" @Accessory[i].MeshName );
				}
			}

			if ( Accessory[i].Mesh != None )
			{
				Accessory[i].Mesh.SetShadowParent( Mesh.ShadowParent );
				Accessory[i].Mesh.SetOcclusionGroup( Mesh.OcclusionGroup );
				Accessory[i].Mesh.SetBlockRigidBody( false );
				Accessory[i].Mesh.CastShadow = true;				
			}

			if ( !Accessory[i].Mesh.bAttached )
				Mesh.AttachComponentToSocket( Accessory[i].Mesh, Accessory[i].SocketName );

			if ( Accessory[i].MaxVisibleDistance > 0.0 )
				SetFadeOut( Accessory[i].Mesh , Accessory[i].MaxVisibleDistance );
		}
	}
	UpdateAttachmentVisibility();
}

simulated function MarkSeeThroughGroupIndex( int SeeThroughGroupIndex )
{
	local int i;
	
	Mesh.SetSeeThroughGroupIndex( SeeThroughGroupIndex );
	
	for ( i = 0 ; i < Accessory.length ; ++ i )
	{	
		if (Accessory[i].Mesh != None)
		{
			Accessory[i].Mesh.SetSeeThroughGroupIndex( SeeThroughGroupIndex );
		}
	}
}

simulated function SetLightEnvironment( LightEnvironmentComponent env )
{
	local int i;

	if (Mesh != None)
		Mesh.SetLightEnvironment( env );	

	for ( i = 0 ; i < Accessory.length ; ++ i )
	{
		if ( Accessory[i].Mesh != None )
			Accessory[i].Mesh.SetLightEnvironment( env );
	}
}

simulated function AttachTo( SkeletalMeshComponent MeshCpnt, name SocketName )
{
	local avaPawn P;	

	SetPhysics( PHYS_None );
	// Attach Weapon mesh to player skelmesh
	if ( Mesh != None )
	{
		//<@ add cascaded shadow ; 2008. 1. 7 changmin
		if( Instigator != None && Instigator.IsLocallyControlled() )
		{
			Mesh.bCastHiddenShadow = true;
		}
		//> changmin
		
		MeshCpnt.AttachComponentToSocket(Mesh, SocketName);
		MeshCpnt.AttachComponentToSocket(CollisionComponent, SocketName);
		//Mesh.AttachComponentToSocket(CollisionComponent, 'Bone01' );
		// Weapon Mesh Shadow
		Mesh.SetShadowParent(MeshCpnt);		
		Mesh.SetOcclusionGroup(MeshCpnt);
		
		P = avaPawn(Instigator);
		if ( P != None )
		{
			// If our pawn is locally controlled, respect the behindview flag.
			if ( Instigator.IsLocallyControlled() )
			{
				Mesh.SetHidden( P.IsFirstPerson() );
			}
		}
	}
	bAttached=true;
	CollisionComponent.SetBlockRigidBody( false );
}

simulated function UpdateAttachmentVisibility()
{
	local PrimitiveComponent	Primitive;
	local int					i;
	for ( i = 0 ; i < Mesh.Attachments.Length ; ++i )
	{
		Primitive = PrimitiveComponent( Mesh.Attachments[i].Component );
		if ( Primitive != None )	Primitive.SetHidden( Mesh.HiddenGame );
		
		//<@ 2008. 1. 7 changmin; add cascaded shadow
		if ( Instigator != None )
			Primitive.bCastHiddenShadow = Instigator.IsLocallyControlled();
		else
			Primitive.bCastHiddenShadow = false;
		//>@ changmin
	}
}

// bVisible 이 true 인 경우는 3인칭으로 바뀐 경우이다.
simulated function ChangeVisibility( bool bVisible )
{
	if ( bAttached && Mesh != None )
	{
		if ( bVisible == true )		Mesh.SetOwnerNoSee( !bVisible );
		Mesh.SetHidden(!bVisible);
		UpdateAttachmentVisibility();
	}
}

simulated function TakeOff( SkeletalMeshComponent MeshCpnt, vector momentum )
{
	local vector	Impulse;

	// Weapon Mesh Shadow
	if ( Mesh != None )	
	{
		Mesh.SetShadowParent(None);	
		Mesh.SetOcclusionGroup(None);
		
		//<@ add cascaded shadow ; 2008. 1. 7 changmin
		Mesh.bCastHiddenShadow = false;
		//>@ changmin
	}

	if ( MeshCpnt != None )	
	{
		// detach weapon mesh from player skelmesh
		if ( Mesh != None )
		{
			MeshCpnt.DetachComponent( Mesh );
			MeshCpnt.DetachComponent( CollisionComponent );
			AttachComponent( Mesh );
			AttachComponent( CollisionComponent );
		}
	}
	CollisionComponent.SetBlockRigidBody( true );
	CollisionComponent.SetTraceBlocking( true, false );
	SetPhysics( PHYS_RigidBody );
	Impulse = momentum;
	CollisionComponent.AddImpulse( Impulse );
	ChangeVisibility( true );
	bAttached=false;	

	/// 이제 별도의 세상으로~~ lighting도 별도로 해야함. :)
	//LightEnvironment.bEnabled = true;
	SetLightEnvironment( LightEnvironment );
}

event TakeDamage( int Damage, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional class<Weapon> weapon)
{
	if ( bAttached )
	{
		Instigator.TakeDamage( Damage, EventInstigator, HitLocation, Momentum, DamageType, HitInfo, weapon );
	}
	else
	{
		Super.TakeDamage( Damage, EventInstigator, HitLocation, Momentum, DamageType, HitInfo, weapon );		
	}
}

defaultproperties
{
	Begin Object Name=MyLightEnvironment		
 	End Object

	Components.Remove(StaticMeshComponent0)
	// KActor Collision 용...
	Begin Object Class=StaticMeshComponent Name=HeadMeshComponent		
		bUseAsOccluder = FALSE
		StaticMesh=StaticMesh'avaCharCommon.Mesh.HelmetCollision'
		BlockRigidBody=true
		HiddenGame=true
		HiddenEditor=true
		Translation=(x=0,y=0,z=4)
		BlockNonZeroExtent=false
		BlockZeroExtent=false
		//bUseHardwareScene=true
		RBChannel			= RBCC_EffectPhysics
		RBDominanceGroup	= 23
		RBCollideWithChannels =	(Default=true,GameplayPhysics=true,EffectPhysics=true)
	End Object
	Components.Add(HeadMeshComponent)
	CollisionComponent=HeadMeshComponent
	StaticMeshComponent=HeadMeshComponent

	// Visual 용...
	Begin Object Class=SkeletalMeshComponent Name=SkeletalMeshComponent0
		bUseAsOccluder = FALSE
		BlockRigidBody=FALSE
		bUpdateSkelWhenNotRendered=FALSE
		bNoSkeletonUpdate=1
	End Object
	Mesh=SkeletalMeshComponent0
	Components.Add(SkeletalMeshComponent0)

	bNoDelete=false
	bCollideActors=true
	bBlockActors=false
	Physics=PHYS_None
	RemoteRole=ROLE_None
	//bReplicateInstigator=true
}