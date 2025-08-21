class avaUIAction_SetEnabled extends UIAction native;

var() bool bEnabled;

cpptext
{
	/**
	 * Activates the state associated with this action for all targets.  If any of the targets cannot change states,
	 * disables all output links.
	 */
	virtual void Activated();
}

DefaultProperties
{
	ObjName="Enabled UI"
	bCallHandler=FALSE

	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Enable",PropertyName=bEnabled,MinVars=0,MaxVars=1))

	OutputLinks(0)=(LinkDesc="Successful")
	OutputLinks(1)=(LinkDesc="Failed")
}
