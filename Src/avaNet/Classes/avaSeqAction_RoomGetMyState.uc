class avaSeqAction_RoomGetMyState extends avaSeqAction;


var int MyTeam;
var int MyClass;
var int MyFace;
var int MyWeapon;
var int MySlot;


event Activated()
{
	if ( class'avaNet.avaNetRequest'.static.GetAvaNetRequest().GetCurrentEquipState(MyTeam, MyClass, MyFace, MyWeapon) )
	{
		MySlot = class'avaNet.avaNetRequest'.static.GetAvaNetRequest().GetMyRoomSlot();
		OutputLinks[0].bHasImpulse = true;
	}
	else
	{
		OutputLinks[1].bHasImpulse = true;
	}
}


defaultproperties
{
	ObjName="(Room) Get My State"

	OutputLinks.Empty
	OutputLinks(0)=(LinkDesc="Succeeded")
	OutputLinks(1)=(LinkDesc="Failed")

    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Team",bWriteable=true,PropertyName=MyTeam)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Int',LinkDesc="Slot",bWriteable=true,PropertyName=MySlot)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Int',LinkDesc="Class",bWriteable=true,PropertyName=MyClass)
	VariableLinks(3)=(ExpectedType=class'SeqVar_Int',LinkDesc="Face",bWriteable=true,PropertyName=MyFace)
	VariableLinks(4)=(ExpectedType=class'SeqVar_Int',LinkDesc="Weapon",bWriteable=true,PropertyName=MyWeapon)

	ObjClassVersion=2
}

