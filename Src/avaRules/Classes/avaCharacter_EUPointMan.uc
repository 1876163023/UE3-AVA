class avaCharacter_EUPointMan extends avaCharacter_PointMan;

defaultproperties
{
	BaseSkeletalMesh	=	SkeletalMesh'avaCharCommon.EU_PointMan_Bone01'	
	Begin Object Name=WPawnSkeletalMeshComponent
		SkeletalMesh	=	SkeletalMesh'avaCharCommon.EU_PointMan_Bone01'
	End Object
	DefaultTeam			= 	TEAM_EU
	BaseModifier(0)		=	class'avarules.avaMod_Mark_EU'
	//BaseModifier(1)		=	class'avaMod_TestHead'
}