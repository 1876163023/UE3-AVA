class UIAction_GetCellPosFromIndex extends UIAction_GetValue;

var() int RowIndex;
var() int ColIndex;

var() float ViewportLeft;
var() float ViewportTop;
var() float ViewportRight;
var() float ViewportBottom;

defaultproperties
{
	ObjName="GetCellPosFromIndexs"
	ObjClassVersion=1

	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Row Index",PropertyName="RowIndex"))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Col Index",PropertyName="ColIndex"))

	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Left",PropertyName="ViewportLeft",bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Top",PropertyName="ViewportTop",bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Right",PropertyName="ViewportRight",bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Bottom",PropertyName="ViewportBottom",bWriteable=true))
}