/**
 * WorldInfo
 *
 * Actor containing all script accessible world properties.
 *
 * Copyright 2004-2006 Epic Games, Inc. All Rights Reserved.
 */
class WorldInfo extends ZoneInfo
	native
	config(game)
	hidecategories(Actor,Advanced,Display,Events,Object,Attachment)
	nativereplication
	notplaceable
	dependson(PostProcessEffect)
	dependson(MusicTrackDataStructures);

/** Default post process settings used by post processing volumes.									*/
var(WorldInfo)	config				PostProcessSettings		DefaultPostProcessSettings;

var(WorldInfo)						bool					bDisplayTonemapInfo;

/** Linked list of post processing volumes, sorted in descending order of priority.					*/
var	const noimport transient		PostProcessVolume		HighestPriorityPostProcessVolume;

/** Default reverb settings used by reverb volumes.									*/
var(WorldInfo)						ReverbSettings			DefaultReverbSettings;


var(WorldInfo)						Texture2D				LoadBackgroud;

var(WorldInfo)						Texture2D				MinimapTexture;
var(WorldInfo)						Texture2D				LargemapTexture;
var(WorldInfo)						Material				LargemapMaterial;
var									vector					MinimapOffset;
var									float					MinimapScale;
var(Worldinfo)						float					MinimapMeterPerTexel;
var(WorldInfo)						float					Radiosity_EmissiveScale;
var(WorldInfo)						float					Radiosity_ReflectivityScale;
var(WorldInfo)						float					Reference_dB;

struct native ColorCorrectionData
{
	var() string											SourcePath;
	var() int												Resolution;
	var const array<Color>									Data;
	var native const pointer								Resource{FColorCorrectionResource};

	structcpptext
	{
		FColorCorrectionResource* CreateResource();

		void UpdateResource();

		void ReleaseResource();

		void ImportData();

		friend FArchive& operator<<( FArchive& Ar, FColorCorrectionData& Data )
		{
			return Ar << Data.SourcePath << Data.Resolution << Data.Data;
		}
	}
};

var(ColorCorrection)				array<ColorCorrectionData> ColorCorrections;

/** Linked list of reverb volumes, sorted in descending order of priority.							*/
var	const noimport transient		ReverbVolume			HighestPriorityReverbVolume;

/** Level collection. ULevels are referenced by FName (Package name) to avoid serialized references. Also contains offsets in world units */
var(WorldInfo) const editconst editinline array<LevelStreaming> StreamingLevels;

/**
 * This is a bool on the level which is set when a light that needs to have lighting rebuilt
 * is moved.  This is then checked in CheckMap for errors to let you know that this level should
 * have lighting rebuilt.
 **/
var 					bool 					bMapNeedsLightingFullyRebuilt;

//<@ ava specific ; 2007. 1. 12 changmin
var						bool					bMapNeedsGeometryFullyRebuilt;
//>@ ava

/** Whether it was requested that the engine bring up a loading screen and block on async loading. */
var						bool					bRequestedBlockOnAsyncLoading;

var(Editor)				array<EdLayer>			Layers;					// The master list of layers defined by the level designers
var(Editor)				BookMark				BookMarks[10];			// Level bookmarks
var						float					TimeDilation;			// Normally 1 - scales real time passage.
var						float					TimeSeconds;			// Time in seconds since level began play, but IS paused when the game is paused.
var						float					RealTimeSeconds;		// Time in seconds since level began play, but is NOT paused when the game is paused.
var	transient const		float					DeltaSeconds;			// Frame delta time in seconds adjusted by e.g. time dilation.
var						float					PauseDelay;				// time at which to start pause
var						float					RealTimeToUnPause;		// If non-zero, when RealTimeSeconds reaches this, unpause the game.

var						PlayerReplicationInfo	Pauser;					// If paused, name of person pausing the game.
var						string					VisibleGroups;			// List of the group names which were checked when the level was last saved
var transient			string					SelectedGroups;			// A list of selected groups in the group browser (only used in editor)

var						bool					bBegunPlay;				// Whether gameplay has begun.
var						bool					bPlayersOnly;			// Only update players.
var						bool					bDropDetail;			// frame rate is below DesiredFrameRate, so drop high detail actors
var						bool					bAggressiveLOD;			// frame rate is well below DesiredFrameRate, so make LOD more aggressive
var						bool					bStartup;				// Starting gameplay.
var						bool					bPathsRebuilt;			// True if path network is valid
var						bool					bHasPathNodes;

