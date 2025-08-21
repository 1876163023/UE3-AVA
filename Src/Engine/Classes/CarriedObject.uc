/**
 * Obsolete/deprecated - will be removed
 * Copyright 2004-2006 Epic Games, Inc. All Rights Reserved.
 */
class CarriedObject extends Actor
    abstract notplaceable deprecated;

var const NavigationPoint LastAnchor;		// recent nearest path
var		float	LastValidAnchorTime;	// last time a valid anchor was found

event NotReachableBy(Pawn P);

defaultproperties
{
}
