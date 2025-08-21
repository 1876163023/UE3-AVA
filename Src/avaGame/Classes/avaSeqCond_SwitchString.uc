class avaSeqCond_SwitchString extends SequenceCondition
	native;

cpptext
{
	virtual void Activated();
	virtual void	UpdateDynamicLinks();
	virtual FColor	GetConnectionColor( INT ConnType, INT ConnIndex, INT MouseOverConnType, INT MouseOverConnIndex );
}

var() array<string>		CompareList;
var() string			StrToCmp;
var() bool				bIgnoreCase;

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
	ObjName="Switch String"

	bIgnoreCase=true

	InputLinks(0)=(LinkDesc="In")
	OutputLinks(0)=(LinkDesc="Default")

	VariableLinks(0)=(ExpectedType=class'SeqVar_String',LinkDesc="Str",PropertyName=StrToCmp)
}