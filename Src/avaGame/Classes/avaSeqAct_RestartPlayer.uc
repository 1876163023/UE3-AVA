class avaSeqAct_RestartPlayer extends SequenceAction;

event Activated()
{
	Local PlayerController PC;
	Local WorldInfo WorldInfo;

	WorldInfo = GetWorldInfo();
	foreach WorldInfo.LocalPlayerControllers(PC)
	{
		if( avaPlayerController(PC) != none )
			avaPlayerController(PC).RequestRestartPlayer();
	}
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
	ObjName="(InGame) Restart Player"
	ObjCategory="avaGame"

	VariableLinks.Empty

	ObjClassVersion=1
}