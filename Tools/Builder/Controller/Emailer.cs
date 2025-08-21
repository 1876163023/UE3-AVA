using System;
using System.Collections.Generic;
using System.Drawing;
using System.Management;
using System.Net;
using System.Net.Mail;
using System.Text;
using Controller;

namespace Controller
{
    class Emailer
    {
        string MailServer = Properties.Settings.Default.MailServer;

        Main Parent;

        public Emailer( Main InParent )
        {
            Parent = InParent;
            Parent.Log( "Emailer created successfully", Color.DarkGreen );
        }

        private void SendMail( string To, string Subject, string Body, MailPriority Priority )
        {
#if DEBUG
            To = "john.scott@epicgames.com";
#endif
            try
            {
                SmtpClient Client = new SmtpClient( MailServer );
                if( Client != null )
                {
                    MailAddress AddrTo;

                    MailMessage Message = new MailMessage();

                    // Client.Credentials = new System.Net.NetworkCredential( "user", "pass" );
                    // Client.UseDefaultCredentials = false;

                    Message.From = new MailAddress( "build@epicgames.com", Parent.MachineName );
                    Message.Priority = Priority;

                    if( To.Length > 0 )
                    {
                        string[] Addresses = To.Split( ';' );
                        foreach( string Address in Addresses )
                        {
                            if( Address.Length > 0 )
                            {
                                AddrTo = new MailAddress( Address );
                                Message.To.Add( AddrTo );
                            }
                        }
                    }
#if !DEBUG
                    AddrTo = new MailAddress( "qa@epicgames.com" );
                    Message.CC.Add( AddrTo );

                    AddrTo = new MailAddress( "john.scott@epicgames.com" );
                    Message.Bcc.Add( AddrTo );

                    AddrTo = new MailAddress( "wes.hunt@epicgames.com" );
                    Message.Bcc.Add( AddrTo );

                    AddrTo = new MailAddress( "josh.adams@epicgames.com" );
                    Message.Bcc.Add( AddrTo );
#endif
                    Message.Subject = Subject;
                    Message.Body = Body;
                    Message.IsBodyHtml = false;

                    Client.Send( Message );

                    Parent.Log( "Email sent to: " + To, Color.Orange );
                }
            }
            catch( Exception e )
            {
                Parent.Log( "Failed to send email to: " + To, Color.Orange );
                Parent.Log( "'" + e.Message + "'", Color.Orange );
            }
        }

        private string BuildTime( BuilderDB DB, string BuildType, int BuildLogID )
        {
            StringBuilder Result = new StringBuilder();

            DateTime Start = DB.GetDateTime( "BuildLog", BuildLogID, "BuildStarted" );
            DateTime End = DB.GetDateTime( "BuildLog", BuildLogID, "BuildEnded" );

            Result.Append( "'" + BuildType + "' started at " + Start );
            Result.Append( " and ended at " + End );
            Result.Append( " taking " );

            TimeSpan Duration = End - Start;
            if( Duration.Hours > 0 )
            {
                Result.Append( Duration.Hours.ToString() + " hour(s) " );
            }
            if( Duration.Hours > 0 || Duration.Minutes > 0 )
            {
                Result.Append( Duration.Minutes.ToString() + " minute(s) " );
            }
            Result.Append( Duration.Seconds.ToString() + " second(s)\r\n" );

            return( Result.ToString() );
        }

        private string AddOperator( BuilderDB DB, int CommandID, string To )
        {
            string Operator = DB.GetString( "Commands", CommandID, "Operator" );
            if( Operator.Length > 0 && Operator != "AutoTimer" && Operator != "LocalUser" )
            {
                if( To.Length > 0 )
                {
                    To += ";";
                }
                To += Operator + "@epicgames.com";
            }
            return ( To );
        }

