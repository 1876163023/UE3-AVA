using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using UnrealSync;
using System.IO;

namespace UnrealSync.Manager
{
    public partial class MgrForm : Form
    {

        private const String DEFAULT_BUILD_FOLDER = "D:\\build";
        private enum action { ADD, EDIT }; 
        private action currentAction;
        private ServiceHelper helper;
        private String selectedJobName;
        private bool formDirty = false;

        public MgrForm()
        {
            InitializeComponent();
            LoadJobs();
            UpdateServiceStatus();
            currentAction = action.EDIT;
            helper = new ServiceHelper();
        }

        private void LoadJobs() {
            lstbxSyncJobs.Items.Clear();
            SyncJob[] jobs = AppSettings.GetSyncJobs();
            for (int i = 0; i < jobs.Length; i++)
            {
                lstbxSyncJobs.Items.Add(jobs[i].Name);
            }
            if (lstbxSyncJobs.Items.Count > 0)
            {
                lstbxSyncJobs.SelectedIndex = 0;
                lstbxSyncJobs.Enabled = true;
            }
            else
            {
                lstbxSyncJobs.Enabled = false;
                btnDelete.Enabled = false;
                btnRun.Enabled = false;
                EnableControls(false);
            }
        }

        private void btnClose_Click(object sender, EventArgs e)
        {
            if (formDirty)
            {
                DialogResult userResponse = MessageBox.Show(this, "Would you like to save your changes?", "Save Changes?", MessageBoxButtons.YesNoCancel);
                if (!userResponse.Equals(DialogResult.Cancel))
                {
                    if (userResponse.Equals(DialogResult.Yes))
                    {
                        SaveJob();
                    }
                    Application.Exit();
                }
            }
            else
            {
                Application.Exit();
            }
        }

        private void lstbxSyncJobs_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (lstbxSyncJobs.SelectedItem != null)
            {
                SyncJob selectedJob = AppSettings.getSyncJobByProjectName(lstbxSyncJobs.SelectedItem.ToString());
                txtJobName.Text = selectedJob.Name;
                txtBatchFilePath.Text = selectedJob.BatchFilePath;
                txtPostBatchPath.Text = selectedJob.PostBatchPath;
                txtGameProcessName.Text = selectedJob.GameProcessName;
                txtClientSpec.Text = selectedJob.PerforceClientSpec;
                txtLabel.Text = selectedJob.Label;
                dtpSyncTime.Value = selectedJob.getComparableDate();
                chkEnabled.Checked = selectedJob.Enabled;
                chkKillProcess.Checked = selectedJob.KillGameProcess;
                currentAction = action.EDIT;
                btnDelete.Enabled = true;
                btnRun.Enabled = true;
                selectedJobName = lstbxSyncJobs.SelectedItem.ToString();

                if (selectedJob.BatchFilePath != null && selectedJob.BatchFilePath.Length > 0)
                {
                    rdoBatch.Checked = true;
                }
                else if (selectedJob.Label != null && selectedJob.Label.Length > 0)
                {
                    rdoLabel.Checked = true;
                }
                else
                {
                    rdoHead.Checked = true;
                }
                markFormClean();
            }
            else
            {
                EnableControls(false);
                btnDelete.Enabled = false;
                btnRun.Enabled = false;
                selectedJobName = "";
                clearJobDetails();
            }
        }

        private void markFormClean()
        {
            formDirty = false;
        }

        private void EnableControls(bool enable)
        {
            txtJobName.Enabled = enable;
            txtGameProcessName.Enabled = enable;
            txtClientSpec.Enabled = enable;
            txtPostBatchPath.Enabled = enable;
            dtpSyncTime.Enabled = enable;
            chkEnabled.Enabled = enable;
            btnSave.Enabled = enable;
            btnBatchBrowse.Enabled = enable;
            btnPostBatchBrowse.Enabled = enable;
            rdoBatch.Enabled = enable;
            rdoHead.Enabled = enable;
            rdoLabel.Enabled = enable;
            chkKillProcess.Enabled = enable;
        }

