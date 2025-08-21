class avaObjectPool extends Actor;

//
// KActors pool
//

/** Maximum number of KActors */
const MAX_KACTORS = 20;

struct KActorCacheDatum
{
	/** Index of the most recently used element */
	var	INT				ListIdx;
	/** List of cached elements */
	var	avaKActor_Debris List[MAX_KACTORS];
};

var KActorCacheDatum KActorCache;

/** Array of Static Meshes to cook physics for at level start up. */
var Array<StaticMesh>	CachedStaticMeshes;


//// Emitters for impact effects
const MAX_IMPACT_EMITTERS = 20;

const MAX_IMPACT_PARTICLE_SYSTEMS = 20;

struct EmitterCacheDatum
{
	var int Type;

	var ParticleSystem ParticleSystemType;

	/** Index of the most recently used tracer */
	var int ListIdx;

	/** List of ParticleSystemComponent this weapon uses, for recycling */
	var ParticleSystemComponent PList[MAX_IMPACT_PARTICLE_SYSTEMS];

	/** List of Emitter this weapon uses, for recycling */
	var Emitter List[MAX_IMPACT_EMITTERS];

};

enum EOPCacheTypes
{
	OP_DEFAULT_GENERAL,
	OP_BULLET_GENERAL,
	OP_BULLET_SMG,
	OP_BULLET_SNIPERRIFLE,
	OP_BULLET_PISTOL,
	OP_BULLET_RIFLE,
	OP_BULLET_SHOTGUN,
	OP_IMPACT_DIRT,
	OP_IMPACT_GROUND,
	OP_IMPACT_WATER,
	OP_IMPACT_WOOD,
	OP_IMPACT_CLOTH,
	OP_IMPACT_WOOD_OLD,	
	OP_IMPACT_GLASS,
	OP_IMPACT_ENFORCER,
	OP_IMPACT_SPARKS,
	OP_IMPACT_SHOTGUN_CLOTH,
	OP_IMPACT_SHOTGUN_DIRT,
	OP_IMPACT_SHOTGUN_GROUND,
	OP_IMPACT_SHOTGUN_METAL,
	OP_IMPACT_SHOTGUN_WATER,
	OP_IMPACT_SHOTGUN_WOOD,
	OP_FOOTSTEP_GENERAL,
	OP_BLOOD_GENERAL,	
	OP_BLOOD_SMALL,
	OP_BLOOD_HEAD,
	OP_BLOOD_TEEN,
	OP_GLASS_SPLATTER,
};

/** default impacts (assaultCOG, assaultLocust), blood impacts, general pool **/
var EmitterCacheDatum EmitterCache[EOPCacheTypes.EnumCount];


simulated event PreBeginPlay()
{	
	CreateImpactEffects();
	CreateDebris();
}


simulated event Destroyed()
{
	CleanUpPools();

	Super.Destroyed();
}



simulated final function CleanUpPools()
{
	local int Idx;
	local int Jdx;

	for( Idx = 0; Idx < EOPCacheTypes.EnumCount; ++Idx )
	{
		for( Jdx = 0; Jdx < MAX_IMPACT_EMITTERS; ++Jdx )
		{
			EmitterCache[Idx].List[Jdx].Destroy();
			EmitterCache[Idx].List[Jdx] = none;
		}
	}

	CleanUpKActors();
	// clean up the raindrops if we ever use them again
}


