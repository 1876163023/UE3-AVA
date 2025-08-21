/*=============================================================================
	UnSceneCapture.h: render scenes to texture
	Copyright 2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// forward decls
class UTextureRenderTarget;
class UTextureRenderTarget2D;
class FSceneRenderer;

/** 
 * Probes added to the scene for rendering captures to a target texture
 * For use on the rendering thread
 */
class FSceneCaptureProbe
{
public:
	
	/** 
	* Constructor (default) 
	*/
	FSceneCaptureProbe(
		const AActor* InViewActor,
		UTextureRenderTarget* InTextureTarget,
		const EShowFlags& InShowFlags,
		const FLinearColor& InBackgroundColor,
		const FLOAT InFrameRate,
		const UPostProcessChain* InPostProcess
		)
		:	ViewActor(InViewActor)
		,	ShowFlags(InShowFlags)
		,	TextureTarget(InTextureTarget)
		,	BackgroundColor(InBackgroundColor)
		,	PostProcess(InPostProcess)
		,	TimeBetweenCaptures(InFrameRate > 0 ? 1/InFrameRate : 0)
		,	LastCaptureTime(0)
	{
	}

	/** 
	* Destructor 
	*/
	virtual ~FSceneCaptureProbe()
	{
	}

	/**
	* Called by the rendering thread to render the scene for the capture
	* @param	MainSceneRenderer - parent scene renderer with info needed 
	*			by some of the capture types.
	*/
	virtual void CaptureScene( FSceneRenderer* MainSceneRenderer ) = 0;

	/** 
	* Determine if a capture is needed based on TimeBetweenCaptures and LastCaptureTime
	* @param CurrentTime - seconds since start of play in the world
	* @return TRUE if the capture needs to be updated
	*/
	UBOOL UpdateRequired(FLOAT CurrentTime) const;
	
	/**
	* Updates LastCaptureTime with the current time
	* @param CurrentTime - seconds since start of play in the world
	*/
	void UpdateLastCaptureTime(FLOAT CurrentTime);

protected:
	/** The actor which is being viewed from. */
	const AActor* ViewActor;
	/** show flags needed for a scene capture */
	EShowFlags ShowFlags;
	/** render target for scene capture */
	UTextureRenderTarget* TextureTarget;
	/** background scene color */
	FLinearColor BackgroundColor;
	/** Post process chain to be used by this capture */
	const UPostProcessChain* PostProcess;

private:

	/** time in seconds between each capture. if == 0 then scene is captured only once */
	FLOAT TimeBetweenCaptures;
	/** time in seconds when last captured */
	FLOAT LastCaptureTime;
};

/** 
 * Renders a scene to a 2D texture target
 * These can be added to a scene without a corresponding 
 * USceneCaptureComponent since they are not dependent. 
 */
class FSceneCaptureProbe2D : public FSceneCaptureProbe
{
public:

	/** 
	* Constructor (init) 
	*/
	FSceneCaptureProbe2D(
		const AActor* InViewActor,
		UTextureRenderTarget* InTextureTarget,
		const EShowFlags& InShowFlags,
		const FLinearColor& InBackgroundColor,
		const FLOAT InFrameRate,
		const UPostProcessChain* InPostProcess,
		const FMatrix& InViewMatrix,
		const FMatrix& InProjMatrix
		)
		:	FSceneCaptureProbe(InViewActor,InTextureTarget,InShowFlags,InBackgroundColor,InFrameRate,InPostProcess)
		,	ViewMatrix(InViewMatrix)
		,	ProjMatrix(InProjMatrix)
	{
	}

	/**
	* Called by the rendering thread to render the scene for the capture
	* @param	MainSceneRenderer - parent scene renderer with info needed 
	*			by some of the capture types.
	*/
	virtual void CaptureScene( FSceneRenderer* MainSceneRenderer );

private:
	/** view matrix for capture render */
	FMatrix ViewMatrix;
	/** projection matrix for capture render */
	FMatrix ProjMatrix;
};

/** 
* Renders a scene to a cube texture target
* These can be added to a scene without a corresponding 
* USceneCaptureComponent since they are not dependent. 
*/
class FSceneCaptureProbeCube : public FSceneCaptureProbe
{
public:

