class avaUICond_IsSpectatorAllowed extends SequenceCondition;

event Activated()
{
	if ( class'avaNetHandler'.static.GetAvaNetHandler().IsSpectatorAllowed() )
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
	ObjName="(Room) Is Spectator Allowed"

	OutputLinks(0)=(LinkDesc="True")
	OutputLinks(1)=(LinkDesc="False")

	VariableLinks.Empty
}