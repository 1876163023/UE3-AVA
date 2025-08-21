class avaSeqCond_BoolTable extends SequenceCondition
	native;

/** �Ҹ������̺� �������� */
var() int BoolTableVarCount;

cpptext
{
	virtual void PostEditChange(UProperty* PropertyThatChanged);

	virtual void Activated();

	virtual void UpdateObject();
	void DeActivated();

protected:
	void UpdateAllDynamicLinks();
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
	return true;
}

defaultproperties
{
	ObjName="Bool Table"

	BoolTableVarCount=0
	VariableLinks.Empty
	OutputLinks.Empty
}