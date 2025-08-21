class avaCharacter_EUSniper extends avaCharacter_Sniper;

defaultproperties
{
	BaseSkeletalMesh	=	SkeletalMesh'avaCharCommon.EU_SniperMan_Bone01'
	HelmetMeshName		=	""
	EnableTakeOffHelmet	=	false
	Begin Object Name=WPawnSkeletalMeshComponent
		SkeletalMesh	=	SkeletalMesh'avaCharCommon.EU_SniperMan_Bone01'
	End Object
	DefaultTeam			=	TEAM_EU
	BaseModifier(0)	=	class'avarules.avaMod_Mark_EU'
	DefaultModifier.Add( class'avaMod_CharHeadD' )
}