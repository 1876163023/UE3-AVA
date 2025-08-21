/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
/** spawns a projectile at a certain location that moves toward the given location with the given Instigator */
class avaSeqAct_SpawnProjectile extends SequenceAction;

/** Class of projectile to spawn */
var() class<Projectile> ProjectileClass;

//var() Actor SpawnPoint, TargetPoint;
//var() vector SpawnLoc, TargetLoc;

event Activated()
{
	local Controller InstigatorController;
	local Projectile Proj;
	local Actor SpawnPoint, TargetPoint;
	local vector SpawnLoc, TargetLoc;
	local SeqVar_Object O;
	local SeqVar_Vector V;

	foreach LinkedVariables(class'SeqVar_Object', O, "Instigator")
	{
		InstigatorController = Controller(O.GetObjectValue());
		if (InstigatorController == None && O != None)
		{
			ScriptLog("ERROR: Instigator specified for" @ self @ "is not a Controller");
		}
	}

	foreach LinkedVariables(class'SeqVar_Object', O, "Spawn Point")
	{
		SpawnPoint = Actor(O.GetObjectValue());
	}

	foreach LinkedVariables(class'SeqVar_Object', O, "Target Point")
	{
		TargetPoint = Actor(O.GetObjectValue());
	}

	if (SpawnPoint != None)
	{
		SpawnLoc = SpawnPoint.Location;
	}
	else
	{
		foreach LinkedVariables(class'SeqVar_Vector', V, "Spawn Location")
		{
			SpawnLoc = V.VectValue;
		}
	}

	if (TargetPoint != None)
	{
		TargetLoc = TargetPoint.Location;
	}
	else
	{
		foreach LinkedVariables(class'SeqVar_Vector', V, "Target Location")
		{
			TargetLoc = V.VectValue;
		}
	}
/*
	// get the instigator
	if (VariableLinks[4].LinkedVariables.length > 0)
	{
		InstigatorController = Controller(SeqVar_Object(VariableLinks[4].LinkedVariables[0]).GetObjectValue());
		if (InstigatorController == None && SeqVar_Object(VariableLinks[4].LinkedVariables[0]).GetObjectValue() != None)
		{
			ScriptLog("ERROR: Instigator specified for" @ self @ "is not a Controller");
		}
	}

	// get the spawn location
	if (SpawnPoint != None)
	{
		SpawnLoc = SpawnPoint.Location;
	}
	/*else
	{
		if (VariableLinks[0].LinkedVariables.Length > 0)
		{
			SpawnLoc = SeqVar_Vector(VariableLinks[0].LinkedVariables[0]).VectValue;
		}
		else
		{
			ScriptLog("ERROR: Spawn location is not valid.");
			return;
		}
	}*/

	// get the target location
	if (TargetPoint != None)
	{
		TargetLoc = TargetPoint.Location;
	}
	/*else
	{
		if (VariableLinks[2].LinkedVariables.Length > 0)
		{
			TargetLoc = SeqVar_Vector(VariableLinks[2].LinkedVariables[0]).VectValue;
		}
		else
		{
			ScriptLog("ERROR: Target location is not valid.");
			return;
		}
	}*/
*/

	//`log( self @ ": SpawnLoc =" @ SpawnLoc @ "TargetLoc =" @ TargetLoc);

	// spawn a projectile at the requested location and point it at the requested target
	Proj = GetWorldInfo().Spawn(ProjectileClass,,, SpawnLoc);
	if (InstigatorController != None)
	{
		Proj.Instigator = InstigatorController.Pawn;
		Proj.InstigatorController = InstigatorController;
	}
	Proj.Init(Normal(TargetLoc - SpawnLoc));
}


defaultproperties
{
	bCallHandler=false
	ObjName="Spawn Projectile"
	VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Vector',LinkDesc="Spawn Location",PropertyName=SpawnLoc,MinVars=0,MaxVars=1)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Spawn Point",PropertyName=SpawnPoint,MinVars=0,MaxVars=1)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Vector',LinkDesc="Target Location",PropertyName=TargetLoc,MinVars=0,MaxVars=1)
	VariableLinks(3)=(ExpectedType=class'SeqVar_Object',LinkDesc="Target Point",PropertyName=TargetPoint,MinVars=0,MaxVars=1)
	VariableLinks(4)=(ExpectedType=class'SeqVar_Object',LinkDesc="Instigator",MinVars=0,MaxVars=1)

	ObjClassVersion=3
}
