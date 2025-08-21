class avaCharacter_USSRPointMan extends avaCharacter_PointMan;

defaultproperties
{
	BaseSkeletalMesh	=	SkeletalMesh'avaCharCommon.NRF_PointMan_Bone'
	Begin Object Name=WPawnSkeletalMeshComponent
		SkeletalMesh	=	SkeletalMesh'avaCharCommon.NRF_PointMan_Bone'
	End Object
	DefaultTeam			=	TEAM_USSR
	BaseModifier(0)	=	class'avarules.avaMod_Mark_NRF'
	DefaultModifier.Add( class'avaMod_CharHeadE' )
}