using System;
using System.Collections;
using System.Drawing;
using System.Windows.Forms;
using System.IO;
using CookerTools;

namespace CookerSync
{
	class ConsoleLogger : IOutputHandler
	{
		// IOutputHandler interface
		void CookerTools.IOutputHandler.OutputText(string Text)
		{
			Console.WriteLine(Text);
		}

		void CookerTools.IOutputHandler.OutputText(string Text, Color OutputColor)
		{
			// ignore the color
			Console.WriteLine(Text);
		}
	};

	/// <summary>
	/// Summary description for Class1.
	/// </summary>
	class CookerSyncApp
	{
		static void ShowUsage()
		{
			Console.WriteLine("Copies game content to one or more locations");
			Console.WriteLine("");
			Console.WriteLine("CookerSync <Name> [-Options] [-b <Dir>] [Destinations...]");
			Console.WriteLine("");
			Console.WriteLine("  Name           : Name of the game to be synchronized");
			Console.WriteLine("  Destinations   : One or more target Xbox names or UNC paths");
			Console.WriteLine("");
			Console.WriteLine("Options:");
			Console.WriteLine("  -b, -base      : Specify base directory <Dir> to be used");
			Console.WriteLine("                   If not specified defaults to: UnrealEngine3");
			Console.WriteLine("  -c, -crc       : Generate CRC when creating the TOC");
			Console.WriteLine("  -f, -force     : Force copying of all files regardless of time stamp");
			Console.WriteLine("  -h, -help      : This help text");
			Console.WriteLine("  -l, -log       : More verbose logging output");
			Console.WriteLine("  -m, -merge     : Use existing TOC file for CRCs");
			Console.WriteLine("  -n, -nosync    : Do not sync files, preview only");
			Console.WriteLine("  -nd, -nodest   : Ignore all destinations (useful for just generating TOCs");
			Console.WriteLine("  -v, -verify    : Verify that the CRCs match between source and destination");
			Console.WriteLine("                   Use in combination with -merge and/or -crc (PC only)");
			Console.WriteLine("");
			Console.WriteLine("Examples:");
			Console.WriteLine("  Copy ExampleGame to the default xbox:");
			Console.WriteLine("\tCookerSync Example");
			Console.WriteLine("");
			Console.WriteLine("  Copy ExampleGame to the xbox named tango and xbox named bravo:");
			Console.WriteLine("\tCookerSync Example tango bravo");
			Console.WriteLine("");
			Console.WriteLine("  Copy ExampleGame to the xbox named tango and used UnrealEngine3-Demo as");
			Console.WriteLine("  the Xbox base directory:");
			Console.WriteLine("\tCookerSync Example tango -base UnrealEngine3-Demo");
			Console.WriteLine("");
			Console.WriteLine("  Copy ExampleGame to the PC Path C:\\DVD\\ generate CRCs for the Table of");
			Console.WriteLine("  contents and verify the CRC on the destination side:");
			Console.WriteLine("\tCookerSync Example -crc -v C:\\DVD\\");
			Console.WriteLine("");
			Console.WriteLine("  Preview the copy of ExampleGame to xbox named tango and PC path \\\\Share\\:");
			Console.WriteLine("\tCookerSync Example -n tango \\\\Share\\");
			Console.WriteLine("");
			Console.WriteLine("  Verify the copy of ExampleGame at C:\\DVD\\ without performing any copying:");
			Console.WriteLine("\tCookerSync Example -n -f -crc -v C:\\DVD\\");
			Console.WriteLine("");
			Console.WriteLine("  Verify the copy of ExampleGame at C:\\DVD\\ using the existing TOC file:");
			Console.WriteLine("\tCookerSync Example -v -m -crc -n -f C:\\DVD\\");
			Console.WriteLine("");
			Console.WriteLine("  Only generate the TOC with CRCs for ExampleGame:");
			Console.WriteLine("\tCookerSync Example -crc -nd");
			Console.WriteLine("");
			Console.WriteLine("  Merge CRCs from existing TOC file and generate CRCs for any missing files");
			Console.WriteLine("  or files that have mismatched lengths.  Write the resulting TOC to disk");
			Console.WriteLine("  without doing anything else:");
			Console.WriteLine("\tCookerSync Example -m -crc -n -nd");
		}

