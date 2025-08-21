class avaUIAction_ToString extends UIAction;

event activated()
{
	Local string OutStr;
	Local SeqVar_String		VarStr;
	Local SeqVar_Int		VarInt;
	Local SeqVar_Bool		VarBool;
	Local SeqVar_Float		VarFloat;
	Local int InInt;
	Local float InFloat;
	Local bool InBool;
	Local bool bHasInt;
	Local bool bHasFloat;
	Local bool bHasBool;


	foreach LinkedVariables(class'SeqVar_Int', VarInt, "Int")
	{
		InInt = VarInt.IntValue;
		bHasInt = true;
	}

	foreach LinkedVariables(class'SeqVar_Float', VarFloat, "Float")
	{
		InFloat = VarFloat.FloatValue;
		bHasFloat = true;
	}

	foreach LinkedVariables(class'SeqVar_Bool', VarBool, "Bool")
	{
		InBool = bool(VarBool.bValue);
		bHasBool = true;
	}

	if( bHasInt )
		OutStr = string(InInt);
	else if ( bHasFloat )
		OutStr = string(InFloat);
	else if ( bHasBool )
		OutStr = string(InBool);

	foreach LinkedVariables(class'SeqVar_String',VarStr,"OutStr")
	{
		VarStr.StrValue = OutStr;
	}
}


defaultproperties
{
	ObjCategory="String Manipulation"
	ObjName="ToString"
	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Int",MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Float",MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Bool",MaxVars=1))

	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="OutStr",bWriteable=true))
}