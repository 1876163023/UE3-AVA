class avaSeqAction_GetItemDesc extends avaSeqAction;


var() int ItemId;
var int LevelLimit;
var int GaugeType;
var int Price;
var int RepairPrice;
var int RebuildPrice;
var string LiteralDesc;
var string ItemName;
var string IconCode;
var bool Customizable;


event Activated()
{
	Local byte byteLevelLimit;
	Local byte byteGaugeType;
	Local bool bResult;
	Local int Custom;

	bResult = class'avaNetHandler'.static.GetAvaNetHandler().GetItemDesc(ItemId, ItemName, byteLevelLimit, byteGaugeType ,Price, RepairPrice, RebuildPrice, LiteralDesc, IconCode, Custom);
	LevelLimit = int(byteLevelLimit);
	GaugeType = int(byteGaugeType);
	Customizable = (Custom > 0);

	if( bResult )
		OutputLinks[0].bHasImpulse = true;
	else
		OutputLinks[1].bHasImpulse = true;

}

defaultproperties
{
	ObjName="Get ItemDesc"

	bAutoActivateOutputLinks=false

    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Item ID",PropertyName=ItemId)

	VariableLinks(1)=(ExpectedType=class'SeqVar_Int',LinkDesc="Level Limit",bWriteable=true,PropertyName=LevelLimit)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Int',LinkDesc="GaugeType",bWriteable=true,PropertyName=GaugeType)
	VariableLinks(3)=(ExpectedType=class'SeqVar_Int',LinkDesc="Price",bWriteable=true,PropertyName=Price)
	VariableLinks(4)=(ExpectedType=class'SeqVar_Int',LinkDesc="Repair Price",bWriteable=true,PropertyName=RepairPrice)
	VariableLinks(5)=(ExpectedType=class'SeqVar_Int',LinkDesc="Rebuild Price",bWriteable=true,PropertyName=RebuildPrice)
	VariableLinks(6)=(ExpectedType=class'SeqVar_String',LinkDesc="Description",bWriteable=true,PropertyName=LiteralDesc)
	VariableLinks(7)=(ExpectedType=class'SeqVar_String',LinkDesc="Item Name",bWriteable=true,PropertyName=ItemName)
	VariableLinks(8)=(ExpectedType=class'SeqVar_String',LinkDesc="Icon Code",bWriteable=true,PropertyName=IconCode)
	VariableLinks(9)=(ExpectedType=class'SeqVar_Bool',LinkDesc="Customizable",bWriteable=true,PropertyName=Customizable)

	OutputLinks(0)=(LinkDesc="Accept")
	OutputLinks(1)=(LinkDesc="Failed")

	ObjClassVersion=4
}
