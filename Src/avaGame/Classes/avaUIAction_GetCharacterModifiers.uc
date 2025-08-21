/*
	ItemDesc���� ĳ������ Slot�� �ش��ϴ� Modifier�� ������ �׼�.

	2007/01/29	����
*/
class avaUIAction_GetCharacterModifiers extends UIAction
	dependson(avaNetHandler);

var() ECharSlot			Slot;			//!< Slot��.
var() array<Object>		ObjList;		//!< Modifier����Ʈ.


event Activated()
{
	local int						i, j;
	local array<int>				Items;
	local SeqVar_ObjectList			VarObjList;
	local string					classname;

	if ( GetOwnerScene() == None )
		return;

	class'avaNetHandler'.static.GetAvaNetHandler().GetAvailableEquipsBySlot(Slot, Items);

	foreach LinkedVariables(class'SeqVar_ObjectList',VarObjList,"ObjectList")
	{
		VarObjList.ObjList.Length = 0;
		for ( j = 0; j < Items.Length; ++ j )
		{
			for ( i = 0; i < class'avaNetHandler'.static.GetAvaNetHandler().CharacterModifierList.Length; ++ i )
			{
				// ���� id�� �ִٸ�
				if ( Items[j] == class'avaNetHandler'.static.GetAvaNetHandler().CharacterModifierList[i].default.ID )
				{
					VarObjList.ObjList[VarObjList.ObjList.Length] = class'avaNetHandler'.static.GetAvaNetHandler().CharacterModifierList[i];
					break;
				}
			}
		}

		break;
	}

	classname = ""$class'avaNetHandler'.static.GetAvaNetHandler().CharacterModifierList[0];

	`log("### "@classname);
	`log("### avaUIAction_GetWeaponModifiers - ObjList(Items/Modifiers) :" @ObjList.Length @"(" 
		 @Items.Length@"/"@class'avaNetHandler'.static.GetAvaNetHandler().CharacterModifierList.Length @") ###");

}

defaultproperties
{
	ObjName="ava GetCharacterModifiers"
	ObjCategory="Object List"
	bCallHandler=false

	Slot = CHAR_SLOT_NONE

	VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Slot",PropertyName=Index,MaxVars=1)
	VariableLinks(1)=(ExpectedType=class'SeqVar_ObjectList',LinkDesc="ObjectList",PropertyName=ObjList,MaxVars=1,bWriteable=true)
}