var						Texture2D				DefaultTexture;
var						Texture2D				WireframeTexture;
var						Texture2D				WhiteSquareTexture;
var						Texture2D				LargeVertex;
var						Texture2D				BSPVertex;

/**
 * This is the array of string which will be called after a tick has occurred.  This allows
 * functions which GC and/or nuke objects to be executed from .uc land!
 **/
var 					array<string>			DeferredExecs;


var transient			GameReplicationInfo		GRI;
var enum ENetMode
{
	NM_Standalone,        // Standalone game.
	NM_DedicatedServer,   // Dedicated server, no local client.
	NM_ListenServer,      // Listen server.
	NM_Client             // Client only, no local server.
} NetMode;

var						string					ComputerName;			// Machine's name according to the OS.
var						string					EngineVersion;			// Engine version.
var						string					MinNetVersion;			// Min engine version that is net compatible.

var						GameInfo				Game;
var()					float					StallZ;					// vehicles stall if they reach this
var	transient			float					WorldGravityZ;			// current gravity actually being used
var	const globalconfig	float					DefaultGravityZ;		// default gravity (game specific) - set in defaultgame.ini
var()					float					GlobalGravityZ;			// optional level specific gravity override set by level designer
var globalconfig		float					RBPhysicsGravityScaling;	// used to scale gravity for rigid body physics

var transient const private	NavigationPoint		NavigationPointList;
var const private			Controller			ControllerList;
var const					Pawn				PawnList;
var transient const			CoverLink			CoverList;

var						float					MoveRepSize;

var						string					NextURL;
var						float					NextSwitchCountdown;

/** Maximum size of textures for packed light and shadow maps */
var()					int						PackedLightAndShadowMapTextureSize;

/** Default color scale for the level */
var()					vector					DefaultColorScale;

/** if true, do not grant player with default inventory (presumably, the LD's will be setting it manually) */
var()					bool					bNoDefaultInventoryForPlayer;

/**
 * This is the list of GameTypes which this map can support.  This is used for SeekFree loading
 * code so the map has a reference to the game type it will be played with so it can cook
 * all of the specific assets
 **/
var() editinline array<class<GameInfo> >		GameTypesSupportedOnThisMap;

/** array of levels that were loaded into this map via PrepareMapChange() / CommitMapChange() (to inform newly joining clients) */
var const transient array<name> PreparingLevelNames;
var const transient array<name> CommittedLevelNames;

/** Kismet controlled music info (to inform newly joining clients) */
var SeqAct_CrossFadeMusicTracks LastMusicAction;
var MusicTrackStruct LastMusicTrack;

/** If true, don't add "no paths from" warnings to map error list in editor.  Useful for maps that don't need AI support, but still have a few NavigationPoint actors in them. */
var() bool bNoPathWarnings;

/** title of the map displayed in the UI */
var() localized string Title;

var() string Author;

/** when this flag is set, more time is allocated to background loading */
var bool bHighPriorityLoading;

struct native AmbientCubeVolumeInfo
{
	var					int						NumAmbientCubeX, NumAmbientCubeY, NumAmbientCubeZ;
	var					vector					AmbientCubeMin, AmbientCubeMax;
	var					int						SHStartIndex;
	var					vector					CubeSize;
	var					int						Priority;
};

// legacy
var						int						NumAmbientCubeX, NumAmbientCubeY, NumAmbientCubeZ;
var						vector					AmbientCubeMin, AmbientCubeMax;

// still in use :)
var	native const		array<AmbientSH>		AmbientSHs;

var						array<AmbientCubeVolumeInfo> AmbientCubeVolumeInfos;

struct native LoadBackgroundInfo
{
	var string MapFileName;
	var string LoadBG;
	var float U,V,UL,VL;
	structdefaultproperties
	{
		U=0
		V=0
		UL=0
		VL=0
	}
};
var globalconfig array<LoadBackgroundInfo>	LoadBackgroundList;

/** game specific map information - access through GetMapInfo()/SetMapInfo() */
var() protected{protected} instanced MapInfo MyMapInfo;

