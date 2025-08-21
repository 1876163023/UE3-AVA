//=============================================================================
// Copyright 1998-2007 Epic Games - All Rights Reserved.
// Confidential.
//=============================================================================
#include "EnginePrivate.h"
#include "CanvasScene.h"

#include "EngineUserInterfaceClasses.h"
#include "EngineUIPrivateClasses.h"

#include "EngineSequenceClasses.h"
#include "EngineUISequenceClasses.h"

IMPLEMENT_CLASS(UUIAnimation);
IMPLEMENT_CLASS(UUIAnimationSeq);


/*=========================================================================================
  UUIAnimationSeq - The actual animation sequence
  ========================================================================================= */
void UUIAnimationSeq::ApplyAnimation(UUIObject* TargetWidget, INT TrackIndex, FLOAT Position, struct FUIAnimSeqRef AnimRefInst)
{
	check( Tracks.IsValidIndex(TrackIndex) );

	FUIAnimTrack& Track = Tracks(TrackIndex);

	FRawDistributionFloat* DistFloat = Track.DistFloat.IsUniform() ? NULL : &Track.DistFloat;
	FRawDistributionVector* DistVect = Track.DistVector.IsUniform() ? NULL : &Track.DistVector;

	FLOAT CurrTime = InTimeMin + Position * (InTimeMax - InTimeMin);

	switch ( Track.TrackType )
	{
		case EAT_Opacity:
		{
			if( DistFloat )
			{
				FLOAT NewOpacity = DistFloat->GetValue(CurrTime);
				TargetWidget->AnimSetOpacity(NewOpacity);
			}
			break;
		}

		case EAT_Visibility:
		{
			if( DistFloat )
			{
				UBOOL NewVisibility = DistFloat->GetValue(CurrTime) > 0.f;
				TargetWidget->AnimSetVisibility( NewVisibility );
			}
			break;
		}

		case EAT_Color:
		{
			if( DistVect )
			{
				FVector NewColor = DistVect->GetValue(CurrTime);
				// @ saperate with alpha later.
				//TargetWidget->AnimSetColor(NewColor);
			}
			break;
		}

		case EAT_Rotation:
		{
			if( DistVect )
			{
				// use each vector component as a rotator component
				// Vector.X => Rotator.Pitch
				// Vector.Y => Rotator.Yaw
				// Vector.Z => Rotator.Roll
				FVector RotVec = DistVect->GetValue(CurrTime) * MAXWORD/2;
				FRotator Rot( RotVec.X, RotVec.Y, RotVec.Z );
				TargetWidget->AnimSetRotation( Rot );
			}
			break;
		}

		case EAT_RelRotation:
		{
			if( DistVect )
			{
				FVector RotVec = DistVect->GetValue(CurrTime) * MAXWORD/2;
				FRotator FinalRot = FRotator( RotVec.X, RotVec.Y, RotVec.Z ) + AnimRefInst.InitialRotation;
				TargetWidget->AnimSetRotation( FinalRot );
			}
			break;
		}

		case EAT_Position:
		{
			if( DistVect )
			{
				FVector NewPos = DistVect->GetValue(CurrTime);
				TargetWidget->AnimSetPosition(NewPos);
			}
			break;
		}

		case EAT_RelPosition:
		{
			if( DistVect )
			{
				FVector NewPos = DistVect->GetValue(CurrTime);
				TargetWidget->AnimSetRelPosition(NewPos, AnimRefInst.InitialRenderOffset);
			}
			break;
		}

		case EAT_Scale:
		{
			if( DistVect )
			{
				FVector NewScale = DistVect->GetValue(CurrTime);
				TargetWidget->AnimSetScale( NewScale );
			}
		}
		// TODO - Add suport for a notify track

	}
}

UClass* UUIAnimationSeq::GetProperDistributionClass(BYTE AnimType)
{
	switch( AnimType )
	{
	case EAT_Position:
	case EAT_RelPosition:
	case EAT_Rotation:
	case EAT_RelRotation:
	case EAT_Scale:
		return UDistributionVectorConstantCurve::StaticClass();
		break;
	case EAT_Color:
	case EAT_Opacity:
	case EAT_Visibility:
		return UDistributionFloatConstantCurve::StaticClass();
		break;
	default:
		check( FALSE );
		return NULL;
		break;
	}
}

INT UUIAnimationSeq::GetAnimTrack( BYTE AnimType )
{
	INT TrackIndexFound = INDEX_NONE;

	FUIAnimTrack* AnimTrack = NULL;
	for( INT TrackIndex = 0 ; TrackIndex < Tracks.Num() ; TrackIndex++ )
	{
		if( Tracks(TrackIndex).TrackType == AnimType )
		{
			TrackIndexFound = TrackIndex;
			break;
		}
	}

	if( TrackIndexFound == INDEX_NONE )
	{
		INT TrackIndex = eventAddEmptyTrackTyped( AnimType );
		if( TrackIndex != INDEX_NONE && Tracks.IsValidIndex(TrackIndex) )
		{
			AnimTrack = &Tracks(TrackIndex);
			if( AnimTrack->DistFloat.Distribution == NULL )
			{
				AnimTrack->DistFloat.Distribution = ConstructObject<UDistributionFloat>(UDistributionFloatConstantCurve::StaticClass(), this);
			}
			if( AnimTrack->DistVector.Distribution == NULL )
			{
				AnimTrack->DistVector.Distribution = ConstructObject<UDistributionVector>(UDistributionVectorConstantCurve::StaticClass(), this);
			}

			TrackIndexFound = TrackIndex;
			Modify();
		}
	}

	return TrackIndexFound;
}

void UUIAnimationSeq::RecalcInTimeRange()
{
	TArray<FLOAT> MaxValueList;
	TArray<FLOAT> MinValueList;

	for( INT TrackIndex = 0 ; TrackIndex < Tracks.Num() ; TrackIndex++ )
	{
		FLOAT MinValue;
		FLOAT MaxValue;
		FUIAnimTrack& AnimTrack = Tracks(TrackIndex);

		UDistributionFloat* DistFloat = Cast<UDistributionFloat>(AnimTrack.DistFloat.Distribution);
		UDistributionVector* DistVect = Cast<UDistributionVector>(AnimTrack.DistVector.Distribution);

		UBOOL IsDistExist = TRUE;
		if( DistFloat )
		{
			DistFloat->GetInRange( MinValue, MaxValue );
		}
		else if ( DistVect )
		{
			DistVect->GetInRange( MinValue, MaxValue );
		}
		else
		{
			IsDistExist = FALSE;
		}

		if( IsDistExist )
		{
			MinValueList.AddItem(MinValue);
			MaxValueList.AddItem(MaxValue);
		}
	}

	if( MaxValueList.Num() == 0 || MinValueList.Num() == 0 )
	{
		InTimeMax = 0;
		InTimeMin = 0;
	}
	else
	{
		InTimeMin = MinValueList(0);
		InTimeMax = MaxValueList(0);
		for( INT ValueIndex = 1 ; ValueIndex < MinValueList.Num() ; ValueIndex++ )
			if( InTimeMin > MinValueList(ValueIndex) )
				InTimeMin = MinValueList(ValueIndex);

		for( INT ValueIndex = 1 ; ValueIndex < MaxValueList.Num() ; ValueIndex++ )
			if( InTimeMax < MaxValueList(ValueIndex) )
				InTimeMax = MaxValueList(ValueIndex);
	}
}