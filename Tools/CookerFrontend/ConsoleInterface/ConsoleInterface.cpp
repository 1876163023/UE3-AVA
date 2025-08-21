// This is the main DLL file.

#include "stdafx.h"
#include <windows.h>
#include "ConsoleInterface.h"

using namespace ConsoleInterface;
using namespace System::Runtime::InteropServices;

DLLInterface::DLLInterface()
{
	CurrentConsoleSupport = NULL;
}

HMODULE DLLHandle = NULL;

DLLInterface::~DLLInterface()
{
	if (DLLHandle)
	{
		CloseHandle(DLLHandle);
	}
}

/**
 * Enables a platform. Previously activated platform will be disabled.
 *
 * @param PlatformName Hardcoded platform name (PS3, Xenon, etc)
 *
 * @return true if successful
 */
bool DLLInterface::ActivatePlatform(String* PlatformName)
{
	// @todo: Search for all DLLs that have a matching PlatformName

	DLLHandle = NULL;
	
	// handle each platform for now
	if (String::Compare(PlatformName, S"Xenon", true) == 0)
	{
		DLLHandle = LoadLibraryA("Xenon\\XeTools.dll");
	}
	else if (String::Compare(PlatformName, S"PS3", true) == 0)
	{
		DLLHandle = LoadLibraryA("PS3\\PS3Tools.dll");
	}

	if (DLLHandle == NULL)
	{
		return false;
	}

	// get the function pointer to the accessor
	FuncGetConsoleSupport GetSupportFunc = (FuncGetConsoleSupport)GetProcAddress(DLLHandle, "GetConsoleSupport");

	// get console support
	CurrentConsoleSupport = GetSupportFunc();

	return true;
}


/**
 * @return the number of known xbox targets
 */
int DLLInterface::GetNumTargets()
{
	return CurrentConsoleSupport->GetNumTargets();
}

/**
 * Get the name of the specified target
 * @param Index Index of the targt to query
 * @return Name of the target, or NULL if the Index is out of bounds
 */
String* DLLInterface::GetTargetName(int Index)
{
	return CurrentConsoleSupport->GetTargetName(Index);
}

/**
 * @return true if this platform needs to have files copied from PC->target (required to implement)
 */
bool DLLInterface::PlatformNeedsToCopyFiles()
{
	return CurrentConsoleSupport->PlatformNeedsToCopyFiles();
}

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
int DLLInterface::ConnectToTarget(String* TargetName)
{
	// copy over the string to non-gc memory
	IntPtr BStr = Marshal::StringToBSTR(TargetName);

	int Ret = CurrentConsoleSupport->ConnectToTarget((wchar_t*)BStr.ToPointer());

	// free temp memory
	Marshal::FreeBSTR(BStr);

	return Ret;
}

/**
 * Called after an atomic operation to close any open connections
 */
void DLLInterface::DisconnectFromTarget(int TargetIndex)
{
	CurrentConsoleSupport->DisconnectFromTarget(TargetIndex);
}

/**
 * Creates a directory
 *
 * @param TargetIndex Index into the Targets array of which target to use
 * @param SourceFilename Platform-independent directory name (ie UnrealEngine3\Binaries)
 *
 * @return true if successful
 */
bool DLLInterface::MakeDirectory(int TargetIndex, String* DirectoryName)
{
	// copy over the string to non-gc memory
	IntPtr BStr = Marshal::StringToBSTR(DirectoryName);

	// do the command
	bool Ret = CurrentConsoleSupport->MakeDirectory(TargetIndex, (wchar_t*)BStr.ToPointer());

	// free temp memory
	Marshal::FreeBSTR(BStr);

	// return the result
	return Ret;
}

/**
 * Determines if the given file needs to be copied
 *
 * @param TargetIndex Index into the Targets array of which target to use
 * @param SourceFilename Path of the source file on PC
 * @param DestFilename Platform-independent destination filename (ie, no xe:\\ for Xbox, etc)
 *
 * @return true if successful
 */
