/*
	Kismet에서 List로 가지고 있는 String들에서 Index값에 해당하는 String을 얻어오는 액션.

	2007/01/29 고광록

	@original avaSeqAct_GetObjectInList.uc by Oz
*/
class avaSeqAct_GetStringInList extends SequenceAction;

var() int			Index;
var() array<string>	StrList;
var	  string		Result;	

event Activated()
{
	if ( Index < StrList.Length && Index >= 0 )
	{
		Result = StrList[Index];
	}
}

defaultproperties
{
	ObjName="Get String in List"
	ObjCategory="Object List"
	bCallHandler=false

	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Index",PropertyName=Index,MaxVars=1)
	VariableLinks(1)=(ExpectedType=class'SeqVar_String',LinkDesc="String",PropertyName=Result,MaxVars=1,bWriteable=true)
}