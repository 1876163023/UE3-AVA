/*
	ObjectList에서 해당하는 오브젝트가 있는지 확인하고 해당 Index를 넘겨준다.
	(Route인 경우에는 NavList에서 검색한다)

	2007/12/21
*/
class avaSeqAct_IsInObjectList extends SequenceAction;

//! 찾을 오브젝트.
var Object			TestObject;

//! Route같은 오브젝트는 NavList를 가지고 있다.
var() Object		TargetRoute;

//! 오브젝트 리스트.
var() array<Object>	ObjList;

//! 찾은 인덱스.
var int				Index;

event Activated()
{
	local Route R;
	local int i;

	Index = -1;

	// Route에서 먼저 찾는다.
	if ( TargetRoute != None && Route(TargetRoute) != None  )
	{
		R = Route(TargetRoute);
		
		for ( i = 0; i < R.NavList.Length; i++ )
			if ( R.NavList[i].Nav == TestObject )
			{
				Index = i;
				break;
			}
	}

	// Route에서 못찾았다면 ObjList에서 다시 찾는다.
	if ( Index == -1 && ObjList.Length > 0 )
	{
		for ( i = 0; i < ObjList.Length; i++ )
			if ( ObjList[i] == TestObject )
			{
				Index = i;
				break;
			}
	}

	OutputLinks[Index >= 0 ? 0 : 1].bHasImpulse = TRUE;
}

/**
 * Determines whether this class should be displayed in the list of available ops in the UI's kismet editor.
 *
 * @param	TargetObject	the widget that this SequenceObject would be attached to.
 *
 * @return	TRUE if this sequence object should be available for use in the UI kismet editor
 */
event bool IsValidUISequenceObject( optional UIScreenObject TargetObject )
{
	return true;
}

defaultproperties
{
	ObjName="IsIn ObjectList (ava)"
	ObjCategory="Object List"
	ObjColor=(R=255,G=0,B=255,A=255)

    // all of the inputs / functionality this Action can do
	InputLinks(0)=(LinkDesc="Test if in List")

	VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="ObjectToTest",PropertyName=TestObject,MinVars=1,MaxVars=1)
	VariableLinks(1)=(ExpectedType=class'SeqVar_ObjectList',LinkDesc="ObjectListVar",PropertyName=TargetRoute,bWriteable=true)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Object',LinkDesc="Route",PropertyName=ObjList,bWriteable=true)
	VariableLinks(3)=(ExpectedType=class'SeqVar_Int',LinkDesc="Index",PropertyName=Index,bWriteable=true)

    // outputs that are set to hot depending
	OutputLinks(0)=(LinkDesc="In List")
	OutputLinks(1)=(LinkDesc="Not in List")
}