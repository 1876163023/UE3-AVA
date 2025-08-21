//=============================================================================
// LocalPlayer
// Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class LocalPlayer extends Player
	within Engine
	config(Engine)
	native
	transient;

/** The controller ID which this player accepts input from. */
var int ControllerId;

/** The master viewport containing this player's view. */
var GameViewportClient ViewportClient;

/** The coordinates for the upper left corner of the master viewport subregion allocated to this player. 0-1 */
var vector2d Origin;

/** The size of the master viewport subregion allocated to this player. 0-1 */
var vector2d Size;

/** Chain of post process effects for this player view */
var PostProcessChain PlayerPostProcess;

var private native const pointer ViewState{FSceneViewStateInterface};

struct SynchronizedActorVisibilityHistory
{
	var pointer State;
	var pointer CriticalSection;
};

var private native transient const SynchronizedActorVisibilityHistory ActorVisibilityHistory;

struct native CurrentPostProcessVolumeInfo
{
	/** Last pp settings used when blending to the next set of volume values. */
	var PostProcessSettings	LastSettings;
	/** The last post process volume that was applied to the scene */
	var PostProcessVolume LastVolumeUsed;
	/** Time when a new post process volume was set */
	var float BlendStartTime;
	/** Time when the settings blend was last updated. */
	var float LastBlendTime;
};
/** current state of post process info set in the scene */
var const noimport transient CurrentPostProcessVolumeInfo CurrentPPInfo;

/** Whether to override the post process settings or not */
var bool bOverridePostProcessSettings;
/** The post process settings to override to */
var PostProcessSettings PostProcessSettingsOverride;
/** The start time of the post process override blend */
var float PPSettingsOverrideStartBlend;

/** set when we've sent a split join request */
var const editconst transient bool bSentSplitJoin;

//@AVA :)
var array<Object> WorkingSet;

struct native	PlayerWorkSet
{
	var array<Object>	objectList;
};

var PlayerWorkSet	PlayerSet[16];
var int				PlayerSetCnt;


cpptext
{
	/** Is object propagation currently overriding our view? */
	static UBOOL bOverrideView;
	static FVector OverrideLocation;
	static FRotator OverrideRotation;
	static FLOAT OverrideFOV;

	// Constructor.
	ULocalPlayer();

	/**
	 * Updates the post-process settings for the player's view.
	 * @param ViewLocation - The player's current view location.
	 */
	void UpdatePostProcessSettings(const FVector& ViewLocation);

	/**
	 * Calculate the view settings for drawing from this view actor
	 *
	 * @param	View - output view struct
	 * @param	ViewLocation - output actor location
	 * @param	ViewRotation - output actor rotation
	 * @param	Viewport - current client viewport
	 */
	FSceneView* CalcSceneView( FSceneViewFamily* ViewFamily, FVector& ViewLocation, FRotator& ViewRotation, FViewport* Viewport );

	// UObject interface.
	virtual void FinishDestroy();

	// FExec interface.
	virtual UBOOL Exec(const TCHAR* Cmd,FOutputDevice& Ar);

	void ExecMacro( const TCHAR* Filename, FOutputDevice& Ar );

}

/**
 * Creates an actor for this player.
 * @param URL - The URL the player joined with.
 * @param OutError - If an error occurred, returns the error description.
 * @return False if an error occurred, true if the play actor was successfully spawned.
 */
native final function bool SpawnPlayActor(string URL,out string OutError);

/** sends a splitscreen join command to the server to allow a splitscreen player to connect to the game
 * the client must already be connected to a server for this function to work
 * @note this happens automatically for all viewports that exist during the initial server connect
 * 	so it's only necessary to manually call this for viewports created after that
 * if the join fails (because the server was full, for example) all viewports on this client will be disconnected
 */
native final function SendSplitJoin();

/**
 * Tests the visibility state of an actor in the most recent frame of this player's view to complete rendering.
 * @param TestActor - The actor to check visibility for.
 * @return True if the actor was visible in the frame.
 */
native final function bool GetActorVisibility(Actor TestActor);

/**
 * Overrides the current post process settings.
 */
simulated function OverridePostProcessSettings( PostProcessSettings OverrideSettings, float StartBlendTime )
{
	PostProcessSettingsOverride = OverrideSettings;
	bOverridePostProcessSettings = true;
	PPSettingsOverrideStartBlend = StartBlendTime;
}

/**
 * Update the override post process settings
 */
simulated function UpdateOverridePostProcessSettings( PostProcessSettings OverrideSettings )
{
	PostProcessSettingsOverride = OverrideSettings;
}

/**
 * Clear the overriding of the post process settings.
 */
simulated function ClearPostProcessSettingsOverride()
{
	bOverridePostProcessSettings = false;
}

simulated function AddPlayerWorkingSet( array<Object> InWorkingSet )
{
	local int		i,index;
	local object	src;
	if ( ++PlayerSetCnt > 16 )
	{
		PlayerSetCnt = 0;
	}
	PlayerSet[PlayerSetCnt].objectList.length = 0;
	
	for ( i = 0 ; i < InWorkingSet.length ; ++ i )
	{
		src = InWorkingSet[i];
		if ( src != None )
		{
			index = PlayerSet[PlayerSetCnt].objectList.length;
			PlayerSet[PlayerSetCnt].objectList.length = index + 1;
			PlayerSet[PlayerSetCnt].objectList[index] = src;
		}
	}
}

/**
 * Changes the ControllerId for this player; if the specified ControllerId is already taken by another player, changes the ControllerId
 * for the other player to the ControllerId currently in use by this player.
 *
 * @param	NewControllerId		the ControllerId to assign to this player.
 */
final function SetControllerId( int NewControllerId )
{
	local LocalPlayer OtherPlayer;
	local int CurrentControllerId;

	if ( ControllerId != NewControllerId )
	{
		`log(Name @ "changing ControllerId from" @ ControllerId @ "to" @ NewControllerId,,'PlayerManagement');

		// first, unregister the player's data stores if we already have a PlayerController.
		if ( Actor != None )
		{
			Actor.UnregisterPlayerDataStores();
		}

		CurrentControllerId = ControllerId;

		// set this player's ControllerId to -1 so that if we need to swap controllerIds with another player we don't
		// re-enter the function for this player.
		ControllerId = -1;

		// see if another player is already using this ControllerId; if so, swap controllerIds with them
		OtherPlayer = ViewportClient.FindPlayerByControllerId(NewControllerId);
		if ( OtherPlayer != None )
		{
			OtherPlayer.SetControllerId(CurrentControllerId);
		}

		ControllerId = NewControllerId;
		if ( Actor != None )
		{
			Actor.RegisterPlayerDataStores();
		}
	}
}

defaultproperties
{
	bOverridePostProcessSettings=false
}
