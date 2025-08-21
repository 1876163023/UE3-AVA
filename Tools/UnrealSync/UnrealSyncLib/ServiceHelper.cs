using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Diagnostics;
using System.Collections;
using System.Net.Mail;

namespace UnrealSync
{
    public class ServiceHelper
    {

        private static string LOG_SEPARATOR = "============================================================================================";
        private EventLog eventLog;
        private Hashtable logWriterHash;
        private Hashtable jobHash;

        public enum ExecuteStatus {STATUS_OK,STATUS_GAME_RUNNING,STATUS_EXECUTION_PROBLEM};

        public ServiceHelper()
        {
            eventLog = new EventLog();
            eventLog.Source = "UnrealSync Manager";
            logWriterHash = new Hashtable();
            jobHash = new Hashtable();
        }

        // Job Spawner
        public void CheckForSyncJobAndRun(bool debug)
        {
            DateTime currentTime = DateTime.Now;            // current time
            int hour = currentTime.Hour;                    // current hour
            int minute = currentTime.Minute;                // current minute
            SyncJob[] jobs = AppSettings.GetSyncJobs();     // list of sync jobs
            DateTime jobTime;                               // current job time
            SyncJob job;

            // Iterate through the job list to see if one should run in this minute
            for (int i = 0; i < jobs.Length; i++)
            {
                job = jobs[i];
                // Get the current time - getting this before iterating through the list in the event that this takes a while.
                jobTime = jobs[i].getComparableDate();
                if (job.Enabled && (debug || (jobTime.Hour == hour && jobTime.Minute == minute)))
                {
                    ExecuteJob(job,true);
                }
            }
        }

        private static StreamWriter GetSyncJobLog(String jobName)
        {
            return new StreamWriter(File.Open(Path.GetTempPath() + "\\UnrealSync_" + jobName + "_" + DateTime.Now.ToFileTime().ToString() + ".txt",FileMode.Create,FileAccess.ReadWrite));
        }

        public ExecuteStatus ExecuteJob(SyncJob job, bool redirectOutput)
        {
            Process[] gameProcesses = null;                 // holds a list of processes that match a game associated with a sync job
            ExecuteStatus returnCode = ExecuteStatus.STATUS_OK;

            // Get a list of processes that have the process name associated with this sync job.  If
            // no process is defined, skip.  (i.e. there is no process checking)
            if (job.GameProcessName != null && job.GameProcessName.Length > 0)
            {
                gameProcesses = System.Diagnostics.Process.GetProcessesByName(job.GameProcessName);
            }

            // Open the log for the sync job
            StreamWriter logWriter = GetSyncJobLog(job.Name);

            // Write log header
            logWriter.WriteLine(LOG_SEPARATOR);
            logWriter.WriteLine("START DATE/TIME: " + DateTime.Now);
            
            // If the game isn't running, sync.
            if (gameProcesses == null || gameProcesses.Length == 0 || job.KillGameProcess)
            {
                if (job.KillGameProcess)
                {
                    for (int i = 0; i < gameProcesses.Length; i++)
                    {
                        try
                        {
                            gameProcesses[i].Kill();
                        }
                        catch (Exception e)
                        {
                            logWriter.WriteLine(e.Message);
                        }
                    }
                }

                Process p = new Process();                      // sync job;

                eventLog.WriteEntry(job.Name + " sync job starting");

                // Redirect the output.
                p.StartInfo.UseShellExecute = false;
                p.StartInfo.RedirectStandardOutput = redirectOutput;
                p.StartInfo.RedirectStandardError = redirectOutput;
                p.StartInfo.CreateNoWindow = redirectOutput;

                // If a batch file is defined, run it
                if (job.BatchFilePath != null && job.BatchFilePath.Length > 0)
                {
                    p.StartInfo.FileName = job.BatchFilePath;
                    p.StartInfo.WorkingDirectory = job.BatchFilePath.Substring(0, job.BatchFilePath.LastIndexOf('\\'));
                    p.StartInfo.Arguments = job.PerforceClientSpec;
                }
                // If a batch file is not defined, sync to head
                else
                {
                    // If the client spec is defined, use it
                    p.StartInfo.FileName = "p4";
                    p.StartInfo.Arguments = "";
                    if (job.PerforceClientSpec != null && job.PerforceClientSpec.Length > 0)
                    {
                        p.StartInfo.Arguments += "-c " + job.PerforceClientSpec;
                    }
                    p.StartInfo.Arguments += " sync";
                    if (job.Label != null && job.Label.Length > 0)
                    {
                        String label = job.Label;
                        label = label.Replace("%D%", DateTime.Now.ToString("yyyy-MM-dd"));
                        p.StartInfo.Arguments += " @" + label;
                    }
                }

                // Spawn the process - try to start the process, handling thrown exceptions as a failure.
                try
                {
                    p.EnableRaisingEvents = true;
                    if (redirectOutput)
                    {
                        p.OutputDataReceived += new DataReceivedEventHandler(PrintLog);
                        p.ErrorDataReceived += new DataReceivedEventHandler(PrintLog);
                    }
                    p.Exited += new EventHandler(ProcessExit);
                    logWriter.WriteLine("STARTING: " + p.StartInfo.FileName + " " + p.StartInfo.Arguments);
                    logWriter.WriteLine(LOG_SEPARATOR);
                    try
                    {
                        p.Start();
                    }
                    catch (Exception e)
                    {
                        logWriter.WriteLine(e.Message);
                    }
                    jobHash.Add(p.Handle, job);
                    logWriterHash.Add(p.Handle, logWriter);
                    if (redirectOutput)
                    {
                        p.BeginOutputReadLine();
                    }
                    else
                    {
                        logWriter.WriteLine("Sync start successful.  Program output redirected to console window.");
                    }
                }
                catch (Exception ex0)
                {
                    eventLog.WriteEntry(job.Name + "  could not run.  Exception: " + ex0.Message);
                    logWriter.WriteLine(LOG_SEPARATOR);
                    logWriter.WriteLine("There was a problem during execution.");
                    logWriter.WriteLine("ERROR:  " + ex0.Message);
                    returnCode = ExecuteStatus.STATUS_EXECUTION_PROBLEM;
                }
            }
            // If the game is running output an error in the log.
            else
            {
                logWriter.WriteLine(LOG_SEPARATOR);
                logWriter.WriteLine("CANNOT SYNC");
                logWriter.WriteLine(job.GameProcessName + " is running");
                logWriter.WriteLine(LOG_SEPARATOR);
                logWriter.Close();
                logWriterHash.Remove(logWriter);
                logWriterHash.Remove(job);
                return ExecuteStatus.STATUS_GAME_RUNNING;
            }
            return returnCode;
        }

