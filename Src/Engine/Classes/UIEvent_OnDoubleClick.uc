/**
 * This event is activated when an object that responds to mouse double-clicks is clicked on.
 *
 * 2006/12/14 윤태식
 * 
 * UIList의 DoubleClickEvent가 필요.
 *
 * @note: native because C++ code activates this event
 */
class UIEvent_OnDoubleClick extends UIEvent
	native(inherit);

DefaultProperties
{
	ObjName="On DblClick"

	ObjPosX=50
	ObjPosY=120
}
