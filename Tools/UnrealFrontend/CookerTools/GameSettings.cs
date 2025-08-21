using System;
using System.Xml;
using System.Xml.Serialization;


// Defines optional per-game settings for the cooker
// See ExampleGame\Config\CookerFrontend_game.xml for an .xml file to copy for your own game

namespace CookerTools
{
	public class FileFilter
	{
		[XmlAttribute]
		public string Name = "";
	};

	public class PathInfo
	{
		/// <summary>
		/// Additional paths to copy to target
		/// </summary>
		[XmlAttribute]
		public string Path = "";

		/// <summary>
		/// Wildcard inside path to search for files
		/// </summary>
		[XmlAttribute]
		public string Wildcard = "";

		/// <summary>
		/// Optional destination directory
		/// </summary>
		[XmlAttribute]
		public string DestPath = null;

		/// <summary>
		/// Optional filter to apply to files (array of filenames) to not copy them
		/// </summary>
		[XmlArray("FileFilters")]
		public FileFilter[] FileFilters = new FileFilter[0];

		/// <summary>
		/// Recursive copy?
		/// </summary>
		[XmlAttribute]
		public bool bIsRecursive;

		/// <summary>
		/// Copy to target? (Will always copy when syncing to a UNC path)
		/// If false, the file will not be put in the TOC
		/// </summary>
		[XmlAttribute]
		public bool bIsForTarget = true;

		/// <summary>
		/// Process only when running CookerSync in demo mode
		/// If false, the file will not be put in the TOC
		/// </summary>
		[XmlAttribute]
		public bool bIsForDemo = true;

		/// <summary>
		/// Only create the directory in the destination, don't attempt to copy any files
		/// If true, all attribs are ignored except for DestPath
		/// </summary>
		[XmlAttribute]
		public bool bCreateDestOnly = false;
	};

	/// <summary>
    /// Summary description for per-platform CookerSettings.
    /// </summary>
    public class SharedSettings
    {
        /// <summary>
        /// This is the set of supported game names
        /// </summary>
        [XmlArray("SyncPaths")]
        public PathInfo[] SyncPaths = new PathInfo[0];

        /// <summary>
        /// Needed for XML serialization. Does nothing
        /// </summary>
        public SharedSettings()
        {
        }
    }
    
 
    /// <summary>
	/// Summary description for per-game CookerSettings.
	/// </summary>
	public class GameSettings
	{
		/// <summary>
		/// This is the set of supported game names
		/// </summary>
		[XmlArray("SyncPaths")]
		public PathInfo[] SyncPaths = new PathInfo[0];

		/// <summary>
		/// This is the set of supported game names
		/// </summary>
		[XmlAttribute]
		public string XGDFileRelativePath;

		[XmlAttribute]
		public string PS3TitleID = "";

		/// <summary>
		/// Needed for XML serialization. Does nothing
		/// </summary>
		public GameSettings()
		{
		}
	}

    /// <summary>
	/// Summary description for per-platform CookerSettings.
	/// </summary>
	public class PlatformSettings
	{
		/// <summary>
		/// This is the set of supported game names
		/// </summary>
		[XmlArray("SyncPaths")]
		public PathInfo[] SyncPaths = new PathInfo[0];

		/// <summary>
		/// Needed for XML serialization. Does nothing
		/// </summary>
		public PlatformSettings()
		{
		}
	}
}
