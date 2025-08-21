class avaCharacter_USSRSniper extends avaCharacter_Sniper;

defaultproperties
{
	BaseSkeletalMesh	=	SkeletalMesh'avaCharCommon.NRF_SniperMan_Bone'
	Begin Object Name=WPawnSkeletalMeshComponent
		SkeletalMesh	=	SkeletalMesh'avaCharCommon.NRF_SniperMan_Bone'
	End Object
	DefaultTeam			=	TEAM_USSR
	BaseModifier(0)		=	class'avarules.avaMod_Mark_NRF'
	DefaultModifier.Add( class'avaMod_CharHeadB' )
}