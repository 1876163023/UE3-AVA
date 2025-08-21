/*=============================================================================
  avaAttachment_RPG7
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2006/06/13 by OZ
		RPG7 을 위한 Attachment

	ShellMesh 의 Visibility 는 LinkedWeapon 의 ReloadCnt 와 연동이 된다.
	(ReloadCnt 가 있으면 보이고 없으면 안보임)

=============================================================================*/
class avaAttachment_RPG7 extends avaWeaponAttachment;

var MeshComponent	ShellMesh;		// 탄두 Static Mesh
var string			ShellMeshName;

// RPG7 은 MuzzleFlash 를 후폭풍 효과 용으로 사용한다.... 
// 후폭풍의 경우 Socket 에 Binding 하지 않는다...
simulated function AttachMuzzleFlash()
{
	local SkeletalMeshComponent SKMesh;
	if ( SocMesh == None )	return;
	SKMesh = SocMesh;
	if (SKMesh != none && MuzzleFlashSocket != '')
	{
		if (MuzzleFlashPSCTemplate != None || MuzzleFlashAltPSCTemplate != None)
		{
			MuzzleFlashPSC = new(self) class'avaParticleSystemComponent';
			AttachComponent(MuzzleFlashPSC);
			MuzzleFlashPSC.SetColorParameter('MuzzleFlashColor', MuzzleFlashColor);
			MuzzleFlashPSC.SetAbsolute(true,true);
			MuzzleFlashPSC.bAutoActivate = FALSE;  
		}
	}
}

simulated function CauseMuzzleFlashLight()
{
	local vector	l;
	local rotator	r;
	if ( SocMesh == None )	return;
	if ( SocMesh.GetSocketWorldLocationAndRotation( MuzzleFlashSocket, l, r ) )
	{
		MuzzleFlashLight = new(self) MuzzleFlashLightClass;
		AttachComponent(MuzzleFlashLight);
	}
}

// RPG7 은 MuzzleFlash 를 후폭풍 효과 용으로 사용한다....
simulated event CauseMuzzleFlash( vector HitLocation )
{
	local vector	l;
	local rotator	r;
	if ( SocMesh == None )	return;
	if ( !SocMesh.GetSocketWorldLocationAndRotation( MuzzleFlashSocket, l, r ) )
		return;

	if ( MuzzleFlashLight == None )
	{
		if ( MuzzleFlashLightClass != None )
			CauseMuzzleFlashLight();
	}
	else
	{
		MuzzleFlashLight.ResetLight();
	}

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
			MuzzleFlashPSC.SetTranslation( l );
			MuzzleFlashPSC.SetRotation( r );
			MuzzleFlashPSC.ActivateSystem();
		}
	}
	SetTimer(MuzzleFlashDuration,false,'MuzzleFlashTimer');
}

simulated function AttachItems()
{
	local StaticMesh	tempStaticMesh;

	Super.AttachItems();

	if ( ShellMeshName != "" )
	{
		tempStaticMesh = StaticMesh( DynamicLoadObject( ShellMeshName, class'StaticMesh' ) );
		StaticMeshComponent(ShellMesh).SetStaticMesh( tempStaticMesh );
		ShellMesh.SetShadowParent( Mesh );
		ShellMesh.SetLightEnvironment( Mesh.LightEnvironment );
	
		SocMesh.AttachComponentToSocket(ShellMesh, 'Shell' );
	}
}

simulated function ApplyShadowParent( SkeletalMeshComponent MeshCpnt )
{
	Super.ApplyShadowParent( MeshCpnt );
	if ( MeshCpnt != None )
		ShellMesh.SetLightEnvironment( MeshCpnt.LightEnvironment );
}

simulated function ChangeShellMeshVisibility( bool bVisible )
{
	if ( !bVisible )				ShellMesh.SetHidden( true );
	else if ( !SocMesh.HiddenGame )	ShellMesh.SetHidden( false );
}

// 매 Tick 마다 Visible 을 Check 해도 될까?
// 이것 때문에 느려질것 같지는 않지만.... 재일 간단하게 처리하는 방법이다...
// 나중에 문제가 생기면 수정하자~~ 2006/06/13 by OZ 
event Tick( float DeltaTime )
{
	if ( LinkedWeapon != None && avaWeap_BaseGun( LinkedWeapon ) != None )
		ChangeShellMeshVisibility( avaWeap_BaseGun( LinkedWeapon ).ReloadCnt > 0 );
}

defaultproperties
{
	WeaponClass	= class'avaWeap_BaseBazooka'
	DeathIconStr="w"

	// 탄두 Mesh
	Begin Object Class=StaticMeshComponent Name=ShellMeshComponent0
	End Object
	ShellMesh=ShellMeshComponent0

	bMeshIsSkeletal		=	true
	MeshName			=	"Wp_Heavy_RPG7.3P_Pos"
	SocMeshName			=	"Wp_Heavy_RPG7.3P_Soc"
	BasicMeshName		=	"Wp_Heavy_RPG7.MS_Heavy_RPG7_3p"
	PosRootBoneName		=	root
	SocRootBoneName		=	root
	//AttachmentBoneName;		// Character 의 손에 해당하는 Bone Name
	CarriedSocketName	=	B3


	AttachmentWeaponType = WBT_SMG01
	AnimPrefix = RPG7

	MuzzleFlashSocket=MuzzleFlashSocket
	MuzzleFlashPSCTemplate=ParticleSystem'avaEffect.Heavyweapons.PS_RPG_Smoke_Back'	
	MuzzleFlashColor=(R=255,G=120,B=255,A=255)
	MuzzleFlashDuration=1
	MuzzleFlashLightClass=None

	ShellMeshName		=	"Wp_Heavy_RPG7.MS_Heavy_RPG7_missile_3p"
}
