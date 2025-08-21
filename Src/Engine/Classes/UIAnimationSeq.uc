/**
 * Copyright 1998-2007 Epic Games, Inc. All Rights Reserved.
 */

class UIAnimationSeq extends UIAnimation
	native(inherit)
	hidecategories(Object)
	editinlinenew;
//	native(UIPrivate);

/** The name of this sequence */
var() name SeqName;

/** The default duration.  PlayUIAnimation can be used to override this */
//var() float SeqDuration;

/** Holds a list of Animation Tracks in this sequence */
var editinline export array<UIAnimTrack> Tracks;

var const float	InTimeMin;	/** InValue LowerBound (Time Unit) */
var const float	InTimeMax;	/** InValue UpperBound (Time Unit)*/
///** If true, any positional changes made using this sequence will be considered as
//    absolute position changes (ie: Move the Archor point to absolute pixel 0.1,0.1
//	instead of moving it 0.1,0.1 pixels. */
//
//var bool bAbsolutePositioning;

/**
 * Apply this animation.  Note, if LFI = NFI this will simply apply the
 * frame without any interpolation
 *
 * @Param TargetWidget		The Widget to apply the animation to
 * @Param Position			Where in the animation are we
 * @Param LFI				The Last Frame Index
 * @Param NFI				The Next Frame Index
 **/

//native function ApplyAnimation(UIObject TargetWidget, INT TrackIndex, FLOAT Position, INT LFI, INT NFI, UIAnimSeqRef AnimRefInst );
native function ApplyAnimation( UIObject TargetWidget, int TrackIndex, float Ratio, UIAnimSeqRef AnimRefInst );

/**
 * AnimType에 해당하는 트랙의 인덱스를 반환한다. 없으면 새로 생성한후 인덱스를 반환한다
 */
native function int GetAnimTrack( EUIAnimType AnimType );

/** AnimType에 따라 필요한 DistributionClass가 다를 수 있다 
 * 예를 들어 Opacity는 DistributionFloat, Rotation은 DistributionVector가 필요하다
 */
native final function class<Object> GetProperDistributionClass( EUIAnimType AnimType );

event int AddEmptyTrackTyped( EUIAnimType AnimType )
{
	Local int TracksLength;
	Local class<Object> DistClass;

	DistClass = GetProperDistributionClass( AnimType );

	if( DistClass == none )
	{
		`warn( "Unexpected AnimType for "$Self$".AddEmptyTrackTyped()" );
		return INDEX_NONE;
	}

	TracksLength = Tracks.Length;
	Tracks.Add(1);

	Tracks[TracksLength].TrackType = AnimType;
	return TracksLength;
}

/** 트랙중에 가장 큰 구간을 계산해서 나중에 에니메이션 재생시 사용한다 */
native final function RecalcInTimeRange();

defaultproperties
{
}
