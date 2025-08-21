#include "PrecompiledHeaders.h"
#include "avaGame.h"

IMPLEMENT_CLASS( UavaSoundNodeAttenuation );
IMPLEMENT_CLASS( UavaSoundNodeDistMix );
IMPLEMENT_CLASS( UavaSoundNodePlaytime );
IMPLEMENT_CLASS( AavaSoundScape );
IMPLEMENT_CLASS( UavaSoundScapeProperty );

#define DVAR_MIX_MIN	0.0f
#define DVAR_MIX_MAX	1.0f

static struct SoundScapeRepository
{
	FCriticalSection* CriticalSection;

	SoundScapeRepository()
		: CriticalSection(NULL)
	{		
	}

	~SoundScapeRepository()
	{
		if ( GSynchronizeFactory != NULL )
			GSynchronizeFactory->Destroy( CriticalSection );
	}	

	TArray<AavaSoundScape*>	Array;
} GSoundscapeRepository;

void InitSoundScapeRepository()
{
	GSoundscapeRepository.CriticalSection = GSynchronizeFactory->CreateCriticalSection();
}

AavaSoundScape::AavaSoundScape()
{
	FScopeLock ScopeLock(GSoundscapeRepository.CriticalSection);

	GSoundscapeRepository.Array.AddItem( this );
}

void AavaSoundScape::FinishDestroy()
{
	{
		FScopeLock ScopeLock(GSoundscapeRepository.CriticalSection);
		GSoundscapeRepository.Array.RemoveItem( this );
	}	
	
	Super::FinishDestroy();
}

static FVector CamLoc;

class SoundscapeCompare 
{
public :
	static inline INT Compare( const AavaSoundScape* A, const AavaSoundScape* B	)
	{
		FLOAT dA = (A->Location - CamLoc).SizeSquared();
		FLOAT dB = (B->Location - CamLoc).SizeSquared();

		if (dA > dB) return 1;
		if (dA < dB) return -1;
		return 0;
	}
};

class AavaSoundScape* AavaSoundScape::FindSoundscape(FVector pos)
{	
	if (!GWorld)
		return NULL;

	CamLoc = pos;

	FScopeLock ScopeLock(GSoundscapeRepository.CriticalSection);

	Sort<AavaSoundScape*,SoundscapeCompare>(&GSoundscapeRepository.Array(0),GSoundscapeRepository.Array.Num());
	
	for (INT i=0; i<GSoundscapeRepository.Array.Num(); ++i)
	{
		FCheckResult Hit(1.f);
		GWorld->SingleLineCheck(Hit, this, GSoundscapeRepository.Array(i)->Location, CamLoc, TRACE_Level);
		if ( Hit.Time == 1.f )
			return GSoundscapeRepository.Array(i);
	}

	return NULL;
}

