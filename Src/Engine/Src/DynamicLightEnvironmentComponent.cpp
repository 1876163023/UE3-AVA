/**
* Copyright 1998-2007 Epic Games, Inc. All Rights Reserved.
*/

#include "EnginePrivate.h"

IMPLEMENT_CLASS(UDynamicLightEnvironmentComponent);

/**
* The character lighting stats.
*/
enum ECharacterLightingStats
{
	STAT_LightVisibilityTime = STAT_CharacterLightingFirstStat,
	STAT_UpdateEnvironmentTime,
	STAT_CreateLightsTime,
	STAT_NumEnvironments,
	STAT_EnvironmentUpdates,
	STAT_EnvironmentFullUpdates,
	STAT_StaticEnvironmentFullUpdates,
	STAT_StaticEnvironmentFullUpdatesSkipped,
};

DECLARE_STATS_GROUP(TEXT("CharacterLighting"),STATGROUP_CharacterLighting);
DECLARE_CYCLE_STAT(TEXT("Light visibility time"),STAT_LightVisibilityTime,STATGROUP_CharacterLighting);
DECLARE_CYCLE_STAT(TEXT("UpdateEnvironment time"),STAT_UpdateEnvironmentTime,STATGROUP_CharacterLighting);
DECLARE_CYCLE_STAT(TEXT("CreateLights time"),STAT_CreateLightsTime,STATGROUP_CharacterLighting);
DECLARE_DWORD_COUNTER_STAT(TEXT("Light Environments"),STAT_NumEnvironments,STATGROUP_CharacterLighting);
DECLARE_DWORD_COUNTER_STAT(TEXT("Environment Updates"),STAT_EnvironmentUpdates,STATGROUP_CharacterLighting);
DECLARE_DWORD_COUNTER_STAT(TEXT("Environment Full Updates"),STAT_EnvironmentFullUpdates,STATGROUP_CharacterLighting);
DECLARE_DWORD_COUNTER_STAT(TEXT("Static Env Full Updates"),STAT_StaticEnvironmentFullUpdates,STATGROUP_CharacterLighting);
DECLARE_DWORD_COUNTER_STAT(TEXT("Static Env Full Updates Skipped"),STAT_StaticEnvironmentFullUpdatesSkipped,STATGROUP_CharacterLighting);

/** The private light environment state. */
class FDynamicLightEnvironmentState
{
public:

	/** Initialization constructor. */
	FDynamicLightEnvironmentState(UDynamicLightEnvironmentComponent* InComponent);

	/** Updates the light environment. */
	void UpdateEnvironment(FLOAT DeltaTime,UBOOL bPerformFullUpdate,UBOOL bForceStaticLightUpdate);

	/** Updates the light environment's state. */
	void Tick(FLOAT DeltaTime);

	/** Creates a light to represent a light environment. */
	UBOOL CreateRepresentativeLight(FSHVectorRGB& RemainingLightEnvironment,UBOOL bCastLight,UBOOL bCastShadows, FLOAT MinIntensity = 0.0f) const;

	/** Detaches the light environment's representative lights. */
	void DetachRepresentativeLights() const;

	/** Creates the lights to represent the character's light environment. */
	void CreateEnvironmentLightList() const;

	/** Builds a list of objects referenced by the state. */
	void AddReferencedObjects(TArray<UObject*>& ObjectArray);

	/** Sets a flag which forces a full update of the light environment's static lighting on the next tick. */
	void BeginDeferredStaticLightingUpdate();

private:

	/** The component which this is state for. */
	UDynamicLightEnvironmentComponent* Component;

	/** The bounds of the owner. */
	FBoxSphereBounds OwnerBounds;

	/** The predicted center of the owner at the time of the next update. */
	FVector PredictedOwnerPosition;

	/** The lighting channels the owner's primitives are affected by. */
	FLightingChannelContainer OwnerLightingChannels;

	/** The time the light environment was last updated. */
	FLOAT LastUpdateTime;

	/** Time between updates for invisible objects. */
	FLOAT InvisibleUpdateTime;

	/** Min time between full environment updates. */
	FLOAT MinTimeBetweenFullUpdates;

	/** A pool of unused light components. */
	mutable TArray<ULightComponent*> RepresentativeLightPool;

	/** The character's current static light environment. */
	FSHVectorRGB StaticLightEnvironment;
	/** The current static shadow environment. */
	FSHVectorRGB StaticShadowEnvironment;
	/** */
	FSHVectorRGB AmbientLightEnvironment;

	FAmbientSH AmbientCube;

	/** The current dynamic light environment. */
	FSHVectorRGB DynamicLightEnvironment;
	/** The current dynamic shadow environment. */
	FSHVectorRGB DynamicShadowEnvironment;

	/** New static light environment to interpolate to. */
	FSHVectorRGB NewStaticLightEnvironment;
	/** New static shadow environment to interpolate to. */
	FSHVectorRGB NewStaticShadowEnvironment;
	/** */
	FSHVectorRGB NewAmbientLightEnvironment;

	/** The dynamic lights which affect the owner. */
	TArray<ULightComponent*> DynamicLights;

	/** Whether light environment has been fully updated at least once. */
	UBOOL bFirstFullUpdate;

	/** Should use Directional shadow source? */
	UBOOL bDirectionalShadow;
	UBOOL bSunVisible, bPointLightVisible;

	//<@ ava specific ; 2008. 1. 7 changmin
	// add cascaded shadow
	UBOOL bCascadedShadowVisible;
	//>@ ava

	/** The positions relative to the owner's bounding box which are sampled for light visibility. */
	TArray<FVector> LightVisibilitySamplePoints;

	/**
	* Determines whether a light is visible, using cached results if available.
	* @param Light - The light to test visibility for.
	* @param OwnerPosition - The position of the owner to compute the light's effect for.
	* @param OutVisibilityFactor - Upon return, contains an appromiate percentage of light that reaches the owner's primitives.
	* @return TRUE if the light reaches the owner's primitives.
	*/
	UBOOL IsLightVisible(const ULightComponent* Light,const FVector& OwnerPosition,FLOAT& OutVisibilityFactor);

