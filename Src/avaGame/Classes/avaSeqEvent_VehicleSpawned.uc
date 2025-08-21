/*
	Client������ Vehicle�� �����Ǿ��ٴ� ���� �˱� ���� �̺�Ʈ

	2007/08/09	����
		SeqEvent_PlayerSpawned�� ����.
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
