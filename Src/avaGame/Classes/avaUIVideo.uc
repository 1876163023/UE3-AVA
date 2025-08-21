class avaUIVideo extends UIObject native(Video);

var private native const pointer ViewState{FSceneViewStateInterface};
var private native transient const SynchronizedActorVisibilityHistory ActorVisibilityHistory;
var const noimport transient CurrentPostProcessVolumeInfo CurrentPPInfo;
var private transient bool bActive;

var private transient float ViewX, ViewY, ViewXL, ViewYL, Alpha, FOV;
var private transient vector ViewLocation;
var private transient rotator ViewRotation;

cpptext
{		
	virtual void FinishDestroy();

	virtual void Render_Widget( FCanvas* Canvas );	

	virtual void PreRenderScene( FCanvas* Canvas, FSceneView* View ) {}
	virtual void PostRenderScene( FCanvas* Canvas ) {}
}