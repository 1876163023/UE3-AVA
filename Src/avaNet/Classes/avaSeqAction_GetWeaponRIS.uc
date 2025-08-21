class avaSeqAction_GetWeaponRIS extends avaSeqAction;

var() int InvenSlot;
var() bool bIsRISConvertible;
var() int RemodelPrice;

event Activated()
{
	Local bool bResult;
	Local byte byteRISConv;

	bResult = class'avaNetHandler'.static.GetAvaNetHandler().GetWeaponRIS(InvenSlot, byteRISConv, RemodelPrice);
	bIsRISConvertible = bool(byteRISConv);

	if( bResult )
		OutputLinks[0].bHasImpulse = true;
	else
		OutputLinks[1].bHasImpulse = true;

}


defaultproperties
{
	ObjName="(Inventory) Get WeaponRIS"

	bAutoActivateOutputLinks=false

    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Inven Slot",PropertyName=InvenSlot)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Bool',LinkDesc="IsRISConvertible",bWriteable=true,PropertyName=bIsRISConvertible)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Int',LinkDesc="Remodel Price",bWriteable=true,PropertyName=RemodelPrice)

	OutputLinks(0)=(LinkDesc="Accept")
	OutputLinks(1)=(LinkDesc="Failed");
}