/*=============================================================================
  avaUIScene
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2006/08/21 by OZ

		UIScene 에서 확장한 avaGame을 위한 UIScene Class 이다.
		
=============================================================================*/
class avaUIScene extends UIScene
	native;

cpptext
{
	virtual void Initialize( UUIScene* inOwnerScene, UUIObject* inOwner=NULL );		
}