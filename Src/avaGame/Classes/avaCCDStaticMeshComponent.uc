//=============================================================================
//  avaCCDStaticMeshComponent
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
// 
//	2006/03/16 by OZ
//		1. Continuos Collision Detection 을 지원하기 위해 StaticMeshComponent 를 확장한 Class
//=============================================================================

class avaCCDStaticMeshComponent extends StaticMeshComponent
	native;

cpptext
{
	virtual void InitComponentRBPhys(UBOOL bFixed);
}