void UavaSoundNodeDistMix::ParseNodes( USoundNode* Parent, INT ChildIndex, UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances )
{
	// A "mixer type" cannot use seamless looping as it waits for the longest path to finish before it "finishes".
	AudioComponent->CurrentUseSeamlessLooping = FALSE;

	FLOAT Distance = FDist( AudioComponent->CurrentLocation, AudioComponent->Listener->Location );

	UBOOL bStereo = FALSE;
	
	if (!AudioComponent->bAllowSpatialization) 
	{
		bStereo = ChildNodes.Num() >= 2 && ChildNodes(2) != NULL;

		if (!bStereo)
		{
			Distance = 0.0f;
		}
	}

	Distance = Clamp( Distance, NearDistance, FarDistance ) - NearDistance;

	// dist 0->1.0

	Distance = Distance / (FarDistance - NearDistance);

	// mix min->max
	FLOAT mixFactor = ((DVAR_MIX_MAX - DVAR_MIX_MIN) * Distance) + DVAR_MIX_MIN;	

	if (!bStereo)
	{
		// 처음!!!
		if (AudioComponent->ForceDistMixSolo == -1)
		{
			if (mixFactor < OneShot_Solo_Threshold)
			{
				AudioComponent->ForceDistMixSolo = 0; // Near				
			}
			else if (mixFactor > 1 - OneShot_Solo_Threshold)
			{
				AudioComponent->ForceDistMixSolo = 1; // Far				
			}
			else
			{
				AudioComponent->ForceDistMixSolo = -2; // Do not re-check solo
			}
		}		

#define SOLO_THRESHOLD_VOLUME 0.1f

		if (AudioComponent->CurrentVolume < SOLO_THRESHOLD_VOLUME)
		{
			mixFactor = (mixFactor > 0.5f) ? 1 : 0;
		}
	}	

	for( INT ChildNodeIndex=0; ChildNodeIndex<ChildNodes.Num(); ChildNodeIndex++ )
	{
		// Solo mode가 작동중이지 않거나, 같은 채널이어야 함
		if( AudioComponent->ForceDistMixSolo < 0 || AudioComponent->ForceDistMixSolo == ChildNodeIndex )
		{
			if( ChildNodes(ChildNodeIndex) != NULL )
			{
				if (bStereo)
				{
					if (ChildNodeIndex == 2)
					{
						FAudioComponentSavedState SavedState;
						// save our state for this path through the SoundCue (i.e. at the end we need to remove any modifications parsing the path did so we can parse the next input with a clean slate)
						SavedState.Set( AudioComponent );									

						AudioComponent->CurrentVolume = 1;

						AudioComponent->CurrentHeadRelative = TRUE;

						// "play" the rest of the tree
						ChildNodes(ChildNodeIndex)->ParseNodes( this, ChildNodeIndex, AudioComponent, WaveInstances );

						SavedState.Restore( AudioComponent );
					}
				}
				else
				{
					if (ChildNodeIndex != 2)
					{				
						FAudioComponentSavedState SavedState;
						// save our state for this path through the SoundCue (i.e. at the end we need to remove any modifications parsing the path did so we can parse the next input with a clean slate)
						SavedState.Set( AudioComponent );									

						// Solo mode가 작동 중이지 않아야 fade.
						if (AudioComponent->ForceDistMixSolo < 0)
						{
							AudioComponent->CurrentVolume *= ChildNodeIndex == 0 ? (1-mixFactor) : mixFactor;
						}						

						// "play" the rest of the tree
						ChildNodes(ChildNodeIndex)->ParseNodes( this, ChildNodeIndex, AudioComponent, WaveInstances );

						SavedState.Restore( AudioComponent );
					}			
				}			
			}
		}
	}
}

// @param1: 현재 존재하는 공간의 기압. PA
// @param2: 현재 공간의 기온. 섭씨로 표시.
// @param3: 현재 공간의 습도
// @param4: 계산하고자 하는 사운드의 frequency
// @return: dB/Km. 1Km마다 감쇠하는 소리의 크기.
// http://www.csgnetwork.com/atmossndabsorbcalc.html 여기에서 데이터 빼왔음
// 1기압(101325PA), 섭씨 30도, 습도 60%, 1000Hz의 사운드 에서는 1Km마다 7.3dB의 소리가 감쇠한다
float CalculateDecibelAttenuate(FLOAT pressure, FLOAT temperature, FLOAT humidity, FLOAT frequency )
{
	temperature+=273.15f; // Kelvin온도로 바꿔줌
	pressure/=101325;
	FLOAT C, h, tr, fr0, frN, alpha;
	C = 4.6151-6.8346*pow((273.16f/temperature),1.261f);
	h=humidity*pow(10.f,C)*pressure;
	tr=temperature/293.15; // 상대적인 공기 온도로 바꿔줌( 섭씨 20도 )
	fr0=pressure*(24+4.04e4*h*(0.02+h)/(0.391+h));
	frN=pressure*pow(tr,-0.5f)*(9+280*h*exp(-4.17*(pow(tr,-1/3.f)-1)));
	alpha=8.686*frequency*frequency*(1.84e-11*(1/pressure)*sqrt(tr)+pow(tr,-2.5f)*(0.01275*(exp(-2239.1/temperature)*1/(fr0+frequency*frequency/fr0))+0.1068*(exp(-3352/temperature)*1/(frN+frequency*frequency/frN))));
	FLOAT dB=1000*alpha;

	if( dB < 0.0001 )
		dB = 0.f;

	return dB;
}

