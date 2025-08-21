class avaSeqAct_GameChat extends SequenceAction;

var() string ChatMessage;

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

event Activated()
{
	Local WorldInfo WorldInfo;
	Local PlayerController PC;
	Local avaPlayerController avaPC;

	WorldInfo = GetWorldInfo();

	foreach WorldInfo.LocalPlayerControllers(PC)
	{
		avaPC = avaPlayerController( PC );

		if( InputLinks[0].bHasImpulse )
		{
			avaPC.TeamSay( ChatMessage );
		}
		if( InputLinks[1].bHasImpulse )
		{
			avaPC.Say( ChatMessage );
		}
	}
}



defaultproperties
{
	ObjName="Game Chat"
	ObjCategory="HUD"

	InputLinks(0)=(LinkDesc="Team")
	InputLinks(1)=(LinkDesc="All")

	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String', LinkDesc="", PropertyName=ChatMessage))
}