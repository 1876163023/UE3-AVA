class avaVehicleUralWheel extends SVehicleWheel;

defaultproperties
{
	WheelRadius=35
	SuspensionTravel=15
	bPoweredWheel=true
	SteerFactor=0.0

	// 이 값의 크기를 조정해서ㅓ.
	LongSlipFactor=1000
	LatSlipFactor=500
	HandbrakeLongSlipFactor=600
	HandbrakeLatSlipFactor=200

//	LongSlipFactor=2.0
//	LatSlipFactor=2.75
//	HandbrakeLongSlipFactor=0.6
//	HandbrakeLatSlipFactor=0.2
//	ParkedSlipFactor=10.0
}