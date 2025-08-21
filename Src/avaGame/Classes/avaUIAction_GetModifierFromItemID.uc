/*
	Get Modifier from ItemID(Weapon/Equip).

	2007/02/05	����
*/
class avaUIAction_GetModifierFromItemID extends UIAction
	dependson(avaNetHandler);

//! ������ Modifier�� ����.
enum EItemModifierType
{
	IMT_CHARACTER,
	IMT_WEAPON,
};

var() EItemModifierType	ModifierType;	//!< ���� Modifier�� ����.

var() string			StrItemID;		//!< ItemID�� ���ڿ� ��.

var Object				Modifier;		//!< ���� Modifier Object.

//! ó�� �Լ�.
event Activated()
{
	local int ItemID;

	ItemID = int(StrItemID);

	if ( ModifierType == IMT_CHARACTER )
		Modifier = class'avaNetHandler'.static.GetAvaNetHandler().GetCharacterModifier(ItemID);
	else if ( ModifierType == IMT_WEAPON )
		Modifier = class'avaNetHandler'.static.GetAvaNetHandler().GetWeaponModifier(ItemID);

//	�׽�Ʈ��.
	`log("StrItemID" @StrItemID @"Modifier = "@Modifier);
}

defaultproperties
{
	ObjName="ava Get Modifier from ItemID"
	ObjCategory="Object List"
	bCallHandler=false

	ModifierType = IMT_CHARACTER

	VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_String',LinkDesc="ItemID",PropertyName=StrItemID,MaxVars=1)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Modifier",PropertyName=Modifier,MaxVars=1)
}