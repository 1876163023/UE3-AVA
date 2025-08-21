class avaSeqAct_Counter extends SequenceAction;

var() int Amount;
var() int MaxValue, MinValue, DefaultValue;
var() bool Wrap;
var int CurrentValue;

event Activated()
{
	local int N;

	N = MaxValue - MinValue + 1;

	if (InputLinks[0].bHasImpulse)		
		CurrentValue = DefaultValue;
	else if (InputLinks[1].bHasImpulse)	
		CurrentValue += Amount;
	else if (InputLinks[2].bHasImpulse)	
		CurrentValue -= Amount;

//	`log( "Counter "@Amount@CurrentValue@MaxValue@MinValue@N );

	if (Wrap)
	{		
		CurrentValue = ((CurrentValue - MinValue + N) % N) + MinValue;
	}
	else
	{
		CurrentValue = Clamp( CurrentValue, MinValue, MaxValue );
	}

//	`log( "#Couner "@Amount@CurrentValue@MaxValue@MinValue );

	UpdateTarget();
}

function UpdateTarget()
{
	local SeqVar_Int v;

	foreach LinkedVariables( class'SeqVar_Int', v, "Target" )
	{
		v.IntValue = CurrentValue;
	}	
}

defaultproperties
{
	ObjCategory="Add Value"
	ObjName="Counter"
	bCallHandler=false

	Amount=1
	MaxValue=1
	MinValue=0
	DefaultValue=0
	Wrap=True

	InputLinks(0)=(LinkDesc="Reset")
	InputLinks(1)=(LinkDesc="Inc")
	InputLinks(2)=(LinkDesc="Dec")

    VariableLinks.Empty	
	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Target",PropertyName=Current, MinVars=1,bWriteable=true )
	VariableLinks(1)=(ExpectedType=class'SeqVar_Int',LinkDesc="Amount",  PropertyName=Amount, MinVars=0, MaxVars=100 )
	VariableLinks(2)=(ExpectedType=class'SeqVar_Int',LinkDesc="Min",  PropertyName=MinValue, MinVars=0, MaxVars=100 )
	VariableLinks(3)=(ExpectedType=class'SeqVar_Int',LinkDesc="Max",  PropertyName=MaxValue, MinVars=0, MaxVars=100 )
	VariableLinks(4)=(ExpectedType=class'SeqVar_Int',LinkDesc="Default",  PropertyName=DefaultValue, MinVars=0, MaxVars=100 )
	VariableLinks(5)=(ExpectedType=class'SeqVar_Bool',LinkDesc="Wrap",  PropertyName=Wrap)
}

