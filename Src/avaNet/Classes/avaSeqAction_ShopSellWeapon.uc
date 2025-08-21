class avaSeqAction_ShopSellWeapon extends avaSeqAction;


var() int ListIndex;


event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().ShopSellWeapon(ListIndex);
}


defaultproperties
{
	ObjName="(Shop) Sell Weapon"

	InputLinks(0)=(LinkDesc="In")

    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="List Index",PropertyName=ListIndex)

	ObjClassVersion=1
}

