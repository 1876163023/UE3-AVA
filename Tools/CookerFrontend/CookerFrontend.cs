using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using System.Diagnostics;
using System.Threading;
using System.IO;
using System.Runtime.InteropServices;
using System.Xml;
using System.Xml.Serialization;
using System.Text.RegularExpressions;
using CookerTools;
using Pipes;

namespace CookerFrontend
{
	/// <summary>
	/// Form for displaying information/interacting with the user
	/// </summary>
	public class CookerFrontendWindow : System.Windows.Forms.Form, CookerTools.IOutputHandler
	{
		#region Windows Forms variables
		private System.Windows.Forms.TextBox TextInput;
		private System.Windows.Forms.RichTextBox LogWindow;
		private System.Windows.Forms.TextBox MapToLaunch;
		private System.Windows.Forms.Button LaunchMap;
		private System.Windows.Forms.GroupBox CookingGroupBox;
		private System.Windows.Forms.ComboBox PCConfiguration;
		private System.Windows.Forms.CheckBox HavePackagesChanged;
		private System.Windows.Forms.Button CopyButton;
		#endregion

		#region Variables
		private const string UNI_COLOR_MAGIC = "`~[~`";

		public bool Running = false;
		private bool IsCooking = false;
		private int TickCount = 0;
		private int LastTickCount = 0;
		/** The running Commandlet process, for reading output and canceling.				*/
		private Process CookerCommandlet;
		/** A Timer object used to read output from the commandlet every N ms.				*/
		private System.Windows.Forms.Timer CookerCommandletTimer;

		/** Thread safe array used to get data from the command console and display.		*/
		private ArrayList LogTextArray = new ArrayList();
		/** Thread that polls the console placing the output in the OutputTextArray.		*/
		private Thread LogPolling;
		private System.Windows.Forms.TextBox BaseDirectory;
		private System.Windows.Forms.NumericUpDown ChapterPointSelect;
		private System.Windows.Forms.CheckBox UseChapterPoint;
		private System.Windows.Forms.MainMenu CFEMainMenu;
		private System.Windows.Forms.MenuItem MenuItem_ImportMapList;
		private System.Windows.Forms.Button Button_StartCooking;
		private System.Windows.Forms.CheckBox CookFinalRelease;
		private System.Windows.Forms.CheckBox CookInisIntsOnly;
		private System.Windows.Forms.GroupBox ToolsGroup;
		private System.Windows.Forms.Label MapsToCookLabel;

		/** Instance of the cooker tools helper class										*/
		private CookerToolsClass CookerTools;
		private System.Windows.Forms.GroupBox RunGroup;
		private System.Windows.Forms.ComboBox ConsoleConfiguration;
		private System.Windows.Forms.CheckedListBox TargetListBox;
		private System.Windows.Forms.Label ConsoleBaseLabel;
		private System.Windows.Forms.ComboBox Platform;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.ComboBox GameName;
		private System.Windows.Forms.Label LabelCommandline;
		private System.Windows.Forms.Button RebootTarget;
		private System.Windows.Forms.CheckBox LaunchAfterCooking;
		private System.Windows.Forms.CheckBox RunWithCookedMap;
		private System.Windows.Forms.ToolTip toolTip1;
		/** Instance of pipes helper class */
		private NamedPipe LogPipe;
		#endregion
				
		[DllImport("kernel32", SetLastError=true)]
		static extern IntPtr SetEnvironmentVariableA (string EnvName,string EnvValue);

		#region Cooking helpers
		/// <summary>
		/// Start up the given executable
		/// </summary>
		/// <param name="Executable">The string name of the executable to run.</param>
		/// <param name="CommandLIne">Any command line parameters to pass to the program.</param>
		/// <returns>The running process if successful, null on failure</returns>
		private Process ExecuteProgram( string Executable, string CommandLine )
		{
			ProcessStartInfo StartInfo = new ProcessStartInfo();

			// Prepare a ProcessStart structure 
			StartInfo.FileName = Executable;
			StartInfo.Arguments = CommandLine;
			StartInfo.UseShellExecute = false;
			// Redirect the output.
			StartInfo.RedirectStandardOutput = true;
			StartInfo.RedirectStandardError = true;
			StartInfo.CreateNoWindow = true;

			AddLine( "Running: " + Executable + " " + CommandLine, Color.OrangeRed );

			// Spawn the process
			Process NewProcess = null;
			// Try to start the process, handling thrown exceptions as a failure.
			try
			{
				NewProcess = Process.Start( StartInfo );
			}
			catch
			{
				return( null );
			}

			return( NewProcess );
		}		
		
		protected void CacheGameNameAndConsoles()
		{
			// get the name from the combo box
			string NewGameName = GameName.SelectedItem.ToString();
			// strip off the trailing Game
			NewGameName = NewGameName.Replace( "Game", "" );

			// collect the checked consoles
			ArrayList ConsoleNames = new ArrayList();
			foreach( string ConsoleStr in TargetListBox.CheckedItems )
			{
				ConsoleNames.Add( ConsoleStr );
			}

			// tell the cooker tools the gamename
			CookerTools.SetGameAndConsoleInfo( NewGameName, ConsoleNames, BaseDirectory.Text );
		}

		#endregion

		private System.ComponentModel.IContainer components;

		#region Windows Form setup


		public CookerFrontendWindow()
		{
			// Required for Windows Form Designer support
			InitializeComponent();
		}

		public bool Init()
		{
			// initialize the helper cooker tools
			CookerTools = new CookerToolsClass( Application.StartupPath, this );

			// Read the application settings
			LoadSupportedGames();

			// Read the settings first and then set the default with those if possible
			if (ReadSettings() == false)
			{
				GameName.SelectedItem				= "WarGame";
				// we need to have found one to set it
				if (Platform.Items.Count > 0)
				{
					Platform.SelectedItem				= Platform.Items[0];
				}
				PCConfiguration.SelectedItem		= "Release";
				ConsoleConfiguration.SelectedItem	= "Release";
				TextInput.Text						= "Entry";
				MapToLaunch.Text					= "Entry";
			}


			// since this was added after the fact, the ReadSettings won't fail, but it won't have
			// a valid setting, so set it here
			if (BaseDirectory.Text.Length == 0)
			{
				BaseDirectory.Text				= "UnrealEngine3";
			}

			GameName_ValueChanged( null, null );

			// Create a Timer object.
			CookerCommandletTimer = new System.Windows.Forms.Timer();
			CookerCommandletTimer.Interval = 100;
			CookerCommandletTimer.Tick += new System.EventHandler( this.OnTimer );

			Show();
			Running = true;
			return( true );
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool bDisposing )
		{
			// Stop the main loop running
			Running = false;

			// Store the current app state
			SaveSettings();
			if( bDisposing )
			{
				if( components != null ) 
				{
					components.Dispose();
				}
			}
			base.Dispose( bDisposing );
		}
		#endregion

