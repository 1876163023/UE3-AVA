class avaSeqAct_ChangeWeapon extends SequenceAction;

var() class< avaWeapon >			WeaponClass;

event Activated()
{
	if ( InputLinks[0].bHasImpulse )
	{
		NextWeapon();		
	}
	else if ( InputLinks[1].bHasImpulse )
	{
		PrevWeapon();
	}
	else if ( InputLinks[2].bHasImpulse )
	{
		SpecificWeapon();
	}
}

simulated function NextWeapon()
{
	local avaCharacter	P;
	local SeqVar_Object	ObjVar;
	ForEach LinkedVariables( class'SeqVar_Object', ObjVar, "Target" )
	{
		P = avaCharacter(ObjVar.GetObjectValue());
		if ( P == None )	continue;
		P.InvManager.NextWeapon();
	}
}

simulated function PrevWeapon()
{
	local avaCharacter	P;
	local SeqVar_Object	ObjVar;
	ForEach LinkedVariables( class'SeqVar_Object', ObjVar, "Target" )
	{
		P = avaCharacter(ObjVar.GetObjectValue());
		if ( P == None )	continue;
		P.InvManager.PrevWeapon();
	}
}

simulated function SpecificWeapon()
{
	local avaCharacter	P;
	local SeqVar_Object	ObjVar;
	local Inventory		ExistenceInv;
	ForEach LinkedVariables( class'SeqVar_Object', ObjVar, "Target" )
	{
		P = avaCharacter(ObjVar.GetObjectValue());
		if ( P == None )	continue;
		ExistenceInv = P.FindInventoryType( WeaponClass );
		if ( ExistenceInv != None )
		{
			P.InvManager.SetCurrentWeapon( Weapon( ExistenceInv ) );
		}
	}
}

defaultproperties
{
	ObjCategory="avaPawn"
	ObjName="Change Weapon"
	bCallHandler=false

	InputLinks(0)=(LinkDesc="Next Weapon")
	InputLinks(1)=(LinkDesc="Prev Weapon")
	InputLinks(2)=(LinkDesc="Specific Weapon")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',	LinkDesc="Target",		PropertyName=Targets)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',	LinkDesc="Weapon",		PropertyName=WeaponClass )
} 