bool DLLInterface::NeedsToSendFile(int TargetIndex, String* SourceFilename, String* DestFilename)
{
	// copy over the strings to non-gc memory
	IntPtr BStr1 = Marshal::StringToBSTR(SourceFilename);
	IntPtr BStr2 = Marshal::StringToBSTR(DestFilename);

	bool Ret = CurrentConsoleSupport->NeedsToCopyFile(TargetIndex, (wchar_t*)BStr1.ToPointer(), (wchar_t*)BStr2.ToPointer());
	
	// free temp memory
	Marshal::FreeBSTR(BStr2);
	Marshal::FreeBSTR(BStr1);

	// return the result
	return Ret;
}

/**
 * Copies a single file from PC to target
 *
 * @param TargetIndex Index into the Targets array of which target to use
 * @param SourceFilename Path of the source file on PC
 * @param DestFilename Platform-independent destination filename (ie, no xe:\\ for Xbox, etc)
 *
 * @return true if successful
 */
bool DLLInterface::SendFile(int TargetIndex, String* SourceFilename, String* DestFilename)
{
	// copy over the strings to non-gc memory
	IntPtr BStr1 = Marshal::StringToBSTR(SourceFilename);
	IntPtr BStr2 = Marshal::StringToBSTR(DestFilename);

	bool Ret = CurrentConsoleSupport->CopyFile(TargetIndex, (wchar_t*)BStr1.ToPointer(), (wchar_t*)BStr2.ToPointer());
	
	// free temp memory
	Marshal::FreeBSTR(BStr2);
	Marshal::FreeBSTR(BStr1);

	// return the result
	return Ret;
}

/**
 * Sets the name of the layout file for the DVD, so that GetDVDFileStartSector can be used
 * 
 * @param DVDLayoutFile Name of the layout file
 *
 * @return true if successful
 */
bool DLLInterface::SetDVDLayoutFile(String* DVDLayoutFile)
{
	// copy over the strings to non-gc memory
	IntPtr BStr1 = Marshal::StringToBSTR(DVDLayoutFile);

	bool Ret = CurrentConsoleSupport->SetDVDLayoutFile((wchar_t*)BStr1.ToPointer());

	// free temp memory
	Marshal::FreeBSTR(BStr1);

	// return the result
	return Ret;
}

/**
 * Gets the starting sector of a file on the DVD (or whatever optical medium)
 *
 * @param DVDFilename Path to the file on the DVD
 * @param SectorHigh High 32 bits of the sector location
 * @param SectorLow Low 32 bits of the sector location
 * 
 * @return true if the start sector was found for the file
 */
bool DLLInterface::GetDVDFileStartSector(String* DVDFilename, unsigned int& SectorHigh, unsigned int& SectorLow)
{
	// copy over the strings to non-gc memory
	IntPtr BStr1 = Marshal::StringToBSTR(DVDFilename);

	bool Ret = CurrentConsoleSupport->GetDVDFileStartSector((wchar_t*)BStr1.ToPointer(), SectorHigh, SectorLow);

	// free temp memory
	Marshal::FreeBSTR(BStr1);

	// return the result
	return Ret;
}

/**
 * Reboots the target console. Must be implemented
 *
 * @param TargetIndex Index into the Targets array of which target to use
 * 
 * @return true if successful
 */
bool DLLInterface::Reboot(int TargetIndex)
{
	// reboot the console
	return CurrentConsoleSupport->Reboot(TargetIndex);
}

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
bool DLLInterface::RebootAndRun(int TargetIndex, String* Configuration, String* BaseDirectory, String* GameName, String* URL)
{
	// copy over the strings to non-gc memory
	IntPtr BStr1 = Marshal::StringToBSTR(Configuration);
	IntPtr BStr2 = Marshal::StringToBSTR(BaseDirectory);
	IntPtr BStr3 = Marshal::StringToBSTR(GameName);
	IntPtr BStr4 = Marshal::StringToBSTR(URL);

	// run the game
	bool Ret = CurrentConsoleSupport->RebootAndRun(TargetIndex, (wchar_t*)BStr1.ToPointer(), (wchar_t*)BStr2.ToPointer(), (wchar_t*)BStr3.ToPointer(), (wchar_t*)BStr4.ToPointer());
	
	// free temp memory
	Marshal::FreeBSTR(BStr4);
	Marshal::FreeBSTR(BStr3);
	Marshal::FreeBSTR(BStr2);
	Marshal::FreeBSTR(BStr1);

	// return the result
	return Ret;
}
