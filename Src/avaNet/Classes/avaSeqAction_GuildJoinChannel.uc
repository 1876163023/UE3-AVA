class avaSeqAction_GuildJoinChannel extends avaSeqAction;

event Activated()
{
	`log( "**************************************************************************" );
	`log( "************************ avaSeqAction_GuildJoinChannel *******************" );
	`log( "**************************************************************************" );
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().GuildJoinChannel();
}


defaultproperties
{
	ObjName="(Guild) Join Channel"

    VariableLinks.Empty
	ObjClassVersion=2
}

