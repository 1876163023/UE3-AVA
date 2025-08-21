/*
	Client에서도 Vehicle이 생성되었다는 것을 알기 위한 이벤트

	2007/08/09	고광록
		SeqEvent_PlayerSpawned를 참고.
*/
class avaSeqEvent_VehicleSpawned extends SequenceEvent;

var Vehicle	SpawnedVehicle;

event Activated()
{
	`log("VehicleSpawned.Activated()");
}

defaultproperties
{
	ObjName="Vehicle Spawned"
	ObjCategory="Vehicle"

	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Spawned Vehicle",bWriteable=TRUE,PropertyName=SpawnedVehicle)
}
