class avaSeqAction_SetLastChannelGroup extends avaSeqAction;

var() array<int> GroupNum;
var() bool bSaveConfig;

event Activated()
{
	if( GroupNum.Length > 0 )
		class'avaGame.avaOptionSettings'.static.SetLastChannelGroup( GroupNum[0] );

	if( bSaveConfig )
		class'avaGame.avaOptionSettings'.static.FlushChanged();
}

defaultproperties
{
	ObjName="(Channel) Set LastChannelGroup"

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Group Num",PropertyName=GroupNum,MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Save Config",PropertyName=bSaveConfig))

	ObjClassVersion=1
}