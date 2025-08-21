class avaSeqEvent_Use extends SequenceEvent;

var array<Object>		UserList;		//!< Modifier리스트.

event Activated()
{
	if (InputLinks[0].bHasImpulse)		// Enable This Event
		bEnabled = true;
	else if (InputLinks[1].bHasImpulse)	// Disable This Event
		bEnabled = false;
}

/** attempts to activate the event with the appropriate output for the given event type and instigator */
function Trigger(name EventType, optional array<Controller> InUserList, optional int Progress )
{
	local array<int> ActivateIndices;
	local int i;
	local SeqVar_ObjectList			VarObjList;
	local SeqVar_Int				var_Progress;
	local SeqVar_Int				var_UserCount;
	local Actor						activator;

	// WarmUp Round 중에는 Use Event 를 Activate 하지 않는다.
	if ( avaGameReplicationInfo( GetWorldInfo().GRI ).bWarmupRound == true )	return;

	for (i = 0; i < OutputLinks.length; i++)
	{
		if (EventType == name(OutputLinks[i].LinkDesc))
		{
			ActivateIndices[ActivateIndices.length] = i;
		}
	}
	
	if (ActivateIndices.length > 0)
	{
		if ( InUserList.length > 0 )
			activator = InUserList[0];
		else
			activator = None;
		if (CheckActivate(Originator, activator, false, ActivateIndices))
		{
			foreach LinkedVariables(class'SeqVar_ObjectList',VarObjList,"UserList")
			{
				VarObjList.ObjList.Length = InUserList.Length;
				for ( i = 0 ; i < InUserList.length ; ++ i )
				{
					VarObjList.ObjList[i] = InUserList[i];
				}
			}
			foreach LinkedVariables(class'SeqVar_int',var_Progress,"Progress")
			{
				var_Progress.IntValue = Progress;
			}

			foreach LinkedVariables(class'SeqVar_int',var_UserCount,"UserCount")
			{
				var_UserCount.IntValue = InUserList.length;
			}
		}
	}
}

defaultproperties
{
	ObjClassVersion=3
	ObjName="Use"
	ObjCategory="Objective"
	bPlayerOnly=false
	MaxTriggerCount=0

	InputLinks(0)=(LinkDesc="Enable")
	InputLinks(1)=(LinkDesc="Disable")

	OutputLinks[0]=(LinkDesc="Start")	
	OutputLinks[1]=(LinkDesc="Cancel")	
	OutputLinks[2]=(LinkDesc="Complete")	
	OutputLinks[3]=(LinkDesc="Progress")

	VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_ObjectList',LinkDesc="UserList",PropertyName=UserList,MaxVars=1,bWriteable=true)
	VariableLinks(1)=(ExpectedType=class'SeqVar_int',LinkDesc="Progress",bWriteable=true)
	VariableLinks(2)=(ExpectedType=class'SeqVar_int',LinkDesc="UserCount",MaxVars=1,bWriteable=true)
}