// NOTE:  we probably want to call this on first usage of an effect so we are datadriven
simulated final function CreateImpactEffects()
{
	local int Idx;

	//// emitters
	CreateEmitter_Worker( OP_BULLET_PISTOL, "avaEffect.Gun_Effect.Ps_Wp_Pistol_cartridge" );
	CreateEmitter_Worker( OP_BULLET_RIFLE, "avaEffect.Gun_Effect.Ps_Wp_Rifle_cartridge" );
	CreateEmitter_Worker( OP_BULLET_SNIPERRIFLE, "avaEffect.Gun_Effect.Ps_Wp_SniperRifle_cartridge" );
	CreateEmitter_Worker( OP_BULLET_SMG, "avaEffect.Gun_Effect.Ps_Wp_SMG_cartridge" );
	CreateEmitter_Worker( OP_BULLET_SHOTGUN, "avaEffect.Gun_Effect.PS_Wp_Shotgun_cartridge" );

	CreateEmitter_Worker( OP_IMPACT_DIRT, "avaEffect.Gun_Effect.PS_Gun_Impact_Dirt" );
	CreateEmitter_Worker( OP_IMPACT_GROUND, "avaEffect.Gun_Effect.PS_Gun_Impact_Ground" );
	CreateEmitter_Worker( OP_IMPACT_WATER, "avaEffect.Gun_Effect.PS_Gun_Impact_Water" );
	CreateEmitter_Worker( OP_IMPACT_WOOD, "avaEffect.Gun_Effect.PS_Gun_Impact_Wood" );
	CreateEmitter_Worker( OP_IMPACT_CLOTH, "avaEffect.Gun_Effect.PS_Gun_Impact_Cloth" );
	CreateEmitter_Worker( OP_IMPACT_WOOD_OLD, "avaEffect.Gun_Effect.PS_Gun_Impact_Wood_old" );	
	CreateEmitter_Worker( OP_IMPACT_GLASS, "avaEffect.Gun_Effect.PS_Glass_Splatter" );
	CreateEmitter_Worker( OP_IMPACT_ENFORCER, "avaEffect.Gun_Effect.P_WP_Enforcer_Impact" );
	CreateEmitter_Worker( OP_IMPACT_SPARKS, "avaEffect.Gun_Effect.PS_Sparks_Metal01" );
	CreatePS_Worker( OP_IMPACT_SPARKS, "avaEffect.Gun_Effect.PS_Sparks_Metal01" );

	CreateEmitter_Worker( OP_IMPACT_SHOTGUN_CLOTH,	"avaEffect2.Gun_Effect2.PS_ShotGun_Impact_Cloth" );
	CreateEmitter_Worker( OP_IMPACT_SHOTGUN_DIRT,	"avaEffect2.Gun_Effect2.PS_ShotGun_Impact_Dirt" );
	CreateEmitter_Worker( OP_IMPACT_SHOTGUN_GROUND, "avaEffect2.Gun_Effect2.PS_ShotGun_Impact_Ground" );
	CreateEmitter_Worker( OP_IMPACT_SHOTGUN_METAL,	"avaEffect2.Gun_Effect2.PS_ShotGun_Impact_Metal" );
	CreateEmitter_Worker( OP_IMPACT_SHOTGUN_WATER,	"avaEffect2.Gun_Effect2.PS_ShotGun_Impact_Water" );
	CreateEmitter_Worker( OP_IMPACT_SHOTGUN_WOOD,	"avaEffect2.Gun_Effect2.PS_ShotGun_Impact_Wood" );


	//// particle system component
	//CreatePS_Worker(OP_BLOOD_LOD,				ParticleSystem'War_Blood.Effects.P_Blood_Full_LOD');
	//CreatePS_Worker(OP_BLOOD_METAL_LOD,		ParticleSystem'War_Blood.Effects.P_Blood_Full_LOD_Metal');
	//CreatePS_Worker(OP_BLOOD_ARMOR_LOD,		ParticleSystem'War_Blood.Effects.P_Blood_Full_LOD_Armor');
	//CreatePS_Worker(OP_BLOOD_DEATH_LOD,		ParticleSystem'War_Blood.Effects.P_Blood_Full_LOD_Death');
	//CreatePS_Worker(OP_BLOOD_FLYING_BODY_PART,	ParticleSystem'War_Effects.Blood.P_Blood_flying_bodypart');
	//CreatePS_Worker(OP_BLOOD_SPRAY_HIT_EFFECT,	ParticleSystem'War_Effects.Blood.P_Bloodspray_hit_effect');
	//CreatePS_Worker(OP_BLOOD_HEADSHOT,			ParticleSystem'War_Blood.Effects.P_Blood_HeadShot');
	//CreatePS_Worker(OP_NO_BLOOD,			    ParticleSystem'War_Blood.Effects.P_Hit_Effect_No_Blood');
	//CreatePS_Worker(OP_BERSERKER_IMPACT,	    ParticleSystem'Geist_Berserker.Particles.P_Berserker_Impact_Weapon_Sparks');

	CreateEmitter_Worker(OP_BLOOD_GENERAL, "avaEffect.Particles.PS_BloodSpurt" );
	CreateEmitter_Worker(OP_BLOOD_HEAD, "avaEffect.Particles.PS_BloodSpurt_Head" );
	CreateEmitter_Worker(OP_BLOOD_SMALL, "avaEffect.Particles.PS_BloodSpurt_Small" );
	CreateEmitter_Worker(OP_BLOOD_TEEN, "avaEffect.Particles.PS_BloodSpurt_Teen" );	
	CreatePS_Worker(OP_BLOOD_GENERAL, "avaEffect.Particles.PS_BloodSpurt" );
	CreatePS_Worker(OP_BLOOD_HEAD, "avaEffect.Particles.PS_BloodSpurt_Head" );
	CreatePS_Worker(OP_BLOOD_SMALL, "avaEffect.Particles.PS_BloodSpurt_Small" );
	CreatePS_Worker(OP_BLOOD_TEEN, "avaEffect.Particles.PS_BloodSpurt_Teen" );	
	CreatePS_Worker(OP_GLASS_SPLATTER, "avaEffect.Gun_Effect.PS_Glass_Splatter" );
	//CreatePS_Worker(OP_BLOOD_MELEE,	        ParticleSystem'War_Blood.Effects.P_Melee_Blood');

	// general other impacts
	for( Idx = 0; Idx < MAX_IMPACT_EMITTERS; ++Idx )
	{
		EmitterCache[OP_DEFAULT_GENERAL].List[Idx] = Spawn( class'avaSpawnedEmitter' );
		EmitterCache[OP_DEFAULT_GENERAL].List[Idx].ParticleSystemComponent.bAutoActivate = FALSE;
		avaEmitter(EmitterCache[OP_DEFAULT_GENERAL].List[Idx]).HideSelf();
		EmitterCache[OP_DEFAULT_GENERAL].List[Idx].ParticleSystemComponent.LODMethod = PARTICLESYSTEMLODMETHOD_DirectSet;

		EmitterCache[OP_BULLET_GENERAL].List[Idx] = Spawn( class'avaSpawnedEmitter' );
		EmitterCache[OP_BULLET_GENERAL].List[Idx].ParticleSystemComponent.bAutoActivate = FALSE;
		avaEmitter(EmitterCache[OP_BULLET_GENERAL].List[Idx]).HideSelf();
		EmitterCache[OP_BULLET_GENERAL].List[Idx].ParticleSystemComponent.LODMethod = PARTICLESYSTEMLODMETHOD_DirectSet;

		EmitterCache[OP_FOOTSTEP_GENERAL].List[Idx] = Spawn( class'avaSpawnedEmitter' );
		EmitterCache[OP_FOOTSTEP_GENERAL].List[Idx].ParticleSystemComponent.bAutoActivate = FALSE;
		avaEmitter(EmitterCache[OP_FOOTSTEP_GENERAL].List[Idx]).HideSelf();
		EmitterCache[OP_FOOTSTEP_GENERAL].List[Idx].ParticleSystemComponent.LODMethod = PARTICLESYSTEMLODMETHOD_DirectSet;
	}
}


