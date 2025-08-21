using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Win32;

namespace UnrealSync
{
    public static class AppSettings
    {

        private static RegistryKey GetUnrealSyncKey()
        {
            return Registry.CurrentUser.OpenSubKey("SOFTWARE", true).CreateSubKey("Epic Games").CreateSubKey("UnrealSync");
        }

        public static SyncJob[] GetSyncJobs()
        {
            RegistryKey UnrealSyncKey = GetUnrealSyncKey();
            List<SyncJob> syncJobs = new List<SyncJob>();
            // If the key exists, continue reading the jobs.
            if (UnrealSyncKey != null)
            {
                string[] projects = UnrealSyncKey.GetSubKeyNames();
                for (int i = 0; i < projects.Length; i++)
                {
                    syncJobs.Add(getSyncJobByProjectName(projects[i]));
                }
            }
            return syncJobs.ToArray();
        }

        public static void AddSyncJob(SyncJob jobToAdd)
        {
            RegistryKey UnrealSyncKey = GetUnrealSyncKey();
            UnrealSyncKey.CreateSubKey(jobToAdd.Name);
            UnrealSyncKey.Close();
            UpdateSyncJob(jobToAdd);

        }

        public static void UpdateSyncJob(SyncJob job)
        {
            RegistryKey jobRegKey = GetUnrealSyncKey().OpenSubKey(job.Name, true);
            jobRegKey.SetValue("Enabled", job.Enabled);
            jobRegKey.SetValue("StartTime", job.StartTime);
            jobRegKey.SetValue("PerforceClientSpec", job.PerforceClientSpec);
            jobRegKey.SetValue("BatchFilePath", job.BatchFilePath);
            jobRegKey.SetValue("PostBatchPath", job.PostBatchPath);
            jobRegKey.SetValue("Label", job.Label);
            jobRegKey.SetValue("GameProcessName", job.GameProcessName);
            jobRegKey.SetValue("GameProcessName", job.GameProcessName);
            jobRegKey.SetValue("KillGameProcess", job.KillGameProcess);
            jobRegKey.Close();
        }

        public static SyncJob RemoveSyncJobByName(String jobName)
        {
            SyncJob removedSyncJob = getSyncJobByProjectName(jobName);
            GetUnrealSyncKey().DeleteSubKey(jobName);
            return removedSyncJob;
        }

        public static SyncJob getSyncJobByProjectName(String jobName)
        {
            RegistryKey currentKey = GetUnrealSyncKey().OpenSubKey(jobName);
            SyncJob retrievedJob = new SyncJob(jobName);
            retrievedJob.Enabled = bool.Parse((string)currentKey.GetValue("Enabled"));
            retrievedJob.StartTime = (string)currentKey.GetValue("StartTime");
            retrievedJob.PerforceClientSpec = (string)currentKey.GetValue("PerforceClientSpec");
            retrievedJob.BatchFilePath = (string)currentKey.GetValue("BatchFilePath");
            retrievedJob.PostBatchPath = (string)currentKey.GetValue("PostBatchPath");
            retrievedJob.Label = (string)currentKey.GetValue("Label");
            retrievedJob.GameProcessName = (string)currentKey.GetValue("GameProcessName");
            String killProc = (string)currentKey.GetValue("KillGameProcess");
            if (killProc != null)
            {
                retrievedJob.KillGameProcess = bool.Parse((string)currentKey.GetValue("KillGameProcess"));
            }
            return retrievedJob;
        }

    }
}