using System;
using System.Collections.Generic;
using System.ServiceProcess;
using System.Text;
using Microsoft.Win32;
using UnrealSync;
using System.Timers;
using System.Diagnostics;
using System.IO;

namespace UnrealSync.Service
{
    public class UnrealSyncService : System.ServiceProcess.ServiceBase
    {

        private List<SyncJob> syncJobs;
        private Timer runTimer;
        private ServiceHelper helper;

        public UnrealSyncService()
        {
            this.ServiceName = "UnrealSync";
            this.CanStop = true;
            this.CanPauseAndContinue = true;
            this.AutoLog = true;
            runTimer = new Timer();
            syncJobs = new List<SyncJob>();
            helper = new ServiceHelper();
            runTimer.Interval = 60000;
            runTimer.Elapsed += new ElapsedEventHandler(CheckForSyncJobAndRun);
        }

        private void CheckForSyncJobAndRun(Object sender, ElapsedEventArgs e)
        {
            helper.CheckForSyncJobAndRun(false);
        }

        [STAThread]
        static void Main()
        {
            System.ServiceProcess.ServiceBase[] ServicesToRun;
            ServicesToRun = new System.ServiceProcess.ServiceBase[] { new UnrealSyncService() };
            System.ServiceProcess.ServiceBase.Run(ServicesToRun);
        }

        protected override void OnStart(string[] args)
        {
            runTimer.AutoReset = true;
            runTimer.Enabled = true;
        }

        protected override void OnStop()
        {
            runTimer.AutoReset = false;
            runTimer.Enabled = false;
        }

    }
}