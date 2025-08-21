class avaSeqAction_LeaveRoom extends avaSeqAction;


var() EavaLeavingReason Reason;


event Activated()
{
	`log("avaSeqAction_LeaveRoom called");
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().LeaveRoom(Reason);
}


defaultproperties
{
	ObjName="(Room) Leave"

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Reason",PropertyName=Reason))

	ObjClassVersion=2
}

