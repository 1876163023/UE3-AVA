/*
	Get Modifier from ItemID(Weapon/Equip).

	2007/02/05	고광록
*/
class avaUIAction_GetModifierFromItemID extends UIAction
	dependson(avaNetHandler);

//! 아이템 Modifier의 종류.
enum EItemModifierType
{
	IMT_CHARACTER,
	IMT_WEAPON,
};

var() EItemModifierType	ModifierType;	//!< 사용될 Modifier의 종류.

var() string			StrItemID;		//!< ItemID의 문자열 값.

var Object				Modifier;		//!< 얻어올 Modifier Object.

//! 처리 함수.
event Activated()
{
	local int ItemID;

	ItemID = int(StrItemID);

	if ( ModifierType == IMT_CHARACTER )
		Modifier = class'avaNetHandler'.static.GetAvaNetHandler().GetCharacterModifier(ItemID);
	else if ( ModifierType == IMT_WEAPON )
		Modifier = class'avaNetHandler'.static.GetAvaNetHandler().GetWeaponModifier(ItemID);

//	테스트용.
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