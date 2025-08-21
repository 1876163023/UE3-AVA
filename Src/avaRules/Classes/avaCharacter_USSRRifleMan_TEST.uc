class avaCharacter_USSRRifleMan_TEST extends avaCharacter_RifleMan;

defaultproperties
{
	BaseSkeletalMesh	=	SkeletalMesh'avaCharCommon.NRF_RifleMan_Bone'
	Begin Object Name=WPawnSkeletalMeshComponent
		SkeletalMesh	=	SkeletalMesh'avaCharCommon.NRF_RifleMan_Bone'
	End Object
	DefaultTeam			=	TEAM_USSR
	BaseModifier(0)		=	class'avarules.avaMod_Mark_NRF'
	DefaultModifier.Add( class'avaMod_CharHeadA' )

//	TakeHitPhysicsBlendOutSpeed = 1.3
//	fTakeHitPhysicsStartWeight = 0.8
//	fTakeHitPhysicsMultiflier = 0.5
//	fComboHitPhysicsMultiflier	= 0.5

	TakeHitPhysicsBlendOutSpeed = 2
	fTakeHitPhysicsStartWeight = 2
	fTakeHitPhysicsMultiflier = 1.5
	fComboHitPhysicsMultiflier = 1
}