		#region Loading/Saving application settings
		/// <summary>
		/// Reads the last used application settings from the XML file
		/// </summary>
		/// <returns>true if there were valid settings, false otherwise</returns>
		bool ReadSettings()
		{
			Stream XmlStream = null;
			try
			{
				string Name = Application.ExecutablePath.Substring(0,Application.ExecutablePath.Length - 4);
				Name += "_user.xml";
				// Get the XML data stream to read from
				XmlStream = new FileStream(Name,FileMode.Open,
					FileAccess.Read,FileShare.None,256 * 1024,false);
				// Creates an instance of the XmlSerializer class so we can
				// read the settings object
				XmlSerializer ObjSer = new XmlSerializer(typeof(UserSettings));
				// Add our callbacks for a busted XML file
				ObjSer.UnknownNode += new XmlNodeEventHandler(XmlSerializer_UnknownNode);
				ObjSer.UnknownAttribute += new XmlAttributeEventHandler(XmlSerializer_UnknownAttribute);
				// Create an object graph from the XML data
				UserSettings AppSettings = (UserSettings)ObjSer.Deserialize(XmlStream);
				// Copy the settings out
				GameName.SelectedItem = AppSettings.GameName;
				Platform.SelectedItem = AppSettings.Platform;
				PCConfiguration.SelectedItem = AppSettings.PCConfiguration;
				ConsoleConfiguration.SelectedItem = AppSettings.ConsoleConfiguration;
				TextInput.Text = AppSettings.CookMaps;
				MapToLaunch.Text = AppSettings.RunMaps;
				BaseDirectory.Text = AppSettings.BaseDirectory;
				UseChapterPoint.Checked = AppSettings.UseChapterPoint;
				ChapterPointSelect.Value = AppSettings.ChapterPoint;
				CookFinalRelease.Checked = AppSettings.FinalReleaseChecked;
				CookInisIntsOnly.Checked = AppSettings.InisIntsOnlyChecked;
				LaunchAfterCooking.Checked = AppSettings.LaunchAfterCooking;
				RunWithCookedMap.Checked = AppSettings.RunWithCookedMap;
			}
			catch (Exception e)
			{
				Console.WriteLine(e.ToString());
				return false;
			}
			finally
			{
				if (XmlStream != null)
				{
					// Done with the file so close it
					XmlStream.Close();
				}
			}
			return true;
		}

		/// <summary>
		/// Saves the current application settings to the XML file
		/// </summary>
		private void SaveSettings()
		{
			Stream XmlStream = null;
			try
			{
				UserSettings AppSettings = new UserSettings();
				// Copy the settings out
				AppSettings.GameName = GameName.SelectedItem.ToString();
				AppSettings.Platform = Platform.SelectedItem.ToString();
				AppSettings.PCConfiguration = PCConfiguration.SelectedItem.ToString();
				AppSettings.ConsoleConfiguration = ConsoleConfiguration.SelectedItem.ToString();
				AppSettings.CookMaps = TextInput.Text;
				AppSettings.RunMaps = MapToLaunch.Text;
				AppSettings.BaseDirectory = BaseDirectory.Text;
				AppSettings.UseChapterPoint = UseChapterPoint.Checked;
				AppSettings.ChapterPoint = Convert.ToInt32( ChapterPointSelect.Value );
				AppSettings.FinalReleaseChecked = CookFinalRelease.Checked;
				AppSettings.InisIntsOnlyChecked = CookInisIntsOnly.Checked;
				AppSettings.LaunchAfterCooking = LaunchAfterCooking.Checked;
				AppSettings.RunWithCookedMap = RunWithCookedMap.Checked;
				string Name = Application.ExecutablePath.Substring(0,Application.ExecutablePath.Length - 4);
				Name += "_user.xml";
				// Get the XML data stream to read from
				XmlStream = new FileStream(Name,FileMode.Create,
					FileAccess.Write,FileShare.None,256 * 1024,false);
				// Creates an instance of the XmlSerializer class so we can
				// save the settings object
				XmlSerializer ObjSer = new XmlSerializer(typeof(UserSettings));
				// Write the object graph as XML data
				ObjSer.Serialize(XmlStream,AppSettings);
			}
			catch (Exception e)
			{
				Console.WriteLine(e.ToString());
			}
			finally
			{
				if (XmlStream != null)
				{
					// Done with the file so close it
					XmlStream.Close();
				}
			}
		}

		/// <summary>
		/// Logs the bad XML information for debugging purposes
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e">The attribute info</param>
		protected void XmlSerializer_UnknownAttribute(object sender, XmlAttributeEventArgs e)
		{
			System.Xml.XmlAttribute attr = e.Attr;
			Console.WriteLine("Unknown attribute " + attr.Name + "='" + attr.Value + "'");
		}

		/// <summary>
		/// Logs the node information for debugging purposes
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e">The info about the bad node type</param>
		protected void XmlSerializer_UnknownNode(object sender, XmlNodeEventArgs e)
		{
			Console.WriteLine("Unknown Node:" + e.Name + "\t" + e.Text);
		}

		/// <summary>
		/// Saves the specified cooker settings to a XML file
		/// </summary>
		/// <param name="Cfg">The settings to write out</param>
		private void SaveCookerSettings(CookerSettings Cfg)
		{
			Stream XmlStream = null;
			try
			{
				string Name = Application.ExecutablePath.Substring(0,Application.ExecutablePath.Length - 4);
				Name += "_cfg.xml";
				// Get the XML data stream to read from
				XmlStream = new FileStream(Name,FileMode.Create,
					FileAccess.Write,FileShare.None,256 * 1024,false);
				// Creates an instance of the XmlSerializer class so we can
				// save the settings object
				XmlSerializer ObjSer = new XmlSerializer(typeof(CookerSettings));
				// Write the object graph as XML data
				ObjSer.Serialize(XmlStream,Cfg);
			}
			catch (Exception e)
			{
				Console.WriteLine(e.ToString());
			}
			finally
			{
				if (XmlStream != null)
				{
					// Done with the file so close it
					XmlStream.Close();
				}
			}
		}

