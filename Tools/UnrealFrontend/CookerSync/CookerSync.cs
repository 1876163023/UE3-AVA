using System;using System.Collections;
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
			Console.WriteLine("CookerSync <Name> [-Options] [-b <Dir>] [-p <Platform>] [Destinations...]");
			Console.WriteLine("");
			Console.WriteLine("  Name           : Name of the game to be synchronized");
			Console.WriteLine("  Destinations   : One or more targets (Xbox only) or UNC paths (any platform)");
			Console.WriteLine("                 : Destination required for PC or PS3. If Xbox360, default Xbox is used.");
			Console.WriteLine("");
			Console.WriteLine("Options:");
			Console.WriteLine("  -b, -base      : Specify base directory <Dir> to be used [Xbox only]");
			Console.WriteLine("                   If not specified defaults to: UnrealEngine3");
			Console.WriteLine("  -c, -crc       : Generate CRC when creating the TOC");
            Console.WriteLine("  -d, -demo      : Only include files with the IsForDemo flag set" );
            Console.WriteLine("  -f, -force     : Force copying of all files regardless of time stamp" );
			Console.WriteLine("  -h, -help      : This help text");
			Console.WriteLine("  -l, -log       : More verbose logging output");
			Console.WriteLine("  -m, -merge     : Use existing TOC file for CRCs");
			Console.WriteLine("  -n, -nosync    : Do not sync files, preview only");
			Console.WriteLine("  -nd, -nodest   : Ignore all destinations (useful for just generating TOCs");
			Console.WriteLine("  -p, -platform  : Specify platform <Platform> to be used");
			Console.WriteLine("                   Can be one of: Xbox360, Xenon, PS3 or PC [Default is Xbox360]");
            Console.WriteLine("  -r, -region    : Three letter code determining packages to copy [Default is None]");
            Console.WriteLine("                 : Used for copying loc packages, all packages of the type _LOC.upk");
            Console.WriteLine("  -s, -sleep     : Specifies sleep (in ms) between each copy and verify to ease network hogging");
            Console.WriteLine("                 : Default is 25ms");
            Console.WriteLine("  -t, -target    : Only copy files with the 'IsForTarget' flag set");
            Console.WriteLine("  -v, -verify    : Verify that the CRCs match between source and destination");
			Console.WriteLine("                   Use in combination with -merge and/or -crc [copying to PC only]");
			Console.WriteLine("");
			Console.WriteLine("Examples:");
			Console.WriteLine("  Copy ExampleGame to the default xbox:");
			Console.WriteLine("\tCookerSync Example");
			Console.WriteLine("");
			Console.WriteLine("  Copy ExampleGame to the xbox named tango and xbox named bravo:");
			Console.WriteLine("\tCookerSync Example tango bravo");
			Console.WriteLine("");
			Console.WriteLine("  Copy ExampleGame for PS3 to the to PC path \\\\Share\\");
			Console.WriteLine("\tCookerSync Example -p PS3 \\\\Share\\");
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
            string CWD = Environment.CurrentDirectory.ToLower();
            if( !CWD.EndsWith( "binaries" ) )
            {
                Console.WriteLine( "Error: CookerSync must be run from the 'Binaries' folder" );
                return 1;
            }
																		
			// get the game/xbox names from the command line
			ArrayList ConsoleNames = new ArrayList();

			// get the paths from the command line
			ArrayList DestinationPaths = new ArrayList();

			// default to default dir
			string BaseDirectory = null;

			// default platform to xbox
			string Platform = "Xbox360";

            // Default is to copy all files
            string LocRegion = "";

            // Sleep duration to ease network hogging
            int SleepDelay = 25;

			// Default flags
			bool Force = false;
			bool NoSync = false;
			bool MergeExistingCRC = false;
			bool ComputeCRC = false;
			bool bIsForTargetOnly = false;
			bool bIsInDemoMode = false;
			bool VerifyCRC = false;
			bool VerboseOutput = false;
			bool IgnoreDest = false;

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
					else if ((String.Compare(Args[ArgIndex], "-p", true) == 0) || (String.Compare(Args[ArgIndex], "-platform", true) == 0))
					{
						// make sure there is another param after this one
						if (Args.Length > ArgIndex + 1)
						{
							// the next param is the base directory
							Platform = Args[ArgIndex + 1];

							// skip over it
							ArgIndex++;
						}
						else
						{
							Console.WriteLine("Error: No platform specified (use -h to see usage).");
							return 0;
						}
					}
                    else if( ( String.Compare( Args[ArgIndex], "-r", true ) == 0 ) || ( String.Compare( Args[ArgIndex], "-region", true ) == 0 ) )
                    {
                        // make sure there is another param after this one
                        if( Args.Length > ArgIndex + 1 )
                        {
                            // the next param is the base directory
                            LocRegion = Args[ArgIndex + 1];

                            // skip over it
                            ArgIndex++;
                        }
                        else
                        {
                            Console.WriteLine( "Error: No region specified (use -h to see usage)." );
                            return 0;
                        }
                    }
                    else if( ( String.Compare( Args[ArgIndex], "-s", true ) == 0 ) || ( String.Compare( Args[ArgIndex], "-sleep", true ) == 0 ) )
                    {
                        // make sure there is another param after this one
                        if( Args.Length > ArgIndex + 1 )
                        {
                            try
                            {
                                SleepDelay = Int32.Parse( Args[ArgIndex + 1] );
                            }
                            catch
                            {
                            }

                            // skip over it
                            ArgIndex++;
                        }
                        else
                        {
                            Console.WriteLine( "Error: No sleep delay specified (use -h to see usage)." );
                            return 0;
                        }
                    }
                    else if( ( String.Compare( Args[ArgIndex], "-c", true ) == 0 ) || ( String.Compare( Args[ArgIndex], "-crc", true ) == 0 ) )
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
					else if ((String.Compare(Args[ArgIndex], "-t", true) == 0) || (String.Compare(Args[ArgIndex], "-target", true) == 0))
					{
						bIsForTargetOnly = true;
					}
					else if ((String.Compare(Args[ArgIndex], "-d", true) == 0) || (String.Compare(Args[ArgIndex], "-demo", true) == 0))
					{
						bIsInDemoMode = true;
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

			// create the cooker tools helper
			CookerToolsClass CookerTools = new CookerToolsClass(new ConsoleLogger());

            // Loc region required as Activate() creates the TOC file
            CookerTools.LocRegion = LocRegion;

            // activate the target platform 
            CookerTools.Activate( Platform );

			// process handling platforms that sync, use the default target if needed
			if (bWasSuccessful && CookerTools.PlatformNeedsToSync() && ConsoleNames.Count == 0 && DestinationPaths.Count == 0 && !IgnoreDest)
			{
				// Get default console
				string DefaultConsole = CookerTools.GetDefaultConsole();

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

			// validate that we have a destination if needed
			if (ConsoleNames.Count == 0 && DestinationPaths.Count == 0 && !IgnoreDest)
			{
				Console.WriteLine("Error: No destination found. Use -h for help.");
				bWasSuccessful = false;
			}

			if (bWasSuccessful)
			{
				CookerTools.Force = Force;
				CookerTools.NoSync = NoSync;
				CookerTools.ComputeCRC = ComputeCRC;
				CookerTools.MergeExistingCRC = MergeExistingCRC;
				CookerTools.bIsForTargetOnly = bIsForTargetOnly;
				CookerTools.bIsInDemoMode = bIsInDemoMode;
				CookerTools.VerifyCopy = VerifyCRC;
				CookerTools.VerboseOutput = VerboseOutput;
                CookerTools.SleepDelay = SleepDelay;

                CookerTools.SetGameAndConsoleInfo( Environment.CurrentDirectory, Args[0], ConsoleNames, BaseDirectory, DestinationPaths );

				if (!CookerTools.VerifyGameInfo(Args[0]))
				{
					Console.WriteLine("Error: '" + Args[0] + "' is not a valid game name.");
					bWasSuccessful = false;
				}
				else
				{
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
			}

			// did we succeed?
			return bWasSuccessful ? 0 : 1;
		}

		static bool IsDirectory( String Directory )
		{
            if( Directory.IndexOf( '\\' ) == -1 )
            {
                return false;
            }

			return true;
		}
	}
}
