/*
	다양한 값을 Integer값으로 변환해준다.

	2007/02/06	고광록
		avaUIAction_ToString.uc를 참고해서 만듬.
		(UI/Level Kismet모두 사용 가능)
*/
class avaSeqAct_SetIntFromVariable extends SeqAct_SetSequenceVariable;

var int IntValue;

event activated()
{
	local SeqVar_String	VarStr;
	local SeqVar_Bool	VarBool;
	local SeqVar_Float	VarFloat;

	local string		InStr;
	local float			InFloat;
	local bool			InBool;

	local bool			bHasStr;
	local bool			bHasFloat;
	local bool			bHasBool;


	foreach LinkedVariables(class'SeqVar_String', VarStr, "Int")
	{
		InStr = VarStr.StrValue;
		bHasStr = true;
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

	if( bHasStr )
		IntValue = int(InStr);
	else if ( bHasFloat )
		IntValue = int(InFloat);
	else if ( bHasBool )
		IntValue = int(InBool);

	`log("IntValue = " @IntValue @InStr);
}


defaultproperties
{
	ObjCategory="Set Variable"
	ObjName="Set Int from Variable"

	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="String",MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Float",MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Bool",MaxVars=1))

	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="OutInt",PropertyName=IntValue,bWriteable=true))
}