	/** Allocates a light, attempting to reuse a light with matching type from the free light pool. */
	template<typename LightType>
	LightType* AllocateLight() const;

	/**
	* Tests whether a light affects the owner.
	* @param Light - The light to test.
	* @param OwnerPosition - The position of the owner to compute the light's effect for.
	* @return TRUE if the light affects the owner.
	*/
	UBOOL DoesLightAffectOwner(const ULightComponent* Light,const FVector& OwnerPosition);

	/**
	* Adds the light's contribution to the light environment.
	* @param	Light				light to add
	* @param	LightEnvironment	light environment to add the light's contribution to
	* @param	ShadowEnvironment	The shadow environment to add the light's shadowing to.
	* @param	OwnerPosition		The position of the owner to compute the light's effect for.
	*/
	UBOOL AddLightToEnvironment( const ULightComponent* Light, FSHVectorRGB& LightEnvironment, FSHVectorRGB& ShadowEnvironment, const FVector& OwnerPosition, UBOOL& bSunVisible, UBOOL& bPointLightVisible );
};

/** Compute the direction which the spherical harmonic is highest at. */
static FVector SHGetMaximumDirection(const FSHVector& SH,UBOOL bLowerHemisphere,UBOOL bUpperHemisphere)
{
	// This is an approximation which only takes into account first and second order spherical harmonics.
	FLOAT Z = SH.V[2];
	if(!bLowerHemisphere)
	{
		Z = Max(Z,0.0f);
	}
	if(!bUpperHemisphere)
	{
		Z = Min(Z,0.0f);
	}
	return FVector(
		-SH.V[3],
		-SH.V[1],
		Z
		);
}

/** Compute the direction which the spherical harmonic is lowest at. */
static FVector SHGetMinimumDirection(const FSHVector& SH,UBOOL bLowerHemisphere,UBOOL bUpperHemisphere)
{
	// This is an approximation which only takes into account first and second order spherical harmonics.
	FLOAT Z = -SH.V[2];
	if(!bLowerHemisphere)
	{
		Z = Max(Z,0.0f);
	}
	if(!bUpperHemisphere)
	{
		Z = Min(Z,0.0f);
	}
	return FVector(
		+SH.V[3],
		+SH.V[1],
		Z
		);
}

/** Computes a brightness and a fixed point color from a floating point color. */
static void ComputeLightBrightnessAndColor(const FLinearColor& InLinearColor,FColor& OutColor,FLOAT& OutBrightness)
{
	FLOAT MaxComponent = Max(DELTA,Max(InLinearColor.R,Max(InLinearColor.G,InLinearColor.B)));
	OutColor = InLinearColor / MaxComponent;
	OutBrightness = MaxComponent;
}

/** Returns the SH coefficients for a point light from the given direction and brightness. */
FSHVector PointLightSH(const FVector& Direction)
{
	FSHVector Result = SHBasisFunction(Direction.SafeNormal());

	// Normalize the SH so its surface adds up to 1.
	static const FLOAT InvSum = appInvSqrt(Dot(Result,Result));
	Result *= InvSum;

	return Result;
}

// Sky light globals.
static UBOOL bComputedSkyFunctions = FALSE;
static FSHVector UpperSkyFunction;
static FSHVector LowerSkyFunction;
static FSHVector AmbientFunction;

/** Precomputes the SH coefficients for the upper and lower sky hemisphere functions. */
void InitSkyLightSH()
{
	if(!bComputedSkyFunctions)
	{
		bComputedSkyFunctions = TRUE;

		// Use numerical integration to project the sky visibility functions into the SH basis.
		static const INT NumIntegrationSamples = 1024;
		for(INT DirectionIndex = 0;DirectionIndex < NumIntegrationSamples;DirectionIndex++)
		{
			FVector Direction = VRand();
			FSHVector DirectionBasis = SHBasisFunction(Direction);
			if(Direction.Z > 0.0f)
			{
				UpperSkyFunction += DirectionBasis;
			}
			else
			{
				LowerSkyFunction += DirectionBasis;
			}
		}

		// Normalize the sky SH projections.
		UpperSkyFunction /= Dot(UpperSkyFunction,PointLightSH(FVector(0,0,+1)));
		LowerSkyFunction /= Dot(LowerSkyFunction,PointLightSH(FVector(0,0,-1)));

		// The ambient function is simply a constant 1 across its surface; the combination of the upper and lower sky functions.
		AmbientFunction = UpperSkyFunction + LowerSkyFunction;
	}
}

/** Clamps each color component above 0. */
static FLinearColor GetPositiveColor(const FLinearColor& Color)
{
	return FLinearColor(
		Max(Color.R,0.0f),
		Max(Color.G,0.0f),
		Max(Color.B,0.0f),
		Color.A
		);
}

/** Remove skylighting from a light environment. */
static void ExtractEnvironmentSkyLight(FSHVectorRGB& LightEnvironment,FLinearColor& OutSkyColor,UBOOL bLowerHemisphere,UBOOL bUpperHemisphere)
{
	FSHVector LightEnvironmentLuminance = LightEnvironment.GetLuminance();
	FVector MinDirection = SHGetMinimumDirection(LightEnvironmentLuminance,bLowerHemisphere,bUpperHemisphere);
	FSHVector UnitLightSH = PointLightSH(MinDirection);

	FLinearColor Intensity = GetPositiveColor(Dot(LightEnvironment,UnitLightSH));

	if(Intensity.R > 0.0f || Intensity.G > 0.0f || Intensity.B > 0.0f)
	{
		OutSkyColor += Intensity;

		if(bLowerHemisphere)
		{
			LightEnvironment -= LowerSkyFunction * Intensity;
		}
		if(bUpperHemisphere)
		{
			LightEnvironment -= UpperSkyFunction * Intensity;
		}
	}
}

