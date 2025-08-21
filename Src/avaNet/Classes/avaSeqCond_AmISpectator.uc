class avaSeqCond_AmISpectator extends avaSeqCondition;



event Activated()
{
	if ( class'avaNet.avaNetRequest'.static.GetAvaNetRequest().AmISpectator() )
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
	ObjName="(Room) Am I Spectator"

	OutputLinks(0)=(LinkDesc="True")
	OutputLinks(1)=(LinkDesc="False")

	VariableLinks.Empty
}
