class avaSeqCond_CanExitGame extends avaSeqCondition;


event Activated()
{
	BoolResult = class'avaNetHandler'.static.GetAvaNetHandler().CanExitGame();
	if ( BoolResult )
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
	ObjName="(Game) Can Exit Game "

	OutputLinks(0)=(LinkDesc="True")
	OutputLinks(1)=(LinkDesc="False")

	VariableLinks.Empty
}