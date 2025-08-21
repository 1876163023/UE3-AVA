class avaUIAction_StrLen extends UIAction;

event activated()
{
	Local string SourceStr;
	Local SeqVar_String		VarStr;
	Local SeqVar_Int		VarInt;
	Local int Length;

	foreach LinkedVariables(class'SeqVar_String',VarStr,"SourceStr")
	{
		SourceStr = VarStr.StrValue;
	}


	Length = Len( SourceStr );
	foreach LinkedVariables(class'SeqVar_Int',VarInt,"Length")
	{
		VarInt.IntValue = Length;
	}
}


defaultproperties
{
	ObjCategory="String Manipulation"
	ObjName="StrLen"
	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="SourceStr",bWriteable=false,MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Length", bWriteable=true))
}