/*
	Vehicle ���� �̺�Ʈ �߻�.

	2007/07/12 ����
		Level Kismet���� Bot�� ���� �����̴� Vehicle�� ���� Event�� �޴´�.
*/
class avaSeqEvent_VehicleBotEvent extends SequenceEvent;

//! Bot���� ScriptedMove�� �̵��� ������ MoveTarget��ü.
var transient Actor		MoveTarget;
//! ���� ���� ��.
var transient Actor		Killer;
//! �ٶ󺸴� ���.
var transient Actor		ViewTarget;

event Activated()
{
	local Pawn P;

	P = Pawn(Instigator);
	if ( P != None )
	{
		// ���� P�� Killer�� ��쿡�� AIController�� �������� �ȴ�.
		if ( AIController(P.Controller) != None )
		{
			// AIController�� ScriptedMoveTarget�� ���´�.
			MoveTarget = AIController(P.Controller).ScriptedMoveTarget;
			ViewTarget = AIController(P.Controller).ScriptedFocus;
			Killer = None;
		}
		else if ( PlayerController(P.Controller) != None )
		{
			MoveTarget = None;
			Killer = P;
		}
	}

	`log("avaSeqEvent_VehicleBotEvent - Pawn =" @Instigator 
		@"MoveTarget =" @MoveTarget @"Killer =" @Killer @"ViewTarget =" @ViewTarget);
}

DefaultProperties
{
	ObjCategory="Vehicle"
	ObjName="Vehicle Event"
	OutputLinks[0]=(LinkDesc="Start Route")					// Route����.
	OutputLinks[1]=(LinkDesc="End Route")					// Route����.
	OutputLinks[2]=(LinkDesc="Failed to End Route")			// Route�������.
	OutputLinks[3]=(LinkDesc="Reached Target")				// Target ����.
	OutputLinks[4]=(LinkDesc="Failed to Reach Target")		// Target ��������.
	OutputLinks[5]=(LinkDesc="Died")						// �׾��� ���.
	bPlayerOnly=false
	MaxTriggerCount=0
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="MoveTarget",bWriteable=true,PropertyName=MoveTarget)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Object',LinkDesc="Killer",bWriteable=true,PropertyName=Killer)
	VariableLinks(3)=(ExpectedType=class'SeqVar_Object',LinkDesc="ViewTarget",bWriteable=true,PropertyName=ViewTarget)
	ObjClassVersion=3
}