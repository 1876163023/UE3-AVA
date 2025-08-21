#include "PrecompiledHeaders.h"
#include "avaGame.h"
#include "avaFactories.h"

#define EVALPOS_MAX 7
#define UIFACE_MAX  4

#if !(CONSOLE || FINAL_RELEASE)
#include "..\..\UnrealEd\Inc\ResourceIDs.h"
#endif

void DSP_Allocate( INT* );
void DSP_Free( INT Handle );
void DSP_Apply( INT Slot, INT Handle, FLOAT Duration, FLOAT Fade );
UBOOL DSP_Reset( INT Handle );
void DSP_LowPass( INT Handle, float CutOff, float Resonance );
void DSP_HighPass( INT Handle, float CutOff, float Resonance );
void DSP_Echo( INT Handle, float Delay, float DecayRatio, float DryMix, float WetMix );
void DSP_Reverb( INT Handle, float Room, float Damp, float DryMix, float WetMix, float Width );
void DSP_Software( INT Handle, float gain, INT num_blocks, void* );

IMPLEMENT_CLASS(UavaDSPSoftware);
IMPLEMENT_CLASS(UavaDSPSoftwareBlock);

#if !(CONSOLE || FINAL_RELEASE)

IMPLEMENT_CLASS(UavaGBType_DSPPreset);

UBOOL Generic_ShowObjectEditor( UObject* InObject, const FString& Title );

IMPLEMENT_CLASS(UavaGBType_SoundScapeProperty);

void AddSupportInfoItem_Sound( TArrayNoInit<struct FGenericBrowserTypeInfo>&, UClass* Class, FColor color );

void UavaGBType_SoundScapeProperty::Init()
{
	SupportInfo.AddItem( FGenericBrowserTypeInfo( UavaSoundScapeProperty::StaticClass(), FColor(200,192,128), NULL ) );
}

UBOOL UavaGBType_SoundScapeProperty::ShowObjectEditor( UObject* InObject )
{
	return Generic_ShowObjectEditor( InObject, FString::Printf( TEXT("SoundScapeProperpty : %s"), *InObject->GetPathName() ) );	
}

void UavaGBType_DSPPreset::Init()
{
	AddSupportInfoItem_Sound( SupportInfo, UavaDSPPreset::StaticClass(), FColor(200,192,128) );		
}

UBOOL UavaGBType_DSPPreset::ShowObjectEditor( UObject* InObject )
{
	return Generic_ShowObjectEditor( InObject, FString::Printf( TEXT("DSP Preset : %s"), *InObject->GetPathName() ) );
}

void UavaGBType_DSPPreset::InvokeCustomCommand( INT InCommand, UObject* InObject )
{
	if( InCommand == IDMN_ObjectContext_Sound_Play )
	{
		UavaDSPPreset*	dsp = Cast<UavaDSPPreset>(InObject);
		
		if (dsp)
		{
			dsp->Apply();
		}
	}
	else if( InCommand == IDMN_ObjectContext_Sound_Stop )
	{
		UavaDSPPreset*	dsp = Cast<UavaDSPPreset>(InObject);

		if (dsp)
		{
			dsp->Stop();
		}		
	}
}

#endif

IMPLEMENT_CLASS(UavaDSPBlock);
IMPLEMENT_CLASS(UavaDSPPreset);
IMPLEMENT_CLASS(UavaDSPEcho);
IMPLEMENT_CLASS(UavaDSPLowPass);
IMPLEMENT_CLASS(UavaDSPHighPass);
IMPLEMENT_CLASS(UavaDSPReverb);

void UavaDSPBlock::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);

	Cast<UavaDSPPreset>(GetOuter())->Reload();
}

UavaDSPPreset::UavaDSPPreset()
: Dirty( TRUE ), Handle(-1)
{		
}

void UavaDSPPreset::FinishDestroy()
{
	DSP_Free( Handle );

	Super::FinishDestroy();
}

void UavaDSPPreset::Apply()
{
	if (Dirty)
		Reload();

	DSP_Apply( DSPSlot, Handle, Duration, Fade );
}

