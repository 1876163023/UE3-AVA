class avaSeqAct_CharacterLookAt extends SequenceAction;

var() avaCharacter		Character;
var() Vector			LookAt;
var() bool				bEnable;

event Activated()
{
	if ( Character == None )	return;
	Character.LookAt( LookAt, bEnable );
}

defaultproperties
{
	ObjCategory="avaPawn"
	ObjName="Look At"
	bCallHandler=false

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Character",PropertyName=Character,MinVars=1 )
	VariableLinks(1)=(ExpectedType=class'SeqVar_Vector',LinkDesc="LookAt",PropertyName=LookAt,MinVars=1 )
	VariableLinks(2)=(ExpectedType=class'SeqVar_Bool',LinkDesc="bEnable",PropertyName=bEnable,MinVars=1 )
}