        private string AddKiller( BuilderDB DB, int CommandID, string To )
        {
            string Killer = DB.GetString( "Commands", CommandID, "Killer" );
            if( Killer.Length > 0 && Killer != "LocalUser" )
            {
                if( To.Length > 0 )
                {
                    To += ";";
                }
                To += Killer + "@epicgames.com";
            }

            return ( To );
        }

        private string GetWMIValue( string Key, ManagementObject ManObject )
        {
            Object Value;

            Value = ManObject.GetPropertyValue( Key );
            if( Value != null )
            {
                return ( Value.ToString() );
            }

            return ( "" );
        }

        public void SendRestartedMail()
        {
            StringBuilder Body = new StringBuilder();

            string Subject = "[BUILDER] Builder synced and restarted!";

            Body.Append( "Controller compiled: " + Parent.CompileDateTime.ToString() + "\r\n\r\n" );
            Body.Append( "MSVC version: " + Parent.MSVCVersion + "\r\n" );
            Body.Append( "DirectX version: " + Parent.DXVersion + "\r\n" );
            Body.Append( "XDK version: " + Parent.XDKVersion + "\r\n" );
            Body.Append( "PS3 SDK version: " + Parent.PS3SDKVersion + "\r\n" );
            Body.Append( "\r\nPath: " + Environment.GetEnvironmentVariable( "PATH" ) + "\r\n\r\n" );

            ManagementObjectSearcher Searcher = new ManagementObjectSearcher( "Select * from CIM_Processor" );
            ManagementObjectCollection Collection = Searcher.Get();

            foreach( ManagementObject Object in Collection )
            {
                Body.Append( GetWMIValue( "Name", Object ) + "\r\n" );
                break;
            }

            Searcher = new ManagementObjectSearcher( "Select * from CIM_BIOSElement" );
            Collection = Searcher.Get();

            foreach( ManagementObject Object in Collection )
            {
                Body.Append( GetWMIValue( "Name", Object ) + "\r\n" );
            }

            Body.Append( "\r\n" );

            Searcher = new ManagementObjectSearcher( "Select * from CIM_PhysicalMemory" );
            Collection = Searcher.Get();

            foreach( ManagementObject Object in Collection )
            {
                string Capacity = GetWMIValue( "Capacity", Object );
                string Speed = GetWMIValue( "Speed", Object );
                Body.Append( "Memory: " + Capacity + " bytes at " + Speed + " MHz\r\n" );
            }

            Body.Append( "\r\n" );

            Searcher = new ManagementObjectSearcher( "Select * from Win32_LogicalDisk" );
            Collection = Searcher.Get();

            foreach( ManagementObject Object in Collection )
            {
                string DriveType = GetWMIValue( "DriveType", Object );
                if( DriveType == "3" )
                {
                    Int64 Size = 0;
                    Int64 FreeSpace = 0;

                    string Name = GetWMIValue( "Caption", Object );
                    string SizeInfo = GetWMIValue( "Size", Object );
                    string FreeSpaceInfo = GetWMIValue( "FreeSpace", Object );

                    try
                    {
                        Size = Int64.Parse( SizeInfo ) / ( 1024 * 1024 * 1024 );
                        FreeSpace = Int64.Parse( FreeSpaceInfo ) / ( 1024 * 1024 * 1024 );
                    }
                    catch
                    {
                    }

                    Body.Append( "'" + Name + "' hard disk: " + Size.ToString() + "GB (" + FreeSpace.ToString() + "GB free)\r\n" );
                }
            }

            Body.Append( "\r\nCheers\r\nBuilder\r\n" );

            SendMail( "john.scott@epicgames.com", Subject, Body.ToString(), MailPriority.Low );
        }

