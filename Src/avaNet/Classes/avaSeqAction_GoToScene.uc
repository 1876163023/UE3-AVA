class avaSeqAction_GoToScene extends avaSeqAction;


//var() avaNetEntryGame.EScenes Scene;
var() string Scene;
//var() array<string> Parameters;

event Activated()
{
	local avaNetEntryGame Game;

	Game = avaNetEntryGame(GetWorldInfo().Game);
	if (Game != None)
	{
		if (InputLinks[0].bHasImpulse)
		{
			Game.GotoScene(Scene);
		}
		else if (InputLinks[1].bHasImpulse)
		{
			Game.GotoNextScene();
		}
	}
	else
	{
		`log( "Error: could not find avaNetEntryGame" );
	}
}


defaultproperties
{
	ObjName="Go To Scene"

	InputLinks(0)=(LinkDesc="In")
	InputLinks(1)=(LinkDesc="Next")

	OutputLinks(0)=(LinkDesc="Output")

    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_String',LinkDesc="Scene",PropertyName=Scene,MaxVars=1,bHidden=true)
	//VariableLinks(0)=(ExpectedType=class'SeqVar_String',LinkDesc="Parameter",PropertyName=Parameters,bHidden=true)

	ObjClassVersion=2
}