/** Adds a skylight to a light environment. */
static void AddSkyLightEnvironment(const USkyLightComponent* SkyLight,FSHVectorRGB& OutLightEnvironment)
{
	OutLightEnvironment += UpperSkyFunction * FLinearColor(SkyLight->LightColor) * SkyLight->Brightness;
	OutLightEnvironment += LowerSkyFunction * FLinearColor(SkyLight->LowerColor) * SkyLight->LowerBrightness;
}

static FLOAT AmbientLight = 1.0f;
static FLOAT AmbientShadow = 1 / 8.0f;

FDynamicLightEnvironmentState::FDynamicLightEnvironmentState(UDynamicLightEnvironmentComponent* InComponent):
	Component(InComponent)
,	PredictedOwnerPosition(0,0,0)
,	LastUpdateTime(0)
,	InvisibleUpdateTime(InComponent->InvisibleUpdateTime)
,	MinTimeBetweenFullUpdates(InComponent->MinTimeBetweenFullUpdates)
,	bFirstFullUpdate(TRUE)
,	bSunVisible(FALSE)
,	bPointLightVisible(FALSE)
,	bCascadedShadowVisible(FALSE)
{
	InitSkyLightSH();

	// Initialize the random light visibility sample points.
	const INT NumLightVisibilitySamplePoints = Component->NumVolumeVisibilitySamples;
	LightVisibilitySamplePoints.Empty(NumLightVisibilitySamplePoints);

	// Always place one sample at the center of the owner's bounds.
	LightVisibilitySamplePoints.AddItem(FVector(0,0,0));

	for(INT PointIndex = 1;PointIndex < NumLightVisibilitySamplePoints;PointIndex++)
	{
		LightVisibilitySamplePoints.AddItem(
			FVector(
			-1.0f + 2.0f * appSRand(),
			-1.0f + 2.0f * appSRand(),
			-1.0f + 2.0f * appSRand()
			)
			);
	}	
}

/** Updates the light environment. */
void FDynamicLightEnvironmentState::UpdateEnvironment(FLOAT DeltaTime,UBOOL bPerformFullUpdate,UBOOL bForceStaticLightUpdate)
{
	// Update the owner's components.
	Component->GetOwner()->ConditionalUpdateComponents();				

	SCOPE_CYCLE_COUNTER(STAT_UpdateEnvironmentTime);
	INC_DWORD_STAT(STAT_EnvironmentUpdates);

	// Find the owner's bounds and lighting channels.
	const FBoxSphereBounds PreviousOwnerBounds = OwnerBounds;
	
	OwnerLightingChannels.Bitfield = 0;
	OwnerLightingChannels.bInitialized = TRUE;

	UBOOL bOwnerBoundsInitialized = FALSE;
	UBOOL bValidCollisionComponent = FALSE;

	if (Component->GetOwner()->CollisionComponent != NULL)
	{
		OwnerBounds = Component->GetOwner()->CollisionComponent->Bounds;

		bValidCollisionComponent = TRUE;
		bOwnerBoundsInitialized = TRUE;
	}
	
	for(INT ComponentIndex = 0;ComponentIndex < Component->GetOwner()->AllComponents.Num();ComponentIndex++)
	{
		UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component->GetOwner()->AllComponents(ComponentIndex));

		// Only look at primitives which use this light environment.
		if(Primitive && Primitive->LightEnvironment == Component)
		{
			// Add the primitive's bounds to the composite owner bounds.
			if (!bOwnerBoundsInitialized)
			{
				OwnerBounds = Primitive->Bounds;

				bOwnerBoundsInitialized = TRUE;
			}
			else if (!bValidCollisionComponent)
			{
				OwnerBounds = OwnerBounds + Primitive->Bounds;
			}				

			// Add the primitive's lighting channels to the composite owner lighting channels.
			OwnerLightingChannels.Bitfield |= Primitive->LightingChannels.Bitfield;
		}
	}

	if (!bOwnerBoundsInitialized)
	{
		OwnerBounds = FBoxSphereBounds(Component->GetOwner()->Location,FVector(0,0,0),50);
	}	
	

	// Attempt to predict the owner's position at the next update.
	const FVector PreviousPredictedOwnerPosition = PredictedOwnerPosition;

	// Disable the velocity offset for the moment, as it causes artifacts when the velocity offset ends in a wall.
#if 0
	if(!bFirstFullUpdate)
	{
		// Compute the owner's velocity.
		const FLOAT FramesPerUpdate = MinTimeBetweenFullUpdates / Max(DELTA,DeltaTime);
		const FVector Velocity = (OwnerBounds.Origin - PreviousOwnerBounds.Origin) * FramesPerUpdate;

		// Predict that the owner will proceed with the same velocity.
		PredictedOwnerPosition = OwnerBounds.Origin + Velocity * 0.5f;
	}
	else
