class avaSeqAction_RoomSetting extends avaSeqAction;



var() int MapIndex;
var() int tkLevel;
var() bool autoBalance;
var() bool allowSpectator;
var() bool allowInterrupt;
var() bool allowBackView;
var() bool allowGhostChat;
var() int roundToWin;
var() int MaxPlayer;
var() bool AutoTeamSwap;
var() int MapOption;


event Activated()
{
	local avaNetRequest.avaRoomSetting RoomSetting;
	local int MapID;

	if (MapIndex < class'avaNetRequest'.static.GetAvaNetRequest().MapList.Length)
	{
		MapID = class'avaNetRequest'.static.GetAvaNetRequest().MapList[MapIndex].MapID;
	}
	else
	{
		return;
	}

	RoomSetting.idMap = MapID;
	RoomSetting.tkLevel = tkLevel;
	RoomSetting.autoBalance = autoBalance;
	RoomSetting.allowSpectator = allowSpectator;
	RoomSetting.allowInterrupt = allowInterrupt;
	RoomSetting.allowBackView = allowBackView;
	RoomSetting.allowGameGhostChat = allowGhostChat;
	RoomSetting.roundToWin = roundToWin;
	RoomSetting.MaxPlayer = MaxPlayer;
	RoomSetting.autoSwapTeam = AutoTeamSwap;
	RoomSetting.mapOption = MapOption;

	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().RoomSetting(RoomSetting);
}


defaultproperties
{
	ObjName="(Room) Change Setting"

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Map List Index",PropertyName=MapIndex))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="TK Level",PropertyName=tkLevel))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Auto Balance",PropertyName=autoBalance))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Allow Spectator",PropertyName=allowSpectator))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Allow Interrupt",PropertyName=allowInterrupt))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Allow BackView",PropertyName=allowBackView))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Allow GhostChat",PropertyName=allowGhostChat))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Round to Win",PropertyName=roundToWin))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Max Players",PropertyName=MaxPlayer))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Auto Team-swap",PropertyName=AutoTeamSwap))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Map Option",PropertyName=MapOption))

	ObjClassVersion=7
}

