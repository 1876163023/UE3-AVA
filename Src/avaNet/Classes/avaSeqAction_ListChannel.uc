class avaSeqAction_ListChannel extends avaSeqAction;



event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().ListChannel();
}


defaultproperties
{
	ObjName="List Channel"

    VariableLinks.Empty
}

