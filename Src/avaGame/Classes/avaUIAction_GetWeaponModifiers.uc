/*
	ItemDesc에서 무기의 Slot에 해당하는 Modifier를 얻어오는 액션.

	2007/01/30	고광록
*/
class avaUIAction_GetWeaponModifiers extends UIAction
	dependson(avaNetHandler);

var() EWeaponClassSlot	Slot;			//!< Slot값.
var array<Object>		ObjList;		//!< Modifier리스트.


event Activated()
{
	local int						i, j;
	local array<int>				Items;
	local SeqVar_ObjectList			VarObjList;

	if ( GetOwnerScene() == None )
		return;

	class'avaNetHandler'.static.GetAvaNetHandler().GetAvailableWeaponsBySlot(Slot, Items);

	foreach LinkedVariables(class'SeqVar_ObjectList',VarObjList,"ObjectList")
	{
		VarObjList.ObjList.Length = 0;
		for ( j = 0; j < Items.Length; ++ j )
		{
			for ( i = 0; i < class'avaNetHandler'.static.GetAvaNetHandler().WeaponModifierList.Length; ++ i )
			{
				// 같은 id가 있다면
				if ( Items[j] == class'avaNetHandler'.static.GetAvaNetHandler().WeaponModifierList[i].default.ID )
				{
					VarObjList.ObjList[VarObjList.ObjList.Length] = class'avaNetHandler'.static.GetAvaNetHandler().WeaponModifierList[i];
					break;
				}
			}
		}

		break;
	}

	`log("### avaUIAction_GetWeaponModifiers - ObjList(Items/Modifiers) :" @ObjList.Length @"(" 
		 @Items.Length@"/"@class'avaNetHandler'.static.GetAvaNetHandler().WeaponModifierList.Length @") ###");
}

defaultproperties
{
	ObjName="ava GetWeaponModifiers"
	ObjCategory="Object List"
	bCallHandler=false

	Slot = WEAPON_CLASS_SLOT_NONE

	VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Slot",PropertyName=Index,MaxVars=1)
	VariableLinks(1)=(ExpectedType=class'SeqVar_ObjectList',LinkDesc="ObjectList",PropertyName=ObjList,MaxVars=1,bWriteable=true)
}