		/// <summary>
		/// Converts the XML file into an object
		/// </summary>
		/// <returns>The new object settings, or null if the file is missing</returns>
		private CookerSettings ReadCookerSettings()
		{
			Stream XmlStream = null;
			CookerSettings CookerCfg = null;
			try
			{
				string Name = Application.ExecutablePath.Substring(0,Application.ExecutablePath.Length - 4);
				Name += "_cfg.xml";
				// Get the XML data stream to read from
				XmlStream = new FileStream(Name,FileMode.Open,FileAccess.Read,FileShare.None,256 * 1024,false);
				// Creates an instance of the XmlSerializer class so we can
				// read the settings object
				XmlSerializer ObjSer = new XmlSerializer(typeof(CookerSettings));
				// Add our callbacks for a busted XML file
				ObjSer.UnknownNode += new XmlNodeEventHandler(XmlSerializer_UnknownNode);
				ObjSer.UnknownAttribute += new XmlAttributeEventHandler(XmlSerializer_UnknownAttribute);
				// Create an object graph from the XML data
				CookerCfg = (CookerSettings)ObjSer.Deserialize(XmlStream);
			}
			catch (Exception e)
			{
				Console.WriteLine(e.ToString());
			}
			finally
			{
				if (XmlStream != null)
				{
					// Done with the file so close it
					XmlStream.Close();
				}
			}
			return CookerCfg;
		}

