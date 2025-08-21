/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaDustVolume extends Volume
	native;

var() float DustIntensity;

cpptext
{
	UBOOL ShouldTrace(UPrimitiveComponent* Primitive,AActor *SourceActor, DWORD TraceFlags);
}

defaultproperties
{
	DustIntensity = 1.0
	bColored=true
	BrushColor=(R=255,G=255,B=100,A=255)

	bCollideActors=true
	bProjTarget=false	
}
