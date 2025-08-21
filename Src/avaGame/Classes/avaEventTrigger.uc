class avaEventTrigger extends object
	native;


static function ActivateEventByName( name EventName , optional string StrParam, optional int IntParam, optional bool BoolParam, optional float FloatParam , optional Object ObjParam)
{
	Local PlayerController PC;
	Local UIInteraction UIController;
	Local array<UIEvent> UIEventsToActivate;
//	Local array<SequenceEvent> EventsToActivate;
	Local int ChildIndex, i;
	Local array<UIObject> Child;	

	PC = GetLocalPC();
	if( PC == None ) 
		return;

	UIController = LocalPlayer(PC.Player).ViewportClient.UIController;
	if( UIController == None || UIController.SceneClient == None )
		return;

	//각각의 Active Scene에 대해
	for( i = 0 ; i < UIController.SceneClient.ActiveScenes.Length ; i++ )
	{
		if( UIController.SceneClient.ActiveScenes[i] != None )
		{
			// 자신(Scene)에서 먼저 Event를 찾고
			UIController.SceneClient.ActiveScenes[i].FindEventsOfClass( class'avaUIEvent_UIRemoteEvent', UIEventsToActivate);
			// 자식들 재귀적으로 찾고
			Child = UIController.SceneClient.ActiveScenes[i].GetChildren(TRUE);
			// 찾은 자식들에 대해 EventClass를 찾는다.
			for( ChildIndex = 0 ; ChildIndex < Child.Length ; ChildIndex++ )
				if( Child[ChildIndex] != None )
					Child[ChildIndex].FindEventsOfClass(class'avaUIEvent_UIRemoteEvent', UIEventsToActivate);
		}
	}

	for( i = 0 ; i < UIEventsToActivate.Length ; i++ )
	{
		if( avaUIEvent_UIRemoteEvent(UIEventsToActivate[i]) == None )
			continue;

		if( avaUIEvent_UIRemoteEvent(UIEventsToActivate[i]).EventName == EventName)
		{
			SetEventParam(UIEventsToActivate[i], StrParam, IntParam, BoolParam, FloatParam, ObjParam);
			UIEventsToActivate[i].ConditionalActivateUIEvent( -1 , UIEventsToActivate[i].GetOwner(), None, FALSE);
		}
	}

	if ( UIEventsToActivate.Length == 0 )
		`log("Failed avaEventTrigger.ActivateEventByName" @EventName);
}

static function ActivateNetEventByClass( class<SequenceEvent> EventClass , string Param1, string Param2, int Param3, int Param4)
{
	Local PlayerController PC;
	Local UIInteraction UIController;
	Local array<UIEvent> UIEventsToActivate;
//	Local array<SequenceEvent> EventsToActivate;
	Local int ChildIndex, i;
	Local array<UIObject> Child;
	Local class<UIEvent>	UIEventClass;
	
	UIEventClass = class<UIEvent>(EventClass);
	if( UIEventClass != None)
	{
		PC = GetLocalPC();
		if( PC == None )
			return;

		UIController = LocalPlayer(PC.Player).ViewportClient.UIController;
		if( UIController == None || UIController.SceneClient == None )
			return;

		//각각의 Active Scene에 대해
		for( i = 0 ; i < UIController.SceneClient.ActiveScenes.Length ; i++ )
		{
			if( UIController.SceneClient.ActiveScenes[i] != None )
			{
				// 자신(Scene)에서 먼저 Event를 찾고
				UIController.SceneClient.ActiveScenes[i].FindEventsOfClass( UIEventClass, UIEventsToActivate);
				// 자식들 재귀적으로 찾고
				Child = UIController.SceneClient.ActiveScenes[i].GetChildren(TRUE);
				// 찾은 자식들에 대해 EventClass를 찾는다.
				for( ChildIndex = 0 ; ChildIndex < Child.Length ; ChildIndex++ )
					if( Child[ChildIndex] != None )
						Child[ChildIndex].FindEventsOfClass(UIEventClass, UIEventsToActivate);
			}
		}

		for( i = 0 ; i < UIEventsToActivate.Length ; i++ )
		{
			if( UIEventsToActivate[i] == None )
				continue;

			SetNetEventParam(UIEventsToActivate[i], Param1, Param2, Param3, Param4);
			UIEventsToActivate[i].ConditionalActivateUIEvent( -1 , UIEventsToActivate[i].GetOwner(), None, FALSE);
		}
	}
	else
	{
		//@TODO 월드 이벤트 발생
	}
}

static function SetNetEventParam(SequenceEvent Event, string Param1, string Param2, int Param3, int Param4)
{
	local SeqVarLink VarLink;
	local SequenceVariable SeqVar;
	local int VarLinksIndex, LinkedVarIndex;
	local string StrVar;
	local int IntVar;

	for( VarLinksIndex = 0 ; VarLinksIndex < Event.VariableLinks.Length ; VarLinksIndex++ )
	{
		VarLink = Event.VariableLinks[VarLinksindex];

		if ( VarLink.LinkDesc == "Param1" || VarLink.LinkDesc == "Param2" )
		{
			if ( VarLink.LinkDesc == "Param1" )
				StrVar = Param1;
			else
				StrVar = Param2;

			for( LinkedVarIndex = 0 ; LinkedVarIndex < VarLink.LinkedVariables.Length ; LinkedVarIndex++ )
			{
				SeqVar = VarLink.LinkedVariables[LinkedVarIndex];
				if( SeqVar_String(SeqVar) != None )
					SeqVar_String(SeqVar).StrValue = StrVar;
			}
		}
		else if ( VarLink.LinkDesc == "Param3" || VarLink.LinkDesc == "Param4" )
		{
			if ( VarLink.LinkDesc == "Param3" )
				IntVar = Param3;
			else
				IntVar = Param4;

			for( LinkedVarIndex = 0 ; LinkedVarIndex < VarLink.LinkedVariables.Length ; LinkedVarIndex++ )
			{
				SeqVar = VarLink.LinkedVariables[LinkedVarIndex];
				if ( SeqVar_Int(SeqVar) != None )
					SeqVar_Int(SeqVar).IntValue = IntVar;
			}
		}
	}
}

static function ActivateEventByClass( class<SequenceEvent> EventClass , optional string StrParam, optional int IntParam, optional bool BoolParam, optional float FloatParam , optional Object ObjParam)
{
	Local PlayerController PC;
	Local UIInteraction UIController;
	Local array<UIEvent> UIEventsToActivate;
//	Local array<SequenceEvent> EventsToActivate;
	Local int ChildIndex, i;
	Local array<UIObject> Child;
	Local class<UIEvent>	UIEventClass;
	

	UIEventClass = class<UIEvent>(EventClass);
	if( UIEventClass != None)
	{
		PC = GetLocalPC();
		if( PC == None )
			return;

		UIController = LocalPlayer(PC.Player).ViewportClient.UIController;
		if( UIController == None || UIController.SceneClient == None )
			return;

		//각각의 Active Scene에 대해
		for( i = 0 ; i < UIController.SceneClient.ActiveScenes.Length ; i++ )
		{
			if( UIController.SceneClient.ActiveScenes[i] != None )
			{
				// 자신(Scene)에서 먼저 Event를 찾고
				UIController.SceneClient.ActiveScenes[i].FindEventsOfClass( UIEventClass, UIEventsToActivate);
				// 자식들 재귀적으로 찾고
				Child = UIController.SceneClient.ActiveScenes[i].GetChildren(TRUE);
				// 찾은 자식들에 대해 EventClass를 찾는다.
				for( ChildIndex = 0 ; ChildIndex < Child.Length ; ChildIndex++ )
					if( Child[ChildIndex] != None )
						Child[ChildIndex].FindEventsOfClass(UIEventClass, UIEventsToActivate);
			}
		}

		for( i = 0 ; i < UIEventsToActivate.Length ; i++ )
		{
			if( UIEventsToActivate[i] == None )
				continue;

			SetEventParam(UIEventsToActivate[i], StrParam, IntParam, BoolParam, FloatParam, ObjParam);
			UIEventsToActivate[i].ConditionalActivateUIEvent( -1 , UIEventsToActivate[i].GetOwner(), None, FALSE);
		}
	}
	else
	{
		//@TODO 월드 이벤트 발생
	}
}

static function SetEventParam(SequenceEvent Event, string StrParam, int IntParam, bool BoolParam, float FloatParam, Object ObjParam)
{
	Local SeqVarLink VarLink;
	Local SequenceVariable SeqVar;
	Local int VarLinksIndex, LinkedVarIndex;

	for( VarLinksIndex = 0 ; VarLinksIndex < Event.VariableLinks.Length ; VarLinksIndex++ )
	{
		VarLink = Event.VariableLinks[VarLinksindex];
		for( LinkedVarIndex = 0 ; LinkedVarIndex < VarLink.LinkedVariables.Length ; LinkedVarIndex++ )
		{
			SeqVar = VarLink.LinkedVariables[LinkedVarIndex];
			if( SeqVar_String(SeqVar) != None )
				SeqVar_String(SeqVar).StrValue = StrParam;
			else if ( SeqVar_Int(SeqVar) != None )
				SeqVar_Int(SeqVar).IntValue = IntParam;
			else if ( SeqVar_Bool(SeqVar) != None )
				SeqVar_Bool(SeqVar).bValue = int(BoolParam);
			else if ( SeqVar_Float(SeqVar) != None )
				SeqVar_Float(SeqVar).FloatValue = FloatParam;
			else if ( SeqVar_Object(SeqVar) != None )
				SeqVar_Object(SeqVar).SetObjectValue(ObjParam);
		}
	}
}

static function ActivateUIRemoveEvent( name EventName, string StrParam, int IntParam, bool BoolParam, float FloatParam)
{
}


native static function WorldInfo GetWorldInfo();
static function PlayerController GetLocalPC()
{
	Local WorldInfo WorldInfo;
	Local PlayerController PC, LocalPC;

	WorldInfo = GetWorldInfo();
	if( WorldInfo == None )
		return None;

	foreach WorldInfo.LocalPlayerControllers(PC)
	{
		LocalPC = PC;
	}
	return LocalPC;
}