/*
	카운트다운 중인가? (=시작버튼이 눌렸는가?)

	2007/03/02	고광록
*/
class avaUICond_IsCountingDown extends SequenceCondition;

var bool	bCountingDown;

event Activated()
{
	if ( class'avaNetHandler'.static.GetAvaNetHandler().IsCountingDown() )
	{
		OutputLinks[0].bHasImpulse = true;
		bCountingDown = true;
	}
	else
	{
		OutputLinks[1].bHasImpulse = true;
		bCountingDown = false;
	}
}

/**
 * Determines whether this class should be displayed in the list of available ops in the level kismet editor.
 *
 * @return	TRUE if this sequence object should be available for use in the level kismet editor
 */
event bool IsValidLevelSequenceObject()
{
	return false;
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

DefaultProperties
{
	ObjCategory="avaNet"
	ObjName="(Room) Is Counting Down"

	OutputLinks(0)=(LinkDesc="True")
	OutputLinks(1)=(LinkDesc="False")

	VariableLinks.Empty
	VariableLinks.Add( (ExpectedType=class'SeqVar_Bool',LinkDesc="Bool",PropertyName=bCountingDown, MinVars=1, MaxVars=1, bWriteable=true ) )
}