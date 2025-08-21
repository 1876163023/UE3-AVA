class avaSeqAct_Arith extends SequenceAction;

enum ArithmeticOperator
{
	AO_Add,
	AO_Sub,
	AO_Mul,
	AO_Div,
	AO_Mod,
};

var() ArithmeticOperator Operator;

event Activated()
{	
	local int IR, IA, IB;
	local float FR, FA, FB;
	local SeqVar_Int IV;
	local SeqVar_Float FV;

	foreach LinkedVariables( class'SeqVar_Int', iv, "First Opererand" )
	{
		IA = IV.IntValue;
		FA = IA;				
	}

	foreach LinkedVariables( class'SeqVar_Float', fv, "First Opererand" )
	{
		FA = FV.FloatValue;
		IA = FA;				
	}
	
	foreach LinkedVariables( class'SeqVar_Int', iv, "Second Opererand" )
	{
		IB = IV.IntValue;
		FB = IB;				
	}

	foreach LinkedVariables( class'SeqVar_Float', fv, "Second Opererand" )
	{
		FB = FV.FloatValue;
		IB = FB;			
	}

	switch (Operator)
	{
	case AO_Add :
		IR = IA + IB; FR = FA + FB; break;
	case AO_Sub :
		IR = IA - IB; FR = FA - FB; break;
	case AO_Mul :
		IR = IA * IB; FR = FA * FB; break;
	case AO_Div :
		IR = IA / IB; FR = FA / FB; break;
	case AO_Mod :
		IR = IA % IB; FR = IR; break;
	}

	foreach LinkedVariables( class'SeqVar_Int', iv, "Result" )
	{
		iv.IntValue = IR;
	}

	foreach LinkedVariables( class'SeqVar_Float', fv, "Result" )
	{
		fv.FloatValue = FR;
	}
}

defaultproperties
{
	ObjCategory="Arith. Operation"
	ObjName="Arithmetic operation"
	bCallHandler=false
	
    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SequenceVariable',LinkDesc="Result", PropertyName=Result, MinVars=1,bWriteable=true )
	VariableLinks(1)=(ExpectedType=class'SequenceVariable',LinkDesc="First Opererand", PropertyName=FirstOperand, MinVars=1, MaxVars=1 )
	VariableLinks(2)=(ExpectedType=class'SequenceVariable',LinkDesc="Second Opererand", PropertyName=SecondOperand, MinVars=1, MaxVars=1 )
}

