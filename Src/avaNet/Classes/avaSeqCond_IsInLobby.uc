class avaSeqCond_IsInLobby extends avaSeqCondition;

var() EChannelFlag TargetChannelFlag;

event Activated()
{
	Local int ChannelFlag;
	ChannelFlag = class'avaNetHandler'.static.GetAvaNetHandler().GetCurrentChannelFlag();

	BoolResult = (TargetChannelFlag == ChannelFlag);
	OutputLinks[ BoolResult ? 0 : 1].bHasImpulse = true;
}

defaultproperties
{
	ObjName = "Is In (Target) Lobby"
	OutputLinks[0]=(LinkDesc="True")
	OutputLinks[1]=(LinkDesc="False")
}