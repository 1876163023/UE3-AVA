class avaWeap_SSG550RIS extends avaWeap_SSG550;

defaultproperties
{
	AttachmentClass			=	class'avaAttachment_SSG550RIS'

	BaseSkelMeshName		=	"Wp_Rif_M4.Wp_Rif_M4_Rail.MS_M4_Rail"
	BaseAnimSetName			=	"Wp_Rif_M4.Ani_M4A1_1P"
//기본적으로 도트를 붙인다.
	DefaultModifiers(0)		=	class'avaRules.avaMod_SSG550RIS_M_Dot'
}
