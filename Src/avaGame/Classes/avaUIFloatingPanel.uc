/**
 * @ingroup _AVAGame
 *
 * 팝업 패널같이 ActiveState를 가지지 못하는 UIObject를 위해 사용
 * 팝업에 ActiveState가 있으면 마우스를 팝업 위에 놓으면 
 * 팝업의 밑에 깔린 리스트가 번갈아가며 엇갈리게 ActiveState를 갖게되 의도하지 않은 동작을 한다
 *
 * 따라서 FloatingPanel의 자식들은 자식으로 등록되는 순간 ActiveState를 빼앗긴다.
 *
 * @date 2007-11-13
 *
 * @author otterrrr
 *
 * @todo 
 *
 */
class avaUIFloatingPanel extends UIPanel
	native(UIPrivate);

/**
 * Controls the wrapping behavior of the list, or what happens when the use attempts to scroll past the last element
 */
var const editconst class<UIState>		StateClassToDetach;

cpptext
{
	/* == UIScreenObject Interfaces == */
	/**
	 * Iterates through the DefaultStates array checking that InactiveStates contains at least one instance of each
	 * DefaultState.  If no instances are found, one is created and added to the InactiveStates array.
	 */
	virtual void CreateDefaultStates();

	/* == UIObject Interfaces == */
	/**
	 * Called immediately after a child has been added to this screen object.
	 *
	 * @param	WidgetOwner		the screen object that the NewChild was added as a child for
	 * @param	NewChild		the widget that was added
	 */
	virtual void NotifyAddedChild( UUIScreenObject* WidgetOwner, UUIObject* NewChild );

	/**
	 * Called immediately after a child has been removed from this screen object.
	 *
	 * @param	WidgetOwner		the screen object that the widget was removed from.
	 * @param	OldChild		the widget that was removed
	 * @param	ExclusionSet	used to indicate that multiple widgets are being removed in one batch; useful for preventing references
	 *							between the widgets being removed from being severed.
	 */
	virtual void NotifyRemovedChild( UUIScreenObject* WidgetOwner, UUIObject* OldChild, TArray<UUIObject*>* ExclusionSet=NULL );
}

defaultproperties
{
	StateClassToDetach = class'UIState_Active'
}