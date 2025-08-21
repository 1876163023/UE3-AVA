/*
	2008/02/12
		불필요한듯 싶어서 문자열만 얻어오도록 수정하였습니다.
*/
class avaSeqAct_GetConnectResult extends UIAction;

var() string LastConnectResult;

event Activated()
{
	local string ConnectResult;
	ConnectResult =	class'avaNetHandler'.static.GetAvaNetHandler().GetConnectResult();
	LastConnectResult = ConnectResult;

	`log( "avaSeqAct_GetConnectResult.Activated" @ConnectResult @GetOwner() @GetOwnerScene() );

/*
	switch(ConnectResult)
	{
		case "connecting":			OutputLinks[0].bHasImpulse=true; break;
		case "loading":				OutputLinks[1].bHasImpulse=true; break;
		case "ok":					OutputLinks[2].bHasImpulse=true; break;
		case "no nick":				OutputLinks[3].bHasImpulse=true; break;
		case "already connected":	OutputLinks[4].bHasImpulse=true; break;
		case "invalid version":		OutputLinks[5].bHasImpulse=true; break;
		case "no key":				OutputLinks[6].bHasImpulse=true; break;
		case "failed":
		default:					OutputLinks[7].bHasImpulse=true; break;

		case "admin kicked":		OutputLinks[8].bHasImpulse=true; break;
		case "channel full":		OutputLinks[9].bHasImpulse=true; break;
		case "gameguard error":		OutputLinks[10].bHasImpulse=true; break;
		case "no resource":			OutputLinks[11].bHasImpulse=true; break;
	}
*/
}

defaultproperties
{
	ObjName="Get Connect Result"
	ObjCategory="avaNet"

/*
	OutputLinks(0)=(LinkDesc="Connecting")
	OutputLinks(1)=(LinkDesc="Loading")
	OutputLinks(2)=(LinkDesc="Ok")
	OutputLinks(3)=(LinkDesc="No Nick")
	OutputLinks(4)=(LinkDesc="Already Connected")
	OutputLinks(5)=(LinkDesc="Invalid Version")
	OutputLinks(6)=(LinkDesc="No Key")
	OutputLinks(7)=(LinkDesc="Failed")

	OutputLinks(8)=(LinkDesc="Admin Kicked")
	OutputLinks(9)=(LinkDesc="Channel Full")
	OutputLinks(10)=(LinkDesc="Gameguard Error")
	OutputLinks(11)=(LinkDesc="No Resource")

	bAutoActivateOutputLinks=false
*/

	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Connect Result",PropertyName=LastConnectResult,bWriteable=true))

	ObjClassVersion=3
}