	/** 
	* Constructor (init) 
	*/
	FSceneCaptureProbeCube(
		const AActor* InViewActor,
		UTextureRenderTarget* InTextureTarget,
		const EShowFlags& InShowFlags,
		const FLinearColor& InBackgroundColor,
		const FLOAT InFrameRate,
		const UPostProcessChain* InPostProcess,
		const FVector& InLocation,
		FLOAT InNearPlane,
		FLOAT InFarPlane
		)
		:	FSceneCaptureProbe(InViewActor,InTextureTarget,InShowFlags,InBackgroundColor,InFrameRate,InPostProcess)
		,	WorldLocation(InLocation)
		,	NearPlane(InNearPlane)
		,	FarPlane(InFarPlane)
	{
	}

	/**
	* Called by the rendering thread to render the scene for the capture
	* @param	MainSceneRenderer - parent scene renderer with info needed 
	*			by some of the capture types.
	*/
	virtual void CaptureScene( FSceneRenderer* MainSceneRenderer );	

private:

	/**
	* Generates a view matrix for a cube face direction 
	* @param	Face - enum for the cube face to use as the facing direction
	* @return	view matrix for the cube face direction
	*/
	FMatrix CalcCubeFaceViewMatrix( ECubeFace Face );

	/** world position of the cube capture */
	FVector WorldLocation;
	/** for proj matrix calc */
	FLOAT NearPlane;
	/** far plane cull distance used for calculating proj matrix and thus the view frustum */
	FLOAT FarPlane;
};

/** 
* Renders a scene as a reflection to a 2d texture target
* These can be added to a scene without a corresponding 
* USceneCaptureComponent since they are not dependent. 
*/
class FSceneCaptureProbeReflect : public FSceneCaptureProbe
{
public:
	/** 
	* Constructor (init) 
	*/
	FSceneCaptureProbeReflect(
		const AActor* InViewActor,
		UTextureRenderTarget* InTextureTarget,
		const EShowFlags& InShowFlags,
		const FLinearColor& InBackgroundColor,
		const FLOAT InFrameRate,
		const UPostProcessChain* InPostProcess,
		const FPlane& InMirrorPlane
		)
		:	FSceneCaptureProbe(InViewActor,InTextureTarget,InShowFlags,InBackgroundColor,InFrameRate,InPostProcess)
		,	MirrorPlane(InMirrorPlane)
	{
	}

	/**
	* Called by the rendering thread to render the scene for the capture
	* @param	MainSceneRenderer - parent scene renderer with info needed 
	*			by some of the capture types.
	*/
	virtual void CaptureScene( FSceneRenderer* MainSceneRenderer );	

private:
	/** plane to reflect against */
	FPlane MirrorPlane;
};

/** 
* Renders a scene as if viewed through a sister portal to a 2d texture target
* These can be added to a scene without a corresponding 
* USceneCaptureComponent since they are not dependent. 
*/
class FSceneCaptureProbePortal : public FSceneCaptureProbe
{
public:

	/** 
	* Constructor (init) 
	*/
	FSceneCaptureProbePortal(
		const AActor* InViewActor,
		UTextureRenderTarget* InTextureTarget,
		const EShowFlags& InShowFlags,
		const FLinearColor& InBackgroundColor,
		const FLOAT InFrameRate,
		const UPostProcessChain* InPostProcess,
		const FMatrix& InSrcWorldToLocalM,
		const FMatrix& InDestLocalToWorldM,
		const FMatrix& InSrcToDestChangeBasisM,
        const AActor* InDestViewActor
		)
		:	FSceneCaptureProbe(InViewActor,InTextureTarget,InShowFlags,InBackgroundColor,InFrameRate,InPostProcess)
		,	SrcWorldToLocalM(InSrcWorldToLocalM)
		,	DestLocalToWorldM(InDestLocalToWorldM)
		,	SrcToDestChangeBasisM(InSrcToDestChangeBasisM)
		,	DestViewActor(InDestViewActor ? InDestViewActor : InViewActor)
	{
	}

	/**
	* Called by the rendering thread to render the scene for the capture
	* @param	MainSceneRenderer - parent scene renderer with info needed 
	*			by some of the capture types.
	*/
	virtual void CaptureScene( FSceneRenderer* MainSceneRenderer );	

private:
	/** Inverse world transform for the source view location/orientation */
	FMatrix SrcWorldToLocalM;
	/** World transform for the the destination view location/orientation */
	FMatrix DestLocalToWorldM;
	/** Transform for source to destination view */
	FMatrix SrcToDestChangeBasisM;	
	/** the destination actor for this portal */
	const AActor* DestViewActor;
};


