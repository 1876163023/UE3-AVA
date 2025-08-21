using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Text;
using System.Xml;
using System.Xml.Serialization;

namespace Controller
{
    public class JobInfo
    {
        [XmlAttribute]
        public string Name = "";

        [XmlAttribute]
        public string Command = "";

        [XmlAttribute]
        public string Parameter = "";
    }

    public class JobDescriptions
    {
        [XmlArray( "Jobs" )]
        public JobInfo[] Jobs = new JobInfo[0];

        public JobDescriptions()
        {
        }
    }

    class Command
    {
        public Main Parent = null;
        private P4 SCC = null;
        private ScriptParser Builder = null;

        private StreamWriter Log;
        private ERRORS ErrorLevel = ERRORS.None;
        private BuildProcess CurrentBuild = null;
        private DateTime StartTime = DateTime.Now;
        private DateTime LastRespondingTime = DateTime.Now;

        private string CommonCommandLine = "-unattended -nopause -buildmachine -forcelogflush";

        private string GetPlatform()
        {
            return ( "-Platform=" + Builder.LabelInfo.Platform );
        }

        public Command( Main InParent, P4 InSCC, ScriptParser InBuilder )
        {
            Parent = InParent;
            SCC = InSCC;
            Builder = InBuilder;
        }

        public ERRORS GetErrorLevel()
        {
            return ( ErrorLevel );
        }

        public BuildProcess GetCurrentBuild()
        {
            return( CurrentBuild );
        }

        protected void XmlSerializer_UnknownAttribute( object sender, XmlAttributeEventArgs e )
        {
        }

        protected void XmlSerializer_UnknownNode( object sender, XmlNodeEventArgs e )
        {
        }

        private JobDescriptions ReadJobs( string FileName )
        {
            JobDescriptions Jobs = null;

            Stream XmlStream = null;
            try
            {
                // Get the XML data stream to read from
                XmlStream = new FileStream( FileName, FileMode.Open, FileAccess.Read, FileShare.None, 256 * 1024, false );

                // Creates an instance of the XmlSerializer class so we can read the settings object
                XmlSerializer ObjSer = new XmlSerializer( typeof( JobDescriptions ) );
                // Add our callbacks for a busted XML file
                ObjSer.UnknownNode += new XmlNodeEventHandler( XmlSerializer_UnknownNode );
                ObjSer.UnknownAttribute += new XmlAttributeEventHandler( XmlSerializer_UnknownAttribute );

                // Create an object graph from the XML data
                Jobs = ( JobDescriptions )ObjSer.Deserialize( XmlStream );
            }
            catch( Exception )
            {
            }
            finally
            {
                if( XmlStream != null )
                {
                    // Done with the file so close it
                    XmlStream.Close();
                }
            }

            return ( Jobs );
        }

        private void CleanIniFiles( GameConfig Config, StreamWriter Log )
        {
            string ConfigFolder = Config.GetConfigFolderName();
            DirectoryInfo Dir = new DirectoryInfo( ConfigFolder );

            foreach( FileInfo File in Dir.GetFiles() )
            {
                if( File.IsReadOnly == false )
                {
                    File.Delete();
                }
            }
        }

        private void SetReadOnlyState( string FileSpec, bool ReadOnly )
        {
            int Offset = FileSpec.LastIndexOf( '/' );
            string Dir = FileSpec.Substring( 0, Offset );
            string File = FileSpec.Substring( Offset + 1, FileSpec.Length - Offset - 1 );

            DirectoryInfo Dirs = new DirectoryInfo( Dir );
            foreach( FileInfo Info in Dirs.GetFiles( File ) )
            {
                Info.IsReadOnly = ReadOnly;
            } 
        }

