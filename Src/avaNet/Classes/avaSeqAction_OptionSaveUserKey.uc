class avaSeqAction_OptionSaveUserKey extends avaSeqAction;

event Activated()
{
	Local WorldInfo WorldInfo;
	Local PlayerController LocalPC, PC;

	WorldInfo = GetWorldInfo();
	if( WorldInfo == None )
		return;

	foreach WorldInfo.LocalPlayerControllers(LocalPC)
	{
		PC = LocalPC;
	}

	if( PC == None || PC.PlayerInput == None )
		return;

	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().OptionSaveUserKey( PC.PlayerInput.GetUserKeyString(), class'avaOptionSettings'.static.GetGameOptionString() );
	//`Log("Send the userKey '"$PC.PlayerInput.GetUserKeyString()$"' to server)");
}


defaultproperties
{
	ObjName="(Option) Save UserKey"

	ObjClassVersion=2
}
