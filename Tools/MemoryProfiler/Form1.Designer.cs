namespace MemoryProfiler
{
    partial class Form1
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            this.MainMenu = new System.Windows.Forms.MenuStrip();
            this.MainMenu_File = new System.Windows.Forms.ToolStripMenuItem();
            this.MainMenu_File_Open = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
            this.MainMenu_File_Exit = new System.Windows.Forms.ToolStripMenuItem();
            this.OpenMProfDialog = new System.Windows.Forms.OpenFileDialog();
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.toolStripContainer1 = new System.Windows.Forms.ToolStripContainer();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.CallGraphView = new System.Windows.Forms.TreeView();
            this.toolStrip1 = new System.Windows.Forms.ToolStrip();
            this.MemTagLabel = new System.Windows.Forms.ToolStripLabel();
            this.MemTagRebuildButton = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.CodeWindow = new System.Windows.Forms.RichTextBox();
            this.DumpToFileButton = new System.Windows.Forms.ToolStripButton();
            this.MainMenu.SuspendLayout();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            this.toolStripContainer1.ContentPanel.SuspendLayout();
            this.toolStripContainer1.TopToolStripPanel.SuspendLayout();
            this.toolStripContainer1.SuspendLayout();
            this.tabControl1.SuspendLayout();
            this.tabPage2.SuspendLayout();
            this.toolStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // MainMenu
            // 
            this.MainMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.MainMenu_File});
            this.MainMenu.Location = new System.Drawing.Point(0, 0);
            this.MainMenu.Name = "MainMenu";
            this.MainMenu.Size = new System.Drawing.Size(1540, 24);
            this.MainMenu.TabIndex = 7;
            this.MainMenu.Text = "MainMenu";
            // 
            // MainMenu_File
            // 
            this.MainMenu_File.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.MainMenu_File_Open,
            this.toolStripMenuItem1,
            this.MainMenu_File_Exit});
            this.MainMenu_File.Name = "MainMenu_File";
            this.MainMenu_File.Size = new System.Drawing.Size(35, 20);
            this.MainMenu_File.Text = "&File";
            // 
            // MainMenu_File_Open
            // 
            this.MainMenu_File_Open.Name = "MainMenu_File_Open";
            this.MainMenu_File_Open.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.O)));
            this.MainMenu_File_Open.Size = new System.Drawing.Size(152, 22);
            this.MainMenu_File_Open.Text = "&Open";
            this.MainMenu_File_Open.Click += new System.EventHandler(this.MainMenu_File_Open_Click);
            // 
            // toolStripMenuItem1
            // 
            this.toolStripMenuItem1.Name = "toolStripMenuItem1";
            this.toolStripMenuItem1.Size = new System.Drawing.Size(149, 6);
            // 
            // MainMenu_File_Exit
            // 
            this.MainMenu_File_Exit.Name = "MainMenu_File_Exit";
            this.MainMenu_File_Exit.Size = new System.Drawing.Size(152, 22);
            this.MainMenu_File_Exit.Text = "E&xit";
            this.MainMenu_File_Exit.Click += new System.EventHandler(this.MainMenu_File_Exit_Click);
            // 
            // OpenMProfDialog
            // 
            this.OpenMProfDialog.DefaultExt = "mprof";
            this.OpenMProfDialog.FileName = "MemoryProfiler.mprof";
            this.OpenMProfDialog.Filter = "Unreal Memory Profiler Dump (*.mprof)|*.mprof";
            this.OpenMProfDialog.RestoreDirectory = true;
            this.OpenMProfDialog.SupportMultiDottedExtensions = true;
            this.OpenMProfDialog.Title = "Please locate a memory dump";
            // 
            // splitContainer1
            // 
            this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer1.Location = new System.Drawing.Point(0, 24);
            this.splitContainer1.Name = "splitContainer1";
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.toolStripContainer1);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.CodeWindow);
            this.splitContainer1.Size = new System.Drawing.Size(1540, 993);
            this.splitContainer1.SplitterDistance = 773;
            this.splitContainer1.TabIndex = 8;
            // 
            // toolStripContainer1
            // 
            // 
            // toolStripContainer1.ContentPanel
            // 
            this.toolStripContainer1.ContentPanel.Controls.Add(this.tabControl1);
            this.toolStripContainer1.ContentPanel.Size = new System.Drawing.Size(773, 968);
            this.toolStripContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.toolStripContainer1.Location = new System.Drawing.Point(0, 0);
            this.toolStripContainer1.Name = "toolStripContainer1";
            this.toolStripContainer1.Size = new System.Drawing.Size(773, 993);
            this.toolStripContainer1.TabIndex = 9;
            this.toolStripContainer1.Text = "toolStripContainer1";
            // 
            // toolStripContainer1.TopToolStripPanel
            // 
            this.toolStripContainer1.TopToolStripPanel.Controls.Add(this.toolStrip1);
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.tabPage1);
            this.tabControl1.Controls.Add(this.tabPage2);
            this.tabControl1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabControl1.Location = new System.Drawing.Point(0, 0);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(773, 968);
            this.tabControl1.TabIndex = 8;
            // 
            // tabPage1
            // 
            this.tabPage1.Location = new System.Drawing.Point(4, 22);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage1.Size = new System.Drawing.Size(765, 942);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "File Function Line View";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.CallGraphView);
            this.tabPage2.Location = new System.Drawing.Point(4, 22);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage2.Size = new System.Drawing.Size(765, 942);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "Call Graph View";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // CallGraphView
            // 
            this.CallGraphView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.CallGraphView.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.CallGraphView.Location = new System.Drawing.Point(3, 3);
            this.CallGraphView.Name = "CallGraphView";
            this.CallGraphView.Size = new System.Drawing.Size(759, 936);
            this.CallGraphView.TabIndex = 0;
            // 
            // toolStrip1
            // 
            this.toolStrip1.Dock = System.Windows.Forms.DockStyle.None;
            this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.MemTagLabel,
            this.MemTagRebuildButton,
            this.toolStripSeparator1,
            this.DumpToFileButton});
            this.toolStrip1.Location = new System.Drawing.Point(3, 0);
            this.toolStrip1.Name = "toolStrip1";
            this.toolStrip1.Size = new System.Drawing.Size(337, 25);
            this.toolStrip1.TabIndex = 0;
            // 
            // MemTagLabel
            // 
            this.MemTagLabel.Name = "MemTagLabel";
            this.MemTagLabel.Size = new System.Drawing.Size(47, 22);
            this.MemTagLabel.Text = "MemTag";
            // 
            // MemTagRebuildButton
            // 
            this.MemTagRebuildButton.Image = ((System.Drawing.Image)(resources.GetObject("MemTagRebuildButton.Image")));
            this.MemTagRebuildButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.MemTagRebuildButton.Name = "MemTagRebuildButton";
            this.MemTagRebuildButton.RightToLeftAutoMirrorImage = true;
            this.MemTagRebuildButton.Size = new System.Drawing.Size(145, 22);
            this.MemTagRebuildButton.Text = "RELOAD WITH MEM TAG";
            this.MemTagRebuildButton.ToolTipText = "Reload and parse data file with new memory tag value";
            this.MemTagRebuildButton.Click += new System.EventHandler(this.MemTagRebuildButton_Click);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(6, 25);
            // 
            // CodeWindow
            // 
            this.CodeWindow.BackColor = System.Drawing.SystemColors.Window;
            this.CodeWindow.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.CodeWindow.Dock = System.Windows.Forms.DockStyle.Fill;
            this.CodeWindow.Font = new System.Drawing.Font("Courier New", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.CodeWindow.Location = new System.Drawing.Point(0, 0);
            this.CodeWindow.Name = "CodeWindow";
            this.CodeWindow.ReadOnly = true;
            this.CodeWindow.Size = new System.Drawing.Size(763, 993);
            this.CodeWindow.TabIndex = 8;
            this.CodeWindow.Text = "";
            this.CodeWindow.WordWrap = false;
            // 
            // DumpToFileButton
            // 
            this.DumpToFileButton.Image = ((System.Drawing.Image)(resources.GetObject("DumpToFileButton.Image")));
            this.DumpToFileButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.DumpToFileButton.Name = "DumpToFileButton";
            this.DumpToFileButton.Size = new System.Drawing.Size(96, 22);
            this.DumpToFileButton.Text = "DUMP TO FILE";
            this.DumpToFileButton.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.DumpToFileButton.Click += new System.EventHandler(this.DumpToFileButton_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1540, 1017);
            this.Controls.Add(this.splitContainer1);
            this.Controls.Add(this.MainMenu);
            this.MainMenuStrip = this.MainMenu;
            this.Name = "Form1";
            this.Text = "Unreal Memory Profiler";
            this.MainMenu.ResumeLayout(false);
            this.MainMenu.PerformLayout();
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel2.ResumeLayout(false);
            this.splitContainer1.ResumeLayout(false);
            this.toolStripContainer1.ContentPanel.ResumeLayout(false);
            this.toolStripContainer1.TopToolStripPanel.ResumeLayout(false);
            this.toolStripContainer1.TopToolStripPanel.PerformLayout();
            this.toolStripContainer1.ResumeLayout(false);
            this.toolStripContainer1.PerformLayout();
            this.tabControl1.ResumeLayout(false);
            this.tabPage2.ResumeLayout(false);
            this.toolStrip1.ResumeLayout(false);
            this.toolStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

		private System.Windows.Forms.MenuStrip MainMenu;
		private System.Windows.Forms.ToolStripMenuItem MainMenu_File;
		private System.Windows.Forms.ToolStripMenuItem MainMenu_File_Open;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
		private System.Windows.Forms.ToolStripMenuItem MainMenu_File_Exit;
		private System.Windows.Forms.OpenFileDialog OpenMProfDialog;
		private System.Windows.Forms.SplitContainer splitContainer1;
		private System.Windows.Forms.TabControl tabControl1;
		private System.Windows.Forms.TabPage tabPage1;
		private System.Windows.Forms.TabPage tabPage2;
		private System.Windows.Forms.TreeView CallGraphView;
		private System.Windows.Forms.RichTextBox CodeWindow;
        private System.Windows.Forms.ToolStripContainer toolStripContainer1;
        private System.Windows.Forms.ToolStrip toolStrip1;
        private System.Windows.Forms.ToolStripLabel MemTagLabel;
        private System.Windows.Forms.ToolStripButton MemTagRebuildButton;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;

        
        private Form2 MemTagSelectionDialog;
        private string MemProfileFilename;
        private System.Windows.Forms.ToolStripButton DumpToFileButton;
    }
}

