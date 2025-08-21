class avaSeqAction_Connect extends avaSeqAction;



var() bool bForce;
var() string USN;
var() string UserID;
var() string KeyString;

event Activated()
{
	if ( !class'avaNet.avaNetRequest'.static.GetAvaNetRequest().AutoConnect(bForce, USN, UserID, KeyString) )
	{
		OutputLinks[1].bhasImpulse = true;
		return;
	}

	OutputLinks[0].bhasImpulse = true;
}


defaultproperties
{
	ObjName="Connect"

	OutputLinks(0)=(LinkDesc="Successful")
	OutputLinks(1)=(LinkDesc="Failed")

	bAutoActivateOutputLinks=false

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Force",PropertyName=bForce))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="USN",PropertyName=USN))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="ID",PropertyName=UserID))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Key",PropertyName=KeyString))

	ObjClassVersion=2
}

