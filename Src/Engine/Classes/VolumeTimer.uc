/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class VolumeTimer extends info;

var PhysicsVolume V;

event PostBeginPlay()
{
	Super.PostBeginPlay();
	SetTimer(1.0, true);
	V = PhysicsVolume(Owner);
}

event Timer()
{
	V.TimerPop(self);
}

defaultproperties
{
	TickGroup=TG_PreAsyncWork

	bStatic=false
	RemoteRole=ROLE_None
}
