/*
	Leopard Factory

	2007/07/26	����
		ResetRound �ȵǴ� ������ UT���� �����´�.
*/
class avaVehicleFactory_Leopard extends avaVehicleFactory;

//! ��ũ�� �̵��ӵ� ����.
var() float	MaxEngineTorque;

//! ��ũ�� ȸ�� �ӵ��� ��ȭ��Ű�� ���.
var() float TurnEngineTorqueFactor<tooltip=not used>;

//! �������� ���� �� �ִ� ����.
var() array< class<DamageType> >	TakenDamageTypes;

//! CylinderComponent�� ����Ǵ� �������� ���̰�.
var() float CollisionRadius<Tooltip=Cylinder Radius(Default=150)>;
var() float CollisionHeight<Tooltip=Cylinder Height(Default=60)>;

var()	float				WheelSuspensionStiffness;
var()	float				WheelSuspensionDamping;
var()	float				WheelSuspensionBias;
var()	float				WheelInertia;

//! Factory���� Spawn�� ���� ȣ��Ǿ� ���� ������ ������ �ش�.
function OnSpawnChildVehicle()
{
	local avaVehicle_Leopard	V;
	local float					Height, Radius;

	V = avaVehicle_Leopard( ChildVehicle );
	if ( V != None )
	{
		TeamNum = int(TeamSpawningControl);

		V.TakenDamageTypes = TakenDamageTypes;
		V.Health = MaxHealth;

		if ( MaxEngineTorque != 0.0 )
			V.SetMaxEngineTorque( MaxEngineTorque );

		if ( TurnEngineTorqueFactor != 0.0 )
			V.SetTurnEngineTorqueFactor( TurnEngineTorqueFactor );

		// ���� 0�� �ƴ� ��� ����.
		if ( CollisionRadius != 0.0 || CollisionHeight != 0.0 )
		{
			if ( CollisionRadius == 0 )
				Radius = V.CylinderComponent.CollisionRadius;
			else
				Radius = CollisionRadius;

			if ( CollisionHeight == 0 )
				Height = V.CylinderComponent.CollisionHeight;
			else
				Height = CollisionHeight;

			V.CylinderComponent.SetSize(Height, Radius);
		}

		V.SimObj.WheelSuspensionStiffness = WheelSuspensionStiffness;
		V.SimObj.WheelSuspensionDamping = WheelSuspensionDamping;
		V.SimObj.WheelSuspensionBias = WheelSuspensionBias;
		V.SimObj.WheelInertia = WheelInertia;
	}
}

//! �߰����ش�.
simulated function OnVehicleBotEvent(avaSeqAct_VehicleBotEvent action)
{
	local avaVehicle_Leopard V;

	V = avaVehicle_Leopard( ChildVehicle );
	if ( V != None )
		V.OnVehicleBotEvent(action);
}

DefaultProperties
{
	Begin Object Class=DynamicLightEnvironmentComponent Name=MyLightEnvironment
	End Object
	Components.Add(MyLightEnvironment)

	Begin Object Class=SkeletalMeshComponent Name=SVehicleMesh
		CollideActors=false
		SkeletalMesh=SkeletalMesh'Avaprop_Trans01.Leopard.SK_Leopard2A6_Final'
		HiddenGame=true
		Translation=(X=0.0,Y=0.0,Z=-10.0)
		Rotation=(Yaw=-16384)
		AlwaysLoadOnClient=false
		AlwaysLoadOnServer=false
	End Object
	Components.Add(SVehicleMesh)
	Components.Remove(Sprite)

	Begin Object Name=CollisionCylinder
		CollisionRadius=150
		CollisionHeight=70.0
	End Object

	VehicleClass=class'avaVehicle_Leopard_Content'
	DrawScale=1.0

	TakenDamageTypes.Add(class'avaDmgType_RPG7');

	SupportedEvents.Add(class'avaSeqEvent_VehicleBotEvent')

	// �ӽ�.
	WheelSuspensionStiffness = 500
	WheelSuspensionDamping = 10
	WheelSuspensionBias = 0.1
	WheelInertia = 1.0
}
