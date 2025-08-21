/** ScreenObject가 보이지 않게 되었을 때 불리는 UIEvent
 *
 * @note: native because C++ code activates this event
 */
class UIEvent_Hidden extends UIEvent
	native(inherit);

DefaultProperties
{
	ObjName="Hidden"

	ObjPosX=50
	ObjPosY=130
}