// pvs info ; 2008. 2. 21 changmin
var(PVSInfo) const editconst editoronly int		VisdataSize;
var(PVSInfo) const editconst editoronly int		PortalClusters;
var(PVSInfo) const editconst editoronly int		AverageClustersVisible;
var(PVSInfo) const editconst editoronly float	ComputePvsTime;
var(PVSInfo) const editconst editoronly bool	UsePvs;
//

/** 넷게임(avaNetEntryGame)에 대한 클래스를 저장 */
var transient class<GameInfo>					NetEntryGameClass;

cpptext
{
	virtual void Serialize(FArchive& Ar);
	// UObject interface.

	/**
	 * Called when a property on this object has been modified externally
	 *
	 * @param PropertyThatChanged the property that was modified
	 */
	void PostEditChange(UProperty* PropertyThatChanged);

	void PostLoad();

	/**
	 * Called after GWorld has been set. Used to load, but not associate, all
	 * levels in the world in the Editor and at least create linkers in the game.
	 * Should only be called against GWorld::PersistentLevel's WorldInfo.
	 */
	void LoadSecondaryLevels();

	// AActor interface.
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );
	void CheckForErrors();

	// Level functions
	void SetZone( UBOOL bTest, UBOOL bForceRefresh );
	void SetVolumes();
	virtual void SetVolumes(const TArray<class AVolume*>& Volumes);
	APhysicsVolume* GetDefaultPhysicsVolume();
	APhysicsVolume* GetPhysicsVolume(const FVector& Loc, AActor *A, UBOOL bUseTouch);
	FLOAT GetRBGravityZ();

	/**
	 * Finds the post process settings to use for a given view location, taking into account the world's default
	 * settings and the post process volumes in the world.
	 * @param	ViewLocation			Current view location.
	 * @param	bUseVolumes				Whether to use the world's post process volumes
	 * @param	OutPostProcessSettings	Upon return, the post process settings for a camera at ViewLocation.
	 * @return	If the settings came from a post process volume, the post process volume is returned.
	 */
	APostProcessVolume* GetPostProcessSettings(const FVector& ViewLocation,UBOOL bUseVolumes,FPostProcessSettings& OutPostProcessSettings);

	/**
	 * Finds the reverb settings to use for a given view location, taking into account the world's default
	 * settings and the reverb volumes in the world.
	 *
	 * @param	ViewLocation			Current view location.
	 * @param	bUseVolumes				Whether to use the world's reverb volumes.
	 * @param	OutReverbSettings		[out] Upon return, the reverb settings for a camera at ViewLocation.
	 * @return							If the settings came from a reverb volume, the reverb volume is returned.
	 */
	AReverbVolume* GetReverbSettings(const FVector& ViewLocation, UBOOL bUseVolumes);

	const FAmbientCubeVolumeInfo* DetermineAmbientCubeVolume( const FVector& Location ) const;
	const FSHVectorRGB& GetAmbientCube( const FAmbientCubeVolumeInfo& Info, const FVector& Location ) const;
	void InterpolateAmbientCube( const FAmbientCubeVolumeInfo& Info, const FVector& Location, FAmbientSH& Result ) const;

	const FSHVectorRGB& GetAmbientCube( const FVector& Location ) const;
	void InterpolateAmbientCube( const FVector& Location, FAmbientSH& Result ) const;

	/**
	 * Sets bMapNeedsLightingFullyRebuild to the specified value.  Marks the worldinfo package dirty if the value changed.
	 *
	 * @param	bInMapNeedsLightingFullyRebuild			The new value.
	 */
	void SetMapNeedsLightingFullyRebuilt(UBOOL bInMapNeedsLightingFullyRebuild);
	
	/**
	 * Sets bMapNeedsGeometryFullyRebuilt to the specified value.  Marks the worldinfo package dirty if the value changed.
	 *
	 * @param	bInMapNeedsLightingFullyRebuilt			The new value.
	 */
	void SetMapNeedsGeometryFullyRebuilt(UBOOL bInMapNeedsGeometryFullyRebuilt);
}

//-----------------------------------------------------------------------------
// Functions.

final function bool IsServer()
{
	return (NetMode == NM_DedicatedServer || NetMode == NM_ListenServer);
}

/**
 * Returns the Z component of the current world gravity and initializes it to the default
 * gravity if called for the first time.
 *
 * @return Z component of current world gravity.
 */
native function float GetGravityZ();

/**
 * Grabs the default game sequence and returns it.
 *
 * @return		the default game sequence object
 */
