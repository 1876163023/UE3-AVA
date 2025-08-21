class avaVehicle_Leopard_Content extends avaVehicle_Leopard
	placeable;

DefaultProperties
{
	// 없으면 차가 삥삥 돌면서 헤매이면서 힘들게 길을 찾아간다.
	// PathNode(Target)
	// 1. Pawn.ReachedDestination(Target)
	// 2. Target.ReachedBy(Pawn)에서 사용된다.
	Begin Object Name=CollisionCylinder
		CollisionHeight=64.0
		CollisionRadius=128.0
	End Object
	CylinderComponent=CollisionCylinder

	// HM 후에도 복구하기 위해 유지되는 값이며
	// Kismet Action으로 바뀌고 저장되는 변수이다.
	MaxEngineTorque = 400

	Begin Object Class=avaVehicleSimTank Name=SimObject
		// (from SVehicleSimBase)
		WheelSuspensionStiffness=45000//100000.0
		WheelSuspensionDamping=3500//2000.0
		WheelSuspensionBias=0.08//0.1
//		WheelInertia=0.8//1.0
		// (from SVehicleSimCar)
		ChassisTorqueScale=0.2
		StopThreshold=100
		ReverseThrottle=0
		EngineBrakeFactor=0.1
		// (from SVehicleSimTank)
		MaxEngineTorque=400.0
		EngineDamping=2
		InsideTrackTorqueFactor=0.2
//		SteeringLatStiffnessFactor=1.0
		TurnInPlaceThrottle=1.0

		// (from avaVehicleSimTank)
		FrontalCollisionGripFactor=0.18
		TurnEngineTorqueFactor=1.0

		// Longitudinal tire model based on 10% slip ratio peak. (from SVehicleBase)
		WheelLongExtremumSlip=1.5
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

	Seats(0)={( Mesh=SVehicleMesh,
				bSeatVisible=false, 
				bStayWhenDied=true,							//
				SeatBone=Bone01,
				SeatOffset=(X=-10,Z=150),					// 캐릭터 위치.
				GunClass=class'avaVWeap_LeopardTurret',
				GunPivotPoints=(Bone01),
				GunSocket=(TurretFireSocket),
				TurretVarPrefix="",
				TurretControls=(TurretPitch,TurretRotate),
				CameraTag=GunViewSocket,
				CameraOffset=-100,		// 카메라 기본 위치에서 X(LookAt-Z)축에 대한 오프셋.
				CameraEyeHeight=70)}

	Seats(1)={( Mesh=SkelGunMesh,
				SeatPawnClass=class'avaWeaponPawn_LeopardMachineGunner',
				ViewPitchMin=-2730,
				ViewPitchMax=3640,
				bSeatVisible=true,
				SeatBone=Char_Pos,
				SeatOffset=(Z=-1.5),					// 캐릭터 위치.
				GunClass=class'avaVWeap_LeopardMachineGun',
				GunPivotPoints=(MG3_Main),
				GunSocket=(GunFireSocket),
				TurretVarPrefix="Gunner",
				TurretControls=(GunRotate/*, GunPitch*/),
				MuzzleFlashLightClass=class'avaGame.avaGunMuzzleFlashLight',
				CameraTag=GunViewSocket,
				CameraOffset=-100)}

	// 기본으로 데미지를 받게 되는 무기 등록.
	TakenDamageTypes.Add(class'avaDmgType_RPG7');
	Health				=	400
	HealthBarLength		=	80
	DrawHUDBoneName		=	Bone01


	// Sounds

	// Engine sound.
	Begin Object Class=AudioComponent Name=LeopardEngineSound
		SoundCue=SoundCue'avaAmbientSound.RisingDust.Tank_Idle'
	End Object
	EngineSound=LeopardEngineSound
	Components.Add(LeopardEngineSound);

	// Wheel sound.
	Begin Object Class=AudioComponent Name=LeopardSquealSound
		SoundCue=SoundCue'avaAmbientSound.RisingDust.Tank_Move'
	End Object
	WheelSound=LeopardSquealSound
	Components.Add(LeopardSquealSound);

/*
	// Collision sound.
	Begin Object Class=AudioComponent Name=LeopardCollideSound
//		SoundCue=SoundCue'A_Vehicle_Goliath.SoundCues.A_Vehicle_Goliath_Collide'
	End Object
	CollideSound=LeopardCollideSound
	Components.Add(LeopardCollideSound);
*/
//	EnterVehicleSound=SoundCue'avaAmbientSound.RisingDust.Tank_Enter'
//	ExitVehicleSound=SoundCue'avaAmbientSound.RisingDust.Tank_Exit'

	// Initialize sound members. 
	// (아래는 기본으로 설정되어진 값입니다. 수정하시려면 주석을 풀고 고쳐서 사용해 주세요)
//	CollisionIntervalSecs = 1.0;
	SquealThreshold = 50.0;
	SquealLatThreshold=50.0f;
//	LatAngleVolumeMult=1.0f;
//	EngineStartOffsetSecs = 2.0;		// 엔진시동시간 정도 넣어주면 된다.
//	EngineStopOffsetSecs = 1.0;


	// 
/*

//	VehicleLockedSound=SoundCue'A_Vehicle_Generic.Vehicle.VehicleNoEntry01Cue';
//	LinkedToCue=SoundCue'A_Vehicle_Generic.Vehicle.VehicleChargeLoopCue'
//	LinkedEndSound=SoundCue'A_Vehicle_Generic.Vehicle.VehicleChargeCompleteCue'
//	HornSounds[0]=SoundCue'A_Vehicle_Generic.Vehicle.VehicleHornCue'
//	LockedOnSound=Soundcue'A_Weapon.AVRiL.Cue.A_Weapon_AVRiL_Lock_Cue'

//	VehicleSounds(0)=(SoundStartTag=DamageSmoke,SoundEndTag=NoDamageSmoke,SoundTemplate=SoundCue'A_Vehicle_Generic.Vehicle.Vehicle_Damage_Burst_Cue')
//	VehicleSounds(1)=(SoundStartTag=DamageSmoke,SoundEndTag=NoDamageSmoke,SoundTemplate=SoundCue'A_Vehicle_Generic.Vehicle.Vehicle_Damage_FireLoop_Cue')

//	ImpactHitSound=SoundCue'A_Weapon_ImpactHammer.ImpactHammer.A_Weapon_ImpactHammer_FireImpactVehicle_Cue'
//	LargeChunkImpactSound=SoundCue'A_Vehicle_Generic.Vehicle.VehicleImpact_MetalLargeCue'
//	MediumChunkImpactSound=SoundCue'A_Vehicle_Generic.Vehicle.VehicleImpact_MetalMediumCue'
//	SmallChunkImpactSound=SoundCue'A_Vehicle_Generic.Vehicle.VehicleImpact_MetalSmallCue'
//	SpawnInSound = SoundCue'A_Vehicle_Generic.Vehicle.VehicleFadeIn01Cue'
//	SpawnOutSound = SoundCue'A_Vehicle_Generic.Vehicle.VehicleFadeOut01Cue'

*/

	// 엔진 시동음, 정지음, 수리완료음(엔진 시동음과 같은 시점?).
	VehicleSounds(0)=(SoundStartTag=EngineStart,SoundTemplate=SoundCue'avaAmbientSound.RisingDust.Tank_Fix')
	VehicleSounds(1)=(SoundStartTag=EngineStop,SoundTemplate=SoundCue'avaAmbientSound.RisingDust.Tank_Engine_Stop')
	VehicleSounds(2)=(SoundStartTag=EngineStart,SoundTemplate=SoundCue'avaAmbientSound.RisingDust.Tank_Fix')
}