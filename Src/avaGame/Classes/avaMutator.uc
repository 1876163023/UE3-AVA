/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaMutator extends Mutator
	abstract;


/* ReplaceWith()
Call this function to replace an actor Other with an actor of aClass.
*/
function bool ReplaceWith(actor Other, string aClassName)
{
	local Actor A;
	local class<Actor> aClass;

	if ( aClassName == "" )
		return true;

	aClass = class<Actor>(DynamicLoadObject(aClassName, class'Class'));
	if ( aClass != None )
		A = Spawn(aClass,Other.Owner,,Other.Location, Other.Rotation);
	return ( A != None );
}