simulated final function CreateEmitter_Worker(int EffectType, string PSTemplateName)
{
	local int	Idx, Jdx;
	local ParticleSystem PSTemplate;

	PSTemplate = ParticleSystem( DynamicLoadObject( PSTemplateName, class'ParticleSystem' ) );

	if (PSTemplate == none)
	{
		`log( "ParticleSystem NOT EXIST: " $ PSTemplateName);
	}

	PSTemplate.bUseFixedRelativeBoundingBox = TRUE;
	//switch (EffectType)
	//{
	//case OP_DEFAULT_IMPACT_STONE:
	//case OP_DEFAULT_IMPACT_DIRT:
	//case OP_DEFAULT_IMPACT_METAL:
	//case OP_DEFAULT_IMPACT_METAL_HOLLOW:
	//case OP_DEFAULT_IMPACT_GLASS:
	//case OP_DEFAULT_IMPACT_LOCUST_HAMMERBURST:
	//case OP_DEFAULT_SHOTGUN:
	//case OP_DEFAULT_SHOTGUN_METAL:
	//case OP_DEFAULT_P_TroikaCabal_Impact_Stone:
	//case OP_DEFAULT_IMPACT_FABRIC:
	//case OP_COVER_HIT:
	//case OP_PLAYER_SLIDE:
	//case OP_PLAYER_ROLL:
	//case OP_BLOOD_MELEE_EMITTER:
	//	PSTemplate.FixedRelativeBoundingBox.IsValid = 1;
	//	PSTemplate.FixedRelativeBoundingBox.Min.X = -64;
	//	PSTemplate.FixedRelativeBoundingBox.Min.Y = -64;
	//	PSTemplate.FixedRelativeBoundingBox.Min.Z = -64;
	//	PSTemplate.FixedRelativeBoundingBox.Max.X =  64;
	//	PSTemplate.FixedRelativeBoundingBox.Max.Y =  64;
	//	PSTemplate.FixedRelativeBoundingBox.Max.Z =  64;
	//	break;
	//}

	for( Idx=0; Idx<MAX_IMPACT_EMITTERS; Idx++)
	{
		EmitterCache[EffectType].List[Idx] = Spawn( class'avaSpawnedEmitter' );
		EmitterCache[EffectType].List[Idx].LifeSpan = 0;
		avaEmitter(EmitterCache[EffectType].List[Idx]).HideSelf();
		EmitterCache[EffectType].List[Idx].ParticleSystemComponent.bAutoActivate = FALSE;
		EmitterCache[EffectType].List[Idx].SetTemplate( PSTemplate, FALSE );


		EmitterCache[EffectType].ParticleSystemType									= PSTemplate;
		EmitterCache[EffectType].List[Idx].ParticleSystemComponent.LODMethod		= PARTICLESYSTEMLODMETHOD_DirectSet;

		for( Jdx=0; Jdx<EmitterCache[EffectType].List[Idx].ParticleSystemComponent.EmitterInstances.Length; Jdx++)
		{
			EmitterCache[EffectType].List[Idx].ParticleSystemComponent.SetKillOnDeactivate(Jdx,FALSE);
			EmitterCache[EffectType].List[Idx].ParticleSystemComponent.SetKillOnCompleted(Jdx,FALSE);
		}
	}
}

simulated final function CreatePS_Worker(int EffectType, string PSTemplateName)
{
	local int	Idx, Jdx;

	local ParticleSystem PSTemplate;

	PSTemplate = ParticleSystem( DynamicLoadObject( PSTemplateName, class'ParticleSystem' ) );

	if (PSTemplate == none)
	{
		`log( "ParticleSystem NOT EXIST: " $ PSTemplateName);
	}

	PSTemplate.bUseFixedRelativeBoundingBox = TRUE;
	//switch (EffectType)
	//{
	//case OP_BLOOD_LOD:
	//case OP_BLOOD_METAL_LOD:
	//case OP_BLOOD_ARMOR_LOD:
	//case OP_BLOOD_DEATH_LOD:
	//case OP_BLOOD_GENERAL:
	//case OP_BLOOD_FLYING_BODY_PART:
	//case OP_BLOOD_SPRAY_HIT_EFFECT:
	//case OP_BLOOD_HEADSHOT:
	//case OP_BERSERKER_IMPACT:
	//case OP_NO_BLOOD:
	//case OP_BLOOD_MELEE:
	//	PSTemplate.FixedRelativeBoundingBox.IsValid = 1;
	//	PSTemplate.FixedRelativeBoundingBox.Min.X = -64;
	//	PSTemplate.FixedRelativeBoundingBox.Min.Y = -64;
	//	PSTemplate.FixedRelativeBoundingBox.Min.Z = -64;
	//	PSTemplate.FixedRelativeBoundingBox.Max.X =  64;
	//	PSTemplate.FixedRelativeBoundingBox.Max.Y =  64;
	//	PSTemplate.FixedRelativeBoundingBox.Max.Z =  64;
	//	break;
	//}


	for( Idx=0; Idx<MAX_IMPACT_PARTICLE_SYSTEMS; Idx++ )
	{
		EmitterCache[EffectType].PList[Idx] = new(Outer) class'ParticleSystemComponent';
		EmitterCache[EffectType].PList[Idx].bAutoActivate = FALSE;
		EmitterCache[EffectType].PList[Idx].SetTemplate( PSTemplate );

		EmitterCache[EffectType].ParticleSystemType					= PSTemplate;
		EmitterCache[EffectType].PList[Idx].LODMethod				= PARTICLESYSTEMLODMETHOD_DirectSet;
		EmitterCache[EffectType].PList[Idx].bIsCachedInPool			= TRUE;		
		EmitterCache[EffectType].PList[Idx].SetTickGroup( TG_PostAsyncWork ); // this is needed to make certain ParticleSystemComponents get attached at the correct location on physicy skeletal meshes

		for( Jdx=0; Jdx<EmitterCache[EffectType].PList[Idx].EmitterInstances.Length; Jdx++ )
		{
			EmitterCache[EffectType].PList[Idx].SetKillOnDeactivate(Jdx,FALSE);
			EmitterCache[EffectType].PList[Idx].SetKillOnCompleted(Jdx,FALSE);
		}
	}
}

