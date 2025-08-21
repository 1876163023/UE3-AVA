class avaSeqAction_GetMapName extends avaSeqAction;


var() int ListIndex;
var() string MapName;



event Activated()
{
	MapName = class'avaNet.avaNetRequest'.static.GetAvaNetRequest().GetMapName(ListIndex);
}


defaultproperties
{
	ObjName="Get Map Name"

    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="List Index",PropertyName=ListIndex)
	VariableLinks(1)=(ExpectedType=class'SeqVar_String',LinkDesc="Map Name",bWriteable=true,PropertyName=MapName)
}

