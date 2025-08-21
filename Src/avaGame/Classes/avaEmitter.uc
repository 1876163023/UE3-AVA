/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaEmitter extends Emitter
	native;

/** Sound to play when spawned */
var()	SoundCue		Sound;
/** Particle System to use */
var()	ParticleSystem	ParticleSystem;

simulated function PostBeginPlay()
{
	super.PostBeginPlay();

	if( WorldInfo.NetMode != NM_DedicatedServer )
	{
		if (ParticleSystem != None)
		{
			SetTemplate( ParticleSystem, bDestroyOnSystemFinish );
		}

		if( Sound != None )
		{
			PlaySound( Sound );
		}
	}
}


/** This is mostly used by the Object pool system **/
function HideSelf()
{
	ParticleSystemComponent.DeActivateSystem(); // if we are hiding this we want to deactivate also
	SetHidden( TRUE );
	bStasis = TRUE;
}

defaultproperties
{
	Begin Object Name=ParticleSystemComponent0
		bAcceptsLights=false
		SecondsBeforeInactive=0
	End Object

	//LifeSpan=7.0
	bNetInitialRotation=true
	bDestroyOnSystemFinish=true
	bNoDelete=false
}