static FLOAT GetReference_dB()
{
	return GWorld && GWorld->GetWorldInfo() ? GWorld->GetWorldInfo()->Reference_dB : 60.0f;
}

#define snd_refdist		1.f	//	걔네가 36=3feet = 우린 대략 1m
#define snd_gain_init	1
#define snd_gain_max	1
#define snd_gain_min	0.01

//#define SNDLVL_TO_DIST_MULT( sndlvl ) ( sndlvl ? ((pow( 10.f, GetReference_dB() / 20 ) / pow( 10.f, (float)sndlvl / 20 )) / snd_refdist) : 0 )
// 원래의 식은 위와 같지만, decibel의 격차에 따른 크기 확장만 해주면 된다
#define SNDLVL_TO_DIST_MULT( sndlvl ) ( sndlvl ? ((pow( 10.f, GetReference_dB() / 20 ) / pow( 10.f, (float)sndlvl / 20 )) ) : 0 )
#define DIST_MULT_TO_SNDLVL( dist_mult ) (int)( dist_mult ? ( 20 * log10( pow( 10.f, GetReference_dB() / 20.f ) / (dist_mult * snd_refdist) ) ) : 0 )

#define SND_GAIN_COMP_EXP_MAX	2.5f		// Increasing SND_GAIN_COMP_EXP_MAX fits compression curve more closely
// to original gain curve as it approaches 1.0.  
#define SND_GAIN_COMP_EXP_MIN	0.8f	
#define SND_GAIN_COMP_THRESH	0.5f		// gain value above which gain curve is rounded to approach 1.0

#define SND_GAIN_THRESHOLD		0.025f

#define SND_DB_MAX				140.0f	// max db of any sound source
#define SND_DB_MED				90.0f	// db at which compression curve changes
#define SND_DB_MIN				60.0f	// min db of any sound source

// Remap a value in the range [A,B] to [C,D].
inline float RemapVal( float val, float A, float B, float C, float D)
{
	return C + (D - C) * (val - A) / (B - A);
}

#define SND_RADIUS_MAX		(10.0f)	// max sound source radius
#define SND_RADIUS_MIN		(1.0f)	// min sound source radius

inline float dB_To_Meter ( float db )
{
	FLOAT radius = SND_RADIUS_MIN + (SND_RADIUS_MAX - SND_RADIUS_MIN) * (db - SND_DB_MIN) / (SND_DB_MAX - SND_DB_MIN);

	return radius;
}

inline float dB_To_Gain ( float dB )
{
	float gain = powf (10.0f, dB / 20.0f);
	return gain;
}

void VectorVectors( const FVector &forward, FVector &right, FVector &up )
{
	FVector tmp;

	if (forward.X == 0 && forward.Y == 0)
	{
		right = FVector( 1, 0, 0 );
		up.X = -forward.Z;
		up.Y = 0;
		up.Z = 0;		
	}
	else
	{
		tmp = FVector( 0, 0, 1 );
		right = forward ^ tmp;
		right.Normalize();

		up = right ^ forward;
		up.Normalize();		
	}
}

FLOAT UavaSoundNodeAttenuation::MaxAudibleDistance(FLOAT CurrentMaxDistance)
{
	if( bAttenuate )
	{
		if( DistanceModel == avaATTENUATION_Linear || DistanceModel == avaATTENUATION_CalcDecibel && SoundPressure < 70)			
		{
			CurrentMaxDistance = ::Max<FLOAT>(CurrentMaxDistance,MaxRadius); 
		}

		if( DistanceModel == avaATTENUATION_CalcDecibel )
		{			
			const FLOAT dist_mult = SNDLVL_TO_DIST_MULT( SoundPressure );			 

			const FLOAT meter = 1 / ( (1 - SND_GAIN_THRESHOLD) * snd_gain_min + SND_GAIN_THRESHOLD * dist_mult );

			const FLOAT ref_dist = 52.5f;						

			return ::Max<FLOAT>(CurrentMaxDistance, meter * ref_dist );
		}
	}

	return CurrentMaxDistance;
}

