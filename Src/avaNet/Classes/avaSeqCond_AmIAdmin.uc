class avaSeqCond_AmIAdmin extends avaSeqCondition;



event Activated()
{
	if ( class'avaNetHandler'.static.GetAvaNetHandler().AmIAdmin() )
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
	ObjName="(Room) Am I Admin"

	OutputLinks(0)=(LinkDesc="True")
	OutputLinks(1)=(LinkDesc="False")

	VariableLinks.Empty
}