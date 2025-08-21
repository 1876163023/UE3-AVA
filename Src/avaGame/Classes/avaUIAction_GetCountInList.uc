/*
	Object List의 개수를 얻어오는 액션.

	2007/01/30	고광록
*/
class avaUIAction_GetCountInList extends UIAction;

//	var array<Object>		ObjList;	//!< 오브젝트리스트.
var int					Count;		//!< 오브젝트의 개수.

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