void UavaSoundNodeAttenuation::ParseNodes( USoundNode* Parent, INT ChildIndex, UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances )
{
	if( AudioComponent->bAllowSpatialization )
	{
		if( bAttenuate )
		{
			RETRIEVE_SOUNDNODE_PAYLOAD( sizeof(FLOAT) + sizeof(FLOAT) );
			DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedMinRadius );
			DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedMaxRadius );

			if( *RequiresInitialization )
			{
				UsedMinRadius = MinRadius;
				UsedMaxRadius = MaxRadius;
				
				*RequiresInitialization = 0;
			}

			const FLOAT Distance = FDist( AudioComponent->CurrentLocation, AudioComponent->Listener->Location );
			const FLOAT dist_mult = SNDLVL_TO_DIST_MULT( SoundPressure );
			const FLOAT ref_dist = 52.5f;						

			if( DistanceModel == avaATTENUATION_Linear )
			{
				//debugf( TEXT("UsedMinRadius: %f,  UsedMaxRadius: %f,  Distance: %f"), UsedMinRadius, UsedMaxRadius, Distance );

				if( Distance >= UsedMaxRadius )
				{
					AudioComponent->CurrentVolume = 0.f;
				}
				// UsedMinRadius is the point at which to start attenuating
				else if( Distance > UsedMinRadius )
				{			
					// determine which AttenuationModel to use here
					//@todo audio:  create more attenuation models and just check which to use here
					// current is:  ATTENUATION_Linear
					AudioComponent->CurrentVolume *= 1.f - (Distance - UsedMinRadius) / (UsedMaxRadius - UsedMinRadius);
				}
			}
			else if( DistanceModel == avaATTENUATION_CalcDecibel)
			{				
				const FLOAT meter = Distance/ref_dist;				
				
				FLOAT gain = snd_gain_init;
				
				const FLOAT relative_dist = meter * dist_mult * dB_To_Gain( 29 * (meter/1000.f) /*+ GetReference_dB() - SoundPressure*/ );					
				
				// 10배 이상으로 커지지 못하게 하기 위해서
				if( relative_dist > 0.1 )
					gain *= (1/relative_dist);
				else
					gain *= 10.f;
				
				// 작은 소리에 대해서만 
				if (SoundPressure < 70)
				{
					UsedMinRadius = Min( UsedMinRadius, 1.5f * ref_dist );

					// UsedMinRadius is the point at which to start attenuating
					if( Distance < UsedMaxRadius && Distance > UsedMinRadius )
					{			
						// determine which AttenuationModel to use here
						//@todo audio:  create more attenuation models and just check which to use here
						// current is:  ATTENUATION_Linear
						gain = Max( gain, 1.f - (Distance - UsedMinRadius) / (UsedMaxRadius - UsedMinRadius) );
					}
					else if (Distance < UsedMinRadius)
					{
						gain = Max( gain, 1.f );
					}
				}

				gain = (gain - SND_GAIN_THRESHOLD) / (1 - SND_GAIN_THRESHOLD);

				if( gain > SND_GAIN_COMP_THRESH )
				{
					FLOAT snd_gain_comp_power = SND_GAIN_COMP_EXP_MAX;
					const FLOAT sndlvl = DIST_MULT_TO_SNDLVL( dist_mult );
					
					if( sndlvl > SND_DB_MED )
						//snd_gain_comp_power = RemapVal( sndlvl, float(SND_DB_MED), SND_DB_MAX, SND_GAIN_COMP_EXP_MAX, SND_GAIN_COMP_EXP_MIN );
						snd_gain_comp_power = RemapVal( sndlvl, SND_DB_MED, SND_DB_MAX, SND_GAIN_COMP_EXP_MAX, SND_GAIN_COMP_EXP_MIN );

					// calculate crossover point
					const FLOAT Y = -1.f/(powf(float(SND_GAIN_COMP_THRESH), snd_gain_comp_power) * (SND_GAIN_COMP_THRESH - 1));

					// Calculate compressed gain
					gain = 1.f - 1.f / (Y * powf(gain, snd_gain_comp_power));
					gain = gain*snd_gain_max;
				}

				if( gain < 0 )
				{
					gain = 0;
				}
				else if( gain < snd_gain_min )
				{
					gain = snd_gain_min * (2.f - relative_dist * snd_gain_min);
					
					if( gain <= 0.0)
						gain = 0.001f;
				}

				AudioComponent->CurrentVolume *= gain;
			}

			FLOAT CurrentTime = GWorld->GetTimeSeconds();
			if (CurrentTime > LastCheckTime + 0.25f) 
			{
				ObstructionGainTarget = 1.0f;

				// Trace the line.
				FCheckResult Hit(1.f);
				GWorld->SingleLineCheck( Hit, NULL, AudioComponent->CurrentLocation, AudioComponent->Listener->Location, TRACE_World|TRACE_StopAtAnyHit );

				if (Hit.Actor != NULL)
				{
					const FLOAT sndlvl = DIST_MULT_TO_SNDLVL( dist_mult );
					FLOAT radius = dB_To_Meter( sndlvl ) * GetReference_dB();		// approximate radius from soundlevel;

					FVector EndPoints[4];
					FVector vsrc_forward, vsrc_up, vsrc_right, vecl, vecr, vecr2, vecl2;

					for (INT i = 0; i < 4; i++)
						EndPoints[i] = AudioComponent->CurrentLocation;

					vsrc_forward = AudioComponent->Listener->Location - AudioComponent->CurrentLocation;
					vsrc_forward.Normalize();
					VectorVectors( vsrc_forward, vsrc_right, vsrc_up );

					vecl = vsrc_up + vsrc_right;

					// if src above listener, force 'up' vector to point down - create diagonals up & down

					if ( AudioComponent->CurrentLocation.Z > AudioComponent->Listener->Location.Z + (10 * 16) )
						vsrc_up.Z = -vsrc_up.Z;

					vecr = vsrc_up - vsrc_right;
					vecl.Normalize();
					vecr.Normalize();				

					// get diagonal vectors from sound source 

					vecl2 = radius * vecl;
					vecr2 = radius * vecr;
					vecl = (radius / 2.0f) * vecl;
					vecr = (radius / 2.0f) * vecr;

					// EndPoints from diagonal vectors

					EndPoints[0] += vecl;
					EndPoints[1] += vecr;
					EndPoints[2] += vecl2;
					EndPoints[3] += vecr2;

					// drop gain for each point on radius diagonal that is obscured

					for (INT count = 0, i = 0; i < 4; i++)
					{
						// UNDONE: some EndPoints are in walls - in this case, trace from the wall hit location

						FCheckResult Hit(1.f);
						GWorld->SingleLineCheck( Hit, NULL, EndPoints[i], AudioComponent->Listener->Location, TRACE_World|TRACE_StopAtAnyHit );					

						if (Hit.Actor != NULL)
						{
							count++;	// skip first obscured point: at least 2 points + center should be obscured to hear db loss
							if (count > 1)
								ObstructionGainTarget *= dB_To_Gain( -2.70f/*g_snd_obscured_loss_db*/ );
						}
					}
				}
			}

			if (ObstructionGain != ObstructionGainTarget)
			{
#define SND_GAIN_FADE_TIME 0.25f
				const FLOAT Delta = ObstructionGainTarget - ObstructionGain;
				const FLOAT Speed = ( GWorld ? (GWorld->GetDeltaSeconds() / SND_GAIN_FADE_TIME) : 1.0f ) * Delta;

				const FLOAT Increment = fabsf(Speed);		

				// if not hit target, keep approaching
				if ( fabsf( Delta ) > Increment )
				{
					if (Delta > 0)
						ObstructionGain += Increment;
					else
						ObstructionGain -= Increment;
				}
				else
				{					
					ObstructionGain = ObstructionGainTarget;
				}
			}

			AudioComponent->CurrentVolume *= ObstructionGain;
		}

		AudioComponent->CurrentUseSpatialization |= bSpatialize;

		/*if (AudioComponent->CurrentUseSpatialization && !AudioComponent->bAllowSpatialization)
		{
			AudioComponent->CurrentHeadRelative = TRUE;
		}*/
	}
	else
	{
		AudioComponent->CurrentUseSpatialization = 0;
	}

	Super::ParseNodes( Parent, ChildIndex, AudioComponent, WaveInstances );
}

