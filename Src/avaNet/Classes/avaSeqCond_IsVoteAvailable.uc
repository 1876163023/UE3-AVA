class avaSeqCond_IsVoteAvailable extends avaSeqCondition;



event Activated()
{
	if ( class'avaNetHandler'.static.GetAvaNetHandler().IsVoteAvailable() )
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
	ObjName="(Room/Game) Is Vote Available"

	OutputLinks(0)=(LinkDesc="True")
	OutputLinks(1)=(LinkDesc="False")

	VariableLinks.Empty
}