using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Management;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Windows;
using System.Windows.Forms;

namespace Controller
{
    enum MODES
    {
        Init,
        Monitor,
        Wait,
        WaitForJobs,
        Finalise,
        Exit,
    }

    enum COMMANDS
    {
        Error,
        Config,
        TriggerMail,
        Cleanup,
        SCC_CheckConsistency,
        SCC_Sync,
        SCC_SyncSingleChangeList,
        SCC_Checkout,
        SCC_OpenForDelete,
        SCC_CheckoutGame,
        SCC_CheckoutShader,
        SCC_CheckoutDialog,
        SCC_CheckoutFonts,
        SCC_CheckoutLocPackage,
        SCC_CheckoutGDF,
        SCC_Submit,
        SCC_CreateNewLabel,
        SCC_UpdateLabelDescription,
        SCC_Revert,
        SCC_RevertFile,
        SCC_Tag,
        SCC_TagFile,
        SCC_TagPCS,
        SCC_TagExe,
        CreatePackageJobs,
        LoadJobs,
        MergeJobs,
        CleanJobs,
        CopyJobs,
        GetJobs,
        WaitForJobs,
        AddJob,
        AddGCCJob,
        AddMSVCJob,
        CookPackageJob,
        CookMapJob,
        SetDependency,
        MakeWritable,
        MakeReadOnly,
        MSVCClean,
        MSVCBuild,
        GCCClean,
        GCCBuild,
        ShaderClean,
        ShaderBuild,
        ShaderBuildState,
        BuildScript,
        PreHeatMapOven,
        PreHeatJobOven,
        CookMaps,
        CreateHashes,
        Wrangle,
        Publish,
        GetCookedBuild,
        GetInstallableBuild,
        BuildInstaller,
        CopyInstaller,
        Conform,
        CrossBuildConform,
        Finished,
        Trigger,
        BumpEngineVersion,
        GetEngineVersion,
        UpdateGDFVersion,
        UpdateSourceServer,
        UpdateSymbolServer,
        UpdateSymbolServerTick,
        CheckSigned,
        Sign,
        SimpleCopy,
        SourceBuildCopy,
        SimpleDelete,
        SimpleRename,
        Wait,
        CheckSpace,
        UpdateLabel,
        UpdateFolder,

        MSVCFull,       // Composite command - clean then build
        GCCFull,        // Composite command - clean then build
        ShaderFull,     // Composite command - clean then build
    }

    enum ERRORS
    {
        None,
        NoScript,
        IllegalCommand,
        TimedOut,
        WaitTimedOut,
        FailedJobs,
        Crashed,
        Cleanup,
        SCC_CheckConsistency,
        SCC_Sync,
        SCC_SyncSingleChangeList,
        SCC_Checkout,
        SCC_OpenForDelete,
        SCC_CheckoutGame,
        SCC_CheckoutShader,
        SCC_CheckoutDialog,
        SCC_CheckoutFonts,
        SCC_CheckoutLocPackage,
        SCC_CheckoutGDF,
        SCC_Resolve,
        SCC_Submit,
        SCC_CreateNewLabel,
        SCC_UpdateLabelDescription,
        SCC_Revert,
        SCC_RevertFile,
        SCC_GetLatest,
        SCC_Tag,
        SCC_TagFile,
        SCC_TagPCS,
        SCC_TagExe,
        CreatePackageJobs,
        LoadJobs,
        MergeJobs,
        CleanJobs,
        CopyJobs,
        GetJobs,
        WaitForJobs,
        AddJob,
        AddGCCJob,
        AddMSVCJob,
        CookPackageJob,
        CookMapJob,
        SCC_GetClientRoot,
        SetDependency,
        MakeWritable,
        MakeReadOnly, 
        Process,
        MSVCClean,
        MSVCBuild,
        GCCClean,
        GCCBuild,
        ShaderClean,
        ShaderBuild,
        ShaderBuildState,
        BuildScript,
        PreHeatMapOven,
        PreHeatJobOven,
        CookMaps,
        CreateHashes,
        Wrangle,
        CookingSuccess,
        CookerSyncSuccess,
        Publish,
        GetCookedBuild,
        GetInstallableBuild,
        BuildInstaller,
        CopyInstaller,
        Conform,
        CrossBuildConform,
        BumpEngineVersion,
        GetEngineVersion,
        UpdateGDFVersion,
        UpdateSourceServer,
        UpdateSymbolServer,
        UpdateSymbolServerTick,
        CheckSigned,
        Sign,
        SimpleCopy,
        SourceBuildCopy,
        SimpleDelete,
        SimpleRename,
        Wait,
        CheckSpace,
        UpdateLabel,
        UpdateFolder
    }
    
    public partial class Main : Form
    {
        [DllImport( "asus.dll" )]
        public static extern Int32 GetCPUTemperature();
        [DllImport( "asus.dll" )]
        public static extern Int32 GetMBTemperature();

        public string LabelRoot = "UnrealEngine3";

        BuilderDB DB = null;
        Emailer Mailer = null;
        P4 SCC = null;

        private DateTime LastPingTime = DateTime.Now;
        private DateTime LastTempTime = DateTime.Now;
        private DateTime LastBuildPollTime = DateTime.Now;
        private DateTime LastJobPollTime = DateTime.Now;
        private DateTime LastRestartTime = DateTime.Now;
        private DateTime LastKillTime = DateTime.Now;

        // CPU temperature
        private int LocalCPUTemperature = 0;
        public int CPUTemperature
        {
            get { return ( LocalCPUTemperature ); }
            set { LocalCPUTemperature = value; }
        }

        // Motherboard temperature
        private int LocalMoboTemperature = 0;
        public int MoboTemperature
        {
            get { return ( LocalMoboTemperature ); }
            set { LocalMoboTemperature = value; }
        }

        // The root folder of the clientspec - where all the branches reside
        private string LocalRootFolder = "";
        public string RootFolder
        {
            get { return ( LocalRootFolder ); }
            set { LocalRootFolder = value; }
        }

        // A timestamp of when this builder was compiled
        private DateTime LocalCompileDateTime;
        public DateTime CompileDateTime
        {
            get { return ( LocalCompileDateTime ); }
            set { LocalCompileDateTime = value; }
        }

        private string LocalMSVCVersion = "";
        public string MSVCVersion
        {
            get { return ( LocalMSVCVersion ); }
            set { LocalMSVCVersion = value; }
        }

        private string LocalDXVersion = "";
        public string DXVersion
        {
            get { return ( LocalDXVersion ); }
            set { LocalDXVersion = value; }
        }
        
        private string LocalXDKVersion = "";
        public string XDKVersion
        {
            get { return ( LocalXDKVersion ); }
            set { LocalXDKVersion = value; }
        }

        private string LocalPS3SDKVersion = "";
        public string PS3SDKVersion
        {
            get { return ( LocalPS3SDKVersion ); }
            set { LocalPS3SDKVersion = value; }
        }

        // Whether this machine is locked to a specific build
        private int LocalMachineLock = 0;
        public int MachineLock
        {
            get { return ( LocalMachineLock ); }
            set { LocalMachineLock = value; }
        }

        // A unique id for each job set
        private long LocalJobSpawnTime = 0;
        public long JobSpawnTime
        {
            get { return ( LocalJobSpawnTime ); }
            set { LocalJobSpawnTime = value; }
        }

        // The number of jobs placed into the job table
        private int LocalNumJobsToWaitFor = 0;
        public int NumJobsToWaitFor
        {
            get { return ( LocalNumJobsToWaitFor ); }
            set { LocalNumJobsToWaitFor = value; }
        }

        // The number of jobs completed so far
        private int LocalNumCompletedJobs = 0;
        public int NumCompletedJobs
        {
            get { return ( LocalNumCompletedJobs ); }
            set { LocalNumCompletedJobs = value; }
        }

        private string[] PotentialBranches = new string[] { "UnrealEngine3", "UnrealEngine3-Delta", "UnrealEngine3-UT3-PC", "UnrealEngine3-UT3-PS3", "UnrealEngine3-UT3-X360" };

