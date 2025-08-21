//=============================================================================
//  avaSeqAct_GetObjectInList
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
// 
//	2006/04/24 by OZ
//		Ojbect List 와 Index 를 Input 으로 받아서 해당하는 Index 의 Object 를 돌려준다.
//=============================================================================

class avaSeqAct_GetObjectInList extends SequenceAction;

var() int			Index;
var() array<Object>	ObjList;
var	  Object		Result;	

event Activated()
{
	if ( Index < ObjList.Length && Index >= 0 )
	{
		Result = ObjList[Index];
	}
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
	ObjName="Get Object in List"
	ObjCategory="Object List"
	ObjColor=(R=255,G=0,B=255,A=255)
	bCallHandler=false

	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Index",PropertyName=Index,MaxVars=1)
	VariableLinks(1)=(ExpectedType=class'SeqVar_ObjectList',LinkDesc="ObjectList",PropertyName=ObjList,MaxVars=1)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Object',LinkDesc="Object",PropertyName=Result,MaxVars=1,bWriteable=true)
}