        private void PrintLog(object Sender, DataReceivedEventArgs e)
        {
            IntPtr handle = ((Process)Sender).Handle;
            StreamWriter logWriter = (StreamWriter)logWriterHash[handle];
            string Line = e.Data;
            if (Line != null)
            {
                logWriter.WriteLine(Line);
            }
        }

        private void ProcessExit(object Sender, System.EventArgs e)
        {
            IntPtr handle = ((Process)Sender).Handle;
            StreamWriter logWriter = (StreamWriter)logWriterHash[handle];
            SyncJob job = (SyncJob)jobHash[handle];
            logWriter.WriteLine(LOG_SEPARATOR);
            logWriter.WriteLine("END DATE/TIME: " + DateTime.Now);
            logWriter.WriteLine(LOG_SEPARATOR);
            if (null != job.PostBatchPath && job.PostBatchPath.Length > 0)
            {
                try
                {
                    logWriter.WriteLine(LOG_SEPARATOR);
                    logWriter.WriteLine("Post sync batch " + job.PostBatchPath + " starting.");
                    logWriter.WriteLine(LOG_SEPARATOR);
                    Process postProcess = new Process();
                    postProcess.StartInfo.FileName = job.PostBatchPath;
                    postProcess.StartInfo.UseShellExecute = false;
                    postProcess.StartInfo.RedirectStandardOutput = true;
                    postProcess.StartInfo.RedirectStandardError = true;
                    postProcess.StartInfo.CreateNoWindow = false;
                    postProcess.EnableRaisingEvents = true;
                    postProcess.OutputDataReceived += new DataReceivedEventHandler(PrintLog);
                    postProcess.ErrorDataReceived += new DataReceivedEventHandler(PrintLog);
                    postProcess.Exited += new EventHandler(PostProcessExit);
                    try
                    {
                        postProcess.Start();
                    }
                    catch (Exception e2)
                    {
                        logWriter.WriteLine(e2.Message);
                    }
                    logWriterHash.Remove(handle);
                    jobHash.Remove(handle);
                    jobHash.Add(postProcess.Handle, job);
                    logWriterHash.Add(postProcess.Handle, logWriter);
                    postProcess.BeginOutputReadLine();
                    postProcess.WaitForExit();
                }
                catch (Exception ex0)
                {
                    eventLog.WriteEntry(job.Name + " post sync batch could not run.  Exception: " + ex0.Message);
                    logWriter.WriteLine(LOG_SEPARATOR);
                    logWriter.WriteLine("There was a problem during execution of the post sync batch.");
                    logWriter.WriteLine("ERROR:  " + ex0.Message);
                    EmailLog(logWriter, job);
                    logWriterHash.Remove(handle);
                    jobHash.Remove(handle);
                }
            }
            else
            {
                logWriter.Flush();
                EmailLog(logWriter, job);
                logWriterHash.Remove(handle);
                jobHash.Remove(handle);
            }
        }

        private static void EmailLog(StreamWriter logWriter, SyncJob job)
        {
            try
            {
                MailAddress currentUserEmail = new MailAddress(System.Security.Principal.WindowsIdentity.GetCurrent().Name.ToString().Split('\\')[1] + "@epicgames.net");
                MailMessage logEmail = new MailMessage(currentUserEmail, currentUserEmail);
                StreamReader logReader = new StreamReader(logWriter.BaseStream);
                SmtpClient smtpClient = new SmtpClient("mail-01.epicgames.net");
                logEmail.BodyEncoding = Encoding.ASCII;
                logEmail.Subject = "[UNREALSYNC] " + job.Name + " Sync Job";
                logEmail.Priority = MailPriority.Normal;
                logReader.BaseStream.Position = 0;
                logEmail.Body = logReader.ReadToEnd();
                smtpClient.Send(logEmail);
                logReader.Close();
            }
            catch { }
        }

        private void PostProcessExit(object Sender, System.EventArgs e) {
            IntPtr handle = ((Process)Sender).Handle;
            StreamWriter logWriter = (StreamWriter)logWriterHash[handle];
            SyncJob job = (SyncJob)jobHash[handle];
            EmailLog(logWriter, job);
            logWriterHash.Remove(handle);
            jobHash.Remove(handle);
        }

    }
}
