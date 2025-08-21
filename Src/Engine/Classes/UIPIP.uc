class UIPIP extends UIObject
	native(UIPrivate);

var native private pointer ViewState{FSceneViewStateInterface};
var native private pointer Scene{FSceneInterface};
var() interp vector ColorScale;
var() bool bEnableColorScaling;
var() interp color FadeColor;
var() bool bEnableFading;
var() PostProcessSettings PostProcessSettings;
var() interp color BackgroundColor;
var() interp vector ViewLocation;
var() interp rotator ViewRotation;
var() bool bConstrainAspectRatio;
var() float AspectRatio;
var() interp float ViewFOV;
var() bool bUsingLookAt;
var() vector LookAt;
var() interp float Zoom;
var() interp float Yaw, Pitch;
var() float NearPlane;
var() interp float FadeAmount;
var() PostProcessChain PostProcess;
var() bool bTransparentBackground;
var() editconst const DirectionalLightComponent	DirectionalLightComponent;
var() interp rotator LightDirection;
var() editconst const SkyLightComponent	SkyLightComponent;
var transient private float LastRenderTime;
var transient array<ActorComponent>	Components;
var transient bool bInitialized;

cpptext
{
	virtual void FinishDestroy();

	virtual void Initialize( UUIScene* inOwnerScene, UUIObject* inOwner=NULL );

	virtual void Render_Widget( FCanvas* Canvas );

	virtual UBOOL ProcessInputKey( const struct FInputEventParameters& EventParms );

	/* === UObject interface === */
	/**
	* Called when a property value from a member struct or array has been changed in the editor.
	*/
	virtual void PostEditChange( FEditPropertyChain& PropertyThatChanged );

	/**
	* Called when a member property value has been changed in the editor.
	*/
	virtual void PostEditChange( UProperty* PropertyThatChanged );

	FSceneView* CalcSceneView( FCanvas* Canvas, FSceneViewFamily* ViewFamily );	

	virtual void NotifySceneActivated();

	virtual void NotifySceneDeactivated();

	virtual void PostUpdateMatinee();
}

native function DeleteScene();
native function SetupScene();

function InitComponents()
{	
	Components[0] = DirectionalLightComponent;
	Components[1] = SkyLightComponent;
}

event Refresh()
{
	if (!bInitialized)
	{
		InitComponents();
		bInitialized = true;
	}
	DeleteScene();	

	SetupScene();
}
	
defaultproperties
{
	Begin Object Class=DirectionalLightComponent Name=DirectionalLightComponent0
	    LightAffectsClassification=LAC_DYNAMIC_AND_STATIC_AFFECTING

	    CastShadows=TRUE
	    CastStaticShadows=TRUE
	    CastDynamicShadows=TRUE
	    bForceDynamicLight=FALSE
	    UseDirectLightMap=FALSE

	    LightingChannels=(BSP=TRUE,Static=TRUE,Dynamic=TRUE,bInitialized=TRUE)
	End Object
	DirectionalLightComponent=DirectionalLightComponent0

	Begin Object Class=SkyLightComponent Name=SkyLightComponent0
	End Object
	SkyLightComponent=SkylightComponent0

	ViewFOV = 90.0
	AspectRatio = 1.777
	NearPlane = 10
	ColorScale = (X=1,Y=1,Z=1)	
}