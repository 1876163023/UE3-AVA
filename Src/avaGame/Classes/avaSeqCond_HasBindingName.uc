/**
 * 옵션창에서 키를 설정할때 이미 설정된 키인지 아닌지 체크
 */
class avaSeqCond_HasBindingName extends SequenceCondition;

var() string	BindName;
var() int		Slot;

event Activated()
{
	local PlayerController		PC;
	local WorldInfo				WorldInfo;
	Local bool					bFound;
	Local KeyBind				KeyBind;

	WorldInfo = GetWorldinfo();
	if( WorldInfo != None )
	{
		foreach WorldInfo.LocalPlayercontrollers(PC)
		{
			if( PC.PlayerInput != None )
				bFound = bFound || PC.PlayerInput.HasBindingName(BindName, KeyBind);
		}
	}

	Slot = KeyBind.Slot;
	OutputLinks[ bFound ? 0 : 1 ].bHasImpulse = true;
}

defaultproperties
{
	ObjName="Has BindingName"

	OutputLinks(0)=(LinkDesc="Has")
	OutputLinks(1)=(LinkDesc="Not")

	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Bind Name",PropertyName=BindName))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Slot Num",PropertyName=Slot))

	ObjClassVersion = 1
}