        private void SCC_CheckConsistency()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_CheckConsistency ) );

                SCC.CheckConsistency( Builder, Log, Builder.CommandLine );

                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_CheckConsistency;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while calling CheckConsistency" );
                    Log.Close();
                }
            }
        }

        private void SCC_Sync()
        {
            string Revision = "";

            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_Sync ) );

                // Name of a build type that we wish to get the last good build from
                Builder.SyncedLabel = Parent.GetLabelToSync();
                SCC.SyncToRevision( Builder, Log, Builder.SyncedLabel );

                if( Builder.LastGoodBuild != 0 )
                {
                    Revision = Parent.GetChangeListToSync();
                    SCC.GetChangesSinceLastBuild( Builder, Log );
                }

                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_Sync;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while calling sync" );
                    Log.Close();
                }
            }
        }

        private void SCC_SyncSingleChangeList()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_SyncSingleChangeList ) );

                // Name of a build type that we wish to get the last good build from
                SCC.SyncSingleChangeList( Builder, Log );

                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_SyncSingleChangeList;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while calling sync single changelist" );
                    Log.Close();
                }
            }
        }

        private void SCC_Checkout()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_Checkout ) );

                if( SCC.CheckoutFileSpec( Builder, Log, Builder.CommandLine, false ) )
                {
                    Builder.FilesCheckedOut = true;
                }

                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_Checkout;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while calling checkout" );
                    Log.Close();
                }
            }
        }

        private void SCC_OpenForDelete()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_OpenForDelete ) );

                if( SCC.OpenForDeleteFileSpec( Builder, Log, Builder.CommandLine ) )
                {
                    Builder.FilesCheckedOut = true;
                }

                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_OpenForDelete;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while calling open for delete" );
                    Log.Close();
                }
            }
        }

        private void SCC_CheckoutGame()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_CheckoutGame ) );

                GameConfig Config = Builder.AddCheckedOutGame();

                string[] Executables = Config.GetExecutableNames();
                foreach( string Executable in Executables )
                {
                    if( Executable.Length > 0 )
                    {
                        if( SCC.CheckoutFileSpec( Builder, Log, Executable, false ) )
                        {
                            Builder.FilesCheckedOut = true;
                        }
                    }
                }

                string SymbolFile = Config.GetSymbolFileName();
                if( SymbolFile.Length > 0 )
                {
                    if( SCC.CheckoutFileSpec( Builder, Log, SymbolFile, false ) )
                    {
                        Builder.FilesCheckedOut = true;
                    }
                }

                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_CheckoutGame;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while calling checkout game" );
                    Log.Close();
                }
            }
        }

        private void SCC_CheckoutShader()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_CheckoutShader ) );

                GameConfig Config = Builder.CreateGameConfig();

                string ShaderFile = Config.GetRefShaderName();
                if( SCC.CheckoutFileSpec( Builder, Log, ShaderFile, false ) )
                {
                    Builder.FilesCheckedOut = true;
                }

                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_CheckoutShader;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while calling checkout shader" );
                    Log.Close();
                }
            }
        }

        private void SCC_CheckoutDialog()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_CheckoutDialog ) );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length != 2 )
                {
                    Builder.Write( Log, "Error, not enough parameters for CheckoutDialog" );
                }
                else
                {
                    GameConfig Config = Builder.CreateGameConfig( Parms[0] );
                    Queue<string> Languages = Builder.GetLanguages();
                    Queue<string> ValidLanguages = new Queue<string>();

                    foreach( string Language in Languages )
                    {
                        string DialogFile = Config.GetDialogFileName( Language, Parms[1] );
                        if( SCC.CheckoutFileSpec( Builder, Log, DialogFile, false ) )
                        {
                            Builder.FilesCheckedOut = true;
                            ValidLanguages.Enqueue( Language );
                        }
                    }

                    Builder.SetValidLanguages( ValidLanguages );
                }

                // Some files are allowed to not exist (and fail checkout)
                ErrorLevel = ERRORS.None;
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_CheckoutDialog;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while calling checkout dialog" );
                    Log.Close();
                }
            }
        }

        private void SCC_CheckoutFonts()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_CheckoutFonts ) );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length != 2 )
                {
                    Builder.Write( Log, "Error, not enough parameters for CheckoutFonts" );
                }
                else
                {
                    GameConfig Config = Builder.CreateGameConfig( Parms[0] );
                    Queue<string> Languages = Builder.GetLanguages();
                    Queue<string> ValidLanguages = new Queue<string>();

                    foreach( string Language in Languages )
                    {
                        string FontFile = Config.GetFontFileName( Language, Parms[1] );
                        if( SCC.CheckoutFileSpec( Builder, Log, FontFile, false ) )
                        {
                            Builder.FilesCheckedOut = true;
                            ValidLanguages.Enqueue( Language );
                        }
                    }

                    Builder.SetValidLanguages( ValidLanguages );
                }

                // Some files are allowed to not exist (and fail checkout)
                ErrorLevel = ERRORS.None;
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_CheckoutFonts;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while calling checkout dialog" );
                    Log.Close();
                }
            }
        }

        private void SCC_CheckoutLocPackage()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_CheckoutLocPackage ) );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length != 1 )
                {
                    Builder.Write( Log, "Error, not enough parameters for CheckoutLocPackage" );
                }
                else
                {
                    GameConfig Config = Builder.CreateGameConfig( "Example" );
                    Queue<string> Languages = Builder.GetLanguages();
                    Queue<string> ValidLanguages = new Queue<string>();

                    foreach( string Language in Languages )
                    {
                        string PackageFile = Config.GetPackageFileName( Language, Parms[0] );
                        if( SCC.CheckoutFileSpec( Builder, Log, PackageFile, false ) )
                        {
                            Builder.FilesCheckedOut = true;
                            ValidLanguages.Enqueue( Language );
                        }
                    }

                    Builder.SetValidLanguages( ValidLanguages );
                }

                // Some files are allowed to not exist (and fail checkout)
                ErrorLevel = ERRORS.None;
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_CheckoutFonts;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while calling checkout dialog" );
                    Log.Close();
                }
            }
        }

        private void SCC_CheckoutGDF()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_CheckoutGDF ) );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length != 2 )
                {
                    Builder.Write( Log, "Error, incorrect number of parameters for CheckoutGDF" );
                    ErrorLevel = ERRORS.SCC_CheckoutGDF;
                }
                else
                {
                    Queue<string> Languages = Builder.GetLanguages();

                    foreach( string Lang in Languages )
                    {
                        string GDFFileName = Parms[1] + "/" + Lang.ToUpper() + "/" + Parms[0] + "Game.gdf.xml";
                        if( SCC.CheckoutFileSpec( Builder, Log, GDFFileName, false ) )
                        {
                            Builder.FilesCheckedOut = true;
                        }
                    }
                }

                // Some files are allowed to not exist (and fail checkout)
                ErrorLevel = ERRORS.None;
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_CheckoutGDF;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while calling checkout dialog" );
                    Log.Close();
                }
            }
        }

        private void MakeWritable()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.MakeWritable ) );

                SetReadOnlyState( Builder.CommandLine, false );

                Builder.AddMadeWritableFileSpec( Builder.CommandLine );

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.MakeWritable;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while making files writable" );
                    Log.Close();
                }
            }
        }

        private void SCC_Submit()
        {
            if( Builder.FilesCheckedOut )
            {
                try
                {
                    Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_Submit ) );
                    int ChangeList = SCC.Submit( Log, Builder, 0 );

                    ErrorLevel = SCC.GetErrorLevel();
                    if( ErrorLevel == ERRORS.SCC_Submit )
                    {
                        // Interrogate P4 to see if any files needs resolving
                        if( SCC.GetIncorrectCheckedOutFiles( Builder ) )
                        {
                            SCC.Resolve( Log, Builder );

                            ErrorLevel = SCC.GetErrorLevel();
                            if( ErrorLevel == ERRORS.None )
                            {
                                SCC.Submit( Log, Builder, ChangeList );
                                ErrorLevel = SCC.GetErrorLevel();
                            }
                        }
                    }

                    if( ErrorLevel == ERRORS.None )
                    {
                        Builder.FilesCheckedOut = false;
                    }

                    Log.Close();
                }
                catch
                {
                    ErrorLevel = ERRORS.SCC_Submit;
                    if( Log != null )
                    {
                        Builder.Write( Log, "Error, exception while calling submit" );
                        Log.Close();
                    }
                }
            }
            else
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_Submit ) );
                ErrorLevel = ERRORS.SCC_Submit;
                Log.Close();
            }
        }

        private void SCC_CreateNewLabel()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_CreateNewLabel ) );

                SCC.CreateNewLabel( Log, Builder );
                Builder.Dependency = Builder.LabelInfo.GetLabelName();

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_CreateNewLabel;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while creating a new Perforce label" );
                    Log.Close();
                }
            }
        }

        private void SCC_UpdateLabelDescription()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_UpdateLabelDescription ) );

                SCC.UpdateLabelDescription( Log, Builder );

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_UpdateLabelDescription;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while updating a Perforce label" );
                    Log.Close();
                }
            }
        }

        private void SCC_Revert()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_Revert ) );

                Parent.Log( "[STATUS] Reverting: '...'", Color.Magenta );
                SCC.Revert( Builder, Log, "..." );

                Builder.FilesCheckedOut = false;
                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_Revert;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while calling revert" );
                    Log.Close();
                }
            }
        }

        private void SCC_RevertFile()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_RevertFile ) );

                Parent.Log( "[STATUS] Reverting: '" + Builder.CommandLine + "'", Color.Magenta );
                SCC.Revert( Builder, Log, Builder.CommandLine );

                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_RevertFile;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while calling revert" );
                    Log.Close();
                }
            }
        }

        private void MakeReadOnly()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.MakeReadOnly ) );

                List<string> WriteableFiles = Builder.GetMadeWritableFiles();
                foreach( string FileSpec in WriteableFiles )
                {
                    Parent.Log( "[STATUS] Making read only: '" + FileSpec + "'", Color.Magenta );
                    SetReadOnlyState( FileSpec, true );
                }

                Builder.ClearMadeWritableFiles();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.MakeReadOnly;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while making files read only" );
                    Log.Close();
                }
            }
        }

        private void SCC_Tag()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_Tag ) );

                SCC.Tag( Builder, Log );
                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_Tag;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while tagging build" );
                    Log.Close();
                }
            }
        }

        private void SCC_TagFile()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_TagFile ) );

                SCC.TagFile( Builder, Builder.CommandLine, Log );
                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_TagFile;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while tagging file" );
                    Log.Close();
                }
            }
        }

        private void SCC_TagPCS()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_TagPCS ) );

                GameConfig Config = Builder.CreateGameConfig();
                SCC.TagFile( Builder, Config.GetRefShaderName(), Log );
                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_TagPCS;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while tagging PCS" );
                    Log.Close();
                }
            }
        }

        private void SCC_TagExe()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_TagExe ) );

                GameConfig Config = Builder.CreateGameConfig();

                // Tag the executable names
                string[] ExeNames = Config.GetExecutableNames();
                foreach( string ExeName in ExeNames )
                {
                    if( ExeName.Length > 0 )
                    {
                        SCC.TagFile( Builder, ExeName, Log );
                    }
                }

                // Tag the symbol files
                string DebugName = Config.GetSymbolFileName();
                if( DebugName.Length > 0 )
                {
                    SCC.TagFile( Builder, DebugName, Log );
                }

                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_TagExe;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while tagging executable and symbol files" );
                    Log.Close();
                }
            }
        }

        private void CreatePackageJobs()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.CreatePackageJobs ) );
                
                GameConfig Config = Builder.CreateGameConfig();
                string Executable = Config.GetExeName();

                string CommandLine = "cookpackages " + GetPlatform() + " -distributed -generatejobs -numjobs=" + Builder.NumJobsToGenerate.ToString() + " " + CommonCommandLine;

                CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", false, true );
                ErrorLevel = CurrentBuild.GetErrorLevel();

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.CreatePackageJobs;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while creating package jobs." );
                    Log.Close();
                }
            }
        }

        private void LoadJobs()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.LoadJobs ) );

                GameConfig Config = Builder.CreateGameConfig();
                string JobsXML = Config.GetCookedFolderName() + "/Jobs.xml";
                JobDescriptions Jobs = ReadJobs( JobsXML );
                if( Jobs != null )
                {
                    Parent.PopulateJobs( Jobs );
                }
                else
                {
                    Builder.Write( Log, "Error, failed to load jobs xml file." );
                    ErrorLevel = ERRORS.LoadJobs;
                }

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.LoadJobs;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while loading jobs." );
                    Log.Close();
                }
            }
        }

        private void MergeJobs()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.MergeJobs ) );

                GameConfig Config = Builder.CreateGameConfig();
                string Executable = Config.GetExeName();

                string CommandLine = "cookpackages " + GetPlatform() + " -mergejobs " + CommonCommandLine;

                CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", false, true );
                ErrorLevel = CurrentBuild.GetErrorLevel();

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.MergeJobs;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while merging jobs." );
                    Log.Close();
                }
            }
        }

        private void CopyJobs()
        {
            try
            {
                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length != 1 )
                {
                    ErrorLevel = ERRORS.CopyJobs;
                }
                else
                {
#if FALSE
                    string Executable = "Binaries/BuilderSync.exe";
                    string CommandLine = Builder.LabelInfo.Game + " -p " + Builder.LabelInfo.Platform;

                    CommandLine += " -b " + Builder.SourceControlBranch;
                    CommandLine += " -crc -v";

                    string PublishFolder = Parms[0].Replace( '/', '\\' ) + "\\" + Builder.LabelInfo.GetFolderName( Builder.AppendLanguage );
                    CommandLine += " " + PublishFolder;

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, Environment.CurrentDirectory + "/Binaries", false, false );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
#else
                    GameConfig Config = Builder.CreateGameConfig();

                    string Source = Config.GetJobsFolderName().Replace( '/', '\\' );
                    string Dest = Parms[0].Replace( '/', '\\' ) + "\\" + Builder.SourceControlBranch + "\\" + Config.GetJobsFolderName().Replace( '/', '\\' );

                    string CommandLine = "/c xcopy.exe " + Source + "\\*.* ";
                    CommandLine += " " + Dest;
                    CommandLine += " /s /f /r /y /i > " + Builder.GetLogFileName( COMMANDS.CopyJobs ) + " 2>&1";

                    CurrentBuild = new BuildProcess( Parent, Builder, CommandLine, "" );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
#endif
                }

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.CopyJobs;
            }
        }

        private void GetJobs()
        {
            try
            {
                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length != 1 )
                {
                    ErrorLevel = ERRORS.GetJobs;
                }
                else
                {
#if FALSE
                    string Executable = "Binaries/BuilderSync.exe";
                    string CommandLine = Builder.LabelInfo.Game + " -p " + Builder.LabelInfo.Platform;

                    CommandLine += " -b " + Builder.SourceControlBranch;
                    CommandLine += " -crc -v";

                    string PublishFolder = Parms[0].Replace( '/', '\\' ) + "\\" + Builder.LabelInfo.GetFolderName( Builder.AppendLanguage );
                    CommandLine += " " + PublishFolder;

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, Environment.CurrentDirectory + "/Binaries", false, false );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
#else
                    GameConfig Config = Builder.CreateGameConfig();

                    string Source = Config.GetJobsFolderName().Replace( '/', '\\' );
                    string Dest = Parms[0].Replace( '/', '\\' ) + "\\" + Builder.SourceControlBranch + "\\" + Config.GetJobsFolderName().Replace( '/', '\\' );

                    string CommandLine = "/c xcopy.exe " + Dest + "\\*.* ";
                    CommandLine += " " + Source;
                    CommandLine += " /s /f /r /y /i > " + Builder.GetLogFileName( COMMANDS.GetJobs ) + " 2>&1";

                    CurrentBuild = new BuildProcess( Parent, Builder, CommandLine, "" );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
#endif
                }

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.GetJobs;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while getting jobs." );
                    Log.Close();
                }
            }
        }

        private void CleanJobs()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.CleanJobs ) );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length != 1 )
                {
                    Builder.Write( Log, "Error, incorrect number of parameters. Usage: CleanJobs <RootPath>." );
                    ErrorLevel = ERRORS.CleanJobs;
                }
                else
                {
                    GameConfig Config = Builder.CreateGameConfig();

                    string Dest = Parms[0].Replace( '/', '\\' ) + "\\" + Builder.SourceControlBranch + "\\" + Config.GetJobsFolderName().Replace( '/', '\\' );
                    Builder.Write( Log, " ... deleting: '" + Dest + "'" );
                    Parent.DeleteDirectory( Dest, 0 );
                }

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.CleanJobs;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while cleaning jobs on server." );
                    Log.Close();
                }
            }
        }

        private void AddSimpleJob( StreamWriter Log, string[] Parameters )
        {
            JobInfo Job = new JobInfo();

            Job.Name = "Job";
            if( Builder.LabelInfo.Game.Length > 0 )
            {
                Job.Name += "_" + Builder.LabelInfo.Game;
            }
            if( Builder.LabelInfo.Platform.Length > 0 )
            {
                Job.Name += "_" + Builder.LabelInfo.Platform;
            }
            Job.Name += "_" + Parameters[1];

            Job.Command = Parameters[0];
            Job.Parameter = Parameters[1];

            for( int i = 2; i < Parameters.Length; i++ )
            {
                Job.Parameter += " " + Parameters[i];
            }

            Parent.AddJob( Job );

            Builder.Write( Log, "Added Job: " + Job.Name );
        }

        private void AddJob()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.AddJob ) );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length < 2 )
                {
                    Builder.Write( Log, "Error, too few parameters. Usage: AddJob <Script> <File1> [File2...]." );
                    ErrorLevel = ERRORS.AddJob;
                }
                else
                {
                    AddSimpleJob( Log, Parms );
                }

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.AddJob;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while adding a job." );
                    Log.Close();
                }
            }
        }

        private void AddGCCJob()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.AddGCCJob ) );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length < 1 )
                {
                    Builder.Write( Log, "Error, too few parameters. Usage: AddGCCJob <Config> [Game1] [Game2...]." );
                    ErrorLevel = ERRORS.AddGCCJob;
                }
                else
                {
                    string[] Parameters = new string[2] { "Jobs/CompileGCCJob", Parms[0] };
                    AddSimpleJob( Log, Parameters );

                    string OrigConfig = Builder.BuildConfiguration;
                    Builder.BuildConfiguration = Parms[0]; 
                    for( int Index = 1; Index < Parms.Length; Index++ )
                    {
                        Builder.LabelInfo.Game = Parms[Index];
                        GameConfig Config = Builder.CreateGameConfig();
                        Builder.LabelInfo.Games.Add( Config );
                    }
                    Builder.BuildConfiguration = OrigConfig;
                }

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.AddGCCJob;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while adding a GCC compile." );
                    Log.Close();
                }
            }
        }

        private void AddMSVCJob()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.AddMSVCJob ) );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length < 1 )
                {
                    Builder.Write( Log, "Error, too few parameters. Usage: AddMSVCJob <Config> [Game1] [Game2...]." );
                    ErrorLevel = ERRORS.AddMSVCJob;
                }
                else
                {
                    string[] Parameters = new string[2] { "Jobs/CompileMSVCJob", Parms[0] };
                    AddSimpleJob( Log, Parameters );

                    string OrigConfig = Builder.BuildConfiguration;
                    Builder.BuildConfiguration = Parms[0];
                    for( int Index = 1; Index < Parms.Length; Index++ )
                    {
                        Builder.LabelInfo.Game = Parms[Index];
                        GameConfig Config = Builder.CreateGameConfig();
                        Builder.LabelInfo.Games.Add( Config );
                    }
                    Builder.BuildConfiguration = OrigConfig;
                }

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.AddMSVCJob;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while adding a MSVC compile." );
                    Log.Close();
                }
            }
        }

        private void CookPackageJob()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.CookPackageJob ) );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length < 1 )
                {
                    Builder.Write( Log, "Error, too few parameters. Usage: CookPackageJob <Package1> [Package2...]." );
                    ErrorLevel = ERRORS.CookPackageJob;
                }
                else
                {
                    GameConfig Config = Builder.CreateGameConfig();

                    string Executable = Config.GetExeName();
                    string CommandLine = "CookPackages " + GetPlatform();

                    for( int i = 0; i < Parms.Length; i++ )
                    {
                        CommandLine += " " + Parms[i];
                    }

                    CommandLine += " -jobname=" + Builder.LabelInfo.BuildType;
                    CommandLine += " -distributed -jobpackages -updateInisAuto " + CommonCommandLine;
                    CommandLine += Builder.GetCompressionConfiguration() + Builder.GetScriptConfiguration();

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", true, true );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
                }

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.CookPackageJob;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while doing a cook package job." );
                    Log.Close();
                }
            }
        }

        private void CookMapJob()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.CookMapJob ) );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length < 1 )
                {
                    Builder.Write( Log, "Error, too few parameters. Usage: CookMapJob <Map1> [Map2...]." );
                    ErrorLevel = ERRORS.CookMapJob;
                }
                else
                {
                    GameConfig Config = Builder.CreateGameConfig();

                    string Executable = Config.GetExeName();
                    string CommandLine = "CookPackages " + GetPlatform();

                    for( int i = 0; i < Parms.Length; i++ )
                    {
                        CommandLine += " " + Parms[i];
                    }

                    CommandLine += " -jobname=" + Builder.LabelInfo.BuildType;
                    CommandLine += " -distributed -mapsonly -updateInisAuto " + CommonCommandLine;
                    CommandLine += Builder.GetCompressionConfiguration() + Builder.GetScriptConfiguration();

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", true, true );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
                }

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.CookMapJob;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while doing a cook map job." );
                    Log.Close();
                }
            }
        }

        private void MSVC_Clean()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.MSVCClean ) );

                string CommandLine = "";
                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length < 1 || Parms.Length > 2 )
                {
                    Builder.Write( Log, "Error, incorrect number of parameters. Usage: MSVCClean <Solution> [Project]" );
                    ErrorLevel = ERRORS.MSVCClean;
                }
                else
                {
                    if( Parms.Length == 1 )
                    {
                        CommandLine = Parms[0] + ".sln /clean \"" + Builder.GetBuildConfiguration() + "\"";
                    }
                    else if( Parms.Length == 2 )
                    {
                        CommandLine = Parms[0] + ".sln /project " + Builder.GetProjectName( Parms[1] ) + " /clean \"" + Builder.GetBuildConfiguration() + "\"";
                    }

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Builder.MSVCApplication, CommandLine, "", true, false );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
                }

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.MSVCClean;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to clean" );
                    Log.Close();
                }
            }
        }

        private void MSVC_Build()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.MSVCBuild ) );

                string CommandLine = "";
                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length < 1 || Parms.Length > 2 )
                {
                    Builder.Write( Log, "Error, incorrect number of parameters. Usage: MSVCBuild <Solution> [Project]" );
                    ErrorLevel = ERRORS.MSVCBuild;
                }
                else
                {
                    if( Parms.Length == 1 )
                    {
                        CommandLine = Parms[0] + ".sln /build \"" + Builder.GetBuildConfiguration() + "\"";
                    }
                    else if( Parms.Length == 2 )
                    {
                        CommandLine = Parms[0] + ".sln /project " + Builder.GetProjectName( Parms[1] ) + " /build \"" + Builder.GetBuildConfiguration() + "\"";
                    }

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Builder.MSVCApplication, CommandLine, "", true, false );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
                }

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.MSVCBuild;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to build" );
                    Log.Close();
                }
            }
        }

        private void GCC_Clean()
        {
            try
            {
                string GameName = "GAMENAME=" + Builder.CommandLine.ToUpper() + "GAME";
                string Config = "BUILDTYPE=" + Builder.GetMakeConfiguration();
                string CommandLine = "/c " + Builder.MakeApplication + " -C Development/Src/PS3 " + GameName + " " + Config + " clean > " + Builder.GetLogFileName( COMMANDS.GCCClean ) + " 2>&1";

                CurrentBuild = new BuildProcess( Parent, Builder, CommandLine, "" );
                ErrorLevel = CurrentBuild.GetErrorLevel();
                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.GCCClean;
            }   
        }

        private void GCC_Build()
        {
            try
            {
                string GameName = "GAMENAME=" + Builder.CommandLine.ToUpper() + "GAME";
                string Config = "BUILDTYPE=" + Builder.GetMakeConfiguration();
                string Defines = "ADD_DEFINES=\"" + Builder.BuildDefine + "\"";

                string CommandLine = "/c " + Builder.MakeApplication + " -C Development/Src/PS3 " + GameName + " " + Config + " " + Defines + " -j 2 > " + Builder.GetLogFileName( COMMANDS.GCCBuild ) + " 2>&1";

                CurrentBuild = new BuildProcess( Parent, Builder, CommandLine, "" );
                ErrorLevel = CurrentBuild.GetErrorLevel();
                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.GCCBuild;
            }
        }

        private void Shader_Clean()
        {
            try
            {
                string LogFileName = Builder.GetLogFileName( COMMANDS.ShaderClean );
                StreamWriter Log = new StreamWriter( LogFileName );

                string ShaderName;
                FileInfo Info;

                GameConfig Config = Builder.CreateGameConfig();

                // Delete ref shader cache
                ShaderName = Config.GetRefShaderName();
                Info = new FileInfo( ShaderName );
                if( Info.Exists )
                {
                    Info.IsReadOnly = false;
                    Info.Delete();
                }

                // Delete local shader cache
                ShaderName = Config.GetLocalShaderName();
                Info = new FileInfo( ShaderName );
                if( Info.Exists )
                {
                    Info.IsReadOnly = false;
                    Info.Delete();
                }

                CleanIniFiles( Config, Log );

                ErrorLevel = ERRORS.None;
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.ShaderClean;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to clean precompiled shaders" );
                    Log.Close();
                }
            }
        }
        
        private void Shader_Build()
        {
            try
            {
                string LogFileName = Builder.GetLogFileName( COMMANDS.ShaderBuild );
                StreamWriter Log = new StreamWriter( LogFileName );

                GameConfig Config = Builder.CreateGameConfig();

                string Executable = Config.GetExeName();
                string CommandLine = "precompileshaders platform=" + Builder.LabelInfo.Platform + " -refcache -ALLOW_PARALLEL_PRECOMPILESHADERS " + CommonCommandLine;

                CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", false, true );
                ErrorLevel = CurrentBuild.GetErrorLevel();

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.ShaderBuild;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to build precompiled shaders" );
                    Log.Close();
                }
            }
        }

        private void Shader_BuildState()
        {
            try
            {
                string LogFileName = Builder.GetLogFileName( COMMANDS.ShaderBuildState );
                StreamWriter Log = new StreamWriter( LogFileName );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length < 2 )
                {
                    Builder.Write( Log, "Error, missing required parameter(s). Usage: ShaderBuild <Game> <Map> [Map...]" );
                    ErrorLevel = ERRORS.ShaderBuild;
                }
                else
                {
                    GameConfig Config = Builder.CreateGameConfig( Parms[0] );

                    string Executable = Config.GetExeName();
                    string CommandLine = "BuildPatchedShaderStates platform=" + Builder.LabelInfo.Platform;

                    for( int i = 1; i < Parms.Length; i++ )
                    {
                        CommandLine += " " + Parms[i];
                    }

                    CommandLine += " " + CommonCommandLine;

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", false, true );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
                }

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.ShaderBuildState;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to build shader states" );
                    Log.Close();
                }
            }
        }

        private void BuildScript()
        {
            try
            {
                string LogFileName = Builder.GetLogFileName( COMMANDS.BuildScript );
                StreamWriter Log = new StreamWriter( LogFileName );

                if( Builder.CommandLine.Length == 0 )
                {
                    Builder.Write( Log, "Error, missing required parameter. Usage: BuildScript <Game>." );
                    ErrorLevel = ERRORS.BuildScript;
                }
                else
                {
                    GameConfig Config = Builder.CreateGameConfig( Builder.CommandLine );

                    string Executable = Config.GetExeName();
                    string CommandLine = "make -full -silentbuild " + CommonCommandLine + " " + Builder.GetScriptConfiguration();

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", false, true );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
                }

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.BuildScript;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to build script" );
                    Log.Close();
                }
            }
        }

        private void DeletLocalShaderCache( string Platform )
        {
            GameConfig Game = Builder.CreateGameConfig( Builder.LabelInfo.Game, Platform );
            Builder.Write( Log, "Deleting: '" + Builder.SourceControlBranch + "/" + Game.GetLocalShaderName() + "'" );
            FileInfo LCSInfo = new FileInfo( Game.GetLocalShaderName() );
            if( LCSInfo.Exists )
            {
                LCSInfo.IsReadOnly = false;
                LCSInfo.Delete();
                Builder.Write( Log, " ... done" );
            }
        }

        private void PreHeat()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.PreHeatMapOven ) );

                if( Builder.CommandLine.Length > 0 )
                {
                    Builder.Write( Log, "Error, too many parameters. Usage: PreheatMapOven." );
                    ErrorLevel = ERRORS.PreHeatMapOven;
                }
                else if( Builder.LabelInfo.Game.Length == 0 )
                {
                    Builder.Write( Log, "Error, no game defined for PreheatMapOven." );
                    ErrorLevel = ERRORS.PreHeatMapOven;
                }
                else
                {
                    // Delete the cooked folder to start from scratch
                    string CookedFolder = Builder.LabelInfo.Game + "Game/Cooked" + Builder.LabelInfo.Platform;
                    Builder.Write( Log, "Deleting: '" + Builder.SourceControlBranch + "/" + CookedFolder + "'" );
                    if( Directory.Exists( CookedFolder ) )
                    {
                        Parent.DeleteDirectory( CookedFolder, 0 );
                        Builder.Write( Log, " ... done" );
                    }

                    // Delete the config folder to start from scratch
                    string ConfigFolder = Builder.LabelInfo.Game + "Game/Config/" + Builder.LabelInfo.Platform + "/Cooked";
                    Builder.Write( Log, "Deleting: '" + Builder.SourceControlBranch + "/" + ConfigFolder + "'" );
                    if( Directory.Exists( ConfigFolder ) )
                    {
                        Parent.DeleteDirectory( ConfigFolder, 0 );
                        Builder.Write( Log, " ... done" );
                    }

                    // Delete the config folder to start from scratch
                    string CoalescedFolder = Builder.LabelInfo.Game + "Game/Localization";
                    Builder.Write( Log, "Deleting loc data from " + CoalescedFolder );
                    DirectoryInfo DirInfo = new DirectoryInfo( CoalescedFolder );
                    if( DirInfo.Exists )
                    {
                        Builder.Write( Log, "Deleting Coalesced.* in: '" + Builder.SourceControlBranch + "/" + CoalescedFolder + "'" );
                        FileInfo[] Files = DirInfo.GetFiles( "Coalesced*" );
                        foreach( FileInfo File in Files )
                        {
                            if( File.Exists )
                            {
                                File.IsReadOnly = false;
                                File.Delete();
                            }
                        }
                    }

                    // Delete the local shader caches
                    DeletLocalShaderCache( Builder.LabelInfo.Platform );
                    if( Builder.LabelInfo.Platform.ToLower() == "pc" )
                    {
                        DeletLocalShaderCache( "PC_SM2" );
                        DeletLocalShaderCache( "PC_SM4" );
                    }

                    Builder.Write( Log, "Deleting ini files" );
                    GameConfig Game = Builder.CreateGameConfig();
                    CleanIniFiles( Game, Log );

                    Builder.ClearPublishDestinations();
                }

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.PreHeatMapOven;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while cleaning cooked data." );
                    Log.Close();
                }
            }
        }

        private void PreHeatJobs()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.PreHeatJobOven ) );

                if( Builder.CommandLine.Length > 0 )
                {
                    Builder.Write( Log, "Error, too many parameters. Usage: PreheatJobOven." );
                    ErrorLevel = ERRORS.PreHeatJobOven;
                }
                else if( Builder.LabelInfo.Game.Length == 0 )
                {
                    Builder.Write( Log, "Error, no game defined for PreheatJobOven." );
                    ErrorLevel = ERRORS.PreHeatJobOven;
                }
                else
                {
                    // Delete the cooked folder to start from scratch
                    GameConfig Config = Builder.CreateGameConfig();

                    Builder.Write( Log, "Deleting: '" + Builder.SourceControlBranch + "/" + Config.GetJobsFolderName() + "'" );
                    if( Directory.Exists( Config.GetJobsFolderName() ) )
                    {
                        Parent.DeleteDirectory( Config.GetJobsFolderName(), 0 );
                        Builder.Write( Log, " ... done" );
                    }
                }

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.PreHeatJobOven;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while cleaning jobs folder." );
                    Log.Close();
                }
            }
        }

        private void Cleanup()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.Cleanup ), true );

                // Delete all cooked data
                string CookedFolder = Builder.LabelInfo.Game + "Game/Cooked" + Builder.LabelInfo.Platform;
                Builder.Write( Log, "Deleting: '" + Builder.SourceControlBranch + "/" + CookedFolder + "'" );

                if( Directory.Exists( CookedFolder ) )
                {
                    Parent.DeleteDirectory( CookedFolder, 0 );
                    Builder.Write( Log, " ... done" );
                }

                // Delete object and other compilation work files
                string IntermediateFolder = "Development/Intermediate";
                Builder.Write( Log, "Deleting: '" + Builder.SourceControlBranch + "/" + IntermediateFolder + "'" );

                if( Directory.Exists( IntermediateFolder ) )
                {
                    Parent.DeleteDirectory( IntermediateFolder, 0 );
                    Builder.Write( Log, " ... done" );
                }

                // Delete the local shader caches
                DeletLocalShaderCache( Builder.LabelInfo.Platform );
                if( Builder.LabelInfo.Platform.ToLower() == "pc" )
                {
                    DeletLocalShaderCache( "PC_SM2" );
                    DeletLocalShaderCache( "PC_SM4" );
                }

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.Cleanup;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, while cleaning up" );
                    Log.Close();
                }
            }
        }

        private void Cook()
        {
            try
            {
                string LogFileName = Builder.GetLogFileName( COMMANDS.CookMaps );
                StreamWriter Log = new StreamWriter( LogFileName );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                GameConfig Config = Builder.CreateGameConfig();

                string Executable = Config.GetExeName();
                string CommandLine = "CookPackages " + GetPlatform();

                for( int i = 0; i < Parms.Length; i++ )
                {
                    CommandLine += " " + Parms[i];
                }

                CommandLine += " -alwaysRecookmaps -alwaysRecookScript -updateInisAuto " + CommonCommandLine;

                string Language = Builder.LabelInfo.Language;
                if( Language.Length == 0 )
                {
                    Language = "INT";
                }

                CommandLine += " -LanguageForCooking=" + Language + Builder.GetCompressionConfiguration() + Builder.GetScriptConfiguration() + Builder.GetCookConfiguration();
                CommandLine += Builder.GetContentPath();

                CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", true, true );
                ErrorLevel = CurrentBuild.GetErrorLevel();

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.CookMaps;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to cook" );
                    Log.Close();
                }
            }
        }

        private void CreateHashes()
        {
            try
            {
                string LogFileName = Builder.GetLogFileName( COMMANDS.CreateHashes );
                StreamWriter Log = new StreamWriter( LogFileName );

                GameConfig Config = Builder.CreateGameConfig();

                string Executable = Config.GetExeName();
                string CommandLine = "CookPackages " + GetPlatform();

                CommandLine += " -sha -updateInisAuto -inisonly " + CommonCommandLine;

                string Language = Builder.LabelInfo.Language;
                if( Language.Length == 0 )
                {
                    Language = "INT";
                }
                CommandLine += " -LanguageForCooking=" + Language + Builder.GetScriptConfiguration();

                CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", true, true );
                ErrorLevel = CurrentBuild.GetErrorLevel();

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.CreateHashes;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to create hashes" );
                    Log.Close();
                }
            }
        }

        private void Wrangle()
        {
            try
            {
                string LogFileName = Builder.GetLogFileName( COMMANDS.Wrangle );
                StreamWriter Log = new StreamWriter( LogFileName );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length < 2 )
                {
                    Builder.Write( Log, "Error, too few parameters. Usage: Wrangle <Game> <Map1> [Map2...]." );
                    ErrorLevel = ERRORS.Wrangle;
                }
                else
                {
                    GameConfig Config = Builder.CreateGameConfig( Parms[0] );

                    Config.DeleteCutdownPackages( Parent );

                    string Executable = Config.GetExeName();
                    string CommandLine = "WrangleContent ";

                    for( int i = 1; i < Parms.Length; i++ )
                    {
                        CommandLine += "-" + Parms[i] + "section ";
                    }

                    CommandLine += "-nosaveunreferenced -removeeditoronly " + CommonCommandLine;

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", true, true );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
                }

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.Wrangle;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to wrangle" );
                    Log.Close();
                }
            }
        }

        private void Publish()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.Publish ) );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length < 1 )
                {
                    Builder.Write( Log, "Error, too few parameters. Usage: Publish <Dest1> [Dest2...]" );
                    ErrorLevel = ERRORS.Publish;
                }
                else
                {
                    string Executable = "Binaries/CookerSync.exe";
                    string CommandLine = Builder.LabelInfo.Game + " -p " + Builder.LabelInfo.Platform;

                    string Language = Builder.LabelInfo.Language;
                    if( Language.Length == 0 )
                    {
                        Language = "INT";
                    }

                    CommandLine += " -r " + Language;
                    CommandLine += " -b " + Builder.SourceControlBranch;
                    CommandLine += " -crc -v" + Builder.GetInstallConfiguration();
                    if( Builder.ForceCopy )
                    {
                        CommandLine += " -f";
                    }

                    for( int i = 0; i < Parms.Length; i++ )
                    {
                        string PublishFolder = Parms[i].Replace( '/', '\\' ) + "\\" + Builder.LabelInfo.GetFolderName( Builder.AppendLanguage );
                        CommandLine += " " + PublishFolder;
                        Builder.AddPublishDestination( PublishFolder );
                    }

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, Environment.CurrentDirectory + "/Binaries", false, false );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
                }

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.Publish;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to publish" );
                    Log.Close();
                }
            }
        }

        private void GetCookedBuild()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.GetCookedBuild ) );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length < 1 )
                {
                    Builder.Write( Log, "Error, too few parameters. Usage: GetCookedBuild <Source>" );
                    ErrorLevel = ERRORS.GetCookedBuild;
                }
                else
                {
                    string SourceFolder = Builder.LabelInfo.GetFolderName( true );
                    string Path = Parms[0] + "/" + SourceFolder + "/" + Builder.SourceControlBranch + "/Binaries";
                    string Executable = Environment.CurrentDirectory + "/Binaries/CookerSync.exe";
                    string CommandLine = Builder.LabelInfo.Game + " -p " + Builder.LabelInfo.Platform;
                    CommandLine += " -b " + Builder.SourceControlBranch;
                    CommandLine += " -crc -v " + Parent.RootFolder;

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, Path, false, false );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
                }
                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.GetCookedBuild;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to get a cooked build" );
                    Log.Close();
                }
            }
        }

        private void GetInstallableBuild()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.GetInstallableBuild ) );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length < 1 )
                {
                    Builder.Write( Log, "Error, too few parameters. Usage: GetInstallableBuild <Source>" );
                    ErrorLevel = ERRORS.GetInstallableBuild;
                }
                else
                {
                    string PublishedFolder = Builder.LabelInfo.GetFolderName( true );
                    string DestFolder = Builder.LabelInfo.GetFolderName( false );

                    string Path = Parms[0] + "/" + PublishedFolder + "/" + Builder.SourceControlBranch + "/Binaries";
                    Parent.Log( "Source folder: " + Path, Color.DarkGreen );
                    string Executable = Environment.CurrentDirectory + "/Binaries/CookerSync.exe";
                    string CommandLine = Builder.LabelInfo.Game + " -p " + Builder.LabelInfo.Platform;
                    CommandLine += " -b " + Builder.SourceControlBranch;
                    CommandLine += " -crc -v" + Builder.GetInstallConfiguration() + " C:\\Builds\\" + DestFolder;

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, Path, false, false );
                    ErrorLevel = CurrentBuild.GetErrorLevel();

                    Builder.CopyDestination = "C:\\Builds\\" + PublishedFolder;
                }
                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.GetInstallableBuild;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to get an installable build" );
                    Log.Close();
                }
            }
        }

        private void BuildInstaller()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.BuildInstaller ) );

                // Make sure we have the latest IS project files
                SCC.SyncBuildScripts( Builder.SourceControlBranch, "/Development/Install/..." );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length < 1 )
                {
                    Builder.Write( Log, "Error, too few parameters. Usage: BuildInstaller <Package>" );
                    ErrorLevel = ERRORS.BuildInstaller;
                }
                else
                {
                    string PublishedFolder = Builder.LabelInfo.GetFolderName( true );

                    string Executable = Builder.ISDevLocation;
                    string CommandLine = "-p Development/Install/" + Parms[0] + "/" + Parms[0] + ".ism";

                    CommandLine += " -l PATH_TO_UNREALENGINE3_FILES=\"C:\\Builds\\" + PublishedFolder + "\\" + Builder.SourceControlBranch + "\"";

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", false, false );
                    ErrorLevel = CurrentBuild.GetErrorLevel();

                    // Force any future copies to go over the install folder
                    Builder.CopyDestination = "C:\\Install";
                }
                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.BuildInstaller;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to building the installer" );
                    Log.Close();
                }
            }
        }

        private void CopyInstaller()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.CopyInstaller ) );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length < 1 )
                {
                    Builder.Write( Log, "Error, too few parameters. Usage: BuildInstaller <Dest>" );
                    ErrorLevel = ERRORS.CopyInstaller;
                }
                else
                {
                    string DestFolder = Parms[0] + "/" + Builder.Dependency + "_Install";
                    string Executable = "Binaries/ISCopyFiles.exe";
                    string CommandLine = "C:/Install " + DestFolder;

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", false, false );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
                }
                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.CopyInstaller;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to copy the installer" );
                    Log.Close();
                }
            }
        }

        private void Conform()
        {
            try
            {
                string Package, LastPackage;

                string LastLanguage = Builder.GetValidLanguages().Dequeue();
                string Language = Builder.GetValidLanguages().Peek();

                string LogFileName = Builder.GetLogFileName( COMMANDS.Conform, Language );
                StreamWriter Log = new StreamWriter( LogFileName );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length != 1 )
                {
                    Builder.Write( Log, "Error, missing package name. Usage: Conform <Package>." );
                    ErrorLevel = ERRORS.Conform;
                }
                else
                {
                    GameConfig Config = Builder.CreateGameConfig();
                    string Executable = Config.GetExeName();

                    if( LastLanguage == "INT" )
                    {
                        LastPackage = Parms[0];
                    }
                    else
                    {
                        LastPackage = Parms[0] + "_" + LastLanguage;
                    }
                    Package = Parms[0] + "_" + Language;

                    string CommandLine = "conform " + Package + " " + LastPackage + " " + CommonCommandLine;

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", false, true );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
                }

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.Conform;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to conform dialog" );
                    Log.Close();
                }
            }
        }

        private void CrossBuildConform()
        {
            try
            {
                string Package, LastPackage;

                string LogFileName = Builder.GetLogFileName( COMMANDS.CrossBuildConform, Builder.LabelInfo.Language );
                StreamWriter Log = new StreamWriter( LogFileName );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length != 1 )
                {
                    Builder.Write( Log, "Error, missing package name. Usage: CrossBuildConform <Package>." );
                    ErrorLevel = ERRORS.CrossBuildConform;
                }
                else
                {
                    GameConfig Config = Builder.CreateGameConfig();
                    string Executable = Config.GetExeName();

                    Package = Parms[0] + "_" + Builder.LabelInfo.Language + ".upk";
                    LastPackage = Builder.SourceBuild + "/" + Builder.SourceControlBranch + "/" + Package;

                    string CommandLine = "conform " + Package + " " + LastPackage + " " + CommonCommandLine;

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", false, true );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
                }

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.CrossBuildConform;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting cross build conform." );
                    Log.Close();
                }
            }
        }

        private void BumpEngineCpp( ScriptParser Builder, List<string> Lines )
        {
            // Bump ENGINE_VERSION and BUILT_FROM_CHANGELIST
            int BumpIncrement = 1;
            if( Builder.CommandLine.Length > 0 )
            {
                BumpIncrement = Builder.SafeStringToInt( Builder.CommandLine );
            }

            for( int i = 0; i < Lines.Count; i++ )
            {
                string[] Parms = Lines[i].Split( " \t".ToCharArray() );

                if( Parms.Length == 3 && Parms[0].ToUpper() == "#DEFINE" )
                {
                    if( Parms[1].ToUpper() == "MAJOR_VERSION" )
                    {
                        Builder.LabelInfo.BuildVersion = new Version( Builder.LabelInfo.SafeStringToInt( Parms[2] ),
                                                                        Builder.LabelInfo.BuildVersion.Minor,
                                                                        Builder.LabelInfo.BuildVersion.Build,
                                                                        Builder.LabelInfo.BuildVersion.Revision );
                    }

                    if( Parms[1].ToUpper() == "MINOR_VERSION" )
                    {
                        Builder.LabelInfo.BuildVersion = new Version( Builder.LabelInfo.BuildVersion.Major,
                                                                        Builder.LabelInfo.SafeStringToInt( Parms[2] ),
                                                                        Builder.LabelInfo.BuildVersion.Build,
                                                                        Builder.LabelInfo.BuildVersion.Revision );
                    }

                    if( Parms[1].ToUpper() == "ENGINE_VERSION" )
                    {
                        Builder.LabelInfo.BuildVersion = new Version( Builder.LabelInfo.BuildVersion.Major,
                                                                        Builder.LabelInfo.BuildVersion.Minor,
                                                                        Builder.LabelInfo.SafeStringToInt( Parms[2] ) + BumpIncrement,
                                                                        Builder.LabelInfo.BuildVersion.Revision );

                        Lines[i] = "#define\tENGINE_VERSION\t" + Builder.LabelInfo.BuildVersion.Build.ToString();
                    }

                    if( Parms[1].ToUpper() == "PRIVATE_VERSION" )
                    {
                        Builder.LabelInfo.BuildVersion = new Version( Builder.LabelInfo.BuildVersion.Major,
                                                                        Builder.LabelInfo.BuildVersion.Minor,
                                                                        Builder.LabelInfo.BuildVersion.Build,
                                                                        Builder.LabelInfo.SafeStringToInt( Parms[2] ) );
                    }

                    if( Parms[1].ToUpper() == "BUILT_FROM_CHANGELIST" )
                    {
                        Lines[i] = "#define\tBUILT_FROM_CHANGELIST\t" + Builder.LabelInfo.Changelist.ToString();
                    }
                }
            }
        }

        private string GetHexVersion( int EngineVersion )
        {
            string HexVersion = "";
            char[] HexDigits = "0123456789abcdef".ToCharArray();

            int MajorVer = Builder.LabelInfo.BuildVersion.Major;
            int MinorVer = Builder.LabelInfo.BuildVersion.Minor;

            // First 4 bits is major version
            HexVersion += HexDigits[MajorVer & 0xf];
            // Next 4 bits is minor version
            HexVersion += HexDigits[MinorVer & 0xf];
            // Next 16 bits is build number
            HexVersion += HexDigits[( EngineVersion >> 12 ) & 0xf];
            HexVersion += HexDigits[( EngineVersion >> 8 ) & 0xf];
            HexVersion += HexDigits[( EngineVersion >> 4 ) & 0xf];
            HexVersion += HexDigits[( EngineVersion >> 0 ) & 0xf];
            // Client code is required to have the first 4 bits be 0x8, where server code is required to have the first 4 bits be 0xC.
            HexVersion += HexDigits[0x8];
            // DiscID varies for different languages
            HexVersion += HexDigits[0x1];

            return ( HexVersion );
        }

        private void BumpEngineXml( ScriptParser Builder, List<string> Lines )
        {
            // Bump build version in Live! stuff
            for( int i = 0; i < Lines.Count; i++ )
            {
                if( Lines[i].Trim().StartsWith( "build=" ) )
                {
                    Lines[i] = "     build=\"" + Builder.LabelInfo.BuildVersion.Build.ToString() + "\"";
                }
                else if( Lines[i].Trim().StartsWith( "<titleversion>" ) )
                {
                    Lines[i] = "  <titleversion>" + GetHexVersion( Builder.LabelInfo.BuildVersion.Build ) + "</titleversion>";
                }
                else if( Lines[i].Trim().StartsWith( "<VersionNumber versionNumber=" ) )
                {
                    Lines[i] = "      <VersionNumber versionNumber=\"" + Builder.LabelInfo.BuildVersion.ToString() + "\" />";
                }
            }
        }

        private void BumpEngineHeader( ScriptParser Builder, List<string> Lines )
        {
            // Bump build version in Live! stuff
            for( int i = 0; i < Lines.Count; i++ )
            {
                if( Lines[i].Trim().StartsWith( "#define BUILD_NUM" ) )
                {
                    Lines[i] = "#define BUILD_NUM " + Builder.LabelInfo.BuildVersion.Build.ToString();
                }
                else if( Lines[i].Trim().StartsWith( "#define VER_STRING" ) )
                {
                    Lines[i] = "#define VER_STRING \"" + Builder.LabelInfo.BuildVersion.ToString() + "\"";
                }
            }
        }

        private void BumpEngineProperties( ScriptParser Builder, List<string> Lines, string TimeStamp, int ChangeList )
        {
            // Bump build version in properties file
            for( int i = 0; i < Lines.Count; i++ )
            {
                if( Lines[i].Trim().StartsWith( "timestampForBVT=" ) )
                {
                    Lines[i] = "timestampForBVT=" + TimeStamp;
                }
                else if( Lines[i].Trim().StartsWith( "changelistBuiltFrom=" ) )
                {
                    Lines[i] = "changelistBuiltFrom=" + ChangeList.ToString();
                }
            }
        }

        private void BumpVersionFile( ScriptParser Builder, string File, bool GetVersion )
        {
            List<string> Lines = new List<string>();
            string Line;

            // Check to see if the version file is writable (otherwise the StreamWriter creation will exception)
            FileInfo Info = new FileInfo( File );
            if( Info.IsReadOnly )
            {
                ErrorLevel = ERRORS.BumpEngineVersion;
                Builder.Write( Log, "Error, version file is read only '" + File + "'" );
                return;
            }

            // Read in existing file
            StreamReader Reader = new StreamReader( File );
            if( Reader == null )
            {
                ErrorLevel = ERRORS.BumpEngineVersion;
                Builder.Write( Log, "Error, failed to open for reading '" + File + "'" );
                return;
            }

            Line = Reader.ReadLine();
            while( Line != null )
            {
                Lines.Add( Line );

                Line = Reader.ReadLine();
            }

            Reader.Close();

            // Bump the version dependent on the file extension
            if( GetVersion && Info.Extension.ToLower() == ".cpp" )
            {
                BumpEngineCpp( Builder, Lines );
            }
            else
            {
                if( Info.Extension.ToLower() == ".xml" )
                {
                    BumpEngineXml( Builder, Lines );
                }
                else if( Info.Extension.ToLower() == ".properties" )
                {
                    BumpEngineProperties( Builder, Lines, Builder.GetTimeStamp(), Builder.LabelInfo.Changelist );
                }
                else if( Info.Extension.ToLower() == ".h" )
                {
                    BumpEngineHeader( Builder, Lines );
                }
                else
                {
                    ErrorLevel = ERRORS.BumpEngineVersion;
                    Builder.Write( Log, "Error, invalid extension for '" + File + "'" );
                    return;
                }
            }

            // Write out version
            StreamWriter Writer;
            if( File.ToLower().IndexOf( ".gdf.xml" ) >= 0 )
            {
                Writer = new StreamWriter( File, false, Encoding.Unicode );
            }
            else
            {
                Writer = new StreamWriter( File );
            }

            if( Writer == null )
            {
                ErrorLevel = ERRORS.BumpEngineVersion;
                Builder.Write( Log, "Error, failed to open for writing '" + File + "'" );
                return;
            }

            foreach( string SingleLine in Lines )
            {
                Writer.Write( SingleLine + "\r\n" );
            }

            Writer.Close();
        }

        private void BumpEngineVersion()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.BumpEngineVersion ) );

                string File = Builder.EngineVersionFile;
                BumpVersionFile( Builder, File, true );

                string[] Files = Builder.MiscVersionFiles.Split( ";".ToCharArray() );
                foreach( string XmlFile in Files )
                {
                    BumpVersionFile( Builder, XmlFile, false );
                }

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.BumpEngineVersion;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, while bumping engine version in '" + Builder.EngineVersionFile + "'" );
                    Log.Close();
                }
            }
        }

        private int GetVersionFile( ScriptParser Builder, string File )
        {
            List<string> Lines = new List<string>();
            string Line;
            int NewEngineVersion = 0;

            // Check to see if the version file is writable (otherwise the StreamWriter creation will exception)
            FileInfo Info = new FileInfo( File );

            // Read in existing file
            StreamReader Reader = new StreamReader( File );
            if( Reader == null )
            {
                ErrorLevel = ERRORS.GetEngineVersion;
                Builder.Write( Log, "Error, failed to open for reading '" + File + "'" );
                return ( 0 );
            }

            Line = Reader.ReadLine();
            while( Line != null )
            {
                Lines.Add( Line );

                Line = Reader.ReadLine();
            }

            Reader.Close();

            for( int i = 0; i < Lines.Count; i++ )
            {
                string[] Parms = Lines[i].Split( " \t".ToCharArray() );

                if( Parms.Length == 3 && Parms[0].ToUpper() == "#DEFINE" )
                {
                    if( Parms[1].ToUpper() == "ENGINE_VERSION" )
                    {
                        NewEngineVersion = Builder.SafeStringToInt( Parms[2] );
                    }
                }
            }

            return ( NewEngineVersion );
        }

        private void GetEngineVersion()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.GetEngineVersion ) );

                int EngineVersion = GetVersionFile( Builder, Builder.EngineVersionFile );
                Builder.LabelInfo.BuildVersion = new Version( Builder.LabelInfo.BuildVersion.Major,
                                                                Builder.LabelInfo.BuildVersion.Minor,
                                                                EngineVersion,
                                                                Builder.LabelInfo.BuildVersion.Revision );

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.BumpEngineVersion;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, while getting engine version in '" + Builder.EngineVersionFile + "'" );
                    Log.Close();
                }
            }
        }

        private void UpdateGDFVersion()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.UpdateGDFVersion ) );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length != 2 )
                {
                    Builder.Write( Log, "Error, too few parameters. Usage: UpdateGDFVersion <Game> <ResourcePath>." );
                    ErrorLevel = ERRORS.UpdateGDFVersion;
                }
                else
                {
                    int EngineVersion = Builder.LabelInfo.BuildVersion.Build;
                    Queue<string> Languages = Builder.GetLanguages();

                    foreach( string Lang in Languages )
                    {
                        string GDFFileName = Parms[1] + "/" + Lang.ToUpper() + "/" + Parms[0] + "Game.gdf.xml";
                        BumpVersionFile( Builder, GDFFileName, false );
                    }
                }

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.BumpEngineVersion;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, while bumping engine version in '" + Builder.EngineVersionFile + "'" );
                    Log.Close();
                }
            }
        }

        private void UpdateSourceServer()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.UpdateSourceServer ) );

                string Executable = "Cmd.exe";
                string CommandLine = "/c \"" + Builder.SourceServerCmd + "\" " + Environment.CurrentDirectory + " " + Environment.CurrentDirectory + "\\Binaries";
                CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", true, false );

                ErrorLevel = CurrentBuild.GetErrorLevel();
                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.UpdateSourceServer;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, while updating source server." );
                    Log.Close();
                }
            }
        }

        private void DeleteIndexFile( StreamWriter Log, string File )
        {
            FileInfo Info = new FileInfo( File );
            if( Info.Exists )
            {
                Info.IsReadOnly = false;
                Info.Delete();
                Builder.Write( Log, "Deleted: " + File );
            }
        }

        private void CreateCommandList( string Title, string Exec, string SymbolFile )
        {
            string Executable = "";
            string ExePath = "";
            string SymPath = "";

            string SymStore = Builder.SymbolStoreLocation;
            string ChangeList = Builder.LabelInfo.Changelist.ToString();
            string Version = Builder.LabelInfo.BuildVersion.Build.ToString();

            Builder.AddUpdateSymStore( "delete" );
            Builder.AddUpdateSymStore( "status " + Exec );

            // Symstore requires \ for some reason
            Executable = Environment.CurrentDirectory + "\\" + Exec.Replace( '/', '\\' );
            ExePath = Executable.Substring( 0, Executable.LastIndexOf( '\\' ) );

            SymbolFile = Environment.CurrentDirectory + "\\" + SymbolFile.Replace( '/', '\\' );
            SymPath = SymbolFile.Substring( 0, SymbolFile.LastIndexOf( '\\' ) );

            Builder.AddUpdateSymStore( "add /g " + ExePath + " /p /l /f " + Executable + " /x exe_index.txt /a /o" );
            Builder.AddUpdateSymStore( "add /g " + SymPath + " /p /l /f " + SymbolFile + " /x pdb_index.txt /a /o" );
            Builder.AddUpdateSymStore( "add /g " + ExePath + " /y exe_index.txt /l /s " + SymStore + " /t " + Title + " /v " + ChangeList + " /c " + Version + " /o /compress" );
            Builder.AddUpdateSymStore( "add /g " + SymPath + " /y pdb_index.txt /l /s " + SymStore + " /t " + Title + " /v " + ChangeList + " /c " + Version + " /o /compress" );
        }

        private void UpdateSymbolServer()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.UpdateSymbolServer ) );

                if( Builder.LabelInfo.BuildVersion.Build == 0 )
                {
                    Builder.Write( Log, "Error, while updating symbol server." );
                    Builder.Write( Log, "Error, no engine version found; has BumpEngineVersion been called?" );
                    ErrorLevel = ERRORS.UpdateSymbolServer;
                }
                else
                {
                    List<GameConfig> GameConfigs = Builder.LabelInfo.GetGameConfigs();

                    foreach( GameConfig Config in GameConfigs )
                    {
                        string Title = Config.GetTitle();
                        string[] Executables = Config.GetExecutableNames();
                        string SymbolFile = Config.GetSymbolFileName();

                        if( Executables[0].Length > 0 && SymbolFile.Length > 0 )
                        {
                            CreateCommandList( Title, Executables[0], SymbolFile );
                        }
                    }

                    // Clean up the files that may have permissions problems
                    Builder.AddUpdateSymStore( "delete" );
                }

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.UpdateSymbolServer;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, while updating symbol server." );
                    Log.Close();
                }
            }
        }

        private void UpdateSymbolServerTick()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.UpdateSymbolServerTick ), true );

                if( Builder.UpdateSymStoreEmpty() )
                {
                    // Force a finish
                    CurrentBuild = null;
                    Builder.Write( Log, "Finished!" );
                    Log.Close();
                }
                else
                {
                    string Executable = Builder.SymbolStoreApp;
                    string CommandLine = Builder.PopUpdateSymStore();

                    if( CommandLine == "delete" )
                    {
                        // Delete the index files
                        DeleteIndexFile( Log, "exe_index.txt" );
                        DeleteIndexFile( Log, "pdb_index.txt" );

                        CommandLine = Builder.PopUpdateSymStore();
                    }

                    if( CommandLine.StartsWith( "status " ) )
                    {
                        Parent.Log( "[STATUS] Updating symbol server for '" + CommandLine.Substring( "status ".Length ) + "'", Color.Magenta );

                        CommandLine = Builder.PopUpdateSymStore();
                    }

                    if( Builder.UpdateSymStoreEmpty() )
                    {
                        // Force a finish
                        CurrentBuild = null;
                        Builder.Write( Log, "Finished!" );
                        Log.Close();
                        return;
                    }

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", true, false );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
                }

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.UpdateSymbolServer;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, while updating symbol server." );
                    Log.Close();
                }
            }
        }

        private void CheckSigned()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.CheckSigned ), true );

                Parent.Log( "[STATUS] Checking '" + Builder.CommandLine + "' for signature", Color.Magenta );

                string SignToolName = Builder.SignToolName;
                string CommandLine = "verify /pa /v " + Builder.CommandLine;

                CurrentBuild = new BuildProcess( Parent, Builder, Log, SignToolName, CommandLine, "", true, false );
                ErrorLevel = ERRORS.CheckSigned;
                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.CheckSigned;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, while checking for signed binaries." );
                    Log.Close();
                }
            }
        }

        private void Sign()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.Sign ), true );

                Parent.Log( "[STATUS] Signing '" + Builder.CommandLine + "'", Color.Magenta );

                string SignToolName = Builder.SignToolName;
                string CommandLine = "sign /f Development/Builder/Auth/EpicGames.pfx /v " + Builder.CommandLine;

                CurrentBuild = new BuildProcess( Parent, Builder, Log, SignToolName, CommandLine, "", true, false );
                ErrorLevel = CurrentBuild.GetErrorLevel();
                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.Sign;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, while signing binaries." );
                    Log.Close();
                }
            }
        }

        private void SimpleCopy()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SimpleCopy ), true );

                string PathName = Builder.CommandLine.Trim();
                Builder.Write( Log, "Copying: " + PathName );
                Builder.Write( Log, " ... to: " + Builder.CopyDestination );

                FileInfo Source = new FileInfo( PathName );
                if( Source.Exists )
                {
                    // Get the filename
                    int FileNameOffset = PathName.LastIndexOf( '/' );
                    string FileName = PathName.Substring( FileNameOffset + 1 );
                    string DestPathName = Builder.CopyDestination + "/" + FileName;

                    // Create the dest folder if it doesn't exist
                    DirectoryInfo DestDir = new DirectoryInfo( Builder.CopyDestination );
                    if( !DestDir.Exists )
                    {
                        DestDir.Create();
                    }
                    
                    // Copy the file
                    FileInfo Dest = new FileInfo( DestPathName );
                    if( Dest.Exists )
                    {
                        Dest.IsReadOnly = false;
                        Dest.Delete();
                    }
                    Source.CopyTo( DestPathName, true );
                }
                else
                {
                    Builder.Write( Log, "Error, source file does not exist for copying" );
                    ErrorLevel = ERRORS.SimpleCopy;
                }

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SimpleCopy;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, while copying file" );
                    Log.Close();
                }
            }
        }

        private void SourceBuildCopy()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SourceBuildCopy ), true );

                string DestFile = Builder.CommandLine.Trim();
                string SourceFile = Builder.SourceBuild + "/" + Builder.SourceControlBranch + "/" + DestFile;

                Builder.Write( Log, "Copying: " );
                Builder.Write( Log, " ... from: " + SourceFile );
                Builder.Write( Log, " ... to: " + DestFile );

                FileInfo Source = new FileInfo( SourceFile );
                if( Source.Exists )
                {
                    FileInfo Dest = new FileInfo( DestFile );
                    if( Dest.Exists && Dest.IsReadOnly )
                    {
                        Dest.IsReadOnly = false;
                    }

                    // Create the dest folder if it doesn't exist
                    DirectoryInfo DestDir = new DirectoryInfo( Dest.DirectoryName );
                    if( !DestDir.Exists )
                    {
                        DestDir.Create();
                    }

                    Source.CopyTo( DestFile, true );
                }
                else
                {
                    Builder.Write( Log, "Error, source file does not exist for copying" );
                    ErrorLevel = ERRORS.SourceBuildCopy;
                }

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SourceBuildCopy;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, while copying file from alternate build" );
                    Log.Close();
                }
            }
        }

        private void SimpleDelete()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SimpleDelete ), true );

                string PathName = Builder.CommandLine.Trim();

                FileInfo Source = new FileInfo( PathName );
                if( Source.Exists )
                {
                    Source.Delete();
                }
                else
                {
                    Builder.Write( Log, "Error, source file does not exist for deletion" );
                    ErrorLevel = ERRORS.SimpleDelete;
                }

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SimpleDelete;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, while deleting file" );
                    Log.Close();
                }
            }
        }

        private void SimpleRename()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SimpleRename ), true );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length != 2 )
                {
                    Builder.Write( Log, "Error, while renaming file" );
                    ErrorLevel = ERRORS.SimpleRename;
                }
                else
                {
                    string BaseFolder = "";
                    if( Builder.CopyDestination.Length > 0 )
                    {
                        BaseFolder = Builder.CopyDestination + "/" + Builder.SourceControlBranch + "/";
                    }

                    FileInfo Source = new FileInfo( BaseFolder + Parms[0] );
                    if( Source.Exists )
                    {
                        FileInfo Dest = new FileInfo( BaseFolder + Parms[1] );
                        if( Dest.Exists )
                        {
                            Dest.IsReadOnly = false;
                            Dest.Delete();
                        }
                        Source.IsReadOnly = false;

                        Source.CopyTo( Dest.FullName );
                        Source.Delete();
                    }
                    else
                    {
                        Builder.Write( Log, "Error, source file does not exist for renaming" );
                        ErrorLevel = ERRORS.SimpleRename;
                    }
                }

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SimpleRename;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, while renaming file" );
                    Log.Close();
                }
            }
        }

        public MODES Execute( COMMANDS CommandID )
        {
            switch( CommandID )
            {
                case COMMANDS.SCC_CheckConsistency:
                    SCC_CheckConsistency();
                    return ( MODES.Finalise );
                
                case COMMANDS.SCC_Sync:
                    SCC_Sync();
                    return ( MODES.Finalise );
                    
                case COMMANDS.SCC_SyncSingleChangeList:
                    SCC_SyncSingleChangeList();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_Checkout:
                    SCC_Checkout();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_OpenForDelete:
                    SCC_OpenForDelete();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_CheckoutGame:
                    SCC_CheckoutGame();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_CheckoutShader:
                    SCC_CheckoutShader();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_CheckoutDialog:
                    SCC_CheckoutDialog();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_CheckoutFonts:
                    SCC_CheckoutFonts();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_CheckoutLocPackage:
                    SCC_CheckoutLocPackage();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_CheckoutGDF:
                    SCC_CheckoutGDF();
                    return ( MODES.Finalise );
                
                case COMMANDS.MakeWritable:
                    MakeWritable();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_Submit:
                    SCC_Submit();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_CreateNewLabel:
                    SCC_CreateNewLabel();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_UpdateLabelDescription:
                    SCC_UpdateLabelDescription();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_Revert:
                    SCC_Revert();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_RevertFile:
                    SCC_RevertFile();
                    return ( MODES.Finalise );

                case COMMANDS.MakeReadOnly:
                    MakeReadOnly();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_Tag:
                    SCC_Tag();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_TagFile:
                    SCC_TagFile();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_TagPCS:
                    SCC_TagPCS();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_TagExe:
                    SCC_TagExe();
                    return ( MODES.Finalise );

                case COMMANDS.CreatePackageJobs:
                    CreatePackageJobs();
                    return ( MODES.Monitor );

                case COMMANDS.LoadJobs:
                    LoadJobs();
                    return ( MODES.Finalise );

                case COMMANDS.MergeJobs:
                    MergeJobs();
                    return ( MODES.Monitor );

                case COMMANDS.CopyJobs:
                    CopyJobs();
                    return ( MODES.Monitor );
                    
                case COMMANDS.GetJobs:
                    GetJobs();
                    return ( MODES.Monitor );

                case COMMANDS.CleanJobs:
                    CleanJobs();
                    return ( MODES.Finalise );
                                
                case COMMANDS.AddJob:
                    AddJob();
                    return ( MODES.Finalise );

                case COMMANDS.AddGCCJob:
                    AddGCCJob();
                    return ( MODES.Finalise );

                case COMMANDS.AddMSVCJob:
                    AddMSVCJob();
                    return ( MODES.Finalise );

                case COMMANDS.CookPackageJob:
                    CookPackageJob();
                    return ( MODES.Monitor );

                case COMMANDS.CookMapJob:
                    CookMapJob();
                    return ( MODES.Monitor );

                case COMMANDS.MSVCClean:
                    MSVC_Clean();
                    return ( MODES.Monitor );

                case COMMANDS.MSVCBuild:
                    MSVC_Build();
                    return ( MODES.Monitor );

                case COMMANDS.GCCClean:
                    GCC_Clean();
                    return ( MODES.Monitor );

                case COMMANDS.GCCBuild:
                    GCC_Build();
                    return ( MODES.Monitor );

                case COMMANDS.ShaderClean:
                    Shader_Clean();
                    return ( MODES.Finalise );
                
                case COMMANDS.ShaderBuild:
                    Shader_Build();
                    return ( MODES.Monitor );

                case COMMANDS.ShaderBuildState:
                    Shader_BuildState();
                    return ( MODES.Monitor );
                
                case COMMANDS.BuildScript:
                    BuildScript();
                    return ( MODES.Monitor );

                case COMMANDS.PreHeatMapOven:
                    PreHeat();
                    return ( MODES.Finalise );

                case COMMANDS.PreHeatJobOven:
                    PreHeatJobs();
                    return ( MODES.Finalise );

                case COMMANDS.CookMaps:
                    Cook();
                    return ( MODES.Monitor );

                case COMMANDS.CreateHashes:
                    CreateHashes();
                    return ( MODES.Monitor );

                case COMMANDS.Wrangle:
                    Wrangle();
                    return ( MODES.Monitor );

                case COMMANDS.Publish:
                    Publish();
                    return ( MODES.Monitor );

                case COMMANDS.GetCookedBuild:
                    GetCookedBuild();
                    return ( MODES.Monitor );

                case COMMANDS.GetInstallableBuild:
                    GetInstallableBuild();
                    return ( MODES.Monitor );

                case COMMANDS.BuildInstaller:
                    BuildInstaller();
                    return ( MODES.Monitor );

                case COMMANDS.CopyInstaller:
                    CopyInstaller();
                    return ( MODES.Monitor );

                case COMMANDS.Conform:
                    Conform();
                    return ( MODES.Monitor );

                case COMMANDS.CrossBuildConform:
                    CrossBuildConform();
                    return ( MODES.Monitor );

                case COMMANDS.BumpEngineVersion:
                    BumpEngineVersion();
                    return ( MODES.Finalise );

                case COMMANDS.GetEngineVersion:
                    GetEngineVersion();
                    return ( MODES.Finalise );

                case COMMANDS.UpdateGDFVersion:
                    UpdateGDFVersion();
                    return ( MODES.Finalise );

                case COMMANDS.UpdateSourceServer:
                    UpdateSourceServer();
                    return ( MODES.Monitor );

                case COMMANDS.UpdateSymbolServer:
                    UpdateSymbolServer();
                    return ( MODES.Finalise );

                case COMMANDS.UpdateSymbolServerTick:
                    UpdateSymbolServerTick();
                    return ( MODES.Monitor );

                case COMMANDS.CheckSigned:
                    CheckSigned();
                    return ( MODES.Monitor );
                
                case COMMANDS.Sign:
                    Sign();
                    return ( MODES.Monitor );

                case COMMANDS.Cleanup:
                    Cleanup();
                    return ( MODES.Finalise );

                case COMMANDS.SimpleCopy:
                    SimpleCopy();
                    return ( MODES.Finalise );

                case COMMANDS.SourceBuildCopy:
                    SourceBuildCopy();
                    return ( MODES.Finalise );

                case COMMANDS.SimpleDelete:
                    SimpleDelete();
                    return ( MODES.Finalise );

                case COMMANDS.SimpleRename:
                    SimpleRename();
                    return ( MODES.Finalise );
            }

            return( MODES.Init );
        }

        public MODES IsFinished()
        {
            // Also check for timeout
            if( CurrentBuild != null )
            {
                if( CurrentBuild.IsFinished )
                {
                    CurrentBuild.Cleanup();
                    return ( MODES.Finalise );
                }

                if( DateTime.Now - StartTime > Builder.GetTimeout() )
                {
                    CurrentBuild.Kill();
                    ErrorLevel = ERRORS.TimedOut;
                    return ( MODES.Finalise );
                }

                if( !CurrentBuild.IsResponding() )
                {
                    if( DateTime.Now - LastRespondingTime > Builder.GetRespondingTimeout() )
                    {
                        CurrentBuild.Kill();
                        ErrorLevel = ERRORS.Crashed;
                        return ( MODES.Finalise );
                    }
                }
                else
                {
                    LastRespondingTime = DateTime.Now;
                }

                return ( MODES.Monitor );
            }

            // No running build? Something went wrong
            return ( MODES.Finalise );
        }

        public void Kill()
        {
            if( CurrentBuild != null )
            {
                CurrentBuild.Kill();
            }
        }
    }
}