#endif
	{
		PredictedOwnerPosition = OwnerBounds.Origin;
	}

	// Only iterate over all lights performing the slow fully update if wanted.
	if( bPerformFullUpdate )
	{		
		if( bFirstFullUpdate || bForceStaticLightUpdate
			||	(OwnerBounds.Origin != PreviousOwnerBounds.Origin) 
			||	(OwnerBounds.SphereRadius != PreviousOwnerBounds.SphereRadius) )
		{
			bSunVisible = bPointLightVisible = FALSE;

			//<@ ava specific ; 2008. 1. 7 changmin
			// add cascaded shadow
			bCascadedShadowVisible = FALSE;
			//>@ ava

			static FAmbientSH Zero = {0};
			// Reset as the below code is going to accumulate from scratch.
			
			extern void BeginFetchSH();
			BeginFetchSH();

			GWorld->GetWorldInfo()->InterpolateAmbientCube( OwnerBounds.Origin, AmbientCube );
			if (!appMemcmp(&AmbientCube,&Zero,sizeof(FAmbientSH)))
			{
				FVector Location = OwnerBounds.Origin;
				Location.Z = OwnerBounds.GetBox().Max.Z;
				GWorld->GetWorldInfo()->InterpolateAmbientCube( Location, AmbientCube );
			}

			// Reset as the below code is going to accumulate from scratch.
			NewStaticLightEnvironment = FSHVectorRGB();
			NewStaticShadowEnvironment	= FSHVectorRGB();			
			NewAmbientLightEnvironment  = (FSHVectorRGB)AmbientCube * AmbientLight;			

			// Iterate over static lights and update the static light environment.
			const INT MaxStaticLightIndex = GWorld->StaticLightList.GetMaxIndex();
			UBOOL bShadowCreated = FALSE;
			for(TSparseArray<ULightComponent*>::TConstIterator LightIt(GWorld->StaticLightList);LightIt;++LightIt)
			{
				const ULightComponent* Light = *LightIt;

				// Prefetch next index. This can potentially be empty but in practice isn't most of the time.
				const INT NextLightIndex = LightIt.GetIndex() + 1;
				if( NextLightIndex < MaxStaticLightIndex )
				{
					PREFETCH( &GWorld->StaticLightList(NextLightIndex) );
				}

				// Add static light to static light environment.
				if (AddLightToEnvironment( Light, NewStaticLightEnvironment, NewStaticShadowEnvironment, PredictedOwnerPosition, bSunVisible, bPointLightVisible ))
					bShadowCreated  = TRUE;
			}

			// Add the ambient shadow source.
			if (!bShadowCreated)
				NewStaticShadowEnvironment += PointLightSH(Component->AmbientShadowSourceDirection) * Component->AmbientShadowColor;

			INC_DWORD_STAT(STAT_StaticEnvironmentFullUpdates);
		}
		else
		{
			INC_DWORD_STAT(STAT_StaticEnvironmentFullUpdatesSkipped);
		}
	}

	DynamicLightEnvironment = FSHVectorRGB();
	DynamicShadowEnvironment = FSHVectorRGB();
	DynamicLights.Empty(DynamicLights.Num());

	UBOOL bDynamicPointLightVisible = FALSE;

	if(Component->bDynamic)
	{
		// Iterate over dynamic lights and update the dynamic light environment.
		const INT MaxDynamicLightIndex = GWorld->DynamicLightList.GetMaxIndex();
		for(TSparseArray<ULightComponent*>::TConstIterator LightIt(GWorld->DynamicLightList);LightIt;++LightIt)
		{
			ULightComponent* Light = *LightIt;

			// Prefetch next index. This can potentially be empty but in practice isn't most of the time.
			const INT NextLightIndex = LightIt.GetIndex() + 1;
			if( NextLightIndex < MaxDynamicLightIndex )
			{
				PREFETCH( &GWorld->DynamicLightList(NextLightIndex) );
			}

			if(GSystemSettings->bUseCompositeDynamicLights)
			{
				// Add the dynamic light to the light environment.
				AddLightToEnvironment(Light,DynamicLightEnvironment,DynamicShadowEnvironment,OwnerBounds.Origin,bSunVisible,bPointLightVisible);
			}
			else
			{
				// Add the dynamic light to the dynamic light list if it affects the owner.
				if(DoesLightAffectOwner(Light,OwnerBounds.Origin))
				{
					DynamicLights.AddItem(Light);
				}
			}
		}
	}	

	bDirectionalShadow = !(bPointLightVisible || bDynamicPointLightVisible) || bSunVisible;		

	// Smoothly interpolate the static light set.
	const FLOAT RemainingTransitionTime = Max(DELTA,LastUpdateTime + MinTimeBetweenFullUpdates - GWorld->GetTimeSeconds());
	const FLOAT TransitionAlpha =
		bFirstFullUpdate ?
		1.0f :
	Clamp(DeltaTime / RemainingTransitionTime,0.0f,1.0f);
	StaticLightEnvironment = StaticLightEnvironment * (1.0f - TransitionAlpha) + NewStaticLightEnvironment * TransitionAlpha;
	StaticShadowEnvironment = StaticShadowEnvironment * (1.0f - TransitionAlpha) + NewStaticShadowEnvironment * TransitionAlpha;
	AmbientLightEnvironment = AmbientLightEnvironment * (1.0f - TransitionAlpha) + NewAmbientLightEnvironment * TransitionAlpha;
}

/** Updates the light environment's state. */
void FDynamicLightEnvironmentState::Tick(FLOAT DeltaTime)
{
	INC_DWORD_STAT(STAT_NumEnvironments);

	// Determine if the light environment's primitives have been rendered in the last second.
	const FLOAT CurrentTime			= GWorld->GetTimeSeconds();
	const FLOAT TimeSinceLastUpdate	= CurrentTime - LastUpdateTime;
	UBOOL bVisible = (CurrentTime - Component->LastRenderTime) < 1.0f;

	// Only perform full updates every so often.
	UBOOL bPerformFullUpdate = FALSE;

	/// 1인칭 무기에 대한 처리를 해야함 ;;
	if (!bVisible)
	{
		APawn* Pawn = Cast<APawn>( Component->GetOwner() );

		if (Pawn && Pawn->Weapon)
		{
			bVisible = (CurrentTime - Pawn->Weapon->LastRenderTime) < 1.0f;

			// 1인칭인 경우 매번 해줍시다. :)
			if (bVisible)
			{
				bPerformFullUpdate = TRUE;
			}
		}
	}

	// Only update the light environment if it's visible, or it hasn't been updated for the last InvisibleUpdateTime seconds.
	if(bFirstFullUpdate || bVisible || TimeSinceLastUpdate > InvisibleUpdateTime)
	{
		// Spread out updating invisible components over several frames to avoid spikes by varying invisible update time by +/- 20%.
		InvisibleUpdateTime = Component->InvisibleUpdateTime * (0.8 + 0.4 * appSRand());			

		if(bFirstFullUpdate || TimeSinceLastUpdate > MinTimeBetweenFullUpdates )
		{
			LastUpdateTime				= CurrentTime;
			bPerformFullUpdate			= TRUE;
			// Create variance to avoid spikes caused by multiple components being updated the same frame.
			MinTimeBetweenFullUpdates	= Component->MinTimeBetweenFullUpdates * (0.8 + 0.4 * appSRand());
		}

		// Update the light environment.
		UpdateEnvironment(DeltaTime,bPerformFullUpdate,FALSE);

		// Update the lights from the environment.
		CreateEnvironmentLightList();

		bFirstFullUpdate = FALSE;
	}
}

