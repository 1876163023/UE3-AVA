class UIEvent_AmmoGraph extends UIEvent native;

/**
 * Determines whether this class should be displayed in the list of available ops in the UI's kismet editor.
 *
 * @param	TargetObject	the widget that this SequenceObject would be attached to.
 *
 * @return	TRUE if this sequence object should be available for use in the UI kismet editor
 */
event bool IsValidUISequenceObject( optional UIScreenObject TargetObject )
{
	return TargetObject == None || avaUIAmmoGraph(TargetObject) != None;
}

/// �̰� ������ Sequence��ü�� ��������� �ʴ´�!! �������.. 3�ð� ���� -_-;
event bool ShouldAlwaysInstance()
{
	return true;
}

DefaultProperties
{
	ObjName="AmmoGraph Reload"
	OutputLinks(0)=(LinkDesc="Stable")
	OutputLinks(1)=(LinkDesc="Reloading")

	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Value",bWriteable=true))
}
