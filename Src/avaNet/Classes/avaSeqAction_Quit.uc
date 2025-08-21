class avaSeqAction_Quit extends avaSeqAction;


var() bool bGraceful;

event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().Quit(bGraceful);

	OutputLinks[0].bhasImpulse = true;
}


defaultproperties
{
	ObjName="Quit"

	OutputLinks(0)=(LinkDesc="Output")

	bAutoActivateOutputLinks=false
	
	bGraceful=true

	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Gracefully",PropertyName=bGraceful))

    VariableLinks.Empty
}

