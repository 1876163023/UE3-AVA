
/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class AnimNodeScaleRateBySpeed extends AnimNodeScalePlayRate
	native(Anim)
	hidecategories(Object);

var() float	BaseSpeed;

cpptext
{
	virtual FLOAT	GetScaleValue();
}
	
defaultproperties
{
	BaseSpeed=350
}
