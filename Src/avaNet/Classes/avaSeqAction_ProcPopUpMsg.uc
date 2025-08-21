class avaSeqAction_ProcPopUpMsg extends avaSeqAction;



event Activated()
{
	//class'avaNetHandler'.static.GetAvaNetHandler().ProcPopUpMsg();
}


defaultproperties
{
	ObjName="Proc Pop Up Message"

	InputLinks(0)=(LinkDesc="In")
	OutputLinks(0)=(LinkDesc="Output")

	bAutoActivateOutputLinks=false

    VariableLinks.Empty

	ObjClassVersion=3
}

