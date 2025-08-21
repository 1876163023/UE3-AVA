class avaCharacter_USSRRifleMan extends avaCharacter_RifleMan;

defaultproperties
{
	BaseSkeletalMesh	=	SkeletalMesh'avaCharCommon.NRF_RifleMan_Bone'
	Begin Object Name=WPawnSkeletalMeshComponent
		SkeletalMesh	=	SkeletalMesh'avaCharCommon.NRF_RifleMan_Bone'
	End Object
	DefaultTeam			=	TEAM_USSR
	BaseModifier(0)		=	class'avarules.avaMod_Mark_NRF'
	DefaultModifier.Add( class'avaMod_CharHeadA' )
}