/*
	avaUICharacter�� ĳ������ ���������� �����ϱ� ���� �׼�.

	2007/01/30	����
*/
class avaSeqAct_UICharacterEventEx extends avaSeqAct_UICharacterEvent;

/*! @brief �̺�Ʈ�� �ش��ϴ� ������ ������Ʈ.
	@note
		UICE_ChangeCharacter ~ UICE_ChangeWeaponMod������ 4���� �̺�Ʈ���� ���Ǹ�
		StrValue�� �̰� �ΰ��� �Ѱ��� ���ؼ� ����ϸ� �ȴ�.
*/
var Object				ObjValue;

defaultproperties
{
	bCallHandler = true
	ObjCategory="Actor"
	ObjName="ava UICharacterEventEx"
	
//	VariableLinks(0)=(ExpectedType=class'SeqVar_Object', LinkDesc="Actor", PropertyName=Actor, MaxVars=1)

	// 0���� Actor�� ����Ǿ� �ִ�.
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object', LinkDesc="Object", PropertyName=ObjValue, MaxVars=1)

	Event = UICE_None
}