        private void btnBatchBrowse_Click(object sender, EventArgs e)
        {
            if (txtBatchFilePath.Text.Length > 0 && File.Exists(txtBatchFilePath.Text))
            {
                ofdBatchFile.FileName = txtBatchFilePath.Text;
            }
            else
            {
                ofdBatchFile.FileName = "Select your batch file";
                ofdBatchFile.InitialDirectory = DEFAULT_BUILD_FOLDER;
            }
            if (DialogResult.OK == ofdBatchFile.ShowDialog(this))
            {
                txtBatchFilePath.Text = ofdBatchFile.FileName;
            }
        }

        private void btnDelete_Click(object sender, EventArgs e)
        {
            AppSettings.RemoveSyncJobByName(lstbxSyncJobs.SelectedItem.ToString());
            LoadJobs();
        }

        private void btnAdd_Click(object sender, EventArgs e)
        {
            clearJobDetails();
            lstbxSyncJobs.SelectedIndex = -1;
            EnableControls(true);
            currentAction = action.ADD;
        }

        private void clearJobDetails()
        {
            txtJobName.Text = "";
            txtBatchFilePath.Text = "";
            txtPostBatchPath.Text = "";
            txtClientSpec.Text = "";
            txtGameProcessName.Text = "";
            txtLabel.Text = "";
            dtpSyncTime.Value = DateTime.Parse("2:00 AM");
            chkEnabled.Checked = true;
            chkKillProcess.Checked = true;
            rdoBatch.Checked = true;
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            LoadJobs();
        }

