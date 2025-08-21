/*=============================================================================
  avaSeqAct_AddWeapon
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2006/08/28 by OZ

		Character 에 Modifier 가 적용된 Weapon 을 추가해준다.
		1회성이며 다음번 Spawn 시에는 적용되지 않는다.
		Pawn 이 이미 Weapon 을 가지고 있다면 bRemovePreExistenceWeapon 이 True 라면 기존의 Weapon 을 지우고 만들며,
		False 이면 Weapon 을 새로 만들지 않는다.

=============================================================================*/
class avaSeqAct_AddWeapon extends SequenceAction;

var() class< avaWeapon >			WeaponClass;
var() array< class<avaMod_Weapon> >	WeaponModifiers;
var() bool							bRemovePreExistenceWeapon;

event Activated()
{
	AddWeapon();
}

simulated function AddWeapon()
{
	local avaCharacter	P;
	local SeqVar_Object	ObjVar;
	local int			index;
	local Inventory		Inv;
	local Inventory		ExistenceInv;

	ForEach LinkedVariables( class'SeqVar_Object', ObjVar, "Target" )
	{
		P = avaCharacter(ObjVar.GetObjectValue());
		if ( P == None )			continue;
		if ( WeaponClass == None )	return;
		ExistenceInv = P.FindInventoryType( WeaponClass );

		if ( ExistenceInv != None )
		{
			if ( bRemovePreExistenceWeapon == true )
			{
				P.InvManager.RemoveFromInventory( ExistenceInv );
				`log( "avaSeqAct_AddWeapon.RemoveFromInventory" @ExistenceInv );
			}
			else
			{
				continue;
			}
		}
		
		Inv = P.Spawn( WeaponClass );
		if ( Inv == None )			return;
		avaWeapon(Inv).AmmoCount = avaWeapon(Inv).MaxAmmoCount;
		for ( index = 0 ; index < WeaponModifiers.length ; index ++ )
		{
			avaWeapon(Inv).AddWeaponModifier( WeaponModifiers[index] );
		}
		avaWeapon(Inv).WeaponModifierDone();
		P.InvManager.AddInventory( Inv, false );
	}
}

defaultproperties
{
	ObjCategory="avaPawn"
	ObjName="Add Weapon"
	bCallHandler=false
	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',	LinkDesc="Target",		PropertyName=Targets)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',	LinkDesc="Weapon",		PropertyName=WeaponClass )
	VariableLinks(2)=(ExpectedType=class'SeqVar_ObjectList',LinkDesc="Modifiers",	PropertyName=WeaponModifiers )
	InputLinks(0)=(LinkDesc="AddWeapon")
} 