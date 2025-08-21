class avaUIAction_InStr extends UIAction;


event activated()
{
	Local string SourceStr, TargetStr;
	Local SeqVar_String		VarStr;
	Local SeqVar_Int		VarInt;
	Local int FindIndex;

	foreach LinkedVariables(class'SeqVar_String',VarStr, "SourceStr")
	{
		SourceStr = VarStr.StrValue;
	}

	foreach LinkedVariables(class'SeqVar_String',VarStr,"TargetStr")
	{
		TargetStr = VarStr.StrValue;
	}

	FindIndex = InStr( SourceStr, TargetStr );
	foreach LinkedVariables(class'SeqVar_Int',VarInt,"FindIndex")
	{
		VarInt.IntValue = FindIndex;
	}
}


defaultproperties
{
	ObjCategory="String Manipulation"
	ObjName="InStr"
	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="SourceStr",bWriteable=false,MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="TargetStr",bWriteable=false,MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="FindIndex"))
}