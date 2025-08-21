class avaUIEvent_ProcBuddyInviteGuild extends UIEvent;


defaultproperties
{
	ObjName="Proc Buddy InviteGuild"
	ObjCategory="ProcBuddy"
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Invitor|GuildName",bWriteable=true))

	ObjClassVersion=1
}