        private void btnSave_Click(object sender, EventArgs e)
        {
            if (txtJobName.Text.Trim().Length == 0)
            {
                MessageBox.Show(this, "You must enter a job name.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            else
            {
                if (rdoBatch.Checked)
                {
                    if (txtBatchFilePath.Text.Length == 0 || txtClientSpec.Text.Length == 0)
                    {
                        MessageBox.Show(this, "You must enter a client spec and batch file path or select a different sync method.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }
                    else
                    {
                        SaveJob();
                    }
                }
                else if (rdoLabel.Checked && txtLabel.Text.Length == 0)
                {
                    MessageBox.Show(this, "You must enter label or select a different sync method.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
                else
                {
                    SaveJob();
                }
            }
        }

        private void SaveJob()
        {
            SyncJob job = new SyncJob(txtJobName.Text);
            job.BatchFilePath = txtBatchFilePath.Text;
            job.PostBatchPath = txtPostBatchPath.Text;
            job.GameProcessName = txtGameProcessName.Text;
            job.PerforceClientSpec = txtClientSpec.Text;
            job.StartTime = dtpSyncTime.Value.ToShortTimeString();
            job.Label = txtLabel.Text;
            job.Enabled = chkEnabled.Checked;
            job.KillGameProcess = chkKillProcess.Checked;

            if (currentAction == action.EDIT)
            {
                if (!selectedJobName.Equals(job.Name))
                {
                    AppSettings.RemoveSyncJobByName(selectedJobName);
                    AppSettings.AddSyncJob(job);
                }
                else
                {
                    AppSettings.UpdateSyncJob(job);
                }
            }
            else
            {
                AppSettings.AddSyncJob(job);
            }
            LoadJobs();
        }

        private void btnStartStop_Click(object sender, EventArgs e)
        {
            System.ServiceProcess.ServiceControllerStatus UnrealSyncStatus = UnrealSyncServiceController.Status;
            if (UnrealSyncStatus == System.ServiceProcess.ServiceControllerStatus.Running)
            {
                try
                {
                    UnrealSyncServiceController.Stop();
                }
                catch { }
            }
            else if (UnrealSyncStatus == System.ServiceProcess.ServiceControllerStatus.Stopped)
            {
                try
                {
                    UnrealSyncServiceController.Start();
                }
                catch { }
            }
            UpdateServiceStatus();
        }

        private void btnStatusRefresh_Click(object sender, EventArgs e)
        {
            UpdateServiceStatus();
        }

        private void UpdateServiceStatus()
        {
            UnrealSyncServiceController.Refresh();
            System.ServiceProcess.ServiceControllerStatus UnrealSyncStatus = UnrealSyncServiceController.Status;
            btnServiceControl.Enabled = true;
            if (UnrealSyncStatus == System.ServiceProcess.ServiceControllerStatus.Running)
            {
                lblServiceStatus.Text = "Running";
                btnServiceControl.Text = "Stop";
            }
            else if (UnrealSyncStatus == System.ServiceProcess.ServiceControllerStatus.Stopped)
            {
                lblServiceStatus.Text = "Stopped";
                btnServiceControl.Text = "Start";
            }
            else
            {
                UpdateServiceStatus();
            }
        }

        private void btnRun_Click(object sender, EventArgs e)
        {
            SyncJob job = AppSettings.getSyncJobByProjectName(lstbxSyncJobs.SelectedItem.ToString());
            ServiceHelper.ExecuteStatus jobStatus = helper.ExecuteJob(job,false);
            if (jobStatus == ServiceHelper.ExecuteStatus.STATUS_EXECUTION_PROBLEM)
            {
                MessageBox.Show("There was a problem executing your sync job.  Check the log file for details.");
            }
            else if (jobStatus == ServiceHelper.ExecuteStatus.STATUS_GAME_RUNNING)
            {
                MessageBox.Show("The game associated with this job is currently running.  Please close it and try again.");
            }
        }

        private void rdoBatch_CheckedChanged(object sender, EventArgs e)
        {
            syncOptionChanged();
        }

        private void rdoLabel_CheckedChanged(object sender, EventArgs e)
        {
            syncOptionChanged();
        }

        private void rdoHead_CheckedChanged(object sender, EventArgs e)
        {
            syncOptionChanged();
        }

        private void syncOptionChanged()
        {
            if (rdoBatch.Checked == true)
            {
                txtBatchFilePath.Enabled = true;
                btnBatchBrowse.Enabled = true;
                txtLabel.Text = "";
                txtLabel.Enabled = false;
            }
            else if (rdoLabel.Checked == true)
            {
                txtLabel.Enabled = true;
                txtBatchFilePath.Text = "";
                txtBatchFilePath.Enabled = false;
                btnBatchBrowse.Enabled = false;
            }
            else if (rdoHead.Checked == true)
            {
                txtBatchFilePath.Text = "";
                txtBatchFilePath.Enabled = false;
                btnBatchBrowse.Enabled = false; 
                txtLabel.Text = "";
                txtLabel.Enabled = false;

            }
        }

        private void btnPostBatchBrowse_Click(object sender, EventArgs e)
        {
            if (txtPostBatchPath.Text.Length > 0 && File.Exists(txtPostBatchPath.Text))
            {
                ofdBatchFile.FileName = txtPostBatchPath.Text;
            }
            else
            {
                ofdBatchFile.FileName = "Select your batch file";
                ofdBatchFile.InitialDirectory = DEFAULT_BUILD_FOLDER;
            }
            if (DialogResult.OK == ofdBatchFile.ShowDialog(this))
            {
                txtPostBatchPath.Text = ofdBatchFile.FileName;
            }
        }

        private void txtJobName_TextChanged(object sender, EventArgs e)
        {
            markFormDirty();
        }

        private void markFormDirty()
        {
            formDirty = true;
        }

        private void txtClientSpec_TextChanged(object sender, EventArgs e)
        {
            markFormDirty();
        }

        private void dtpSyncTime_ValueChanged(object sender, EventArgs e)
        {
            markFormDirty();
        }

        private void txtGameProcessName_TextChanged(object sender, EventArgs e)
        {
            markFormDirty();
        }

        private void chkKillProcess_CheckedChanged(object sender, EventArgs e)
        {
            markFormDirty();
        }

        private void txtBatchFilePath_TextChanged(object sender, EventArgs e)
        {
            markFormDirty();
        }

        private void txtLabel_TextChanged(object sender, EventArgs e)
        {
            markFormDirty();
        }

        private void txtPostBatchPath_TextChanged(object sender, EventArgs e)
        {
            markFormDirty();
        }

        private void chkEnabled_CheckedChanged(object sender, EventArgs e)
        {
            markFormDirty();
        }

    }
}