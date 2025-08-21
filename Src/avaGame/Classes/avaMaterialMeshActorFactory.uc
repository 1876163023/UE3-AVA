/*
	avaMaterialMeshActor Factory

	2007/01/03	°í±¤·Ï
*/
class avaMaterialMeshActorFactory extends ActorFactoryStaticMesh
	config(Editor)
	native;

cpptext
{
	virtual AActor* CreateActor( const FVector* const Location, const FRotator* const Rotation, const class USeqAct_ActorFactory* const ActorFactoryData );
}

defaultproperties
{
	MenuName="Add avaMaterialMesh"
	NewActorClass=class'avaMaterialMeshActor'
}