        public bool Ticking = true;
        public bool Restart = false;
        public string MachineName = "";
        private string LoggedOnUser = "";
        private string SourceBranch = "";

        private int CommandID = 0;
        private int JobID = 0;
        private int BuilderID = 0;
        private int BuildLogID = 0;
        private int Promotable = 0;
        private Command CurrentCommand = null;
        private Queue<COMMANDS> PendingCommands = new Queue<COMMANDS>();
        private int BlockingBuildID = 0;
        private int BlockingBuildLogID = 0;
        private DateTime BlockingBuildStartTime = DateTime.Now;
        private ScriptParser Builder = null;
        private MODES Mode = 0;
        private string FinalStatus = "";

        delegate void DelegateAddLine( string Line, Color TextColor );
        delegate void DelegateSetStatus( string Line );
        delegate void DelegateMailGlitch();

        // Recursively mark each file in the folder as read write so it can be deleted
        private void MarkReadWrite( string Directory )
        {
            DirectoryInfo DirInfo = new DirectoryInfo( Directory );
            if( DirInfo.Exists )
            {
                FileInfo[] Infos = DirInfo.GetFiles();
                foreach( FileInfo Info in Infos )
                {
                    if( Info.Exists )
                    {
                        Info.IsReadOnly = false;
                    }
                }

                DirectoryInfo[] SubDirInfo = DirInfo.GetDirectories();
                foreach( DirectoryInfo Info in SubDirInfo )
                {
                    if( Info.Exists )
                    {
                        MarkReadWrite( Info.FullName );
                    }
                }
            }
        }

        // Recursively delete an ensure directory tree if the subfolders are older than 5 days
        public void DeleteDirectory( string Path, int DaysOld )
        {
            DirectoryInfo DirInfo = new DirectoryInfo( Path );
            if( DirInfo.Exists )
            {
                DirectoryInfo[] Directories = DirInfo.GetDirectories();
                foreach( DirectoryInfo Directory in Directories )
                {
                    TimeSpan Age = DateTime.UtcNow - Directory.CreationTimeUtc;
                    TimeSpan MaxAge = new TimeSpan( DaysOld, 0, 0, 0 );
                    if( Age > MaxAge )
                    {
                        MarkReadWrite( Directory.FullName );
                        Directory.Delete( true );
                    }
                }

                MarkReadWrite( DirInfo.FullName );
                DirInfo.Delete( true );
            }
        }

        // Ensure the base build folder exists to copy builds to
        private void EnsureDirectoryExists( string Path )
        {
            DirectoryInfo Dir = new DirectoryInfo( Path );
            if( !Dir.Exists )
            {
                Dir.Create();
            }
        }

        // Extract the time and date of compilation from the version number
        private void CalculateCompileDateTime()
        {
            System.Version Version = System.Reflection.Assembly.GetExecutingAssembly().GetName().Version;
            CompileDateTime = new DateTime( Version.Build * TimeSpan.TicksPerDay + Version.Revision * TimeSpan.TicksPerSecond * 2 ).AddYears( 1999 );

            Log( "Controller compiled on " + CompileDateTime.ToString(), Color.Blue );
        }

        private void GetInfo()
        {
            ManagementObjectSearcher Searcher = new ManagementObjectSearcher( "Select * from Win32_ComputerSystem" );
            ManagementObjectCollection Collection = Searcher.Get();

            foreach( ManagementObject Object in Collection )
            {
                Object Value;

                Value = Object.GetPropertyValue( "UserName" );
                if( Value != null )
                {
                    LoggedOnUser = Value.ToString();
                }

                Value = Object.GetPropertyValue( "Name" );
                if( Value != null )
                {
                    MachineName = Value.ToString();
                }

                Log( "Welcome \"" + LoggedOnUser + "\" running on \"" + MachineName + "\"", Color.Blue );
                break;
            }
        }

        private void GetMSVCVersion()
        {
            try
            {
                string MSVCExe = Environment.GetEnvironmentVariable( "VS80COMNTOOLS" ) + "/../IDE/Devenv.exe";
                FileVersionInfo VI = FileVersionInfo.GetVersionInfo( MSVCExe );
                MSVCVersion = VI.FileDescription + " version " + VI.ProductVersion;
            }
            catch
            {
                Ticking = false;
            }

            Log( "Compiling with: " + MSVCVersion, Color.Blue );
        }

        private void ValidatePS3EnvVars()
        {
            string[] PS3EnvVars = new string[] { "CELL_SDK", "CELL_SDK_WIN", "OSDIR", "MINGW", "SCE_PS3_ROOT" };

            foreach( string EnvVar in PS3EnvVars )
            {
                string EnvVarValue = Environment.GetEnvironmentVariable( EnvVar );
                Log( " ...... EnvVar '" + EnvVar + "' = '" + EnvVarValue + "'", Color.Blue );
            }
        }

        private void GetPS3SDKVersion()
        {
            try
            {
                string Line = "";

                string PS3SDK = Environment.GetEnvironmentVariable( "SCE_PS3_ROOT" ) + "/version-SDK";
                StreamReader Reader = new StreamReader( PS3SDK );
                if( Reader != null )
                {
                    Line = Reader.ReadToEnd();
                    Reader.Close();
                }
                PS3SDKVersion = Line.Trim();
            }
            catch
            {
            }

            if( PS3SDKVersion.Length > 0 )
            {
                Log( " ... using PS3 SDK: " + PS3SDKVersion, Color.Blue );

                ValidatePS3EnvVars();
            }
        }

        private void GetXDKVersion()
        {
            try
            {
                string Line;

                string XDK = Environment.GetEnvironmentVariable( "XEDK" ) + "/include/win32/vs2005/xdk.h";
                StreamReader Reader = new StreamReader( XDK );
                if( Reader != null )
                {
                    Line = Reader.ReadLine();
                    while( Line != null )
                    {
                        if( Line.StartsWith( "#define" ) )
                        {
                            int Offset = Line.IndexOf( "_XDK_VER" );
                            if( Offset >= 0 )
                            {
                                XDKVersion = Line.Substring( Offset + "_XDK_VER".Length ).Trim();
                                break;
                            }
                        }

                        Line = Reader.ReadLine();
                    }
                    Reader.Close();
                }
            }
            catch
            {
            }
            Log( " ... using XDK: " + XDKVersion, Color.Blue );
        }

        private void GetDirectXVersion()
        {
            try
            {
                DXVersion = "UNKNOWN";
                string DirectXLocation = Environment.GetEnvironmentVariable( "DXSDK_DIR" );
                int OpenParenIndex = DirectXLocation.LastIndexOf( '(' );
                int CloseParenIndex = DirectXLocation.LastIndexOf( ')' );
                if( OpenParenIndex >= 0 && CloseParenIndex >= 0 && CloseParenIndex > OpenParenIndex )
                {
                    DXVersion = DirectXLocation.Substring( OpenParenIndex + 1, CloseParenIndex - OpenParenIndex - 1 );
                    Log( " ... found " + DXVersion + " DirectX", Color.Blue );
                }
                else
                {
                    Log( " ... did not find any version of DirectX", Color.Blue );
                }
            }
            catch
            {
                Ticking = false;
            }
        }

