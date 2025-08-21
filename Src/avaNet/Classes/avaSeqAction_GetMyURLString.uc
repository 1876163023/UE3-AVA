class avaSeqAction_GetMyURLString extends avaSeqAction;

var() string			PlayerPropertyName<CurrentAvailables:"ChItem","SkillP","SkillR","SkillS","WeaponP","WeaponR","WeaponS","LastClass","LastWeapon">;
var() string			OutString;
var() int				OutInt;

event Activated()
{
	OutString = class'avaNetHandler'.static.GetAvaNetHandler().GetMyURLString(PlayerPropertyName);
	OutInt = int(OutString);

	if( Len(OutString) != 0 )
		OutputLinks[0].bHasImpulse = true;
	else
		OutputLinks[1].bHasImpulse = true;
}

defaultproperties
{
	ObjName="Get PlayerProperty (URL)"
	bAutoActivateOutputLinks=false

	OutputLinks.Empty

	OutputLinks(0)=(LinkDesc="Accept")
	OutputLinks(1)=(LinkDesc="Empty")

	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_string',LinkDesc="PlayerProperty",PropertyName=PlayerPropertyName))
	VariableLinks.Add((ExpectedType=class'SeqVar_string',LinkDesc="OutStr",PropertyName=OutString,bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_int',LinkDesc="OutInt",PropertyName=OutInt,bWriteable=true))
}