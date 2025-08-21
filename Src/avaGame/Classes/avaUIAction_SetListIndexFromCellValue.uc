/*
	List���� �ش� CellFieldName�� CellStringValue�� ���� Cell�� ���õǾ������� �Ѵ�.

	2007/02/14	����

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
//! ���� Cell�� ���ڿ� ��.
var() string		CellStringValue;
//! ���� ������ Index(array���� ���� �ƴ�).
var int				ItemIndex;
//! ���� ������ List Index.
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
