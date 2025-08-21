/**
 * Copyright © 1998-2007 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Holds the base settings for an online game search
 */
class OnlineGameSearch extends Settings
	native;

/** Max number of queries returned by the matchmaking service */
var int MaxSearchResults;

/** The query to use for finding matching servers */
var LocalizedStringSetting Query;

/** Whether the query is intended for LAN matches or not */
var databinding bool bIsLanQuery;

/** Whether to use arbitration or not */
var databinding bool bUsesArbitration;

/** Whether the search object in question is in progress or not */
var const bool bIsSearchInProgress;

/** Struct used to return matching servers */
struct native OnlineGameSearchResult
{
	/** The settings used by this particular server */
	var const OnlineGameSettings GameSettings;
	/**
	 * Platform/online provider specific data
	 * NOTE: It is imperative that the subsystem be called to clean this data
	 * up or the PlatformData will leak memory!
	 */
	var const native pointer PlatformData{void};

	structcpptext
	{
		/** Default constructor does nothing and is here for NoInit types */
		FOnlineGameSearchResult()
		{
		}

		/** Zeroing constructor */
		FOnlineGameSearchResult(EEventParm) :
			GameSettings(NULL),
			PlatformData(NULL)
		{
		}
	}
};

/** The class to create for each returned result from the search */
var class<OnlineGameSettings> GameSettingsClass;

/** The list of servers and their settings that match the search */
var const array<OnlineGameSearchResult> Results;

defaultproperties
{
	// Override this with your game specific class so that metadata can properly
	// expose the game information to the UI
	GameSettingsClass=class'Engine.OnlineGameSettings'
	MaxSearchResults=25
}
