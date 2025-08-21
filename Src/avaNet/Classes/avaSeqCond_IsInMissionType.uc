class avaSeqCond_IsInMissionType extends avaSeqCondition;

var() ENetMissionType MissionTypeToCheck;

event Activated()
{
	Local int CurrentMissionType;
	CurrentMissionType = class'avaNetHandler'.static.GetAvaNetHandler().GetCurrentMapMissionType();

	BoolResult = (MissionTypeToCheck == CurrentMissionType);
	OutputLinks[ BoolResult ? 0 : 1].bHasImpulse = true;
}

defaultproperties
{
	ObjName = "Is In (Target) Mission"
	OutputLinks[0]=(LinkDesc="True")
	OutputLinks[1]=(LinkDesc="False")
}