        public void SendTriggeredMail( BuilderDB DB, ScriptParser Builder, int CommandID )
        {
            string To = AddOperator( DB, CommandID, Builder.TriggerAddress );

            string Subject = "[BUILDER] You triggered a build!";
            StringBuilder Body = new StringBuilder();

            Body.Append( "Build type: '" + DB.GetString( "Commands", CommandID, "Description" ) + "'\r\n" );
            Body.Append( "\r\nCheers\r\nBuilder\r\n" );

            SendMail( To, Subject, Body.ToString(), MailPriority.Low );
        }

        public void SendKilledMail( BuilderDB DB, ScriptParser Builder, int CommandID, int BuildLogID )
        {
            string To = AddOperator( DB, CommandID, "" );
            To = AddKiller( DB, CommandID, To );

            string BuildType = DB.GetString( "Commands", CommandID, "Description" );

            string Subject = "[BUILDER] '" + BuildType + "' KIA!";
            StringBuilder Body = new StringBuilder();

            Body.Append( "Build type: '" + BuildType );
            Body.Append( "' from changelist " + DB.GetInt( "BuildLog", BuildLogID, "ChangeList" ) + "\r\n" );
            Body.Append( "Build started at " + DB.GetDateTime( "BuildLog", BuildLogID, "BuildStarted" ) );
            Body.Append( " and ended at " + DB.GetDateTime( "BuildLog", BuildLogID, "BuildEnded" ) + "\r\n" );
            Body.Append( "Started by: " + DB.GetString( "Commands", CommandID, "Operator" ) + "\r\n" );
            Body.Append( "Killed by: " + DB.GetString( "Commands", CommandID, "Killer" ) + "\r\n" );
            Body.Append( "\r\nCheers\r\nBuilder\r\n" );

            SendMail( To, Subject, Body.ToString(), MailPriority.High );
        }

        public void SendFailedMail( BuilderDB DB, ScriptParser Builder, int CommandID, int BuildLogID, string FailureMessage, string LogFileName, string OptionalDistro )
        {
            string Subject = "[BUILDER] Job failed!";
            string BuildType = "Job";
            string To = Builder.FailAddress;

            if( CommandID != 0 )
            {
                To = AddOperator( DB, CommandID, Builder.FailAddress + ";" + OptionalDistro );
                BuildType = DB.GetString( "Commands", CommandID, "Description" );
                Subject = "[BUILDER] '" + BuildType + "' failed!";
            }

            StringBuilder Body = new StringBuilder();

            Body.Append( "Build type: '" + BuildType );
            Body.Append( "' from changelist " + DB.GetInt( "BuildLog", BuildLogID, "ChangeList" ) + "\r\n" );
            Body.Append( "Build started at " + DB.GetDateTime( "BuildLog", BuildLogID, "BuildStarted" ) );
            Body.Append( " and ended at " + DB.GetDateTime( "BuildLog", BuildLogID, "BuildEnded" ) + "\r\n" );
            Body.Append( "\r\nDetailed log copied to: '" + Properties.Settings.Default.FailedLogLocation + "/" + LogFileName + "'\r\n" );
            Body.Append( "\r\n" + FailureMessage + "\r\n" );
            Body.Append( "\r\nCheers\r\nBuilder\r\n" );

            SendMail( To, Subject, Body.ToString(), MailPriority.High );
        }

        // Sends mail stating the version of the build that was just created
        public void SendSucceededMail( BuilderDB DB, ScriptParser Builder, int CommandID, int BuildLogID, string FinalStatus )
        {
            string To = AddOperator( DB, CommandID, Builder.SuccessAddress );
            
            string BuildType = DB.GetString( "Commands", CommandID, "Description" );

            string Label = Builder.LabelInfo.GetLabelName();
            string Subject = "[BUILDER] New '" + BuildType + "' build! (" + Label + ")";
            StringBuilder Body = new StringBuilder();

            Body.Append( BuildTime( DB, BuildType, BuildLogID ) );

            if( Builder.CreateLabel )
            {
                if( Label.Length > 0 )
                {
                    Body.Append( "\r\nBuild is labeled '" + Label + "'\r\n\r\n" );
                }
            }

            Body.Append( "\r\n" + FinalStatus + "\r\n" );
            Body.Append( "\r\nCheers\r\nBuilder\r\n" );

            SendMail( To, Subject, Body.ToString(), MailPriority.Normal );
        }

