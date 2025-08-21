class avaUIAction_OpenSceneManaged extends UIAction_Scene
	native;

/** Output variable for the scene that was opened. */
var() EScenePriorityType		Priority;
var() class<GameInfo>			BaseGameClass;
var() array<avaNetEventParam>	EventParams;
var() float						LifeTime;
var() array<UIScene>			ExclusiveScenes;
var() array<name>				ExclusiveSceneTags;

var() string					SceneName;

// @TODO : Event를 더 추가할 수 있도록 VariableLink를 에디터에서 추가할 수 있도록 하면 좋겠음 ( 유사한 예제 - SeqAct_Switch, SeqAct_Interp)
// 불행히도 현재는 한개의 이벤트만 발생할 수 있다.
var() class<avaUIEvent_PopupBase>		EventClass;
var() string					StrParam;
var() int						IntParam;
var() float						FloatParam;
var() bool						BoolParam;
var() object					ObjParam;

event Activated()
{
	Local int			Length;
	Local array<Name>	ExclSceneTags;
	Local UIScene		UIScene;
	local name			SceneTag;

	if( EventClass != none )
	{
		Length = EventParams.Length;
		EventParams.Add(1);

		EventParams[Length].EventClass = EventClass;
		EventParams[Length].StrParam = StrParam;
		EventParams[Length].BoolParam = BoolParam;
		EventParams[Length].IntParam = IntParam;
		EventParams[Length].FloatParam = FloatParam;
		EventParams[Length].ObjParam = ObjParam;
	}

	foreach ExclusiveScenes(UIScene)
	{
		ExclSceneTags.AddItem( UIScene.SceneTag );
	}

	foreach ExclusiveSceneTags(SceneTag)
	{
		ExclSceneTags.AddItem( SceneTag );
	}	

	if( Len(SceneName) == 0 || SceneName ~= "none" )
		class'avaNetHandler'.static.GetAvaNetHandler().OpenSceneManaged(Scene, BaseGameClass, Priority, EventParams, ExclSceneTags, LifeTime);
	else
		class'avaNetHandler'.static.GetAvaNetHandler().OpenSceneManagedName( SceneName, BaseGameClass, Priority, EventParams, ExclSceneTags, LifeTime);

	//! GlobalScene에서 호출했더니 계속 쌓여서 여러개가 가는 것 같은??
	EventParams.Length = 0;

	OutputLinks[0].bHasImpulse = true;
}

cpptext
{
	/* === UObject interface === */
	/**
	 * Called after this object has been de-serialized from disk.
	 *
	 * This version converts the deprecated PRIVATE_DisallowReparenting flag to PRIVATE_EditorNoReparent, if set.
	 */
	virtual void PostLoad();
}

defaultproperties
{
	ObjName = "(AVA) Open Scene"
	ObjCategory = "UI"

	Priority=EScenePrior_UIScene_Normal
	BaseGameClass=class'GameInfo'

	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Scene Name",PropertyName=SceneName))

	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="Event Class",PropertyName=EventClass,bHidden=true,MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Str",PropertyName=StrParam,bHidden=true,MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Int",PropertyName=IntParam,bHidden=true,MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Float",PropertyName=FloatParam,bHidden=true,MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Bool",PropertyName=BoolParam,bHidden=true,MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="Obj",PropertyName=ObjParam,bHidden=true,MaxVars=1))

	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Life Time",PropertyName=LifeTime,bHidden=true,MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="Excl. Scenes",PropertyName=ExclusiveScenes,bHidden=true))

	ObjClassVersion = 2
}