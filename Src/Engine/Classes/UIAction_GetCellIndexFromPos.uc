class UIAction_GetCellIndexFromPos extends UIAction_GetValue;

var() int RowIndex;
var() int ColIndex;

var() bool bAbsoluteIndex;

var() int CollectionIndex;

defaultproperties
{
	ObjName="GetCellIndexFromPos"
	ObjClassVersion=1

	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Row Index",PropertyName="RowIndex",bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Col Index",PropertyName="ColIndex",bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Item",PropertyName="CollectionIndex",bWriteable=true))

	bAbsoluteIndex = true
}