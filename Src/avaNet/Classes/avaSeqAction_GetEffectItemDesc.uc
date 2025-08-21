class avaSeqAction_GetEffectItemDesc extends avaSeqAction;


var() int ItemId;
var int GaugeType;
var int EffectType;
var int EffectValue;
var string ItemDesc;
var string ItemName;
var string IconStr;

event Activated()
{
	Local bool bResult;

	bResult = class'avaNetHandler'.static.GetAvaNetHandler().GetEffectItemDesc(ItemId, GaugeType, EffectType, EffectValue, ItemName, ItemDesc, IconStr);

	if( bResult )
		OutputLinks[0].bHasImpulse = true;
	else
		OutputLinks[1].bHasImpulse = true;

}

defaultproperties
{
	ObjName="Get EffectItemDesc"

	bAutoActivateOutputLinks=false

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Item ID",PropertyName=ItemId))

	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Gauge Type",bWriteable=true,PropertyName=GaugeType))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Effect Type",bWriteable=true,PropertyName=EffectType))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Effect Value",bWriteable=true,PropertyName=EffectValue)
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Item Desc",bWriteable=true,PropertyName=ItemDesc))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Item Name",bWriteable=true,PropertyName=ItemName))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Icon Str",bWriteable=true,PropertyName=IconStr))

	OutputLinks(0)=(LinkDesc="Accept")
	OutputLinks(1)=(LinkDesc="Failed")

	ObjClassVersion=4
}
