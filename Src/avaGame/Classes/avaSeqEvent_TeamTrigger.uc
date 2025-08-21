class avaSeqEvent_TeamTrigger extends SequenceEvent;

var array<Object>		UserList;		//!< Modifier리스트.

function Trigger(name EventType, optional array<Pawn> InUserList )
{
	local array<int>		ActivateIndices;
	local int				i;
	local SeqVar_ObjectList	VarObjList;
	local SeqVar_Int		VarObjCount;
	local Actor				activator;

	// WarmUp Round 중에는 TeamTrigger Event 를 Activate 하지 않는다.
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
		if ( InUserList.length > 0 )	activator	=	InUserList[0];
		else							activator	=	None;

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
			foreach LinkedVariables(class'SeqVar_int',VarObjCount,"UserCount")
			{
				VarObjCount.IntValue = InUserList.length;
			}
		}
	}
}

defaultproperties
{
	ObjClassVersion		=	1
	ObjName				=	"Team Trigger"
	ObjCategory			=	"Objective"
	bPlayerOnly			=	false
	MaxTriggerCount		=	0

	OutputLinks[0]		=	(LinkDesc="Activate")
	OutputLinks[1]		=	(LinkDesc="Deactivate")
	OutputLinks[2]		=	(LinkDesc="CountChange")

	VariableLinks.Empty
	VariableLinks(0)	=	(ExpectedType=class'SeqVar_ObjectList',LinkDesc="UserList",PropertyName=UserList,MaxVars=1,bWriteable=true)
	VariableLinks(1)	=	(ExpectedType=class'SeqVar_int',LinkDesc="UserCount",bWriteable=true)
}
