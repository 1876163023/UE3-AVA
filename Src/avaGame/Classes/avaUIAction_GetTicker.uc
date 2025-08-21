class avaUIAction_GetTicker extends UIAction;

var() string		TickerMsg;

event Activated()
{
	local WorldInfo					WorldInfo;
	if ( GetOwnerScene() == None )						return;
	WorldInfo = GetOwnerScene().GetWorldInfo();
	TickerMsg = avaNetEntryGameEx( WorldInfo.Game ).GetCurrentTickerMsg();
}

defaultproperties
{
	ObjName="Get Ticker Message"
	bCallHandler=false

	VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_String',LinkDesc="TickerMsg",PropertyName=TickerMsg,MaxVars=1,bWriteable=true)
}