/** Creates a light to represent a light environment. */
UBOOL FDynamicLightEnvironmentState::CreateRepresentativeLight(FSHVectorRGB& RemainingLightEnvironment,UBOOL bCastLight,UBOOL bCastShadows, FLOAT MinIntensity ) const
{
	// Find the direction in the light environment with the highest luminance.
	FSHVector RemainingLuminance = RemainingLightEnvironment.GetLuminance();
	FVector MaxDirection = SHGetMaximumDirection(RemainingLuminance,TRUE,TRUE);
	if(MaxDirection.SizeSquared() >= DELTA)
	{
		// Calculate the light intensity for this direction.
		FSHVector UnitLightSH = PointLightSH(MaxDirection);
		FLinearColor Intensity = GetPositiveColor(Dot(RemainingLightEnvironment,UnitLightSH));

		if(Intensity.R > MinIntensity || Intensity.G > MinIntensity || Intensity.B > MinIntensity)
		{
			// Remove this light from the environment.
			RemainingLightEnvironment -= UnitLightSH * Intensity;			

			//<@ ava specific ; changmin 2006. 9. 19
			ULightComponent* Light = NULL;
			UPointLightComponent* PointLight = NULL;
			UDirectionalLightComponent* DirectionalLight = NULL;
			if( /*GWorld && GWorld->bDrawEmissiveDynamicPrimitives == FALSE && */bCastLight )
			{
				Light = PointLight = AllocateLight<USHPointLightComponent>();
			}
			else
			{
				if (bDirectionalShadow)
				{
					Light = DirectionalLight = AllocateLight<UDirectionalLightComponent>();
				}					
				else
				{
					Light = PointLight = AllocateLight<UPointLightComponent>();
				}
			}
			const FVector LightDirection = MaxDirection.SafeNormal();
			const FVector LightPosition = OwnerBounds.Origin + LightDirection * OwnerBounds.SphereRadius * (Component->LightDistance + 1);
			Light->LightingChannels = OwnerLightingChannels;			
			Light->bAffectsDefaultLightEnvironment = FALSE;

			if(bCastLight)
			{
				PointLight->Radius = OwnerBounds.SphereRadius * (Component->LightDistance + Component->ShadowDistance + 2);				
				const FLOAT RadialAttenuation = appPow(Max(1.0f - ((LightPosition - OwnerBounds.Origin) / PointLight->Radius).SizeSquared(),0.0f),PointLight->FalloffExponent);
				ComputeLightBrightnessAndColor(Intensity / RadialAttenuation,Light->LightColor,Light->Brightness);
			}
			else
			{
				if (!bDirectionalShadow)
				{
					PointLight->Radius = OwnerBounds.SphereRadius * (Component->LightDistance + Component->ShadowDistance + 2);
				}

				Light->Brightness = 0.0f;										
			}

			Light->CastShadows = bCastShadows;
			if(bCastShadows)
			{
				Light->LightShadowMode = LightShadow_Modulate;

				if (bDirectionalShadow)
				{
					DirectionalLight->ShadowFalloffExponent = 1.0f / 3.0f;
					DirectionalLight->ShadowFalloffDistance = OwnerBounds.SphereRadius * (Component->LightDistance + Component->ShadowDistance + 2);
					DirectionalLight->ShadowOffset = (LightDirection | LightPosition);
				}
				else
				{
					PointLight->ShadowFalloffExponent = 1.0f / 3.0f;
				}					

				// Choose a ModShadowColor based on the percent of the light environment that this light represents.
				FLinearColor RemainingIntensity = GetPositiveColor(Dot(RemainingLightEnvironment + (AmbientLightEnvironment * AmbientShadow),AmbientFunction));
				Light->ModShadowColor.R = Min(1.0f,RemainingIntensity.R / Max(RemainingIntensity.R + Intensity.R,DELTA));
				Light->ModShadowColor.G = Min(1.0f,RemainingIntensity.G / Max(RemainingIntensity.G + Intensity.G,DELTA));
				Light->ModShadowColor.B = Min(1.0f,RemainingIntensity.B / Max(RemainingIntensity.B + Intensity.B,DELTA));
			}

			if( bCastLight )
			{
				Component->LastDirection	= LightDirection;
				Component->LastColor		= Light->LightColor;
				Component->LastBrightness	= Light->Brightness;
			}

			Component->SetLightInteraction(Light,TRUE);			

			if( bCastLight || !bDirectionalShadow )
			{
				Light->ConditionalAttach(Component->GetScene(),NULL,FTranslationMatrix(LightPosition));
			}
			else
			{
				Light->ConditionalAttach(Component->GetScene(),NULL,FRotationMatrix((-LightDirection).Rotation()));
			}	

			return TRUE;
		}
	}

	if( bCastLight && !bCastShadows )
	{
		MaxDirection = FVector( 0, 0, 1 );
		FLinearColor Intensity( 0, 0, 0, 0 );

		// Calculate the light intensity for this direction.
		FSHVector UnitLightSH = PointLightSH(MaxDirection);									
		

		//<@ ava specific ; changmin 2006. 9. 19
		UPointLightComponent* Light = NULL;			

		Light = AllocateLight<USHPointLightComponent>();

		const FVector LightDirection = MaxDirection.SafeNormal();
		const FVector LightPosition = OwnerBounds.Origin + LightDirection * OwnerBounds.SphereRadius * (Component->LightDistance + 1);
		Light->LightingChannels = OwnerLightingChannels;
		Light->Radius = OwnerBounds.SphereRadius * (Component->LightDistance + Component->ShadowDistance + 2);
		Light->bAffectsDefaultLightEnvironment = FALSE;

		Light->Radius = OwnerBounds.SphereRadius * (Component->LightDistance + Component->ShadowDistance + 2);
		const FLOAT RadialAttenuation = appPow(Max(1.0f - ((LightPosition - OwnerBounds.Origin) / Light->Radius).SizeSquared(),0.0f),Light->FalloffExponent);
		ComputeLightBrightnessAndColor(Intensity / RadialAttenuation,Light->LightColor,Light->Brightness);			

		Light->CastShadows = FALSE;

		Component->LastDirection	= LightDirection;
		Component->LastColor		= Light->LightColor;
		Component->LastBrightness	= Light->Brightness;			

		Light->Brightness = 1.0f;
		Light->LightColor = Intensity;

		Component->SetLightInteraction(Light,TRUE);			

		Light->ConditionalAttach(Component->GetScene(),NULL,FTranslationMatrix(LightPosition));			

		Component->Lights.AddItem(Light);

		return TRUE;			
	}

	return FALSE;
}

