class avaPickupDogTag extends avaPickup;

var string					EUDogTagName;
var string					NRFDogTagName;
var ParticleSystemComponent	LampPSC;
var ParticleSystem			LampPSCTemplate;

simulated function CreateComponent()
{
}

simulated function ChangedTeam()
{
	local StaticMesh		tmpStaticMesh;
	local string			MeshName;
	Super.ChangedTeam();
	if ( TeamIdx == 0 )
	{
		MeshName = EUDogTagName;
	}
	else
	{
		MeshName = NRFDogTagName;
	}
	tmpStaticMesh = StaticMesh( DynamicLoadObject( MeshName, class'StaticMesh' ) );
	StaticMeshComponent.SetStaticMesh( tmpStaticMesh );

	LampPSC = new(self) class'ParticleSystemComponent';
	LampPSC.SetTemplate( LampPSCTemplate );
	AttachComponent( LampPSC );
}

state Pickup
{
	function bool ValidTouch(Pawn Other)
	{
		return true;
	}

	function Timer()
	{
		PickedUpBy(None);
	}

	function BeginState(Name PreviousStateName)
	{
		SetTimer( LifeTime, false );
		AddToLevel();
	}
}

function GiveTo( Pawn P )
{
	avaPawn(P).AddDogTag();
	PickedUpBy( P );
}

static function DLO( string resource )
{
	if (resource != "")
	{		
		DynamicLoadObject( resource, class'Object' );
	}
}

static event LoadDLOs()
{
	DLO( default.EUDogTagName );
	DLO( default.NRFDogTagName );
}

defaultproperties
{
	Begin Object Name=StaticMeshComponent0		
		StaticMesh=StaticMesh'ava_DogTag.dogTag_EU.MS_DogTag_EU'
		BlockZeroExtent=false
		BlockNonZeroExtent=false
	End Object	

	EUDogTagName	=	"ava_DogTag.dogTag_EU.MS_DogTag_EU"
	NRFDogTagName	=	"ava_DogTag.dogTag_NRF.MS_DogTag_NRF"
	LampPSCTemplate	=	ParticleSystem'avaEffect2.dogTag.PS_Dogtag_Light'
	LifeTime		=	15.0
	bDynamicSpawned	=	true
	DrawScale3D		=	(X=1.2,Y=1.2,Z=1.5)
}
