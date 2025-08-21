/**
 * SceneCaptureComponent
 *
 * Base class for scene recording components
 *
 * Copyright 2006 Epic Games, Inc. All Rights Reserved.
 */
class SceneCaptureComponent extends ActorComponent
	native
	abstract
	hidecategories(Object);

/** toggle scene post-processing */
var(Capture) bool bEnablePostProcess;
/** toggle fog */
var(Capture) bool bEnableFog;
/** background color */
var(Capture) color ClearColor;

// draw modes - based on ESceneViewMode
enum ESceneCaptureViewMode
{
	// lit/shadowed scene
	SceneCapView_Lit,
	// no shadows or lights
	SceneCapView_Unlit,
	// lit/unshadowed scene
	SceneCapView_LitNoShadows,
	// wireframe
	SceneCapView_Wire
};
/** how to draw the scene */
var(Capture) ESceneCaptureViewMode ViewMode;
/** level-of-detail setting */
var(Capture) int SceneLOD;
/**
 * rate to capture the scene,
 * TimeBetweenCaptures = Max( 1/FrameRate, DeltaTime),
 * if the FrameRate is 0 then the scene is captured only once
 */
var(Capture) float FrameRate;
/** Chain of post process effects for this post process view */
var(Capture) PostProcessChain PostProcess;

// transients

/** ptr to the scene capture probe */
var private const transient native pointer CaptureInfo{FCaptureSceneInfo};
/** TRUE if the scene capture needs to be updated in the scene */
var private const transient native bool bNeedsSceneUpdate;

cpptext
{
protected:	

	/** 
	* Constructor 
	*/
	USceneCaptureComponent();
	
	// UActorComponent interface.

	/**
	* Adds a capture proxy for this component to the scene
	*/
	virtual void Attach();

	/**
	* Removes a capture proxy for thsi component from the scene
	*/
	virtual void Detach();

	/**
	* Tick the component to handle updates
	*/
	virtual void Tick(FLOAT DeltaTime);

	/**
	 * Sets the ParentToWorld transform the component is attached to.
	 * @param ParentToWorld - The ParentToWorld transform the component is attached to.
	 */
	virtual void SetParentToWorld(const FMatrix& ParentToWorld);

public:
	/**
	* Create a new probe with info needed to render the scene
	*/
	virtual class FSceneCaptureProbe* CreateSceneCaptureProbe() { return NULL; }

	/**
	* Map the various capture view settings to show flags.
	*/
	virtual EShowFlags GetSceneShowFlags();
}

defaultproperties
{
	ViewMode=SceneCapView_LitNoShadows
	ClearColor=(R=0,G=0,B=0,A=255)
	FrameRate=30
}
