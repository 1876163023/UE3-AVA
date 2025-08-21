class avaSeqCond_IsStealthMode extends avaSeqCondition;

event Activated()
{
	if ( class'avaNetHandler'.static.GetAvaNetHandler().IsStealthMode() )
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
	ObjName="(Room) Is StealthMode"

	OutputLinks(0)=(LinkDesc="True")
	OutputLinks(1)=(LinkDesc="False")

	VariableLinks.Empty
}