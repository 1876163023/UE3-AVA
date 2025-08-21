class avaUIAction_GetVisibility extends UIAction;

var() bool bVisibility;

event Activated()
{
	Local int i;
	for( i = 0 ; i < Targets.Length ; i++ )
		if( UIScreenObject(Targets[i]) != None )
			bVisibility = UIScreenObject(Targets[i]).IsVisible();
}

defaultproperties
{
	ObjName="Get Visibility"
	ObjCategory="UI"
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Visibility",PropertyName="bVisibility",bWriteable=true))
}