/**
 * This will retrieve an Emitter from our pool.
 *
 * NOTE: turn this into an object pool where we cache the various types of particle systems
 **/
simulated final function Emitter GetImpactEmitter(ParticleSystem PS_Type, vector SpawnLocation, rotator SpawnRotation)
{
	if (PS_Type.Name == 'PS_Gun_Impact_Dirt')
	{
		return GetImpactEmitter_Worker( EmitterCache[OP_IMPACT_DIRT], SpawnLocation, SpawnRotation, none );
	}
	else if (PS_Type.Name == 'PS_Gun_Impact_Ground')
	{
		return GetImpactEmitter_Worker( EmitterCache[OP_IMPACT_GROUND], SpawnLocation, SpawnRotation, none );
	}
	else if (PS_Type.Name == 'PS_Gun_Impact_Water')
	{
		return GetImpactEmitter_Worker( EmitterCache[OP_IMPACT_WATER], SpawnLocation, SpawnRotation, none );
	}
	else if (PS_Type.Name == 'PS_Gun_Impact_Wood')
	{
		return GetImpactEmitter_Worker( EmitterCache[OP_IMPACT_WOOD], SpawnLocation, SpawnRotation, none );
	}
	else if (PS_Type.Name == 'PS_Gun_Impact_Cloth')
	{
		return GetImpactEmitter_Worker( EmitterCache[OP_IMPACT_CLOTH], SpawnLocation, SpawnRotation, none );
	}
	else if (PS_Type.Name == 'PS_Gun_Impact_Wood_old')
	{
		return GetImpactEmitter_Worker( EmitterCache[OP_IMPACT_WOOD_OLD], SpawnLocation, SpawnRotation, none );
	}
	else if (PS_Type.Name == 'PS_Glass_Splatter')
	{
		return GetImpactEmitter_Worker( EmitterCache[OP_IMPACT_GLASS], SpawnLocation, SpawnRotation, none );
	}
	else if (PS_Type.Name == 'P_WP_Enforcer_Impact')
	{
		return GetImpactEmitter_Worker( EmitterCache[OP_IMPACT_ENFORCER], SpawnLocation, SpawnRotation, none );
	}
	else if (PS_Type.Name == 'PS_Sparks_Metal01')
	{
		return GetImpactEmitter_Worker( EmitterCache[OP_IMPACT_SPARKS], SpawnLocation, SpawnRotation, none );
	}
	else if( PS_Type.Name == 'PS_BloodSpurt' )
	{
		return GetImpactEmitter_Worker( EmitterCache[OP_BLOOD_GENERAL], SpawnLocation, SpawnRotation, none );
	}	
	else if( PS_Type.Name == 'PS_BloodSpurt_Head' )
	{
		return GetImpactEmitter_Worker( EmitterCache[OP_BLOOD_HEAD], SpawnLocation, SpawnRotation, none );
	}	
	else if( PS_Type.Name == 'PS_BloodSpurt_Small' )
	{
		return GetImpactEmitter_Worker( EmitterCache[OP_BLOOD_SMALL], SpawnLocation, SpawnRotation, none );
	}	
	else if( PS_Type.Name == 'PS_BloodSpurt_Teen' )
	{
		return GetImpactEmitter_Worker( EmitterCache[OP_BLOOD_TEEN], SpawnLocation, SpawnRotation, none );
	}	
	else if( PS_Type.Name == 'PS_Glass_Splatter')
	{
		return GetImpactEmitter_Worker( EmitterCache[OP_GLASS_SPLATTER], SpawnLocation, SpawnRotation, none );
	}
	else if( PS_Type.Name == 'PS_ShotGun_Impact_Cloth')
	{
		return GetImpactEmitter_Worker( EmitterCache[OP_IMPACT_SHOTGUN_CLOTH], SpawnLocation, SpawnRotation, none );
	}
	else if( PS_Type.Name == 'PS_ShotGun_Impact_Dirt')
	{
		return GetImpactEmitter_Worker( EmitterCache[OP_IMPACT_SHOTGUN_DIRT], SpawnLocation, SpawnRotation, none );
	}
	else if( PS_Type.Name == 'PS_ShotGun_Impact_Ground')
	{
		return GetImpactEmitter_Worker( EmitterCache[OP_IMPACT_SHOTGUN_GROUND], SpawnLocation, SpawnRotation, none );
	}
	else if( PS_Type.Name == 'PS_ShotGun_Impact_Metal')
	{
		return GetImpactEmitter_Worker( EmitterCache[OP_IMPACT_SHOTGUN_METAL], SpawnLocation, SpawnRotation, none );
	}
	else if( PS_Type.Name == 'PS_ShotGun_Impact_Water')
	{
		return GetImpactEmitter_Worker( EmitterCache[OP_IMPACT_SHOTGUN_WATER], SpawnLocation, SpawnRotation, none );
	}
	else if( PS_Type.Name == 'PS_ShotGun_Impact_Wood')
	{
		return GetImpactEmitter_Worker( EmitterCache[OP_IMPACT_SHOTGUN_WOOD], SpawnLocation, SpawnRotation, none );
	}
	else
	{
		`log( "ParticleSystem NOT CACHED: GetImpactEmitter PS_Type: " $ PS_Type );
		return GetImpactEmitter_Worker( EmitterCache[OP_DEFAULT_GENERAL], SpawnLocation, SpawnRotation, PS_Type );
	}
}

simulated final function Emitter GetBulletEmitter(ParticleSystem PS_Type, vector SpawnLocation, rotator SpawnRotation)
{
	/// priority :) rifle > smg > pistol > sniper (deif ¸¾)
	if (PS_Type.Name == 'Ps_Wp_Rifle_cartridge')
	{
		return GetImpactEmitter_Worker( EmitterCache[OP_BULLET_RIFLE], SpawnLocation, SpawnRotation, none );
	}
	else if (PS_Type.Name == 'Ps_Wp_SMG_cartridge')
	{
		return GetImpactEmitter_Worker( EmitterCache[OP_BULLET_SMG], SpawnLocation, SpawnRotation, none );
	}
	else if (PS_Type.Name == 'Ps_Wp_shotgun_cartridge' )
	{
		return GetImpactEmitter_Worker( EmitterCache[OP_BULLET_SHOTGUN], SpawnLocation, SpawnRotation, none );
	}
	else if (PS_Type.Name == 'Ps_Wp_Pistol_cartridge')
	{
		return GetImpactEmitter_Worker( EmitterCache[OP_BULLET_PISTOL], SpawnLocation, SpawnRotation, none );
	}
	else if (PS_Type.Name == 'Ps_Wp_SniperRifle_cartridge')
	{
		return GetImpactEmitter_Worker( EmitterCache[OP_BULLET_SNIPERRIFLE], SpawnLocation, SpawnRotation, none );
	}	
	else
	{
		`log( "ParticleSystem NOT CACHED: GetBulletEmitter PS_Type: " $ PS_Type );
		return GetImpactEmitter_Worker( EmitterCache[OP_BULLET_GENERAL], SpawnLocation, SpawnRotation, PS_Type );
	}
}

