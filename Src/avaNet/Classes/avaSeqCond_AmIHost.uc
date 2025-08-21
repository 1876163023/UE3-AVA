class avaSeqCond_AmIHost extends avaSeqCondition;



event Activated()
{
	if ( class'avaNet.avaNetRequest'.static.GetAvaNetRequest().AmIHost() )
	{
		OutputLinks[0].bHasImpulse = true;
	}
	else
	{
		OutputLinks[1].bHasImpulse = true;
	}
}


defaultproperties
{
	ObjCategory="avaNet"
	ObjName="(Room) Am I Host"

	OutputLinks(0)=(LinkDesc="True")
	OutputLinks(1)=(LinkDesc="False")

	VariableLinks.Empty
}
