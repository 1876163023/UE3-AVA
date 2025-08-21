/*
	레디버튼이 눌렸는가?

	2007/03/02	고광록
*/
class avaUICond_IsReady extends SequenceCondition;

var bool	bReady;

event Activated()
{
	if ( class'avaNetHandler'.static.GetAvaNetHandler().AmIReady() )
	{
		OutputLinks[0].bHasImpulse = true;
	}
	else
	{
		OutputLinks[1].bHasImpulse = true;
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
	ObjName="(Room) Am I Ready"

	OutputLinks(0)=(LinkDesc="True")
	OutputLinks(1)=(LinkDesc="False")

	VariableLinks.Empty
	VariableLinks.Add( (ExpectedType=class'SeqVar_Bool',LinkDesc="Bool",PropertyName=bReady, MinVars=1, MaxVars=1, bWriteable=true ) )
}