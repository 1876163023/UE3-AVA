class avaAttachment_DesertEagle extends avaAttachment_BasePistol;

defaultproperties
{
	WeaponClass			=	class'avaWeap_DesertEagle'

	DeathIconStr		=	"f"

	bMeshIsSkeletal		=	true
	MeshName			=	"Wp_Pis_DesertEG.3P_Pos"
	SocMeshName			=	"Wp_Pis_DesertEG.3P_Soc"
	BasicMeshName		=	"Wp_Pis_DesertEG.MS_DesertEG_3p"
	PosRootBoneName		=	root
	SocRootBoneName		=	root
	//AttachmentBoneName;		// Character �� �տ� �ش��ϴ� Bone Name
	//CarriedSocketName	=	Weapon01
	AnimPrefix			=	Desert

//�߻�� ���� �׽�Ʈ
	AbsMuzzleFlashPSCTemplate = ParticleSystem'avaEffect.Particles.P_WP_Sniper_Muzzlesmoke_3P'
// �������� ������ ���� ���̱�
	MuzzleFlashSocket=MuzzleFlashSocket
	MuzzleFlashPSCTemplate=ParticleSystem'avaEffect.Particles.P_WP_SniperRifle_MuzzleFlash_3P'	
	MuzzleFlashColor=(R=255,G=120,B=255,A=255)
	MuzzleFlashDuration=0.33
	MuzzleFlashLightClass=class'avaGame.avaGunMuzzleFlashLight'
}
