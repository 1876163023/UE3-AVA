using System;
using System.Collections;
using System.IO;

using cs_IniHandlerDevelop;

namespace CommandletUtils
{
	/// <summary>
	/// Helper class to interface with Unreal files
	/// </summary>
	public class UnrealTools
	{
		// The map extension for the given GameName
		public string MapExtension;

		// Ini file utility for getting map extension and package paths
		private IniStructure IniFile;

		public UnrealTools()
		{
		}

		/// <summary>
		/// Tell the unreal tools that the game name has changed, update accordingly
		/// </summary>
		/// <returns></returns>
		public void SetGameName(string GameName)
		{
			// Make a path to the ini file
			string IniPath = "..\\" + GameName + "Game\\Config\\" + GameName + "Engine.ini";

			// Open the .ini file
			IniFile = IniStructure.ReadIni(IniPath);

			// If the ini file was found, get map extension from it
			if (IniFile != null)
			{
				// get the map extension
				MapExtension = IniFile.GetValue("URL", "MapExt");
			}
			// otherwise, we don't have a good map extension
			else
			{
				MapExtension = null;
			}
		}

		/// <summary>
		/// Generate the executable name for the commandlet
		/// </summary>
		/// <returns></returns>
		public string GetExecutableName(string GameName, bool bRunInSeparateWindow, bool bUseDebugExecutable)
		{
			string Executable = "";

			// if the debug check is selected, add Debug- to the executable name
			if (bUseDebugExecutable)
			{
				Executable = "Debug-";
			}
			// if we are redirecting input, we need to run the .com
			if (!bRunInSeparateWindow)
			{
				Executable += GameName + "Game.com";
			}
			// otherwise, run the .exe to pop up the window
			else
			{
				Executable += GameName+ "Game.exe";
			}
			return Executable;
		}



		/// <summary>
		/// Get a list of possible maps for this game (to be baked)
		/// </summary>
		public void FindPackages(ArrayList PackageList, bool bFindMaps, bool bFindPackages)
		{
			// @todo: Set Busy cursor

			// look for all the path keys in the ini file (the tag is for multiple Path lines)
			int PathTag = 0;
			while (true)
			{
				// get a path entry
				string PathValue = IniFile.GetValue("Core.System", "Paths", PathTag++);
				// if the path entry wasn't found, we are done looking
				if (PathValue == null)
				{
					break;
				}

				// find packages in this directory and its subdirectories
				ProcessDirectory(PathValue, PackageList, bFindMaps, bFindPackages);
			}
		}

		/// <summary>
		/// Recursive function to look for maps in the directory and its subdirectories
		/// </summary>
		/// <param name="DirectoryName"></param>
		private void ProcessDirectory(string DirectoryName, ArrayList PackageList, bool bFindMaps, bool bFindPackages)
		{
			// get all the maps that we can find
			DirectoryInfo Dir = new DirectoryInfo(DirectoryName);

			// we can't find maps without a valid map extension
			if (bFindMaps && MapExtension != null)
			{
				// get a list of maps in the dir
				FileInfo[] Files = Dir.GetFiles("*." + MapExtension);
				foreach (FileInfo File in Files)
				{
					// add the filename to the list of maps
					PackageList.Add(File.Name);
				}
			}

			if (bFindPackages)
			{
				// get a list of maps in the dir
				FileInfo[] Files = Dir.GetFiles("*.upk");
				foreach (FileInfo File in Files)
				{
					// add the filename to the list of package
					PackageList.Add(File.Name);
				}
			}

			// get the sub directories of this directory
			DirectoryInfo[] SubDirs = Dir.GetDirectories();

			// recurse into them looking for more files
			for (int Index = 0; Index < SubDirs.Length; Index++)
			{
				ProcessDirectory(SubDirs[Index].FullName, PackageList, bFindMaps, bFindPackages);
			}
		}

	}
}
