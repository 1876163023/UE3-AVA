/** 인벤토리 메세지 (Set Weapon, SetEquip 등등)에 대한 응답이벤트
 *
 *	어떤응답(타임아웃, 실패, 성공,...)이 들어오더라도 불린다는 점이 일반적인 이벤트와 다른점이다
 **/

class avaUIEvent_ResponseInventory extends UIEvent;

defaultproperties
{
	ObjName="Response Inventory"
	ObjCategory="avaNet"
}