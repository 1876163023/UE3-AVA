/*=============================================================================
	avaAnimBlendByRun
 
	Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2007/01/08 by OZ

		Run Node �� ����ȭ�ϱ� ���Ͽ�...
		
=============================================================================*/
class avaAnimBlendByRun extends avaAnimBlendBase
	native;

var		float	NormalModeElapsedTime;		// Delta Seconds �� ������ Time �� �ʿ�....
var()	float	NormalModeTransitionTime;	// Delta Seconds Max
var		float	LastRotationYaw;
var()	float	MaxRotationDelta;

cpptext
{
	virtual void TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight );
}

defaultproperties
{
	Children(0)					=	(Name="Normal",Weight=1.0)
	Children(1)					=	(Name="Idle")
	bFixNumChildren				=	true
	NormalModeTransitionTime	=	1.0
	MaxRotationDelta			=	50000.0
}