void UavaDSPPreset::Stop()
{
	DSP_Apply( DSPSlot, -1, 0, 0 );
}

void UavaDSPPreset::Reload()
{
	if (Handle < 0)
		DSP_Allocate( &Handle );

	if (Handle < 0)
		return;

	UBOOL Activated = DSP_Reset( Handle );
	
	for (INT i=0; i<DSPBlocks.Num(); ++i)
	{
		UavaDSPBlock* pBlock = DSPBlocks(i);

		if (!pBlock) continue;

		if (pBlock->IsA( UavaDSPLowPass::StaticClass() ))
		{
			UavaDSPLowPass* pFilter = Cast<UavaDSPLowPass>( pBlock );

			DSP_LowPass( Handle, pFilter->CutOff, pFilter->Resonance );
		}
		else if (pBlock->IsA( UavaDSPHighPass::StaticClass() ))
		{
			UavaDSPHighPass* pFilter = Cast<UavaDSPHighPass>( pBlock );

			DSP_LowPass( Handle, pFilter->CutOff, pFilter->Resonance );
		}
		else if (pBlock->IsA( UavaDSPEcho::StaticClass() ))
		{
			UavaDSPEcho* pFilter = Cast<UavaDSPEcho>( pBlock );

			DSP_Echo( Handle, pFilter->delay, pFilter->DecayRatio, pFilter->DryMix, pFilter->WetMix );
		}
		else if (pBlock->IsA( UavaDSPReverb::StaticClass() ))
		{
			UavaDSPReverb* pFilter = Cast<UavaDSPReverb>( pBlock );

			DSP_Reverb( Handle, pFilter->RoomSize, pFilter->Damp, pFilter->DryMix, pFilter->WetMix, pFilter->Width );
		}
		else if (pBlock->IsA( UavaDSPSoftware::StaticClass() ))
		{
			UavaDSPSoftware* pFilter = Cast<UavaDSPSoftware>( pBlock );			

			FDescriptor blocks[64];

			for (INT i=0; i<pFilter->Blocks.Num(); ++i)
			{
				if (pFilter->Blocks(i))
					pFilter->Blocks(i)->eventFillValue( blocks[i] );
			}
			
			DSP_Software( Handle, pFilter->Gain, pFilter->Blocks.Num(), blocks );
		}
	}

	Dirty = FALSE;

	if (Activated)
		DSP_Apply( DSPSlot, Handle, Duration, Fade );
}

void UavaDSPPresetFactoryNew::StaticConstructor()
{
	new(GetClass()->HideCategories) FName(TEXT("Object"));
}

/**
* Initializes property values for intrinsic classes.  It is called immediately after the class default object
* is initialized against its archetype, but before any objects of this class are created.
*/
void UavaDSPPresetFactoryNew::InitializeIntrinsicPropertyValues()
{
	SupportedClass	= UavaDSPPreset::StaticClass();
	bCreateNew		= 1;
	Description		= TEXT("AVA DSP Preset");
}
//
//	UCurveEdPresetCurveFactoryNew::FactoryCreateNew
//
UObject* UavaDSPPresetFactoryNew::FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn)
{
	return StaticConstructObject(Class,InParent,Name,Flags);
}

IMPLEMENT_CLASS(UavaDSPPresetFactoryNew);

void UavaSoundScapePropertyFactoryNew::StaticConstructor()
{
	new(GetClass()->HideCategories) FName(TEXT("Object"));
}

/**
* Initializes property values for intrinsic classes.  It is called immediately after the class default object
* is initialized against its archetype, but before any objects of this class are created.
*/
void UavaSoundScapePropertyFactoryNew::InitializeIntrinsicPropertyValues()
{
	SupportedClass	= UavaSoundScapeProperty::StaticClass();
	bCreateNew		= 1;
	Description		= TEXT("AVA SoundScape");
}
//
//	UCurveEdPresetCurveFactoryNew::FactoryCreateNew
//
UObject* UavaSoundScapePropertyFactoryNew::FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn)
{
	return StaticConstructObject(Class,InParent,Name,Flags);
}

IMPLEMENT_CLASS(UavaSoundScapePropertyFactoryNew);
