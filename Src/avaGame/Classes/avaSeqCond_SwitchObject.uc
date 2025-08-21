/*
	Switch Object

	2007/11/30	����
		avaSeqCond_SwitchString���� ������.
*/
class avaSeqCond_SwitchObject extends SequenceCondition
	native;

cpptext
{
	virtual void	Activated();
	virtual void	UpdateDynamicLinks();
	virtual FColor	GetConnectionColor( INT ConnType, INT ConnIndex, INT MouseOverConnType, INT MouseOverConnIndex );
}

var() array<Object>		CompareList;	//! ������Ʈ ����Ʈ.
var() Object			Compare;		//! ���� ������Ʈ

/**
 * Determines whether this class should be displayed in the list of available ops in the UI's kismet editor.
 *
 * @param	TargetObject	the widget that this SequenceObject would be attached to.
 *
 * @return	TRUE if this sequence object should be available for use in the UI kismet editor
 */
event bool IsValidUISequenceObject( optional UIScreenObject TargetObject )
{
	return true;
}

defaultproperties
{
	ObjName="Switch Object"

	InputLinks(0)=(LinkDesc="In")
	OutputLinks(0)=(LinkDesc="None")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Compare",PropertyName=Compare)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Int',LinkDesc="SelectedIndex",bWriteable=true,bHidden=true)
}