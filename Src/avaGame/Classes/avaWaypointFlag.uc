class avaWaypointFlag extends Actor;

var StaticMeshComponent			FlagMeshComp;
var String						StaticMeshName;

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();
	CreateComponent();
}

simulated event Touch( Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal);

simulated function CreateComponent()
{
	Local StaticMesh Mesh;

	if ( StaticMeshName != "" )
	{
		Mesh = StaticMesh( DynamicLoadObject( StaticMeshName, class'StaticMesh' ) );
		if ( Mesh != None )	
		{
			FlagMeshComp.SetStaticMesh( Mesh );
		}
	}
}

simulated function SetStaticMeshName(string MeshName)
{
	StaticMeshName = MeshName;
	CreateComponent();
}

defaultproperties
{
	Begin Object Class=StaticMeshComponent Name=FlagMeshComponent0
		bUseAsOccluder = FALSE
		CollideActors=false
		CastShadow=false
	End Object
	FlagMeshComp=FlagMeshComponent0
	CollisionComponent=FlagMeshComponent0
 	Components.Add(FlagMeshComponent0)
}