		/// <summary>
		/// Reads the supported game type and configuration settings. Saves a
		/// set of defaults if missing
		/// </summary>
		protected void LoadSupportedGames()
		{
			// Read the XML file
			CookerSettings CookerCfg = ReadCookerSettings();
			if (CookerCfg == null)
			{
				// Create a set of defaults
				CookerCfg = new CookerSettings(0);
				// Save these settings so they are there next time
				SaveCookerSettings(CookerCfg);
			}
			// Clear any previous data
			GameName.Items.Clear();
			PCConfiguration.Items.Clear();
			ConsoleConfiguration.Items.Clear();
			// Set the strings in the game drop downs
			foreach (string Item in CookerCfg.Games)
			{
				GameName.Items.Add(Item);
			}
			// Now do the PC configs
			foreach (string Item in CookerCfg.PCConfigs)
			{
				PCConfiguration.Items.Add(Item);
			}
			// And finally the target configs
			foreach (string Item in CookerCfg.ConsoleConfigs)
			{
				ConsoleConfiguration.Items.Add(Item);
			}

			// query for support of known platforms, and add appropriatelyu
			for (int PlatformIndex = 0; PlatformIndex < CookerCfg.Platforms.Length; PlatformIndex++)
			{
				if (CookerTools.Activate(CookerCfg.Platforms[PlatformIndex].ToString()))
				{
					Platform.Items.Add(CookerCfg.Platforms[PlatformIndex]);
				}
			}
		}
		#endregion

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(CookerFrontendWindow));
            this.TextInput = new System.Windows.Forms.TextBox();
            this.LogWindow = new System.Windows.Forms.RichTextBox();
            this.MapToLaunch = new System.Windows.Forms.TextBox();
            this.LaunchMap = new System.Windows.Forms.Button();
            this.Button_StartCooking = new System.Windows.Forms.Button();
            this.HavePackagesChanged = new System.Windows.Forms.CheckBox();
            this.CookingGroupBox = new System.Windows.Forms.GroupBox();
            this.label1 = new System.Windows.Forms.Label();
            this.CookInisIntsOnly = new System.Windows.Forms.CheckBox();
            this.CookFinalRelease = new System.Windows.Forms.CheckBox();
            this.PCConfiguration = new System.Windows.Forms.ComboBox();
            this.MapsToCookLabel = new System.Windows.Forms.Label();
            this.RunGroup = new System.Windows.Forms.GroupBox();
            this.RunWithCookedMap = new System.Windows.Forms.CheckBox();
            this.LaunchAfterCooking = new System.Windows.Forms.CheckBox();
            this.LabelCommandline = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.UseChapterPoint = new System.Windows.Forms.CheckBox();
            this.ChapterPointSelect = new System.Windows.Forms.NumericUpDown();
            this.ConsoleConfiguration = new System.Windows.Forms.ComboBox();
            this.RebootTarget = new System.Windows.Forms.Button();
            this.CFEMainMenu = new System.Windows.Forms.MainMenu(this.components);
            this.MenuItem_ImportMapList = new System.Windows.Forms.MenuItem();
            this.CopyButton = new System.Windows.Forms.Button();
            this.ToolsGroup = new System.Windows.Forms.GroupBox();
            this.TargetListBox = new System.Windows.Forms.CheckedListBox();
            this.ConsoleBaseLabel = new System.Windows.Forms.Label();
            this.BaseDirectory = new System.Windows.Forms.TextBox();
            this.Platform = new System.Windows.Forms.ComboBox();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.GameName = new System.Windows.Forms.ComboBox();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.CookingGroupBox.SuspendLayout();
            this.RunGroup.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.ChapterPointSelect)).BeginInit();
            this.ToolsGroup.SuspendLayout();
            this.SuspendLayout();
            // 
            // TextInput
            // 
            this.TextInput.Location = new System.Drawing.Point(81, 40);
            this.TextInput.Name = "TextInput";
            this.TextInput.Size = new System.Drawing.Size(839, 20);
            this.TextInput.TabIndex = 0;
            this.TextInput.Text = "entry ";
            this.toolTip1.SetToolTip(this.TextInput, "List of maps to cook for the given platform and game. Use a space to separate map" +
                    "s.\nStreaming sublevels will automatically be cooked, so only specify the persist" +
                    "ent level.");
            // 
            // LogWindow
            // 
            this.LogWindow.BackColor = System.Drawing.SystemColors.Window;
            this.LogWindow.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.LogWindow.Location = new System.Drawing.Point(8, 280);
            this.LogWindow.Name = "LogWindow";
            this.LogWindow.ReadOnly = true;
            this.LogWindow.Size = new System.Drawing.Size(912, 336);
            this.LogWindow.TabIndex = 14;
            this.LogWindow.Text = "";
            this.LogWindow.WordWrap = false;
            // 
            // MapToLaunch
            // 
            this.MapToLaunch.Location = new System.Drawing.Point(16, 88);
            this.MapToLaunch.Name = "MapToLaunch";
            this.MapToLaunch.Size = new System.Drawing.Size(200, 20);
            this.MapToLaunch.TabIndex = 12;
            this.toolTip1.SetToolTip(this.MapToLaunch, "Enter any commandline options you want to use when running the game (usually the " +
                    "name of the map you want to play).\nIf the \"(Use cooked map)\" option is selected," +
                    " this is ignored.");
            this.MapToLaunch.Visible = false;
            // 
            // LaunchMap
            // 
            this.LaunchMap.Location = new System.Drawing.Point(16, 152);
            this.LaunchMap.Name = "LaunchMap";
            this.LaunchMap.Size = new System.Drawing.Size(200, 23);
            this.LaunchMap.TabIndex = 13;
            this.LaunchMap.Text = "Launch Map";
            this.toolTip1.SetToolTip(this.LaunchMap, "Run the game on the selected targets using the commandline specified above.");
            this.LaunchMap.Click += new System.EventHandler(this.LaunchMap_Click);
            // 
            // Button_StartCooking
            // 
            this.Button_StartCooking.Location = new System.Drawing.Point(16, 152);
            this.Button_StartCooking.Name = "Button_StartCooking";
            this.Button_StartCooking.Size = new System.Drawing.Size(200, 24);
            this.Button_StartCooking.TabIndex = 5;
            this.Button_StartCooking.Text = "Start Cooking!";
            this.toolTip1.SetToolTip(this.Button_StartCooking, resources.GetString("Button_StartCooking.ToolTip"));
            this.Button_StartCooking.Click += new System.EventHandler(this.StartCooking_Click);
            // 
            // HavePackagesChanged
            // 
            this.HavePackagesChanged.Checked = true;
            this.HavePackagesChanged.CheckState = System.Windows.Forms.CheckState.Checked;
            this.HavePackagesChanged.Location = new System.Drawing.Point(24, 72);
            this.HavePackagesChanged.Name = "HavePackagesChanged";
            this.HavePackagesChanged.Size = new System.Drawing.Size(176, 16);
            this.HavePackagesChanged.TabIndex = 4;
            this.HavePackagesChanged.Text = "Packages have changed";
            this.toolTip1.SetToolTip(this.HavePackagesChanged, "Select this option to have the cooker always fully cook the maps and script code." +
                    " This should usually be checked.");
            // 
            // CookingGroupBox
            // 
            this.CookingGroupBox.Controls.Add(this.label1);
            this.CookingGroupBox.Controls.Add(this.CookInisIntsOnly);
            this.CookingGroupBox.Controls.Add(this.CookFinalRelease);
            this.CookingGroupBox.Controls.Add(this.Button_StartCooking);
            this.CookingGroupBox.Controls.Add(this.HavePackagesChanged);
            this.CookingGroupBox.Controls.Add(this.PCConfiguration);
            this.CookingGroupBox.Location = new System.Drawing.Point(8, 72);
            this.CookingGroupBox.Name = "CookingGroupBox";
            this.CookingGroupBox.Size = new System.Drawing.Size(232, 192);
            this.CookingGroupBox.TabIndex = 1;
            this.CookingGroupBox.TabStop = false;
            this.CookingGroupBox.Text = "Cooking";
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(16, 24);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(112, 16);
            this.label1.TabIndex = 11;
            this.label1.Text = "PC Configuration:";
            // 
            // CookInisIntsOnly
            // 
            this.CookInisIntsOnly.Location = new System.Drawing.Point(24, 96);
            this.CookInisIntsOnly.Name = "CookInisIntsOnly";
            this.CookInisIntsOnly.Size = new System.Drawing.Size(176, 16);
            this.CookInisIntsOnly.TabIndex = 6;
            this.CookInisIntsOnly.Text = "Cook Inis/Ints only";
            this.toolTip1.SetToolTip(this.CookInisIntsOnly, resources.GetString("CookInisIntsOnly.ToolTip"));
            // 
            // CookFinalRelease
            // 
            this.CookFinalRelease.Location = new System.Drawing.Point(24, 120);
            this.CookFinalRelease.Name = "CookFinalRelease";
            this.CookFinalRelease.Size = new System.Drawing.Size(176, 16);
            this.CookFinalRelease.TabIndex = 3;
            this.CookFinalRelease.Text = "Cook Final Release Script";
            this.toolTip1.SetToolTip(this.CookFinalRelease, "Select this option if you have compiled script (MyGame make -finalrelease) and yo" +
                    "u want to use those script packages when cooking.");
            // 
            // PCConfiguration
            // 
            this.PCConfiguration.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.PCConfiguration.Location = new System.Drawing.Point(16, 40);
            this.PCConfiguration.Name = "PCConfiguration";
            this.PCConfiguration.Size = new System.Drawing.Size(200, 21);
            this.PCConfiguration.TabIndex = 2;
            this.toolTip1.SetToolTip(this.PCConfiguration, "Select which PC executable configuration (debug or release) you would like to use" +
                    " when cooking maps (usually Release).");
            // 
            // MapsToCookLabel
            // 
            this.MapsToCookLabel.ForeColor = System.Drawing.SystemColors.ActiveCaption;
            this.MapsToCookLabel.Location = new System.Drawing.Point(8, 43);
            this.MapsToCookLabel.Name = "MapsToCookLabel";
            this.MapsToCookLabel.Size = new System.Drawing.Size(80, 16);
            this.MapsToCookLabel.TabIndex = 10;
            this.MapsToCookLabel.Text = "Maps to cook:";
            // 
            // RunGroup
            // 
            this.RunGroup.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.RunGroup.Controls.Add(this.RunWithCookedMap);
            this.RunGroup.Controls.Add(this.LaunchAfterCooking);
            this.RunGroup.Controls.Add(this.LabelCommandline);
            this.RunGroup.Controls.Add(this.label2);
            this.RunGroup.Controls.Add(this.UseChapterPoint);
            this.RunGroup.Controls.Add(this.ChapterPointSelect);
            this.RunGroup.Controls.Add(this.ConsoleConfiguration);
            this.RunGroup.Controls.Add(this.MapToLaunch);
            this.RunGroup.Controls.Add(this.LaunchMap);
            this.RunGroup.Location = new System.Drawing.Point(688, 82);
            this.RunGroup.Name = "RunGroup";
            this.RunGroup.Size = new System.Drawing.Size(232, 192);
            this.RunGroup.TabIndex = 3;
            this.RunGroup.TabStop = false;
            this.RunGroup.Text = "Run map";
            // 
            // RunWithCookedMap
            // 
            this.RunWithCookedMap.ForeColor = System.Drawing.SystemColors.ActiveCaption;
            this.RunWithCookedMap.Location = new System.Drawing.Point(104, 72);
            this.RunWithCookedMap.Name = "RunWithCookedMap";
            this.RunWithCookedMap.Size = new System.Drawing.Size(120, 16);
            this.RunWithCookedMap.TabIndex = 19;
            this.RunWithCookedMap.Text = "(Use cooked map)";
            this.toolTip1.SetToolTip(this.RunWithCookedMap, "Uses the \"Maps to cook\" string as the commandline.\nIf cooking multiple maps, the " +
                    "game should use the first map in the list and ignore the others.");
            this.RunWithCookedMap.CheckedChanged += new System.EventHandler(this.RunWithCookedMap_CheckedChanged);
            // 
            // LaunchAfterCooking
            // 
            this.LaunchAfterCooking.Location = new System.Drawing.Point(16, 120);
            this.LaunchAfterCooking.Name = "LaunchAfterCooking";
            this.LaunchAfterCooking.Size = new System.Drawing.Size(176, 16);
            this.LaunchAfterCooking.TabIndex = 18;
            this.LaunchAfterCooking.Text = "Launch after cooking/copying";
            this.toolTip1.SetToolTip(this.LaunchAfterCooking, "After cooking and/or syncing finish, launch the game.");
            // 
            // LabelCommandline
            // 
            this.LabelCommandline.Location = new System.Drawing.Point(16, 72);
            this.LabelCommandline.Name = "LabelCommandline";
            this.LabelCommandline.Size = new System.Drawing.Size(80, 16);
            this.LabelCommandline.TabIndex = 17;
            this.LabelCommandline.Text = "Commandline:";
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(16, 24);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(120, 16);
            this.label2.TabIndex = 16;
            this.label2.Text = "Console Configuration:";
            // 
            // UseChapterPoint
            // 
            this.UseChapterPoint.CheckAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.UseChapterPoint.Location = new System.Drawing.Point(16, 72);
            this.UseChapterPoint.Name = "UseChapterPoint";
            this.UseChapterPoint.Size = new System.Drawing.Size(144, 24);
            this.UseChapterPoint.TabIndex = 15;
            this.UseChapterPoint.Text = "Start from chapterpoint";
            this.UseChapterPoint.CheckStateChanged += new System.EventHandler(this.UseChapterPoint_StateChanged);
            // 
            // ChapterPointSelect
            // 
            this.ChapterPointSelect.Location = new System.Drawing.Point(168, 72);
            this.ChapterPointSelect.Maximum = new decimal(new int[] {
            69,
            0,
            0,
            0});
            this.ChapterPointSelect.Minimum = new decimal(new int[] {
            37,
            0,
            0,
            0});
            this.ChapterPointSelect.Name = "ChapterPointSelect";
            this.ChapterPointSelect.Size = new System.Drawing.Size(48, 20);
            this.ChapterPointSelect.TabIndex = 14;
            this.ChapterPointSelect.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.ChapterPointSelect.Value = new decimal(new int[] {
            37,
            0,
            0,
            0});
            // 
            // ConsoleConfiguration
            // 
            this.ConsoleConfiguration.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.ConsoleConfiguration.Location = new System.Drawing.Point(16, 40);
            this.ConsoleConfiguration.Name = "ConsoleConfiguration";
            this.ConsoleConfiguration.Size = new System.Drawing.Size(200, 21);
            this.ConsoleConfiguration.TabIndex = 11;
            this.toolTip1.SetToolTip(this.ConsoleConfiguration, "Select the configuration (debug, release, etc) of the game you want to run on the" +
                    " target console (usually Release).");
            // 
            // RebootTarget
            // 
            this.RebootTarget.Location = new System.Drawing.Point(120, 24);
            this.RebootTarget.Name = "RebootTarget";
            this.RebootTarget.Size = new System.Drawing.Size(96, 24);
            this.RebootTarget.TabIndex = 8;
            this.RebootTarget.Text = "Reboot Targets";
            this.toolTip1.SetToolTip(this.RebootTarget, "Reboot the selected targets.");
            this.RebootTarget.Click += new System.EventHandler(this.RebootTarget_Click);
            // 
            // CFEMainMenu
            // 
            this.CFEMainMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.MenuItem_ImportMapList});
            // 
            // MenuItem_ImportMapList
            // 
            this.MenuItem_ImportMapList.Index = 0;
            this.MenuItem_ImportMapList.Text = "Import Map List";
            this.MenuItem_ImportMapList.Click += new System.EventHandler(this.MenuItem_ImportMapList_Click);
            // 
            // CopyButton
            // 
            this.CopyButton.Location = new System.Drawing.Point(16, 24);
            this.CopyButton.Name = "CopyButton";
            this.CopyButton.Size = new System.Drawing.Size(96, 24);
            this.CopyButton.TabIndex = 7;
            this.CopyButton.Text = "Copy to Targets";
            this.toolTip1.SetToolTip(this.CopyButton, "Synchronizes the files on the selected targets with the cooked files on your PC.");
            this.CopyButton.Click += new System.EventHandler(this.CopyButton_Click);
            // 
            // ToolsGroup
            // 
            this.ToolsGroup.Controls.Add(this.CopyButton);
            this.ToolsGroup.Controls.Add(this.RebootTarget);
            this.ToolsGroup.Location = new System.Drawing.Point(352, 200);
            this.ToolsGroup.Name = "ToolsGroup";
            this.ToolsGroup.Size = new System.Drawing.Size(232, 64);
            this.ToolsGroup.TabIndex = 2;
            this.ToolsGroup.TabStop = false;
            this.ToolsGroup.Text = "Tools";
            // 
            // TargetListBox
            // 
            this.TargetListBox.Location = new System.Drawing.Point(352, 80);
            this.TargetListBox.Name = "TargetListBox";
            this.TargetListBox.Size = new System.Drawing.Size(232, 109);
            this.TargetListBox.TabIndex = 13;
            this.toolTip1.SetToolTip(this.TargetListBox, "Select the targets to use for copying, launching and rebooting operations.");
            // 
            // ConsoleBaseLabel
            // 
            this.ConsoleBaseLabel.Location = new System.Drawing.Point(568, 11);
            this.ConsoleBaseLabel.Name = "ConsoleBaseLabel";
            this.ConsoleBaseLabel.Size = new System.Drawing.Size(136, 16);
            this.ConsoleBaseLabel.TabIndex = 14;
            this.ConsoleBaseLabel.Text = "Console Base Directory:";
            // 
            // BaseDirectory
            // 
            this.BaseDirectory.Location = new System.Drawing.Point(698, 8);
            this.BaseDirectory.Name = "BaseDirectory";
            this.BaseDirectory.Size = new System.Drawing.Size(222, 20);
            this.BaseDirectory.TabIndex = 9;
            this.BaseDirectory.Text = "UnrealEngine3";
            this.toolTip1.SetToolTip(this.BaseDirectory, resources.GetString("BaseDirectory.ToolTip"));
            // 
            // Platform
            // 
            this.Platform.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.Platform.ItemHeight = 13;
            this.Platform.Location = new System.Drawing.Point(60, 8);
            this.Platform.Name = "Platform";
            this.Platform.Size = new System.Drawing.Size(180, 21);
            this.Platform.TabIndex = 15;
            this.toolTip1.SetToolTip(this.Platform, "Select the platform for which you are cooking data.");
            this.Platform.SelectedIndexChanged += new System.EventHandler(this.Platform_ValueChanged);
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(8, 11);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(56, 16);
            this.label3.TabIndex = 16;
            this.label3.Text = "Platform:";
            // 
            // label4
            // 
            this.label4.Location = new System.Drawing.Point(288, 11);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(40, 16);
            this.label4.TabIndex = 18;
            this.label4.Text = "Game:";
            // 
            // GameName
            // 
            this.GameName.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.GameName.ItemHeight = 13;
            this.GameName.Location = new System.Drawing.Point(327, 8);
            this.GameName.Name = "GameName";
            this.GameName.Size = new System.Drawing.Size(184, 21);
            this.GameName.TabIndex = 17;
            this.toolTip1.SetToolTip(this.GameName, "Select the game whose data you are cooking (more can be added in the CookerFronte" +
                    "nd_cfg.xml file).");
            this.GameName.SelectedIndexChanged += new System.EventHandler(this.GameName_ValueChanged);
            // 
            // toolTip1
            // 
            this.toolTip1.AutoPopDelay = 7000;
            this.toolTip1.InitialDelay = 500;
            this.toolTip1.IsBalloon = true;
            this.toolTip1.ReshowDelay = 100;
            // 
            // CookerFrontendWindow
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(928, 627);
            this.Controls.Add(this.TextInput);
            this.Controls.Add(this.BaseDirectory);
            this.Controls.Add(this.GameName);
            this.Controls.Add(this.Platform);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.ConsoleBaseLabel);
            this.Controls.Add(this.ToolsGroup);
            this.Controls.Add(this.RunGroup);
            this.Controls.Add(this.MapsToCookLabel);
            this.Controls.Add(this.CookingGroupBox);
            this.Controls.Add(this.LogWindow);
            this.Controls.Add(this.TargetListBox);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.Menu = this.CFEMainMenu;
            this.MinimumSize = new System.Drawing.Size(934, 6);
            this.Name = "CookerFrontendWindow";
            this.Text = "Unreal Cooker Frontend";
            this.Closing += new System.ComponentModel.CancelEventHandler(this.CookFrontEnd_Closing);
            this.Load += new System.EventHandler(this.CookerFrontendWindow_Load);
            this.CookingGroupBox.ResumeLayout(false);
            this.RunGroup.ResumeLayout(false);
            this.RunGroup.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.ChapterPointSelect)).EndInit();
            this.ToolsGroup.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

		}
		#endregion

		#region Main program loop
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main(string[] Args) 
		{
			CookerFrontendWindow CookerFrontEnd = new CookerFrontendWindow();
			if( !CookerFrontEnd.Init() )
			{
				return;
			}

			while( CookerFrontEnd.Running )
			{
				// Process system messages (allow OnTimer to be called)
				Application.DoEvents();

				// Tick changes once every 100ms
				if( CookerFrontEnd.GetTicked() != 0 )
				{
					CookerFrontEnd.RunFrame();
				}

				// Yield to system
				System.Threading.Thread.Sleep( 5 );
			}
		}
		#endregion

		#region Log window helpers.

		void AddLine( string Line, Color TextColor )
		{
			Monitor.Enter( LogWindow );

			LogWindow.Focus();
			LogWindow.SelectionColor = TextColor;
			LogWindow.SelectionStart = LogWindow.TextLength;
			LogWindow.AppendText( Line + "\r\n" );

			Monitor.Exit( LogWindow );
		}
		
		// IOutputHandler interface
		void CookerTools.IOutputHandler.OutputText(string Text)
		{
			AddLine( Text, LogWindow.ForeColor );
		}

		void CookerTools.IOutputHandler.OutputText( string Text, Color OutputColor )
		{
			AddLine( Text, OutputColor );
		}

		/**
		 * Performs output gathering on another thread
		 */
		private void PollForOutput()
		{
			LogPipe = new NamedPipe();
			LogPipe.Connect( CookerCommandlet );

			while( CookerCommandlet != null )
			{
				try
				{
					string StdOutLine = LogPipe.Read();
					// Add that line to the output array in a thread-safe way
					Monitor.Enter( LogTextArray );
					if( StdOutLine.Length > 0 )
					{
						LogTextArray.Add( StdOutLine.Replace( "\r\n", "" ) );
					}
				}
				catch( ThreadAbortException )
				{
				}
				finally
				{
					Monitor.Exit( LogTextArray );
				}
			}

			LogPipe.Disconnect();
			LogPolling = null;
		}

		int GetTicked()
		{	
			int	Ticked = TickCount - LastTickCount;
			LastTickCount = TickCount;
			return( Ticked );
		}

		private void OnTimer( object sender, System.EventArgs e )
		{
			TickCount++;
		}
		#endregion

		#region Main functions
		public void RunFrame()
		{
			if( CookerCommandlet == null )
			{
				return;
			}

			if( !CookerCommandlet.HasExited )
			{
				// Lock the array for thread safety.
				Monitor.Enter( LogTextArray );
	
				// Iterate over all of the strings in the array.
				foreach( string Line in LogTextArray )
				{
					Color TextColor = LogWindow.ForeColor;
					// Figure out which color to use for line.
					if( Line.StartsWith( UNI_COLOR_MAGIC ) )
					{
						// Ignore any special console colours
						continue;
					}
					else if( Line.StartsWith( "Warning" ) )
					{
						TextColor = Color.Orange;
					}
					else if( Line.StartsWith( "Error" ) )
					{
						TextColor = Color.Red;
					}

					// Add the line to the log window with the appropriate color.
					AddLine( Line, TextColor );
				}

				// Empty the array and release the lock
				LogTextArray.Clear();
				Monitor.Exit( LogTextArray );
			}
			else
			{
				StopCommandlet();
				AddLine( "\r\n[COMMANDLET FINISHED]\r\n", Color.Green );
	
				// Sync up PC/console.
				SyncWithRetry();

				// if the user wants to, run the game after cooking is finished
				if (LaunchAfterCooking.Checked)
				{
					LaunchMap_Click(null, null);
				}

			}
		}

		/// <summary>
		/// Retry syncing to the target until it succeeds or the user chooses to cancel
		/// </summary>
		private void SyncWithRetry()
		{
			SetCopyingState( true );

			CacheGameNameAndConsoles();

			while (CookerTools.TargetSync() == false)
			{
				// Allow them to cancel if need be
				if( MessageBox.Show(
						"Please make sure your target is on and connected to the network", 
						"Can't find target",	
						MessageBoxButtons.RetryCancel ) == DialogResult.Cancel )
				{
					SetCopyingState( false );
					return;
				}
			}

			AddLine( "\r\n[ALL TARGETS PROCESSED]\r\n", Color.Green );
			SetCopyingState( false );
		}

		private void CheckedReboot(bool bShouldRunGame, string CommandLine)
		{
			CacheGameNameAndConsoles();

			AddLine( "\r\n[REBOOTING]\r\n", Color.Green );

			if (CookerTools.TargetReboot(bShouldRunGame, ConsoleConfiguration.SelectedItem.ToString(), CommandLine) == false)
			{
				MessageBox.Show( 
					this, 
					"Failed to reboot console. Check log for details.",
					"Reboot failed",
					MessageBoxButtons.OK,
					MessageBoxIcon.Error );
			}
		}
		#endregion

		#region Cooking buttons

		private void CleanupXMAWorkFiles()
		{
			AddLine( "[DELETING TEMPORARY XMA ENCODER FILES]\r\n", Color.Green );
			try
			{
				// Get temporary files left behind by XMA encoder and nuke them.
				string[] TempFiles = Directory.GetFiles( Path.GetTempPath(), "EncStrm*" );
				foreach( string TempFile in TempFiles )
				{
					File.Delete( TempFile );
				}
			}
			catch( Exception Error )
			{
				AddLine( Error.ToString(), Color.Red );
			}
		}

		private string GetExecutableName()
		{
			// Figure out executable and command line.
			string Executable = Application.StartupPath + "\\";

			if( PCConfiguration.SelectedItem.ToString() == "Debug" )
			{
				Executable += "Debug-";
			}

			Executable += CookerTools.GetGameName() + "Game.exe";
			return( Executable );	
		}

		private string GetCookingCommandLine()
		{
			// Base command
			string CommandLine = "CookPackages -platform=" + Platform.Text;

			if( CookInisIntsOnly.Checked )
			{
				CommandLine += " -inisOnly";
			}
			else
			{
				// Add in map name
				CommandLine += " " + TextInput.Text;

				if( HavePackagesChanged.Checked )
				{
					CommandLine += " -alwaysRecookmaps -alwaysRecookScript";
				}

				// Skip cooking maps if we didn't specify any.
				if( TextInput.Text.Trim().Length == 0 )
				{
					CommandLine += " -skipMaps";
				}
			}

			// Add in the final release option if necessary
			if( CookFinalRelease.Checked )
			{
				CommandLine += " -final_release";
			}

			// we always want to have the latest .inis (this stops one from firing off the cooker and then coming back and seeing the "update .inis" message 
			// and then committing seppuku)
			CommandLine += " -updateInisAuto";

			return( CommandLine );
		}

		private void SetCookingState( bool On )
		{
			if( On )
			{
				// Disable start button while we are running.
				CopyButton.Enabled = false;
				LaunchMap.Enabled = false;

				// Kick off the thread that monitors the commandlet's ouput
				ThreadStart ThreadDelegate = new ThreadStart( PollForOutput );
				LogPolling = new Thread( ThreadDelegate );
				LogPolling.Start();

				// start the timer to read output
				CookerCommandletTimer.Start();

				Button_StartCooking.Text = "Stop Cooking!";
				
				IsCooking = true;
			}
			else
			{
				// Enable start button again.
				CopyButton.Enabled = true;
				LaunchMap.Enabled = true;

				// Stop the timer now.
				CookerCommandletTimer.Stop();
				
				Button_StartCooking.Text = "Start Cooking!";

				IsCooking = false;
			}
		}

		private void SetCopyingState( bool On )
		{
			if( On )
			{
				Button_StartCooking.Enabled = false;
				CopyButton.Enabled = false;
				RebootTarget.Enabled = false;
				LaunchMap.Enabled = false;
			}
			else
			{
				Button_StartCooking.Enabled = true;
				CopyButton.Enabled = CookerTools.PlatformNeedsToSync();
				RebootTarget.Enabled = true;
				LaunchMap.Enabled = true;
			}
		}

		/**
		 * Spawn a copy of the commandlet and trap the output
		 */
		private void RunCommandlet()
		{
			AddLine( "[COMMANDLET STARTED]\r\n", Color.Green );

			string Executable = GetExecutableName();
			string CommandLine = GetCookingCommandLine();
			
			// Launch the cooker.
			CookerCommandlet = ExecuteProgram( Executable, CommandLine );

			// Launch successful.
			if( CookerCommandlet != null )
			{
				SetCookingState( true );
			}
			else
			{
				// Failed to launch executable.
				MessageBox.Show(
					this,
					"Failed to launch commandlet (" + Executable + "). Check your settings and try again.",
					"Launch Failed",
					MessageBoxButtons.OK,
					MessageBoxIcon.Error );
			}		
		}

		private void StopCommandlet()
		{
			if( CookerCommandlet != null && !CookerCommandlet.HasExited )
			{
				// Need to null this in a threadsafe way
				Monitor.Enter( CookerCommandlet );
				// Since we entered it means the thread is done with it
				Monitor.Exit( CookerCommandlet );

				CookerCommandlet.Kill();
				CookerCommandlet.WaitForExit();
			}
			CookerCommandlet = null;

			SetCookingState( false );
		}

		private void StartCooking()
		{
			LogWindow.Clear();

			CacheGameNameAndConsoles();

			CleanupXMAWorkFiles();

			RunCommandlet();
		}

		private bool StopCooking()
		{
			DialogResult result = MessageBox.Show( "Are you sure you want to stop cooking?", "Stop cooking", MessageBoxButtons.YesNo, MessageBoxIcon.Question );
			if( result == DialogResult.Yes )
			{
				StopCommandlet();
				AddLine( "\r\n[COMMANDLET ABORTED]\r\n", Color.Green );
				return( true );
			}

			return( false );
		}

		private void StartCooking_Click( object sender, System.EventArgs e )
		{
			if( IsCooking == false )
			{
				StartCooking();
			}
			else
			{
				StopCooking();
			}
		}

		#endregion

		#region Misc buttons
		private void LaunchMap_Click(object sender, System.EventArgs e)
		{
			// Reboot target, launch executable with commandline of map to load
			string Commandline = RunWithCookedMap.Checked ?
				TextInput.Text.ToLower() :
				MapToLaunch.Text.ToLower();

			// If we're in Wargame, and the map is of the form "sp_XXXX_p", then use the streaming notation
			if( GameName.SelectedItem.ToString() == "WarGame" )
			{
				if( UseChapterPoint.Checked )
				{
					Commandline = "wargame_p?loadchapter=" + ChapterPointSelect.Value.ToString();
				}
				else
				{
					Commandline = "";
				}
			}
			
			CheckedReboot(true, Commandline);
		}

		private void RebootTarget_Click(object sender, System.EventArgs e)
		{
			CheckedReboot(false, null);
		}

		private void CookerFrontendWindow_Load( object sender, System.EventArgs e )
		{
			string[] cmdparms = Environment.GetCommandLineArgs();

			string Xedk = Environment.GetEnvironmentVariable( "XEDK" );
			string CurrentPath = Environment.GetEnvironmentVariable( "Path" );

			SetEnvironmentVariableA( "Path", CurrentPath + ";" + Xedk + "\\bin\\win32" );

			if( cmdparms.Length == 2 && File.Exists( cmdparms[1] ) )
			{
				ImportMapList( cmdparms[1] );
				StartCooking();
			}
		}
		private void MenuItem_ImportMapList_Click(object sender, System.EventArgs e)
		{
			OpenFileDialog ofd;
			ofd = new OpenFileDialog();
			ofd.Filter = "TXT Files (*.txt)|*.txt";
			if( ofd.ShowDialog() == DialogResult.OK )
			{
				ImportMapList(ofd.FileName);
			}
		}
		private void ImportMapList(string filename)
		{
			System.IO.StreamReader streamIn = new System.IO.StreamReader(filename);
			TextInput.Text = streamIn.ReadToEnd().Replace("\r\n"," ");
			streamIn.Close();
		}

		private void CopyButton_Click(object sender, System.EventArgs e)
		{
			LogWindow.Clear();

			// Copy data to console.
			SyncWithRetry();

            // if the user wants to, run the game after cooking/syncing is finished
            if (LaunchAfterCooking.Checked)
            {
                LaunchMap_Click(null, null);
            }

		}
		#endregion

		private void GameName_ValueChanged( object sender, System.EventArgs e )
		{
			string Game = GameName.Text.ToLower();
			if( Game == "wargame" )
			{
				UseChapterPoint_StateChanged( null, null );

				ChapterPointSelect.Show();
				UseChapterPoint.Show();
				LabelCommandline.Hide();
				MapToLaunch.Hide();
				RunWithCookedMap.Hide();
			}
			else
			{
				ChapterPointSelect.Hide();
				UseChapterPoint.Hide();
				LabelCommandline.Show();
				MapToLaunch.Show();			
				RunWithCookedMap.Show();
			}
		}

		private void UseChapterPoint_StateChanged( object sender, System.EventArgs e )
		{
			ChapterPointSelect.Enabled = false;
			if( UseChapterPoint.Checked )
			{
				ChapterPointSelect.Enabled = true;
			}
		}

		private void CookFrontEnd_Closing(object sender, System.ComponentModel.CancelEventArgs e)
		{
			if( CookerCommandlet != null )
			{
				if( !StopCooking() )
				{
					// Don't exit!
					e.Cancel = true;
				}
			}
		}

		private void Platform_ValueChanged(object sender, System.EventArgs e)
		{
			// Only add a list of targets if the CookerTools say it is OK
			if (CookerTools != null && CookerTools.Activate(Platform.Text))
			{
				// Getlist of all known consoles
				ArrayList ConsoleList = CookerTools.GetKnownConsoles();
				string DefaultConsole = CookerTools.GetDefaultConsole();

				// remove existing entries
				TargetListBox.Items.Clear();

				// Initialize the consoles list
				for (int Index = 0; Index < ConsoleList.Count; Index++)
				{
					// Check this item if it is the default
					TargetListBox.Items.Add(ConsoleList[Index], (string)ConsoleList[Index] == DefaultConsole);
				}

				// gray out the sync button if the platform doesnt need it
				CopyButton.Enabled = CookerTools.PlatformNeedsToSync();
			}
		}

		private void RunWithCookedMap_CheckedChanged(object sender, System.EventArgs e)
		{
			MapToLaunch.Enabled = !RunWithCookedMap.Checked;
		}
	}
}
