class avaBulletTrailComponent extends PrimitiveComponent
	native
	noexport;

var() const MaterialInstance		Material;
var() float	Intensity;
var() float	HalfLife;
var() float Size;
var() float Speed;
var() float AddDeltaTime;

struct BulletTrailEntry
{
	var private vector Source, Destination;
	var private vector InitVel;
	var private float Speed;
	var private float FiredTime;	
	var private float AddDeltaTime;
};

var private native transient array<BulletTrailEntry> BulletTrails;
var private native pointer Renderer;

native simulated function Fire( vector Source, vector Destination, vector InitVel );

defaultproperties
{
	Intensity = 2.0
	HalfLife = 0.1
	
	CastShadow=FALSE
	bAcceptsLights=TRUE
	
	CollideActors=False
	BlockActors=False
	BlockZeroExtent=False
	BlockNonZeroExtent=False
	BlockRigidBody=False	

	Material=Material'avaTestEffect.bulletTrailMaterial'	
	bUseAsOccluder=FALSE

	Size = 0.5
	Speed = 16000.0
	AddDeltaTime = 0.01
}