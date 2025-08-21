using System;
using System.Collections.Generic;
using System.Text;

namespace UnrealSync
{
    public class SyncJob
    {
        private string name;
        private bool enabled = false;
        private bool killGameProcess = false;
        private string startTime = "2:00 AM";
        private string perforceClientSpec = "";
        private string batchFilePath = "";
        private string gameProcessName = "";
        private string label = "";
        private string postBatchPath = "";

        public bool KillGameProcess
        {
            get { return killGameProcess; }
            set { killGameProcess = value; }
        }

        public string PostBatchPath
        {
            get { return postBatchPath; }
            set { postBatchPath = value; }
        }

        public string Label
        {
            get { return label; }
            set { label = value; }
        }

        public SyncJob(String name)
        {
            this.name = name;
        }

        public string Name
        {
            get { return name; }
            set { name = value; }
        }

        public bool Enabled
        {
            get { return enabled; }
            set { enabled = value; }
        }

        public string StartTime
        {
            get { return startTime; }
            set { startTime = value; }
        }

        public string PerforceClientSpec
        {
            get { return perforceClientSpec; }
            set { perforceClientSpec = value; }
        }

        public string BatchFilePath
        {
            get { return batchFilePath; }
            set { batchFilePath = value; }
        }

        public string GameProcessName
        {
            get { return gameProcessName; }
            set { gameProcessName = value; }
        }

        public DateTime getComparableDate()
        {
            return DateTime.Parse(startTime);
        }

    }
}
