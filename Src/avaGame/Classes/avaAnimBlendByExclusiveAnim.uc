/*=============================================================================
  avaAnimBlendByExclusiveAnim
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2006/06/12 by OZ
		
		이 node 는 상하체 분리 Animation 이 아닌 독점적인 Animation 을 Play 하도록 한다.
		중화기를 거치한 경우, 거치된 중화기를 잡은 경우에 해당한다.
		(움직이지 못하는 경우...)
		
=============================================================================*/
class avaAnimBlendByExclusiveAnim extends avaAnimBlendBase
	native;

cpptext
{
	virtual	void TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight );
}

defaultproperties
{
	Children(0)=(Name="None Exc",Weight=1.0)
	Children(1)=(Name="Install")
	Children(2)=(Name="Fixed")
	bFixNumChildren=true
}
