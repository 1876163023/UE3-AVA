class avaCharacter_EURifleMan extends avaCharacter_RifleMan;

defaultproperties
{
	BaseSkeletalMesh	=	SkeletalMesh'avaCharCommon.EU_RifleMan_Bone01'
	Begin Object Name=WPawnSkeletalMeshComponent
		SkeletalMesh	=	SkeletalMesh'avaCharCommon.EU_RifleMan_Bone01'
	End Object
	DefaultTeam			=	TEAM_EU
	BaseModifier(0)	=	class'avarules.avaMod_Mark_EU'
	//ĳ���� ������ �⺻ �󱼰� ���ƾ� �Ѵ�
	DefaultModifier.Add( class'avaMod_CharHeadC' )
}