/*=============================================================================
	AnimationCompressionAlgorithm_BitwiseCompressionOnly.cpp: Bitwise animation compression only; performs no key reduction.
	Copyright 2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

#include "EnginePrivate.h"
#include "EngineAnimClasses.h"
#include "AnimationUtils.h"

IMPLEMENT_CLASS(UAnimationCompressionAlgorithm_BitwiseCompressOnly);

/**
 * Bitwise animation compression only; performs no key reduction.
 */
void UAnimationCompressionAlgorithm_BitwiseCompressOnly::DoReduction(UAnimSequence* AnimSeq, USkeletalMesh* SkelMesh, const TArray<FBoneData>& BoneData)
{
	// Separate to raw data to tracks so that the bitwise compressor will have a data to operate on.
	AnimSeq->SeparateRawDataToTracks( AnimSeq->RawAnimData, AnimSeq->SequenceLength, AnimSeq->TranslationData, AnimSeq->RotationData );
}
