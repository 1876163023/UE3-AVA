class avaUIAction_StrMid extends UIAction;

event activated()
{
	Local string SourceStr, OutStr;
	Local SeqVar_String		VarStr;
	Local SeqVar_Int		VarInt;
	Local int StartIndex, Count;

	foreach LinkedVariables(class'SeqVar_String',VarStr,"SourceStr")
	{
		SourceStr = VarStr.StrValue;
	}

	StartIndex = -1;
	Count = -1;

	foreach LinkedVariables(class'SeqVar_Int', VarInt, "StartIndex")
	{
		StartIndex = VarInt.IntValue;
	}

	foreach LinkedVariables(class'SeqVar_Int', VarInt, "Count")
	{
		Count = VarInt.IntValue;
	}
	
	StartIndex = StartIndex >= 0 ? StartIndex : 0;

	if( Count < 0 )
		OutStr = Mid(SourceStr, StartIndex);	
	else
		Outstr = Mid(SourceStr, StartIndex, Count);

	foreach LinkedVariables(class'SeqVar_String',VarStr,"OutStr")
	{
		VarStr.StrValue = OutStr;
	}
}


defaultproperties
{
	ObjCategory="String Manipulation"
	ObjName="StrMid"
	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="SourceStr",bWriteable=false,MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="StartIndex",MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Count",MaxVars=1))

	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="OutStr",bWriteable=true))
}