void FDynamicLightEnvironmentState::DetachRepresentativeLights() const
{
	// Detach the environment's representative lights.
	for(INT LightIndex = 0;LightIndex < RepresentativeLightPool.Num();LightIndex++)
	{
		RepresentativeLightPool(LightIndex)->ConditionalDetach();
	}
}
	
/** Creates the lights to represent the character's light environment. */
void FDynamicLightEnvironmentState::CreateEnvironmentLightList() const
{
	SCOPE_CYCLE_COUNTER(STAT_CreateLightsTime);

	FSHVectorRGB RemainingLightEnvironment = StaticLightEnvironment + DynamicLightEnvironment;

	// Detach the old representative lights.
	DetachRepresentativeLights();

	// Reset the light environment's light list.
	Component->ResetLightInteractions();

	// Add the dynamic lights to the light environment's light list.
	for(INT LightIndex = 0;LightIndex < DynamicLights.Num();LightIndex++)
	{
		if(DynamicLights(LightIndex))
		{
			Component->SetLightInteraction(DynamicLights(LightIndex),TRUE);
		}
	}
	
	// Move as much light as possible into the sky light.
	FLinearColor LowerSkyLightColor(FLinearColor::Black);
	FLinearColor UpperSkyLightColor(FLinearColor::Black);
	
	if (GSystemSettings->DiffuseCubeResolution == 0)
	{
		RemainingLightEnvironment += AmbientLightEnvironment;

		ExtractEnvironmentSkyLight(RemainingLightEnvironment,UpperSkyLightColor,FALSE,TRUE);
		ExtractEnvironmentSkyLight(RemainingLightEnvironment,LowerSkyLightColor,TRUE,FALSE);
	}	

	// Create a point light that is representative of the light environment.
	UBOOL bLightsCreated = CreateRepresentativeLight(RemainingLightEnvironment,TRUE,FALSE,-1.0f/*Always create SHPoint*/);
	if (bLightsCreated)
		Component->bHasLighting = TRUE;
	else
		Component->bHasLighting = FALSE;

	// Create a shadow-only point light that is representative of the shadow-casting light environment.
	if( Component->bCastShadows && GSystemSettings->bAllowLightEnvironmentShadows )
	{
		FSHVectorRGB RemainingShadowLightEnvironment = StaticShadowEnvironment + DynamicShadowEnvironment;

		//<@ ava specific ; 2008. 1. 7 changmin
		// add cascaded shadow
		extern UBOOL GUseCascadedShadow;
		if( !(GUseCascadedShadow && bCascadedShadowVisible) )
			CreateRepresentativeLight(RemainingShadowLightEnvironment,FALSE,TRUE);
		//>@ ava
	}

	// Add the remaining light to the sky light as ambient light.
	if (GSystemSettings->DiffuseCubeResolution == 0)
	{
		FLinearColor RemainingIntensity = Dot(RemainingLightEnvironment,AmbientFunction);
		UpperSkyLightColor += RemainingIntensity;
		LowerSkyLightColor += RemainingIntensity;
		RemainingLightEnvironment -= AmbientFunction * RemainingIntensity;
	}	

	// Create a sky light for the lights not represented by the directional lights.
	//USkyLightComponent* SkyLight = AllocateLight<USkyLightComponent>();
	//<@ ava specific ; 2006. 9 20 changmin
	USkyLightComponent* SkyLight = AllocateLight<USkyLightComponent>();
	//>@ ava

	SkyLight->LightingChannels = OwnerLightingChannels;
	SkyLight->bAffectsDefaultLightEnvironment = FALSE;

	RemainingLightEnvironment += AmbientLightEnvironment;
	ComputeLightBrightnessAndColor(UpperSkyLightColor,SkyLight->LightColor,SkyLight->Brightness);
	ComputeLightBrightnessAndColor(LowerSkyLightColor,SkyLight->LowerColor,SkyLight->LowerBrightness);	
	SkyLight->IrradianceSH = *(FAmbientSH*)&RemainingLightEnvironment;

	Component->LastUpperSkyBrightness	=	SkyLight->Brightness;
	Component->LastUpperSkyColor		=	SkyLight->LightColor;
	Component->LastLowerSkyBrightness	=	SkyLight->LowerBrightness;
	Component->LastLowerSkyColor		=	SkyLight->LowerColor;

	Component->SetLightInteraction(SkyLight,TRUE);

	// Attach the skylight after it is associated with the light environment to ensure it is only attached once.
	SkyLight->ConditionalAttach(Component->GetScene(),NULL,FMatrix::Identity);	
}

