class avaSeqAct_ParseIntoArray extends UIAction
	native;


var() string SrcStr;
var() string Delimeter;
var() string StrParm[8];
var() editconst int StrParmCount;

cpptext
{
	void Activated();
}


defaultproperties
{
	ObjCategory="String Manipulation"
	ObjName="ParseIntoArray"
	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="SourceStr",PropertyName=SrcStr,bWriteable=false,MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Delim",PropertyName=Delimeter,bWriteable=false,MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Parm1"))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Parm2",bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Parm3",bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Parm4",bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Parm5",bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Parm6",bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Parm7",bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Parm8",bHidden=true))

	StrParmCount = 8
	Delimeter="|"
}