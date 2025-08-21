/**
 * This action instructs the owning widget to refresh and reapply the value of a data store binding.
 * UIAction_RefreshBindingValue와 다른점은 갱신시에 ListIndex를 보존해준다는 점이다.
 *
 * Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved
 */
class avaUIAction_RefreshBindingValue extends UIAction_RefreshBindingValue
	native;

enum ERefreshBindingValue_Option
{
	ERBV_None,
	ERBV_List_PreserveIndex,
	ERBV_List_SelectNewItem,
};

var(List) ERefreshBindingValue_Option	Option;
var(List) Name							IDCellTag;
var(List) bool							bSkipNotification;	//!< UIEvent 등이 발생하지 않도록 막는다.

cpptext
{
	/**
	 * If the owning widget implements the UIDataStoreSubscriber interface, calls the appropriate method for refreshing the
	 * owning widget's value from the data store.
	 */
	virtual void Activated();
}

/**
 * Determines whether this class should be displayed in the list of available ops in the UI's kismet editor.
 *
 * @param	TargetObject	the widget that this SequenceObject would be attached to.
 *
 * @return	TRUE if this sequence object should be available for use in the UI kismet editor
 */
event bool IsValidUISequenceObject( optional UIScreenObject TargetObject )
{
	if ( !Super.IsValidUISequenceObject(TargetObject) )
	{
		return false;
	}

	if ( TargetObject != None )
	{
		// this op can only be used by widgets that implement the UIDataStoreSubscriber interface
		return UIDataStoreSubscriber(TargetObject) != None;
	}

	return true;
}

DefaultProperties
{
	ObjName="(AVA)Refresh Value"
	Option=ERBV_List_PreserveIndex
	ObjClassVersion = 2
}
