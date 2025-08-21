class avaSeqAction_JoinChannel extends avaSeqAction;

var() int ListIndex;
var() string Flags;

event Activated()
{
	`log( "**************************************************************************" );
	`log( "************************ avaSeqAction_JoinChannel ************************" );
	`log( "**************************************************************************" );

	if (InputLinks[0].bHasImpulse)
		class'avaNet.avaNetRequest'.static.GetAvaNetRequest().JoinChannel(ListIndex);
	else if (InputLinks[1].bHasImpulse)
		class'avaNet.avaNetRequest'.static.GetAvaNetRequest().QuickJoinChannel(Flags);
}


defaultproperties
{
	ObjName="Join Channel"

	ListIndex=-1

	InputLinks(0)=(LinkDesc="Join")
	InputLinks(1)=(LinkDesc="Quick Join")

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Index",PropertyName=ListIndex))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Flags",PropertyName=Flags))

	ObjClassVersion=3
}

