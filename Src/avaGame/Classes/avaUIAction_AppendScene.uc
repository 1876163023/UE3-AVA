class avaUIAction_AppendScene extends UIAction;

var() UIScene							UIScene;
var() array<avaNetEventParam>		EventParams;
var() EScenePriorityType				Priority;
var() class<GameInfo>					BaseGameClasses;

event Activated()
{
//	class'avaNetHandler'.static.GetAvaNetHandler().OpenSceneManaged();
}

defaultproperties
{
	ObjCategory="Scene"
	ObjName="Append Scene"
}