class avaCharacter_EURifleMan extends avaCharacter_RifleMan;

defaultproperties
{
	BaseSkeletalMesh	=	SkeletalMesh'avaCharCommon.EU_RifleMan_Bone01'
	Begin Object Name=WPawnSkeletalMeshComponent
		SkeletalMesh	=	SkeletalMesh'avaCharCommon.EU_RifleMan_Bone01'
	End Object
	DefaultTeam			=	TEAM_EU
	BaseModifier(0)	=	class'avarules.avaMod_Mark_EU'
	//캐릭터 생성시 기본 얼굴과 같아야 한다
	DefaultModifier.Add( class'avaMod_CharHeadC' )
}