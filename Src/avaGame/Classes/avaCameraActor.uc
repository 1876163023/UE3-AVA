class avaCameraActor extends CameraActor
	native
	placeable;

enum EPDACameraHeight
{
	PDACameraHeight_Low,
	PDACameraHeight_Normal,
	PDACameraHeight_High,
};

struct native PDACameraInfo
{
	var() vector CamLocation;
	var() rotator CamRotation;
	var() float FOV;
	var() bool bFixedLocation;

	structdefaultproperties
	{
		CamLocation=(X=0,Y=0,Z=1000)
		CamRotation=(Pitch=-16384,Yaw=32767,Roll=32767)
		bFixedLocation=false
		FOV=90
	}
};

var EPDACameraHeight			CurrentPDAHeight;
var(PDA) array<PDACameraInfo>	PDACameraData;

var vector						LatestLocation;
var Rotator						LatestRotation;

var const float					MoveDuration;
var float						MoveUpdateTime;

var vector						NewLocation;
var Rotator						NewRotation;

//<@ changmin 2007. 5. 22
var()							float ReferenceResolution;
native simulated function float ComputeFOV();
simulated function GetCameraView(float DeltaTime, out TPOV OutPOV )
{
	GetActorEyesViewPoint( OutPOV.Location, OutPOV.Rotation );
	OutPOV.FOV = ComputeFOV();
}
//>@ changmin

native final function CalcPickRay( int x, int y, out vector RayLoc,out vector RayDir);


function PostBeginPlay()
{
	Local Rotator PDAViewRot;

	Super.PostBeginPlay();

	PDAViewRot.Pitch = -65535/4;
	PDAViewRot.Yaw = 65535/2;
	PDAViewRot.Roll = 65535/2;
	
	SetRotation(PDAViewRot);
}

function SetCameraHeight(EPDACameraHeight CamHeight)
{
	Local PlayerController PC;

	CurrentPDAHeight = CamHeight;
	
	PC = PlayerController(Owner);
	if( PDACameraData[CurrentPDAHeight].bFixedLocation )
		NewLocation = PDACameraData[CurrentPDAHeight].CamLocation;
	else
	{
		NewLocation = ( (PC != None && PC.Pawn != None) ? PC.Pawn.Location : Location);
		NewLocation.Z = PDACameraData[CurrentPDAHeight].CamLocation.Z;
	}
	
	NewRotation = PDACameraData[CurrentPDAHeight].CamRotation;
	
	Assert(PC != None && PC.WorldInfo != None);

	LatestLocation = Location;
	LatestRotation = Rotation;

	//`Log("CurrentViewTarget = "$PC.GetViewTarget());
	//`Log(LatestLocation$" -> "$NewLocation);
	//`Log(LatestRotation$" -> "$NewRotation);

	MoveUpdateTime = PC.WorldInfo.TimeSeconds + MoveDuration;
//	`Log("StartTimer");
//	SetTimer(0.01, true);
}

//function Timer()
//{
//	Local PlayerController PC;
//	Local float TimeLeft;
//	Local vector DiffLocation;
//	Local rotator DiffRotation;
//
//	
//	PC = PlayerController(Owner);
//	if( ( PC != None && PC.WorldInfo != None ) &&	
//		MoveUpdateTime >= PC.WorldInfo.TimeSeconds)
//	{
//		TimeLeft = MoveUpdateTime - PC.WorldInfo.TimeSeconds;
//		DiffLocation = LatestLocation - NewLocation;
//		DiffRotation = LatestRotation - NewRotation;
//
//		SetLocation( NewLocation + DiffLocation * ( TimeLeft / MoveDuration) );
//		SetRotation( NewRotation + DiffRotation * ( TimeLeft / MoveDuration) );	
//	}
//	else
//	{
//		SetLocation( NewLocation );
//		SetRotation( NewRotation );
//		ClearTimer();
//	}
//}

function float GetFOVAngle()
{
	return PDACameraData[CurrentPDAHeight].FOV;
}

function array<PDACameraInfo> GetPDACameraInfo()
{
	return PDACameraData;
}

function PushUp()
{
	Local int CameraHeight;

	CameraHeight = (int(CurrentPDAHeight) + 1) >= PDACameraHeight_MAX ? 
		PDACameraHeight_MAX - 1 : CurrentPDAHeight + 1;

	SetCameraHeight(EPDACameraHeight(CameraHeight));
}

function PullDown()
{
	CurrentPDAHeight = (int(CurrentPDAHeight) - 1) < 0 ? 
		EPDACameraHeight(0) : EPDACameraHeight( CurrentPDAHeight - 1 );

	SetCameraHeight(CurrentPDAHeight);
}

function SetPDACameraInfo(int PDACamHeight, vector CamLocation, rotator CamRotation, float FOV, bool bFixedLocation = false)
{
	if( !(0 <= PDACamHeight && PDACamHeight < PDACameraHeight_MAX) )
	{
		`Warn("Inacceptable PDACameraHeight = "$PDACamHeight);
		return;
	}

	PDACameraData[PDACamHeight].CamLocation = CamLocation;
	PDACameraData[PDACamHeight].CamRotation = CamRotation;
	PDACameraData[PDACamHeight].FOV = FOV;
	PDACameraData[PDACamHeight].bFixedLocation = bFixedLocation;
}

defaultproperties
{
	bConstrainAspectRatio=false
	bStatic=false
	bNoDelete=false

	MoveDuration=0.5

	PDACameraData(PDACameraHeight_High)=(CamLocation=(X=0,Y=0,Z=7000),FOV=70,bFixedLocation=true)
	PDACameraData(PDACameraHeight_Normal)=(CamLocation=(X=0,Y=0,Z=4000),FOV=60,bFixedLocation=false)
	PDACameraData(PDACameraHeight_Low)=(CamLocation=(X=0,Y=0,Z=3000),FOV=50,bFixedLocation=false)
	
	ReferenceResolution=1024.0
}