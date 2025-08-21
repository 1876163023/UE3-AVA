class avaProp extends KActor placeable native;

struct native PropDamageInfo
{
	var() int DamageMin, DamageMax;	
	var() StaticMeshComponent Mesh;
	var() SoundCue Sound;
	var() ParticleSystemComponent Particle;
};

var(Prop) editinline array<PropDamageInfo> DamageInfos;
var(Prop) int Health;
var(Prop) bool Useable;
var(Prop) SoundCue RollingSound;

var protected AudioComponent RollingSoundComponent;
var float RollTime;
var int CurrentDamage;

cpptext
{	
	virtual void OnRigidBodyCollision(const FRigidBodyCollisionInfo& Info0, const FRigidBodyCollisionInfo& Info1, const FCollisionImpactData& RigidCollisionData);
}

event function StartRolling()
{	
}

event TakeDamage( int Damage, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional class<Weapon> weapon)
{
	CurrentDamage += Damage;	

	super.TakeDamage( Damage, EventInstigator, HitLocation, Momentum, DamageType, HitInfo, weapon );
}

simulated function RollingSoundFinished( AudioComponent AC )
{
}

defaultproperties
{
	Health=100
	Begin Object Name=StaticMeshComponent0
	    bNotifyRigidBodyCollision=True
	End Object
}