/*
	Object List�� ������ ������ �׼�.

	2007/01/30	����
*/
class avaUIAction_GetCountInList extends UIAction;

//	var array<Object>		ObjList;	//!< ������Ʈ����Ʈ.
var int					Count;		//!< ������Ʈ�� ����.

event Activated()
{
	local SeqVar_ObjectList		VarObjList;

	foreach LinkedVariables(class'SeqVar_ObjectList',VarObjList,"ObjectList")
	{
		`log("Find it "@VarObjList.ObjList.Length);
		Count = VarObjList.ObjList.Length;
		break;
	}

	`log("### avaUIAction_GetCountInList - Count : " @Count @" ###");
}

defaultproperties
{
	ObjName="ava Get Count in List"
	ObjCategory="Object List"
	bCallHandler=false

	VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_ObjectList',LinkDesc="ObjectList",MaxVars=1)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Int',LinkDesc="Count",PropertyName=Count,MaxVars=1,bWriteable=true)
}
