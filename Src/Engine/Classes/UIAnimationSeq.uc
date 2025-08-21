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
 * AnimType�� �ش��ϴ� Ʈ���� �ε����� ��ȯ�Ѵ�. ������ ���� �������� �ε����� ��ȯ�Ѵ�
 */
native function int GetAnimTrack( EUIAnimType AnimType );

/** AnimType�� ���� �ʿ��� DistributionClass�� �ٸ� �� �ִ� 
 * ���� ��� Opacity�� DistributionFloat, Rotation�� DistributionVector�� �ʿ��ϴ�
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

/** Ʈ���߿� ���� ū ������ ����ؼ� ���߿� ���ϸ��̼� ����� ����Ѵ� */
native final function RecalcInTimeRange();

defaultproperties
{
}
