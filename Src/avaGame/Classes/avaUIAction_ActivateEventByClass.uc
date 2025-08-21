class avaUIAction_ActivateEventByClass extends UIAction;

var() class<UIEvent> EventToActivate;
var array<string>	StringValue;
var array<float>	FloatValue;
var array<int>		BoolValue;
var array<int>		IntValue;
var array<Object>  ObjectValue;

event Activated()
{
	Local int i, ObjectIndex, LinkedVarIndex, VarLinksIndex;
	Local UIScreenObject OwnerWidget;
	Local UIScene OwnerScene;
	Local GameUISceneClient SceneClient;
	Local UIEvent Event;
	Local SequenceVariable SeqVar;

	Local array<UIEvent> ActivatedEvents;
	Local array<UIObject>  ObjectsToFind;

	OwnerWidget = GetOwner();
	OwnerScene =  GetOwnerScene();

	if( OwnerWidget == None || OwnerScene == None || GameUISceneClient(OwnerScene.SceneClient) == None)
	{
		OutputLinks[0].bHasImpulse=true;
		return;
	}

	IntValue.Length = 0;
	FloatValue.Length = 0;
	StringValue.Length = 0;
	boolValue.Length = 0;
	ObjectValue.Length = 0;

	// PropertyName이 실제 연관된 LinkedVariable와 값이 같지 않은 경우가 생긴다. 미리 초기화하였음.
	for( i = 0 ; i < VariableLinks.Length ; i++ )
	{
		for(LinkedVarIndex = 0 ; LinkedVarIndex < VariableLinks[i].LinkedVariables.Length ; LinkedVarIndex++ )
		{
			SeqVar = VariableLinks[i].LinkedVariables[LinkedVarIndex];

			if( SeqVar_Int(SeqVar) != None )
				IntValue[IntValue.Length]= SeqVar_Int(SeqVar).IntValue;
			else if (SeqVar_Float(SeqVar) != None )
				FloatValue[FloatValue.Length] = SeqVar_Float(SeqVar).FloatValue;
			else if (SeqVar_String(SeqVar) != None )
				StringValue[StringValue.Length] = SeqVar_String(SeqVar).StrValue;
			else if( SeqVar_Bool(SeqVar) != None )
				BoolValue[BoolValue.Length] = SeqVar_Bool(SeqVar).bValue;
			else if( SeqVar_Object(SeqVar) != None)
				ObjectValue[ObjectValue.Length] = SeqVar_Object(SeqVar).GetObjectValue();
		}
	}

	if( EventToActivate == None )
	{
		OutputLinks[0].bHasImpulse=true;
		return;
	}

	// Activate할 Event들을 먼저 찾아 ActivatedEvent에 넣는다.
	SceneClient = GameUISceneClient(OwnerScene.SceneClient);
	for( i = 0 ; i < SceneClient.ActiveScenes.Length ; i++ )
	{
		SceneClient.ActiveScenes[i].FindEventsOfClass(EventToActivate, ActivatedEvents);
		ObjectsToFind = SceneClient.ActiveScenes[i].GetChildren(TRUE);
		for( ObjectIndex = 0 ; ObjectIndex < ObjectsToFind.Length ; ObjectIndex++ )
		{
			ObjectsToFind[ObjectIndex].FindEventsOfClass(EventToActivate, ActivatedEvents);
		}
	}

	// 실제로 활성화한다.
	for( i = 0 ; i < ActivatedEvents.Length ; i++ )
	{
		Event = ActivatedEvents[i];
		Event.ConditionalActivateUIEvent( INDEX_NONE , ActivatedEvents[i].GetOwner() , None, FALSE );
		for( VarLinksIndex = 0 ; VarLinksIndex < Event.VariableLinks.Length ; VarLinksIndex++ )
		{
			for( LinkedVarIndex = 0 ; LinkedVarIndex < Event.VariableLinks[VarLinksindex].LinkedVariables.Length ; LinkedVarIndex++ )
			{
				SeqVar = Event.VariableLinks[VarLinksindex].LinkedVariables[LinkedVarIndex];

				if( SeqVar_Int(SeqVar) != None && IntValue.Length > 0)
				{
					SeqVar_Int(SeqVar).IntValue= IntValue[ IntValue.Length - 1 ];
					IntValue.Remove( IntValue.Length - 1 , 1 );
				}
				else if (SeqVar_Float(SeqVar) != None && FloatValue.Length > 0)
				{
					SeqVar_Float(SeqVar).FloatValue = FloatValue[ FloatValue.Length - 1 ];
					FloatValue.Remove( FloatValue.Length - 1 ,1);
				}
				else if (SeqVar_String(SeqVar) != None && StringValue.Length > 0 )
				{
					SeqVar_String(SeqVar).StrValue = StringValue[StringValue.Length - 1];
					StringValue.Remove( StringValue.Length - 1, 1 );
				}
				else if( SeqVar_Bool(SeqVar) != None && BoolValue.Length > 0 )
				{
					SeqVar_Bool(SeqVar).bValue = BoolValue[ BoolValue.Length - 1 ];
					BoolValue.Remove( BoolValue.Length - 1, 1 );
				}
				else if( SeqVar_Object(SeqVar) != None && ObjectValue.Length > 0 )
				{
					SeqVar_Object(SeqVar).SetObjectValue(ObjectValue[ObjectValue.Length - 1]);
					ObjectValue.Remove( ObjectValue.Length - 1, 1 );
				}
			}
		}
	}


	// 활성화된 Event의 갯수를 반환
	for( i = 0 ; i < VariableLinks.Length ; i++ )
	{
		if( VariableLinks[i].LinkDesc == "NumActivated" )
		{
			for(LinkedVarIndex = 0 ; LinkedVarIndex < VariableLinks[i].LinkedVariables.Length ; LinkedVarIndex++ )
			{
				SeqVar_Int(VariableLinks[i].LinkedVariables[LinkedVarIndex]).IntValue = ActivatedEvents.Length;
			}
		}
	}

	if( ActivatedEvents.Length == 0 )
		OutputLinks[0].bHasImpulse = true;
	else
		OutputLinks[1].bHasImpulse = true;
}

defaultproperties
{
	ObjName="Activate UIEvent(Class)"
	ObjCategory="UI"

	OutputLinks.Empty
	OutputLinks(0)=(LinkDesc="Failed")
	OutputLinks(1)=(LinkDesc="Success")

	bAutoActivateOutputLinks=false

	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="EventClass",PropertyName="EventToActivate",bHidden=false))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="StrParm",PropertyName="StringValue",bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="IntParm",PropertyName="IntValue",bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="FloatParm",PropertyName="FloatValue",bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="BoolParm",PropertyName="BoolValue",bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="ObjectParm",bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="NumActivated"))
}