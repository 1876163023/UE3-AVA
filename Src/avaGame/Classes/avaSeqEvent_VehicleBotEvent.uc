/*
	Vehicle 관련 이벤트 발생.

	2007/07/12 고광록
		Level Kismet에서 Bot에 의해 움직이는 Vehicle에 대한 Event을 받는다.
*/
class avaSeqEvent_VehicleBotEvent extends SequenceEvent;

//! Bot에서 ScriptedMove로 이동해 도착한 MoveTarget객체.
var transient Actor		MoveTarget;
//! 나를 죽인 자.
var transient Actor		Killer;
//! 바라보는 대상.
var transient Actor		ViewTarget;

event Activated()
{
	local Pawn P;

	P = Pawn(Instigator);
	if ( P != None )
	{
		// 만약 P가 Killer인 경우에는 AIController는 없어지게 된다.
		if ( AIController(P.Controller) != None )
		{
			// AIController의 ScriptedMoveTarget를 얻어온다.
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
	OutputLinks[0]=(LinkDesc="Start Route")					// Route시작.
	OutputLinks[1]=(LinkDesc="End Route")					// Route종료.
	OutputLinks[2]=(LinkDesc="Failed to End Route")			// Route종료실패.
	OutputLinks[3]=(LinkDesc="Reached Target")				// Target 도착.
	OutputLinks[4]=(LinkDesc="Failed to Reach Target")		// Target 도착실패.
	OutputLinks[5]=(LinkDesc="Died")						// 죽었을 경우.
	bPlayerOnly=false
	MaxTriggerCount=0
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="MoveTarget",bWriteable=true,PropertyName=MoveTarget)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Object',LinkDesc="Killer",bWriteable=true,PropertyName=Killer)
	VariableLinks(3)=(ExpectedType=class'SeqVar_Object',LinkDesc="ViewTarget",bWriteable=true,PropertyName=ViewTarget)
	ObjClassVersion=3
}