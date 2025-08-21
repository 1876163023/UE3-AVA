class avaWeap_FamasG2 extends avaWeap_FamasF1;

defaultproperties
{
	ClipCnt			=	30
	AmmoCount			=	30
	MaxAmmoCount			=	90

	SightInfos(1) = (FOV=75,ChangeTime=0.15)

	bHideWeaponInSightMode = false
	bHideCursorInSightMode = false
	ScopeMeshName          = ""
	SightInAnim            = ""
	SightOutAnim           = ""
	bReleaseZoomAfterFire = false

	AttachmentClass			=	class'avaAttachment_FamasG2'

	BaseSkelMeshName		=	"Wp_Rif_Famas.Wp_Rif_Famas_G2.MS_Rif_Famas_Rail"
	BaseAnimSetName			=	"Wp_Rif_Famas.Ani_FamasG2_1P"
}
