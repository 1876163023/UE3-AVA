//=============================================================================
//  avaSkeletalMeshComponent
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
// 
//	2006/04/07 by OZ
//		CollisionGroup 을 Modify 하기 위한 Code 추가.
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

	// CollisionGroup 을 Modify 하기 위한 용도이다.
	virtual void InitComponentRBPhys(UBOOL bFixed);
}

DefaultProperties
{
	PhysicsAssetCollisionGroupIdx = 0
}
