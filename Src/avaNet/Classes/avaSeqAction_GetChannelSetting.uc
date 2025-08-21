class avaSeqAction_GetChannelSetting extends avaSeqAction;

var() EChannelSetting Setting;
var bool BoolResult;
var int IntResult;

event Activated()
{
	Local int Result;
	Result = class'avaNetHandler'.static.GetAvaNetHandler().GetChannelSetting(Setting);

	if (Result == -1)
	{
		OutputLinks[1].bHasImpulse = true;
		return;
	}

	IntResult = Result;
	BoolResult = bool(Result);
	OutputLinks[0].bHasImpulse = true;
}

defaultproperties
{
	ObjName="(Channel) Get Setting"

	//InputLinks(0)=(LinkDesc="Get")
	//InputLinks(1)=(LinkDesc="Get Current")

	OutputLinks(0)=(LinkDesc="Out")
	OutputLinks(1)=(LinkDesc="Failed")

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Setting",PropertyName=Setting))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Bool Result",bWriteable=true,PropertyName=BoolResult))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Int Result",bWriteable=true,PropertyName=IntResult))

	ObjClassVersion=1
}

