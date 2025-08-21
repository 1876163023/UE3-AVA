#include "EnginePrivate.h"
#include "Engine.h"
#include "EngineDSPClasses.h"

IMPLEMENT_CLASS(UReverbProperty)

void UReverbProperty::PostEditChange(UProperty* PropertyThatChanged)
{
	UAudioDevice* AudioDevice = GEngine->Client->GetAudioDevice();
	if( AudioDevice )
	{
		AudioDevice->OnReverbSettingsChanged();
	}

	Super::PostEditChange(PropertyThatChanged);
}

void AReverbVolume::PostEditChange(UProperty* PropertyThatChanged)
{
	UAudioDevice* AudioDevice = GEngine->Client->GetAudioDevice();
	if( AudioDevice )
	{
		AudioDevice->OnReverbSettingsChanged();
	}

	Super::PostEditChange(PropertyThatChanged);
}