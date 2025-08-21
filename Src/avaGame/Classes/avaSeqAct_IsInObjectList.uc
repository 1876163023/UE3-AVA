/*
	ObjectList���� �ش��ϴ� ������Ʈ�� �ִ��� Ȯ���ϰ� �ش� Index�� �Ѱ��ش�.
	(Route�� ��쿡�� NavList���� �˻��Ѵ�)

	2007/12/21
*/
class avaSeqAct_IsInObjectList extends SequenceAction;

//! ã�� ������Ʈ.
var Object			TestObject;

//! Route���� ������Ʈ�� NavList�� ������ �ִ�.
var() Object		TargetRoute;

//! ������Ʈ ����Ʈ.
var() array<Object>	ObjList;

//! ã�� �ε���.
var int				Index;

event Activated()
{
	local Route R;
	local int i;

	Index = -1;

	// Route���� ���� ã�´�.
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

	// Route���� ��ã�Ҵٸ� ObjList���� �ٽ� ã�´�.
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