class avaUIAction_ApplyModifier extends UIAction_SetValue;

var() array< class<avaModifier> >	Modifiers;

defaultproperties
{
	ObjName="ava ApplyModifier"
	VariableLinks.Add( (ExpectedType=class'SeqVar_Object',LinkDesc="Modifier",PropertyName=Modifiers) )
}
