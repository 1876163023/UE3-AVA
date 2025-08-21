class avaSeqAction_ShopBuyWeapon extends avaSeqAction;


var() int ListIndex;


event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().ShopBuyWeapon(ListIndex);
}


defaultproperties
{
	ObjName="(Shop) Buy Weapon"

	InputLinks(0)=(LinkDesc="In")

    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="List Index",PropertyName=ListIndex)

	ObjClassVersion=1
}

