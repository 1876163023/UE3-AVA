class avaEmit_Camera extends avaEmitter
	native;

var()	float		DistFromCamera;
var		private		avaPlayerCamera	Cam;

simulated function PostBeginPlay()
{
	//ParticleSystemComponent.SetDepthPriorityGroup( SDPG_Foreground );
	//Super.PostBeginPlay();
}

simulated function Destroyed()
{
}

/** Tell the emitter what camera it is attached to. */
simulated function RegisterCamera(avaPlayerCamera inCam)
{
	Cam = inCam;
}

simulated native function UpdateLocation( const out vector CamLoc, const out rotator CamRot, float CamFOVDeg );

defaultproperties
{
	TickGroup=TG_PostAsyncWork
	DistFromCamera=180
	LifeSpan=10.0f
}


