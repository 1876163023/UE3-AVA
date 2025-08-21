class avaUIAction_ConsoleCommand extends UIAction;

var() bool		bWriteToLog;

event Activated()
{
	Local PlayerController PC;
	Local WorldInfo WorldInfo;
	Local SeqVar_String VarStr;

	if( GetOwnerScene() == None )
		return;

	WorldInfo = GetOwnerScene().GetWorldInfo();

	foreach WorldInfo.LocalPlayerControllers(PC)
	{
		foreach LinkedVariables(class'SeqVar_String', VarStr,"Command")
		{
			avaPlayerController(PC).ConsoleCommand(VarStr.StrValue, bWriteToLog);
		}
	}
}


defaultproperties
{
	bCallHandler=false
	ObjCategory="Misc"
	ObjName="avaUIConsoleCommand"

	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Command",bWriteable=false))

	bWriteToLog=false
}