void FDynamicLightEnvironmentState::AddReferencedObjects(TArray<UObject*>& ObjectArray) 
{
	// Add the light environment's dynamic lights.
	for(INT LightIndex = 0;LightIndex < DynamicLights.Num();LightIndex++)
	{
		if (DynamicLights(LightIndex) != NULL && DynamicLights(LightIndex)->IsPendingKill())
		{
			DynamicLights(LightIndex) = NULL;
		}
		else
		{
			UObject::AddReferencedObject(ObjectArray,DynamicLights(LightIndex));
		}
	}

	// Add the light environment's representative lights.
	for(INT LightIndex = 0;LightIndex < RepresentativeLightPool.Num();LightIndex++)
	{
		UObject::AddReferencedObject(ObjectArray,RepresentativeLightPool(LightIndex));
	}
}


void FDynamicLightEnvironmentState::BeginDeferredStaticLightingUpdate()
{
	bFirstFullUpdate = TRUE;
}

/** Determines whether a light is visible, using cached results if available. */
UBOOL FDynamicLightEnvironmentState::IsLightVisible(const ULightComponent* Light,const FVector& OwnerPosition,FLOAT& OutVisibilityFactor)
{
	SCOPE_CYCLE_COUNTER(STAT_LightVisibilityTime);

	// Sky lights are always invisible for AVA.
	if(Light->IsA(USkyLightComponent::StaticClass()))
	{
		OutVisibilityFactor = 0.0f;
		return TRUE;
	}

	// Lights which don't cast static shadows are always visible.
	if(!Light->CastShadows || !Light->CastStaticShadows)
	{
		OutVisibilityFactor = 1.0f;
		return TRUE;
	}

	// Compute light visibility for one or more points within the owner's bounds.
	INT NumVisibleSamples = 0;
	INT NumVisibilitySamplesToUse = Light->HasStaticLighting() ? LightVisibilitySamplePoints.Num() : 1;
	for(INT SampleIndex = 0;SampleIndex < NumVisibilitySamplesToUse; SampleIndex++)
	{
		// Determine a random point to test visibility for in the owner's bounds.
		const FVector VisibilityTestPoint = PredictedOwnerPosition + LightVisibilitySamplePoints(SampleIndex) * OwnerBounds.BoxExtent;

		// Determine the direction from the primitive to the light.
		FVector4 LightPosition = Light->GetPosition();
		FVector LightVector = (FVector)LightPosition - VisibilityTestPoint * LightPosition.W;

		// Check the line between the light and the primitive's origin for occlusion.
		FCheckResult Hit(1.0f);
		const UBOOL bPointIsLit = GWorld->SingleLineCheck(
			Hit,
			NULL,
			VisibilityTestPoint,
			VisibilityTestPoint + LightVector,
			TRACE_Level|TRACE_Actors|TRACE_ShadowCast|TRACE_StopAtAnyHit,
			FVector(0,0,0),
			const_cast<ULightComponent*>(Light)
			);
		if(bPointIsLit)
		{
			NumVisibleSamples++;
		}
	}

	OutVisibilityFactor = (FLOAT)NumVisibleSamples / (FLOAT)NumVisibilitySamplesToUse;

	return OutVisibilityFactor > 0.0f;
}

template<typename LightType>
LightType* FDynamicLightEnvironmentState::AllocateLight() const
{
	// Try to find an unattached light of matching type in the representative light pool.
	for(INT LightIndex = 0;LightIndex < RepresentativeLightPool.Num();LightIndex++)
	{
		ULightComponent* Light = RepresentativeLightPool(LightIndex);
		if(Light && !Light->IsAttached() && Light->GetClass() == LightType::StaticClass())
		{
			return CastChecked<LightType>(Light);
		}
	}

	// Create a new light.
	LightType* NewLight = ConstructObject<LightType>(LightType::StaticClass(),Component);
	RepresentativeLightPool.AddItem(NewLight);
	return NewLight;
}

UBOOL FDynamicLightEnvironmentState::DoesLightAffectOwner(const ULightComponent* Light,const FVector& OwnerPosition)
{
	// Use the CompositeDynamic lighting channel as the Dynamic lighting channel. 
	FLightingChannelContainer ConvertedLightingChannels = Light->LightingChannels;
	ConvertedLightingChannels.Dynamic = FALSE;
	if(ConvertedLightingChannels.CompositeDynamic)
	{
		ConvertedLightingChannels.CompositeDynamic = FALSE;
		ConvertedLightingChannels.Dynamic = TRUE;
	}

	// Skip lights which don't affect the owner's lighting channels.
	if(!ConvertedLightingChannels.OverlapsWith(OwnerLightingChannels))
	{
		return FALSE;
	}

	// Skip lights which don't affect the owner's predicted bounds.
	if(!Light->AffectsBounds(FBoxSphereBounds(OwnerPosition,OwnerBounds.BoxExtent,OwnerBounds.SphereRadius)))
	{
		return FALSE;
	}

	// Skip disabled lights.
	if(!Light->bEnabled)
	{
		return FALSE;
	}

	return TRUE;
}

