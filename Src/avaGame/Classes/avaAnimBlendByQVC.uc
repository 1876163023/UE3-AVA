//=============================================================================
//  avaAnimBlendByQVC
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
// 
//	2006/03/28 by OZ
//		1. Quick Voice Chat �� ����ȣ�� Play �ϱ� ���� animblend
//=============================================================================
class avaAnimBlendByQVC extends avaAnimBlendBase
	Native;

cpptext
{
	virtual void PlayAnim(UBOOL bLoop/* =FALSE */,FLOAT Rate/* =1.000000 */,FLOAT StartTime/* =0.000000 */);
}

defaultproperties
{
	Children(0)=(Name="child0",Weight=1.0)
}