native final function Sequence GetGameSequence();

native final function SetLevelRBGravity(vector NewGrav);

//
// Return the URL of this level on the local machine.
//
native simulated function string GetLocalURL();

//
// Demo build flag
//
native simulated static final function bool IsDemoBuild();  // True if this is a demo build.

/**
 * Returns whether we are running on a console platform or on the PC.
 *
 * @return TRUE if we're on a console, FALSE if we're running on a PC
 */
native simulated static final function bool IsConsoleBuild();

native simulated final function ForceGarbageCollection();

native simulated final function VerifyNavList();

//
// Return the URL of this level, which may possibly
// exist on a remote machine.
//
native simulated function string GetAddressURL();

simulated function class<GameInfo> GetGameClass()
{
	if(WorldInfo.Game != None)
		return WorldInfo.Game.Class;

	if (GRI != None && GRI.GameClass != None)
		return GRI.GameClass;

	return None;
}

//
// Jump the server to a new level.
//
event ServerTravel(string URL)
{
	if ( InStr(url,"%")>=0 )
	{
		`log("URL Contains illegal character '%'.");
		return;
	}

	if (InStr(url,":")>=0 || InStr(url,"/")>=0 || InStr(url,"\\")>=0)
	{
		`log("URL blocked");
		return;
	}

	// Check for an error in the server's connection
	if (Game != None && Game.bHasNetworkError)
	{
		`Log("Not traveling because of network error");
		return;
	}

	// if we're not already in a level change, start one now
	if (NextURL == "" && !IsInSeamlessTravel())
	{
		NextURL = URL;
		if (Game != None)
		{
			Game.ProcessServerTravel(URL);
		}
		else
		{
			NextSwitchCountdown = 0;
		}
	}
}

//
// ensure the DefaultPhysicsVolume class is loaded.
//
function ThisIsNeverExecuted(DefaultPhysicsVolume P)
{
	P = None;
}

/**
 * Reset actor to initial state - used when restarting level without reloading.
 */
function Reset()
{
	// perform garbage collection of objects (not done during gameplay)
	//ConsoleCommand("OBJ GARBAGE");
	Super.Reset();
}

function SetNetEntryGameClass( GameInfo GameInfoInst )
{
	NetEntryGameClass = GameInfoInst.class;
}

//-----------------------------------------------------------------------------
// Network replication.

replication
{
	if( bNetDirty && Role==ROLE_Authority )
		Pauser, TimeDilation, WorldGravityZ, bHighPriorityLoading;
}

/**
 * Steve, please fix me
 */
simulated function int EffectsDetailLevel()
{
 	return 0;
}

/** returns all NavigationPoints in the NavigationPointList that are of the specified class or a subclass
 * @note this function cannot be used during level startup because the NavigationPointList is created in C++ ANavigationPoint::PreBeginPlay()
 * @param BaseClass the base class of NavigationPoint to return
 * @param (out) N the returned NavigationPoint for each iteration
 */
native final iterator function AllNavigationPoints(class<NavigationPoint> BaseClass, out NavigationPoint N);

/** returns all NavigationPoints whose cylinder intersects with a sphere of the specified radius at the specified point
 * this function uses the navigation octree and is therefore fast for reasonably small radii
 * @note this function cannot be used during level startup because the navigation octree is populated in C++ ANavigationPoint::PreBeginPlay()
 * @param BaseClass the base class of NavigationPoint to return
 * @param (out) N the returned NavigationPoint for each iteration
 * @param Radius the radius to search in
 */
native final iterator function RadiusNavigationPoints(class<NavigationPoint> BaseClass, out NavigationPoint N, vector Point, float Radius);

/** returns a list of NavigationPoints and ReachSpecs that intersect with the given point and extent
 * @param Point point to check
 * @param Extent box extent to check
 * @param (optional, out) Navs list of NavigationPoints that intersect
 * @param (optional, out) Specs list of ReachSpecs that intersect
 */
native final noexport function NavigationPointCheck(vector Point, vector Extent, optional out array<NavigationPoint> Navs, optional out array<ReachSpec> Specs);

/** returns all Controllers in the ControllerList that are of the specified class or a subclass
 * @note this function is only useful on the server; if you need the local player on a client, use Actor::LocalPlayerControllers()
 * @param BaseClass the base class of Controller to return
 * @param (out) C the returned Controller for each iteration
 */
native final iterator function AllControllers(class<Controller> BaseClass, out Controller C);

/** returns all Pawns in the PawnList that are of the specified class or a subclass
 * @note: useful on both client and server; pawns are added/removed from pawnlist in PostBeginPlay (on clients, only relevant pawns will be available)
 * @param BaseClass the base class of Pawn to return
 * @param (out) P the returned Pawn for each iteration
 */
native final iterator function AllPawns(class<Pawn> BaseClass, out Pawn P);



/**
 * Called by GameInfo.StartMatch, used to notify native classes of match startup (such as Kismet).
 */
native final function NotifyMatchStarted(optional bool bShouldActivateLevelStartupEvents=true, optional bool bShouldActivateLevelBeginningEvents=true, optional bool bShouldActivateLevelLoadedEvents=false);


/** asynchronously loads the given levels in preparation for a streaming map transition.
 * This codepath is designed for worlds that heavily use level streaming and gametypes where the game state should
 * be preserved through a transition.
 * @param LevelNames the names of the level packages to load. LevelNames[0] will be the new persistent (primary) level
 */
native final function PrepareMapChange(const out array<name> LevelNames);
/** returns whether there's a map change currently in progress */
native final function bool IsPreparingMapChange();
/** if there is a map change being prepared, returns whether that change is ready to be committed
 * (if no change is pending, always returns false)
 */
native final function bool IsMapChangeReady();
/** actually performs the map transition prepared by PrepareMapChange()
 * it happens in the next tick to avoid GC issues
 * if a map change is being prepared but isn't ready yet, the transition code will block until it is
 * wait until IsMapChangeReady() returns true if this is undesired behavior
 */
native final function CommitMapChange(optional bool bShouldSkipLevelStartupEvent=FALSE,optional bool bShouldSkipLevelBeginningEvent=FALSE);


/** seamlessly travels to the given URL by first loading the entry level in the background,
 * switching to it, and then loading the specified level. Does not disrupt network communication or disconnet clients.
 * You may need to implement GameInfo::GetSeamlessTravelActorList(), PlayerController::GetSeamlessTravelActorList(),
 * GameInfo::PostSeamlessTravel(), and/or GameInfo::HandleSeamlessTravelPlayer() to handle preserving any information
 * that should be maintained (player teams, etc)
 * This codepath is designed for worlds that use little or no level streaming and gametypes where the game state
 * is reset/reloaded when transitioning. (like UT)
 * @param URL the URL to travel to; must be relative to the current URL (same server)
 */
native final function SeamlessTravel(string URL);

/** @return whether we're currently in a seamless transition */
native final function bool IsInSeamlessTravel();

/**
 * Enables/disables the standby cheat detection code
 *
 * @param bShouldEnable true to enable checking, false to disable it
 * @param StandbyTimeout the amount of time to allow before pausing the game
 */
native function SetStandbyCheatDetection(bool bShouldEnable,optional float StandbyTimeout = 1.0);

/** @return the current MapInfo that should be used. May return one of the inner StreamingLevels' MapInfo if a streaming level
 *	transition has occurred via PrepareMapChange()
 */
native final function MapInfo GetMapInfo();

/** sets the current MapInfo to the passed in one */
native final function SetMapInfo(MapInfo NewMapInfo);

defaultproperties
{
	Components.Remove(Sprite)

	RemoteRole=ROLE_SimulatedProxy
	bAlwaysRelevant=true
	TimeDilation=1.0
	bHiddenEd=True
	DefaultTexture=Texture2D'EngineResources.DefaultTexture'
	WhiteSquareTexture=WhiteSquareTexture
	LargeVertex=Texture2D'EngineResources.LargeVertex'
	BSPVertex=Texture2D'EngineResources.BSPVertex'
	bWorldGeometry=true
	VisibleGroups="None"
	MoveRepSize=+42.0
	bBlockActors=true
	StallZ=+1000000.0
	PackedLightAndShadowMapTextureSize=1024

	DefaultColorScale=(X=1.f,Y=1.f,Z=1.f)

	MinimapScale=1.0
	
	//<@ ava specific ; 2007. 1. 12 changmin
	bMapNeedsGeometryFullyRebuilt=TRUE;
	//>@ ava

	Radiosity_EmissiveScale = 1.0
	Radiosity_ReflectivityScale = 0.6666666	

	Reference_dB = 60.0
}
