using System;
using System.Xml;
using System.Xml.Serialization;


// Defines optional per-game settings for the cooker
// See ExampleGame\Config\CookerFrontend_game.xml for an .xml file to copy for your own game

namespace CookerTools
{
	public class PathInfo
	{
		/// <summary>
		/// Additional paths to copy to target
		/// </summary>
		[XmlAttribute]
		public string Path;

		/// <summary>
		/// Wildcard inside path to search for files
		/// </summary>
		[XmlAttribute]
		public string Wildcard;

		/// <summary>
		/// Recursive copy?
		/// </summary>
		[XmlAttribute]
		public bool bIsRecursive;

	};
	/// <summary>
	/// Summary description for CookerSettings.
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

		/// <summary>
		/// Needed for XML serialization. Does nothing
		/// </summary>
		public GameSettings()
		{
		}
	}
}