        private void BroadcastMachine()
        {
            string Query, Command;
            int ID;

            // Clearing out zombie connections if the process stopped unexpectedly
            Query = "SELECT [ID] FROM [Builders] WHERE ( Machine ='" + MachineName + "' AND State != 'Dead' AND State != 'Zombied' )";
            ID = DB.ReadInt( Query );
            while( ID != 0 )
            {
                Command = "UPDATE Builders SET State = 'Zombied' WHERE ( ID = " + ID.ToString() + " )";
                DB.Update( Command );

                Log( "Cleared zombie connection " + ID.ToString() + " from database", Color.Orange );
                ID = DB.ReadInt( Query );
            }

            // Clear out zombie commands
            Query = "SELECT [ID] FROM [Commands] WHERE ( Machine ='" + MachineName + "' )";
            ID = DB.ReadInt( Query );
            if( ID != 0 )
            {
                DB.Delete( "Commands", ID, "Machine" );
                DB.Delete( "Commands", ID, "BuildLogID" );
            }

            Log( "Registering '" + MachineName + "' with database", Color.Blue );

            // Insert machine as new row
            Command = "INSERT INTO [Builders] ( Machine ) VALUES ( '" + MachineName + "' )";
            DB.Update( Command );

            Query = "SELECT [ID] FROM [Builders] WHERE ( Machine = '" + MachineName + "' AND State is NULL )";
            ID = DB.ReadInt( Query );
            if( ID != 0 )
            {
                Command = "UPDATE Builders SET State = 'Connected' WHERE ( ID = " + ID.ToString() + " )";
                DB.Update( Command );

                Command = "UPDATE Builders SET StartTime = '" + DateTime.Now.ToString() + "' WHERE ( ID = " + ID.ToString() + " )";
                DB.Update( Command );

                foreach( string Branch in PotentialBranches )
                {
                    if( SCC.BranchExists( Branch ) )
                    {
                        Command = "INSERT INTO [Branches] ( BuilderID, Branch ) VALUES ( " + ID.ToString() + ", '" + Branch + "' )";
                        DB.Update( Command );
                    }
                }
#if DEBUG
                Command = "INSERT INTO [Branches] ( BuilderID, Branch ) VALUES ( " + ID.ToString() + ", 'UnrealEngine3-Builder' )";
                DB.Update( Command );
#endif
            }
        }

        private void MaintainMachine()
        {
            // Ping every 30 seconds
            TimeSpan PingTime = new TimeSpan( 0, 0, 30 );
            if( DateTime.Now - LastPingTime > PingTime )
            {
                string Command = "SELECT [ID] FROM [Builders] WHERE ( Machine = '" + MachineName + "' AND State != 'Dead' AND State != 'Zombied' )";
                int ID = DB.ReadInt( Command );
                if( ID != 0 )
                {
                    Command = "UPDATE Builders SET CurrentTime = '" + DateTime.Now.ToString() + "' WHERE ( ID = " + ID.ToString() + " )";
                    DB.Update( Command );
                }

                LastPingTime = DateTime.Now;
            }
        }

        private void GetTemperatures()
        {
            // Acquire every 30 seconds
            TimeSpan TemperatureTime = new TimeSpan( 0, 0, 3 );
            if( DateTime.Now - LastTempTime > TemperatureTime )
            {
                try
                {
                    // CPUTemperature = GetCPUTemperature();
                    // MoboTemperature = GetMBTemperature();

                    if( CPUTemperature > 65 || MoboTemperature > 45 )
                    {
                        Mailer.SendTemperatureWarning( CPUTemperature, MoboTemperature );
                    }

                    Text = "Build Controller : CPU " + CPUTemperature.ToString() + "C / Motherboard " + MoboTemperature.ToString() + "C";
                }
                catch
                {
                }

                LastTempTime = DateTime.Now;
                Text = "Build Controller : CPU " + CPUTemperature.ToString() + "C / Motherboard " + MoboTemperature.ToString() + "C";
            }
        }

        private int PollForBuild()
        {
            int ID = 0;

            // Check every 5 seconds
            TimeSpan PingTime = new TimeSpan( 0, 0, 5 );
            if( DateTime.Now - LastBuildPollTime > PingTime )
            {
                ID = DB.PollForBuild( MachineName );
                LastBuildPollTime = DateTime.Now;

                // Check for build already running
                string Query = "SELECT [Machine] FROM [Commands] WHERE ( ID = " + ID.ToString() + " )";
                string Machine = DB.ReadString( Query );
                if( Machine.Length > 0 )
                {
                    string BuildType = DB.GetString( "Commands", ID, "Description" );
                    Log( "[STATUS] Suppressing retrigger of '" + BuildType + "'", Color.Magenta );
                    Mailer.SendAlreadyInProgressMail( DB, Builder, ID, BuildType );

                    ID = 0;
                }
            }

            return ( ID );
        }

        private int PollForJob()
        {
            int ID = 0;

            if( MachineLock == 0 )
            {
                // Check every 5 seconds
                TimeSpan PingTime = new TimeSpan( 0, 0, 5 );
                if( DateTime.Now - LastJobPollTime > PingTime )
                {
                    ID = DB.PollForJob( MachineName );
                    LastJobPollTime = DateTime.Now;
                }
            }

            return ( ID );
        }

        private int PollForKill()
        {
            int ID = 0;

            // Check every 5 seconds
            TimeSpan PingTime = new TimeSpan( 0, 0, 5 );
            if( DateTime.Now - LastKillTime > PingTime )
            {
                ID = DB.PollForKill();
                LastKillTime = DateTime.Now;
            }

            return ( ID );
        }

        private void UnbroadcastMachine()
        {
            Log( "Unregistering '" + MachineName + "' from database", Color.Blue );

            string Command = "SELECT [ID] FROM [Builders] WHERE ( Machine = '" + MachineName + "' AND State != 'Dead' AND State != 'Zombied' )";
            int ID = DB.ReadInt( Command );
            if( ID != 0 )
            {
                Command = "UPDATE Builders SET EndTime = '" + DateTime.Now.ToString() + "' WHERE ( ID = " + ID.ToString() + " )";
                DB.Update( Command ); 

                Command = "UPDATE Builders SET State = 'Dead' WHERE ( ID = " + ID.ToString() + " )";
                DB.Update( Command );
            }
        }

        private void CheckRestart()
        {
            // Check every 5 seconds
            TimeSpan PingTime = new TimeSpan( 0, 0, 5 );
            if( DateTime.Now - LastRestartTime > PingTime )
            {
                string Command = "SELECT [ID] FROM [Builders] WHERE ( Machine = '" + MachineName + "' AND State != 'Dead' AND State != 'Zombied' )";
                int ID = DB.ReadInt( Command );
                if( ID != 0 )
                {
                    Command = "SELECT [Restart] FROM Builders WHERE ( ID = " + ID.ToString() + " AND Restart is not NULL )";
                    if( DB.ReadBool( Command ) )
                    {
                        SCC.SyncBuildScripts( SourceBranch, "/Development/Builder/..." );
                        Restart = true;
                        Ticking = false;
                    }
                }
                else
                {
                    // Error reading from DB, so wait a while for things to settle, then restart
                    System.Threading.Thread.Sleep( 1000 * 60 );

                    Restart = true;
                    Ticking = false;
                }

                LastRestartTime = DateTime.Now;
            }
        }

        private void GetSourceBranch()
        {
            string CWD = Environment.CurrentDirectory;
            int Index = CWD.LastIndexOf( '\\' );
            if( Index >= 0 )
            {
                RootFolder = CWD.Substring( 0, Index );
                SourceBranch = CWD.Substring( Index + 1 );
            }
        }

        public void Init()
        {
            // Show log window
            Show();

            DB = new BuilderDB( this );
            Mailer = new Emailer( this );
            SCC = new P4( this );

            Application.DoEvents();

            GetInfo();
            CalculateCompileDateTime();
            GetMSVCVersion();
            GetDirectXVersion();
            GetPS3SDKVersion();
            GetXDKVersion();
            GetSourceBranch();

            Application.DoEvents();

            // Something went wrong during setup - sleep and retry
            if( !Ticking )
            {
                System.Threading.Thread.Sleep( 30000 );
                Restart = true;
                return;
            }

            // Register with DB
            BroadcastMachine();
            // Inform user of restart
            Mailer.SendRestartedMail();

            // If this machine locked to a build then don't allow it to grab normal ones
            string Query = "SELECT [ID] FROM Commands WHERE ( MachineLock = '" + MachineName + "' )";
            MachineLock = DB.ReadInt( Query );

            Log( "Running from root folder '" + RootFolder + "'", Color.DarkGreen );
        }

        public void Destroy()
        {
            UnbroadcastMachine();

            if( DB != null )
            {
                DB.Destroy();
            }
        }

