/*
	VehicleBot 전용 이벤트.

	2007/07/10	고광록
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

//! 이벤트.
var() EVehicleBotEvent	Event;

//! Vehicle의 SimTank의 MaxEngineTorque값을 수정해 준다.
//! (1000보다 작으면 언덕 올라가기가 힘들더라)
var() float				MaxEngineTorque<Tooltip=VHBot_MaxEngineTorque>;

//! 회전시 기본 EngineTorque에 곱해준다.
var() float				TurnEngineTorqueFactor<Tooltip=VHBot_TrunEngineTorqueFactor>;

//! 체력값.
var() int				MaxHealth<Tooltip=VHBot_GetHealth, VHBot_SetHealth>;

//! 타고 있는 플레이어가 컨트롤 하는 Pawn의 개수.
var int					PlayerPawnCount;

//! 초단위로 무적시간을 설정한다.
var() float				GodModeTime<Tooltip=VHBot_GodMode, VHBot_SetHealth>;

//! 바라볼 대상.
var() Actor				ViewTarget<Tooltip=VHBot_LookAt, VHBot_Fire>;

//! 발사 준비 시간.
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