        // Sends mail stating the version of the build that was used to create the data
        public void SendPromotedMail( BuilderDB DB, ScriptParser Builder, int CommandID )
        {
            string To = AddOperator( DB, CommandID, Builder.SuccessAddress );
            
            string BuildType = DB.GetString( "Commands", CommandID, "Description" );

            string Subject = "[BUILDER] '" + BuildType + "' Promoted! (" + Builder.LabelInfo.GetLabelName() + ")";
            StringBuilder Body = new StringBuilder();

            string Label = Builder.LabelInfo.GetLabelName();
            if( Label.Length > 0 )
            {
                Body.Append( "\r\nThe build labeled '" + Label + "' was promoted (details below)\r\n\r\n" );
            }

            Body.Append( "\r\n" + Builder.GetChanges() + "\r\n" );
            Body.Append( "\r\nCheers\r\nBuilder\r\n" );

            SendMail( To, Subject, Body.ToString(), MailPriority.Normal );
        }

        public void SendPublishedMail( BuilderDB DB, ScriptParser Builder, int CommandID, int BuildLogID )
        {
            string To = AddOperator( DB, CommandID, Builder.SuccessAddress );
            
            string BuildType = DB.GetString( "Commands", CommandID, "Description" );

            string Subject = "[BUILDER] '" + BuildType + "' Published! (" + Builder.LabelInfo.GetFolderName( false ) + ")";
            StringBuilder Body = new StringBuilder();

            Body.Append( BuildTime( DB, BuildType, BuildLogID ) );

            List<string> Dests = Builder.GetPublishDestinations();
            if( Dests.Count > 0 )
            {
                Body.Append( "\r\nBuild was published to -\r\n" );
                foreach( string Dest in Dests )
                {
                    Body.Append( "\t'" + Dest + "'\r\n" );
                }
            }

            string Label = Builder.SyncedLabel;
            if( Label.Length > 0 )
            {
                Body.Append( "\r\nThe build labeled '" + Label + "' was used to cook the data (details below)\r\n\r\n" );
            }

            Body.Append( "\r\n" + Builder.GetChanges() + "\r\n" );
            Body.Append( "\r\nCheers\r\nBuilder\r\n" );

            SendMail( To, Subject, Body.ToString(), MailPriority.Normal );
        }

        public void SendMakingInstallMail( BuilderDB DB, ScriptParser Builder, int CommandID, int BuildLogID )
        {
            string To = AddOperator( DB, CommandID, Builder.SuccessAddress );

            string BuildType = DB.GetString( "Commands", CommandID, "Description" );

            string Subject = "[BUILDER] '" + BuildType + "' installable made! (" + Builder.Dependency + "_Install)";
            StringBuilder Body = new StringBuilder();

            Body.Append( BuildTime( DB, BuildType, BuildLogID ) );

            Body.Append( "\r\nInstall files were copied to -\r\n" );
            Body.Append( "\t'" + Builder.Dependency + "_Install'\r\n" );

            string Label = Builder.LabelInfo.GetLabelName();
            if( Label.Length > 0 )
            {
                Body.Append( "\r\nThe build labeled '" + Label + "' was used to cook the data (details below)\r\n\r\n" );
            }

            Body.Append( "\r\n" + Builder.GetChanges() + "\r\n" );
            Body.Append( "\r\nCheers\r\nBuilder\r\n" );

            SendMail( To, Subject, Body.ToString(), MailPriority.Normal );
        }

