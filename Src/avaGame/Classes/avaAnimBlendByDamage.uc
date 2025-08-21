/*=============================================================================
  avaAnimBlendByDamage
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2006/07/18 by OZ

		Damage �� Animation �� Play �ϱ� ���� Blend Node
		
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

// ������ġ, ��������(Angle), Damage Motion ���ӽð�
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