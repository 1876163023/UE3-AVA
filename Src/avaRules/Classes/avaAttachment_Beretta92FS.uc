class avaAttachment_Beretta92FS extends avaAttachment_BasePistol;

defaultproperties
{
	WeaponClass=class'avaWeap_Beretta92FS'

	DeathIconStr="0"
	
	bMeshIsSkeletal		=	true
	MeshName			=	"Wp_Pis_Beretta92Fs.3P_Pos"
	SocMeshName			=	"Wp_Pis_Beretta92Fs.3P_Soc"
	BasicMeshName		=	"Wp_Pis_Beretta92Fs.MS_Pis_Bereta92Fs_3p"
	PosRootBoneName		=	root
	SocRootBoneName		=	root
	//AttachmentBoneName;		// Character 의 손에 해당하는 Bone Name
	//CarriedSocketName	=	Weapon01
	AnimPrefix = Beretta
	SilencerMeshName = "Wp_Silencer.MS_P226_Silencer_3p"
}
