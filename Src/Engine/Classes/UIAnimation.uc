/**
 * Copyright 1998-2007 Epic Games, Inc. All Rights Reserved.
 */

class UIAnimation extends UIRoot
	abstract
	native(UserInterface);


// Describes what value is animated on a given Opperation
enum EUIAnimType
{
	EAT_Position,	// We are animating the position of the widget
	EAT_RelPosition,// We are animating the Position of a widget relative to it's anchor point
	EAT_Rotation,	// We are animating the rotation of the widget
	EAT_RelRotation,// We are animating the rotation of the widget relative to it's starting rotation.
	EAT_Color,		// We are animating the Color of the widget
	EAT_Opacity,	// We are animating the opacity of the widget
	EAT_Visibility,	// We are animating the visibiltiy of the widget
	EAT_Scale,		// We are animating the scale of the widget
};


/** The different type of notification events */
// Widget Render Modifier만 쓸겁니다. 이런건 안씁니다.
//enum EUIAnimNotifyType
//{
//	EANT_WidgetFunction,
//	EANT_SceneFunction,
//	EANT_KismetEvent,
//	EANT_Sound,
//};


/**
 * Holds information about a given notify
 */
// Widget Render Modifier만 쓸겁니다. 이런건 안씁니다.
//struct native UIAnimationNotify
//{
//	// What type of notication is this
//	var EUIAnimNotifyType NotifyType;
//
//	/** Holds the name of the function to call or UI sound to play */
//
//	var name NotifyName;
//};


/**
 * We don't have unions in script so we burn a little more space
 * than I would like.  Which value will be used depends on the OpType.
 */
// UDistributionFloat, UDistributionVector로 대체
//struct native UIAnimationRawData
//{
//	var float				DestAsFloat;
//	var LinearColor 		DestAsColor;
//	var Rotator				DestAsRotator;
//	var Vector				DestAsVector;
//	var UIAnimationNotify   DestAsNotify;
//};

///**
// * UTUIAnimationKeyFrames are a collection of 1 or more UTUIAnimationOps that
// * will be applied to a given widget.
// */
//struct native UIAnimationKeyFrame
//{
//	/** This is the timemark where this frame is fully in effect.  Note Timemark is
//		represented as a % of the total  */
//	var float TimeMark;
//
//	/** This holds the array of AnimationOps that will be applied to this Widget */
//	var UIAnimationRawData Data;
//};


/**
 * This defines a single animation track.  Each track will animation only a single
 * type of data.
 */

struct native UIAnimTrack
{
	/** The type of animation date contained in this track */
	var EUIAnimType TrackType;

	/** Which child widget does this track affect.  Can be NAME_None */
	var name TrackWidgetTag;

	/** @deprecated - Holds the actual key frame data */
	//var array<UIAnimationKeyFrame> KeyFrames;

	/** Can be a DistributionFloat or DistributionVector */
	//var export Object Distribution;
	var RawDistributionFloat	DistFloat;
	var RawDistributionVector	DistVector;

	/* ---------- These are only available when playing -----*/

	/** The widget we are being applied to if it's not the current */
	var transient UIObject TargetWidget;

	structdefaultproperties
	{
		Begin Object Class=DistributionFloatConstantCurve Name=NewDistributionFloat
			Constant=1.0;
		End Object
		DistFloat=(Distribution=NewDistributionFloat)

		Begin Object Class=DistributionVectorConstantCurve Name=NewDistributionVector
			Constant=(X=1.0,Y=1.0,Z=1.0)
		End Object
		DistVector=(Distribution=NewDistributionVector)
	}
};


/** Holds a reference to an animation Sequence */

struct native UIAnimSeqRef
{

	/** The Template to use */
	var UIAnimationSeq SeqRef;

	/** How fast are we playing this back */
	var float PlaybackRate;

	/** How long have we been playing */
	var float AnimTime;

	/** This animation is playing */
	var bool bIsPlaying;

	/** This animation is looping */
	var bool bIsLooping;

	/** Holds a count of the # of times this animation has looped. */
	var int LoopCount;

	/** Initial rendering offset before any animation has occurred. */
	var vector InitialRenderOffset;

	/** Initial rotation before any animation has occurred. */
	var rotator InitialRotation;
};

defaultproperties
{
}
