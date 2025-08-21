class avaSeqAction_InvenEnter extends avaSeqAction;



event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().EnterInven();
}


defaultproperties
{
	ObjName="(Inventory) Enter"

	InputLinks(0)=(LinkDesc="In")

    VariableLinks.Empty

	ObjClassVersion=1
}

