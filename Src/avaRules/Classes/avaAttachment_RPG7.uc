/*=============================================================================
  avaAttachment_RPG7
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2006/06/13 by OZ
		RPG7 �� ���� Attachment

	ShellMesh �� Visibility �� LinkedWeapon �� ReloadCnt �� ������ �ȴ�.
	(ReloadCnt �� ������ ���̰� ������ �Ⱥ���)

=============================================================================*/
class avaAttachment_RPG7 extends avaWeaponAttachment;

var MeshComponent	ShellMesh;		// ź�� Static Mesh
var string			ShellMeshName;

// RPG7 �� MuzzleFlash �� ����ǳ ȿ�� ������ ����Ѵ�.... 
// ����ǳ�� ��� Socket �� Binding ���� �ʴ´�...
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

// RPG7 �� MuzzleFlash �� ����ǳ ȿ�� ������ ����Ѵ�....
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

// �� Tick ���� Visible �� Check �ص� �ɱ�?
// �̰� ������ �������� ������ ������.... ���� �����ϰ� ó���ϴ� ����̴�...
// ���߿� ������ ����� ��������~~ 2006/06/13 by OZ 
event Tick( float DeltaTime )
{
	if ( LinkedWeapon != None && avaWeap_BaseGun( LinkedWeapon ) != None )
		ChangeShellMeshVisibility( avaWeap_BaseGun( LinkedWeapon ).ReloadCnt > 0 );
}

defaultproperties
{
	WeaponClass	= class'avaWeap_BaseBazooka'
	DeathIconStr="w"

	// ź�� Mesh
	Begin Object Class=StaticMeshComponent Name=ShellMeshComponent0
	End Object
	ShellMesh=ShellMeshComponent0

	bMeshIsSkeletal		=	true
	MeshName			=	"Wp_Heavy_RPG7.3P_Pos"
	SocMeshName			=	"Wp_Heavy_RPG7.3P_Soc"
	BasicMeshName		=	"Wp_Heavy_RPG7.MS_Heavy_RPG7_3p"
	PosRootBoneName		=	root
	SocRootBoneName		=	root
	//AttachmentBoneName;		// Character �� �տ� �ش��ϴ� Bone Name
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
