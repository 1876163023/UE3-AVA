class avaKActor_NoSync extends avaKActor;

defaultproperties
{
	Begin Object Name=StaticMeshComponent0	
		RBChannel				= RBCC_EffectPhysics
		RBDominanceGroup		= 23
		RBCollideWithChannels	= (Default=true,GameplayPhysics=true,EffectPhysics=true)
		BlockNonZeroExtent		= false
		BlockZeroExtent			= true
		BlockRigidBody			= true
	End Object

	bCollideActors		=	true
	bBlockActors		=	false
	bReplicateMovement	=	false
	bAlwaysRelevant		=	false

	bExchangedRoles		=	true
	RemoteRole=ROLE_None
}