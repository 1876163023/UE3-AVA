class avaUIEvent_UpdateTicker extends UIEvent;

var() string	TickerMsg;

event Activated()
{
	Local SequenceVariable SeqVar;

	if ( VariableLinks.Length >= 1 && VariableLinks[0].LinkedVariables.Length > 0 )
	{
		SeqVar = VariableLinks[0].LinkedVariables[0];
		SeqVar_String(SeqVar).StrValue = TickerMsg;
	}
}

function Trigger( string inTickerMsg )
{
	TickerMsg = inTickerMsg;
	ActivateUIEvent( -1 , GetOwner(), None, FALSE);
}

defaultproperties
{
	ObjName="Update Ticker"
	ObjCategory="avaNet"

	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="TickerMsg",PropertyName=TickerMsg,MinVars=1,bWriteable=true))
}