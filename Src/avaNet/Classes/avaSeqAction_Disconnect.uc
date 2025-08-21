class avaSeqAction_Disconnect extends avaSeqAction;


event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().CloseConnection(2);

	OutputLinks[0].bhasImpulse = true;
}


defaultproperties
{
	ObjName="Disconnect"

	OutputLinks(0)=(LinkDesc="Output")

	bAutoActivateOutputLinks=false

    VariableLinks.Empty
}

