//=============================================================================
//  avaKActorFactory
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
// 
//	2006/03/22 by OZ
//		1. Editor ���� ���ϰ� avaKActor �� ��ġ�ϱ� ����.
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