/*
	avaUICharacter의 캐릭터의 여러가지를 변경하기 위한 액션.

	2007/01/30	고광록
*/
class avaSeqAct_UICharacterEventEx extends avaSeqAct_UICharacterEvent;

/*! @brief 이벤트에 해당하는 생성용 오브젝트.
	@note
		UICE_ChangeCharacter ~ UICE_ChangeWeaponMod까지의 4가지 이벤트에서 사용되며
		StrValue와 이것 두개중 한개만 택해서 사용하면 된다.
*/
var Object				ObjValue;

defaultproperties
{
	bCallHandler = true
	ObjCategory="Actor"
	ObjName="ava UICharacterEventEx"
	
//	VariableLinks(0)=(ExpectedType=class'SeqVar_Object', LinkDesc="Actor", PropertyName=Actor, MaxVars=1)

	// 0번은 Actor로 예약되어 있다.
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object', LinkDesc="Object", PropertyName=ObjValue, MaxVars=1)

	Event = UICE_None
}