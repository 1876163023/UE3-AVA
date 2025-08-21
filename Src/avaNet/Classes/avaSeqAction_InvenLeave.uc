class avaSeqAction_InvenLeave extends avaSeqAction;



event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().LeaveInven();
}


defaultproperties
{
	ObjName="(Inventory) Leave"

	InputLinks(0)=(LinkDesc="In")

    VariableLinks.Empty

	ObjClassVersion=1
}

