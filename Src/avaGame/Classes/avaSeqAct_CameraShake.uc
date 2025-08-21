/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaSeqAct_CameraShake extends SequenceAction;

var() avaPlayerController.ViewShakeInfo CameraShake;

defaultproperties
{
	ObjCategory="Camera"
	ObjName="Shake"

	CameraShake=(RotMag=(Z=250),RotRate=(Z=2500),RotTime=6,OffsetMag=(Z=15),OffsetRate=(Z=200),OffsetTime=10)
}