		/// <summary>
		/// Main entry point
		/// </summary>
		/// <param name="Args">Command line arguments</param>
		/// <returns></returns>
		[STAThread]
		static int Main(string[] Args)
		{
			// default to successful
			bool bWasSuccessful = true;

			if (Args.Length == 0)
			{
				ShowUsage();
				return 1;
			}

			if (Args[0].StartsWith("-"))
			{
				ShowUsage();
				return 1;
			}
																												
			// create the cooker tools helper
			CookerToolsClass CookerTools = new CookerToolsClass(Application.StartupPath, new ConsoleLogger());

			// activate the target platform 
			CookerTools.Activate("Xenon");

			// get the game/xbox names from the command line
			ArrayList ConsoleNames = new ArrayList();

			// get the paths from the command line
			ArrayList DestinationPaths = new ArrayList();

			// default to default dir
			string BaseDirectory = null;

			// Getlist of all known consoles
			ArrayList ConsoleList = CookerTools.GetKnownConsoles();
			string DefaultConsole = CookerTools.GetDefaultConsole();

			// Default flags
			bool Force = false;
			bool NoSync = false;
			bool MergeExistingCRC = false;
			bool ComputeCRC = false;
			bool VerifyCRC = false;
			bool VerboseOutput = false;
			bool IgnoreDest = false;

			if ( !CookerTools.VerifyGameInfo(Args[0]) )
			{
				Console.WriteLine("Error: '" + Args[0] + "' is not a valid game name.");
				return 0;
			}

			// if any params were specified, parse them
			if (Args.Length > 1)
			{
				for (int ArgIndex = 1; ArgIndex < Args.Length; ArgIndex++)
				{
					if ((String.Compare(Args[ArgIndex], "-b", true) == 0) || (String.Compare(Args[ArgIndex], "-base", true) == 0))
					{
						// make sure there is another param after this one
						if (Args.Length > ArgIndex + 1)
						{
							// the next param is the base directory
							BaseDirectory = Args[ArgIndex + 1];

							// skip over it
							ArgIndex++;
						}
						else
						{
							Console.WriteLine("Error: No base specified (use -h to see usage).");
							return 0;
						}
					}
					else if ((String.Compare(Args[ArgIndex], "-c", true) == 0) || (String.Compare(Args[ArgIndex], "-crc", true) == 0))
					{
						ComputeCRC = true;
					}
					else if ((String.Compare(Args[ArgIndex], "-f", true) == 0) || (String.Compare(Args[ArgIndex], "-force", true) == 0))
					{
						Force = true;
					}
					else if ((String.Compare(Args[ArgIndex], "-h", true) == 0) || (String.Compare(Args[ArgIndex], "-help", true) == 0))
					{
						ShowUsage();
						return 0;
					}
					else if ((String.Compare(Args[ArgIndex], "-l", true) == 0) || (String.Compare(Args[ArgIndex], "-log", true) == 0))
					{
						VerboseOutput = true;
					}
					else if ((String.Compare(Args[ArgIndex], "-m", true) == 0) || (String.Compare(Args[ArgIndex], "-merge", true) == 0))
					{
						MergeExistingCRC = true;
					}
					else if ((String.Compare(Args[ArgIndex], "-n", true) == 0) || (String.Compare(Args[ArgIndex], "-nosync", true) == 0))
					{
						NoSync = true;
					}
					else if ((String.Compare(Args[ArgIndex], "-nd", true) == 0) || (String.Compare(Args[ArgIndex], "-nodest", true) == 0))
					{
						IgnoreDest = true;
					}
					else if ((String.Compare(Args[ArgIndex], "-v", true) == 0) || (String.Compare(Args[ArgIndex], "-verify", true) == 0))
					{
						VerifyCRC = true;
					}
					else if (Args[ArgIndex].StartsWith("-"))
					{
						Console.WriteLine("Error: '" + Args[ArgIndex] + "' is not a valid option (use -h to see usage).");
						return 0;
					}
					else
					{
						// is this a PC destination path?
						if ( IsDirectory(Args[ArgIndex]) )
						{
							DestinationPaths.Add(Args[ArgIndex]);
						}
						else
						{
							ConsoleNames.Add( Args[ArgIndex] );
							bWasSuccessful = true;
						}
					}
				}
			}
			
			if ( bWasSuccessful && ConsoleNames.Count == 0 && DestinationPaths.Count == 0 && !IgnoreDest )
			{
				// is there a default console?
				if ( DefaultConsole != null && DefaultConsole.Length > 0)
				{
					ConsoleNames.Add(DefaultConsole);
				}
				else
				{
					Console.WriteLine("Error: No default Xbox has been specified in the Xbox 360 Neighborhood.");
					bWasSuccessful = false;
				}
			}

			if (bWasSuccessful)
			{
				CookerTools.Force = Force;
				CookerTools.NoSync = NoSync;
				CookerTools.ComputeCRC = ComputeCRC;
				CookerTools.MergeExistingCRC = MergeExistingCRC;
				CookerTools.VerifyCopy = VerifyCRC;
				CookerTools.VerboseOutput = VerboseOutput;

				CookerTools.SetGameAndConsoleInfo(Args[0], ConsoleNames, BaseDirectory, DestinationPaths);

				try
				{
					// try to sync to the given xbox
					bWasSuccessful = CookerTools.TargetSync();
				}
				catch (System.Exception e)
				{
					bWasSuccessful = false;
					Console.WriteLine("Exception was " + e.ToString());
				}
			}

			// did we succeed?
			return bWasSuccessful ? 0 : 1;
		}

		static bool IsDirectory( String Directory )
		{
			if ( Directory.IndexOf('\\') == -1 )
				return false;

			return true;
		}
	}
}
