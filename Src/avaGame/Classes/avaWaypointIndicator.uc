class avaWaypointIndicator extends Actor;



var StaticMeshComponent			IndicatorMeshComp;
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
			IndicatorMeshComp.SetStaticMesh( Mesh );
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
	Begin Object Class=StaticMeshComponent Name=IndicatorMeshComponent0
		bUseAsOccluder = FALSE
		CollideActors=false
		CastShadow=false
		DepthPriorityGroup=SDPG_ForeGround
		Scale=0.1
	End Object
	IndicatorMeshComp=IndicatorMeshComponent0
	CollisionComponent=IndicatorMeshComponent0
 	Components.Add(IndicatorMeshComponent0)
}