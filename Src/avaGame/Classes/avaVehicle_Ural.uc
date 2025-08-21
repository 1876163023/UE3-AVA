/*
	avaVehicle를 테스트 하기 위한 임시 클래스.

	q. Material이 적용안되는 Mesh는 머지??
	a. MaterialInstaceConstant에서 같은 이름으로 Parameter를 지어서 그렇더라.
*/
class avaVehicle_Ural extends avaVehicle
	placeable
	native;

var() float	CurrentWheelRadius;
var() float	CurrentSuspensionTravel;

simulated function PostBeginPlay()
{
	super.PostBeginPlay();

	// 바로 지형과 적용되도록 해준다.
	if ( Mesh != None )
		Mesh.WakeRigidBody();
}

/*
exec simulated function TestUralImpact()
{
	Mesh.AddImpulse(Vect(0,1000,0), Vect(0,0,0));
}
*/

DefaultProperties
{
	Health=300
	StolenAnnouncementIndex=36

	COMOffset=(x=-40.0,y=0.0,z=-36.0)
	UprightLiftStrength=280.0
	UprightTime=1.25
	UprightTorqueStrength=500.0
	bCanFlip=true
	bSeparateTurretFocus=true
	bHasHandbrake=true
	bStickDeflectionThrottle=true
	GroundSpeed=950
	AirSpeed=1150
	ObjectiveGetOutDist=1500.0
	MaxDesireability=0.4
	HeavySuspensionShiftPercent=0.75f;
	bUseLookSteer=false

	Begin Object name=SVehicleMesh
		SkeletalMesh=SkeletalMesh'Avaprop_Trans.ural.SK_URAL'
		AnimTreeTemplate=AnimTree'Avaprop_Trans.ural.SK_URAL_AnimTree'
		PhysicsAsset=PhysicsAsset'Avaprop_Trans.ural.SK_URAL_Physics'
	End Object

	Begin Object class=avaVehicleUralWheel name=FLWheel
		BoneName="L_Front_Bone01"
		SkelControlName="FL_Tire_Ctrl"
		Side=SIDE_Left
		// 바퀴에 Steering기능 넣기.
		SteerFactor=1.0
	End Object

	Begin Object class=avaVehicleUralWheel name=FRWheel
		BoneName="R_Front_Bone01"
		SkelControlName="FR_Tire_Ctrl"
		Side=SIDE_Right
		// 바퀴에 Steering기능 넣기.
		SteerFactor=1.0
	End Object

	Begin Object class=avaVehicleUralWheel name=MLWheel
		BoneName="L_Middle_Bone01"
		SkelControlName="ML_Tire_Ctrl"
		Side=SIDE_Left
	End Object

	Begin Object class=avaVehicleUralWheel name=MRWheel
		BoneName="R_Middle_Bone01"
		SkelControlName="MR_Tire_Ctrl"
		Side=SIDE_Right
	End Object

	Begin Object class=avaVehicleUralWheel name=RLWheel
		BoneName="L_Rear_Bone01"
		SkelControlName="RL_Tire_Ctrl"
		Side=SIDE_Left
	End Object

	Begin Object class=avaVehicleUralWheel name=RRWheel
		BoneName="R_Rear_Bone01"
		SkelControlName="RR_Tire_Ctrl"
		Side=SIDE_Right
	End Object

	Wheels(0) = FLWheel
	Wheels(1) = MLWheel
	Wheels(2) = RLWheel
	Wheels(3) = FRWheel
	Wheels(4) = MRWheel
	Wheels(5) = RRWheel

	Begin Object Class=avaVehicleSimCar Name=SimObject
		// (from SVehicleSimBase)
		WheelSuspensionStiffness=100.0
		WheelSuspensionDamping=3.0
		WheelSuspensionBias=0.1
		// (from SVehicleSimCar)
		ChassisTorqueScale=0.5
		MaxBrakeTorque=5.0
		StopThreshold=100
		MaxSteerAngleCurve=(Points=((InVal=0,OutVal=45),(InVal=600.0,OutVal=15.0),(InVal=1500.0,OutVal=6.0),(InVal=2500.0,OutVal=1.0)))
		SteerSpeed=110

		// (from avaVehicleSimCar)
		LSDFactor=0.0
		TorqueVSpeedCurve=(Points=((InVal=-600.0,OutVal=0.0),(InVal=-300.0,OutVal=80.0),(InVal=0.0,OutVal=120.0),(InVal=950.0,OutVal=120.0),(InVal=1050.0,OutVal=10.0),(InVal=1150.0,OutVal=0.0)))
		EngineRPMCurve=(Points=((InVal=-500.0,OutVal=2500.0),(InVal=0.0,OutVal=500.0),(InVal=549.0,OutVal=3500.0),(InVal=550.0,OutVal=1000.0),(InVal=849.0,OutVal=4500.0),(InVal=850.0,OutVal=1500.0),(InVal=1100.0,OutVal=5000.0)))
		EngineBrakeFactor=0.025
		ThrottleSpeed=0.1
		NumWheelsForFullSteering=6
		SteeringReductionFactor=0.0
		SteeringReductionMinSpeed=1100.0
		SteeringReductionSpeed=1400.0
		bAutoHandbrake=true

		// (from SVehicleBase)
		WheelInertia=0.2

		// Longitudinal tire model based on 10% slip ratio peak. (from SVehicleBase)
		WheelLongExtremumSlip=0.1
		WheelLongExtremumValue=1.0
		WheelLongAsymptoteSlip=2.0
		WheelLongAsymptoteValue=0.6

		// Lateral tire model based on slip angle (radians). (from SVehicleBase)
   		WheelLatExtremumSlip=0.35     // 20 degrees
		WheelLatExtremumValue=0.85
		WheelLatAsymptoteSlip=1.4     // 80 degrees
		WheelLatAsymptoteValue=0.7
	End Object
	SimObj=SimObject
	Components.Add(SimObject)


	Seats(0)={( bSeatVisible=true, 
				SeatOffset=(X=0,Z=72), 
				CameraBaseOffset=(X=70,Y=0,Z=80),
				CameraOffset=-115,
				CameraEyeHeight=50)}

	// 어짜피 지금은 적용되지 않는다.(epic PrimitiveComponent갱신해야...)
//	DefaultPhysicalMaterial=PhysicalMaterial'Avaprop_Trans.ural.SK_URAL_PhyMat'
//	DrivingPhysicalMaterial=PhysicalMaterial'Avaprop_Trans.ural.SK_URAL_DrivingPhyMat'

	TeamBeaconOffset=(z=60.0)
	SpawnRadius=125.0

	BaseEyeheight=30
	Eyeheight=30
	DefaultFOV=80
	bLightArmor=true

	bReducedFallingCollisionDamage=true
}