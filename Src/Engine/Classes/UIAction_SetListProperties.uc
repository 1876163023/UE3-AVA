/**
 *	UIAction_SetListProperties.
 *
 *	리스트내의 속성 필드(NoSelectionWhenRefresh, DisableSelection 등등 )를 키스멧에서 변경
 */
class UIAction_SetListProperties extends UIAction;

var() bool	bDisableItemSelection;
var() bool	bCellUnitDrawing;
var() bool	bDrawVisibleItems;
var() bool	bEnableMultiSelect;
var() bool	bEnableVerticalScrollbar;
var() bool	bNoSelectWhenRefresh;
var() bool	bAlwaysNotifyChanged;

event Activated()
{
	Local int i;
	Local UIList List;
	Local SeqVar_Bool BoolVal;

	for( i = 0 ; i < Targets.Length ; i++ )
	{
		List = UIList(Targets[i]);
		if( List == none )
			continue;

		foreach LinkedVariables(class'SeqVar_Bool', BoolVal, "DisableSelection")
		{
			List.bAllowDisabledItemSelection = bool(BoolVal.bValue);
		}
		foreach LinkedVariables(class'SeqVar_Bool', BoolVal, "CellUnitDraw")
		{
			List.bCellUnitDrawing = bool(BoolVal.bValue);
		}
		foreach LinkedVariables(class'SeqVar_Bool', BoolVal, "DrawVisItems")
		{
			List.bDrawVisibleItems = bool(BoolVal.bValue);
		}
		foreach LinkedVariables(class'SeqVar_Bool', BoolVal, "EnbMultiSel")
		{
			List.bEnableMultiSelect = bool(BoolVal.bValue);
		}
		foreach LinkedVariables(class'SeqVar_Bool', BoolVal, "EnbVertScroll")
		{
			List.bEnableVerticalScrollbar = bool(BoolVal.bValue);
		}
		foreach LinkedVariables(class'SeqVar_Bool', BoolVal, "NoSelWhenRefresh")
		{
			List.bNoSelectWhenRefresh = bool(BoolVal.bValue);
		}
		foreach LinkedVariables(class'SeqVar_Bool', BoolVal, "AlwaysNotify")
		{
			List.bNotifyAlways = bool(BoolVal.bValue);
		}
	}
}

defaultproperties
{
	ObjName = "Set List Properties"
	ObjCategory = "Set Value"

	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="DisableSelection",PropertyName=bDisableItemSelection,bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="CellUnitDraw",PropertyName=bCellUnitDrawing,bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="DrawVisItems",PropertyName=bDrawVisibleItems,bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="EnbMultiSel",PropertyName=bEnableMultiSelect,bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="EnbVertScroll",PropertyName=bEnableVerticalScrollbar,bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="NoSelWhenRefresh",PropertyName=bNoSelectWhenRefresh,bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="AlwaysNotify",PropertyName=bAlwaysNotifyChanged,bHidden=true))

	ObjClassVersion=1
}