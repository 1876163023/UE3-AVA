// ConsoleInterface.h

#pragma once

#define MANAGED_CPP_MAGIC __nogc

#include "..\..\..\Src\Engine\Inc\UnConsoleTools.h"

using namespace System;

namespace ConsoleInterface
{

public __gc class DLLInterface
{

public:
	/**
	 * Constructor
	 */
	DLLInterface();

	/**
	 * Destructor
	 */
	~DLLInterface();

	/**
	 * Enables a platform. Previously activated platform will be disabled.
	 *
	 * @param PlatformName Hardcoded platform name (PS3, Xenon, etc)
	 *
	 * @return true if successful
	 */
	bool ActivatePlatform(String* PlatformName);




	/////////////////////////////////////
	// MAIN INTERFACE
	/////////////////////////////////////
	/**
	 * @return the number of known xbox targets
	 */
	int GetNumTargets();

	/**
	 * Get the name of the specified target
	 * @param Index Index of the targt to query
	 * @return Name of the target, or NULL if the Index is out of bounds
	 */
	String* GetTargetName(int Index);
	
	/**
	 * @return true if this platform needs to have files copied from PC->target (required to implement)
	 */
	bool PlatformNeedsToCopyFiles();

	/**
	 * Open an internal connection to a target. This is used so that each operation is "atomic" in 
	 * that connection failures can quickly be 'remembered' for one "sync", etc operation, so
	 * that a connection is attempted for each file in the sync operation...
	 * For the next operation, the connection can try to be opened again.
	 *
	 * @param TargetName Name of target
	 *
	 * @return -1 for failure, or the index used in the other functions (MakeDirectory, etc)
	 */
	int ConnectToTarget(String* TargetName);

	/**
	 * Called after an atomic operation to close any open connections
	 */
	void DisconnectFromTarget(int TargetIndex);

	/**
	 * Creates a directory
	 *
	 * @param TargetIndex Index into the Targets array of which target to use
	 * @param SourceFilename Platform-independent directory name (ie UnrealEngine3\Binaries)
	 *
	 * @return true if successful
	 */
	bool MakeDirectory(int TargetIndex, String* DirectoryName);

	/**
	 * Determines if the given file needs to be copied
	 *
	 * @param TargetIndex Index into the Targets array of which target to use
	 * @param SourceFilename Path of the source file on PC
	 * @param DestFilename Platform-independent destination filename (ie, no xe:\\ for Xbox, etc)
	 *
	 * @return true if successful
	 */
	bool NeedsToSendFile(int TargetIndex, String* SourceFilename, String* DestFilename);

	/**
	 * Copies a single file from PC to target
	 *
	 * @param TargetIndex Index into the Targets array of which target to use
	 * @param SourceFilename Path of the source file on PC
	 * @param DestFilename Platform-independent destination filename (ie, no xe:\\ for Xbox, etc)
	 *
	 * @return true if successful
	 */
	bool SendFile(int TargetIndex, String* SourceFilename, String* DestFilename);

	/**
	 * Sets the name of the layout file for the DVD, so that GetDVDFileStartSector can be used
	 * 
	 * @param DVDLayoutFile Name of the layout file
	 *
	 * @return true if successful
	 */
	bool SetDVDLayoutFile(String* DVDLayoutFile);

	/**
	 * Gets the starting sector of a file on the DVD (or whatever optical medium)
	 *
	 * @param DVDFilename Path to the file on the DVD
	 * @param SectorHigh High 32 bits of the sector location
	 * @param SectorLow Low 32 bits of the sector location
	 * 
	 * @return true if the start sector was found for the file
	 */
	bool GetDVDFileStartSector(String* DVDFilename, unsigned int& SectorHigh, unsigned int& SectorLow);

	/**
	 * Reboots the target console. Must be implemented
	 *
	 * @param TargetIndex Index into the Targets array of which target to use
	 * 
	 * @return true if successful
	 */
	bool Reboot(int TargetIndex);

	/**
	 * Reboots the target console. Must be implemented
	 *
	 * @param TargetIndex Index into the Targets array of which target to use
	 * @param Configuration Build type to run (Debug, Release, RelaseLTCG, etc)
	 * @param BaseDirectory Location of the build on the console (can be empty for platforms that don't copy to the console)
	 * @param GameName Name of the game to run (Example, UT, etc)
	 * @param URL Optional URL to pass to the executable
	 * 
	 * @return true if successful
	 */
	bool RebootAndRun(int TargetIndex, String* Configuration, String* BaseDirectory, String* GameName, String* URL);

private:
	FConsoleSupport* CurrentConsoleSupport;
};


}
