/**
 * Activated when a certain amount of damage is taken.  Allows the designer to define how much and
 * which types of damage should be be required (or ignored).
 * Originator: the actor that was damaged
 * Instigator: the actor that did the damaging
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class SeqEvent_TakeDamage extends SequenceEvent;

struct DamageInfo
{
	var()	class<DamageType>	damageType;
	var()	float				damageAmplifier;
	structdefaultproperties
	{
		damageAmplifier = 1.0;
	}
};

/** Damage must exceed this value to be counted */
var() float MinDamageAmount;

/** Total amount of damage to take before activating the event */
var() float DamageThreshold;

/** Types of damage that are counted */
var()	array<DamageInfo>		DamageTypes;
//var() array<class<DamageType> > DamageTypes;

/** Types of damage that are ignored */
var() array<class<DamageType> > IgnoreDamageTypes;


var() bool						ResetDamageAfterActivate;



/** Current damage amount */
var float CurrentDamage;

/**
 * Searches DamageTypes[] for the specified damage type.
 */
final function bool IsValidDamageType(class<DamageType> inDamageType, out int validDamageTypeIdx )
{
	local bool bValid;
	Local bool bFound;
	Local int i;

	bValid = true;
	if (DamageTypes.Length > 0)
	{
		bFound = false;
		for( i = 0 ; i < DamageTypes.Length ; i++ )
		{
			// @TODO : ClassType.operator==(ClassType) Doesn't work
			// @TODO : Class.IsA(ClassTypeName) Doesn't work
			if( inDamageType.Name == DamageTypes[i].damageType.Name )
			{
				validDamageTypeIdx	= i;
				bFound				= true;
				break;
			}
		}
		bValid = bFound;
	// @TODO : it doesn't work. ClassType Comparison
//		bValid = (DamageTypes.Find(inDamageType) == -1);
	}
	if (bValid &&
		IgnoreDamageTypes.Length > 0)
	{
		bFound = false;
		for( i = 0 ; i < IgnoreDamageTypes.Length ; i++ )
		{
			// @TODO : ClassType.operator==(ClassType)  Doesn't work
			// @TODO : Class.IsA(ClassTypeName)  Doesn't work
			if(InDamageType.Name == IgnoreDamageTypes[i].Name)
			{
				bFound = true;
				break;
			}
		}

		bValid = !bFound;
		// make sure it's not an ignored type
		// @TODO : it doesn't work. ClassType Comparison
		// bValid = (IgnoreDamageTypes.Find(inDamageType) == -1);
	}
	return bValid;
}

/**
 * Applies the damage and checks for activation of the event.
 */
final function HandleDamage(Actor inOriginator, Actor inInstigator, class<DamageType> inDamageType, int inAmount, optional vector HitLocation)
{
	local SeqVar_Float	FloatVar;
	local bool			bAlreadyActivatedThisTick;
	local int			validDamageTypeIdx;

	validDamageTypeIdx = -1;
	if (inOriginator != None &&
		bEnabled &&
		inAmount >= MinDamageAmount &&
		IsValidDamageType(inDamageType,validDamageTypeIdx) &&
		(!bPlayerOnly ||
		 (inInstigator!= None && inInstigator.IsPlayerOwned())))
	{
		if ( validDamageTypeIdx != -1 )
			CurrentDamage += (inAmount * DamageTypes[validDamageTypeIdx].damageAmplifier);
		else
			CurrentDamage += inAmount;

		if (CurrentDamage >= DamageThreshold)
		{
			bAlreadyActivatedThisTick = (bActive && ActivationTime ~= GetWorldInfo().TimeSeconds);
			if (CheckActivate(inOriginator,inInstigator,false))
			{
				// write to any variables that want to know the exact damage taken
				foreach LinkedVariables(class'SeqVar_Float', FloatVar, "Damage Taken")
				{
					//@hack carry over damage from multiple hits in the same tick
					//since Kismet doesn't currently support multiple activations in the same tick
					if (bAlreadyActivatedThisTick)
					{
						FloatVar.FloatValue += CurrentDamage;
					}
					else
					{
						FloatVar.FloatValue = CurrentDamage;
					}
				}
				// reset the damage counter on activation
				if ( ResetDamageAfterActivate == true )
				{
					CurrentDamage	= 0.f;
				}
				else
				{
					if (DamageThreshold <= 0.f)
					{
						CurrentDamage = 0.f;
					}
					else
					{
						CurrentDamage -= DamageThreshold;
					}
				}
				SetHitParameters(inOriginator, inInstigator, HitLocation);
			}
		}
	}
}


// [2006/11/03 otterrrr]  Set Location and Rotation of the Hit Event

function SetHitParameters(Actor inOriginator, Actor inInstigator, vector HitLocation)
{
	Local SeqVar_Vector VectorVar;
	Local vector HitDirection, HitOrigin;
	Local PlayerController PC;

	Assert(inOriginator != None && inInstigator != None);
	
	// HitLocation is Invalid
	if( VSize(HitLocation) == 0 )
		HitLocation = inOriginator.Location;

	PC = PlayerController(inInstigator);

	HitOrigin = (PC != None && PC.Pawn != None) ? PC.Pawn.Location : inInstigator.Location;
	HitDirection = HitOrigin - HitLocation;

	foreach LinkedVariables(class'SeqVar_Vector', VectorVar, "HitLocation")
		VectorVar.VectValue = HitLocation;
	
	foreach LinkedVariables(class'SeqVar_Vector', VectorVar, "HitDirection")
		VectorVar.VectValue = HitDirection;
}


function Reset()
{
	Super.Reset();

	CurrentDamage = 0.f;
}

defaultproperties
{
	ObjName="Take Damage"
	ObjCategory="Actor"
	ObjClassVersion=2

	DamageThreshold=100.f
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Damage Taken",bWriteable=true))

	//[2006.11.03 otterrrr] add variablelinks for indication a HitLocation
	VariableLinks.Add((ExpectedType=class'SeqVar_Vector',LinkDesc="HitLocation",bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Vector',LinkDesc="HitDirection",bWriteable=true))

}
