class avaSeqAction_GetLastChannelGroup extends avaSeqAction;

var() int GroupNum;

event Activated()
{
	Local avaOptionSettings OptionSettings;
	OptionSettings = class'avaGame.avaOptionSettings'.static.GetDefaultObject();
	GroupNum = OptionSettings.GetLastChannelGroup();
	`Log("GroupNum = "$GroupNum);
}

defaultproperties
{
	ObjName="(Channel) Get Last ChannelGroup"

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Group Num",PropertyName=GroupNum))

	ObjClassVersion=1
}