void UavaSoundNodePlaytime::NotifyWaveInstanceFinished( struct FWaveInstance* WaveInstance )
{
	UAudioComponent* AudioComponent = WaveInstance->AudioComponent;
	RETRIEVE_SOUNDNODE_PAYLOAD( sizeof(FLOAT) + sizeof(INT) );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, StartOfPlay );
	DECLARE_SOUNDNODE_ELEMENT( INT, FinishedCount );
	check(*RequiresInitialization == 0 );

	if( GWorld->GetTimeSeconds() - StartOfPlay < StartDelayTime + PlayTime )
	{
		// Add to map of nodes that might need to have bFinished reset.
		AudioComponent->SoundNodeResetWaveMap.AddUnique( this, WaveInstance );

		// Figure out how many child wave nodes are in subtree - this could be precomputed.
		INT NumChildNodes = 0;
		TArray<USoundNode*> CurrentChildNodes;
		GetNodes( AudioComponent, CurrentChildNodes );
		for( INT NodeIndex=1; NodeIndex<CurrentChildNodes.Num(); NodeIndex++ )
		{
			if( Cast<USoundNodeWave>(CurrentChildNodes(NodeIndex)) )
			{
				NumChildNodes++;
			}
		}

		// Wait till all leaves are finished.
		if( ++FinishedCount == NumChildNodes )
		{
			FinishedCount = 0;

			// Retrieve all child nodes.
			TArray<USoundNode*> AllChildNodes;
			GetAllNodes( AllChildNodes );

			// GetAllNodes includes current node so we have to start at Index 1.
			for( INT NodeIndex=1; NodeIndex<AllChildNodes.Num(); NodeIndex++ )
			{
				// Reset all child nodes so they are initialized again.
				USoundNode* ChildNode = AllChildNodes(NodeIndex);
				UINT* Offset = AudioComponent->SoundNodeOffsetMap.Find( ChildNode );
				if( Offset )
				{
					UBOOL* RequiresInitialization = (UBOOL*) &AudioComponent->SoundNodeData( *Offset );
					*RequiresInitialization = TRUE;
				}
			}

			// Reset wave instances that notified us of completion.
			ResetWaveInstances( AudioComponent );
		}
	}
}

void UavaSoundNodePlaytime::ParseNodes( USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances )
{
	RETRIEVE_SOUNDNODE_PAYLOAD( sizeof(FLOAT) + sizeof(INT) );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, StartOfPlay );
	DECLARE_SOUNDNODE_ELEMENT( INT, FinishedCount );

	if( *RequiresInitialization )
	{
		StartOfPlay = GWorld->GetTimeSeconds();
		*RequiresInitialization = FALSE;
	}

	FLOAT TimeSpentPlaying = GWorld->GetTimeSeconds()-StartOfPlay;

	if( TimeSpentPlaying < StartDelayTime )
		AudioComponent->bFinished = FALSE;
	else if( TimeSpentPlaying  < StartDelayTime + PlayTime )
	{
		AudioComponent->bFinished = FALSE;
		AudioComponent->CurrentNotifyBufferFinishedHook = this;
		AudioComponent->CurrentUseSeamlessLooping		= TRUE;
		Super::ParseNodes( Parent, ChildIndex, AudioComponent, WaveInstances );
	}
	else
		AudioComponent->bFinished = TRUE;
}

FLOAT UavaSoundNodePlaytime::GetDuration()
{
	return StartDelayTime + PlayTime;
}