/*=============================================================================
  avaHudObject
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2006/08/21 by OZ

		HUD 에 쓰일 UI Object 를 위한 상위 Class이다.
		UTHudObject 를 참조
		
=============================================================================*/
class avaHudObject extends avaUIObject
	abstract
	native;

native function HUD GetHudOwner();

defaultproperties
{
}