UBOOL FDynamicLightEnvironmentState::AddLightToEnvironment(const ULightComponent* Light, FSHVectorRGB& LightEnvironment, FSHVectorRGB& ShadowEnvironment,const FVector& OwnerPosition, UBOOL& bSunVisible, UBOOL& bPointLightVisible )
{
	UBOOL bRetValue = FALSE;
	// Determine whether the light affects the owner, and its visibility factor.
	FLOAT VisibilityFactor;
	if(DoesLightAffectOwner(Light,OwnerPosition) && IsLightVisible(Light,OwnerPosition,VisibilityFactor))
	{
		if(Light->IsA(USkyLightComponent::StaticClass()))
		{				
			const USkyLightComponent* SkyLight = ConstCast<USkyLightComponent>(Light);
			
			// Add the sky light SH to the light environment SH.
			AddSkyLightEnvironment(SkyLight,LightEnvironment);

			if(Light->bCastCompositeShadow)
			{
				// Add the sky light SH to the shadow casting environment SH.
				AddSkyLightEnvironment(SkyLight,ShadowEnvironment);
			}
		}
		else
		{
			// Compute the light's intensity at the actor's origin.
			const FLinearColor Intensity = Light->GetDirectIntensity(OwnerBounds.Origin);
			FSHVectorRGB IndividualLightEnvironment;

			if(!Light->IsA(USunLightComponent::StaticClass()))
			{						
				// Determine the direction from the primitive to the light.
				FVector4 LightPosition = Light->GetPosition();
				FVector LightVector = (FVector)LightPosition - OwnerBounds.Origin * LightPosition.W;
				const FSHVectorRGB IndividualLightEnvironment = PointLightSH(LightVector) * Intensity;

				// Add the light to the static light environment SH.
				LightEnvironment += IndividualLightEnvironment;

				if(Light->bCastCompositeShadow)
				{
					bPointLightVisible = TRUE;
				}
			}
			else
			{
				//<@ ava specific ; 2007. 10. 29 changmin
				// add cascaded shadow
				if(Light->bCastCompositeShadow)
				{
					bSunVisible = TRUE;
				}

				if( Light->bUseCascadedShadowmap )
				{
					bCascadedShadowVisible = TRUE;
				}

				extern UBOOL GUseCascadedShadow;
				if( !Light->bUseCascadedShadowmap || !GUseCascadedShadow )
				//>@ ava
				{
					FVector LightVector = -Cast<USunLightComponent>(const_cast<ULightComponent*>(Light))->GetDirection();
					IndividualLightEnvironment = PointLightSH(LightVector) * Intensity;

					// Add the light to the static light environment SH.
					LightEnvironment += IndividualLightEnvironment;		
				}			
			}

			extern UBOOL GUseCascadedShadow;
			if(Light->bCastCompositeShadow
			&& !(Light->bUseCascadedShadowmap && GUseCascadedShadow) )	//<@ 2007. 10. 28 changmin ; add cascaded shadow
			{
				// Add the light to the shadow casting environment SH.
				ShadowEnvironment += IndividualLightEnvironment;

				bRetValue = TRUE;
			}			
		}
	}

	return bRetValue;
}

void UDynamicLightEnvironmentComponent::FinishDestroy()
{
	Super::FinishDestroy();

	// Clean up the light environment's state.
	delete State;
	State = NULL;
}

void UDynamicLightEnvironmentComponent::AddReferencedObjects(TArray<UObject*>& ObjectArray)
{
	Super::AddReferencedObjects(ObjectArray);

	if(State)
	{
		State->AddReferencedObjects(ObjectArray);
	}
}

void UDynamicLightEnvironmentComponent::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if(!Ar.IsSaving() && !Ar.IsLoading())
	{
		// If serialization is being used to find references for garbage collection, use AddReferencedObjects to gather a list to serialize.
		TArray<UObject*> ReferencedObjects;
		AddReferencedObjects(ReferencedObjects);
		Ar << ReferencedObjects;
	}
}

void UDynamicLightEnvironmentComponent::Tick(FLOAT DeltaTime)
{
	Super::Tick(DeltaTime);

	if(bEnabled)
	{
		// Update the light environment's state.
		check(State);
		State->Tick(DeltaTime);
	}
}

void UDynamicLightEnvironmentComponent::Attach()
{
	Super::Attach();

	if(bEnabled)
	{
		// Initialize the light environment's state the first time it's attached.
		if(!State)
		{
			State = new FDynamicLightEnvironmentState(this);
		}

		// Outside the game we're not ticked, so update the light environment on attach.
		if(!GIsGame)
		{
			State->UpdateEnvironment(MAX_FLT,TRUE,TRUE);
		}

		// Add the light environment to the world's list, so it can be updated when static lights change.
		if(!GIsGame && Scene->GetWorld())
		{
			Scene->GetWorld()->LightEnvironmentList.AddItem(this);
		}		

		// Recreate the lights.
		State->CreateEnvironmentLightList();
	}
}

void UDynamicLightEnvironmentComponent::UpdateTransform()
{
	Super::UpdateTransform();

	if(bEnabled)
	{
		// Outside the game we're not ticked, so update the light environment on attach.
		if(!GIsGame)
		{
			State->UpdateEnvironment(MAX_FLT,TRUE,TRUE);
			State->CreateEnvironmentLightList();
		}
	}
}

void UDynamicLightEnvironmentComponent::Detach()
{
	Super::Detach();

	// Remove the light environment from the world's list.
	if(!GIsGame && Scene->GetWorld())
	{
		for(TSparseArray<ULightEnvironmentComponent*>::TIterator It(Scene->GetWorld()->LightEnvironmentList);It;++It)
		{
			if(*It == this)
			{
				Scene->GetWorld()->LightEnvironmentList.Remove(It.GetIndex());
				break;
			}
		}
	}

	// Reset the light environment's light list.
	Lights.Empty();

	if(State)
	{
		// Detach the light environment's representative lights.
		State->DetachRepresentativeLights();
	}
}

void UDynamicLightEnvironmentComponent::UpdateLight(const ULightComponent* Light)
{
	if(bEnabled && IsAttached())
	{
		if(GIsGame && Light->HasStaticShadowing())
		{
			// In the game, update light environments when a static light changes.
			// The update is deferred until the next tick.
			State->BeginDeferredStaticLightingUpdate();
		}
		else if(!GIsGame)
		{
			// Outside the game, update the environment if any light changes.
			BeginDeferredUpdateTransform();
		}
	}
}