        public Main()
        {
            InitializeComponent();
            
            MainWindow_SizeChanged( null, null );
        }

        private void MainWindow_SizeChanged( object sender, EventArgs e )
        {
            System.Drawing.Size logSize = new Size();
            logSize.Height = this.TextBox_Log.Parent.Size.Height - 55;
            logSize.Width = this.TextBox_Log.Parent.Size.Width - 10;
            this.TextBox_Log.Size = logSize;
        }

        public void Log( string Line, Color TextColour )
        {
            if( Line == null || !Ticking )
            {
                return;
            }

            // if we need to, invoke the delegate
            if( InvokeRequired )
            {
                Invoke( new DelegateAddLine( Log ), new object[] { Line, TextColour } );
                return;
            }

            DateTime Now = DateTime.Now;
            string FullLine = Now.ToLongTimeString() + ": " + Line;

            TextBox_Log.Focus();
            TextBox_Log.SelectionLength = 0;

            // Only set the color if it is different than the foreground colour
            if( TextBox_Log.SelectionColor != TextColour )
            {
                TextBox_Log.SelectionColor = TextColour;
            }

            TextBox_Log.AppendText( FullLine + "\r\n" );

            CheckStatusUpdate( Line );
        }

        public void CheckStatusUpdate( string Line )
        {
            if( Line == null || !Ticking || BuildLogID == 0 )
            {
                return;
            }

            // Handle any special controls
            if( Line.StartsWith( "[STATUS] " ) )
            {
                Line = Line.Substring( "[STATUS] ".Length );
                SetStatus( Line.Trim() );
            }
            else if( Line.IndexOf( "------" ) >= 0 )
            {
                Line = Line.Substring( Line.IndexOf( "------" ) + "------".Length );
                Line = Line.Substring( 0, Line.IndexOf( "------" ) );
                SetStatus( Line.Trim() );
            }

            if( Line.IndexOf( "=> NETWORK " ) >= 0 )
            {
                MailGlitch();
            }
        }

        private long GetDirectorySize( string Directory )
        {
            long CurrentSize = 0;

            DirectoryInfo DirInfo = new DirectoryInfo( Directory );
            if( DirInfo.Exists )
            {
                FileInfo[] Infos = DirInfo.GetFiles();
                foreach( FileInfo Info in Infos )
                {
                    if( Info.Exists )
                    {
                        CurrentSize += Info.Length;
                    }
                }

                DirectoryInfo[] SubDirInfo = DirInfo.GetDirectories();
                foreach( DirectoryInfo Info in SubDirInfo )
                {
                    if( Info.Exists )
                    {
                        CurrentSize += GetDirectorySize( Info.FullName );
                    }
                }
            }

            return ( CurrentSize );
        }

        private string GetDirectoryStatus( ScriptParser Builder )
        {
            StringBuilder Status = new StringBuilder();

            try
            {
                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );

                foreach( string Directory in Parms )
                {
                    long BytesUsed = GetDirectorySize( Directory ) / ( 1024 * 1024 * 1024 );

                    Status.Append( "'" + Directory + "' consumes " + BytesUsed.ToString() + " GB\r\n" );
                }
            }
            catch
            {
                Status = new StringBuilder();
                Status.Append( "ERROR while interrogating directories" );
            }
            return ( Status.ToString() );
        }

        public void MailGlitch()
        {
            if( InvokeRequired )
            {
                Invoke( new DelegateMailGlitch( MailGlitch ), new object[] {} );
                return;
            }

            Mailer.SendGlitchMail();
        }

        public void SetStatus( string Line )
        {
            // if we need to, invoke the delegate
            if( InvokeRequired )
            {
                Invoke( new DelegateSetStatus( SetStatus ), new object[] { Line } );
                return;
            }

            if( Line.Length > 127 )
            {
                Line = Line.Substring( 0, 127 );
            }

            DB.SetString( "BuildLog", BuildLogID, "CurrentStatus", Line );
        }

        public void Log( Array Lines, Color TextColour )
        {
            foreach( string Line in Lines )
            {
                Log( Line, TextColour );
            }
        }

        public string ExpandString( string Input, string Branch )
        {
            string[] Parms = Input.Split( " \t".ToCharArray() );

            for( int i = 0; i < Parms.Length; i++ )
            {
                if( Parms[i] == "DatabaseParameter" )
                {
                    Parms[i] = DB.GetString( "Commands", CommandID, "Parameter" );
                }
                else if( Parms[i] == "JobParameter" )
                {
                    Parms[i] = DB.GetString( "Jobs", JobID, "Parameter" );
                }
                else if( Parms[i] == "JobGame" )
                {
                    Parms[i] = DB.GetString( "Jobs", JobID, "Game" );
                }
                else if( Parms[i] == "JobPlatform" )
                {
                    Parms[i] = DB.GetString( "Jobs", JobID, "Platform" );
                }
                else if( Parms[i].StartsWith( "#" ) )
                {
                    Parms[i] = DB.GetVariable( Branch, Parms[i].Substring( 1 ) );
                }
            }

            // Reconstruct Command line
            string Line = "";
            foreach( string Parm in Parms )
            {
                Line += Parm + " ";
            }

            return ( Line.Trim() );
        }

        private void Cleanup()
        {
            // Revert all open files
            CurrentCommand = new Command( this, SCC, Builder );
            CurrentCommand.Execute( COMMANDS.SCC_Revert );

            // Mark as read only any files marked writable
            CurrentCommand = new Command( this, SCC, Builder );
            CurrentCommand.Execute( COMMANDS.MakeReadOnly );

            DB.SetString( "Builders", BuilderID, "State", "Connected" );

            if( CommandID != 0 )
            {
                DB.Delete( "Commands", CommandID, "Machine" );
                DB.Delete( "Commands", CommandID, "Killing" );
                DB.Delete( "Commands", CommandID, "BuildLogID" );
                DB.Delete( "Commands", CommandID, "ConchHolder" );
            }
            else if( JobID != 0 )
            {
                DB.SetBool( "Jobs", JobID, "Complete", true );
            }

            // Set the DB up with the result of the build
            DB.SetString( "BuildLog", BuildLogID, "CurrentStatus", FinalStatus );

            FinalStatus = "";
            CurrentCommand = null;
            Builder.Destroy();
            Builder = null;
            BlockingBuildID = 0;
            BlockingBuildLogID = 0;
            Promotable = 0;
            JobID = 0;
            CommandID = 0;
            BuilderID = 0;
            BuildLogID = 0;
        }

        private MODES HandleComplete()
        {
            DB.SetDateTime( "BuildLog", BuildLogID, "BuildEnded", DateTime.Now );

            if( CommandID != 0 )
            {
                if( Builder.IsPromoting )
                {
                    // There was a 'tag' command in the script
                    Mailer.SendPromotedMail( DB, Builder, CommandID );
                }
                else if( Builder.IsPublishing )
                {
                    // There was a 'publish' command in the script
                    Mailer.SendPublishedMail( DB, Builder, CommandID, BuildLogID );
                }
                else if( Builder.IsBuilding )
                {
                    // There was a 'submit' command in the script
                    Mailer.SendSucceededMail( DB, Builder, CommandID, BuildLogID, Builder.GetChanges() );
                }
                else if( Builder.IsMakingInstall )
                {
                    // There was a 'buildinstall' command in the script
                    Mailer.SendMakingInstallMail( DB, Builder, CommandID, BuildLogID );
                }

                if( Builder.LabelInfo.Changelist > Builder.LastGoodBuild )
                {
                    DB.SetLastGoodBuild( CommandID, Builder.LabelInfo.Changelist, DateTime.Now );

                    string Label = Builder.LabelInfo.GetLabelName();
                    if( Builder.CreateLabel || Builder.NewLabelCreated )
                    {
                        DB.SetString( "Commands", CommandID, "LastGoodLabel", Label );
                        DB.SetString( "BuildLog", BuildLogID, "BuildLabel", Label );
                    }

                    if( Builder.IsPromoting )
                    {
                        DB.SetString( "Commands", CommandID, "LastGoodLabel", Label );
                    }
                }
            }
            else if( JobID != 0 )
            {
                DB.SetBool( "Jobs", JobID, "Succeeded", true );
            }

            FinalStatus = "Succeeded";
            return ( MODES.Exit );
        }

