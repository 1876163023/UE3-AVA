/*
	VehicleBot ���� �̺�Ʈ.

	2007/07/10	����
*/
class avaSeqAct_VehicleBotEvent extends SequenceAction;

enum EVehicleBotEvent
{
	VHBot_None,
	VHBot_Go,
	VHBot_Stop,
	VHBot_MaxEngineTorque,
	VHBot_TurnEngineTorqueFactor,
	VHBot_GetHealth_DontUse,
	VHBot_ResetHealth,
	VHBot_GetPlayerPawnCount,
	VHBot_GodMode,
	VHBot_LookAt,
	VHBot_Fire,
	VHBot_Destroy,
};

//! �̺�Ʈ.
var() EVehicleBotEvent	Event;

//! Vehicle�� SimTank�� MaxEngineTorque���� ������ �ش�.
//! (1000���� ������ ��� �ö󰡱Ⱑ �������)
var() float				MaxEngineTorque<Tooltip=VHBot_MaxEngineTorque>;

//! ȸ���� �⺻ EngineTorque�� �����ش�.
var() float				TurnEngineTorqueFactor<Tooltip=VHBot_TrunEngineTorqueFactor>;

//! ü�°�.
var() int				MaxHealth<Tooltip=VHBot_GetHealth, VHBot_SetHealth>;

//! Ÿ�� �ִ� �÷��̾ ��Ʈ�� �ϴ� Pawn�� ����.
var int					PlayerPawnCount;

//! �ʴ����� �����ð��� �����Ѵ�.
var() float				GodModeTime<Tooltip=VHBot_GodMode, VHBot_SetHealth>;

//! �ٶ� ���.
var() Actor				ViewTarget<Tooltip=VHBot_LookAt, VHBot_Fire>;

//! �߻� �غ� �ð�.
var() float				FireTime;

/*
event Activated()
{
	`log("avaSeqAct_VehicleBotEvent.Event=" @Event);
}
*/

DefaultProperties
{
	bCallHandler = true
	ObjCategory="Pawn"
	ObjName="VehicleBotEvent (ava)"
	ObjClassVersion=2
	FireTime = 3

	VariableLinks.Add( (ExpectedType=class'SeqVar_Int',LinkDesc="Health",PropertyName=Health, MinVars=0,bWriteable=true,bHidden=true) )
	VariableLinks.Add( (ExpectedType=class'SeqVar_Int',LinkDesc="PlayerPawnCount",PropertyName=PlayerPawnCount, MinVars=0,bWriteable=true,bHidden=true) )
	VariableLinks.Add( (ExpectedType=class'SeqVar_Object',LinkDesc="ViewTarget",PropertyName=ViewTarget, MinVars=0,bWriteable=true,bHidden=true) )
}