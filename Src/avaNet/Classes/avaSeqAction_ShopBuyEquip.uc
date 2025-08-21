class avaSeqAction_ShopBuyEquip extends avaSeqAction;


var() int ListIndex;


event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().ShopBuyEquip(ListIndex);
}


defaultproperties
{
	ObjName="(Shop) Buy Equip"

	InputLinks(0)=(LinkDesc="In")

    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="List Index",PropertyName=ListIndex)

	ObjClassVersion=1
}

