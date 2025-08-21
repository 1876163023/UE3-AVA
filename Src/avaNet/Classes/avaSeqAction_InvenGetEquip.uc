class avaSeqAction_InvenGetEquip extends avaSeqAction;


var() EPlayerClass InPlayerClass;
var() int PrimaryID, PistolID, KnifeID, GrenadeID1, GrenadeID2, GrenadeID3;



event Activated()
{
}


defaultproperties
{
	ObjName="(Inventory) Get Equip Set"

    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Class",PropertyName=InPlayerClass)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Int',LinkDesc="Primary ID",bWriteable=true,PropertyName=PrimaryID)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Int',LinkDesc="Pistol ID",bWriteable=true,PropertyName=PistolID)
	VariableLinks(3)=(ExpectedType=class'SeqVar_Int',LinkDesc="Knife ID",bWriteable=true,PropertyName=KnifeID)
	VariableLinks(4)=(ExpectedType=class'SeqVar_Int',LinkDesc="Grenade ID 1",bWriteable=true,PropertyName=GrenadeID1)
	VariableLinks(5)=(ExpectedType=class'SeqVar_Int',LinkDesc="Grenade ID 2",bWriteable=true,PropertyName=GrenadeID2)
	VariableLinks(6)=(ExpectedType=class'SeqVar_Int',LinkDesc="Grenade ID 3",bWriteable=true,PropertyName=GrenadeID3)
}

