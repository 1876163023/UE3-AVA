/*
	Leopard Factory

	2007/07/26	고광록
		ResetRound 안되는 문제로 UT에서 가져온다.
*/
class avaVehicleFactory_Leopard extends avaVehicleFactory;

//! 탱크의 이동속도 조정.
var() float	MaxEngineTorque;

//! 탱크의 회전 속도을 변화시키는 계수.
var() float TurnEngineTorqueFactor<tooltip=not used>;

//! 데미지를 입을 수 있는 종류.
var() array< class<DamageType> >	TakenDamageTypes;

//! CylinderComponent에 적용되는 반지름과 높이값.
var() float CollisionRadius<Tooltip=Cylinder Radius(Default=150)>;
var() float CollisionHeight<Tooltip=Cylinder Height(Default=60)>;

var()	float				WheelSuspensionStiffness;
var()	float				WheelSuspensionDamping;
var()	float				WheelSuspensionBias;
var()	float				WheelInertia;

//! Factory에서 Spawn된 직후 호출되어 각종 값들을 수정해 준다.
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

		// 값이 0이 아닌 경우 적용.
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

//! 중계해준다.
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

	// 임시.
	WheelSuspensionStiffness = 500
	WheelSuspensionDamping = 10
	WheelSuspensionBias = 0.1
	WheelInertia = 1.0
}
