using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace MemoryProfiler
{
	public partial class Form1 : Form
	{
		FStreamParser StreamParser;
		FTreeListView FileTreeListView = new FTreeListView();
		FTreeListView CallGraphTreeListView = new FTreeListView();        

		public Form1()
		{
			InitializeComponent();

			FileTreeListView.Columns.Add(new FTreeListViewColumnHeader("Name"));
			FileTreeListView.Columns.Add(new FTreeListViewColumnHeader("Size of Active Allocations"));
			FileTreeListView.Columns.Add(new FTreeListViewColumnHeader("Allocation Count"));
			FileTreeListView.Dock = DockStyle.Fill;
			FileTreeListView.ItemMouseClick += new EventHandler<FTreeListViewMouseClickItemEventArgs>(FileTreeListView_ItemMouseClick);
			FileTreeListView.ItemMouseDoubleClick += new EventHandler<FTreeListViewMouseClickItemEventArgs>(FileTreeListView_ItemMouseDoubleClick);
			this.tabPage1.Controls.Add(FileTreeListView);

			CallGraphView.Visible = false;
			CallGraphTreeListView.Columns.Add(new FTreeListViewColumnHeader("Function Name"));
			CallGraphTreeListView.Columns.Add(new FTreeListViewColumnHeader("Size of Active Allocations"));
			CallGraphTreeListView.Columns.Add(new FTreeListViewColumnHeader("Allocation Count"));
			CallGraphTreeListView.Columns.Add(new FTreeListViewColumnHeader("File Name"));
			CallGraphTreeListView.Dock = DockStyle.Fill;
			CallGraphTreeListView.ItemMouseClick += new EventHandler<FTreeListViewMouseClickItemEventArgs>(CallGraphTreeListView_ItemMouseClick);
			this.tabPage2.Controls.Add(CallGraphTreeListView);

            MemTagSelectionDialog = new Form2();
            for (int i = 0; i < 32; i++)
            {
                MemTagSelectionDialog.MemTagCheckedListBox.Items.Add(String.Format("MemTag bit {0}",i),false);
            }
		}

		void CallGraphTreeListView_ItemMouseClick(object Sender, FTreeListViewMouseClickItemEventArgs Event)
		{
			int AddressIndex = (int)Event.Item.Tag;
			FStreamParser.FCallStackAddress Address = this.StreamParser.GetCallStackAddressFromAddressIndex(AddressIndex);

			if(Address != null)
			{
				if(System.IO.File.Exists(Event.Item.SubItems[2].Text))
				{
					CodeWindow.LoadFile(Event.Item.SubItems[2].Text, RichTextBoxStreamType.PlainText);
					SelectLine(Address.LineNumber, 20);
				}
				else
				{
					CodeWindow.Text = "File unavailable.";
				}
			}
		}

		void FileTreeListView_ItemMouseDoubleClick(object sender, FTreeListViewMouseClickItemEventArgs Event)
		{
			// Populate callgraph tree based on selected node.
			FNodePayload Payload = Event.Item.Tag as FNodePayload;
			if(Payload != null)
			{
				StreamParser.ParseCallGraph(Payload, CallGraphTreeListView);
				tabControl1.SelectTab(1);
			}
		}

		void FileTreeListView_ItemMouseClick(object sender, FTreeListViewMouseClickItemEventArgs Event)
		{
			FNodePayload Payload = Event.Item.Tag as FNodePayload;
			if(Payload != null)
			{
				if(System.IO.File.Exists(Payload.Filename))
				{
					CodeWindow.LoadFile(Payload.Filename, RichTextBoxStreamType.PlainText);
					SelectLine(Payload.LineNumber, 20);
				}
				else
				{
					CodeWindow.Text = "File unavailable.";
				}
			}
		}

		/**
		 *  Selects passed in line in code window and scrolls ScrollOffset lines above that line.
		 */
		private void SelectLine(int LineNumber, int ScrollOffset)
		{
			// Line numbers are 1- based.
			if( LineNumber > 0 && LineNumber < CodeWindow.Lines.Length )
			{
				int LineIndex = LineNumber - 1;

				// Convert line number to character index for selection.
				int LineCharIndex = CodeWindow.GetFirstCharIndexFromLine(LineIndex);
				int ScrollCharIndex = CodeWindow.GetFirstCharIndexFromLine(LineIndex > ScrollOffset ? LineIndex-ScrollOffset : 0 );

				// Reset text coloration.
				CodeWindow.SelectAll();
				CodeWindow.SelectionBackColor = CodeWindow.BackColor;

				// Highlight wanted line.
				CodeWindow.SelectionStart = LineCharIndex;
				CodeWindow.SelectionLength = CodeWindow.Lines[LineIndex].Length;
				CodeWindow.SelectionBackColor = Color.Red;

				// Scroll to ScrollOffset lines before selection.
				CodeWindow.SelectionStart = ScrollCharIndex;
				CodeWindow.SelectionLength = 0;
				CodeWindow.SelectionBackColor = CodeWindow.BackColor;
				CodeWindow.ScrollToCaret();
			}
		}

		private void MainMenu_File_Exit_Click(object sender, EventArgs e)
		{
			this.Close();
		}

		private void MainMenu_File_Open_Click(object sender, EventArgs e)
		{
			OpenMProfDialog.InitialDirectory = System.IO.Directory.GetCurrentDirectory();

			if(OpenMProfDialog.ShowDialog(this) == DialogResult.OK)
			{
				StreamParser = new FStreamParser();
                MemProfileFilename = OpenMProfDialog.FileName;
                StreamParser.InitParser(MemProfileFilename,0,false);
				
				FileTreeListView.BeginUpdate();
				FileTreeListView.Items.Clear();
				FileTreeListView.EndUpdate();
				
				// here we want to look at each data file and then do computation on it
                for ( int i = 0; i < StreamParser.DataFilesArray.Length; ++i )
                {
                    Console.WriteLine( "DataFiles: {0} {1}", i, StreamParser.DataFilesArray[i] );
                }

                for( int i = 0; i < StreamParser.DataFilesArray.Length; ++i )
                {
                    // Create binary reader and file info object from filename.
                    Console.WriteLine( "Reading DataFile:  {0}", StreamParser.DataFilesArray[i] );

                    StreamParser.ParserFileStream = File.OpenRead( StreamParser.DataFilesArray[i] );
                    if ( StreamParser.bParserFileStreamIsBigEndian == true )
                    {
                        StreamParser.BinaryStream = new BinaryReaderBigEndian( StreamParser.ParserFileStream );
                    }
                    else
                    {
                        StreamParser.BinaryStream = new BinaryReader( StreamParser.ParserFileStream );
                    }

                    StreamParser.StartOfStream = 0;

					StreamParser.ParseFileFunctionLine(FileTreeListView);
				}
			}
		}

        private void MemTagRebuildButton_Click(object sender, EventArgs e)
        {
            if (MemTagSelectionDialog.ShowDialog(this) == DialogResult.OK)
            {
                // default mem tag of 0 will load all allocations
                UInt32 MemTagsForParsing = 0;
                // get the selected tags specified
                foreach (int CheckedIdx in MemTagSelectionDialog.MemTagCheckedListBox.CheckedIndices)
                {
                    MemTagsForParsing |= (UInt32)(1 << CheckedIdx);
                }
                
                if (OpenMProfDialog.ShowDialog(this) == DialogResult.OK)
                {
                    MemProfileFilename = OpenMProfDialog.FileName;
                }
                
                if (MemProfileFilename != null)
                {
                    // reload data file with the selected tags
                    StreamParser = new FStreamParser();
                    StreamParser.InitParser(MemProfileFilename, MemTagsForParsing,false);

                    FileTreeListView.BeginUpdate();
                    FileTreeListView.Items.Clear();
                    FileTreeListView.EndUpdate();

                    StreamParser.ParseFileFunctionLine(FileTreeListView);
                }
            }
        }

        private void DumpToFileButton_Click(object sender, EventArgs e)
        {
            if (MemTagSelectionDialog.ShowDialog(this) == DialogResult.OK)
            {
                // default mem tag of 0 will load all allocations
                UInt32 MemTagsForParsing = 0;
                // get the selected tags specified
                foreach (int CheckedIdx in MemTagSelectionDialog.MemTagCheckedListBox.CheckedIndices)
                {
                    MemTagsForParsing |= (UInt32)(1 << CheckedIdx);
                }

                if (OpenMProfDialog.ShowDialog(this) == DialogResult.OK)
                {
                    MemProfileFilename = OpenMProfDialog.FileName;
                    if (MemProfileFilename != null)
                    {
                        // reload data file with the selected tags
                        StreamParser = new FStreamParser();
                        StreamParser.InitParser(MemProfileFilename, MemTagsForParsing, true);
						
                        // here we want to look at each data file and then do computation on it
                        for ( int i = 0; i < StreamParser.DataFilesArray.Length; ++i )
                        {
                            Console.WriteLine( "DataFiles: {0} {1}", i, StreamParser.DataFilesArray[i] );
                        }

                        for( int i = 0; i < StreamParser.DataFilesArray.Length; ++i )
                        {
                            // Create binary reader and file info object from filename.
                            Console.WriteLine( "Reading DataFile:  {0}", StreamParser.DataFilesArray[i] );

                            StreamParser.ParserFileStream = File.OpenRead( StreamParser.DataFilesArray[i] );
                            if ( StreamParser.bParserFileStreamIsBigEndian == true )
                            {
                                StreamParser.BinaryStream = new BinaryReaderBigEndian( StreamParser.ParserFileStream );
                            }
                            else
                            {
                                StreamParser.BinaryStream = new BinaryReader( StreamParser.ParserFileStream );
                            }

                            StreamParser.StartOfStream = 0;


                            // adds a ".dump" and tags
                            StreamParser.ParseFileToDump( MemProfileFilename, StreamParser, true );
                        }
                    }
                }
            }

        }
	}
}