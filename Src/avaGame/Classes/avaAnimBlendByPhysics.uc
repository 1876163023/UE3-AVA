/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaAnimBlendByPhysics extends avaAnimBlendBase
		Native;

/** Maps the PSY_enums to child nodes */

var(Animations) int    		PhysicsMap[12];

/** Holds the last known physics type for the tree's owner. */

var int						LastPhysics;		// Track the last physics

cpptext
{
	virtual	void TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight  );
}


defaultproperties
{
	PhysicsMap(0)	=-1		//PHYS_None,
	PhysicsMap(1)	= 0		//PHYS_Walking,
	PhysicsMap(2)	= 1		//PHYS_Falling,
	PhysicsMap(3)	=-1		//PHYS_Swimming,
	PhysicsMap(4)	=-1		//PHYS_Flying,
	PhysicsMap(5)	=-1		//PHYS_Rotating,
	PhysicsMap(6)	=-1		//PHYS_Projectile,
	PhysicsMap(7)	= 0		//PHYS_Interpolating,
	PhysicsMap(8)	=-1		//PHYS_Spider,
	PhysicsMap(9)	= 2			//PHYS_Ladder,
	PhysicsMap(10)	=-1		//PHYS_RigidBody,
	PhysicsMap(11)	=-1

	// 현재는 PHYS_Walking, PHYS_Falling, PHYS_Ladder 만 사용함
	Children(0)=(Name="PHYS_Walking",Weight=1.0)
	Children(1)=(Name="PHYS_Falling")
	Children(2)=(Name="PHYS_Ladder")
}
