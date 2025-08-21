class avaKActor_Debris extends avaKActor native;

native function Initialize( bool bInteractive, vector InVelocity, float Burst, avaKActor Parent );

simulated function InitializeForPooling()
{
	SetHidden(FALSE);
	StaticMeshComponent.SetHidden(FALSE);
	SetPhysics(Phys_RigidBody);
}

simulated function Recycle()
{
	SetHidden(TRUE);
	StaticMeshComponent.SetStaticMesh(None);
	StaticMeshComponent.SetHidden(TRUE);
	SetPhysics(PHYS_None);
	ClearTimer('Recycle');
}

defaultproperties
{
	bNoDelete=false
	RemoteRole=ROLE_None

	Begin Object Name=StaticMeshComponent0
		//bUseHardwareScene				=	true
		RBChannel=RBCC_EffectPhysics
		RBCollideWithChannels=(Default=TRUE,GameplayPhysics=TRUE,EffectPhysics=TRUE)
		RBDominanceGroup=23
	End Object
}