        public void SendUpToDateMail( BuilderDB DB, ScriptParser Builder, int CommandID, string FinalStatus )
        {
            string To = AddOperator( DB, CommandID, "" );

            string BuildType = DB.GetString( "Commands", CommandID, "Description" );

            string Subject = "[BUILDER] '" + BuildType + "' already up to date!";
            StringBuilder Body = new StringBuilder();

            Body.Append( "\r\n" + FinalStatus + "\r\n" );
            Body.Append( "\r\nCheers\r\nBuilder\r\n" );

            SendMail( To, Subject, Body.ToString(), MailPriority.Normal );
        }

        public void SendAlreadyInProgressMail( BuilderDB DB, ScriptParser Builder, int CommandID, string BuildType )
        {
            string To = AddOperator( DB, CommandID, "" );

            string Subject = "[BUILDER] '" + BuildType + "' is already building!";
            StringBuilder Body = new StringBuilder();

            string FinalStatus = "Build '" + BuildType + "' not retriggered because it is already building.";

            Body.Append( "\r\n" + FinalStatus + "\r\n" );
            Body.Append( "\r\nCheers\r\nBuilder\r\n" );

            SendMail( To, Subject, Body.ToString(), MailPriority.Normal );
        }

        public void SendStatusMail( BuilderDB DB, ScriptParser Builder, int CommandID, string FinalStatus )
        {
            string To = AddOperator( DB, CommandID, Builder.SuccessAddress );

            string Subject = "[BUILDER] Drive status";
            StringBuilder Body = new StringBuilder();

            Body.Append( "Build storage summary\r\n" );
            Body.Append( "\r\n" + FinalStatus + "\r\n" );
            Body.Append( "\r\nCheers\r\nBuilder\r\n" );

            SendMail( To, Subject, Body.ToString(), MailPriority.Normal );
        }

        public void SendGlitchMail()
        {
            string To = "john.scott@epicgames.com";

            string Subject = "[BUILDER] NETWORK GLITCH";
            StringBuilder Body = new StringBuilder();

            Body.Append( "\r\nNetwork diagnostics\r\n\r\n" );

            ManagementObjectSearcher Searcher = new ManagementObjectSearcher( "Select * from Win32_NetworkConnection" );
            ManagementObjectCollection Collection = Searcher.Get();

            foreach( ManagementObject Object in Collection )
            {
                Body.Append( GetWMIValue( "LocalName", Object ) + "\r\n" );
                Body.Append( GetWMIValue( "Name", Object ) + "\r\n" );
                Body.Append( GetWMIValue( "RemoteName", Object ) + "\r\n" );
                Body.Append( GetWMIValue( "ProviderName", Object ) + "\r\n" );
                Body.Append( GetWMIValue( "ResourceType", Object ) + "\r\n" );

                Body.Append( GetWMIValue( "Caption", Object ) + "\r\n" );
                Body.Append( GetWMIValue( "ConnectionState", Object ) + "\r\n" );
                Body.Append( GetWMIValue( "ConnectionType", Object ) + "\r\n" );
                Body.Append( GetWMIValue( "DisplayType", Object ) + "\r\n" );
                Body.Append( GetWMIValue( "Status", Object ) + "\r\n" );
                Body.Append( "\r\n" );
            }

            Body.Append( "\r\nCheers\r\nBuilder\r\n" );

            SendMail( To, Subject, Body.ToString(), MailPriority.High );
        }

        public void SendTemperatureWarning( int CPUTemp, int MoboTemp )
        {
            string To = "john.scott@epicgames.com";

            string Subject = "[BUILDER] TEMPERATURE WARNING!";
            StringBuilder Body = new StringBuilder();

            Body.Append( "\r\nCPU is running at " + CPUTemp.ToString() + "\r\n" );
            Body.Append( "\r\nMotherboard is running at " + MoboTemp.ToString() + "\r\n" );

            Body.Append( "\r\nCheers\r\nBuilder\r\n" );

            SendMail( To, Subject, Body.ToString(), MailPriority.High );
        }
    }
}
