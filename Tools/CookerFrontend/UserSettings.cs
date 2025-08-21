using System;
using System.Xml;
using System.Xml.Serialization;

namespace CookerFrontend
{
	/// <summary>
	/// Stores the last used settings for the application
	/// </summary>
	public class UserSettings
	{
		/// <summary>
		/// Stores the last used set of maps to cook
		/// </summary>
		[XmlAttribute]
		public string CookMaps;
		/// <summary>
		/// Stores the last used set of maps to run
		/// </summary>
		[XmlAttribute]
		public string RunMaps;
		/// <summary>
		/// Stores the last used xbox base directory to copy to
		/// </summary>
		[XmlAttribute]
		public string BaseDirectory;
		/// <summary>
		/// Stores the last used platform
		/// </summary>
		[XmlAttribute]
		public string Platform;
		/// <summary>
		/// Stores the last used game name
		/// </summary>
		[XmlAttribute]
		public string GameName;
		/// <summary>
		/// Stores the last used PC configuration
		/// </summary>
		[XmlAttribute]
		public string PCConfiguration;
		/// <summary>
		/// Stores the last used xbox configuration
		/// </summary>
		[XmlAttribute]
		public string ConsoleConfiguration;

		/// <summary>
		/// Whether to start from a chapterpoint
		/// </summary>		
		[XmlAttribute]
		public bool UseChapterPoint;

		/// <summary>
		/// ChapterPoint to start from
		/// </summary>		
		[XmlAttribute]
		public int ChapterPoint;

		/// <summary>
		/// Whether to cook final release scripts
		/// </summary>		
		[XmlAttribute]
		public bool FinalReleaseChecked;

		/// <summary>
		/// Whether to cook final release scripts
		/// </summary>		
		[XmlAttribute]
		public bool InisIntsOnlyChecked;

		/// <summary>
		/// Whether or not to auto run the console after cooking
		/// </summary>
		[XmlAttribute]
		public bool LaunchAfterCooking;

		/// <summary>
		/// Whether or not to use the cook maps string as the commandline
		/// </summary>
		[XmlAttribute]
		public bool RunWithCookedMap;

		/// <summary>
		/// Needed for XML serialization. Does nothing
		/// </summary>
		public UserSettings()
		{
		}
	}
}
