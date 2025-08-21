class avaPhysicalMaterialProperty extends PhysicalMaterialPropertyBase
	native;

/** Type of material this is (dirt, gravel, brick, etc) used for looking up material specific effects */
var() name MaterialType;

var() float				PenetrationDamper;
var() float				DamageModifier;
var() SoundCue			FootstepLeftSound, FootstepRightSound;
var() SoundCue			JumpSound, LandSound;
var() ParticleSystem	FootstepParticle;

enum DecalAlignType
{
	DECAL_ALIGN_NORMAL,
	DECAL_ALIGN_VIEW,
	DECAL_ALIGN_RANDOM,
};

/** struct for list to map material types supported by an actor to impact sounds and effects */

struct native ImpactDecalData
{	
	var() MaterialInstance DecalMaterial;
	var() float DecalWidth, DecalHeight;
};

struct native MaterialImpactEffect
{
	var() name DamageCode;	
	var() SoundCue Sound;
	var MaterialInstance DecalMaterial;
	var float DecalWidth, DecalHeight;
	var() ParticleSystem ParticleTemplate;
	var() DecalAlignType	decalAlignType;

	var() editinline array<ImpactDecalData> ImpactDecals;
};

/** Struct for list to map materials to sounds, for sound only applications (e.g. tires) */
struct native MaterialSoundEffect
{
	var name MaterialType;
	var SoundCue Sound;
};

/** Struct for list to map materials to a particle effect */
struct native MaterialParticleEffect
{
	var name MaterialType;
	var ParticleSystem ParticleTemplate;
};

var() editinline array<MaterialImpactEffect> ImpactEffects;

cpptext
{
	virtual void PostLoad();
}

defaultproperties
{
	PenetrationDamper = 1.0
	DamageModifier = 0.6
}