simulated final function Emitter GetFootstepEmitter(ParticleSystem PS_Type, vector SpawnLocation, rotator SpawnRotation)
{
	if (false)
	{
	}
	else
	{
		`log( "ParticleSystem NOT CACHED: GetFootstepEmitter PS_Type: " $ PS_Type );
		return GetImpactEmitter_Worker( EmitterCache[OP_FOOTSTEP_GENERAL], SpawnLocation, SpawnRotation, PS_Type );
	}
}

simulated final function Emitter GetImpactEmitter_Worker( out EmitterCacheDatum TheDatum, vector SpawnLocation, rotator SpawnRotation, ParticleSystem PS_Type )
//client function ParticleSystemComponent GetImpactParticleSystemComponent_Worker( out EmitterCacheDatum TheDatum, vector SpawnLocation, rotator SpawnRotation, ParticleSystem PS_Type )
{
	local int Jdx;

	if( ++TheDatum.ListIdx >= MAX_IMPACT_EMITTERS )
	{
		TheDatum.ListIdx = 0;
	}
	TheDatum.List[TheDatum.ListIdx].bStasis = FALSE;

	TheDatum.List[TheDatum.ListIdx].SetBase( none );

	//TheDatum.List[TheDatum.ListIdx].SetAbsolute( TRUE, TRUE, FALSE );
	//TheDatum.List[TheDatum.ListIdx].SetTranslation( SpawnLocation );
	//TheDatum.List[TheDatum.ListIdx].SetRotation( SpawnRotation );

	TheDatum.List[TheDatum.ListIdx].SetLocation( SpawnLocation );
	TheDatum.List[TheDatum.ListIdx].SetRotation( SpawnRotation );

	// we check to see if we are getting a non default impact and then if we are
	// we have to SetTemplate on it as the set of all none defaults share their
	// own cache
	if( PS_Type != none )
	{
		TheDatum.List[TheDatum.ListIdx].SetTemplate( PS_Type, FALSE ); // this will ActivateSystem if you do not explicitly set ParticleSystemComponent.bAutoActivate = FALSE;

		for( Jdx = 0; Jdx < TheDatum.List[TheDatum.ListIdx].ParticleSystemComponent.EmitterInstances.Length; ++Jdx )
		{
			TheDatum.List[TheDatum.ListIdx].ParticleSystemComponent.SetKillOnDeactivate(Jdx,FALSE);
			TheDatum.List[TheDatum.ListIdx].ParticleSystemComponent.SetKillOnCompleted(Jdx,FALSE);
		}

	}	

	//`log( "GetImpactEmitter_Worker: " $ TheDatum.ListIdx $ " PS_Type: " $ PS_Type );
	TheDatum.List[TheDatum.ListIdx].ClearTimer( 'HideSelf' );
	//TheDatum.List[TheDatum.ListIdx].ParticleSystemComponent.bIsCachedInPool = TRUE;
	TheDatum.List[TheDatum.ListIdx].SetHidden(FALSE);	
	TheDatum.List[TheDatum.ListIdx].SetDrawScale( 1.0 ); // reset this in case caller changed it
	TheDatum.List[TheDatum.ListIdx].ParticleSystemComponent.bAllowApproximateOcclusion = TRUE;
	TheDatum.List[TheDatum.ListIdx].ParticleSystemComponent.SetLODLevel(avaGameReplicationInfo(WorldInfo.GRI).GetLODLevelToUse( TheDatum.List[TheDatum.ListIdx].ParticleSystemComponent.Template, SpawnLocation) );

	return TheDatum.List[TheDatum.ListIdx];
}


simulated final function ParticleSystemComponent GetImpactParticleSystemComponent( ParticleSystem PS_Type )
{
	if( PS_Type.Name == 'PS_Sparks_Metal01' )
	{
		return GetImpactParticleSystemComponent_Worker( EmitterCache[OP_IMPACT_SPARKS], none );
	}
	else if( PS_Type.Name == 'PS_BloodSpurt' )
	{
		return GetImpactParticleSystemComponent_Worker( EmitterCache[OP_BLOOD_GENERAL], none );
	}	
	else if( PS_Type.Name == 'PS_BloodSpurt_Head' )
	{
		return GetImpactParticleSystemComponent_Worker( EmitterCache[OP_BLOOD_HEAD], none );
	}	
	else if( PS_Type.Name == 'PS_BloodSpurt_Small' )
	{
		return GetImpactParticleSystemComponent_Worker( EmitterCache[OP_BLOOD_SMALL], none );
	}	
	else if( PS_Type.Name == 'PS_BloodSpurt_Teen' )
	{
		return GetImpactParticleSystemComponent_Worker( EmitterCache[OP_BLOOD_TEEN], none );
	}	
	else
	{
		`log( "ParticleSystem NOT CACHED: ParticleSystemComponent PS_Type: " $ PS_Type );
		return GetImpactParticleSystemComponent_Worker( EmitterCache[OP_BLOOD_GENERAL], PS_Type );
	}
}


// ???
// need to do this as the particle components are attached to the mob
simulated final function ParticleSystemComponent GetImpactParticleSystemComponent_Worker( out EmitterCacheDatum TheDatum, ParticleSystem PS_Type )
{
	local ParticleSystemComponent PSC;
	local int Jdx;

	if( ++TheDatum.ListIdx >= MAX_IMPACT_EMITTERS )
	{
		TheDatum.ListIdx = 0;
	}

	PSC = TheDatum.PList[TheDatum.ListIdx];

	// Spawn a new component if the one in the cache has been destroyed. This can happen if the actor the component is attached to
	// gets destroyed and henceforth is marked as pending kill.
	if( PSC == none || PSC.IsPendingKill() )
	{
		//`log( "newing a PSC of type: " $  TheDatum.ParticleSystemType  $ " PSC: " $ PSC.Template );
		PSC = new(Outer) class'ParticleSystemComponent';
		// Replace entry in cache.
		TheDatum.PList[TheDatum.ListIdx] = PSC;

		PSC.LODMethod = PARTICLESYSTEMLODMETHOD_DirectSet;
		PSC.bIsCachedInPool = TRUE;

		PSC.SetTemplate( TheDatum.ParticleSystemType );
		PSC.bAutoActivate = FALSE;
	}

	// Detach from existing actor if we're re-using.
	if( PSC.Owner != none )
	{
		//`log( "Detaching component from: " $ PSC.Owner $ " PSC: " $ PSC.Template );
		PSC.Owner.DetachComponent( PSC );
	}

	PSC.bAllowApproximateOcclusion = TRUE;
	PSC.SetHidden( FALSE );

	if( PS_Type != none )
	{
		//`log( "GetImpactParticleSystemComponent_Worker:  having to SetTemplate :-(  Previous: " $ PSC.Template $ " New: " $ PS_Type );
		PSC.SetTemplate( PS_Type );

		for( Jdx = 0; Jdx < PSC.EmitterInstances.Length; ++Jdx )
		{
			PSC.SetKillOnDeactivate(Jdx,FALSE);
			PSC.SetKillOnCompleted(Jdx,FALSE);
		}

	}

	//PSC.OnSystemFinished = OnParticleSystemFinished_Hider;


	return PSC;
}





//
// KActors Pool
//

simulated final function CreateDebris()
{
	local int				Idx;
	local vector			SpawnLocation;

	// 262144 is HALF_WORLD_MAX @see engine.h
	// reduced a bit from that so that it doesn't get destroyed due to being out of the world
	SpawnLocation = vect(250000,250000,250000);

	for( Idx=0; Idx<MAX_KACTORS; Idx++ )
	{
		if (KActorCache.List[Idx] == None)
		{
			KActorCache.List[Idx] = Spawn( class'avaKActor_Debris',,, SpawnLocation );
			//KActorCache.List[Idx].RemoteRole = ROLE_None;
			//KActorCache.List[Idx].SetCollision(false, false); // Don't let these actors block players
			//KActorCache.List[Idx].bCollideWorld = TRUE;
			//KActorCache.List[Idx].bBlocksNavigation = FALSE;
			////KActorCache.List[Idx].LightEnvironment.bEnabled = TRUE;
			//KActorCache.List[Idx].StaticMeshComponent.CastShadow = FALSE; // turn off shadow casting fo perf
			KActorCache.List[Idx].Recycle();
		}
	}
}


simulated final function avaKActor_Debris GetDebris(Vector SpawnLocation, Rotator SpawnRotation)
{
	local avaKActor_Debris KA;

	if( ++KActorCache.ListIdx >= MAX_KACTORS )
	{
		KActorCache.ListIdx = 0;
	}

	if (KActorCache.List[KActorCache.ListIdx] == None)
	{
		CreateDebris();
	}

	KA = KActorCache.List[KActorCache.ListIdx];

	if( KA != None )
	{
		KA.bCollideWorld = FALSE;
		KA.bCollideWhenPlacing = FALSE;

		KA.SetLocation(SpawnLocation);
		KA.SetRotation(SpawnRotation);
		
		//KA.ResetComponents();
		KA.InitializeForPooling();
		
		KA.bCollideWorld = TRUE;
		KA.bCollideWhenPlacing = TRUE;
	}

	return KA;
}


simulated final function CleanUpKActors()
{
	local int Idx;

	for( Idx=0; Idx<MAX_KACTORS; Idx++ )
	{
		KActorCache.List[Idx].Destroy();
		KActorCache.List[Idx] = none;
	}
}


defaultproperties
{
	bHidden=TRUE
	bMovable=FALSE		
}
