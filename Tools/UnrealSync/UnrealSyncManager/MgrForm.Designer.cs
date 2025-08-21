namespace UnrealSync.Manager
{
    partial class MgrForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MgrForm));
            this.btnClose = new System.Windows.Forms.Button();
            this.grpAddEditSyncJob = new System.Windows.Forms.GroupBox();
            this.grpJobDetails = new System.Windows.Forms.GroupBox();
            this.chkKillProcess = new System.Windows.Forms.CheckBox();
            this.lblPostBatchPath = new System.Windows.Forms.Label();
            this.txtPostBatchPath = new System.Windows.Forms.TextBox();
            this.btnPostBatchBrowse = new System.Windows.Forms.Button();
            this.lblJobName = new System.Windows.Forms.Label();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.tbxLabelNote = new System.Windows.Forms.TextBox();
            this.txtLabel = new System.Windows.Forms.TextBox();
            this.rdoBatch = new System.Windows.Forms.RadioButton();
            this.rdoHead = new System.Windows.Forms.RadioButton();
            this.rdoLabel = new System.Windows.Forms.RadioButton();
            this.lblBatchPath = new System.Windows.Forms.Label();
            this.txtBatchFilePath = new System.Windows.Forms.TextBox();
            this.btnBatchBrowse = new System.Windows.Forms.Button();
            this.txtJobName = new System.Windows.Forms.TextBox();
            this.chkEnabled = new System.Windows.Forms.CheckBox();
            this.lblExePath = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.txtGameProcessName = new System.Windows.Forms.TextBox();
            this.txtClientSpec = new System.Windows.Forms.TextBox();
            this.btnSave = new System.Windows.Forms.Button();
            this.dtpSyncTime = new System.Windows.Forms.DateTimePicker();
            this.lblClientSpec = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.lblServiceStatus = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.lblServiceStatusTitle = new System.Windows.Forms.Label();
            this.btnServiceControl = new System.Windows.Forms.Button();
            this.btnRun = new System.Windows.Forms.Button();
            this.lstbxSyncJobs = new System.Windows.Forms.ListBox();
            this.btnAdd = new System.Windows.Forms.Button();
            this.btnDelete = new System.Windows.Forms.Button();
            this.ofdBatchFile = new System.Windows.Forms.OpenFileDialog();
            this.UnrealSyncServiceController = new System.ServiceProcess.ServiceController();
            this.grpAddEditSyncJob.SuspendLayout();
            this.grpJobDetails.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // btnClose
            // 
            this.btnClose.Location = new System.Drawing.Point(273, 460);
            this.btnClose.Name = "btnClose";
            this.btnClose.Size = new System.Drawing.Size(75, 23);
            this.btnClose.TabIndex = 1000;
            this.btnClose.Text = "Close";
            this.btnClose.UseVisualStyleBackColor = true;
            this.btnClose.Click += new System.EventHandler(this.btnClose_Click);
            // 
            // grpAddEditSyncJob
            // 
            this.grpAddEditSyncJob.Controls.Add(this.grpJobDetails);
            this.grpAddEditSyncJob.Controls.Add(this.groupBox1);
            this.grpAddEditSyncJob.Controls.Add(this.btnRun);
            this.grpAddEditSyncJob.Controls.Add(this.lstbxSyncJobs);
            this.grpAddEditSyncJob.Controls.Add(this.btnAdd);
            this.grpAddEditSyncJob.Controls.Add(this.btnDelete);
            this.grpAddEditSyncJob.Location = new System.Drawing.Point(10, 12);
            this.grpAddEditSyncJob.Name = "grpAddEditSyncJob";
            this.grpAddEditSyncJob.Size = new System.Drawing.Size(602, 442);
            this.grpAddEditSyncJob.TabIndex = 25;
            this.grpAddEditSyncJob.TabStop = false;
            this.grpAddEditSyncJob.Text = "Add/Edit/Delete Sync Jobs";
            // 
            // grpJobDetails
            // 
            this.grpJobDetails.Controls.Add(this.chkKillProcess);
            this.grpJobDetails.Controls.Add(this.lblPostBatchPath);
            this.grpJobDetails.Controls.Add(this.txtPostBatchPath);
            this.grpJobDetails.Controls.Add(this.btnPostBatchBrowse);
            this.grpJobDetails.Controls.Add(this.lblJobName);
            this.grpJobDetails.Controls.Add(this.groupBox2);
            this.grpJobDetails.Controls.Add(this.txtJobName);
            this.grpJobDetails.Controls.Add(this.chkEnabled);
            this.grpJobDetails.Controls.Add(this.lblExePath);
            this.grpJobDetails.Controls.Add(this.label1);
            this.grpJobDetails.Controls.Add(this.txtGameProcessName);
            this.grpJobDetails.Controls.Add(this.txtClientSpec);
            this.grpJobDetails.Controls.Add(this.btnSave);
            this.grpJobDetails.Controls.Add(this.dtpSyncTime);
            this.grpJobDetails.Controls.Add(this.lblClientSpec);
            this.grpJobDetails.Location = new System.Drawing.Point(188, 19);
            this.grpJobDetails.Name = "grpJobDetails";
            this.grpJobDetails.Size = new System.Drawing.Size(408, 417);
            this.grpJobDetails.TabIndex = 85;
            this.grpJobDetails.TabStop = false;
            this.grpJobDetails.Text = "Job Details";
            // 
            // chkKillProcess
            // 
            this.chkKillProcess.AutoSize = true;
            this.chkKillProcess.Location = new System.Drawing.Point(123, 127);
            this.chkKillProcess.Name = "chkKillProcess";
            this.chkKillProcess.Size = new System.Drawing.Size(80, 17);
            this.chkKillProcess.TabIndex = 52;
            this.chkKillProcess.Text = "Kill Process";
            this.chkKillProcess.UseVisualStyleBackColor = true;
            this.chkKillProcess.CheckedChanged += new System.EventHandler(this.chkKillProcess_CheckedChanged);
            // 
            // lblPostBatchPath
            // 
            this.lblPostBatchPath.AutoSize = true;
            this.lblPostBatchPath.Location = new System.Drawing.Point(16, 353);
            this.lblPostBatchPath.Name = "lblPostBatchPath";
            this.lblPostBatchPath.Size = new System.Drawing.Size(130, 13);
            this.lblPostBatchPath.TabIndex = 85;
            this.lblPostBatchPath.Text = "Post Sync Batch File Path";
            // 
            // txtPostBatchPath
            // 
            this.txtPostBatchPath.Location = new System.Drawing.Point(152, 350);
            this.txtPostBatchPath.Name = "txtPostBatchPath";
            this.txtPostBatchPath.Size = new System.Drawing.Size(170, 20);
            this.txtPostBatchPath.TabIndex = 70;
            this.txtPostBatchPath.TextChanged += new System.EventHandler(this.txtPostBatchPath_TextChanged);
            // 
            // btnPostBatchBrowse
            // 
            this.btnPostBatchBrowse.Location = new System.Drawing.Point(328, 348);
            this.btnPostBatchBrowse.Name = "btnPostBatchBrowse";
            this.btnPostBatchBrowse.Size = new System.Drawing.Size(55, 23);
            this.btnPostBatchBrowse.TabIndex = 71;
            this.btnPostBatchBrowse.Text = "Browse";
            this.btnPostBatchBrowse.UseVisualStyleBackColor = true;
            this.btnPostBatchBrowse.Click += new System.EventHandler(this.btnPostBatchBrowse_Click);
            // 
            // lblJobName
            // 
            this.lblJobName.AutoSize = true;
            this.lblJobName.Location = new System.Drawing.Point(16, 25);
            this.lblJobName.Name = "lblJobName";
            this.lblJobName.Size = new System.Drawing.Size(55, 13);
            this.lblJobName.TabIndex = 16;
            this.lblJobName.Text = "Job Name";
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.tbxLabelNote);
            this.groupBox2.Controls.Add(this.txtLabel);
            this.groupBox2.Controls.Add(this.rdoBatch);
            this.groupBox2.Controls.Add(this.rdoHead);
            this.groupBox2.Controls.Add(this.rdoLabel);
            this.groupBox2.Controls.Add(this.lblBatchPath);
            this.groupBox2.Controls.Add(this.txtBatchFilePath);
            this.groupBox2.Controls.Add(this.btnBatchBrowse);
            this.groupBox2.Location = new System.Drawing.Point(18, 173);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(374, 147);
            this.groupBox2.TabIndex = 52;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Sync Options";
            // 
            // tbxLabelNote
            // 
            this.tbxLabelNote.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.tbxLabelNote.Enabled = false;
            this.tbxLabelNote.Location = new System.Drawing.Point(227, 79);
            this.tbxLabelNote.Multiline = true;
            this.tbxLabelNote.Name = "tbxLabelNote";
            this.tbxLabelNote.Size = new System.Drawing.Size(141, 41);
            this.tbxLabelNote.TabIndex = 76;
            this.tbxLabelNote.TabStop = false;
            this.tbxLabelNote.Text = "Note:\r\nUsing %D% will substitute the \r\ndate in YYYY-MM-DD format.";
            // 
            // txtLabel
            // 
            this.txtLabel.Location = new System.Drawing.Point(38, 92);
            this.txtLabel.Name = "txtLabel";
            this.txtLabel.Size = new System.Drawing.Size(183, 20);
            this.txtLabel.TabIndex = 62;
            this.txtLabel.TextChanged += new System.EventHandler(this.txtLabel_TextChanged);
            // 
            // rdoBatch
            // 
            this.rdoBatch.AutoSize = true;
            this.rdoBatch.Location = new System.Drawing.Point(17, 19);
            this.rdoBatch.Name = "rdoBatch";
            this.rdoBatch.Size = new System.Drawing.Size(130, 17);
            this.rdoBatch.TabIndex = 55;
            this.rdoBatch.TabStop = true;
            this.rdoBatch.Text = "Sync with a Batch File";
            this.rdoBatch.UseVisualStyleBackColor = true;
            this.rdoBatch.CheckedChanged += new System.EventHandler(this.rdoBatch_CheckedChanged);
            // 
            // rdoHead
            // 
            this.rdoHead.AutoSize = true;
            this.rdoHead.Location = new System.Drawing.Point(17, 119);
            this.rdoHead.Name = "rdoHead";
            this.rdoHead.Size = new System.Drawing.Size(94, 17);
            this.rdoHead.TabIndex = 57;
            this.rdoHead.Text = "Sync To Head";
            this.rdoHead.UseVisualStyleBackColor = true;
            this.rdoHead.CheckedChanged += new System.EventHandler(this.rdoHead_CheckedChanged);
            // 
            // rdoLabel
            // 
            this.rdoLabel.AutoSize = true;
            this.rdoLabel.Location = new System.Drawing.Point(17, 69);
            this.rdoLabel.Name = "rdoLabel";
            this.rdoLabel.Size = new System.Drawing.Size(90, 17);
            this.rdoLabel.TabIndex = 56;
            this.rdoLabel.Text = "Sync to Label";
            this.rdoLabel.UseVisualStyleBackColor = true;
            this.rdoLabel.CheckedChanged += new System.EventHandler(this.rdoLabel_CheckedChanged);
            // 
            // lblBatchPath
            // 
            this.lblBatchPath.AutoSize = true;
            this.lblBatchPath.Location = new System.Drawing.Point(35, 45);
            this.lblBatchPath.Name = "lblBatchPath";
            this.lblBatchPath.Size = new System.Drawing.Size(79, 13);
            this.lblBatchPath.TabIndex = 20;
            this.lblBatchPath.Text = "Batch File Path";
            // 
            // txtBatchFilePath
            // 
            this.txtBatchFilePath.Location = new System.Drawing.Point(120, 42);
            this.txtBatchFilePath.Name = "txtBatchFilePath";
            this.txtBatchFilePath.Size = new System.Drawing.Size(183, 20);
            this.txtBatchFilePath.TabIndex = 60;
            this.txtBatchFilePath.TextChanged += new System.EventHandler(this.txtBatchFilePath_TextChanged);
            // 
            // btnBatchBrowse
            // 
            this.btnBatchBrowse.Location = new System.Drawing.Point(309, 40);
            this.btnBatchBrowse.Name = "btnBatchBrowse";
            this.btnBatchBrowse.Size = new System.Drawing.Size(55, 23);
            this.btnBatchBrowse.TabIndex = 61;
            this.btnBatchBrowse.Text = "Browse";
            this.btnBatchBrowse.UseVisualStyleBackColor = true;
            this.btnBatchBrowse.Click += new System.EventHandler(this.btnBatchBrowse_Click);
            // 
            // txtJobName
            // 
            this.txtJobName.Location = new System.Drawing.Point(123, 22);
            this.txtJobName.Name = "txtJobName";
            this.txtJobName.Size = new System.Drawing.Size(100, 20);
            this.txtJobName.TabIndex = 10;
            this.txtJobName.TextChanged += new System.EventHandler(this.txtJobName_TextChanged);
            // 
            // chkEnabled
            // 
            this.chkEnabled.AutoSize = true;
            this.chkEnabled.Location = new System.Drawing.Point(229, 24);
            this.chkEnabled.Name = "chkEnabled";
            this.chkEnabled.Size = new System.Drawing.Size(65, 17);
            this.chkEnabled.TabIndex = 20;
            this.chkEnabled.Text = "Enabled";
            this.chkEnabled.UseVisualStyleBackColor = true;
            this.chkEnabled.CheckedChanged += new System.EventHandler(this.chkEnabled_CheckedChanged);
            // 
            // lblExePath
            // 
            this.lblExePath.AutoSize = true;
            this.lblExePath.Location = new System.Drawing.Point(16, 103);
            this.lblExePath.Name = "lblExePath";
            this.lblExePath.Size = new System.Drawing.Size(107, 13);
            this.lblExePath.TabIndex = 25;
            this.lblExePath.Text = "Game Process Name";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(15, 77);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(57, 13);
            this.label1.TabIndex = 13;
            this.label1.Text = "Sync Time";
            // 
            // txtGameProcessName
            // 
            this.txtGameProcessName.Location = new System.Drawing.Point(123, 100);
            this.txtGameProcessName.Name = "txtGameProcessName";
            this.txtGameProcessName.Size = new System.Drawing.Size(183, 20);
            this.txtGameProcessName.TabIndex = 50;
            this.txtGameProcessName.TextChanged += new System.EventHandler(this.txtGameProcessName_TextChanged);
            // 
            // txtClientSpec
            // 
            this.txtClientSpec.Location = new System.Drawing.Point(123, 48);
            this.txtClientSpec.Name = "txtClientSpec";
            this.txtClientSpec.Size = new System.Drawing.Size(183, 20);
            this.txtClientSpec.TabIndex = 30;
            this.txtClientSpec.TextChanged += new System.EventHandler(this.txtClientSpec_TextChanged);
            // 
            // btnSave
            // 
            this.btnSave.Location = new System.Drawing.Point(177, 388);
            this.btnSave.Name = "btnSave";
            this.btnSave.Size = new System.Drawing.Size(55, 23);
            this.btnSave.TabIndex = 80;
            this.btnSave.Text = "Save";
            this.btnSave.UseVisualStyleBackColor = true;
            this.btnSave.Click += new System.EventHandler(this.btnSave_Click);
            // 
            // dtpSyncTime
            // 
            this.dtpSyncTime.CustomFormat = "hh:mm tt";
            this.dtpSyncTime.Format = System.Windows.Forms.DateTimePickerFormat.Custom;
            this.dtpSyncTime.Location = new System.Drawing.Point(123, 74);
            this.dtpSyncTime.Name = "dtpSyncTime";
            this.dtpSyncTime.ShowUpDown = true;
            this.dtpSyncTime.Size = new System.Drawing.Size(71, 20);
            this.dtpSyncTime.TabIndex = 40;
            this.dtpSyncTime.ValueChanged += new System.EventHandler(this.dtpSyncTime_ValueChanged);
            // 
            // lblClientSpec
            // 
            this.lblClientSpec.AutoSize = true;
            this.lblClientSpec.Location = new System.Drawing.Point(16, 51);
            this.lblClientSpec.Name = "lblClientSpec";
            this.lblClientSpec.Size = new System.Drawing.Size(104, 13);
            this.lblClientSpec.TabIndex = 18;
            this.lblClientSpec.Text = "Perforce Client Spec";
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.lblServiceStatus);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.lblServiceStatusTitle);
            this.groupBox1.Controls.Add(this.btnServiceControl);
            this.groupBox1.Location = new System.Drawing.Point(39, 269);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(105, 113);
            this.groupBox1.TabIndex = 26;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Service Control";
            // 
            // lblServiceStatus
            // 
            this.lblServiceStatus.AutoSize = true;
            this.lblServiceStatus.Location = new System.Drawing.Point(9, 45);
            this.lblServiceStatus.Name = "lblServiceStatus";
            this.lblServiceStatus.Size = new System.Drawing.Size(37, 13);
            this.lblServiceStatus.TabIndex = 3;
            this.lblServiceStatus.Text = "Status";
            this.lblServiceStatus.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 45);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(0, 13);
            this.label2.TabIndex = 2;
            // 
            // lblServiceStatusTitle
            // 
            this.lblServiceStatusTitle.AutoSize = true;
            this.lblServiceStatusTitle.Location = new System.Drawing.Point(9, 21);
            this.lblServiceStatusTitle.Name = "lblServiceStatusTitle";
            this.lblServiceStatusTitle.Size = new System.Drawing.Size(40, 13);
            this.lblServiceStatusTitle.TabIndex = 1;
            this.lblServiceStatusTitle.Text = "Status:";
            // 
            // btnServiceControl
            // 
            this.btnServiceControl.Location = new System.Drawing.Point(15, 72);
            this.btnServiceControl.Name = "btnServiceControl";
            this.btnServiceControl.Size = new System.Drawing.Size(75, 23);
            this.btnServiceControl.TabIndex = 100;
            this.btnServiceControl.Text = "Start or Stop";
            this.btnServiceControl.UseVisualStyleBackColor = true;
            this.btnServiceControl.Click += new System.EventHandler(this.btnStartStop_Click);
            // 
            // btnRun
            // 
            this.btnRun.Location = new System.Drawing.Point(6, 225);
            this.btnRun.Name = "btnRun";
            this.btnRun.Size = new System.Drawing.Size(55, 23);
            this.btnRun.TabIndex = 1;
            this.btnRun.Text = "Run";
            this.btnRun.UseVisualStyleBackColor = true;
            this.btnRun.Click += new System.EventHandler(this.btnRun_Click);
            // 
            // lstbxSyncJobs
            // 
            this.lstbxSyncJobs.FormattingEnabled = true;
            this.lstbxSyncJobs.Location = new System.Drawing.Point(6, 21);
            this.lstbxSyncJobs.Name = "lstbxSyncJobs";
            this.lstbxSyncJobs.Size = new System.Drawing.Size(176, 199);
            this.lstbxSyncJobs.TabIndex = 0;
            this.lstbxSyncJobs.SelectedIndexChanged += new System.EventHandler(this.lstbxSyncJobs_SelectedIndexChanged);
            // 
            // btnAdd
            // 
            this.btnAdd.Location = new System.Drawing.Point(67, 225);
            this.btnAdd.Name = "btnAdd";
            this.btnAdd.Size = new System.Drawing.Size(55, 23);
            this.btnAdd.TabIndex = 2;
            this.btnAdd.Text = "Add";
            this.btnAdd.UseVisualStyleBackColor = true;
            this.btnAdd.Click += new System.EventHandler(this.btnAdd_Click);
            // 
            // btnDelete
            // 
            this.btnDelete.Location = new System.Drawing.Point(127, 225);
            this.btnDelete.Name = "btnDelete";
            this.btnDelete.Size = new System.Drawing.Size(55, 23);
            this.btnDelete.TabIndex = 3;
            this.btnDelete.Text = "Delete";
            this.btnDelete.UseVisualStyleBackColor = true;
            this.btnDelete.Click += new System.EventHandler(this.btnDelete_Click);
            // 
            // ofdBatchFile
            // 
            this.ofdBatchFile.DefaultExt = "bat";
            this.ofdBatchFile.FileName = "openFileDialog1";
            this.ofdBatchFile.Filter = "Artist Sync Batch File|*.bat";
            this.ofdBatchFile.Title = "Select your Artist Sync Batch File";
            // 
            // UnrealSyncServiceController
            // 
            this.UnrealSyncServiceController.ServiceName = "UnrealSync";
            // 
            // MgrForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(621, 495);
            this.Controls.Add(this.btnClose);
            this.Controls.Add(this.grpAddEditSyncJob);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "MgrForm";
            this.Text = "UnrealSync Manager";
            this.grpAddEditSyncJob.ResumeLayout(false);
            this.grpJobDetails.ResumeLayout(false);
            this.grpJobDetails.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button btnClose;
        private System.Windows.Forms.GroupBox grpAddEditSyncJob;
        private System.Windows.Forms.ListBox lstbxSyncJobs;
        private System.Windows.Forms.Button btnBatchBrowse;
        private System.Windows.Forms.Button btnAdd;
        private System.Windows.Forms.Label lblBatchPath;
        private System.Windows.Forms.TextBox txtBatchFilePath;
        private System.Windows.Forms.Button btnDelete;
        private System.Windows.Forms.Label lblClientSpec;
        private System.Windows.Forms.DateTimePicker dtpSyncTime;
        private System.Windows.Forms.TextBox txtClientSpec;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label lblJobName;
        private System.Windows.Forms.CheckBox chkEnabled;
        private System.Windows.Forms.TextBox txtJobName;
        private System.Windows.Forms.Button btnSave;
        private System.Windows.Forms.Label lblExePath;
        private System.Windows.Forms.TextBox txtGameProcessName;
        private System.Windows.Forms.OpenFileDialog ofdBatchFile;
        private System.ServiceProcess.ServiceController UnrealSyncServiceController;
        private System.Windows.Forms.Button btnServiceControl;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Label lblServiceStatusTitle;
        private System.Windows.Forms.Label lblServiceStatus;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button btnRun;
        private System.Windows.Forms.RadioButton rdoBatch;
        private System.Windows.Forms.RadioButton rdoHead;
        private System.Windows.Forms.RadioButton rdoLabel;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.TextBox txtLabel;
        private System.Windows.Forms.TextBox tbxLabelNote;
        private System.Windows.Forms.GroupBox grpJobDetails;
        private System.Windows.Forms.Label lblPostBatchPath;
        private System.Windows.Forms.TextBox txtPostBatchPath;
        private System.Windows.Forms.Button btnPostBatchBrowse;
        private System.Windows.Forms.CheckBox chkKillProcess;
    }
}