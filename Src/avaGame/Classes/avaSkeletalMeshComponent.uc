//=============================================================================
//  avaSkeletalMeshComponent
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
// 
//	2006/04/07 by OZ
//		CollisionGroup �� Modify �ϱ� ���� Code �߰�.
//=============================================================================

class avaSkeletalMeshComponent extends SkeletalMeshComponent
	native;

var()	int				PhysicsAssetCollisionGroupIdx;
var		bool			bNoForceUpdateBound;

native function SetPhysicsAssetCollisionGroup(int GroupIdx);

cpptext
{
	virtual void UpdateBounds();

	virtual void Tick(FLOAT DeltaTime);

	// CollisionGroup �� Modify �ϱ� ���� �뵵�̴�.
	virtual void InitComponentRBPhys(UBOOL bFixed);
}

DefaultProperties
{
	PhysicsAssetCollisionGroupIdx = 0
}
