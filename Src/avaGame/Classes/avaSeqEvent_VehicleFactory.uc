/**
 *
 * Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved.
 */


class avaSeqEvent_VehicleFactory extends SequenceEvent;
	
/* [-] 20070904	
cpptext
{
	// {{ 20070731 dEAthcURe|HM	
	virtual void PreSave() {
		Super::PreSave();
		hmId = ++GhmId;
	}
	// }} 20070731 dEAthcURe|HM
}
var int hmId; // 20070723 dEAthcURe|HM
*/

/** reference to the vehicle spawned by the factory */
var avaVehicle SpawnedVehicle;

event Activated()
{
	if (avaVehicleFactory(Originator) != None)
	{
		SpawnedVehicle = avaVehicleFactory(Originator).ChildVehicle;
		//SpawnedVehicle.hmId = hmId; // [-] 20070904
	}
}

defaultproperties
{
	//hmId = -1; // [-] 20070904 // 20070731 dEAthcURe|HM
	ObjCategory="Vehicle"
	ObjName="Vehicle Factory Event"
	OutputLinks[0]=(LinkDesc="Spawned")
	OutputLinks[1]=(LinkDesc="Taken")
	OutputLinks[2]=(LinkDesc="Destroyed")
	OutputLinks[3]=(LinkDesc="VehicleEntered")
	OutputLinks[4]=(LinkDesc="VehicleLeft")
	bPlayerOnly=false
	MaxTriggerCount=0
	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Spawned Vehicle",bWriteable=true,PropertyName=SpawnedVehicle)
	ObjClassVersion=2
}
