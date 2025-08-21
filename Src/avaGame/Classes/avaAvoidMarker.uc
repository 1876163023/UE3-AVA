/**
 * Copyright 1998-2007 Epic Games, Inc. All Rights Reserved.
 */
//=============================================================================
// avaAvoidMarker.
// Bots avoid these spots when moving - used for very short term stationary hazards like bio goo or sticky grenades
//=============================================================================
class avaAvoidMarker extends Actor
	native
	notPlaceable;

var byte TeamNum;
var CylinderComponent CollisionCylinder;

simulated function byte GetTeamNum()
{
	return TeamNum;
}

event Touch( actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal )
{
	if ( (Pawn(Other) != None) && (avaBot(Pawn(Other).Controller) != None) && !WorldInfo.GRI.OnSameTeam(self,Other) )
		avaBot(Pawn(Other).Controller).FearThisSpot(self);
}

function StartleBots()
{
	local Pawn P;

	ForEach TouchingActors(class'Pawn', P)
		if ( (avaBot(P.Controller) != None) && !WorldInfo.GRI.OnSameTeam(self,P.Controller) )
			avaBot(P.Controller).Startle(self);
}

defaultproperties
{
	Begin Object Class=SpriteComponent Name=Sprite
		Sprite=Texture2D'EngineResources.S_Actor'
		HiddenGame=True
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
	End Object
	Components.Add(Sprite)

	TeamNum=255

	Begin Object Class=CylinderComponent Name=Cylinder
		CollisionRadius=+0100.000000
		CollisionHeight=+0040.000000
		CollideActors=true
	End Object
	Components.Add(Cylinder)
	CollisionComponent=Cylinder
	CollisionCylinder=Cylinder

	bCollideActors=true
}