        public string GetLabelToSync()
        {
            // If we have a valid label - use that
            if( Builder.LabelInfo.ValidLabel )
            {
                string Label = Builder.LabelInfo.GetLabelName();
                Log( "Label revision: @" + Label, Color.DarkGreen );
                return ( "@" + Label );
            }

            // No label to sync - sync to head
            Log( "Invalid or nonexistant label, default: #head", Color.DarkGreen );
            return ( "#head" );
        }

        public string GetChangeListToSync()
        {
            if( Builder.LabelInfo.ValidLabel )
            {
                string Changelist = Builder.LabelInfo.Changelist.ToString();
                Log( "Changelist revision: @" + Changelist, Color.DarkGreen );
                return ( "@" + Changelist );
            }

            // No label to sync - sync to head
            Log( "No changelist found, default: #head", Color.DarkGreen );
            return ( "#head" );
        }

        public string GetBuildToSync()
        {
            if( Builder.Dependency.Length == 0 )
            {
                return ( "" );
            }

            Builder.PublishFolder = Builder.Dependency;

            string Path = Builder.CommandLine + "/" + Builder.Dependency;
            return ( Path );
        }

        // Populate the Jobs table with entries from the jobs xml file
        public void PopulateJobs( JobDescriptions Jobs )
        {
            JobSpawnTime = DateTime.UtcNow.Ticks;
            NumJobsToWaitFor = Jobs.Jobs.Length;

            foreach( JobInfo Job in Jobs.Jobs )
            {
                string CommandString = "INSERT INTO [Jobs] ( Name, Command, Platform, Game, Parameter, Branch, Label, Machine, Active, Complete, Succeeded, SpawnTime ) VALUES ( '";
                CommandString += Job.Name + "','" + Job.Command + "','" + Builder.LabelInfo.Platform + "','";
                CommandString += Builder.LabelInfo.Game + "','" + Job.Parameter + "','" + Builder.SourceControlBranch + "','" + Builder.Dependency + "','',0,0,0," + JobSpawnTime.ToString() + " )";
                DB.Update( CommandString );
            }

            Log( "Added " + NumJobsToWaitFor.ToString() + " package jobs.", Color.DarkGreen );
        }

        public void AddJob( JobInfo Job )
        {
            string CommandString = "INSERT INTO [Jobs] ( Name, Command, Platform, Game, Parameter, Branch, Label, Machine, BuildLogID, Active, Complete, Succeeded, SpawnTime ) VALUES ( '";
            CommandString += Job.Name + "','" + Job.Command + "','" + Builder.LabelInfo.Platform + "','";
            CommandString += Builder.LabelInfo.Game + "','" + Job.Parameter + "','" + Builder.SourceControlBranch + "','" + Builder.Dependency + "','',0,0,0,0," + JobSpawnTime.ToString() + " )";
            DB.Update( CommandString );

            NumJobsToWaitFor++;

            Log( "Added job: " + Job.Name, Color.DarkGreen );
        }

        // Kill an in progress build
        private void KillBuild( int ID )
        {
            if( CommandID > 0 && CommandID == ID )
            {
                if( CurrentCommand != null )
                {
                    CurrentCommand.Kill();
                }

                Mode = MODES.Exit;
                Log( "Process killed", Color.Red );

                Mailer.SendKilledMail( DB, Builder, CommandID, BuildLogID );

                Cleanup();
            }
        }

        private void SpawnBuild( int ID )
        {
            string CommandString;

            DeleteDirectory( "C:\\Builds", 5 );
            EnsureDirectoryExists( "C:\\Builds" );

            DeleteDirectory( "C:\\Install", 0 );
            EnsureDirectoryExists( "C:\\Install" );
            
            CommandID = ID;

            DateTime BuildStarted = DateTime.Now;
            string Script = DB.GetString( "Commands", CommandID, "Command" );
            string Description = DB.GetString( "Commands", CommandID, "Description" );
            string SourceControlBranch = DB.GetString( "Commands", CommandID, "Branch" );
            int LastGoodBuild = DB.GetInt( "Commands", CommandID, "LastGoodChangeList" );

            Environment.CurrentDirectory = RootFolder + "\\" + SourceControlBranch;
            EnsureDirectoryExists( "Development\\Builder\\Logs" );

            // Make sure we have the latest build scripts
            SCC.SyncBuildScripts( SourceControlBranch, "/Development/Builder/..." );

            // Make sure there are no pending kill commands
            DB.Delete( "Commands", CommandID, "Killing" );
            // Clear out killer (if any)
            DB.SetString( "Commands", CommandID, "Killer", "" );

            Builder = new ScriptParser( this, Script, Description, SourceControlBranch, LastGoodBuild, BuildStarted );

            CommandString = "SELECT [ID] FROM [Builders] WHERE ( Machine = '" + MachineName + "' AND State != 'Dead' AND State != 'Zombied' )";
            BuilderID = DB.ReadInt( CommandString );

            // Set builder to building
            DB.SetString( "Builders", BuilderID, "State", "Building" );

            // Add a new entry with the command
            CommandString = "INSERT INTO [BuildLog] ( BuildStarted ) VALUES ( '" + BuildStarted.ToString() + "' )";
            DB.Update( CommandString );

            ProcedureParameter ParmTable = new ProcedureParameter( "TableName", "BuildLog", SqlDbType.Text, "BuildLog".Length );
            BuildLogID = DB.ReadInt( "GetNewID", ParmTable, null );

            if( BuildLogID != 0 )
            {
                CommandString = "UPDATE BuildLog SET CurrentStatus = 'Spawning' WHERE ( ID = " + BuildLogID.ToString() + " )";
                DB.Update( CommandString );

                CommandString = "UPDATE BuildLog SET Machine = '" + MachineName + "' WHERE ( ID = " + BuildLogID.ToString() + " )";
                DB.Update( CommandString );

                CommandString = "UPDATE BuildLog SET Command = '" + Script + "' WHERE ( ID = " + BuildLogID.ToString() + " )";
                DB.Update( CommandString );

                CommandString = "UPDATE Commands SET BuildLogID = '" + BuildLogID.ToString() + "' WHERE ( ID = " + CommandID.ToString() + " )";
                DB.Update( CommandString );

                CommandString = "UPDATE Commands SET Machine = '" + MachineName + "' WHERE ( ID = " + CommandID.ToString() + " )";
                DB.Update( CommandString );

                CommandString = "SELECT Promotable FROM Commands WHERE ( ID = " + CommandID.ToString() + " )";
                Promotable = DB.ReadInt( CommandString );
                DB.SetInt( "BuildLog", BuildLogID, "Promotable", Promotable );
            }

            SCC.Init();
            PendingCommands.Clear();
            JobSpawnTime = DateTime.UtcNow.Ticks;

            Mode = MODES.Init;
        }

