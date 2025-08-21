/*
	List에서 해당 CellFieldName과 CellStringValue와 같은 Cell을 선택되어지도록 한다.

	2007/02/14	고광록

	@reference UIAction_GetCellValue.uc
*/
class avaUIAction_SetListIndexFromCellValue extends UIAction
	native;

cpptext
{
	virtual void Activated();
}

//! The name of the field to retrieve the data markup for
var() name			CellFieldName;
//! 비교할 Cell의 문자열 값.
var() string		CellStringValue;
//! 새로 설정된 Index(array상의 값이 아님).
var int				ItemIndex;
//! 새로 설정된 List Index.
var int				ListIndex;

var()	int			TopIndex;

DefaultProperties
{
	ObjClassVersion=2
	bCallHandler=false

	ObjCategory="Set Value"
	ObjName="Set List Index form Cell Value"

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Target",PropertyName=Targets)
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Cell StringValue",PropertyName=CellStringValue))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="ItemIndex",PropertyName=ItemIndex,MaxVars=1,bWriteable=true,bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="ListIndex",PropertyName=ItemIndex,MaxVars=1,bWriteable=true,bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Top Index",PropertyName=TopIndex,MaxVars=1))

	OutputLinks(0)=(LinkDesc="Success")
	OutputLinks(1)=(LinkDesc="Failure")

	TopIndex = -1;
}
