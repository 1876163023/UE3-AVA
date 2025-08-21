/**
 *	������ �׼ǵ��� �����Ҷ� ���� ���̺� Ÿ��
 */
class avaSeqAct_CableTie extends SequenceAction;

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
	ObjName="Cable Tie"
	ObjCategory="Misc"

	VariableLinks.Empty
}