        private void SpawnJob( int ID )
        {
            string CommandString;

            DeleteDirectory( "C:\\Builds", 5 );
            EnsureDirectoryExists( "C:\\Builds" );

            DeleteDirectory( "C:\\Install", 0 );
            EnsureDirectoryExists( "C:\\Install" );

            JobID = ID;

            DateTime BuildStarted = DateTime.Now;
            string JobName = DB.GetString( "Jobs", JobID, "Name" );
            string Script = DB.GetString( "Jobs", JobID, "Command" );
            string SourceControlBranch = DB.GetString( "Jobs", JobID, "Branch" );

            Environment.CurrentDirectory = RootFolder + "\\" + SourceControlBranch;
            EnsureDirectoryExists( "Development\\Builder\\Logs" );

            // Make sure we have the latest build scripts
            SCC.SyncBuildScripts( SourceControlBranch, "/Development/Builder/..." );

            Builder = new ScriptParser( this, Script, JobName, SourceControlBranch, 0, BuildStarted );

            CommandString = "SELECT [ID] FROM [Builders] WHERE ( Machine = '" + MachineName + "' AND State != 'Dead' AND State != 'Zombied' )";
            BuilderID = DB.ReadInt( CommandString );

            // Set builder to building
            DB.SetString( "Builders", BuilderID, "State", "Building" );

            // Add a new entry with the command
            CommandString = "INSERT INTO [BuildLog] ( BuildStarted ) VALUES ( '" + BuildStarted.ToString() + "' )";
            DB.Update( CommandString );

            ProcedureParameter ParmTable = new ProcedureParameter( "TableName", "BuildLog", SqlDbType.Text, "BuildLog".Length );
            BuildLogID = DB.ReadInt( "GetNewID", ParmTable, null );

            if( BuildLogID != 0 )
            {
                CommandString = "UPDATE BuildLog SET CurrentStatus = 'Spawning' WHERE ( ID = " + BuildLogID.ToString() + " )";
                DB.Update( CommandString );

                CommandString = "UPDATE BuildLog SET Machine = '" + MachineName + "' WHERE ( ID = " + BuildLogID.ToString() + " )";
                DB.Update( CommandString );

                CommandString = "UPDATE BuildLog SET Command = '" + Script + "' WHERE ( ID = " + BuildLogID.ToString() + " )";
                DB.Update( CommandString );

                CommandString = "UPDATE Jobs SET BuildLogID = '" + BuildLogID.ToString() + "' WHERE ( ID = " + JobID.ToString() + " )";
                DB.Update( CommandString );

                CommandString = "UPDATE Jobs SET Machine = '" + MachineName + "' WHERE ( ID = " + JobID.ToString() + " )";
                DB.Update( CommandString );
            }

            // Grab game and platform
            CommandString = "SELECT [Game] FROM [Jobs] WHERE ( ID = " + JobID.ToString() + " )";
            Builder.LabelInfo.Game = DB.ReadString( CommandString );

            CommandString = "SELECT [Platform] FROM [Jobs] WHERE ( ID = " + JobID.ToString() + " )";
            Builder.LabelInfo.Platform = DB.ReadString( CommandString );

            Builder.Dependency = DB.GetString( "Jobs", JobID, "Label" );
            Builder.LabelInfo.Init( SCC, Builder );

            SCC.Init();
            PendingCommands.Clear();

            Mode = MODES.Init;
        }

        private MODES HandleError()
        {
            int TimeOutMinutes;
            bool CheckCookingSuccess = ( Builder.GetCommandID() == COMMANDS.CookMaps );
            bool CheckCookerSyncSuccess = ( Builder.GetCommandID() == COMMANDS.Publish );
            string GeneralStatus = "";
            string Status = "Succeeded";

            // Internal error?
            ERRORS ErrorLevel = Builder.GetErrorLevel();

            if( CurrentCommand != null && ErrorLevel == ERRORS.None )
            {
                // ...or error that requires parsing the log
                ErrorLevel = CurrentCommand.GetErrorLevel();

                LogParser Parser = new LogParser( Builder );
                bool ReportEverything = ( ErrorLevel >= ERRORS.SCC_Sync && ErrorLevel <= ERRORS.SCC_GetClientRoot );
                Status = Parser.Parse( ReportEverything, CheckCookingSuccess, CheckCookerSyncSuccess, ref ErrorLevel );
            }
#if !DEBUG
            // If we were cooking, and didn't find the cooking success message, set to fail
            if( CheckCookingSuccess )
            {
                if( ErrorLevel == ERRORS.CookingSuccess )
                {
                    ErrorLevel = ERRORS.None;
                }
                else if( ErrorLevel == ERRORS.None && Status == "Succeeded" )
                {
                    ErrorLevel = ERRORS.CookMaps;
                    Status = "Could not find cooking successful message";
                }
            }
#endif

            // If we were publishing, and didn't find the publish success message, set to fail
            if( CheckCookerSyncSuccess )
            {
                // Free up the conch so other builders can publish
                DB.Delete( "Commands", CommandID, "ConchHolder" );

                if( ErrorLevel == ERRORS.CookerSyncSuccess )
                {
                    ErrorLevel = ERRORS.None;
                }
                else if( ErrorLevel == ERRORS.None && Status == "Succeeded" )
                {
                    ErrorLevel = ERRORS.Publish;
                    Status = "Could not find publish completion message";
                }
            }

            // Check for total success
            if( ErrorLevel == ERRORS.None && Status == "Succeeded" )
            {
                return ( MODES.Init );
            }

            // If we were checking to see if a file was signed, conditionally add another command
            if( ErrorLevel == ERRORS.CheckSigned )
            {
                // Error checking to see if file was signed - so sign it
                if( CurrentCommand.GetCurrentBuild().GetExitCode() != 0 )
                {
                    PendingCommands.Enqueue( COMMANDS.Sign );
                }
                return ( MODES.Init );
            }

            // Copy the failure log to a common network location
            FileInfo LogFile = new FileInfo( "None" );
            if( Builder.LogFileName.Length > 0 )
            {
                LogFile = new FileInfo( Builder.LogFileName );
                try
                {
                    if( LogFile.Exists )
                    {
                        LogFile.CopyTo( Properties.Settings.Default.FailedLogLocation + "/" + LogFile.Name );
                    }
                }
                catch
                {
                }
            }

            if( Status == "Succeeded" )
            {
                Status = "Could not find error";
            }

            // Handle specific errors
            switch( ErrorLevel )
            {
                case ERRORS.None:
                    Mailer.SendFailedMail( DB, Builder, CommandID, BuildLogID, Status, LogFile.Name, "" );

                    Log( "LOG ERROR: " + Builder.GetCommandID() + " " + Builder.CommandLine + " failed", Color.Red );
                    Log( Status, Color.Red );
                    break;

                case ERRORS.NoScript:
                    Status = "No build script";
                    Log( "ERROR: " + Status, Color.Red );
                    Mailer.SendFailedMail( DB, Builder, CommandID, BuildLogID, Status, LogFile.Name, "" );
                    break;

                case ERRORS.IllegalCommand:
                    Status = "Illegal command: '" + Builder.CommandLine + "'";
                    Log( "ERROR: " + Status, Color.Red );
                    Mailer.SendFailedMail( DB, Builder, CommandID, BuildLogID, Status, LogFile.Name, "" );
                    break;

                case ERRORS.SCC_Submit:
                    GeneralStatus = Builder.GetCommandID() + " " + Builder.CommandLine + " failed with error '" + ErrorLevel.ToString() + "'";
                    GeneralStatus += "\r\n\r\n" + Status;

                    GeneralStatus += SCC.GetIncorrectCheckedOutFiles();

                    Log( "ERROR: " + GeneralStatus, Color.Red );
                    Mailer.SendFailedMail( DB, Builder, CommandID, BuildLogID, GeneralStatus, LogFile.Name, "" );
                    break;

                case ERRORS.SCC_Sync:
                case ERRORS.SCC_Checkout:
                case ERRORS.SCC_Revert:
                case ERRORS.SCC_Tag:
                case ERRORS.SCC_GetClientRoot:
                case ERRORS.MakeWritable:
                case ERRORS.MakeReadOnly:
                case ERRORS.UpdateSymbolServer:
                    GeneralStatus = Builder.GetCommandID() + " " + Builder.CommandLine + " failed with error '" + ErrorLevel.ToString() + "'";
                    GeneralStatus += "\r\n\r\n" + Status;
                    Log( "ERROR: " + GeneralStatus, Color.Red );
                    Mailer.SendFailedMail( DB, Builder, CommandID, BuildLogID, GeneralStatus, LogFile.Name, "" );
                    break;
                
                case ERRORS.Process:
                case ERRORS.MSVCClean:
                case ERRORS.MSVCBuild:
                case ERRORS.GCCBuild:
                case ERRORS.GCCClean:
                case ERRORS.ShaderBuild:
                case ERRORS.ShaderClean:
                case ERRORS.BuildScript:
                case ERRORS.CookMaps:
                    Mailer.SendFailedMail( DB, Builder, CommandID, BuildLogID, Status, LogFile.Name, "" );
                    break;
                
                case ERRORS.Publish:
                    Mailer.SendFailedMail( DB, Builder, CommandID, BuildLogID, Status, LogFile.Name, "IT@epicgames.com" );
                    break;

                case ERRORS.CheckSigned:
                    return ( MODES.Init );

                case ERRORS.TimedOut:
                    TimeOutMinutes = ( int )Builder.GetTimeout().TotalMinutes;
                    GeneralStatus = "'" + Builder.GetCommandID() + " " + Builder.CommandLine + "' TIMED OUT after " + TimeOutMinutes.ToString() + " minutes";
                    GeneralStatus += "\r\n";
                    Log( "ERROR: " + GeneralStatus, Color.Red );
                    Mailer.SendFailedMail( DB, Builder, CommandID, BuildLogID, GeneralStatus, LogFile.Name, "" );
                    break;

                case ERRORS.WaitTimedOut:
                    TimeOutMinutes = ( int )Builder.GetTimeout().TotalMinutes;
                    Status = "Waiting for '" + Builder.CommandLine + "' TIMED OUT after " + TimeOutMinutes.ToString() + " minutes";
                    Log( "ERROR: " + Status, Color.Red );
                    Mailer.SendFailedMail( DB, Builder, CommandID, BuildLogID, Status, LogFile.Name, "" );
                    break;

                case ERRORS.FailedJobs:
                    Status = "All jobs completed, but one or more failed.";
                    Log( "ERROR: " + Status, Color.Red );
                    Mailer.SendFailedMail( DB, Builder, CommandID, BuildLogID, Status, LogFile.Name, "" );
                    break;

                case ERRORS.Crashed:
                    int NotRespondingMinutes = ( int )Builder.GetRespondingTimeout().TotalMinutes;
                    Status = "'" + Builder.GetCommandID() + " " + Builder.CommandLine + "' was not responding for " + NotRespondingMinutes.ToString() + " minutes; presumed crashed.";
                    Log( "ERROR: " + Status, Color.Red );
                    Mailer.SendFailedMail( DB, Builder, CommandID, BuildLogID, Status, LogFile.Name, "" );
                    break;

                default:
                    Status = "'" + Builder.GetCommandID() + " " + Builder.CommandLine + "' unhandled error '" + ErrorLevel.ToString() + "'";
                    Log( "ERROR: " + Status, Color.Red );
                    Mailer.SendFailedMail( DB, Builder, CommandID, BuildLogID, Status, LogFile.Name, "" );
                    break;
            }

            FinalStatus = "Failed";
            return ( MODES.Exit );
        }

