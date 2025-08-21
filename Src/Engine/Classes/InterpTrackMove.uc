class InterpTrackMove extends InterpTrack
	native(Interpolation);

/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 *
 * Track containing data for moving an actor around over time.
 * There is no UpdateTrack function. In the game, its the PHYS_Interpolating physics mode which 
 * updates the position based on the interp track.
 */

cpptext
{
	// UObject interface
	virtual void PostEditChange(UProperty* PropertyThatChanged);

	// InterpTrack interface
	virtual INT GetNumKeyframes();
	virtual void GetTimeRange(FLOAT& StartTime, FLOAT& EndTime);
	virtual FLOAT GetKeyframeTime(INT KeyIndex);
	virtual INT AddKeyframe(FLOAT Time, UInterpTrackInst* TrInst);
	virtual void UpdateKeyframe(INT KeyIndex, UInterpTrackInst* TrInst);
	virtual INT SetKeyframeTime(INT KeyIndex, FLOAT NewKeyTime, UBOOL bUpdateOrder=true);
	virtual void RemoveKeyframe(INT KeyIndex);
	virtual INT DuplicateKeyframe(INT KeyIndex, FLOAT NewKeyTime);
	virtual UBOOL GetClosestSnapPosition(FLOAT InPosition, TArray<INT> &IgnoreKeys, FLOAT& OutPosition);

	virtual void ConditionalPreviewUpdateTrack(FLOAT NewPosition, class UInterpTrackInst* TrInst);
	virtual void PreviewUpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst);

	virtual class UMaterial* GetTrackIcon();
	virtual FColor GetKeyframeColor(INT KeyIndex);
	virtual void Render3DTrack(UInterpTrackInst* TrInst, const FSceneView* View, FPrimitiveDrawInterface* PDI, INT TrackIndex, const FColor& TrackColor, TArray<class FInterpEdSelKey>& SelectedKeys);
	virtual void SetTrackToSensibleDefault();

	// FCurveEdInterface interface
	virtual INT		GetNumKeys();
	virtual INT		GetNumSubCurves();
	virtual FLOAT	GetKeyIn(INT KeyIndex);
	virtual FLOAT	GetKeyOut(INT SubIndex, INT KeyIndex);
	virtual void	GetInRange(FLOAT& MinIn, FLOAT& MaxIn);
	virtual void	GetOutRange(FLOAT& MinOut, FLOAT& MaxOut);
	virtual FColor	GetKeyColor(INT SubIndex, INT KeyIndex, const FColor& CurveColor);
	virtual BYTE	GetKeyInterpMode(INT KeyIndex);
	virtual void	GetTangents(INT SubIndex, INT KeyIndex, FLOAT& ArriveTangent, FLOAT& LeaveTangent);
	virtual FLOAT	EvalSub(INT SubIndex, FLOAT InVal);

	virtual INT		CreateNewKey(FLOAT KeyIn);
	virtual void	DeleteKey(INT KeyIndex);

	virtual INT		SetKeyIn(INT KeyIndex, FLOAT NewInVal);
	virtual void	SetKeyOut(INT SubIndex, INT KeyIndex, FLOAT NewOutVal);
	virtual void	SetKeyInterpMode(INT KeyIndex, EInterpCurveMode NewMode);
	virtual void	SetTangents(INT SubIndex, INT KeyIndex, FLOAT ArriveTangent, FLOAT LeaveTangent);


	// InterpTrackMove interface
	virtual void GetKeyTransformAtTime(FLOAT Time, FVector& OutPos, FRotator& OutRot);
	virtual void GetLocationAtTime(UInterpTrackInst* TrInst, FLOAT Time, FVector& OutPos, FRotator& OutRot);
	virtual FMatrix GetMoveRefFrame(UInterpTrackInstMove* MoveTrackInst);

	INT CalcSubIndex(UBOOL bPos, INT Index);
}

/** Actual position keyframe data. */
var		InterpCurveVector	PosTrack;

/** Actual rotation keyframe data, stored as Euler angles in degrees, for easy editing on curve. */
var		InterpCurveVector	EulerTrack;

/** When using IMR_LookAtGroup, specifies the Group which this track should always point its actor at. */
var()	name				LookAtGroupName;

/** Controls the tightness of the curve for the translation path. */
var()	float				LinCurveTension;

/** Controls the tightness of the curve for the rotation path. */
var()	float				AngCurveTension;

/** 
 *	Use a Quaternion linear interpolation between keys. 
 *	This is robust and will find the 'shortest' distance between keys, but does not support ease in/out.
 */
var()	bool				bUseQuatInterpolation;

/** In the editor, show a small arrow at each keyframe indicating the rotation at that key. */
var()	bool				bShowArrowAtKeys;

/** Disable previewing of this track - will always position Actor at Time=0.0. Useful when keyframing an object relative to this group. */
var()	bool				bDisableMovement;

/** If false, when this track is displayed on the Curve Editor in Matinee, do not show the Translation tracks. */
var()	bool				bShowTranslationOnCurveEd;

/** If false, when this track is displayed on the Curve Editor in Matinee, do not show the Rotation tracks. */
var()	bool				bShowRotationOnCurveEd;

/** If true, 3D representation of this track in the 3D viewport is disabled. */
var()	bool				bHide3DTrack;

enum EInterpTrackMoveFrame
{
	/** Track should be fixed relative to the world. */
	IMF_World,

	/** Track should move relative to the initial position of the actor when the interp sequence was started. */
	IMF_RelativeToInitial
};

/** Indicates what the movement track should be relative to. */
var()	editconst EInterpTrackMoveFrame	MoveFrame;


enum EInterpTrackMoveRotMode
{
	/** Should take orientation from the . */
	IMR_Keyframed,

	/** Point the X-Axis of the controlled Actor at the group specified by LookAtGroupName. */
	IMR_LookAtGroup

	/** Should look along the direction of the translation path, with Z always up. */
	// IMR_LookAlongPath // TODO!
};

var()	EInterpTrackMoveRotMode RotMode;

defaultproperties
{
	TrackInstClass=class'Engine.InterpTrackInstMove'

	bOnePerGroup=true

	TrackTitle="Movement"

	LinCurveTension=0.0
	AngCurveTension=0.0

	MoveFrame=IMF_World
	RotMode=IMR_Keyframed

	bShowTranslationOnCurveEd=true
	bShowRotationOnCurveEd=false
}
