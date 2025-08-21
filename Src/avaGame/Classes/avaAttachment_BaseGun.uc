class avaAttachment_BaseGun extends avaWeaponAttachment;

var		StaticMeshComponent			SilencerMesh;			// 소음기 Mesh Component
var		string						SilencerMeshName;
//var	avaFlashlightComponent		FlashlightComponent;	//	
//var()	StaticMeshComponent			LightConeComponent;	

static simulated function PreCache( out array< object > outList )
{
	Super.PreCache( outList );
	DLO( default.SilencerMeshName, outList );	
}

static event LoadDLOs()
{
	local array< object > outList;
	super.LoadDLOs();
	PreCache( outList );
}

simulated function AttachItems()
{
	Super.AttachItems();
	AttachSilencer();
	//AttachFlashLight();
}

simulated function AttachSilencer()
{
	local StaticMesh	tempStaticMesh;
	if ( SilencerMeshName != "" )
	{
		tempStaticMesh = StaticMesh( DynamicLoadObject( SilencerMeshName, class'StaticMesh' ) );
		SilencerMesh.SetStaticMesh( tempStaticMesh );
		SilencerMesh.SetShadowParent( Mesh );
		SilencerMesh.SetOcclusionGroup( Mesh );		
		SilencerMesh.SetLightEnvironment( Mesh.LightEnvironment );
		SocMesh.AttachComponentToSocket(SilencerMesh, 'silencer' );
		ChangeSilencerVisibility( avaWeap_BaseGun( LinkedWeapon ).bMountSilencer );
	}
}

//simulated function AttachFlashLight()
//{
//	if ( avaWeap_BaseGun( LinkedWeapon ).bEnableFlashLight == true  && SocMesh != None )
//	{
//		if ( SocMesh.GetSocketByName( 'Flash' ) != None )
//		{
//			SocMesh.AttachComponentToSocket( FlashlightComponent, 'Flash' );
//			SocMesh.AttachComponentToSocket( LightConeComponent, 'Front' );
//			UpdateFlashLight();
//		}
//	}
//}
//
//simulated function UpdateFlashLight()
//{
//	local bool bActivated, bFirstPerson;
//	bActivated = avaWeap_BaseGun( LinkedWeapon ).bActivateFlashLight;
//	bFirstPerson = Instigator.IsFirstPerson();
//	FlashlightComponent.SetEnabled( bActivated && !bFirstPerson );
//	LightConeComponent.SetHidden( !bActivated || bFirstPerson );
//}

simulated function ApplyShadowParent( SkeletalMeshComponent MeshCpnt )
{
	Super.ApplyShadowParent( MeshCpnt );
	if ( MeshCpnt != None && SilencerMesh != none )
		SilencerMesh.SetLightEnvironment( MeshCpnt.LightEnvironment );
		
	if (SilencerMesh != None)
	{
		SilencerMesh.SetShadowParent( MeshCpnt );
		SilencerMesh.SetOcclusionGroup( MeshCpnt );		
	}	
}

simulated function ChangeSilencerVisibility( bool bVisible )
{
	if ( !bVisible )				
	{
		SilencerMesh.bCastHiddenShadow = false;		// add cascaded shadow ; 2007. 1. 7 changmin
		SilencerMesh.SetHidden( true );
	}
	else if ( !SocMesh.HiddenGame )	
	{
		SilencerMesh.SetHidden( false );
	}
	else if( SocMesh.bCastHiddenShadow )
	{
		SilencerMesh.bCastHiddenShadow = true;		// add cascaded shadow ; 2007. 1. 7 changmin
	}
}

simulated function ChangeMeshVisibility(bool bIsVisible, MeshComponent C)
{
	Super.ChangeMeshVisibility( bIsVisible, C );
	if ( SilencerMesh != None )
		ChangeSilencerVisibility( avaWeap_BaseGun( LinkedWeapon ).bMountSilencer );
	//UpdateFlashLight();
}

simulated event CauseMuzzleFlash( vector HitLocation )
{
	if ( avaWeap_BaseGun( LinkedWeapon ).bMountSilencer == true )	return;	// Silencer 장착 상태	
	Super.CauseMuzzleFlash( HitLocation );
}


simulated function FirstPersonFireEffects(Weapon PawnWeapon, vector HitLocation)	// Should be subclassed
{
	avaWeap_BaseGun(LinkedWeapon).PlayFiringSound2();

	Super.FirstPersonFireEffects(PawnWeapon,HitLocation);
}

simulated event ThirdPersonFireEffects(vector HitLocation)
{
	avaWeap_BaseGun(LinkedWeapon).PlayFiringSound2();

	Super.ThirdPersonFireEffects(HitLocation);
}

defaultproperties
{
	MuzzleFlashSocket=MuzzleFlashSocket
	MuzzleFlashPSCTemplate=ParticleSystem'avaEffect.Particles.P_WP_Rifle_MuzzleFlash_1P'	
	MuzzleFlashColor=(R=255,G=120,B=255,A=255)
	MuzzleFlashDuration=0.33
	MuzzleFlashLightClass=class'avaGame.avaGunMuzzleFlashLight'

	Begin Object Class=LightFunction Name=FlashLightFunction
		SourceMaterial=Material'avaEffect.Light.M_FlashLight'
		Scale=(X=1024.0,Y=1024.0,Z=1024.0)
	End Object
	
	//Begin Object Class=avaFlashlightComponent Name=SpotLightComponent0
	//    LightAffectsClassification	=	LAC_DYNAMIC_AND_STATIC_AFFECTING
	//    CastShadows					=	FALSE
	//    CastStaticShadows			=	FALSE
	//    CastDynamicShadows			=	FALSE
	//    bForceDynamicLight			=	FALSE
	//    UseDirectLightMap			=	FALSE
	//    LightingChannels			=	(BSP=TRUE,Static=TRUE,Dynamic=TRUE,bInitialized=TRUE)
	//	bEnabled					=	TRUE
	//	InnerConeAngle				=	12
	//	OuterConeAngle				=	20
	//	Brightness					=	2.0
	//	Function					=	FlashLightFunction
	//	Rotation					=	(Yaw = 32768)
	//End Object
	//FlashlightComponent = SpotLightComponent0
	
	//Begin Object Class=StaticMeshComponent Name=SpotLightComponent1
	//	bUseAsOccluder		=	FALSE
	//	StaticMesh			=	StaticMesh'avaEffect.Light.MS_Flash'
	//	HiddenGame			=	TRUE
	//	CastShadow			=	FALSE
	//	bAcceptsLights		=	FALSE
	//	BlockZeroExtent		=	FALSE
	//	BlockNonZeroExtent	=	FALSE
	//End Object
	//LightConeComponent=SpotLightComponent1

	Begin Object Class=StaticMeshComponent Name=SilencerMeshComponent0
		bUseAsOccluder = FALSE
	End Object
	SilencerMesh=SilencerMeshComponent0

}