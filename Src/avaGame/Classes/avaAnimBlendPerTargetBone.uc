//=============================================================================
//  avaAnimBlendPerTargetBone
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
// 
//	2006/03/28 by OZ
//=============================================================================
class avaAnimBlendPerTargetBone extends AnimNodeBlendPerBone
	native;

cpptext
{
	virtual void PlayAnim(UBOOL bLoop=FALSE,FLOAT Rate=1.000000,FLOAT StartTime=0.000000);
}

defaultproperties
{
	Child2Weight=1.0
}
