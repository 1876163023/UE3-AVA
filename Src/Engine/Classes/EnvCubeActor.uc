class EnvCubeActor extends Actor
	native
	placeable;

var const TextureCube Texture;

/** for visualizing the cube capture */
var const StaticMeshComponent StaticMesh;

var() Package TexturePackage;
var() int TextureSize;
var() array<Actor> Filter;

cpptext
{	
	// AActor interface

	virtual void Spawned();

	// UObject interface

	virtual void FinishDestroy();
	virtual void PostLoad();

private:

	/**
	* Init the helper components 
	*/
	virtual void Init();
}


defaultproperties
{
	// cube map scene capture component 
	Begin Object Class=SceneCaptureCubeMapComponent Name=SceneCaptureCubeMapComponent0
	End Object
	Components.Add(SceneCaptureCubeMapComponent0)	

	// sphere for better viz
	Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0		
	HiddenGame=true
	CastShadow=false
	bAcceptsLights=false
	CollideActors=false
	Scale3D=(X=0.6,Y=0.6,Z=0.6)
	StaticMesh=StaticMesh'EditorMeshes.TexPropSphere'
	End Object
	StaticMesh=StaticMeshComponent0
	Components.Add(StaticMeshComponent0)

	// allow for actor to tick locally on clients
	bStatic=true
	bNoDelete=true
	RemoteRole=ROLE_SimulatedProxy	

	bEdShouldSnap=true
	bHardAttach=true
	bGameRelevant=true	

	TextureSize=32
}
