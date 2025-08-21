class avaPickUp_Indicator extends Actor;

var()	SkeletalMeshComponent	Mesh;		// Indicator 용 Mesh
var()	name					IdleAnim;
var()	int						IndicatorType;

var		array<Material>			IndicatorMaterial;

simulated function Init()
{
	// IndicatorType 이 0 이면 Default 값이기 때문에 바꿔줄 필요는 없다.
	if ( IndicatorType < IndicatorMaterial.length && IndicatorType > 0 )	
		Mesh.SetMaterial( 0, IndicatorMaterial[IndicatorType] );
	Mesh.PlayAnim( IdleAnim,, true );
}

defaultproperties
{

	IdleAnim = arrow_Idle

	Begin Object class=AnimNodeSequence Name=SequenceA
	End Object

	Begin Object Class=SkeletalMeshComponent Name=IndicatorComponent
		SkeletalMesh=SkeletalMesh'avaHUD.Prop.Arrow'
		Animations=SequenceA
		PhysicsAsset=None
		AnimSets(0)=AnimSet'avaHUD.Prop.AnimSet_0'
		CastShadow=false
		CollideActors=false
		bUpdateSkelWhenNotRendered=false		
		bCastDynamicShadow=false
		bUseAsOccluder=false
	End Object
	Mesh=IndicatorComponent
	Components.Add(IndicatorComponent)

	bNoDelete=false
	bCollideActors=false
	bBlockActors=false
	Physics=PHYS_None
	RemoteRole=ROLE_None

	DrawScale = 0.7
	PrePivot	= (z=-10)

	IndicatorMaterial(0)	=	Material'avaHUD.Prop.MT_Arrow_Yellow'
	IndicatorMaterial(1)	=	Material'avaHUD.Prop.MT_Arrow_Blue'
	IndicatorMaterial(2)	=	Material'avaHUD.Prop.MT_Arrow_Green'
}