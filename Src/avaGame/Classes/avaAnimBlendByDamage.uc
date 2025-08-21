/*=============================================================================
  avaAnimBlendByDamage
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2006/07/18 by OZ

		Damage 시 Animation 을 Play 하기 위한 Blend Node
		
=============================================================================*/
class avaAnimBlendByDamage extends avaAnimBlendBase
	native;

//var EShotInfo	LastShotInfo;
//var float		LastShotAngle;
//var float		DurationTime;

cpptext
{
	virtual	void TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight );
}

// 맞은위치, 맞은방향(Angle), Damage Motion 지속시간
//function Damaged( EShotInfo si, float angle, float fDurationTime )
//{
//	LastShotInfo	= si;
//	LastShotAngle	= angle;
//	DurationTime	= fDurationTime;
//}

defaultproperties
{
	Children(0)=(Name="None",Weight=1.0)
	Children(1)=(Name="UpperBody")
	bFixNumChildren=true
}