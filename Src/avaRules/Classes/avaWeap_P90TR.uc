class avaWeap_P90TR extends avaWeap_P90;

defaultproperties
{
	SightInfos(0) = (FOV=90,ChangeTime=0.2)
	SightInfos(1) = (FOV=75,ChangeTime=0.15)

	bHideWeaponInSightMode = true
	bHideCursorInSightMode = true
	bReleaseZoomAfterFire = false

	AttachmentClass		= class'avaAttachment_P90TR'
	BaseSkelMeshName	= "Wp_Smg_P90TR.MS_Smg_P90TR"
	BaseAnimSetName		= "Wp_Smg_P90TR.Ani_P90Rail_1P"

	WeaponFireSnd			=	SoundCue'avaWeaponSounds.SMG_P90.SMG_P90_fire'	

	SilencerMeshName		= "Wp_Silencer.MS_SMG_MP5K_Silencer"
	SilencerBoneName		= Bone05
}