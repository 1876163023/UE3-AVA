class avaSeqAction_ListRoom extends avaSeqAction;



event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().ListRoom();
}


defaultproperties
{
	ObjName="(Lobby) List Room"

    VariableLinks.Empty
    
    ObjClassVersion=2
}

