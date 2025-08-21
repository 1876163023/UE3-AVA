//=============================================================================
//  avaKActorFactory
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
// 
//	2006/03/22 by OZ
//		1. Editor 에서 편하게 avaKActor 를 배치하기 위함.
//=============================================================================
class avaKActorFactory extends ActorFactoryRigidBody
	config(Editor)
	collapsecategories
	hidecategories(Object)
	native;

var() class<avaKActor> InventoryClass;

cpptext
{
	virtual AActor* CreateActor( const FVector* const Location, const FRotator* const Rotation, const class USeqAct_ActorFactory* const ActorFactoryData );
}

defaultproperties
{
	MenuName="Add avaKActor"

	NewActorClass=class'avaKActor'
	//GameplayActorClass=class'Engine.KActorSpawnable'
}