        private void RunBuild()
        {
            switch( Mode )
            {
                case MODES.Init:
                    COMMANDS NextCommand;

                    CurrentCommand = null;
                    
                    // Get a new command ...
                    if( PendingCommands.Count > 0 )
                    {
                        // ... either pending 
                        NextCommand = PendingCommands.Dequeue();
                        Builder.SetCommandID( NextCommand );
                    }
                    else
                    {
                        // ... or from the script
                        NextCommand = Builder.ParseNextLine();
                    }

                    // Expand out any variables
                    Builder.CommandLine = ExpandString( Builder.CommandLine, Builder.SourceControlBranch ); ;

                    switch( NextCommand )
                    {
                        case COMMANDS.Error:
                            Mode = MODES.Finalise;
                            break;

                        case COMMANDS.Finished:
                            Mode = HandleComplete();
                            break;

                        case COMMANDS.Config:
                            break;

                        case COMMANDS.TriggerMail:
                            Mailer.SendTriggeredMail( DB, Builder, CommandID );
                            Mode = MODES.Finalise;
                            break;

                        case COMMANDS.Wait:
                            string Query = "SELECT [ID] FROM [Commands] WHERE ( Description = '" + Builder.CommandLine + "' )";
                            BlockingBuildID = DB.ReadInt( Query );
                            BlockingBuildStartTime = DateTime.Now;
                            Mode = MODES.Wait;
                            break;

                        case COMMANDS.WaitForJobs:
                            Log( "[STATUS] Waiting for " + NumJobsToWaitFor.ToString() + " package jobs.", Color.Magenta );
                            NumCompletedJobs = 0;
                            Mode = MODES.WaitForJobs;
                            break;

                        case COMMANDS.GetJobs:
                            CurrentCommand = new Command( this, SCC, Builder );
                            Mode = CurrentCommand.Execute( COMMANDS.GetJobs );

                            PendingCommands.Enqueue( COMMANDS.CleanJobs ); 
                            break;

                        case COMMANDS.SetDependency:
                            Builder.LabelInfo.Init( SCC, Builder );
                            Mode = MODES.Finalise;
                            break;

                        case COMMANDS.CreatePackageJobs:
                            CurrentCommand = new Command( this, SCC, Builder );
                            Mode = CurrentCommand.Execute( COMMANDS.CreatePackageJobs );

                            PendingCommands.Enqueue( COMMANDS.LoadJobs );
                            break;

                        case COMMANDS.MSVCFull:
                            CurrentCommand = new Command( this, SCC, Builder );
                            Mode = CurrentCommand.Execute( COMMANDS.MSVCClean );

                            PendingCommands.Enqueue( COMMANDS.MSVCBuild );
                            break;

                        case COMMANDS.GCCFull:
                            CurrentCommand = new Command( this, SCC, Builder );
                            Mode = CurrentCommand.Execute( COMMANDS.GCCClean );

                            PendingCommands.Enqueue( COMMANDS.GCCBuild );
                            break;

                        case COMMANDS.ShaderFull:
                            CurrentCommand = new Command( this, SCC, Builder );
                            Mode = CurrentCommand.Execute( COMMANDS.ShaderClean );

                            PendingCommands.Enqueue( COMMANDS.ShaderBuild );
                            break;

                        case COMMANDS.UpdateSourceServer:
                            CurrentCommand = new Command( this, SCC, Builder );
                            Mode = CurrentCommand.Execute( COMMANDS.UpdateSourceServer );

                            PendingCommands.Enqueue( COMMANDS.UpdateSymbolServer );
                            break;

                        case COMMANDS.UpdateSymbolServer:
                            CurrentCommand = new Command( this, SCC, Builder );
                            Mode = CurrentCommand.Execute( COMMANDS.UpdateSymbolServer );

                            PendingCommands.Enqueue( COMMANDS.UpdateSymbolServerTick );
                            break;

                        case COMMANDS.UpdateSymbolServerTick:
                            CurrentCommand = new Command( this, SCC, Builder );
                            Mode = CurrentCommand.Execute( COMMANDS.UpdateSymbolServerTick );

                            if( CurrentCommand.GetCurrentBuild() != null )
                            {
                                PendingCommands.Enqueue( COMMANDS.UpdateSymbolServerTick );
                            }
                            break;

                        case COMMANDS.Trigger:
                            string BuildType = Builder.CommandLine;
                            Log( "[STATUS] Triggering build '" + BuildType + "'", Color.Magenta );
                            if( !DB.Trigger( CommandID, BuildType ) )
                            {
                                Log( "[STATUS] Suppressing retrigger of '" + BuildType + "'", Color.Magenta );
                                Mailer.SendAlreadyInProgressMail( DB, Builder, CommandID, BuildType );
                            }
                            break;

                        case COMMANDS.Conform:
                            if( Builder.GetValidLanguages().Count > 2 )
                            {
                                PendingCommands.Enqueue( COMMANDS.Conform );
                            }

                            CurrentCommand = new Command( this, SCC, Builder );
                            Mode = CurrentCommand.Execute( COMMANDS.Conform );
                            break;

                        case COMMANDS.SCC_Submit:
                            CurrentCommand = new Command( this, SCC, Builder );
                            Mode = CurrentCommand.Execute( NextCommand );

                            if( Builder.CreateLabel && SCC.GetErrorLevel() == ERRORS.None )
                            {
                                string CommandString;

                                string Label = Builder.LabelInfo.GetLabelName();
                                if( Promotable > 0 )
                                {
                                    CommandString = "UPDATE Variables SET Value = '" + Label + "' WHERE ( Variable = 'LatestBuild' AND Branch = '" + Builder.SourceControlBranch + "' )";
                                    DB.Update( CommandString );
                                }

                                CommandString = "UPDATE Variables SET Value = '" + Label + "' WHERE ( Variable = 'LatestRawBuild' AND Branch = '" + Builder.SourceControlBranch + "' )";
                                DB.Update( CommandString );
                            }
                            break;

                        case COMMANDS.CheckSpace:
                            try
                            {
                                string FinalStatus = GetDirectoryStatus( Builder );

                                Mailer.SendStatusMail( DB, Builder, CommandID, FinalStatus );
                            }
                            catch
                            {
                            }
                            break;

                        case COMMANDS.UpdateLabel:
                            string UpdateLabel = "UPDATE Variables SET Value = '" + Builder.LabelInfo.GetLabelName() + "' WHERE ( Variable = '" + Builder.CommandLine + "' AND Branch = '" + Builder.SourceControlBranch + "' )";
                            DB.Update( UpdateLabel );
                            break;

                        case COMMANDS.UpdateFolder:
                            string UpdateFolder = "UPDATE Variables SET Value = '" + Builder.LabelInfo.GetFolderName( true ) + "' WHERE ( Variable = '" + Builder.CommandLine + "' AND Branch = '" + Builder.SourceControlBranch + "' )";
                            DB.Update( UpdateFolder );
                            break;
                        
                        case COMMANDS.Publish:
                            if( Builder.BlockOnPublish )
                            {
                                if( !DB.AvailableBandwidth( CommandID ) )
                                {
                                    PendingCommands.Enqueue( COMMANDS.Publish );

                                    if( Builder.StartWaitForConch == Builder.BuildStartedAt )
                                    {
                                        Builder.StartWaitForConch = DateTime.Now;
                                        Builder.LastWaitForConchUpdate = Builder.StartWaitForConch;

                                        Log( "[STATUS] Waiting for network bandwidth ( 00:00:00 )", Color.Yellow );
                                    }
                                    else if( DateTime.Now - Builder.LastWaitForConchUpdate > new TimeSpan( 0, 0, 5 ) )
                                    {
                                        Builder.LastWaitForConchUpdate = DateTime.Now;
                                        TimeSpan Taken = DateTime.Now - Builder.StartWaitForConch;
                                        Log( "[STATUS] Waiting for network bandwidth ( " + Taken.Hours.ToString( "00" ) + ":" + Taken.Minutes.ToString( "00" ) + ":" + Taken.Seconds.ToString( "00" ) + " )", Color.Yellow );
                                    }
                                }
                                else
                                {
                                    Log( "[STATUS] Network bandwidth acquired - publishing!", Color.Magenta );

                                    CurrentCommand = new Command( this, SCC, Builder );
                                    Mode = CurrentCommand.Execute( NextCommand );
                                }
                            }
                            else
                            {
                                CurrentCommand = new Command( this, SCC, Builder );
                                Mode = CurrentCommand.Execute( NextCommand );
                            }
                            break;

                        default:
                            // Fire off a new process safely
                            CurrentCommand = new Command( this, SCC, Builder );
                            Mode = CurrentCommand.Execute( NextCommand );

                            // Handle the returned info (special case)
                            if( NextCommand == COMMANDS.SCC_Sync )
                            {
                                DB.SetInt( "BuildLog", BuildLogID, "ChangeList", Builder.LabelInfo.Changelist );
#if FALSE
                                int LastGoodChangeList = DB.GetInt( "Commands", CommandID, "LastGoodChangeList" );
                                if( LastGoodChangeList >= Builder.BuildingFromChangeList )
                                {
                                    FinalStatus = "Build '" + DB.GetString( "Commands", CommandID, "Description" ) + "' already up to date";
                                    Log( "[STATUS] " + FinalStatus, Color.Magenta );
                                    Mailer.SendUpToDateMail( DB, Builder, CommandID, FinalStatus );
                                    Mode = MODES.Exit;
                                }
#endif
                            }
                            break;
                    }
                    break;

                case MODES.Monitor:
                    // Check for completion
                    Mode = CurrentCommand.IsFinished();
                    break;

                case MODES.Wait:
                    if( BlockingBuildID != 0 )
                    {
                        // Has the child build been updated to the same build?
                        int LastGoodChangeList = DB.GetInt( "Commands", BlockingBuildID, "LastGoodChangeList" );
                        if( LastGoodChangeList >= Builder.LabelInfo.Changelist )
                        {
                            Mode = MODES.Finalise;
                        }
                        else
                        {
                            // Try to get the build log
                            if( BlockingBuildLogID == 0 )
                            {
                                BlockingBuildLogID = DB.GetInt( "Commands", BlockingBuildID, "BuildLogID" );
                            }

                            // Try and get when the build started
                            if( BlockingBuildLogID != 0 )
                            {
                                BlockingBuildStartTime = DB.GetDateTime( "BuildLog", BlockingBuildLogID, "BuildStarted" );
                            }

                            // Check to see if the build timed out (default time is when wait was started)
                            if( DateTime.Now - BlockingBuildStartTime > Builder.GetTimeout() )
                            {
                                Builder.SetErrorLevel( ERRORS.WaitTimedOut );
                                Mode = MODES.Finalise;
                            }
                        }
                    }
                    else
                    {
                        Mode = MODES.Finalise;
                    }
                    break;

                case MODES.WaitForJobs:
                    string WaitQuery = "SELECT COUNT( ID ) FROM Jobs WHERE ( SpawnTime = " + JobSpawnTime.ToString() + " AND Complete = 1 )";
                    int Count = DB.ReadInt( WaitQuery );

                    if( NumCompletedJobs != Count )
                    {
                        int RemainingJobs = NumJobsToWaitFor - Count;
                        Log( "[STATUS] Waiting for " + RemainingJobs.ToString() + " package jobs.", Color.Magenta );
                        NumCompletedJobs = Count;
                    }

                    if( Count == NumJobsToWaitFor )
                    {
                        WaitQuery = "SELECT COUNT( ID ) FROM Jobs WHERE ( SpawnTime = " + JobSpawnTime.ToString() + " AND Succeeded = 1 )";
                        Count = DB.ReadInt( WaitQuery );
                        if( Count != NumJobsToWaitFor )
                        {
                            Builder.SetErrorLevel( ERRORS.FailedJobs );
                        }

                        NumJobsToWaitFor = 0;
                        JobSpawnTime = DateTime.UtcNow.Ticks;
                        Mode = MODES.Finalise;
                    }
                    break;

                case MODES.Finalise:
                    // Analyse logs and restart or exit
                    Mode = HandleError();
                    break;

                case MODES.Exit:
                    Cleanup();
                    break;
            }
        }

        public void Run()
        {
            if( DB == null )
            {
                return;
            }

            // Ping the server to say we're still alive every 30 seconds
            MaintainMachine();

            // Monitor the CPU and mobo temps
            GetTemperatures();

            if( BuildLogID != 0 )
            {
                RunBuild();

                int ID = PollForKill();
                if( ID != 0 )
                {
                    KillBuild( ID );
                }
            }
            else
            {
                // Poll the DB for commands
                int ID = PollForBuild();
                if( ID != 0 )
                {
                    SpawnBuild( ID );
                    return;
                }

                ID = PollForJob();
                if( ID != 0 )
                {
                    SpawnJob( ID );
                    return;
                }

                CheckRestart();
            }
        }

        private void Main_FormClosed( object sender, FormClosedEventArgs e )
        {
            DB.SetString( "Commands", CommandID, "Killer", "LocalUser" );

            KillBuild( CommandID );
            Ticking = false;
        }
    }
}