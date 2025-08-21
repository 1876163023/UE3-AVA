class avaSeqAction_InvenGetUsedItemEffect extends avaSeqAction;

var() EItemEffectType ItemEffect;
var INT Count;

event Activated()
{
	Local SeqVar_Int IntVar;

	foreach LinkedVariables(class'SeqVar_Int', IntVar, "Effect Type")
	{
		ItemEffect = EItemEffectType(IntVar.IntValue);
	}

	Count = class'avaNetRequest'.static.GetAvaNetRequest().GetUsedItemEffect(ItemEffect);
}


defaultproperties
{
	ObjName="(inventory) Get Used Item Effect"

    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Effect Type",PropertyName=ItemEffect)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Int',LinkDesc="Count",bWriteable=true,PropertyName=Count)
}