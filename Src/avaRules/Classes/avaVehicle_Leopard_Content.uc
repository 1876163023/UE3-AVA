class avaVehicle_Leopard_Content extends avaVehicle_Leopard
	placeable;

DefaultProperties
{
	// ������ ���� ��� ���鼭 ����̸鼭 ����� ���� ã�ư���.
	// PathNode(Target)
	// 1. Pawn.ReachedDestination(Target)
	// 2. Target.ReachedBy(Pawn)���� ���ȴ�.
	Begin Object Name=CollisionCylinder
		CollisionHeight=64.0
		CollisionRadius=128.0
	End Object
	CylinderComponent=CollisionCylinder

	// HM �Ŀ��� �����ϱ� ���� �����Ǵ� ���̸�
	// Kismet Action���� �ٲ�� ����Ǵ� �����̴�.
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
				SeatOffset=(X=-10,Z=150),					// ĳ���� ��ġ.
				GunClass=class'avaVWeap_LeopardTurret',
				GunPivotPoints=(Bone01),
				GunSocket=(TurretFireSocket),
				TurretVarPrefix="",
				TurretControls=(TurretPitch,TurretRotate),
				CameraTag=GunViewSocket,
				CameraOffset=-100,		// ī�޶� �⺻ ��ġ���� X(LookAt-Z)�࿡ ���� ������.
				CameraEyeHeight=70)}

	Seats(1)={( Mesh=SkelGunMesh,
				SeatPawnClass=class'avaWeaponPawn_LeopardMachineGunner',
				ViewPitchMin=-2730,
				ViewPitchMax=3640,
				bSeatVisible=true,
				SeatBone=Char_Pos,
				SeatOffset=(Z=-1.5),					// ĳ���� ��ġ.
				GunClass=class'avaVWeap_LeopardMachineGun',
				GunPivotPoints=(MG3_Main),
				GunSocket=(GunFireSocket),
				TurretVarPrefix="Gunner",
				TurretControls=(GunRotate/*, GunPitch*/),
				MuzzleFlashLightClass=class'avaGame.avaGunMuzzleFlashLight',
				CameraTag=GunViewSocket,
				CameraOffset=-100)}

	// �⺻���� �������� �ް� �Ǵ� ���� ���.
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
	// (�Ʒ��� �⺻���� �����Ǿ��� ���Դϴ�. �����Ͻ÷��� �ּ��� Ǯ�� ���ļ� ����� �ּ���)
//	CollisionIntervalSecs = 1.0;
	SquealThreshold = 50.0;
	SquealLatThreshold=50.0f;
//	LatAngleVolumeMult=1.0f;
//	EngineStartOffsetSecs = 2.0;		// �����õ��ð� ���� �־��ָ� �ȴ�.
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

	// ���� �õ���, ������, �����Ϸ���(���� �õ����� ���� ����?).
	VehicleSounds(0)=(SoundStartTag=EngineStart,SoundTemplate=SoundCue'avaAmbientSound.RisingDust.Tank_Fix')
	VehicleSounds(1)=(SoundStartTag=EngineStop,SoundTemplate=SoundCue'avaAmbientSound.RisingDust.Tank_Engine_Stop')
	VehicleSounds(2)=(SoundStartTag=EngineStart,SoundTemplate=SoundCue'avaAmbientSound.RisingDust.Tank_Fix')
}