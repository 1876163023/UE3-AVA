class avaSeqAction_RoomGetSetting extends avaSeqAction;



var int MapIndex;
var int tkLevel;
var bool autoBalance;
var bool allowSpectator;
var bool allowInterrupt;
var bool allowBackView;
var bool allowGhostChat;
var int roundToWin;
var int MaxPlayer;
var bool AutoTeamSwap;
var int MapOption;
var bool bPassword;


event Activated()
{
	local avaNetRequest.avaRoomSetting RoomSetting;
	local int Len;

	if ( class'avaNet.avaNetRequest'.static.GetAvaNetRequest().GetCurrentRoomSetting(RoomSetting) )
	{
		Len = class'avaNetRequest'.static.GetAvaNetRequest().MapList.Length;
		for (MapIndex = 0; MapIndex < Len; MapIndex++)
			if (class'avaNetRequest'.static.GetAvaNetRequest().MapList[MapIndex].MapID == RoomSetting.idMap)
				break;

		if (MapIndex == Len)
		{
			OutputLinks[1].bHasImpulse = true;
			return;
		}

		tkLevel = RoomSetting.tkLevel;
		autoBalance = RoomSetting.autoBalance;
		allowSpectator = RoomSetting.allowSpectator;
		allowInterrupt = RoomSetting.allowInterrupt;
		allowBackView = RoomSetting.allowBackView;
		allowGhostChat = RoomSetting.allowGameGhostChat;
		roundToWin = RoomSetting.roundToWin;
		MaxPlayer = RoomSetting.MaxPlayer;
		AutoTeamSwap = RoomSetting.autoSwapTeam;
		MapOption = RoomSetting.mapOption;

		bPassword = RoomSetting.bPassword;

		OutputLinks[0].bHasImpulse = true;
	}
	else
	{
		OutputLinks[1].bHasImpulse = true;
	}
}


defaultproperties
{
	ObjName="(Room) Get Setting"

	OutputLinks(0)=(LinkDesc="Successful")
	OutputLinks(1)=(LinkDesc="Failed")

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Map List Index",bWriteable=true,PropertyName=MapIndex))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="TK Level",bWriteable=true,PropertyName=tkLevel))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Auto Balance",bWriteable=true,PropertyName=autoBalance))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Allow Spectator",bWriteable=true,PropertyName=allowSpectator))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Allow Interrupt",bWriteable=true,PropertyName=allowInterrupt))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Allow BackView",bWriteable=true,PropertyName=allowBackView))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Allow GhostChat",bWriteable=true,PropertyName=allowGhostChat))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Round to Win",bWriteable=true,PropertyName=roundToWin))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Max Players",bWriteable=true,PropertyName=MaxPlayer))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Auto Team-swap",bWriteable=true,PropertyName=AutoTeamSwap))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Map Option",bWriteable=true,PropertyName=MapOption))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Password",bWriteable=true,PropertyName=bPassword))

	ObjClassVersion=8
}

