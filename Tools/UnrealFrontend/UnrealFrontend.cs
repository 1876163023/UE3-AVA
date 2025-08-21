using System;
using System.Drawing;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using System.Diagnostics;
using System.Threading;
using System.IO;
using System.Runtime.InteropServices;
using System.Xml;
using System.Xml.Serialization;
using System.Text;
using System.Text.RegularExpressions;
using CookerTools;
using Pipes;

namespace UnrealFrontend
{
	/// <summary>
	/// Form for displaying information/interacting with the user
	/// </summary>
	public class UnrealFrontendWindow : System.Windows.Forms.Form, CookerTools.IOutputHandler
	{
		#region Variables / enums

		private System.ComponentModel.IContainer components;

		/** true if the form is currently being disposed */
		private bool bDisposing = false;

		/** Holds all of the map extensions for the various game types. */
		private Dictionary<string, string> MapExtensionDictionary = new Dictionary<string, string>();

		/** A map from platform to targets and their associated state (true or false)*/
		private Dictionary<string, Dictionary<string, bool>> PlatformDefaults = new Dictionary<string, Dictionary<string, bool>>();
		
		// current version
		private const string VersionStr = "Unreal Frontend v0.9.0";

		/** Path to where to store settings files */
		private string SettingsPath;

		/** Hardcoded name for UFE settings */
		private const string SettingsFilename = "UnrealFrontend.ini";

		private const string UNI_COLOR_MAGIC = "`~[~`";

		public bool Running = false;
		private int TickCount = 0;
		private int LastTickCount = 0;

		/** Thread safe array used to get data from the command console and display.		*/
		private ArrayList LogTextArray = new ArrayList();
		/** Thread that polls the console placing the output in the OutputTextArray.		*/
		private Thread LogPolling;

		/** Instance of the cooker tools helper class										*/
		private CookerToolsClass CookerTools;
		// Use a default color for the forecolor instead of querying the log window
		private Color DefaultLogWindowForegroundColor;

		/** Instance of pipes helper class */
		private NamedPipe LogPipe;

		// Holds the game name, from the combo box
		private string CachedGameName;


		// lists of options that we serialize
		private ArrayList Options;
		private ArrayList SPOptions;
		private ArrayList MPOptions;
		private ArrayList CompileOptions;
		private ArrayList CookingOptions;
        private ArrayList PS3CompileOptions;
        private Dictionary<string, ArrayList> PerGameOptions;

		// number of clients currently active
		private int NumClients = 0;
		// list of processes spawned
		private ArrayList Processes = new ArrayList();

        // make a list of usable platforms
        List<string> UsablePlatforms = new List<string>();
        
        // properties for placing the app in the system tray
		private NotifyIcon TrayIcon;
		private ContextMenu TrayMenu;

		/** Types of commandlets - used to know which buttons to disable, etc */
		enum ECommandletType
		{
			CT_None,
			CT_Cook,
			CT_Compile,
		}
		/** Currently running commandlet type												*/
		private ECommandletType RunningCommandletType = ECommandletType.CT_None;

		/** The running Commandlet process, for reading output and canceling.				*/
		private Process CommandletProcess;

		/** A Timer object used to read output from the commandlet every N ms.				*/
		private System.Windows.Forms.Timer CommandletTimer;

		/** A dialog for asking the name of the mod											*/
		private StringQuery ModNameDialog = new StringQuery();

		enum ECurrentConfig
		{
			Global,
			SP,
			MP,
		};

		#endregion

		#region Windows Forms variables

		private System.Windows.Forms.CheckBox CheckBox_UseCookedMap;
		private RichTextBox RichTextBox_LogWindow;
		private TabPage Tab_Cooking;
		private GroupBox CookingGroupBox;
		private CheckBox CheckBox_LaunchAfterCooking;
		private CheckBox CheckBox_CookInisIntsOnly;
		private CheckBox CheckBox_CookFinalReleaseScript;
		private Button Button_StartCooking;
		private Button Button_ImportMapList;
		private TextBox TextBox_MapsToCook;
		private Label MapsToCookLabel;
		private TabPage Tab_ConsoleSetup;
		private CheckedListBox ListBox_Targets;
		private Label label2;
		private ComboBox ComboBox_ConsoleConfiguration;
		private Label ConsoleBaseLabel;
		private TabPage Tab_PCSetup;
		private Label label1;
		private ComboBox ComboBox_PCConfiguration;
		private ComboBox ComboBox_ProjectDir;
		private Label label8;
		private TabPage Tab_Multiplayer;
		private GroupBox groupBox3;
		private ListBox ListBox_ServerType;
		private TextBox TextBox_ExecMP;
		private Label label6;
		private CheckBox CheckBox_UnattendedMP;
		private ComboBox ComboBox_ResolutionMP;
		private CheckBox CheckBox_FixedSeedMP;
		private CheckBox CheckBox_VSyncMP;
		private CheckBox CheckBox_RCMP;
		private CheckBox CheckBox_PostProcessMP;
		private CheckBox CheckBox_UnlitMP;
		private CheckBox CheckBox_MultiThreadMP;
		private CheckBox CheckBox_ShowLogMP;
		private CheckBox CheckBox_SoundMP;
		private GroupBox groupBox4;
		private Button Button_BrowseMP;
		private Button Button_KillAll;
		private ComboBox ComboBox_MapnameMP;
		private NumericUpDown NumericUpDown_Clients;
		private Button Button_ServerClient;
		private Button Button_AddClient;
		private Button Button_Server;
		private TabPage Tab_Local;
		private GroupBox groupBox5;
		private TextBox TextBox_ExecSP;
		private Label label5;
		private CheckBox CheckBox_UnattendedSP;
		private ComboBox ComboBox_ResolutionSP;
		private CheckBox CheckBox_FixedSeedSP;
		private CheckBox CheckBox_VSyncSP;
		private CheckBox CheckBox_RCSP;
		private CheckBox CheckBox_PostProcessSP;
		private CheckBox CheckBox_UnlitSP;
		private CheckBox CheckBox_MultiThreadSP;
		private CheckBox CheckBox_ShowLogSP;
		private CheckBox CheckBox_SoundSP;
		private GroupBox groupBox2;
		private Button Button_Editor;
		private Button Button_BrowseSP;
		private ComboBox ComboBox_MapnameSP;
		private Button Button_LoadMap;
		private TabControl TabControl_Main;
		private GroupBox groupBox1;
		private Button Button_RebootTargets;
		private Button Button_CopyToTargets;
		private Label label7;
		private ComboBox ComboBox_URLOptionsSP;
		private Label label9;
		private ComboBox ComboBox_URLOptionsMP;
		private GroupBox groupBox6;
		private Label label10;
		private Button Button_Compile;
		private ComboBox ComboBox_BaseDirectory;
		private System.Windows.Forms.ToolTip FormToolTips;
		private ComboBox ComboBox_ScriptConfig;
		private CheckBox CheckBox_FullScriptCompile;
		private CheckBox CheckBox_AutomaticCompileMode;
		private TabPage Tab_Help;
		private RichTextBox RichTextBox_Help;
		private Panel panel1;
		private Label label11;
		private CheckBox CheckBox_RunWithCookedData;
		private CheckBox UseChapterPoint;
		private NumericUpDown ChapterPointSelect;
		private StatusStrip statusStrip1;
		private ToolStripStatusLabel ToolStripStatusLabel_PCConfig;
		private ToolStripStatusLabel ToolStripStatusLabel_ConsoleConfig;
		private ToolStripStatusLabel ToolStripStatusLabel_LocalMap;
		private ToolStripStatusLabel ToolStripStatusLabel_MPMap;
		private ToolStripStatusLabel ToolStripStatusLabel_CookMaps;
		private ToolStripStatusLabel toolStripStatusLabel1;
		private ToolStripStatusLabel toolStripStatusLabel2;
		private ToolStripStatusLabel toolStripStatusLabel3;
		private ToolStripStatusLabel toolStripStatusLabel4;
		private ToolStripStatusLabel toolStripStatusLabel5;
		private ToolStripStatusLabel toolStripStatusLabel6;
		private ToolStripStatusLabel ToolStripStatusLabel_URL;
		private ComboBox ComboBox_CookingLanguage;
		private ToolStripLabel toolStripLabel1;
		private ToolStripComboBox ComboBox_Platform;
		private ToolStripSeparator toolStripSeparator1;
		private ToolStripLabel toolStripLabel2;
		private ToolStripComboBox ComboBox_Game;
		private ToolStripSeparator toolStripSeparator2;
		private ToolStripButton ToolStripButton_Launch;
		private ToolStripButton ToolStripButton_Editor;
		private ToolStripSeparator toolStripSeparator3;
		private ToolStripSeparator toolStripSeparator4;
		private ToolStripButton ToolStripButton_Cook;
		private ToolStripSeparator toolStripSeparator5;
		private ToolStripButton ToolStripButton_CompileScript;
		private ToolStripSeparator toolStripSeparator6;
		private ToolStripButton ToolStripButton_Sync;
		private ToolStripButton ToolStripButton_Reboot;
		private Label label3;
		private ToolStripSplitButton ToolStripButton_Server;
		private ToolStripMenuItem ToolStripButton_ServerAndClients;
		private ToolStripMenuItem ToolStripButton_AddClient;
		private ToolStripMenuItem ToolStripButton_KillAll;
		private CheckBox CheckBox_FullCook;
		private TabPage Tab_PS3;
        private Label Label_Threads2;
		private CheckBox CheckBox_FullRebuild;
		private CheckBox CheckBox_NoDebugInfo;
        private Button Button_PS3Compile;
        private ToolStripSeparator toolStripSeparator7;
        private ToolStripSeparator toolStripSeparator8;
        private NumericUpDown NumericUpDown_NumberOfThreads;
        private ComboBox ComboBox_SelectPS3Compile;
        private Label Label_AddProfileInfo;
        private ComboBox ComboBox_AddProfInfo;
        private CheckBox CheckBox_LaunchAfterPS3Compile;
        private CheckBox CheckBox_StopOnFirstError;
		private Button Button_PackageMod;
		private CheckBox CheckBox_CookInUserMode;
		private ToolTip CookMapToolTip;
		private CheckBox CheckBox_ForceCookSeekfree;
		private CheckBox CheckBox_CookAllMaps;
        private CheckBox CheckBox_QuietMode;
		private ToolStrip toolStrip1;
		#endregion

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.components = new System.ComponentModel.Container();
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(UnrealFrontendWindow));
			this.CheckBox_UseCookedMap = new System.Windows.Forms.CheckBox();
			this.FormToolTips = new System.Windows.Forms.ToolTip(this.components);
			this.TextBox_MapsToCook = new System.Windows.Forms.TextBox();
			this.Button_StartCooking = new System.Windows.Forms.Button();
			this.CheckBox_CookFinalReleaseScript = new System.Windows.Forms.CheckBox();
			this.CheckBox_CookInisIntsOnly = new System.Windows.Forms.CheckBox();
			this.CheckBox_LaunchAfterCooking = new System.Windows.Forms.CheckBox();
			this.ComboBox_ConsoleConfiguration = new System.Windows.Forms.ComboBox();
			this.ListBox_Targets = new System.Windows.Forms.CheckedListBox();
			this.ComboBox_PCConfiguration = new System.Windows.Forms.ComboBox();
			this.Button_CopyToTargets = new System.Windows.Forms.Button();
			this.Button_RebootTargets = new System.Windows.Forms.Button();
			this.ComboBox_ProjectDir = new System.Windows.Forms.ComboBox();
			this.Button_Compile = new System.Windows.Forms.Button();
			this.ComboBox_URLOptionsSP = new System.Windows.Forms.ComboBox();
			this.TextBox_ExecSP = new System.Windows.Forms.TextBox();
			this.CheckBox_UnattendedSP = new System.Windows.Forms.CheckBox();
			this.ComboBox_ResolutionSP = new System.Windows.Forms.ComboBox();
			this.CheckBox_FixedSeedSP = new System.Windows.Forms.CheckBox();
			this.CheckBox_VSyncSP = new System.Windows.Forms.CheckBox();
			this.CheckBox_RCSP = new System.Windows.Forms.CheckBox();
			this.CheckBox_PostProcessSP = new System.Windows.Forms.CheckBox();
			this.CheckBox_UnlitSP = new System.Windows.Forms.CheckBox();
			this.CheckBox_MultiThreadSP = new System.Windows.Forms.CheckBox();
			this.CheckBox_ShowLogSP = new System.Windows.Forms.CheckBox();
			this.CheckBox_SoundSP = new System.Windows.Forms.CheckBox();
			this.Button_Editor = new System.Windows.Forms.Button();
			this.Button_BrowseSP = new System.Windows.Forms.Button();
			this.ComboBox_MapnameSP = new System.Windows.Forms.ComboBox();
			this.Button_LoadMap = new System.Windows.Forms.Button();
			this.ComboBox_BaseDirectory = new System.Windows.Forms.ComboBox();
			this.CheckBox_RunWithCookedData = new System.Windows.Forms.CheckBox();
			this.Button_BrowseMP = new System.Windows.Forms.Button();
			this.Button_KillAll = new System.Windows.Forms.Button();
			this.ComboBox_MapnameMP = new System.Windows.Forms.ComboBox();
			this.NumericUpDown_Clients = new System.Windows.Forms.NumericUpDown();
			this.Button_ServerClient = new System.Windows.Forms.Button();
			this.Button_AddClient = new System.Windows.Forms.Button();
			this.Button_Server = new System.Windows.Forms.Button();
			this.ComboBox_CookingLanguage = new System.Windows.Forms.ComboBox();
			this.CheckBox_FullCook = new System.Windows.Forms.CheckBox();
			this.ComboBox_ScriptConfig = new System.Windows.Forms.ComboBox();
			this.CheckBox_AutomaticCompileMode = new System.Windows.Forms.CheckBox();
			this.CheckBox_FullScriptCompile = new System.Windows.Forms.CheckBox();
			this.CheckBox_CookInUserMode = new System.Windows.Forms.CheckBox();
			this.CheckBox_ForceCookSeekfree = new System.Windows.Forms.CheckBox();
			this.CheckBox_CookAllMaps = new System.Windows.Forms.CheckBox();
			this.CheckBox_QuietMode = new System.Windows.Forms.CheckBox();
			this.RichTextBox_LogWindow = new System.Windows.Forms.RichTextBox();
			this.Tab_Cooking = new System.Windows.Forms.TabPage();
			this.CookingGroupBox = new System.Windows.Forms.GroupBox();
			this.MapsToCookLabel = new System.Windows.Forms.Label();
			this.Button_ImportMapList = new System.Windows.Forms.Button();
			this.label3 = new System.Windows.Forms.Label();
			this.Tab_ConsoleSetup = new System.Windows.Forms.TabPage();
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.label2 = new System.Windows.Forms.Label();
			this.ConsoleBaseLabel = new System.Windows.Forms.Label();
			this.Tab_PCSetup = new System.Windows.Forms.TabPage();
			this.groupBox6 = new System.Windows.Forms.GroupBox();
			this.label10 = new System.Windows.Forms.Label();
			this.label1 = new System.Windows.Forms.Label();
			this.label8 = new System.Windows.Forms.Label();
			this.Tab_Multiplayer = new System.Windows.Forms.TabPage();
			this.groupBox3 = new System.Windows.Forms.GroupBox();
			this.label9 = new System.Windows.Forms.Label();
			this.ComboBox_URLOptionsMP = new System.Windows.Forms.ComboBox();
			this.ListBox_ServerType = new System.Windows.Forms.ListBox();
			this.TextBox_ExecMP = new System.Windows.Forms.TextBox();
			this.label6 = new System.Windows.Forms.Label();
			this.CheckBox_UnattendedMP = new System.Windows.Forms.CheckBox();
			this.ComboBox_ResolutionMP = new System.Windows.Forms.ComboBox();
			this.CheckBox_FixedSeedMP = new System.Windows.Forms.CheckBox();
			this.CheckBox_VSyncMP = new System.Windows.Forms.CheckBox();
			this.CheckBox_RCMP = new System.Windows.Forms.CheckBox();
			this.CheckBox_PostProcessMP = new System.Windows.Forms.CheckBox();
			this.CheckBox_UnlitMP = new System.Windows.Forms.CheckBox();
			this.CheckBox_MultiThreadMP = new System.Windows.Forms.CheckBox();
			this.CheckBox_ShowLogMP = new System.Windows.Forms.CheckBox();
			this.CheckBox_SoundMP = new System.Windows.Forms.CheckBox();
			this.groupBox4 = new System.Windows.Forms.GroupBox();
			this.Tab_Local = new System.Windows.Forms.TabPage();
			this.groupBox5 = new System.Windows.Forms.GroupBox();
			this.label7 = new System.Windows.Forms.Label();
			this.label5 = new System.Windows.Forms.Label();
			this.groupBox2 = new System.Windows.Forms.GroupBox();
			this.ChapterPointSelect = new System.Windows.Forms.NumericUpDown();
			this.UseChapterPoint = new System.Windows.Forms.CheckBox();
			this.TabControl_Main = new System.Windows.Forms.TabControl();
			this.Tab_PS3 = new System.Windows.Forms.TabPage();
			this.Button_PackageMod = new System.Windows.Forms.Button();
			this.CheckBox_LaunchAfterPS3Compile = new System.Windows.Forms.CheckBox();
			this.CheckBox_StopOnFirstError = new System.Windows.Forms.CheckBox();
			this.Label_AddProfileInfo = new System.Windows.Forms.Label();
			this.ComboBox_AddProfInfo = new System.Windows.Forms.ComboBox();
			this.ComboBox_SelectPS3Compile = new System.Windows.Forms.ComboBox();
			this.NumericUpDown_NumberOfThreads = new System.Windows.Forms.NumericUpDown();
			this.Button_PS3Compile = new System.Windows.Forms.Button();
			this.Label_Threads2 = new System.Windows.Forms.Label();
			this.CheckBox_FullRebuild = new System.Windows.Forms.CheckBox();
			this.CheckBox_NoDebugInfo = new System.Windows.Forms.CheckBox();
			this.Tab_Help = new System.Windows.Forms.TabPage();
			this.RichTextBox_Help = new System.Windows.Forms.RichTextBox();
			this.panel1 = new System.Windows.Forms.Panel();
			this.label11 = new System.Windows.Forms.Label();
			this.statusStrip1 = new System.Windows.Forms.StatusStrip();
			this.toolStripStatusLabel1 = new System.Windows.Forms.ToolStripStatusLabel();
			this.ToolStripStatusLabel_PCConfig = new System.Windows.Forms.ToolStripStatusLabel();
			this.toolStripStatusLabel2 = new System.Windows.Forms.ToolStripStatusLabel();
			this.ToolStripStatusLabel_ConsoleConfig = new System.Windows.Forms.ToolStripStatusLabel();
			this.toolStripStatusLabel3 = new System.Windows.Forms.ToolStripStatusLabel();
			this.ToolStripStatusLabel_LocalMap = new System.Windows.Forms.ToolStripStatusLabel();
			this.toolStripStatusLabel4 = new System.Windows.Forms.ToolStripStatusLabel();
			this.ToolStripStatusLabel_MPMap = new System.Windows.Forms.ToolStripStatusLabel();
			this.toolStripStatusLabel5 = new System.Windows.Forms.ToolStripStatusLabel();
			this.ToolStripStatusLabel_CookMaps = new System.Windows.Forms.ToolStripStatusLabel();
			this.toolStripStatusLabel6 = new System.Windows.Forms.ToolStripStatusLabel();
			this.ToolStripStatusLabel_URL = new System.Windows.Forms.ToolStripStatusLabel();
			this.toolStrip1 = new System.Windows.Forms.ToolStrip();
			this.toolStripLabel1 = new System.Windows.Forms.ToolStripLabel();
			this.ComboBox_Platform = new System.Windows.Forms.ToolStripComboBox();
			this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
			this.toolStripLabel2 = new System.Windows.Forms.ToolStripLabel();
			this.ComboBox_Game = new System.Windows.Forms.ToolStripComboBox();
			this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
			this.ToolStripButton_Launch = new System.Windows.Forms.ToolStripButton();
			this.toolStripSeparator7 = new System.Windows.Forms.ToolStripSeparator();
			this.ToolStripButton_Editor = new System.Windows.Forms.ToolStripButton();
			this.toolStripSeparator3 = new System.Windows.Forms.ToolStripSeparator();
			this.ToolStripButton_Server = new System.Windows.Forms.ToolStripSplitButton();
			this.ToolStripButton_ServerAndClients = new System.Windows.Forms.ToolStripMenuItem();
			this.ToolStripButton_AddClient = new System.Windows.Forms.ToolStripMenuItem();
			this.ToolStripButton_KillAll = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripSeparator4 = new System.Windows.Forms.ToolStripSeparator();
			this.ToolStripButton_Cook = new System.Windows.Forms.ToolStripButton();
			this.toolStripSeparator5 = new System.Windows.Forms.ToolStripSeparator();
			this.ToolStripButton_CompileScript = new System.Windows.Forms.ToolStripButton();
			this.toolStripSeparator6 = new System.Windows.Forms.ToolStripSeparator();
			this.ToolStripButton_Sync = new System.Windows.Forms.ToolStripButton();
			this.toolStripSeparator8 = new System.Windows.Forms.ToolStripSeparator();
			this.ToolStripButton_Reboot = new System.Windows.Forms.ToolStripButton();
			this.CookMapToolTip = new System.Windows.Forms.ToolTip(this.components);
			((System.ComponentModel.ISupportInitialize)(this.NumericUpDown_Clients)).BeginInit();
			this.Tab_Cooking.SuspendLayout();
			this.CookingGroupBox.SuspendLayout();
			this.Tab_ConsoleSetup.SuspendLayout();
			this.groupBox1.SuspendLayout();
			this.Tab_PCSetup.SuspendLayout();
			this.groupBox6.SuspendLayout();
			this.Tab_Multiplayer.SuspendLayout();
			this.groupBox3.SuspendLayout();
			this.groupBox4.SuspendLayout();
			this.Tab_Local.SuspendLayout();
			this.groupBox5.SuspendLayout();
			this.groupBox2.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.ChapterPointSelect)).BeginInit();
			this.TabControl_Main.SuspendLayout();
			this.Tab_PS3.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.NumericUpDown_NumberOfThreads)).BeginInit();
			this.Tab_Help.SuspendLayout();
			this.statusStrip1.SuspendLayout();
			this.toolStrip1.SuspendLayout();
			this.SuspendLayout();
			// 
			// CheckBox_UseCookedMap
			// 
			this.CheckBox_UseCookedMap.AutoSize = true;
			this.CheckBox_UseCookedMap.ForeColor = System.Drawing.SystemColors.ControlText;
			this.CheckBox_UseCookedMap.Location = new System.Drawing.Point(197, 58);
			this.CheckBox_UseCookedMap.Name = "CheckBox_UseCookedMap";
			this.CheckBox_UseCookedMap.Size = new System.Drawing.Size(113, 17);
			this.CheckBox_UseCookedMap.TabIndex = 19;
			this.CheckBox_UseCookedMap.Text = "(Use cooked map)";
			this.FormToolTips.SetToolTip(this.CheckBox_UseCookedMap, "Uses the \"Maps\" string on the Cooking tab as the map.\nIf cooking multiple maps, t" +
					"he game should use the first map in the list and ignore the others.");
			this.CheckBox_UseCookedMap.CheckedChanged += new System.EventHandler(this.OnUseCookedMapChecked);
			// 
			// FormToolTips
			// 
			this.FormToolTips.AutoPopDelay = 7000;
			this.FormToolTips.InitialDelay = 900;
			this.FormToolTips.ReshowDelay = 100;
			// 
			// TextBox_MapsToCook
			// 
			this.TextBox_MapsToCook.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.TextBox_MapsToCook.Location = new System.Drawing.Point(39, 6);
			this.TextBox_MapsToCook.Name = "TextBox_MapsToCook";
			this.TextBox_MapsToCook.Size = new System.Drawing.Size(286, 20);
			this.TextBox_MapsToCook.TabIndex = 27;
			this.TextBox_MapsToCook.Text = "entry ";
			this.FormToolTips.SetToolTip(this.TextBox_MapsToCook, "List of maps to cook for the given platform and game. Use a space to separate map" +
					"s.\nStreaming sublevels will automatically be cooked, so only specify the persist" +
					"ent level.");
			this.TextBox_MapsToCook.TextChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// Button_StartCooking
			// 
			this.Button_StartCooking.Location = new System.Drawing.Point(9, 239);
			this.Button_StartCooking.Name = "Button_StartCooking";
			this.Button_StartCooking.Size = new System.Drawing.Size(91, 24);
			this.Button_StartCooking.TabIndex = 30;
			this.Button_StartCooking.Text = "Cook";
			this.FormToolTips.SetToolTip(this.Button_StartCooking, resources.GetString("Button_StartCooking.ToolTip"));
			this.Button_StartCooking.Click += new System.EventHandler(this.OnStartCookingClick);
			// 
			// CheckBox_CookFinalReleaseScript
			// 
			this.CheckBox_CookFinalReleaseScript.AutoSize = true;
			this.CheckBox_CookFinalReleaseScript.Location = new System.Drawing.Point(6, 62);
			this.CheckBox_CookFinalReleaseScript.Name = "CheckBox_CookFinalReleaseScript";
			this.CheckBox_CookFinalReleaseScript.Size = new System.Drawing.Size(146, 17);
			this.CheckBox_CookFinalReleaseScript.TabIndex = 3;
			this.CheckBox_CookFinalReleaseScript.Text = "Cook Final Release script";
			this.FormToolTips.SetToolTip(this.CheckBox_CookFinalReleaseScript, "Select this option if you have compiled script (MyGame make -finalrelease) and yo" +
					"u want to use those script packages when cooking [-final_release].");
			this.CheckBox_CookFinalReleaseScript.CheckedChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// CheckBox_CookInisIntsOnly
			// 
			this.CheckBox_CookInisIntsOnly.AutoSize = true;
			this.CheckBox_CookInisIntsOnly.Location = new System.Drawing.Point(6, 39);
			this.CheckBox_CookInisIntsOnly.Name = "CheckBox_CookInisIntsOnly";
			this.CheckBox_CookInisIntsOnly.Size = new System.Drawing.Size(114, 17);
			this.CheckBox_CookInisIntsOnly.TabIndex = 6;
			this.CheckBox_CookInisIntsOnly.Text = "Cook Inis/Ints only";
			this.FormToolTips.SetToolTip(this.CheckBox_CookInisIntsOnly, resources.GetString("CheckBox_CookInisIntsOnly.ToolTip"));
			this.CheckBox_CookInisIntsOnly.CheckedChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// CheckBox_LaunchAfterCooking
			// 
			this.CheckBox_LaunchAfterCooking.AutoSize = true;
			this.CheckBox_LaunchAfterCooking.Location = new System.Drawing.Point(6, 131);
			this.CheckBox_LaunchAfterCooking.Name = "CheckBox_LaunchAfterCooking";
			this.CheckBox_LaunchAfterCooking.Size = new System.Drawing.Size(210, 17);
			this.CheckBox_LaunchAfterCooking.TabIndex = 19;
			this.CheckBox_LaunchAfterCooking.Text = "Run Local game after cooking/syncing";
			this.FormToolTips.SetToolTip(this.CheckBox_LaunchAfterCooking, "After cooking and/or syncing finish, launch the Local (single player) game using " +
					"the options in the Local tab.");
			// 
			// ComboBox_ConsoleConfiguration
			// 
			this.ComboBox_ConsoleConfiguration.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.ComboBox_ConsoleConfiguration.Location = new System.Drawing.Point(9, 24);
			this.ComboBox_ConsoleConfiguration.Name = "ComboBox_ConsoleConfiguration";
			this.ComboBox_ConsoleConfiguration.Size = new System.Drawing.Size(156, 21);
			this.ComboBox_ConsoleConfiguration.TabIndex = 24;
			this.FormToolTips.SetToolTip(this.ComboBox_ConsoleConfiguration, "Select the configuration (debug, release, etc) of the game you want to run on the" +
					" target console (usually Release).");
			this.ComboBox_ConsoleConfiguration.SelectedValueChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// ListBox_Targets
			// 
			this.ListBox_Targets.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.ListBox_Targets.Location = new System.Drawing.Point(9, 98);
			this.ListBox_Targets.Name = "ListBox_Targets";
			this.ListBox_Targets.Size = new System.Drawing.Size(316, 169);
			this.ListBox_Targets.TabIndex = 26;
			this.ListBox_Targets.ThreeDCheckBoxes = true;
			this.FormToolTips.SetToolTip(this.ListBox_Targets, "Select the targets to use for copying, launching and rebooting operations.");
			// 
			// ComboBox_PCConfiguration
			// 
			this.ComboBox_PCConfiguration.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.ComboBox_PCConfiguration.Location = new System.Drawing.Point(9, 24);
			this.ComboBox_PCConfiguration.Name = "ComboBox_PCConfiguration";
			this.ComboBox_PCConfiguration.Size = new System.Drawing.Size(156, 21);
			this.ComboBox_PCConfiguration.TabIndex = 12;
			this.FormToolTips.SetToolTip(this.ComboBox_PCConfiguration, "Select which PC executable configuration (debug or release) you will use for runn" +
					"ing the PC (game, editor, cooker, script compiler, etc). Usually Release.");
			this.ComboBox_PCConfiguration.SelectedValueChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// Button_CopyToTargets
			// 
			this.Button_CopyToTargets.Location = new System.Drawing.Point(7, 18);
			this.Button_CopyToTargets.Name = "Button_CopyToTargets";
			this.Button_CopyToTargets.Size = new System.Drawing.Size(96, 24);
			this.Button_CopyToTargets.TabIndex = 27;
			this.Button_CopyToTargets.Text = "Copy to Targets";
			this.FormToolTips.SetToolTip(this.Button_CopyToTargets, "Synchronizes the files on the selected targets with the cooked files on your PC.");
			this.Button_CopyToTargets.Click += new System.EventHandler(this.OnCopyToTargetsClick);
			// 
			// Button_RebootTargets
			// 
			this.Button_RebootTargets.Location = new System.Drawing.Point(7, 54);
			this.Button_RebootTargets.Name = "Button_RebootTargets";
			this.Button_RebootTargets.Size = new System.Drawing.Size(96, 24);
			this.Button_RebootTargets.TabIndex = 28;
			this.Button_RebootTargets.Text = "Reboot Targets";
			this.FormToolTips.SetToolTip(this.Button_RebootTargets, "Reboot the selected targets.");
			this.Button_RebootTargets.Click += new System.EventHandler(this.OnRebootTargetsClick);
			// 
			// ComboBox_ProjectDir
			// 
			this.ComboBox_ProjectDir.FormattingEnabled = true;
			this.ComboBox_ProjectDir.Location = new System.Drawing.Point(9, 69);
			this.ComboBox_ProjectDir.Name = "ComboBox_ProjectDir";
			this.ComboBox_ProjectDir.Size = new System.Drawing.Size(319, 21);
			this.ComboBox_ProjectDir.TabIndex = 10;
			this.FormToolTips.SetToolTip(this.ComboBox_ProjectDir, "Specify the directory to run the PC executable from. This can be used to manage m" +
					"ultiple builds without running multiple Frontends.");
			this.ComboBox_ProjectDir.Leave += new System.EventHandler(this.OnProjectDirChange);
			// 
			// Button_Compile
			// 
			this.Button_Compile.Location = new System.Drawing.Point(10, 109);
			this.Button_Compile.Name = "Button_Compile";
			this.Button_Compile.Size = new System.Drawing.Size(101, 23);
			this.Button_Compile.TabIndex = 29;
			this.Button_Compile.Text = "Compile Scripts";
			this.FormToolTips.SetToolTip(this.Button_Compile, "Compile script code using the options specified.");
			this.Button_Compile.UseVisualStyleBackColor = true;
			this.Button_Compile.Click += new System.EventHandler(this.OnCompileClick);
			// 
			// ComboBox_URLOptionsSP
			// 
			this.ComboBox_URLOptionsSP.FormattingEnabled = true;
			this.ComboBox_URLOptionsSP.Location = new System.Drawing.Point(94, 136);
			this.ComboBox_URLOptionsSP.Name = "ComboBox_URLOptionsSP";
			this.ComboBox_URLOptionsSP.Size = new System.Drawing.Size(216, 21);
			this.ComboBox_URLOptionsSP.TabIndex = 10;
			this.FormToolTips.SetToolTip(this.ComboBox_URLOptionsSP, "Enter additional URL (game) options (in the form \"?option1?option2=value\").");
			this.ComboBox_URLOptionsSP.Leave += new System.EventHandler(this.OnGenericComboBoxLeave);
			this.ComboBox_URLOptionsSP.TextChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// TextBox_ExecSP
			// 
			this.TextBox_ExecSP.Location = new System.Drawing.Point(193, 44);
			this.TextBox_ExecSP.Multiline = true;
			this.TextBox_ExecSP.Name = "TextBox_ExecSP";
			this.TextBox_ExecSP.Size = new System.Drawing.Size(114, 86);
			this.TextBox_ExecSP.TabIndex = 4;
			this.FormToolTips.SetToolTip(this.TextBox_ExecSP, "Enter commands to be run ingame on the first frame of gameplay [-exec=].");
			// 
			// CheckBox_UnattendedSP
			// 
			this.CheckBox_UnattendedSP.AutoSize = true;
			this.CheckBox_UnattendedSP.Location = new System.Drawing.Point(6, 22);
			this.CheckBox_UnattendedSP.Name = "CheckBox_UnattendedSP";
			this.CheckBox_UnattendedSP.Size = new System.Drawing.Size(82, 17);
			this.CheckBox_UnattendedSP.TabIndex = 6;
			this.CheckBox_UnattendedSP.Text = "Unattended";
			this.FormToolTips.SetToolTip(this.CheckBox_UnattendedSP, "If checked, the game will never display message boxes, and use default answers in" +
					"stead [-unattended]");
			this.CheckBox_UnattendedSP.UseVisualStyleBackColor = true;
			this.CheckBox_UnattendedSP.CheckedChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// ComboBox_ResolutionSP
			// 
			this.ComboBox_ResolutionSP.FormattingEnabled = true;
			this.ComboBox_ResolutionSP.Location = new System.Drawing.Point(94, 17);
			this.ComboBox_ResolutionSP.Name = "ComboBox_ResolutionSP";
			this.ComboBox_ResolutionSP.Size = new System.Drawing.Size(88, 21);
			this.ComboBox_ResolutionSP.TabIndex = 1;
			this.FormToolTips.SetToolTip(this.ComboBox_ResolutionSP, "Choose the resolution to run at for the PC game.");
			this.ComboBox_ResolutionSP.TextChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// CheckBox_FixedSeedSP
			// 
			this.CheckBox_FixedSeedSP.AutoSize = true;
			this.CheckBox_FixedSeedSP.Location = new System.Drawing.Point(6, 113);
			this.CheckBox_FixedSeedSP.Name = "CheckBox_FixedSeedSP";
			this.CheckBox_FixedSeedSP.Size = new System.Drawing.Size(79, 17);
			this.CheckBox_FixedSeedSP.TabIndex = 8;
			this.CheckBox_FixedSeedSP.Text = "Fixed Seed";
			this.FormToolTips.SetToolTip(this.CheckBox_FixedSeedSP, "Will set the random number generator to a fixed seed, reducing randomness between" +
					" runs [-fixedseed].");
			this.CheckBox_FixedSeedSP.UseVisualStyleBackColor = true;
			this.CheckBox_FixedSeedSP.CheckedChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// CheckBox_VSyncSP
			// 
			this.CheckBox_VSyncSP.AutoSize = true;
			this.CheckBox_VSyncSP.Location = new System.Drawing.Point(94, 90);
			this.CheckBox_VSyncSP.Name = "CheckBox_VSyncSP";
			this.CheckBox_VSyncSP.Size = new System.Drawing.Size(57, 17);
			this.CheckBox_VSyncSP.TabIndex = 7;
			this.CheckBox_VSyncSP.Text = "VSync";
			this.FormToolTips.SetToolTip(this.CheckBox_VSyncSP, "Controls whether or not VSyncing is disabled (no VSync will have a smoother frame" +
					"rate but the screen will tear) [-novsync].");
			this.CheckBox_VSyncSP.UseVisualStyleBackColor = true;
			this.CheckBox_VSyncSP.CheckedChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// CheckBox_RCSP
			// 
			this.CheckBox_RCSP.AutoSize = true;
			this.CheckBox_RCSP.Location = new System.Drawing.Point(94, 113);
			this.CheckBox_RCSP.Name = "CheckBox_RCSP";
			this.CheckBox_RCSP.Size = new System.Drawing.Size(99, 17);
			this.CheckBox_RCSP.TabIndex = 5;
			this.CheckBox_RCSP.Text = "Remote Control";
			this.FormToolTips.SetToolTip(this.CheckBox_RCSP, "On PC, controls whether or not the Remote Control side window is displayed [-norc" +
					"].");
			this.CheckBox_RCSP.UseVisualStyleBackColor = true;
			this.CheckBox_RCSP.CheckedChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// CheckBox_PostProcessSP
			// 
			this.CheckBox_PostProcessSP.AutoSize = true;
			this.CheckBox_PostProcessSP.Location = new System.Drawing.Point(94, 44);
			this.CheckBox_PostProcessSP.Name = "CheckBox_PostProcessSP";
			this.CheckBox_PostProcessSP.Size = new System.Drawing.Size(88, 17);
			this.CheckBox_PostProcessSP.TabIndex = 4;
			this.CheckBox_PostProcessSP.Text = "Post Process";
			this.FormToolTips.SetToolTip(this.CheckBox_PostProcessSP, "Controls whether or not postprocessing is disabled [show postprocess].");
			this.CheckBox_PostProcessSP.UseVisualStyleBackColor = true;
			this.CheckBox_PostProcessSP.CheckedChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// CheckBox_UnlitSP
			// 
			this.CheckBox_UnlitSP.AutoSize = true;
			this.CheckBox_UnlitSP.Location = new System.Drawing.Point(94, 67);
			this.CheckBox_UnlitSP.Name = "CheckBox_UnlitSP";
			this.CheckBox_UnlitSP.Size = new System.Drawing.Size(47, 17);
			this.CheckBox_UnlitSP.TabIndex = 4;
			this.CheckBox_UnlitSP.Text = "Unlit";
			this.FormToolTips.SetToolTip(this.CheckBox_UnlitSP, "Controls whehter or not the game starts up in unlit mode [viewmode unlit].");
			this.CheckBox_UnlitSP.UseVisualStyleBackColor = true;
			this.CheckBox_UnlitSP.CheckedChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// CheckBox_MultiThreadSP
			// 
			this.CheckBox_MultiThreadSP.AutoSize = true;
			this.CheckBox_MultiThreadSP.Location = new System.Drawing.Point(6, 90);
			this.CheckBox_MultiThreadSP.Name = "CheckBox_MultiThreadSP";
			this.CheckBox_MultiThreadSP.Size = new System.Drawing.Size(81, 17);
			this.CheckBox_MultiThreadSP.TabIndex = 3;
			this.CheckBox_MultiThreadSP.Text = "Multi-thread";
			this.FormToolTips.SetToolTip(this.CheckBox_MultiThreadSP, "Controls whether or not the rendering thread is disabled [-onethread].");
			this.CheckBox_MultiThreadSP.UseVisualStyleBackColor = true;
			this.CheckBox_MultiThreadSP.CheckedChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// CheckBox_ShowLogSP
			// 
			this.CheckBox_ShowLogSP.AutoSize = true;
			this.CheckBox_ShowLogSP.Location = new System.Drawing.Point(6, 44);
			this.CheckBox_ShowLogSP.Name = "CheckBox_ShowLogSP";
			this.CheckBox_ShowLogSP.Size = new System.Drawing.Size(74, 17);
			this.CheckBox_ShowLogSP.TabIndex = 0;
			this.CheckBox_ShowLogSP.Text = "Show Log";
			this.FormToolTips.SetToolTip(this.CheckBox_ShowLogSP, "On PC, this will open a log window when the game is launched [-log].");
			this.CheckBox_ShowLogSP.UseVisualStyleBackColor = true;
			this.CheckBox_ShowLogSP.CheckedChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// CheckBox_SoundSP
			// 
			this.CheckBox_SoundSP.AutoSize = true;
			this.CheckBox_SoundSP.Location = new System.Drawing.Point(6, 67);
			this.CheckBox_SoundSP.Name = "CheckBox_SoundSP";
			this.CheckBox_SoundSP.Size = new System.Drawing.Size(57, 17);
			this.CheckBox_SoundSP.TabIndex = 2;
			this.CheckBox_SoundSP.Text = "Sound";
			this.FormToolTips.SetToolTip(this.CheckBox_SoundSP, "Controls whether or not sound playback is disabled [-nosound].");
			this.CheckBox_SoundSP.UseVisualStyleBackColor = true;
			this.CheckBox_SoundSP.CheckedChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// Button_Editor
			// 
			this.Button_Editor.Location = new System.Drawing.Point(7, 54);
			this.Button_Editor.Name = "Button_Editor";
			this.Button_Editor.Size = new System.Drawing.Size(74, 23);
			this.Button_Editor.TabIndex = 26;
			this.Button_Editor.Text = "Edit Map";
			this.FormToolTips.SetToolTip(this.Button_Editor, "Run the PC editor on the selected map.");
			this.Button_Editor.UseVisualStyleBackColor = true;
			this.Button_Editor.Click += new System.EventHandler(this.OnEditorClick);
			// 
			// Button_BrowseSP
			// 
			this.Button_BrowseSP.Location = new System.Drawing.Point(235, 19);
			this.Button_BrowseSP.Name = "Button_BrowseSP";
			this.Button_BrowseSP.Size = new System.Drawing.Size(75, 23);
			this.Button_BrowseSP.TabIndex = 7;
			this.Button_BrowseSP.Text = "Browse...";
			this.FormToolTips.SetToolTip(this.Button_BrowseSP, "Browse for a map to load or edit.");
			this.Button_BrowseSP.UseVisualStyleBackColor = true;
			this.Button_BrowseSP.Click += new System.EventHandler(this.OnBrowseSPClick);
			// 
			// ComboBox_MapnameSP
			// 
			this.ComboBox_MapnameSP.FormattingEnabled = true;
			this.ComboBox_MapnameSP.Location = new System.Drawing.Point(87, 20);
			this.ComboBox_MapnameSP.Name = "ComboBox_MapnameSP";
			this.ComboBox_MapnameSP.Size = new System.Drawing.Size(142, 21);
			this.ComboBox_MapnameSP.TabIndex = 2;
			this.FormToolTips.SetToolTip(this.ComboBox_MapnameSP, "Sets the map name for loading or editing. Use drop down for previously used maps." +
					"");
			this.ComboBox_MapnameSP.Leave += new System.EventHandler(this.OnGenericComboBoxLeave);
			this.ComboBox_MapnameSP.TextChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// Button_LoadMap
			// 
			this.Button_LoadMap.Location = new System.Drawing.Point(6, 19);
			this.Button_LoadMap.Name = "Button_LoadMap";
			this.Button_LoadMap.Size = new System.Drawing.Size(75, 23);
			this.Button_LoadMap.TabIndex = 0;
			this.Button_LoadMap.Tag = "1";
			this.Button_LoadMap.Text = "Load Map";
			this.FormToolTips.SetToolTip(this.Button_LoadMap, "Launch the selected map (or run default if no map given) on the target platform (" +
					"including multiple targets if using a console).");
			this.Button_LoadMap.UseVisualStyleBackColor = true;
			this.Button_LoadMap.Click += new System.EventHandler(this.OnLoadMapClick);
			// 
			// ComboBox_BaseDirectory
			// 
			this.ComboBox_BaseDirectory.Location = new System.Drawing.Point(9, 69);
			this.ComboBox_BaseDirectory.Name = "ComboBox_BaseDirectory";
			this.ComboBox_BaseDirectory.Size = new System.Drawing.Size(156, 21);
			this.ComboBox_BaseDirectory.TabIndex = 24;
			this.FormToolTips.SetToolTip(this.ComboBox_BaseDirectory, "Selects the location to use on the console hard drive to store files. Use this to" +
					" have multiple builds or multiple users share a devkit. All spaces will be remov" +
					"ed.");
			// 
			// CheckBox_RunWithCookedData
			// 
			this.CheckBox_RunWithCookedData.AutoSize = true;
			this.CheckBox_RunWithCookedData.Location = new System.Drawing.Point(9, 100);
			this.CheckBox_RunWithCookedData.Name = "CheckBox_RunWithCookedData";
			this.CheckBox_RunWithCookedData.Size = new System.Drawing.Size(134, 17);
			this.CheckBox_RunWithCookedData.TabIndex = 33;
			this.CheckBox_RunWithCookedData.Text = "Run with Cooked Data";
			this.FormToolTips.SetToolTip(this.CheckBox_RunWithCookedData, "Controls whether or not PC games will run with cooked data or the original packag" +
					"es [-seekfreeloading].");
			this.CheckBox_RunWithCookedData.UseVisualStyleBackColor = true;
			// 
			// Button_BrowseMP
			// 
			this.Button_BrowseMP.Location = new System.Drawing.Point(235, 19);
			this.Button_BrowseMP.Name = "Button_BrowseMP";
			this.Button_BrowseMP.Size = new System.Drawing.Size(75, 23);
			this.Button_BrowseMP.TabIndex = 9;
			this.Button_BrowseMP.Text = "Browse...";
			this.FormToolTips.SetToolTip(this.Button_BrowseMP, "Browse for a map for the server to load.");
			this.Button_BrowseMP.UseVisualStyleBackColor = true;
			this.Button_BrowseMP.Click += new System.EventHandler(this.OnBrowseMPClick);
			// 
			// Button_KillAll
			// 
			this.Button_KillAll.Location = new System.Drawing.Point(235, 54);
			this.Button_KillAll.Name = "Button_KillAll";
			this.Button_KillAll.Size = new System.Drawing.Size(75, 23);
			this.Button_KillAll.TabIndex = 8;
			this.Button_KillAll.Text = "Kill All";
			this.FormToolTips.SetToolTip(this.Button_KillAll, "[PC only] Kills all the server and any clients launched after the server was laun" +
					"ched.");
			this.Button_KillAll.UseVisualStyleBackColor = true;
			this.Button_KillAll.Click += new System.EventHandler(this.OnKillAllClick);
			// 
			// ComboBox_MapnameMP
			// 
			this.ComboBox_MapnameMP.FormattingEnabled = true;
			this.ComboBox_MapnameMP.Location = new System.Drawing.Point(87, 20);
			this.ComboBox_MapnameMP.Name = "ComboBox_MapnameMP";
			this.ComboBox_MapnameMP.Size = new System.Drawing.Size(142, 21);
			this.ComboBox_MapnameMP.TabIndex = 3;
			this.FormToolTips.SetToolTip(this.ComboBox_MapnameMP, "Select the map for the server to use.");
			this.ComboBox_MapnameMP.Leave += new System.EventHandler(this.OnGenericComboBoxLeave);
			this.ComboBox_MapnameMP.TextChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// NumericUpDown_Clients
			// 
			this.NumericUpDown_Clients.Location = new System.Drawing.Point(195, 56);
			this.NumericUpDown_Clients.Maximum = new decimal(new int[] {
            4,
            0,
            0,
            0});
			this.NumericUpDown_Clients.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.NumericUpDown_Clients.Name = "NumericUpDown_Clients";
			this.NumericUpDown_Clients.Size = new System.Drawing.Size(35, 20);
			this.NumericUpDown_Clients.TabIndex = 3;
			this.FormToolTips.SetToolTip(this.NumericUpDown_Clients, "[PC only] Select the number of clients to run when using the Server + Client(s) b" +
					"utton");
			this.NumericUpDown_Clients.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.NumericUpDown_Clients.ValueChanged += new System.EventHandler(this.NumericUpDown_Clients_ValueChanged);
			// 
			// Button_ServerClient
			// 
			this.Button_ServerClient.Location = new System.Drawing.Point(87, 54);
			this.Button_ServerClient.Name = "Button_ServerClient";
			this.Button_ServerClient.Size = new System.Drawing.Size(106, 23);
			this.Button_ServerClient.TabIndex = 2;
			this.Button_ServerClient.Text = "Server + 1 Client";
			this.FormToolTips.SetToolTip(this.Button_ServerClient, "[PC only] Start a server and the number of clients specified to the right.");
			this.Button_ServerClient.UseVisualStyleBackColor = true;
			this.Button_ServerClient.Click += new System.EventHandler(this.OnServerClientClick);
			// 
			// Button_AddClient
			// 
			this.Button_AddClient.Location = new System.Drawing.Point(6, 54);
			this.Button_AddClient.Name = "Button_AddClient";
			this.Button_AddClient.Size = new System.Drawing.Size(75, 23);
			this.Button_AddClient.TabIndex = 1;
			this.Button_AddClient.Text = "Add Client";
			this.FormToolTips.SetToolTip(this.Button_AddClient, "[PC only] Launch a client to connect to the server running on this PC.");
			this.Button_AddClient.UseVisualStyleBackColor = true;
			this.Button_AddClient.Click += new System.EventHandler(this.OnClientClick);
			// 
			// Button_Server
			// 
			this.Button_Server.Location = new System.Drawing.Point(6, 19);
			this.Button_Server.Name = "Button_Server";
			this.Button_Server.Size = new System.Drawing.Size(75, 23);
			this.Button_Server.TabIndex = 0;
			this.Button_Server.Text = "Server";
			this.FormToolTips.SetToolTip(this.Button_Server, "Launch a server using the selected map. Choose List or Dedicated in the Options s" +
					"ection below to choose the type of server.");
			this.Button_Server.UseVisualStyleBackColor = true;
			this.Button_Server.Click += new System.EventHandler(this.OnServerClick);
			// 
			// ComboBox_CookingLanguage
			// 
			this.ComboBox_CookingLanguage.FormattingEnabled = true;
			this.ComboBox_CookingLanguage.Location = new System.Drawing.Point(250, 14);
			this.ComboBox_CookingLanguage.Name = "ComboBox_CookingLanguage";
			this.ComboBox_CookingLanguage.Size = new System.Drawing.Size(60, 21);
			this.ComboBox_CookingLanguage.TabIndex = 20;
			this.FormToolTips.SetToolTip(this.ComboBox_CookingLanguage, "Select the language to cook for. Blank or \"int\" will cook for English.");
			this.ComboBox_CookingLanguage.Leave += new System.EventHandler(this.OnGenericComboBoxLeave);
			this.ComboBox_CookingLanguage.TextChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// CheckBox_FullCook
			// 
			this.CheckBox_FullCook.AutoSize = true;
			this.CheckBox_FullCook.Location = new System.Drawing.Point(6, 85);
			this.CheckBox_FullCook.Name = "CheckBox_FullCook";
			this.CheckBox_FullCook.Size = new System.Drawing.Size(78, 17);
			this.CheckBox_FullCook.TabIndex = 19;
			this.CheckBox_FullCook.Text = "Full recook";
			this.FormToolTips.SetToolTip(this.CheckBox_FullCook, "If checked, this will do a full recook of content. May be slow! [-full]");
			this.CheckBox_FullCook.CheckedChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// ComboBox_ScriptConfig
			// 
			this.ComboBox_ScriptConfig.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.ComboBox_ScriptConfig.Location = new System.Drawing.Point(10, 36);
			this.ComboBox_ScriptConfig.Name = "ComboBox_ScriptConfig";
			this.ComboBox_ScriptConfig.Size = new System.Drawing.Size(149, 21);
			this.ComboBox_ScriptConfig.TabIndex = 33;
			this.FormToolTips.SetToolTip(this.ComboBox_ScriptConfig, "Select the script code configuration to compile (Debug, Release, Final Release). " +
					"Note that Final Release will output to a separate directory (usually ScriptFinal" +
					"Release).");
			this.ComboBox_ScriptConfig.SelectedIndexChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// CheckBox_AutomaticCompileMode
			// 
			this.CheckBox_AutomaticCompileMode.AutoSize = true;
			this.CheckBox_AutomaticCompileMode.Location = new System.Drawing.Point(10, 86);
			this.CheckBox_AutomaticCompileMode.Name = "CheckBox_AutomaticCompileMode";
			this.CheckBox_AutomaticCompileMode.Size = new System.Drawing.Size(103, 17);
			this.CheckBox_AutomaticCompileMode.TabIndex = 32;
			this.CheckBox_AutomaticCompileMode.Text = "Automatic Mode";
			this.FormToolTips.SetToolTip(this.CheckBox_AutomaticCompileMode, "Compile script in fully automatic mode (no prompts, auto .h updating, etc) [-auto" +
					" -unattended -updateinisauto]");
			this.CheckBox_AutomaticCompileMode.UseVisualStyleBackColor = true;
			this.CheckBox_AutomaticCompileMode.CheckedChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// CheckBox_FullScriptCompile
			// 
			this.CheckBox_FullScriptCompile.AutoSize = true;
			this.CheckBox_FullScriptCompile.Location = new System.Drawing.Point(10, 63);
			this.CheckBox_FullScriptCompile.Name = "CheckBox_FullScriptCompile";
			this.CheckBox_FullScriptCompile.Size = new System.Drawing.Size(95, 17);
			this.CheckBox_FullScriptCompile.TabIndex = 32;
			this.CheckBox_FullScriptCompile.Text = "Full Recompile";
			this.FormToolTips.SetToolTip(this.CheckBox_FullScriptCompile, "Check this to recompile all .u packages, instead just the out-of-date packages [-" +
					"full]");
			this.CheckBox_FullScriptCompile.UseVisualStyleBackColor = true;
			this.CheckBox_FullScriptCompile.CheckedChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// CheckBox_CookInUserMode
			// 
			this.CheckBox_CookInUserMode.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.CheckBox_CookInUserMode.AutoSize = true;
			this.CheckBox_CookInUserMode.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.CheckBox_CookInUserMode.ForeColor = System.Drawing.SystemColors.ControlText;
			this.CheckBox_CookInUserMode.Location = new System.Drawing.Point(250, 252);
			this.CheckBox_CookInUserMode.Name = "CheckBox_CookInUserMode";
			this.CheckBox_CookInUserMode.RightToLeft = System.Windows.Forms.RightToLeft.Yes;
			this.CheckBox_CookInUserMode.Size = new System.Drawing.Size(75, 17);
			this.CheckBox_CookInUserMode.TabIndex = 35;
			this.CheckBox_CookInUserMode.Text = "Cook Mod";
			this.FormToolTips.SetToolTip(this.CheckBox_CookInUserMode, "Controls whether or not the cooker will use seek free data. Required for mods.");
			this.CheckBox_CookInUserMode.UseVisualStyleBackColor = true;
			this.CheckBox_CookInUserMode.CheckedChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// CheckBox_ForceCookSeekfree
			// 
			this.CheckBox_ForceCookSeekfree.AutoSize = true;
			this.CheckBox_ForceCookSeekfree.Location = new System.Drawing.Point(6, 19);
			this.CheckBox_ForceCookSeekfree.Name = "CheckBox_ForceCookSeekfree";
			this.CheckBox_ForceCookSeekfree.Size = new System.Drawing.Size(133, 17);
			this.CheckBox_ForceCookSeekfree.TabIndex = 30;
			this.CheckBox_ForceCookSeekfree.Text = "Force recook seekfree";
			this.FormToolTips.SetToolTip(this.CheckBox_ForceCookSeekfree, resources.GetString("CheckBox_ForceCookSeekfree.ToolTip"));
			this.CheckBox_ForceCookSeekfree.CheckedChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// CheckBox_CookAllMaps
			// 
			this.CheckBox_CookAllMaps.AutoSize = true;
			this.CheckBox_CookAllMaps.Location = new System.Drawing.Point(6, 108);
			this.CheckBox_CookAllMaps.Name = "CheckBox_CookAllMaps";
			this.CheckBox_CookAllMaps.Size = new System.Drawing.Size(92, 17);
			this.CheckBox_CookAllMaps.TabIndex = 31;
			this.CheckBox_CookAllMaps.Text = "Cook all maps";
			this.FormToolTips.SetToolTip(this.CheckBox_CookAllMaps, "This will cook all maps that are found [-cookallmaps]");
			this.CheckBox_CookAllMaps.CheckedChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// CheckBox_QuietMode
			// 
			this.CheckBox_QuietMode.AutoSize = true;
			this.CheckBox_QuietMode.Location = new System.Drawing.Point(112, 62);
			this.CheckBox_QuietMode.Name = "CheckBox_QuietMode";
			this.CheckBox_QuietMode.Size = new System.Drawing.Size(81, 17);
			this.CheckBox_QuietMode.TabIndex = 34;
			this.CheckBox_QuietMode.Text = "Quiet Mode";
			this.FormToolTips.SetToolTip(this.CheckBox_QuietMode, "Check this to recompile all .u packages, instead just the out-of-date packages [-" +
					"full]");
			this.CheckBox_QuietMode.UseVisualStyleBackColor = true;
			// 
			// RichTextBox_LogWindow
			// 
			this.RichTextBox_LogWindow.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.RichTextBox_LogWindow.BackColor = System.Drawing.SystemColors.Window;
			this.RichTextBox_LogWindow.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.RichTextBox_LogWindow.Location = new System.Drawing.Point(360, 50);
			this.RichTextBox_LogWindow.Name = "RichTextBox_LogWindow";
			this.RichTextBox_LogWindow.ReadOnly = true;
			this.RichTextBox_LogWindow.Size = new System.Drawing.Size(351, 280);
			this.RichTextBox_LogWindow.TabIndex = 27;
			this.RichTextBox_LogWindow.Text = "";
			this.RichTextBox_LogWindow.WordWrap = false;
			// 
			// Tab_Cooking
			// 
			this.Tab_Cooking.Controls.Add(this.CheckBox_CookInUserMode);
			this.Tab_Cooking.Controls.Add(this.CookingGroupBox);
			this.Tab_Cooking.Controls.Add(this.Button_StartCooking);
			this.Tab_Cooking.Controls.Add(this.Button_ImportMapList);
			this.Tab_Cooking.Controls.Add(this.TextBox_MapsToCook);
			this.Tab_Cooking.Controls.Add(this.label3);
			this.Tab_Cooking.Location = new System.Drawing.Point(4, 22);
			this.Tab_Cooking.Name = "Tab_Cooking";
			this.Tab_Cooking.Padding = new System.Windows.Forms.Padding(3);
			this.Tab_Cooking.Size = new System.Drawing.Size(331, 272);
			this.Tab_Cooking.TabIndex = 4;
			this.Tab_Cooking.Text = "Cooking";
			this.Tab_Cooking.ToolTipText = "This controls cooking content for the selected platform.";
			this.Tab_Cooking.UseVisualStyleBackColor = true;
			// 
			// CookingGroupBox
			// 
			this.CookingGroupBox.Controls.Add(this.CheckBox_CookAllMaps);
			this.CookingGroupBox.Controls.Add(this.CheckBox_ForceCookSeekfree);
			this.CookingGroupBox.Controls.Add(this.ComboBox_CookingLanguage);
			this.CookingGroupBox.Controls.Add(this.CheckBox_FullCook);
			this.CookingGroupBox.Controls.Add(this.CheckBox_LaunchAfterCooking);
			this.CookingGroupBox.Controls.Add(this.CheckBox_CookInisIntsOnly);
			this.CookingGroupBox.Controls.Add(this.CheckBox_CookFinalReleaseScript);
			this.CookingGroupBox.Controls.Add(this.MapsToCookLabel);
			this.CookingGroupBox.Location = new System.Drawing.Point(9, 72);
			this.CookingGroupBox.Name = "CookingGroupBox";
			this.CookingGroupBox.Size = new System.Drawing.Size(316, 155);
			this.CookingGroupBox.TabIndex = 31;
			this.CookingGroupBox.TabStop = false;
			this.CookingGroupBox.Text = "Options";
			// 
			// MapsToCookLabel
			// 
			this.MapsToCookLabel.AutoSize = true;
			this.MapsToCookLabel.ForeColor = System.Drawing.SystemColors.ControlText;
			this.MapsToCookLabel.Location = new System.Drawing.Point(186, 17);
			this.MapsToCookLabel.Name = "MapsToCookLabel";
			this.MapsToCookLabel.Size = new System.Drawing.Size(58, 13);
			this.MapsToCookLabel.TabIndex = 28;
			this.MapsToCookLabel.Text = "Language:";
			// 
			// Button_ImportMapList
			// 
			this.Button_ImportMapList.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.Button_ImportMapList.Location = new System.Drawing.Point(9, 32);
			this.Button_ImportMapList.Name = "Button_ImportMapList";
			this.Button_ImportMapList.Size = new System.Drawing.Size(91, 23);
			this.Button_ImportMapList.TabIndex = 29;
			this.Button_ImportMapList.Text = "Import Map List";
			this.Button_ImportMapList.UseVisualStyleBackColor = true;
			this.Button_ImportMapList.Click += new System.EventHandler(this.OnImportMapListClick);
			// 
			// label3
			// 
			this.label3.AutoSize = true;
			this.label3.Location = new System.Drawing.Point(5, 9);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(36, 13);
			this.label3.TabIndex = 32;
			this.label3.Text = "Maps:";
			// 
			// Tab_ConsoleSetup
			// 
			this.Tab_ConsoleSetup.Controls.Add(this.groupBox1);
			this.Tab_ConsoleSetup.Controls.Add(this.ListBox_Targets);
			this.Tab_ConsoleSetup.Controls.Add(this.label2);
			this.Tab_ConsoleSetup.Controls.Add(this.ComboBox_BaseDirectory);
			this.Tab_ConsoleSetup.Controls.Add(this.ComboBox_ConsoleConfiguration);
			this.Tab_ConsoleSetup.Controls.Add(this.ConsoleBaseLabel);
			this.Tab_ConsoleSetup.Location = new System.Drawing.Point(4, 22);
			this.Tab_ConsoleSetup.Name = "Tab_ConsoleSetup";
			this.Tab_ConsoleSetup.Padding = new System.Windows.Forms.Padding(3);
			this.Tab_ConsoleSetup.Size = new System.Drawing.Size(331, 272);
			this.Tab_ConsoleSetup.TabIndex = 3;
			this.Tab_ConsoleSetup.Text = "Console";
			this.Tab_ConsoleSetup.ToolTipText = "This tab has settings for console platforms, as well as some utilities like Reboo" +
				"t and Copy data to target.";
			this.Tab_ConsoleSetup.UseVisualStyleBackColor = true;
			// 
			// groupBox1
			// 
			this.groupBox1.Controls.Add(this.Button_RebootTargets);
			this.groupBox1.Controls.Add(this.Button_CopyToTargets);
			this.groupBox1.Location = new System.Drawing.Point(215, 7);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(110, 85);
			this.groupBox1.TabIndex = 29;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "Tools";
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(6, 6);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(113, 13);
			this.label2.TabIndex = 25;
			this.label2.Text = "Console Configuration:";
			// 
			// ConsoleBaseLabel
			// 
			this.ConsoleBaseLabel.Location = new System.Drawing.Point(6, 53);
			this.ConsoleBaseLabel.Name = "ConsoleBaseLabel";
			this.ConsoleBaseLabel.Size = new System.Drawing.Size(136, 16);
			this.ConsoleBaseLabel.TabIndex = 23;
			this.ConsoleBaseLabel.Text = "Console Base Directory:";
			// 
			// Tab_PCSetup
			// 
			this.Tab_PCSetup.Controls.Add(this.CheckBox_RunWithCookedData);
			this.Tab_PCSetup.Controls.Add(this.groupBox6);
			this.Tab_PCSetup.Controls.Add(this.label1);
			this.Tab_PCSetup.Controls.Add(this.ComboBox_PCConfiguration);
			this.Tab_PCSetup.Controls.Add(this.ComboBox_ProjectDir);
			this.Tab_PCSetup.Controls.Add(this.label8);
			this.Tab_PCSetup.Location = new System.Drawing.Point(4, 22);
			this.Tab_PCSetup.Name = "Tab_PCSetup";
			this.Tab_PCSetup.Padding = new System.Windows.Forms.Padding(3);
			this.Tab_PCSetup.Size = new System.Drawing.Size(331, 272);
			this.Tab_PCSetup.TabIndex = 2;
			this.Tab_PCSetup.Text = "PC";
			this.Tab_PCSetup.ToolTipText = "This tab contains PC-based settings as well the ability to compile script code.";
			this.Tab_PCSetup.UseVisualStyleBackColor = true;
			// 
			// groupBox6
			// 
			this.groupBox6.Controls.Add(this.CheckBox_QuietMode);
			this.groupBox6.Controls.Add(this.ComboBox_ScriptConfig);
			this.groupBox6.Controls.Add(this.CheckBox_AutomaticCompileMode);
			this.groupBox6.Controls.Add(this.CheckBox_FullScriptCompile);
			this.groupBox6.Controls.Add(this.label10);
			this.groupBox6.Controls.Add(this.Button_Compile);
			this.groupBox6.Location = new System.Drawing.Point(6, 124);
			this.groupBox6.Name = "groupBox6";
			this.groupBox6.Size = new System.Drawing.Size(318, 141);
			this.groupBox6.TabIndex = 29;
			this.groupBox6.TabStop = false;
			this.groupBox6.Text = "Script Compiling";
			// 
			// label10
			// 
			this.label10.AutoSize = true;
			this.label10.Location = new System.Drawing.Point(7, 20);
			this.label10.Name = "label10";
			this.label10.Size = new System.Drawing.Size(110, 13);
			this.label10.TabIndex = 30;
			this.label10.Text = "Script Compile Config:";
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(6, 6);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(112, 16);
			this.label1.TabIndex = 13;
			this.label1.Text = "PC Configuration:";
			// 
			// label8
			// 
			this.label8.AutoSize = true;
			this.label8.Location = new System.Drawing.Point(6, 53);
			this.label8.Name = "label8";
			this.label8.Size = new System.Drawing.Size(117, 13);
			this.label8.TabIndex = 9;
			this.label8.Text = "Directory To Run From:";
			// 
			// Tab_Multiplayer
			// 
			this.Tab_Multiplayer.Controls.Add(this.groupBox3);
			this.Tab_Multiplayer.Controls.Add(this.groupBox4);
			this.Tab_Multiplayer.Location = new System.Drawing.Point(4, 22);
			this.Tab_Multiplayer.Name = "Tab_Multiplayer";
			this.Tab_Multiplayer.Padding = new System.Windows.Forms.Padding(3);
			this.Tab_Multiplayer.Size = new System.Drawing.Size(331, 272);
			this.Tab_Multiplayer.TabIndex = 1;
			this.Tab_Multiplayer.Text = "MP";
			this.Tab_Multiplayer.ToolTipText = "This tab allows starting a server on the selected platform, as well as adding mul" +
				"tiple clients for PC games.";
			this.Tab_Multiplayer.UseVisualStyleBackColor = true;
			// 
			// groupBox3
			// 
			this.groupBox3.Controls.Add(this.label9);
			this.groupBox3.Controls.Add(this.ComboBox_URLOptionsMP);
			this.groupBox3.Controls.Add(this.ListBox_ServerType);
			this.groupBox3.Controls.Add(this.TextBox_ExecMP);
			this.groupBox3.Controls.Add(this.label6);
			this.groupBox3.Controls.Add(this.CheckBox_UnattendedMP);
			this.groupBox3.Controls.Add(this.ComboBox_ResolutionMP);
			this.groupBox3.Controls.Add(this.CheckBox_FixedSeedMP);
			this.groupBox3.Controls.Add(this.CheckBox_VSyncMP);
			this.groupBox3.Controls.Add(this.CheckBox_RCMP);
			this.groupBox3.Controls.Add(this.CheckBox_PostProcessMP);
			this.groupBox3.Controls.Add(this.CheckBox_UnlitMP);
			this.groupBox3.Controls.Add(this.CheckBox_MultiThreadMP);
			this.groupBox3.Controls.Add(this.CheckBox_ShowLogMP);
			this.groupBox3.Controls.Add(this.CheckBox_SoundMP);
			this.groupBox3.Location = new System.Drawing.Point(6, 98);
			this.groupBox3.Name = "groupBox3";
			this.groupBox3.Size = new System.Drawing.Size(316, 167);
			this.groupBox3.TabIndex = 1;
			this.groupBox3.TabStop = false;
			this.groupBox3.Text = "Options";
			// 
			// label9
			// 
			this.label9.AutoSize = true;
			this.label9.Location = new System.Drawing.Point(6, 139);
			this.label9.Name = "label9";
			this.label9.Size = new System.Drawing.Size(73, 13);
			this.label9.TabIndex = 12;
			this.label9.Text = "Extra Options:";
			// 
			// ComboBox_URLOptionsMP
			// 
			this.ComboBox_URLOptionsMP.FormattingEnabled = true;
			this.ComboBox_URLOptionsMP.Location = new System.Drawing.Point(94, 136);
			this.ComboBox_URLOptionsMP.Name = "ComboBox_URLOptionsMP";
			this.ComboBox_URLOptionsMP.Size = new System.Drawing.Size(216, 21);
			this.ComboBox_URLOptionsMP.TabIndex = 11;
			this.ComboBox_URLOptionsMP.Leave += new System.EventHandler(this.OnGenericComboBoxLeave);
			this.ComboBox_URLOptionsMP.TextChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// ListBox_ServerType
			// 
			this.ListBox_ServerType.FormattingEnabled = true;
			this.ListBox_ServerType.Items.AddRange(new object[] {
            "Listen",
            "Dedicated"});
			this.ListBox_ServerType.Location = new System.Drawing.Point(193, 19);
			this.ListBox_ServerType.Name = "ListBox_ServerType";
			this.ListBox_ServerType.ScrollAlwaysVisible = true;
			this.ListBox_ServerType.Size = new System.Drawing.Size(114, 17);
			this.ListBox_ServerType.TabIndex = 10;
			this.ListBox_ServerType.SelectedValueChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// TextBox_ExecMP
			// 
			this.TextBox_ExecMP.Location = new System.Drawing.Point(193, 67);
			this.TextBox_ExecMP.Multiline = true;
			this.TextBox_ExecMP.Name = "TextBox_ExecMP";
			this.TextBox_ExecMP.Size = new System.Drawing.Size(114, 63);
			this.TextBox_ExecMP.TabIndex = 4;
			// 
			// label6
			// 
			this.label6.AutoSize = true;
			this.label6.Location = new System.Drawing.Point(190, 44);
			this.label6.Name = "label6";
			this.label6.Size = new System.Drawing.Size(63, 13);
			this.label6.TabIndex = 9;
			this.label6.Text = "Exec Cmds:";
			// 
			// CheckBox_UnattendedMP
			// 
			this.CheckBox_UnattendedMP.AutoSize = true;
			this.CheckBox_UnattendedMP.Location = new System.Drawing.Point(6, 22);
			this.CheckBox_UnattendedMP.Name = "CheckBox_UnattendedMP";
			this.CheckBox_UnattendedMP.Size = new System.Drawing.Size(82, 17);
			this.CheckBox_UnattendedMP.TabIndex = 6;
			this.CheckBox_UnattendedMP.Text = "Unattended";
			this.CheckBox_UnattendedMP.UseVisualStyleBackColor = true;
			this.CheckBox_UnattendedMP.CheckedChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// ComboBox_ResolutionMP
			// 
			this.ComboBox_ResolutionMP.FormattingEnabled = true;
			this.ComboBox_ResolutionMP.Location = new System.Drawing.Point(94, 17);
			this.ComboBox_ResolutionMP.Name = "ComboBox_ResolutionMP";
			this.ComboBox_ResolutionMP.Size = new System.Drawing.Size(88, 21);
			this.ComboBox_ResolutionMP.TabIndex = 1;
			this.ComboBox_ResolutionMP.TextChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// CheckBox_FixedSeedMP
			// 
			this.CheckBox_FixedSeedMP.AutoSize = true;
			this.CheckBox_FixedSeedMP.Location = new System.Drawing.Point(6, 113);
			this.CheckBox_FixedSeedMP.Name = "CheckBox_FixedSeedMP";
			this.CheckBox_FixedSeedMP.Size = new System.Drawing.Size(79, 17);
			this.CheckBox_FixedSeedMP.TabIndex = 8;
			this.CheckBox_FixedSeedMP.Text = "Fixed Seed";
			this.CheckBox_FixedSeedMP.UseVisualStyleBackColor = true;
			this.CheckBox_FixedSeedMP.CheckedChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// CheckBox_VSyncMP
			// 
			this.CheckBox_VSyncMP.AutoSize = true;
			this.CheckBox_VSyncMP.Location = new System.Drawing.Point(94, 90);
			this.CheckBox_VSyncMP.Name = "CheckBox_VSyncMP";
			this.CheckBox_VSyncMP.Size = new System.Drawing.Size(57, 17);
			this.CheckBox_VSyncMP.TabIndex = 7;
			this.CheckBox_VSyncMP.Text = "VSync";
			this.CheckBox_VSyncMP.UseVisualStyleBackColor = true;
			this.CheckBox_VSyncMP.CheckedChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// CheckBox_RCMP
			// 
			this.CheckBox_RCMP.AutoSize = true;
			this.CheckBox_RCMP.Location = new System.Drawing.Point(94, 113);
			this.CheckBox_RCMP.Name = "CheckBox_RCMP";
			this.CheckBox_RCMP.Size = new System.Drawing.Size(99, 17);
			this.CheckBox_RCMP.TabIndex = 5;
			this.CheckBox_RCMP.Text = "Remote Control";
			this.CheckBox_RCMP.UseVisualStyleBackColor = true;
			this.CheckBox_RCMP.CheckedChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// CheckBox_PostProcessMP
			// 
			this.CheckBox_PostProcessMP.AutoSize = true;
			this.CheckBox_PostProcessMP.Location = new System.Drawing.Point(94, 44);
			this.CheckBox_PostProcessMP.Name = "CheckBox_PostProcessMP";
			this.CheckBox_PostProcessMP.Size = new System.Drawing.Size(88, 17);
			this.CheckBox_PostProcessMP.TabIndex = 4;
			this.CheckBox_PostProcessMP.Text = "Post Process";
			this.CheckBox_PostProcessMP.UseVisualStyleBackColor = true;
			this.CheckBox_PostProcessMP.CheckedChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// CheckBox_UnlitMP
			// 
			this.CheckBox_UnlitMP.AutoSize = true;
			this.CheckBox_UnlitMP.Location = new System.Drawing.Point(94, 67);
			this.CheckBox_UnlitMP.Name = "CheckBox_UnlitMP";
			this.CheckBox_UnlitMP.Size = new System.Drawing.Size(47, 17);
			this.CheckBox_UnlitMP.TabIndex = 4;
			this.CheckBox_UnlitMP.Text = "Unlit";
			this.CheckBox_UnlitMP.UseVisualStyleBackColor = true;
			this.CheckBox_UnlitMP.CheckedChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// CheckBox_MultiThreadMP
			// 
			this.CheckBox_MultiThreadMP.AutoSize = true;
			this.CheckBox_MultiThreadMP.Location = new System.Drawing.Point(6, 90);
			this.CheckBox_MultiThreadMP.Name = "CheckBox_MultiThreadMP";
			this.CheckBox_MultiThreadMP.Size = new System.Drawing.Size(81, 17);
			this.CheckBox_MultiThreadMP.TabIndex = 3;
			this.CheckBox_MultiThreadMP.Text = "Multi-thread";
			this.CheckBox_MultiThreadMP.UseVisualStyleBackColor = true;
			this.CheckBox_MultiThreadMP.CheckedChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// CheckBox_ShowLogMP
			// 
			this.CheckBox_ShowLogMP.AutoSize = true;
			this.CheckBox_ShowLogMP.Location = new System.Drawing.Point(6, 44);
			this.CheckBox_ShowLogMP.Name = "CheckBox_ShowLogMP";
			this.CheckBox_ShowLogMP.Size = new System.Drawing.Size(74, 17);
			this.CheckBox_ShowLogMP.TabIndex = 0;
			this.CheckBox_ShowLogMP.Text = "Show Log";
			this.CheckBox_ShowLogMP.UseVisualStyleBackColor = true;
			this.CheckBox_ShowLogMP.CheckedChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// CheckBox_SoundMP
			// 
			this.CheckBox_SoundMP.AutoSize = true;
			this.CheckBox_SoundMP.Location = new System.Drawing.Point(6, 67);
			this.CheckBox_SoundMP.Name = "CheckBox_SoundMP";
			this.CheckBox_SoundMP.Size = new System.Drawing.Size(57, 17);
			this.CheckBox_SoundMP.TabIndex = 2;
			this.CheckBox_SoundMP.Text = "Sound";
			this.CheckBox_SoundMP.UseVisualStyleBackColor = true;
			this.CheckBox_SoundMP.CheckedChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// groupBox4
			// 
			this.groupBox4.Controls.Add(this.Button_BrowseMP);
			this.groupBox4.Controls.Add(this.Button_KillAll);
			this.groupBox4.Controls.Add(this.ComboBox_MapnameMP);
			this.groupBox4.Controls.Add(this.NumericUpDown_Clients);
			this.groupBox4.Controls.Add(this.Button_ServerClient);
			this.groupBox4.Controls.Add(this.Button_AddClient);
			this.groupBox4.Controls.Add(this.Button_Server);
			this.groupBox4.Location = new System.Drawing.Point(6, 6);
			this.groupBox4.Name = "groupBox4";
			this.groupBox4.Size = new System.Drawing.Size(316, 86);
			this.groupBox4.TabIndex = 3;
			this.groupBox4.TabStop = false;
			this.groupBox4.Text = "Multiplayer";
			// 
			// Tab_Local
			// 
			this.Tab_Local.Controls.Add(this.groupBox5);
			this.Tab_Local.Controls.Add(this.groupBox2);
			this.Tab_Local.Location = new System.Drawing.Point(4, 22);
			this.Tab_Local.Name = "Tab_Local";
			this.Tab_Local.Padding = new System.Windows.Forms.Padding(3);
			this.Tab_Local.Size = new System.Drawing.Size(331, 272);
			this.Tab_Local.TabIndex = 0;
			this.Tab_Local.Text = "SP";
			this.Tab_Local.ToolTipText = "This tab allows running a single player game on the selected platform, or running" +
				" the editor on the PC.";
			this.Tab_Local.UseVisualStyleBackColor = true;
			// 
			// groupBox5
			// 
			this.groupBox5.Controls.Add(this.label7);
			this.groupBox5.Controls.Add(this.ComboBox_URLOptionsSP);
			this.groupBox5.Controls.Add(this.TextBox_ExecSP);
			this.groupBox5.Controls.Add(this.label5);
			this.groupBox5.Controls.Add(this.CheckBox_UnattendedSP);
			this.groupBox5.Controls.Add(this.ComboBox_ResolutionSP);
			this.groupBox5.Controls.Add(this.CheckBox_FixedSeedSP);
			this.groupBox5.Controls.Add(this.CheckBox_VSyncSP);
			this.groupBox5.Controls.Add(this.CheckBox_RCSP);
			this.groupBox5.Controls.Add(this.CheckBox_PostProcessSP);
			this.groupBox5.Controls.Add(this.CheckBox_UnlitSP);
			this.groupBox5.Controls.Add(this.CheckBox_MultiThreadSP);
			this.groupBox5.Controls.Add(this.CheckBox_ShowLogSP);
			this.groupBox5.Controls.Add(this.CheckBox_SoundSP);
			this.groupBox5.Location = new System.Drawing.Point(6, 98);
			this.groupBox5.Name = "groupBox5";
			this.groupBox5.Size = new System.Drawing.Size(316, 167);
			this.groupBox5.TabIndex = 10;
			this.groupBox5.TabStop = false;
			this.groupBox5.Text = "Options";
			// 
			// label7
			// 
			this.label7.AutoSize = true;
			this.label7.Location = new System.Drawing.Point(6, 139);
			this.label7.Name = "label7";
			this.label7.Size = new System.Drawing.Size(73, 13);
			this.label7.TabIndex = 11;
			this.label7.Text = "Extra Options:";
			// 
			// label5
			// 
			this.label5.AutoSize = true;
			this.label5.Location = new System.Drawing.Point(190, 22);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(63, 13);
			this.label5.TabIndex = 9;
			this.label5.Text = "Exec Cmds:";
			// 
			// groupBox2
			// 
			this.groupBox2.Controls.Add(this.CheckBox_UseCookedMap);
			this.groupBox2.Controls.Add(this.Button_Editor);
			this.groupBox2.Controls.Add(this.Button_BrowseSP);
			this.groupBox2.Controls.Add(this.Button_LoadMap);
			this.groupBox2.Controls.Add(this.ChapterPointSelect);
			this.groupBox2.Controls.Add(this.ComboBox_MapnameSP);
			this.groupBox2.Controls.Add(this.UseChapterPoint);
			this.groupBox2.Location = new System.Drawing.Point(6, 6);
			this.groupBox2.Name = "groupBox2";
			this.groupBox2.Size = new System.Drawing.Size(316, 86);
			this.groupBox2.TabIndex = 2;
			this.groupBox2.TabStop = false;
			this.groupBox2.Text = "Local";
			// 
			// ChapterPointSelect
			// 
			this.ChapterPointSelect.Location = new System.Drawing.Point(226, 21);
			this.ChapterPointSelect.Maximum = new decimal(new int[] {
            33,
            0,
            0,
            0});
			this.ChapterPointSelect.Name = "ChapterPointSelect";
			this.ChapterPointSelect.Size = new System.Drawing.Size(48, 20);
			this.ChapterPointSelect.TabIndex = 27;
			this.ChapterPointSelect.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			// 
			// UseChapterPoint
			// 
			this.UseChapterPoint.CheckAlign = System.Drawing.ContentAlignment.MiddleRight;
			this.UseChapterPoint.Location = new System.Drawing.Point(87, 19);
			this.UseChapterPoint.Name = "UseChapterPoint";
			this.UseChapterPoint.Size = new System.Drawing.Size(134, 28);
			this.UseChapterPoint.TabIndex = 28;
			this.UseChapterPoint.Text = "Start from chapterpoint";
			this.UseChapterPoint.CheckedChanged += new System.EventHandler(this.OnUseChapterPointChanged);
			// 
			// TabControl_Main
			// 
			this.TabControl_Main.Controls.Add(this.Tab_Local);
			this.TabControl_Main.Controls.Add(this.Tab_Multiplayer);
			this.TabControl_Main.Controls.Add(this.Tab_Cooking);
			this.TabControl_Main.Controls.Add(this.Tab_PCSetup);
			this.TabControl_Main.Controls.Add(this.Tab_ConsoleSetup);
			this.TabControl_Main.Controls.Add(this.Tab_PS3);
			this.TabControl_Main.Controls.Add(this.Tab_Help);
			this.TabControl_Main.Location = new System.Drawing.Point(9, 32);
			this.TabControl_Main.Name = "TabControl_Main";
			this.TabControl_Main.SelectedIndex = 0;
			this.TabControl_Main.Size = new System.Drawing.Size(339, 298);
			this.TabControl_Main.TabIndex = 22;
			this.TabControl_Main.SelectedIndexChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// Tab_PS3
			// 
			this.Tab_PS3.Controls.Add(this.Button_PackageMod);
			this.Tab_PS3.Controls.Add(this.CheckBox_LaunchAfterPS3Compile);
			this.Tab_PS3.Controls.Add(this.CheckBox_StopOnFirstError);
			this.Tab_PS3.Controls.Add(this.Label_AddProfileInfo);
			this.Tab_PS3.Controls.Add(this.ComboBox_AddProfInfo);
			this.Tab_PS3.Controls.Add(this.ComboBox_SelectPS3Compile);
			this.Tab_PS3.Controls.Add(this.NumericUpDown_NumberOfThreads);
			this.Tab_PS3.Controls.Add(this.Button_PS3Compile);
			this.Tab_PS3.Controls.Add(this.Label_Threads2);
			this.Tab_PS3.Controls.Add(this.CheckBox_FullRebuild);
			this.Tab_PS3.Controls.Add(this.CheckBox_NoDebugInfo);
			this.Tab_PS3.Location = new System.Drawing.Point(4, 22);
			this.Tab_PS3.Name = "Tab_PS3";
			this.Tab_PS3.Padding = new System.Windows.Forms.Padding(3);
			this.Tab_PS3.Size = new System.Drawing.Size(331, 272);
			this.Tab_PS3.TabIndex = 6;
			this.Tab_PS3.Text = "PS3";
			this.Tab_PS3.UseVisualStyleBackColor = true;
			// 
			// Button_PackageMod
			// 
			this.Button_PackageMod.Location = new System.Drawing.Point(16, 243);
			this.Button_PackageMod.Name = "Button_PackageMod";
			this.Button_PackageMod.Size = new System.Drawing.Size(97, 23);
			this.Button_PackageMod.TabIndex = 16;
			this.Button_PackageMod.Text = "Package Mod...";
			this.Button_PackageMod.UseVisualStyleBackColor = true;
			this.Button_PackageMod.Click += new System.EventHandler(this.ButtonPackageMod_Click);
			// 
			// CheckBox_LaunchAfterPS3Compile
			// 
			this.CheckBox_LaunchAfterPS3Compile.AutoSize = true;
			this.CheckBox_LaunchAfterPS3Compile.Location = new System.Drawing.Point(16, 118);
			this.CheckBox_LaunchAfterPS3Compile.Name = "CheckBox_LaunchAfterPS3Compile";
			this.CheckBox_LaunchAfterPS3Compile.Size = new System.Drawing.Size(173, 17);
			this.CheckBox_LaunchAfterPS3Compile.TabIndex = 15;
			this.CheckBox_LaunchAfterPS3Compile.Text = "Launch SP map after compiling";
			this.CheckBox_LaunchAfterPS3Compile.UseVisualStyleBackColor = true;
			// 
			// CheckBox_StopOnFirstError
			// 
			this.CheckBox_StopOnFirstError.AutoSize = true;
			this.CheckBox_StopOnFirstError.Location = new System.Drawing.Point(16, 95);
			this.CheckBox_StopOnFirstError.Name = "CheckBox_StopOnFirstError";
			this.CheckBox_StopOnFirstError.Size = new System.Drawing.Size(106, 17);
			this.CheckBox_StopOnFirstError.TabIndex = 14;
			this.CheckBox_StopOnFirstError.Text = "Stop on first error";
			this.CheckBox_StopOnFirstError.UseVisualStyleBackColor = true;
			// 
			// Label_AddProfileInfo
			// 
			this.Label_AddProfileInfo.AutoSize = true;
			this.Label_AddProfileInfo.Location = new System.Drawing.Point(13, 150);
			this.Label_AddProfileInfo.Name = "Label_AddProfileInfo";
			this.Label_AddProfileInfo.Size = new System.Drawing.Size(104, 13);
			this.Label_AddProfileInfo.TabIndex = 13;
			this.Label_AddProfileInfo.Text = "Additional profile info";
			// 
			// ComboBox_AddProfInfo
			// 
			this.ComboBox_AddProfInfo.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.ComboBox_AddProfInfo.FormattingEnabled = true;
			this.ComboBox_AddProfInfo.Location = new System.Drawing.Point(170, 145);
			this.ComboBox_AddProfInfo.Name = "ComboBox_AddProfInfo";
			this.ComboBox_AddProfInfo.Size = new System.Drawing.Size(147, 21);
			this.ComboBox_AddProfInfo.TabIndex = 12;
			this.ComboBox_AddProfInfo.SelectedIndexChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			this.ComboBox_AddProfInfo.SelectedValueChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// ComboBox_SelectPS3Compile
			// 
			this.ComboBox_SelectPS3Compile.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.ComboBox_SelectPS3Compile.FormattingEnabled = true;
			this.ComboBox_SelectPS3Compile.Items.AddRange(new object[] {
            "GCC Make",
            "Incredibuild Make",
            "XGE Make (remote)"});
			this.ComboBox_SelectPS3Compile.Location = new System.Drawing.Point(16, 8);
			this.ComboBox_SelectPS3Compile.Name = "ComboBox_SelectPS3Compile";
			this.ComboBox_SelectPS3Compile.Size = new System.Drawing.Size(147, 21);
			this.ComboBox_SelectPS3Compile.TabIndex = 11;
			this.ComboBox_SelectPS3Compile.SelectedIndexChanged += new System.EventHandler(this.ComboBox_SelectPS3Compile_SelectionChangeCommitted);
			this.ComboBox_SelectPS3Compile.SelectedValueChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// NumericUpDown_NumberOfThreads
			// 
			this.NumericUpDown_NumberOfThreads.Location = new System.Drawing.Point(280, 180);
			this.NumericUpDown_NumberOfThreads.Maximum = new decimal(new int[] {
            8,
            0,
            0,
            0});
			this.NumericUpDown_NumberOfThreads.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.NumericUpDown_NumberOfThreads.Name = "NumericUpDown_NumberOfThreads";
			this.NumericUpDown_NumberOfThreads.Size = new System.Drawing.Size(37, 20);
			this.NumericUpDown_NumberOfThreads.TabIndex = 10;
			this.NumericUpDown_NumberOfThreads.Value = new decimal(new int[] {
            2,
            0,
            0,
            0});
			this.NumericUpDown_NumberOfThreads.ValueChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// Button_PS3Compile
			// 
			this.Button_PS3Compile.Location = new System.Drawing.Point(219, 6);
			this.Button_PS3Compile.Name = "Button_PS3Compile";
			this.Button_PS3Compile.Size = new System.Drawing.Size(97, 23);
			this.Button_PS3Compile.TabIndex = 8;
			this.Button_PS3Compile.Text = "Compile";
			this.Button_PS3Compile.UseVisualStyleBackColor = true;
			this.Button_PS3Compile.Click += new System.EventHandler(this.Button_PS3Compile_Click);
			// 
			// Label_Threads2
			// 
			this.Label_Threads2.AutoSize = true;
			this.Label_Threads2.Location = new System.Drawing.Point(13, 184);
			this.Label_Threads2.Name = "Label_Threads2";
			this.Label_Threads2.Size = new System.Drawing.Size(121, 13);
			this.Label_Threads2.TabIndex = 7;
			this.Label_Threads2.Text = "Threads for the makefile";
			this.Label_Threads2.TextAlign = System.Drawing.ContentAlignment.TopCenter;
			// 
			// CheckBox_FullRebuild
			// 
			this.CheckBox_FullRebuild.AutoSize = true;
			this.CheckBox_FullRebuild.Location = new System.Drawing.Point(16, 48);
			this.CheckBox_FullRebuild.Name = "CheckBox_FullRebuild";
			this.CheckBox_FullRebuild.Size = new System.Drawing.Size(76, 17);
			this.CheckBox_FullRebuild.TabIndex = 5;
			this.CheckBox_FullRebuild.Text = "Full rebuild";
			this.CheckBox_FullRebuild.UseVisualStyleBackColor = true;
			this.CheckBox_FullRebuild.CheckedChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// CheckBox_NoDebugInfo
			// 
			this.CheckBox_NoDebugInfo.AutoSize = true;
			this.CheckBox_NoDebugInfo.Location = new System.Drawing.Point(16, 72);
			this.CheckBox_NoDebugInfo.Name = "CheckBox_NoDebugInfo";
			this.CheckBox_NoDebugInfo.Size = new System.Drawing.Size(93, 17);
			this.CheckBox_NoDebugInfo.TabIndex = 4;
			this.CheckBox_NoDebugInfo.Text = "No debug info";
			this.CheckBox_NoDebugInfo.UseVisualStyleBackColor = true;
			this.CheckBox_NoDebugInfo.CheckedChanged += new System.EventHandler(this.OnGenericControlValueChanged);
			// 
			// Tab_Help
			// 
			this.Tab_Help.Controls.Add(this.RichTextBox_Help);
			this.Tab_Help.Location = new System.Drawing.Point(4, 22);
			this.Tab_Help.Name = "Tab_Help";
			this.Tab_Help.Padding = new System.Windows.Forms.Padding(3);
			this.Tab_Help.Size = new System.Drawing.Size(331, 272);
			this.Tab_Help.TabIndex = 5;
			this.Tab_Help.Text = "Help";
			// 
			// RichTextBox_Help
			// 
			this.RichTextBox_Help.BackColor = System.Drawing.SystemColors.Window;
			this.RichTextBox_Help.Dock = System.Windows.Forms.DockStyle.Fill;
			this.RichTextBox_Help.Location = new System.Drawing.Point(3, 3);
			this.RichTextBox_Help.Name = "RichTextBox_Help";
			this.RichTextBox_Help.ReadOnly = true;
			this.RichTextBox_Help.ScrollBars = System.Windows.Forms.RichTextBoxScrollBars.Vertical;
			this.RichTextBox_Help.Size = new System.Drawing.Size(325, 266);
			this.RichTextBox_Help.TabIndex = 1;
			this.RichTextBox_Help.Text = "";
			this.RichTextBox_Help.LinkClicked += new System.Windows.Forms.LinkClickedEventHandler(this.OnTextBoxHelpClicked);
			// 
			// panel1
			// 
			this.panel1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)));
			this.panel1.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.panel1.Location = new System.Drawing.Point(353, 0);
			this.panel1.Margin = new System.Windows.Forms.Padding(0);
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(4, 362);
			this.panel1.TabIndex = 29;
			// 
			// label11
			// 
			this.label11.AutoSize = true;
			this.label11.Location = new System.Drawing.Point(360, 32);
			this.label11.Name = "label11";
			this.label11.Size = new System.Drawing.Size(103, 13);
			this.label11.TabIndex = 31;
			this.label11.Text = "Commandlet Output:";
			// 
			// statusStrip1
			// 
			this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabel1,
            this.ToolStripStatusLabel_PCConfig,
            this.toolStripStatusLabel2,
            this.ToolStripStatusLabel_ConsoleConfig,
            this.toolStripStatusLabel3,
            this.ToolStripStatusLabel_LocalMap,
            this.toolStripStatusLabel4,
            this.ToolStripStatusLabel_MPMap,
            this.toolStripStatusLabel5,
            this.ToolStripStatusLabel_CookMaps,
            this.toolStripStatusLabel6,
            this.ToolStripStatusLabel_URL});
			this.statusStrip1.Location = new System.Drawing.Point(0, 337);
			this.statusStrip1.Name = "statusStrip1";
			this.statusStrip1.Size = new System.Drawing.Size(719, 22);
			this.statusStrip1.TabIndex = 32;
			this.statusStrip1.Text = "statusStrip1";
			// 
			// toolStripStatusLabel1
			// 
			this.toolStripStatusLabel1.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Bold);
			this.toolStripStatusLabel1.Name = "toolStripStatusLabel1";
			this.toolStripStatusLabel1.Size = new System.Drawing.Size(24, 17);
			this.toolStripStatusLabel1.Text = "PC:";
			this.toolStripStatusLabel1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// ToolStripStatusLabel_PCConfig
			// 
			this.ToolStripStatusLabel_PCConfig.BorderSides = System.Windows.Forms.ToolStripStatusLabelBorderSides.Right;
			this.ToolStripStatusLabel_PCConfig.Name = "ToolStripStatusLabel_PCConfig";
			this.ToolStripStatusLabel_PCConfig.Size = new System.Drawing.Size(55, 17);
			this.ToolStripStatusLabel_PCConfig.Text = "PCConfig";
			this.ToolStripStatusLabel_PCConfig.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// toolStripStatusLabel2
			// 
			this.toolStripStatusLabel2.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Bold);
			this.toolStripStatusLabel2.Name = "toolStripStatusLabel2";
			this.toolStripStatusLabel2.Size = new System.Drawing.Size(54, 17);
			this.toolStripStatusLabel2.Text = "Console:";
			this.toolStripStatusLabel2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// ToolStripStatusLabel_ConsoleConfig
			// 
			this.ToolStripStatusLabel_ConsoleConfig.BorderSides = System.Windows.Forms.ToolStripStatusLabelBorderSides.Right;
			this.ToolStripStatusLabel_ConsoleConfig.Name = "ToolStripStatusLabel_ConsoleConfig";
			this.ToolStripStatusLabel_ConsoleConfig.Size = new System.Drawing.Size(80, 17);
			this.ToolStripStatusLabel_ConsoleConfig.Text = "ConsoleConfig";
			this.ToolStripStatusLabel_ConsoleConfig.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// toolStripStatusLabel3
			// 
			this.toolStripStatusLabel3.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Bold);
			this.toolStripStatusLabel3.Name = "toolStripStatusLabel3";
			this.toolStripStatusLabel3.Size = new System.Drawing.Size(51, 17);
			this.toolStripStatusLabel3.Text = "SP Map:";
			this.toolStripStatusLabel3.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// ToolStripStatusLabel_LocalMap
			// 
			this.ToolStripStatusLabel_LocalMap.BorderSides = System.Windows.Forms.ToolStripStatusLabelBorderSides.Right;
			this.ToolStripStatusLabel_LocalMap.Name = "ToolStripStatusLabel_LocalMap";
			this.ToolStripStatusLabel_LocalMap.Size = new System.Drawing.Size(55, 17);
			this.ToolStripStatusLabel_LocalMap.Text = "LocalMap";
			this.ToolStripStatusLabel_LocalMap.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// toolStripStatusLabel4
			// 
			this.toolStripStatusLabel4.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Bold);
			this.toolStripStatusLabel4.Name = "toolStripStatusLabel4";
			this.toolStripStatusLabel4.Size = new System.Drawing.Size(54, 17);
			this.toolStripStatusLabel4.Text = "MP Map:";
			this.toolStripStatusLabel4.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// ToolStripStatusLabel_MPMap
			// 
			this.ToolStripStatusLabel_MPMap.BorderSides = System.Windows.Forms.ToolStripStatusLabelBorderSides.Right;
			this.ToolStripStatusLabel_MPMap.Name = "ToolStripStatusLabel_MPMap";
			this.ToolStripStatusLabel_MPMap.Size = new System.Drawing.Size(45, 17);
			this.ToolStripStatusLabel_MPMap.Text = "MPMap";
			this.ToolStripStatusLabel_MPMap.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// toolStripStatusLabel5
			// 
			this.toolStripStatusLabel5.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Bold);
			this.toolStripStatusLabel5.Name = "toolStripStatusLabel5";
			this.toolStripStatusLabel5.Size = new System.Drawing.Size(85, 17);
			this.toolStripStatusLabel5.Text = "Cooked Maps:";
			this.toolStripStatusLabel5.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// ToolStripStatusLabel_CookMaps
			// 
			this.ToolStripStatusLabel_CookMaps.BorderSides = System.Windows.Forms.ToolStripStatusLabelBorderSides.Right;
			this.ToolStripStatusLabel_CookMaps.Name = "ToolStripStatusLabel_CookMaps";
			this.ToolStripStatusLabel_CookMaps.Size = new System.Drawing.Size(60, 17);
			this.ToolStripStatusLabel_CookMaps.Text = "CookMaps";
			this.ToolStripStatusLabel_CookMaps.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// toolStripStatusLabel6
			// 
			this.toolStripStatusLabel6.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Bold);
			this.toolStripStatusLabel6.Name = "toolStripStatusLabel6";
			this.toolStripStatusLabel6.Size = new System.Drawing.Size(32, 17);
			this.toolStripStatusLabel6.Text = "URL:";
			this.toolStripStatusLabel6.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// ToolStripStatusLabel_URL
			// 
			this.ToolStripStatusLabel_URL.Name = "ToolStripStatusLabel_URL";
			this.ToolStripStatusLabel_URL.Overflow = System.Windows.Forms.ToolStripItemOverflow.Never;
			this.ToolStripStatusLabel_URL.Size = new System.Drawing.Size(109, 17);
			this.ToolStripStatusLabel_URL.Spring = true;
			this.ToolStripStatusLabel_URL.Text = "Preview";
			this.ToolStripStatusLabel_URL.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// toolStrip1
			// 
			this.toolStrip1.AllowItemReorder = true;
			this.toolStrip1.AllowMerge = false;
			this.toolStrip1.GripStyle = System.Windows.Forms.ToolStripGripStyle.Hidden;
			this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripLabel1,
            this.ComboBox_Platform,
            this.toolStripSeparator1,
            this.toolStripLabel2,
            this.ComboBox_Game,
            this.toolStripSeparator2,
            this.ToolStripButton_Launch,
            this.toolStripSeparator7,
            this.ToolStripButton_Editor,
            this.toolStripSeparator3,
            this.ToolStripButton_Server,
            this.toolStripSeparator4,
            this.ToolStripButton_Cook,
            this.toolStripSeparator5,
            this.ToolStripButton_CompileScript,
            this.toolStripSeparator6,
            this.ToolStripButton_Sync,
            this.toolStripSeparator8,
            this.ToolStripButton_Reboot});
			this.toolStrip1.LayoutStyle = System.Windows.Forms.ToolStripLayoutStyle.HorizontalStackWithOverflow;
			this.toolStrip1.Location = new System.Drawing.Point(0, 0);
			this.toolStrip1.Name = "toolStrip1";
			this.toolStrip1.Size = new System.Drawing.Size(719, 25);
			this.toolStrip1.TabIndex = 33;
			this.toolStrip1.Text = "toolStrip1";
			// 
			// toolStripLabel1
			// 
			this.toolStripLabel1.Name = "toolStripLabel1";
			this.toolStripLabel1.Size = new System.Drawing.Size(51, 22);
			this.toolStripLabel1.Text = "Platform:";
			// 
			// ComboBox_Platform
			// 
			this.ComboBox_Platform.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.ComboBox_Platform.Name = "ComboBox_Platform";
			this.ComboBox_Platform.Size = new System.Drawing.Size(125, 25);
			this.ComboBox_Platform.SelectedIndexChanged += new System.EventHandler(this.OnPlatformChanged);
			// 
			// toolStripSeparator1
			// 
			this.toolStripSeparator1.Name = "toolStripSeparator1";
			this.toolStripSeparator1.Size = new System.Drawing.Size(6, 25);
			// 
			// toolStripLabel2
			// 
			this.toolStripLabel2.Name = "toolStripLabel2";
			this.toolStripLabel2.Size = new System.Drawing.Size(38, 22);
			this.toolStripLabel2.Text = "Game:";
			// 
			// ComboBox_Game
			// 
			this.ComboBox_Game.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.ComboBox_Game.Name = "ComboBox_Game";
			this.ComboBox_Game.Size = new System.Drawing.Size(126, 25);
			this.ComboBox_Game.SelectedIndexChanged += new System.EventHandler(this.OnGameNameChanged);
			// 
			// toolStripSeparator2
			// 
			this.toolStripSeparator2.Name = "toolStripSeparator2";
			this.toolStripSeparator2.Size = new System.Drawing.Size(6, 25);
			// 
			// ToolStripButton_Launch
			// 
			this.ToolStripButton_Launch.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
			this.ToolStripButton_Launch.Image = ((System.Drawing.Image)(resources.GetObject("ToolStripButton_Launch.Image")));
			this.ToolStripButton_Launch.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.ToolStripButton_Launch.Name = "ToolStripButton_Launch";
			this.ToolStripButton_Launch.Size = new System.Drawing.Size(45, 22);
			this.ToolStripButton_Launch.Text = "Launch";
			this.ToolStripButton_Launch.Click += new System.EventHandler(this.OnLoadMapClick);
			// 
			// toolStripSeparator7
			// 
			this.toolStripSeparator7.Name = "toolStripSeparator7";
			this.toolStripSeparator7.Size = new System.Drawing.Size(6, 25);
			// 
			// ToolStripButton_Editor
			// 
			this.ToolStripButton_Editor.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
			this.ToolStripButton_Editor.Image = ((System.Drawing.Image)(resources.GetObject("ToolStripButton_Editor.Image")));
			this.ToolStripButton_Editor.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.ToolStripButton_Editor.Name = "ToolStripButton_Editor";
			this.ToolStripButton_Editor.Size = new System.Drawing.Size(39, 22);
			this.ToolStripButton_Editor.Text = "Editor";
			this.ToolStripButton_Editor.Click += new System.EventHandler(this.OnEditorClick);
			// 
			// toolStripSeparator3
			// 
			this.toolStripSeparator3.Name = "toolStripSeparator3";
			this.toolStripSeparator3.Size = new System.Drawing.Size(6, 25);
			// 
			// ToolStripButton_Server
			// 
			this.ToolStripButton_Server.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
			this.ToolStripButton_Server.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.ToolStripButton_ServerAndClients,
            this.ToolStripButton_AddClient,
            this.ToolStripButton_KillAll});
			this.ToolStripButton_Server.Image = ((System.Drawing.Image)(resources.GetObject("ToolStripButton_Server.Image")));
			this.ToolStripButton_Server.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.ToolStripButton_Server.Name = "ToolStripButton_Server";
			this.ToolStripButton_Server.Size = new System.Drawing.Size(55, 22);
			this.ToolStripButton_Server.Text = "Server";
			this.ToolStripButton_Server.ButtonClick += new System.EventHandler(this.OnServerClick);
			// 
			// ToolStripButton_ServerAndClients
			// 
			this.ToolStripButton_ServerAndClients.Name = "ToolStripButton_ServerAndClients";
			this.ToolStripButton_ServerAndClients.Size = new System.Drawing.Size(172, 22);
			this.ToolStripButton_ServerAndClients.Text = "Server + n Clients";
			this.ToolStripButton_ServerAndClients.Click += new System.EventHandler(this.OnServerClientClick);
			// 
			// ToolStripButton_AddClient
			// 
			this.ToolStripButton_AddClient.Name = "ToolStripButton_AddClient";
			this.ToolStripButton_AddClient.Size = new System.Drawing.Size(172, 22);
			this.ToolStripButton_AddClient.Text = "Add Client";
			this.ToolStripButton_AddClient.Click += new System.EventHandler(this.OnClientClick);
			// 
			// ToolStripButton_KillAll
			// 
			this.ToolStripButton_KillAll.Name = "ToolStripButton_KillAll";
			this.ToolStripButton_KillAll.Size = new System.Drawing.Size(172, 22);
			this.ToolStripButton_KillAll.Text = "Kill All";
			this.ToolStripButton_KillAll.Click += new System.EventHandler(this.OnKillAllClick);
			// 
			// toolStripSeparator4
			// 
			this.toolStripSeparator4.Name = "toolStripSeparator4";
			this.toolStripSeparator4.Size = new System.Drawing.Size(6, 25);
			// 
			// ToolStripButton_Cook
			// 
			this.ToolStripButton_Cook.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
			this.ToolStripButton_Cook.Image = ((System.Drawing.Image)(resources.GetObject("ToolStripButton_Cook.Image")));
			this.ToolStripButton_Cook.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.ToolStripButton_Cook.Name = "ToolStripButton_Cook";
			this.ToolStripButton_Cook.Size = new System.Drawing.Size(35, 22);
			this.ToolStripButton_Cook.Text = "Cook";
			this.ToolStripButton_Cook.Click += new System.EventHandler(this.OnStartCookingClick);
			// 
			// toolStripSeparator5
			// 
			this.toolStripSeparator5.Name = "toolStripSeparator5";
			this.toolStripSeparator5.Size = new System.Drawing.Size(6, 25);
			// 
			// ToolStripButton_CompileScript
			// 
			this.ToolStripButton_CompileScript.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
			this.ToolStripButton_CompileScript.Image = ((System.Drawing.Image)(resources.GetObject("ToolStripButton_CompileScript.Image")));
			this.ToolStripButton_CompileScript.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.ToolStripButton_CompileScript.Name = "ToolStripButton_CompileScript";
			this.ToolStripButton_CompileScript.Size = new System.Drawing.Size(36, 22);
			this.ToolStripButton_CompileScript.Text = "Make";
			this.ToolStripButton_CompileScript.Click += new System.EventHandler(this.OnCompileClick);
			// 
			// toolStripSeparator6
			// 
			this.toolStripSeparator6.Name = "toolStripSeparator6";
			this.toolStripSeparator6.Size = new System.Drawing.Size(6, 25);
			// 
			// ToolStripButton_Sync
			// 
			this.ToolStripButton_Sync.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
			this.ToolStripButton_Sync.Image = ((System.Drawing.Image)(resources.GetObject("ToolStripButton_Sync.Image")));
			this.ToolStripButton_Sync.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.ToolStripButton_Sync.Name = "ToolStripButton_Sync";
			this.ToolStripButton_Sync.Size = new System.Drawing.Size(34, 22);
			this.ToolStripButton_Sync.Text = "Sync";
			this.ToolStripButton_Sync.Click += new System.EventHandler(this.OnCopyToTargetsClick);
			// 
			// toolStripSeparator8
			// 
			this.toolStripSeparator8.Name = "toolStripSeparator8";
			this.toolStripSeparator8.Size = new System.Drawing.Size(6, 25);
			// 
			// ToolStripButton_Reboot
			// 
			this.ToolStripButton_Reboot.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
			this.ToolStripButton_Reboot.Image = ((System.Drawing.Image)(resources.GetObject("ToolStripButton_Reboot.Image")));
			this.ToolStripButton_Reboot.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.ToolStripButton_Reboot.Name = "ToolStripButton_Reboot";
			this.ToolStripButton_Reboot.Size = new System.Drawing.Size(46, 22);
			this.ToolStripButton_Reboot.Text = "Reboot";
			this.ToolStripButton_Reboot.Click += new System.EventHandler(this.OnRebootTargetsClick);
			// 
			// CookMapToolTip
			// 
			this.CookMapToolTip.ShowAlways = true;
			// 
			// UnrealFrontendWindow
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(719, 359);
			this.Controls.Add(this.toolStrip1);
			this.Controls.Add(this.statusStrip1);
			this.Controls.Add(this.panel1);
			this.Controls.Add(this.TabControl_Main);
			this.Controls.Add(this.RichTextBox_LogWindow);
			this.Controls.Add(this.label11);
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.KeyPreview = true;
			this.MinimumSize = new System.Drawing.Size(727, 393);
			this.Name = "UnrealFrontendWindow";
			this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Show;
			this.Text = "Unreal Frontend";
			this.SizeChanged += new System.EventHandler(this.OnSizeChanged);
			this.Closing += new System.ComponentModel.CancelEventHandler(this.CookFrontEnd_Closing);
			this.KeyDown += new System.Windows.Forms.KeyEventHandler(this.OnUnrealFrontendWindowKeyDown);
			this.Load += new System.EventHandler(this.OnUnrealFrontendWindowLoad);
			((System.ComponentModel.ISupportInitialize)(this.NumericUpDown_Clients)).EndInit();
			this.Tab_Cooking.ResumeLayout(false);
			this.Tab_Cooking.PerformLayout();
			this.CookingGroupBox.ResumeLayout(false);
			this.CookingGroupBox.PerformLayout();
			this.Tab_ConsoleSetup.ResumeLayout(false);
			this.Tab_ConsoleSetup.PerformLayout();
			this.groupBox1.ResumeLayout(false);
			this.Tab_PCSetup.ResumeLayout(false);
			this.Tab_PCSetup.PerformLayout();
			this.groupBox6.ResumeLayout(false);
			this.groupBox6.PerformLayout();
			this.Tab_Multiplayer.ResumeLayout(false);
			this.groupBox3.ResumeLayout(false);
			this.groupBox3.PerformLayout();
			this.groupBox4.ResumeLayout(false);
			this.Tab_Local.ResumeLayout(false);
			this.groupBox5.ResumeLayout(false);
			this.groupBox5.PerformLayout();
			this.groupBox2.ResumeLayout(false);
			this.groupBox2.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.ChapterPointSelect)).EndInit();
			this.TabControl_Main.ResumeLayout(false);
			this.Tab_PS3.ResumeLayout(false);
			this.Tab_PS3.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.NumericUpDown_NumberOfThreads)).EndInit();
			this.Tab_Help.ResumeLayout(false);
			this.statusStrip1.ResumeLayout(false);
			this.statusStrip1.PerformLayout();
			this.toolStrip1.ResumeLayout(false);
			this.toolStrip1.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}
		#endregion

		#region Application

		#region Main/startup
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main(string[] Args)
		{
			Application.EnableVisualStyles();

			UnrealFrontendWindow UnrealFrontend = new UnrealFrontendWindow();
			if (!UnrealFrontend.Init())
			{
				return;
			}

			while (UnrealFrontend.Running)
			{
				// Process system messages (allow OnTimer to be called)
				Application.DoEvents();

                // special ps3 compilation handler
                if (UnrealFrontend.bNeedToHandlePS3MakeCompleted)
                {
                    UnrealFrontend.HandlePS3MakeCompleted();
                }

				// Tick changes once every 100ms
				if (UnrealFrontend.GetTicked() != 0)
				{
					UnrealFrontend.RunFrame();
				}

				// Yield to system
				System.Threading.Thread.Sleep(1);
			}
		}

		public UnrealFrontendWindow()
		{
			// Required for Windows Form Designer support
			InitializeComponent();

			// Add the extensions for the various game types
			MapExtensionDictionary.Add("ExampleGame", ".umap");
			MapExtensionDictionary.Add("GearGame", ".gear");
			MapExtensionDictionary.Add("UTGame", ".ut3");
			MapExtensionDictionary.Add("WarGame", ".war");

			// remember the foreground color
			DefaultLogWindowForegroundColor = RichTextBox_LogWindow.ForeColor;

			// set the window title to the current version
//			this.Text = VersionStr;

			// add pretty text to the help text
			Font DefaultFont = RichTextBox_Help.SelectionFont;
			Font BoldFont = new Font(DefaultFont.FontFamily, DefaultFont.Size, FontStyle.Bold);

			RichTextBox_Help.SelectionFont = BoldFont;
			RichTextBox_Help.AppendText("Info:\r\n");

			RichTextBox_Help.SelectionFont = DefaultFont;
			RichTextBox_Help.AppendText("This program can be used as a frontend for controlling many aspects of the Unreal Engine. It " +
				"can launch your game on the PC or a console, run multiple clients for testing multiplayer games locally, edit maps, " +
				"compile script code, cook data for PC or console, reboot consoles, etc.\r\nCommandlets (cooking, script compiling) " +
				"will output to the text box on the right.\r\n\r\n");

			RichTextBox_Help.SelectionFont = BoldFont;
			RichTextBox_Help.AppendText("Shortcuts:\r\n");

			RichTextBox_Help.SelectionFont = DefaultFont;
			RichTextBox_Help.AppendText("  F1-F6 - Quickly jump between tabs.\r\n");
			RichTextBox_Help.AppendText("  Ctrl-L - Loads SP map.\r\n");
			RichTextBox_Help.AppendText("  Ctrl-E - Launches editor.\r\n");
			RichTextBox_Help.AppendText("  Ctrl-S - Starts server.\r\n");
			RichTextBox_Help.AppendText("  Ctrl-K - Cooks data.\r\n");
			RichTextBox_Help.AppendText("  Ctrl-M - Compiles script code.\r\n");
			RichTextBox_Help.AppendText("  Ctrl-Y - Syncs data to targets.\r\n");
			RichTextBox_Help.AppendText("  Ctrl-R - Reboots targets.\r\n\r\n");

			RichTextBox_Help.SelectionFont = BoldFont;
			RichTextBox_Help.AppendText("More Help:\r\n");

			RichTextBox_Help.SelectionFont = DefaultFont;
			RichTextBox_Help.AppendText("Most controls have toolstips to explain what they do. Also, visit https://udn.epicgames.com/Three/UnrealFrontend " + 
				"for more information.\r\n\r\n");

			// copy over SP->MP tooltips
			DuplicateToolTips();

			ListBox_Targets.ItemCheck += new ItemCheckEventHandler(ListBox_Targets_ItemCheck);

			TextBox_MapsToCook.TextChanged += new EventHandler(TextBox_MapsToCook_TextChanged);
			TextBox_MapsToCook.KeyPress += new KeyPressEventHandler(TextBox_MapsToCook_KeyPress);
			TextBox_MapsToCook.LostFocus += new EventHandler(TextBox_MapsToCook_LostFocus);
		}

		/// <summary>
		/// Event handler for when TextBox_MapsToCook loses its input focus.
		/// </summary>
		/// <param name="sender">The object that generated the event.</param>
		/// <param name="e">Information about the event.</param>
		void TextBox_MapsToCook_LostFocus(object sender, EventArgs e)
		{
			CookMapToolTip.Hide(TextBox_MapsToCook.Parent);
		}

		/// <summary>
		/// Event handler for when a key has been pressed in the text box that holds onto the list of maps that are to be cooked.
		/// </summary>
		/// <param name="sender">The text box instance.</param>
		/// <param name="e">Information about the event.</param>
		void TextBox_MapsToCook_KeyPress(object sender, KeyPressEventArgs e)
		{
			// When enter is hit fill in the text box with all maps that fit the current search criteria
			if(e.KeyChar == '\r')
			{
				Cursor CurCursor = this.Cursor;
				this.Cursor = Cursors.WaitCursor;

				BuildMapsToCookString();

				this.Cursor = CurCursor;
			}
		}

		/// <summary>
		/// Builds the string of maps to be cooked based on a set of filters.
		/// </summary>
		private void BuildMapsToCookString()
		{
			try
			{
				string DirContent = this.ComboBox_ProjectDir.Text + "..\\" + ComboBox_Game.Text + "\\Content";
				StringBuilder EntryBldr = new StringBuilder();

				string[] SubStrings = TextBox_MapsToCook.Text.Trim().Split(' ');

				for(int i = 0; i < SubStrings.Length; ++i)
				{
					// Check to see if a wild card is present, if one isn't then append our own
					string SearchString = SubStrings[i];

					if(!SearchString.Contains("*") || !SearchString.Contains("?"))
					{
						SearchString += "*";
					}

					// Append the extension of the current game's maps if an extension isn't already provided
					string Extension;
					if(MapExtensionDictionary.TryGetValue(ComboBox_Game.Text, out Extension) && !Path.HasExtension(SearchString))
					{
						SearchString += Extension;
					}

					if(Directory.Exists(DirContent))
					{
						BuildCookMapEntriesString(EntryBldr, DirContent, SearchString);
					}
				}

				// Get rid of trailing space
				if(EntryBldr.Length > 0)
				{
					EntryBldr.Length = EntryBldr.Length - 1;
				}

				TextBox_MapsToCook.Text = EntryBldr.ToString();
			}
			catch(Exception Error)
			{
				System.Diagnostics.Trace.WriteLine(Error.ToString());
			}
		}

		/// <summary>
		/// Builds the list of maps to be cooked.
		/// </summary>
		/// <param name="EntryBldr">The <see cref="System.StringBuilder"/> containing the resulting list of maps.</param>
		/// <param name="Dir">The directory to search for files in.</param>
		/// <param name="SearchString">The string used to filter files in <see cref="Dir"/>.</param>
		void BuildCookMapEntriesString(StringBuilder EntryBldr, string Dir, string SearchString)
		{
			string[] Files = Directory.GetFiles(Dir, SearchString, SearchOption.AllDirectories);

			foreach(string File in Files)
			{
				if(!File.Contains("Autosaves"))
				{
					EntryBldr.AppendFormat("{0} ", Path.GetFileNameWithoutExtension(File));
				}
			}
		}

		/// <summary>
		/// Event handler for when the text box containing the maps to be cooked has been modified.
		/// </summary>
		/// <param name="sender">The text box that has been modified.</param>
		/// <param name="e">Information about the event.</param>
		void TextBox_MapsToCook_TextChanged(object sender, EventArgs e)
		{
			if(!bDisposing && TextBox_MapsToCook.Focused)
			{
				BuildCookMapToolTip();
			}
		}

		/// <summary>
		/// Creates a tool tip for showing all available maps for the current game type.
		/// </summary>
		private void BuildCookMapToolTip()
		{
			try
			{
				string DirContent = this.ComboBox_ProjectDir.Text + "..\\" + ComboBox_Game.Text + "\\Content";
				StringBuilder ToolTipBldr = new StringBuilder();

				string[] SubStrings = TextBox_MapsToCook.Text.Trim().Split(' ');

				for(int i = 0; i < SubStrings.Length; ++i)
				{
					// Check to see if a wild card is present, if one isn't then append our own
					string SearchString = SubStrings[i];

					if(!SearchString.Contains("*") || !SearchString.Contains("?"))
					{
						SearchString += "*";
					}

					// Append the extension of the current game's maps if an extension isn't already provided
					string Extension;
					if(MapExtensionDictionary.TryGetValue(ComboBox_Game.Text, out Extension) && !Path.HasExtension(SearchString))
					{
						SearchString += Extension;
					}

					if(Directory.Exists(DirContent))
					{
						BuildCookMapToolTipString(ToolTipBldr, DirContent, SearchString);
					}
				}

				ToolTipBldr.Append("\r\nHit the \'Enter\' key to automatically fill in the list of maps.\r\nMultiple search strings can be separated by a space.");

				CookMapToolTip.Show(ToolTipBldr.ToString(), TextBox_MapsToCook.Parent, TextBox_MapsToCook.Location.X + TextBox_MapsToCook.Width, TextBox_MapsToCook.Location.Y, 100000);
			}
			catch(Exception Error)
			{
				System.Diagnostics.Trace.WriteLine(Error.ToString());
			}
		}

		/// <summary>
		/// Builds the tool tip string used for displaying all maps that fit the supplied search string when typing in the name of a map to cook.
		/// </summary>
		/// <param name="ToolTipBldr">The <see cref="System.StringBuilder"/> object containing the resulting tool tip string.</param>
		/// <param name="Dir">The directory to search for files in.</param>
		/// <param name="SearchString">The string used to filter the files in <see cref="Dir"/>.</param>
		void BuildCookMapToolTipString(StringBuilder ToolTipBldr, string Dir, string SearchString)
		{
			string[] Files = Directory.GetFiles(Dir, SearchString, SearchOption.AllDirectories);

			foreach(string File in Files)
			{
				string FullPath = Path.GetFullPath(File);

				if(!FullPath.Contains("Autosaves"))
				{
					ToolTipBldr.AppendFormat("{0}\r\n", FullPath);
				}
			}
		}

		/// <summary>
		/// Handles the ItemCheckEvent for the target list box and updates the state of the current platform's targets.
		/// </summary>
		/// <param name="sender">The instance that triggered the event.</param>
		/// <param name="e">Information about the event.</param>
		void ListBox_Targets_ItemCheck(object sender, ItemCheckEventArgs e)
		{
			Dictionary<string, bool> CurrentPlatform;

			//get the current platform
			if(PlatformDefaults.TryGetValue(ComboBox_Platform.Text, out CurrentPlatform))
			{
				//get the current target
				if(CurrentPlatform.ContainsKey(ListBox_Targets.Items[e.Index].ToString()))
				{
					//update the target's state
					CurrentPlatform[ListBox_Targets.Items[e.Index].ToString()] = e.NewValue == CheckState.Checked;
				}
			}
		}

		/// <summary>
		/// Copy a single controls tooltip to another control
		/// </summary>
		/// <param name="Source">Control to copy the tooltip from</param>
		/// <param name="Dest">Control to copy the tooltip to</param>
		private void DuplicateToolTip(Control Source, Control Dest)
		{
			this.FormToolTips.SetToolTip(Dest, FormToolTips.GetToolTip(Source));

		}

		/// <summary>
		/// Copy a single controls tooltip to another control
		/// </summary>
		/// <param name="Source">Control to copy the tooltip from</param>
		/// <param name="Dest">Control to copy the tooltip to</param>
		private void DuplicateToolTip(Control Source, ToolStripItem Dest)
		{
			Dest.ToolTipText = FormToolTips.GetToolTip(Source);

		}

		/// <summary>
		/// For all the duplicated options, duplicate the tooltips in code so that 
		/// changing one will change both
		/// </summary>
		private void DuplicateToolTips()
		{
			// duplicate SP->MP button tooltips
			DuplicateToolTip(CheckBox_UnattendedSP, CheckBox_UnattendedMP);
			DuplicateToolTip(CheckBox_ShowLogSP, CheckBox_ShowLogMP);
			DuplicateToolTip(CheckBox_SoundSP, CheckBox_SoundMP);
			DuplicateToolTip(CheckBox_MultiThreadSP, CheckBox_MultiThreadMP);
			DuplicateToolTip(CheckBox_FixedSeedSP, CheckBox_FixedSeedMP);
			DuplicateToolTip(CheckBox_PostProcessSP, CheckBox_PostProcessMP);
			DuplicateToolTip(CheckBox_UnlitSP, CheckBox_UnlitMP);
			DuplicateToolTip(CheckBox_VSyncSP, CheckBox_VSyncMP);
			DuplicateToolTip(CheckBox_RCSP, CheckBox_RCMP);
			DuplicateToolTip(ComboBox_ResolutionSP, ComboBox_ResolutionMP);
			DuplicateToolTip(TextBox_ExecSP, TextBox_ExecMP);
			DuplicateToolTip(ComboBox_URLOptionsSP, ComboBox_URLOptionsMP);

			// duplicate tooltips into the toolstrip buttons
			DuplicateToolTip(Button_LoadMap, ToolStripButton_Launch);
			DuplicateToolTip(Button_Editor, ToolStripButton_Editor);
			DuplicateToolTip(Button_Server, ToolStripButton_Server);
			DuplicateToolTip(Button_ServerClient, ToolStripButton_ServerAndClients);
			DuplicateToolTip(Button_AddClient, ToolStripButton_AddClient);
			DuplicateToolTip(Button_KillAll, ToolStripButton_KillAll);
			DuplicateToolTip(Button_StartCooking, ToolStripButton_Cook);
			DuplicateToolTip(Button_Compile, ToolStripButton_CompileScript);
			DuplicateToolTip(Button_CopyToTargets, ToolStripButton_Sync);
			DuplicateToolTip(Button_RebootTargets, ToolStripButton_Reboot);
		}

		private void OnUnrealFrontendWindowLoad(object sender, EventArgs e)
		{
			string[] cmdparms = Environment.GetCommandLineArgs();

			if (cmdparms.Length == 2 && File.Exists(cmdparms[1]))
			{
				ImportMapList(cmdparms[1]);
				StartCommandlet(ECommandletType.CT_Cook);
			}
		}

		/// <summary>
		/// Reads the supported game type and configuration settings. Saves a
		/// set of defaults if missing
		/// </summary>
		protected bool LoadCookerConfigAndMakeGameOptions()
		{
			// Read the global settings XML file
			CookerSettings CookerCfg = ReadCookerSettings();
            if( CookerCfg == null )
            {
                MessageBox.Show( "Failed to find the UnrealFrontend_cfg.xml file, which is required to run.", "Failed to find settings file", MessageBoxButtons.OK, MessageBoxIcon.Error );
                return ( false );
            }
            else
            {
                // get the path to the directory where we will store settings
                SettingsPath = Environment.GetFolderPath( Environment.SpecialFolder.MyDocuments ) + "\\My Games\\" + CookerCfg.UserSettingsDirectory;

                // make sure it exists
                Directory.CreateDirectory( SettingsPath );
                Directory.CreateDirectory( SettingsPath + "\\Binaries" );

                // PC is always usable
				if( CookerTools.Activate("PC") )
                {
					UsablePlatforms.Add( "PC" );
				}

                // query for support of known platforms, and add to the list of good platforms
                foreach( string PlatformName in CookerCfg.Platforms )
                {
                    if( CookerTools.Activate( PlatformName ) )
                    {
                        UsablePlatforms.Add( PlatformName );
                    }
                }

                Options = new ArrayList();
                Options.Add( new ComboBoxGameOption( ComboBox_Game.ComboBox, "GameName", CookerCfg.Games ) );
                Options.Add( new ComboBoxGameOption( ComboBox_PCConfiguration, "CookerConfig", CookerCfg.PCConfigs ) );
                Options.Add( new ComboBoxGameOption( ComboBox_ConsoleConfiguration, "GameConfig", CookerCfg.ConsoleConfigs ) );
                Options.Add( new ComboBoxGameOption( ComboBox_Platform.ComboBox, "Platform", UsablePlatforms.ToArray() ) );
                Options.Add( new ComboBoxGameOption( ComboBox_ProjectDir, "Dir", new string[] { Application.StartupPath + "\\" } ) );
                Options.Add( new ComboBoxGameOption( ComboBox_BaseDirectory, "BaseDir", new string[] { "UnrealEngine3" } ) );
                Options.Add( new CheckBoxGameOption( CheckBox_RunWithCookedData, GameOption.EOptionType.OT_CUSTOM, "RunWithCookedData", " -seekfreeloading", "", false ) );
                Options.Add( new SimpleIntGameOption( "WindowSpacing", 40 ) );

                // cooker options
                Options.Add( new SimpleIntGameOption( "FrontendX", -1 ) );
                Options.Add( new SimpleIntGameOption( "FrontendY", -1 ) );
                Options.Add( new SimpleIntGameOption( "FrontendW", -1 ) );
                Options.Add( new SimpleIntGameOption( "FrontendH", -1 ) );
                Options.Add( new SimpleIntGameOption( "SelectedTab", 0 ) );
                Options.Add( new SimpleIntGameOption( "WindowState", ( int )FormWindowState.Normal ) );

                CookingOptions = new ArrayList();
                CookingOptions.Add( new CheckBoxGameOption( CheckBox_CookInisIntsOnly, GameOption.EOptionType.OT_CUSTOM, "CookInis", "", "", false ) );
                CookingOptions.Add( new CheckBoxGameOption( CheckBox_CookFinalReleaseScript, GameOption.EOptionType.OT_CUSTOM, "CookFinalRelease", "", "", false ) );
                CookingOptions.Add( new CheckBoxGameOption( CheckBox_CookAllMaps, GameOption.EOptionType.OT_CUSTOM, "CookAllMaps", "", "", false ) );
                CookingOptions.Add( new CheckBoxGameOption( CheckBox_LaunchAfterCooking, GameOption.EOptionType.OT_CUSTOM, "LaunchAfterCooking", "", "", false ) );
                CookingOptions.Add( new CheckBoxGameOption( CheckBox_UseCookedMap, GameOption.EOptionType.OT_CUSTOM, "RunWithCookedMap", "", "", false ) );
                CookingOptions.Add( new ComboBoxGameOption( ComboBox_CookingLanguage, "CookingLanguage", new string[] { "int" } ) );
                CookingOptions.Add( new CheckBoxGameOption( CheckBox_CookInUserMode, GameOption.EOptionType.OT_CUSTOM, "CookInUserMode", "", "", false ) );

                // setup pergame 
                PerGameOptions = new Dictionary<string, ArrayList>();
                foreach( string Game in CookerCfg.Games )
                {
                    PerGameOptions[Game] = new ArrayList();
                    PerGameOptions[Game].Add( new TextBoxGameOption( TextBox_MapsToCook, "Maps", GameOption.EOptionType.OT_CUSTOM ) );
                    PerGameOptions[Game].Add( new ComboBoxGameOption( ComboBox_MapnameSP, "MapNameSP", null ) );
                    PerGameOptions[Game].Add( new ComboBoxGameOption( ComboBox_MapnameMP, "MapNameMP", null ) );
                }

                CompileOptions = new ArrayList();
                CompileOptions.Add( new ComboBoxGameOption( ComboBox_ScriptConfig, "ScriptConfig", new string[] { "Release", "Debug", "Final Release" } ) );
                CompileOptions.Add( new CheckBoxGameOption( CheckBox_FullScriptCompile, GameOption.EOptionType.OT_CMDLINE, "FullCompile", " -full", "", false ) );
                CompileOptions.Add( new CheckBoxGameOption( CheckBox_AutomaticCompileMode, GameOption.EOptionType.OT_CMDLINE, "AutoCompileMode", " -auto -unattended -updateinisauto", "", false ) );
                CompileOptions.Add( new CheckBoxGameOption( CheckBox_QuietMode, GameOption.EOptionType.OT_CMDLINE, "QuietMode", " -silentbuild", "", true ) );

                PS3CompileOptions = new ArrayList();
                PS3CompileOptions.Add( new ComboBoxGameOption( ComboBox_SelectPS3Compile, "SelectPS3Compile", GameOption.EOptionType.OT_CUSTOM ) );
                PS3CompileOptions.Add( new CheckBoxGameOption( CheckBox_NoDebugInfo, GameOption.EOptionType.OT_CMDLINE, "NoDebugInfo", " NO_DEBUG_INFO=true", "", false ) );
                PS3CompileOptions.Add( new CheckBoxGameOption( CheckBox_LaunchAfterPS3Compile, GameOption.EOptionType.OT_CUSTOM, "LaunchAfterPS3Compile", "", "", true ) );
                PS3CompileOptions.Add( new CheckBoxGameOption( CheckBox_StopOnFirstError, GameOption.EOptionType.OT_CMDLINE, "StopOnFirstError", "", " -k", true ) );
                PS3CompileOptions.Add( new NumericUpDownGameOption( NumericUpDown_NumberOfThreads, GameOption.EOptionType.OT_INTEGER, "NumberOfThreads", 2 ) );
                PS3CompileOptions.Add( new ComboBoxGameOption( ComboBox_AddProfInfo, "PS3ProfileOptions", new string[] { "None", "PASuite" } ) );
                PS3CompileOptions.Add( new TextBoxGameOption( ModNameDialog.TextBox_String, "PS3ModName", GameOption.EOptionType.OT_CUSTOM ) );

                SPOptions = new ArrayList();
                SPOptions.Add( new ComboBoxGameOption( ComboBox_URLOptionsSP, "URLOptions", GameOption.EOptionType.OT_CUSTOM ) );
                SPOptions.Add( new CheckBoxGameOption( CheckBox_ShowLogSP, GameOption.EOptionType.OT_CMDLINE, "ShowLog", " -log", "", false ) );
                SPOptions.Add( new CheckBoxGameOption( CheckBox_SoundSP, GameOption.EOptionType.OT_CMDLINE, "EnableSound", "", " -nosound", true ) );
                SPOptions.Add( new CheckBoxGameOption( CheckBox_RCSP, GameOption.EOptionType.OT_CMDLINE, "RemoteControl", "", " -norc", false ) );
                SPOptions.Add( new CheckBoxGameOption( CheckBox_FixedSeedSP, GameOption.EOptionType.OT_CMDLINE, "FixedSeed", " -fixedseed", "", false ) );
                SPOptions.Add( new CheckBoxGameOption( CheckBox_UnattendedSP, GameOption.EOptionType.OT_CMDLINE, "Unattended", " -unattended", "", false ) );
                SPOptions.Add( new CheckBoxGameOption( CheckBox_VSyncSP, GameOption.EOptionType.OT_CMDLINE, "VSync", "", " -novsync", true ) );
                SPOptions.Add( new CheckBoxGameOption( CheckBox_MultiThreadSP, GameOption.EOptionType.OT_CMDLINE, "MultiThread", "", " -onethread", true ) );
                SPOptions.Add( new CheckBoxGameOption( CheckBox_UnlitSP, GameOption.EOptionType.OT_EXEC, "Unlit", "viewmode unlit", "", false ) );
                SPOptions.Add( new CheckBoxGameOption( CheckBox_PostProcessSP, GameOption.EOptionType.OT_EXEC, "PostProcess", "", "show postprocess", true ) );
                SPOptions.Add( new ResolutionGameOption( ComboBox_ResolutionSP, "Resolution", new string[] { "1280x720", "800x600", "640x480" } ) );
                SPOptions.Add( new TextBoxGameOption( TextBox_ExecSP, "Execs", GameOption.EOptionType.OT_EXEC ) );

                MPOptions = new ArrayList();
                MPOptions.Add( new ComboBoxGameOption( ComboBox_URLOptionsMP, "URLOptions", GameOption.EOptionType.OT_CUSTOM ) );
                MPOptions.Add( new CheckBoxGameOption( CheckBox_ShowLogMP, GameOption.EOptionType.OT_CMDLINE, "ShowLog", " -log", "", false ) );
                MPOptions.Add( new CheckBoxGameOption( CheckBox_SoundMP, GameOption.EOptionType.OT_CMDLINE, "EnableSound", "", " -nosound", true ) );
                MPOptions.Add( new CheckBoxGameOption( CheckBox_RCMP, GameOption.EOptionType.OT_CMDLINE, "RemoteControl", "", " -norc", false ) );
                MPOptions.Add( new CheckBoxGameOption( CheckBox_FixedSeedMP, GameOption.EOptionType.OT_CMDLINE, "FixedSeed", " -fixedseed", "", false ) );
                MPOptions.Add( new CheckBoxGameOption( CheckBox_UnattendedMP, GameOption.EOptionType.OT_CMDLINE, "Unattended", " -unattended", "", false ) );
                MPOptions.Add( new CheckBoxGameOption( CheckBox_VSyncMP, GameOption.EOptionType.OT_CMDLINE, "VSync", "", " -novsync", true ) );
                MPOptions.Add( new CheckBoxGameOption( CheckBox_MultiThreadMP, GameOption.EOptionType.OT_CMDLINE, "MultiThread", "", " -onethread", true ) );
                MPOptions.Add( new CheckBoxGameOption( CheckBox_UnlitMP, GameOption.EOptionType.OT_EXEC, "Unlit", "viewmode unlit", "", false ) );
                MPOptions.Add( new CheckBoxGameOption( CheckBox_PostProcessMP, GameOption.EOptionType.OT_EXEC, "PostProcess", "", "show postprocess", true ) );
                MPOptions.Add( new ResolutionGameOption( ComboBox_ResolutionMP, "Resolution", new string[] { "1280x720", "800x600", "640x480" } ) );
                MPOptions.Add( new TextBoxGameOption( TextBox_ExecMP, "Execs", GameOption.EOptionType.OT_EXEC ) );
            }
            return ( true );
        }

        private void UpdatePS3Misc()
        {
            bool bSupportsPS3 = false;
            bool bSupportsPS3Mod = false;
            foreach( string Platform in UsablePlatforms )
            {
                string Lower = Platform.ToLower();
                if(Lower == "ps3")
                {
                    bSupportsPS3 = true;
                }
                else if(Lower == "ps3 mod")
                {
                    bSupportsPS3Mod = true;
                }
            }

            if( bSupportsPS3 )
            {
                ComboBox_SelectPS3Compile_SelectionChangeCommitted( null, null );

				ModNameDialog.Title = "Mod name";
				ModNameDialog.Caption = "Please enter the name of this mod. Any mod with the same name will be overwritten on the PS3. Max 127 characters, no symbols or spaces.";
            }
            else
            {
                TabControl_Main.Controls.Remove(Tab_PS3);

                if(bSupportsPS3Mod)
                {
                    ModNameDialog.Title = "Mod name";
                    ModNameDialog.Caption = "Please enter the name of this mod. Any mod with the same name will be overwritten on the PS3. Max 127 characters, no symbols or spaces.";
                }
            }
        }

		public bool Init()
		{
			// initialize the helper cooker tools
			CookerTools = new CookerToolsClass(this);

			// Read the application settings and set up some game options
            if( !LoadCookerConfigAndMakeGameOptions() )
            {
                return ( false );
            }

			// build the options lists
			//@todo - assert unique option name and control?

			// attempt to load user options
			LoadOptions();

            // force the run from path to be the startup path initially
            ComboBox_ProjectDir.Text = Application.StartupPath + "\\";

			// remove old options that may still be saved in file
			ComboBox_AddProfInfo.Items.Remove("CPUProfile");
			ComboBox_AddProfInfo.Items.Remove("GPUProfile");
			ComboBox_AddProfInfo.Items.Remove("GPUDebug");
			ComboBox_AddProfInfo.Items.Remove("GCMReplay");

            // set the initial state of controls, without saving off
			UpdatePerGameControls("", GetOptionByName("GameName").GetCommandlineOption(), true);

			// setup the tray icon
			SetupTrayIcon();

			// kick off some updates
			ProcessGameNameChanged();
			ProcessPlatformChanged();

			// make sure the project directory exists, otherwise, crashes will ensue
			if (!Directory.Exists(ComboBox_ProjectDir.Text))
			{
				// if it doesn't exist, use the directory we ran from, as if there was no setting in the .ini file
				ComboBox_ProjectDir.Text = Application.StartupPath + "\\";
			}

			// set our current directory (needed by loaded DLLs)
			Directory.SetCurrentDirectory(ComboBox_ProjectDir.Text);

			// Create a Timer object.
			CommandletTimer = new System.Windows.Forms.Timer();
			CommandletTimer.Interval = 100;
			CommandletTimer.Tick += new System.EventHandler(this.OnTimer);

			int FrontendX = ((SimpleIntGameOption)GetOptionByName("FrontendX")).Value;
			int FrontendY = ((SimpleIntGameOption)GetOptionByName("FrontendY")).Value;
			int FrontendW = ((SimpleIntGameOption)GetOptionByName("FrontendW")).Value;
			int FrontendH = ((SimpleIntGameOption)GetOptionByName("FrontendH")).Value;
			// width of -1 is uninitialized
			if (FrontendW != -1)
			{
				StartPosition = FormStartPosition.Manual;

				Location = new Point(FrontendX, FrontendY);
				Width = FrontendW;
				Height = FrontendH;
			}

			// select the last selected tab (assuming its valid)
			int SelectedTab = ((SimpleIntGameOption)GetOptionByName("SelectedTab")).Value;
			if (SelectedTab < TabControl_Main.TabPages.Count)
			{
				TabControl_Main.SelectedIndex = SelectedTab;
			}

			Show();

			// set the window state (minimized, maximized, normal)
			FormWindowState SavedWindowState = (FormWindowState)((SimpleIntGameOption)GetOptionByName("WindowState")).Value;
			if (SavedWindowState != FormWindowState.Minimized)
			{
				WindowState = SavedWindowState;
			}

            UpdatePS3Misc();

			UpdateInfoText();
			
			Running = true;
			return (true);
		}

		#endregion

		#region Shutdown

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose(bool bDisposing)
		{
			this.bDisposing = bDisposing;

            if( Running )
            {
                // Stop the main loop running
                Running = false;

                // remember location (if not min or maximized)
                if( WindowState == FormWindowState.Normal )
                {
                    ( ( SimpleIntGameOption )GetOptionByName( "FrontendX" ) ).Value = Location.X;
                    ( ( SimpleIntGameOption )GetOptionByName( "FrontendY" ) ).Value = Location.Y;
                    ( ( SimpleIntGameOption )GetOptionByName( "FrontendW" ) ).Value = Width;
                    ( ( SimpleIntGameOption )GetOptionByName( "FrontendH" ) ).Value = Height;
                    ( ( SimpleIntGameOption )GetOptionByName( "SelectedTab" ) ).Value = TabControl_Main.SelectedIndex;
                }

                // remember state
                ( ( SimpleIntGameOption )GetOptionByName( "WindowState" ) ).Value = ( int )WindowState;

                // Store the current app state
                SaveOptions();
            }

			if (bDisposing)
			{
				if (components != null)
				{
					components.Dispose();
				}
			}
			base.Dispose(bDisposing);
		}

		private void CookFrontEnd_Closing(object sender, System.ComponentModel.CancelEventArgs e)
		{
			if (CommandletProcess != null)
			{
				if (!CheckedStopCommandlet())
				{
					// Don't exit!
					e.Cancel = true;
				}
			}

			// make sure the named pipe is no longer in use by blocking until its accessible
			if (LogPipe != null)
			{
				Monitor.Enter(LogPipe);
				Monitor.Exit(LogPipe);
			}
		}
		
		#endregion

		#region Process running helpers (PC/console)

		/// <summary>
		/// Calculate the name of the PC executable for commandlets and PC game
		/// </summary>
		/// <returns></returns>
		private string GetExecutablePath()
		{
			// Figure out executable path.
			string Executable = this.ComboBox_ProjectDir.Text;
            string Selected = ComboBox_PCConfiguration.SelectedItem.ToString();

            if( Selected == "Debug" )
			{
				Executable += "Debug-" + ComboBox_Game.Text + ".exe";
			}
            else if( Selected == "Debug-G4WLive" )
			{
				Executable += "Debug-" + ComboBox_Game.Text + "-G4WLive.exe";
			}
            else if( Selected == "Release-G4WLive" )
            {
                Executable += ComboBox_Game.Text + "-G4WLive.exe";
            }
            else
            {
                // Gets Release and ReleaseLTCG
                Executable += ComboBox_Game.Text + ".exe";
            }

			return Executable;
		}

		/// <summary>
		/// Start up the given executable
		/// </summary>
		/// <param name="Executable">The string name of the executable to run.</param>
		/// <param name="CommandLIne">Any command line parameters to pass to the program.</param>
		/// <param name="bOutputToTextBox">Specifying true will output </param>
		/// <returns>The running process if successful, null on failure</returns>
		private Process ExecuteProgram(string Executable, string CommandLine, bool bOutputToTextBox)
		{
			ProcessStartInfo StartInfo = new ProcessStartInfo();
			// Prepare a ProcessStart structure 
			StartInfo.FileName = Executable;
			StartInfo.WorkingDirectory = this.ComboBox_ProjectDir.Text;
			StartInfo.Arguments = CommandLine;

			Process NewProcess = null;
			if (bOutputToTextBox)
			{
				// Redirect the output.
				StartInfo.UseShellExecute = false;
				StartInfo.RedirectStandardOutput = true;
				StartInfo.RedirectStandardError = true;
				StartInfo.CreateNoWindow = true;

				AddLine("Running: " + Executable + " " + CommandLine, Color.OrangeRed);
			}

			// Spawn the process
			// Try to start the process, handling thrown exceptions as a failure.
			try
			{
				NewProcess = Process.Start(StartInfo);
            }
			catch
			{
				NewProcess = null;
			}

			return NewProcess;
		}

		/// <summary>
		/// This runs the PC game for non-commandlets (editor, game, etc)
		/// </summary>
		/// <param name="InitialCmdLine"></param>
		/// <param name="PostCmdLine"></param>
		/// <param name="ConfigType"></param>
		private void LaunchApp(string InitialCmdLine, string PostCmdLine, ECurrentConfig ConfigType, bool bForcePC)
		{
			// put together the final commandline (and generate exec file
			string CmdLine = GetFinalURL(InitialCmdLine, PostCmdLine, ConfigType, true);

			if (bForcePC || ComboBox_Platform.Text == "PC")
			{
				Process NewProcess = ExecuteProgram(GetExecutablePath(), CmdLine, false);
				if (NewProcess != null)
				{
					Processes.Add(NewProcess);
				}
				else
				{
					MessageBox.Show("Failed to launch game executable (" + GetExecutablePath() + ")", "Failed to launch", MessageBoxButtons.OK, MessageBoxIcon.Error);
				}
			}
			else
			{
				CheckedReboot(true, CmdLine);
			}
		}

		/// <summary>
		/// Reboot and optionally run the game on a console
		/// </summary>
		/// <param name="bShouldRunGame"></param>
		/// <param name="CommandLine"></param>
		private void CheckedReboot(bool bShouldRunGame, string CommandLine)
		{
			CacheGameNameAndConsoles();

			AddLine("\r\n[REBOOTING]\r\n", Color.Green);

            // Debug/Release etc
            string Configuration = ComboBox_ConsoleConfiguration.SelectedItem.ToString();

            // Adjust for the special PS3 parameters
            if( ComboBox_Platform.SelectedItem.ToString() == "PS3" )
            {
                if( ComboBox_AddProfInfo.SelectedItem.ToString() != "None" )
                {
                    Configuration += "_" + ComboBox_AddProfInfo.SelectedItem.ToString();
                }
            }

            if( CookerTools.TargetReboot( bShouldRunGame, Configuration, CommandLine ) == false )
			{
				MessageBox.Show(
					this,
					"Failed to reboot console. Check log for details.",
					"Reboot failed",
					MessageBoxButtons.OK,
					MessageBoxIcon.Error);
			}
		}

		#endregion

		#region Load/save application settings
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
		/// Converts the XML file into an object
		/// </summary>
		/// <returns>The new object settings, or null if the file is missing</returns>
		private CookerSettings ReadCookerSettings()
		{
			Stream XmlStream = null;
			CookerSettings CookerCfg = null;
			try
			{
				// Get the XML data stream to read from
				XmlStream = new FileStream("UnrealFrontend_cfg.xml", FileMode.Open, FileAccess.Read, FileShare.None, 256 * 1024, false);
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

		#endregion

		#region Save/load user options
		private void LoadOptions()
		{
			// get user settings directory

			if (File.Exists(SettingsPath + "\\" + SettingsFilename))
			{
				StreamReader Reader = File.OpenText(SettingsPath + "\\" + SettingsFilename);
				if (Reader != null)
				{
					ArrayList ConfigOptions = Options;
					Regex SectionRegex = new Regex("^\\[(\\w+)\\]");
					Regex OptionRegex = new Regex("^(\\w+)=(.+)");
					string Line;
					// set if we got a bad section - skip the rest of it
					bool bSkippingSection = false;
					// try to parse each line from the file
					while ((Line = Reader.ReadLine()) != null)
					{
						Match SectionMatch = SectionRegex.Match(Line);
						if (SectionMatch.Success)
						{
							// we are no longer skipping until the next section
							bSkippingSection = false;

							string SectionName = SectionMatch.Groups[1].Value.ToString();
							if (SectionName == "SP")
							{
								ConfigOptions = SPOptions;
							}
							else if (SectionName == "MP")
							{
								ConfigOptions = MPOptions;
							}
							else if (SectionName == "Compile")
							{
								ConfigOptions = CompileOptions;
							}
							else if (SectionName == "Cooking")
							{
								ConfigOptions = CookingOptions;
							}
                            else if( SectionName == "PS3Compile" )
                            {
                                ConfigOptions = PS3CompileOptions;
                            }
                            else if( SectionName.StartsWith( "Game_" ) )
							{
								string GameName = SectionName.TrimStart("Game_".ToCharArray());
								if (PerGameOptions.TryGetValue(GameName, out ConfigOptions) == false)
								{
									bSkippingSection = true;
								}
							}
							else
							{
								ConfigOptions = Options;
							}
							// skip to next line
							continue;
						}

						if (!bSkippingSection)
						{
							Match OptionMatch = OptionRegex.Match(Line);
							// if there was a match
							if (OptionMatch.Success)
							{
								// read the name/value
								string OptionName = OptionMatch.Groups[1].Value;
								string OptionValue = OptionMatch.Groups[2].Value;
								// search for the option in the correct list based on the current section
								foreach (GameOption Option in ConfigOptions)
								{
									if (Option.OptionName == OptionName)
									{
										// and try to apply it to an option
										Option.LoadFromString(OptionValue);
										Option.SaveValue();
										break;
									}
								}
								// skip to next line
								continue;
							}
						}
						// else skip over the line
					}
					// all done, close the reader
					Reader.Close();
				}
				// else file is locked? not sure if we care
			}
			// else file doesn't exist, no problem for loading
		}

		// write out all options in a given set of options
		private void SaveOptionGroup(StreamWriter Writer, string GroupName, ArrayList Options)
		{
			Writer.WriteLine("");
			Writer.WriteLine("[" + GroupName + "]");
			foreach (GameOption Option in Options)
			{
				// grab from option
				string OptionValue;
				Option.WriteToString(out OptionValue);
				// write the value out to the file
				Writer.WriteLine(Option.OptionName + "=" + OptionValue);
			}
		}

		private void SaveOptions()
		{
			StreamWriter Writer = File.CreateText(SettingsPath + "\\" + SettingsFilename);
			if (Writer != null)
			{
				// global options first
				SaveOptionGroup(Writer, "Global", Options);
				// singleplayer
				SaveOptionGroup(Writer, "SP", SPOptions);
				// multiplayer
				SaveOptionGroup(Writer, "MP", MPOptions);
				// script compiling
				SaveOptionGroup(Writer, "Compile", CompileOptions);
				// cooking
				SaveOptionGroup(Writer, "Cooking", CookingOptions);
                // script compiling
                SaveOptionGroup( Writer, "PS3Compile", PS3CompileOptions );

				// remember old gamename
				string PrevGameName = CachedGameName;
				// per-game options
				foreach (string Game in PerGameOptions.Keys)
				{
					UpdatePerGameControls(PrevGameName, Game, false);
					SaveOptionGroup(Writer, "Game_" + Game, PerGameOptions[Game]);
					PrevGameName = Game;
				}

				// restore controls to before
				UpdatePerGameControls(PrevGameName, CachedGameName, false);

				// all done, close the file
				Writer.Close();
			}
			// else failed to create writer, file locked or something
		}
		#endregion

		#endregion

		#region Misc

		#region Log window helpers.

		// The proper way to update UI in C#/.NET is to use to all UI updates on the render thread 
		// which is done using InvokeRequired & Invoke to achieve.
		delegate void DelegateAddLine(string Line, Color TextColor);
		delegate void DelegateClear();

		void LogClear()
		{
			if (InvokeRequired)
			{
				Invoke(new DelegateClear(LogClear), new object[0]);
				return;
			}
			RichTextBox_LogWindow.Clear();
		}

		void AddLine(string Line, Color TextColor)
		{
			if (Line == null)
			{
				return;
			}

			// if we need to, invoke the delegate
			if (InvokeRequired)
			{
				Invoke(new DelegateAddLine(AddLine), new object[] { Line, TextColor });
				return;
			}

			RichTextBox_LogWindow.Focus();

			// Reset the initial selection length to zero
			RichTextBox_LogWindow.SelectionLength = 0;

			int Count = 0;
			// Setting the value of SelectionStart has a random chance of throwing a null reference exception.
			// However it still seems to set the actual value of the selection start internally, so we check to see
			// if it (appears) to have succeeded and continue on. If not, try a few times. If that still fails then
			// rethrow the exception
			while (Count <= 3 && RichTextBox_LogWindow.SelectionStart != RichTextBox_LogWindow.TextLength)
			{
				Count++;
				try
				{
					RichTextBox_LogWindow.SelectionStart = RichTextBox_LogWindow.TextLength;
				}
				catch (System.Exception e)
				{
					if (Count == 3)
					{
						Debug.WriteLine(e.Message);
						Debug.WriteLine(e.StackTrace);
						throw;
					}
				}
			};

			// Only set the color if it is different than the foreground colour
			if (DefaultLogWindowForegroundColor != TextColor)
			{
				RichTextBox_LogWindow.SelectionColor = TextColor;
			}

			RichTextBox_LogWindow.AppendText(Line + "\r\n");
		}

		// IOutputHandler interface
		void CookerTools.IOutputHandler.OutputText(string Text)
		{
			// Use the presaved forecolor
			AddLine(Text, DefaultLogWindowForegroundColor);
		}

		void CookerTools.IOutputHandler.OutputText(string Text, Color OutputColor)
		{
			AddLine(Text, OutputColor);
		}

		/**
		 * Performs output gathering on another thread
		 */
		private void PollForOutput()
		{
			// we need a process to poll
			if (CommandletProcess == null)
			{
				return;
			}

			// take control of the named pipe object
			Monitor.Enter(LogPipe);

			while (CommandletProcess != null)
			{
				try
				{
					string StdOutLine = LogPipe.Read();

					// share the LogTextArray with main thread
					Monitor.Enter(LogTextArray);

					// Add that line to the output array in a thread-safe way
					if (StdOutLine.Length > 0)
					{
						LogTextArray.Add(StdOutLine.Replace("\r\n", ""));
					}
				}
				// Display the exception information that occurs to help debug frontend problems
				catch (ThreadAbortException ExceptionInfo)
				{
					LogTextArray.Add("Error: UnrealFrontend Exception: " + ExceptionInfo.ToString());
				}
				catch (Exception ExceptionInfo)
				{
					LogTextArray.Add("Error: UnrealFrontend Exception: " + ExceptionInfo.ToString());
					throw;	// Rethrow the last error to be consistent with the previous behaviour.
				}
				finally
				{
					Monitor.Exit(LogTextArray);
				}
			}

			LogPipe.Disconnect();
			LogPolling = null;

			Monitor.Exit(LogPipe);
		}

		int GetTicked()
		{
			int Ticked = TickCount - LastTickCount;
			LastTickCount = TickCount;
			return (Ticked);
		}

		private void OnTimer(object sender, System.EventArgs e)
		{
			TickCount++;
		}
		#endregion

		#region Commandlet functionality

		/// <summary>
		/// Handle commandlet process (output, commandlet exited, etc)
		/// </summary>
		public void RunFrame()
		{
			// nothing to do if we aren't running a commandlet
			if (CommandletProcess == null)
			{
				return;
			}

			if (!CommandletProcess.HasExited)
			{
				// Lock the array for thread safety.
				Monitor.Enter(LogTextArray);

				// Iterate over all of the strings in the array.
				foreach (string Line in LogTextArray)
				{
					// Use DefaultLogWindowForegroundColor
					Color TextColor = DefaultLogWindowForegroundColor;

					// Figure out which color to use for line.
					if (Line.StartsWith(UNI_COLOR_MAGIC))
					{
						// Ignore any special console colours
						continue;
					}
					else if (Line.StartsWith("Warning"))
					{
						TextColor = Color.Orange;
					}
					else if (Line.StartsWith("Error"))
					{
						TextColor = Color.Red;
					}

					// Add the line to the log window with the appropriate color.
					AddLine(Line, TextColor);
				}

				// Empty the array and release the lock
				LogTextArray.Clear();
				Monitor.Exit(LogTextArray);
			}
			else
			{
				// save the commandlet type, as StopCommandlet will reset it
				ECommandletType CommandletType = RunningCommandletType;

				StopCommandlet();
				AddLine("\r\n[COMMANDLET FINISHED]\r\n", Color.Green);

				// when the cooker finishes naturally (not killed), do some automatic things (copy, run, etc)
				if (CommandletType == ECommandletType.CT_Cook)
				{
					// only copy if the platform allows it
					if (Button_CopyToTargets.Enabled)
					{
						// Sync up PC/console.
						SyncWithRetry();
					}

					// handle cooking in user mode for PS3 (packaging it up)
					if (ComboBox_Platform.Text == "PS3 Mod" || (CheckBox_CookInUserMode.Checked && ComboBox_Platform.SelectedItem.ToString() == "PS3"))
					{
						// make list of files in cooked directory
						string CookedPath = SettingsPath + "\\" + ComboBox_Game.Text + "\\Mods\\PS3\\" + ModNameDialog.String;
						DirectoryInfo CookedDirectory = new DirectoryInfo(CookedPath);
						FileInfo[] Files = CookedDirectory.GetFiles();

						// Convert FileInfo array to string array
						string[] FileNames = new string[Files.Length];
						for (int FileIndex = 0; FileIndex < Files.Length; FileIndex++)
						{
							FileNames[FileIndex] = Files[FileIndex].FullName;
						}

						PackageMod(FileNames, ModNameDialog.String);
					}

					// if the user wants to (and the platform allows it), run the game after cooking is finished
					if (CheckBox_LaunchAfterCooking.Checked)
					{
						OnLoadMapClick(null, null);
					}
				}
			}
		}
		
		private void CleanupXMAWorkFiles()
		{
			try
			{
				// Get temporary files left behind by XMA encoder and nuke them.
				string[] TempFiles = Directory.GetFiles(Path.GetTempPath(), "EncStrm*");
				foreach (string TempFile in TempFiles)
				{
					File.Delete(TempFile);
				}
			}
			catch (Exception Error)
			{
				AddLine(Error.ToString(), Color.Red);
			}
		}

		private void SetCommandletState(bool bIsStarting)
		{
			if (bIsStarting)
			{
				// Disable some run buttons while we are running.
				Button_CopyToTargets.Enabled = false;
				Button_Compile.Enabled = false;
				Button_StartCooking.Enabled = false;
				Button_StartCooking.Enabled = false;
				TabControl_Main.TabPages[0].Enabled = false;
				TabControl_Main.TabPages[1].Enabled = false;
				ToolStripButton_Launch.Enabled = false;
				ToolStripButton_Editor.Enabled = false;
				ToolStripButton_Server.Enabled = false;
				ToolStripButton_Cook.Enabled = false;
				ToolStripButton_CompileScript.Enabled = false;
				ToolStripButton_Sync.Enabled = false;

				switch (RunningCommandletType)
				{
					case ECommandletType.CT_Compile:
						Button_Compile.Enabled = true;
						Button_Compile.Text = "Stop Compile";
						ToolStripButton_CompileScript.Enabled = true;
						ToolStripButton_CompileScript.Text = "Stop";
						break;

					case ECommandletType.CT_Cook:
						Button_StartCooking.Enabled = true;
						Button_StartCooking.Text = "Stop Cooking";
						ToolStripButton_Cook.Enabled = true;
						ToolStripButton_Cook.Text = "Stop";
						break;
				}

				// Kick off the thread that monitors the commandlet's ouput
				ThreadStart ThreadDelegate = new ThreadStart(PollForOutput);
				LogPolling = new Thread(ThreadDelegate);
				LogPolling.Start();

				// start the timer to read output
				CommandletTimer.Start();
			}
			else
			{
				Button_CopyToTargets.Enabled = true;
				Button_Compile.Enabled = true;
				Button_StartCooking.Enabled = true;
				TabControl_Main.TabPages[0].Enabled = true;
				TabControl_Main.TabPages[1].Enabled = true;
				ToolStripButton_Launch.Enabled = true;
				ToolStripButton_Editor.Enabled = true;
				ToolStripButton_Server.Enabled = true;
				ToolStripButton_Cook.Enabled = true;
				ToolStripButton_CompileScript.Enabled = true;
				ToolStripButton_Sync.Enabled = true;

				switch (RunningCommandletType)
				{
					case ECommandletType.CT_Compile:
						Button_Compile.Text = "Compile Script";
						ToolStripButton_CompileScript.Text = "Make";
						break;

					case ECommandletType.CT_Cook:
						Button_StartCooking.Text = "Cook";
						ToolStripButton_Cook.Text = "Cook";
						break;
				}

				// Stop the timer now.
				CommandletTimer.Stop();

				RunningCommandletType = ECommandletType.CT_None;
			}
		}

		/**
		 * Spawn a copy of the commandlet and trap the output
		 */
		private void StartCommandlet(ECommandletType CommandletType)
		{
			StartCommandlet(CommandletType, GetExecutablePath());
		}

		/**
		 * Spawn a copy of the commandlet and trap the output
		 */
		private void StartCommandlet(ECommandletType CommandletType, string ExecutablePath)
		{
			// Use the threadsafe version of clear
			LogClear();

			// @todo: Move this into XeTools.dll, possibly?
			CleanupXMAWorkFiles();

			string CommandLine = "";

			// do per-commandlet stuff here (like figure out command line)
			switch (CommandletType)
			{
				case ECommandletType.CT_Cook:
					// perform some cooking-only behavior
					CacheGameNameAndConsoles();

					CommandLine = GetCookingCommandLine();
					break;

				case ECommandletType.CT_Compile:
					CommandLine = GetCompileCommandLine();
					break;
			}

			// Launch the cooker.
			CommandletProcess = ExecuteProgram(ExecutablePath, CommandLine, true);

			// Launch successful.
			if (CommandletProcess != null)
			{
				AddLine("[COMMANDLET STARTED]\r\n", Color.Green);

				// immediately connect the named pipe
				LogPipe = new NamedPipe();
				
				// connect it to the commandlet
				LogPipe.Connect(CommandletProcess);

				// cache the commandlet type
				RunningCommandletType = CommandletType;

				// this will start the pipe-monitoring thread, which will take over ownership of the LogPipe
				SetCommandletState(true);
			}
			else
			{
				// Failed to launch executable.
				MessageBox.Show(
					this,
					"Failed to launch commandlet (" + ExecutablePath + "). Check your settings and try again.",
					"Launch Failed",
					MessageBoxButtons.OK,
					MessageBoxIcon.Error);
			}
		}

		private void StopCommandlet()
		{
			// force stop the commandlet if needed
			if (CommandletProcess != null && !CommandletProcess.HasExited)
			{
				// Need to null this in a threadsafe way
				Monitor.Enter(CommandletProcess);
				// Since we entered it means the thread is done with it
				Monitor.Exit(CommandletProcess);

				CommandletProcess.Kill();
				CommandletProcess.WaitForExit();

			}
			CommandletProcess = null;

			// note that we are stopping the commandlet
			SetCommandletState(false);
		}

		private bool CheckedStopCommandlet()
		{
			DialogResult result = MessageBox.Show("Are you sure you want to cancel?", "Stop commandlet", MessageBoxButtons.YesNo, MessageBoxIcon.Question);
			if (result == DialogResult.Yes)
			{
				StopCommandlet();
				AddLine("\r\n[COMMANDLET ABORTED]\r\n", Color.Green);
				return (true);
			}

			return (false);
		}


		#endregion

		#region Misc helper functions

		private GameOption GetOptionByName(string OptionName)
		{
			foreach (GameOption Option in Options)
			{
				if (Option.OptionName == OptionName)
				{
					return Option;
				}
			}
			return null;
		}

		private GameOption GetSPOptionByName(string OptionName)
		{
			foreach (GameOption Option in SPOptions)
			{
				if (Option.OptionName == OptionName)
				{
					return Option;
				}
			}
			return null;
		}

		private GameOption GetMPOptionByName(string OptionName)
		{
			foreach (GameOption Option in MPOptions)
			{
				if (Option.OptionName == OptionName)
				{
					return Option;
				}
			}
			return null;
		}

		private GameOption GetCompileOptionByName(string OptionName)
		{
			foreach (GameOption Option in CompileOptions)
			{
				if (Option.OptionName == OptionName)
				{
					return Option;
				}
			}
			return null;
		}

		private void GetMPResolution(out int ResX, out int ResY)
		{
			ResX = 640;
			ResY = 480;
			Regex ResolutionRegex = new Regex("(\\d+)x(\\d+)");
			Match ResolutionMatch = ResolutionRegex.Match(this.ComboBox_ResolutionMP.Text);
			if (ResolutionMatch.Success)
			{
				ResX = Convert.ToInt32(ResolutionMatch.Groups[1].Value);
				ResY = Convert.ToInt32(ResolutionMatch.Groups[2].Value);
			}
		}

		private void CreateTempExec(ArrayList ConfigOptions)
		{
			StreamWriter Writer;
			if (ComboBox_Platform.Text == "PC")
			{
				Writer = File.CreateText(SettingsPath + "\\Binaries\\UnrealFrontend_TmpExec.txt");
			}
			else
			{
				Writer = File.CreateText(ComboBox_ProjectDir.Text + "UnrealFrontend_TmpExec.txt");
			}
			if (Writer != null)
			{
				foreach (GameOption Option in ConfigOptions)
				{
					if (Option.Type == GameOption.EOptionType.OT_EXEC)
					{
						Option.WriteExecOptions(Writer);
					}
				}
				Writer.Close();
			}
		}

		/// <summary>
		/// Go through the per-game options, saving current controls settings to options, and
		/// setting controls with new values from options for that new game
		/// </summary>
		private void UpdatePerGameControls(string OldGameName, string NewGameName, bool bRestoreOnly)
		{
            if (PerGameOptions != null)
			{
				if (!bRestoreOnly)
				{
                    if (PerGameOptions.ContainsKey(OldGameName))
                    {
                        // save off control value for "exiting" options
                        foreach (GameOption Option in PerGameOptions[OldGameName])
                        {
                            // update option with control value
                            Option.SaveValue();
                        }
                    }
				}

				// set control to "entering" options
                if (PerGameOptions.ContainsKey(NewGameName))
                {
                    foreach (GameOption Option in PerGameOptions[NewGameName])
                    {
                        // update option with control value
                        Option.RestoreValue();
                    }
                }
			}
		}

		private bool BrowseForMap(ComboBox MapBox, out string MapName)
		{
			OpenFileDialog FileDlg = new OpenFileDialog();
			FileDlg.InitialDirectory = this.ComboBox_ProjectDir.Text + "..\\" + ComboBox_Game.Text + "\\Content\\Maps";
			FileDlg.Filter = "Map Files (*.war;*.umap;*.ut3;*.gear)|*.war;*.umap;*.ut3;*.gear";
			FileDlg.RestoreDirectory = true;
			if (FileDlg.ShowDialog() == DialogResult.OK)
			{
				MapName = Path.GetFileNameWithoutExtension(FileDlg.FileName);
				if (!MapBox.Items.Contains(MapName))
				{
					MapBox.Items.Add(MapName);
				}
				return true;
			}
			MapName = "";
			return false;
		}

		private void ParseURLOptions(ECurrentConfig Config, out string GameOptions, out string EngineOptions)
		{
			// get which URL to parse
			string URL = (Config == ECurrentConfig.MP) ? 
				GetMPOptionByName("URLOptions").GetCommandlineOption() : 
				GetSPOptionByName("URLOptions").GetCommandlineOption();

			// split up the options
			string[] Options = URL.Split(' ');

			// empty out both outputs
			GameOptions = "";
			EngineOptions = "";
			foreach (string Option in Options)
			{
				if (Option.Length > 0)
				{
					// put all ? options together
					if (Option[0] == '?')
					{
						GameOptions += Option;
					}
					// put all - options together
					else if (Option[0] == '-')
					{
						EngineOptions += " " + Option;
					}
					// ignore anything without ? or -
				}
			}
		}

		/// <summary>
		/// Generate a final URL to pass to the game
		/// </summary>
		/// <param name="GameOptions">Game type options (? options)</param>
		/// <param name="PostCmdLine">Engine type options (- options)</param>
		/// <param name="ConfigType">SP, MP or neither</param>
		/// <param name="bCreateExecFile"></param>
		/// <returns></returns>
		private string GetFinalURL(string GameOptions, string EngineOptions, ECurrentConfig ConfigType, bool bCreateExecFile)
		{
			ArrayList ConfigOptions = null;
			if (ConfigType == ECurrentConfig.SP)
			{
				ConfigOptions = SPOptions;
			}
			else if (ConfigType == ECurrentConfig.MP)
			{
				ConfigOptions = MPOptions;
			}

			// build the commandline
			string CmdLine = GameOptions;
			// first add any url options
			if (ConfigOptions != null)
			{
				foreach (GameOption Option in ConfigOptions)
				{
					if (Option.Type == GameOption.EOptionType.OT_URL)
					{
						CmdLine += Option.GetCommandlineOption();
					}
				}
				// second pass for normal cmdline options
				foreach (GameOption Option in ConfigOptions)
				{
					if (Option.Type == GameOption.EOptionType.OT_CMDLINE)
					{
						CmdLine += Option.GetCommandlineOption();
					}
				}
			}

			// tack on exec and other engine options
			CmdLine += EngineOptions;
			CmdLine += " -Exec=UnrealFrontend_TmpExec.txt";

			// final pass for execs
			if (bCreateExecFile && ConfigOptions != null)
			{
				CreateTempExec(ConfigOptions);
			}

			return CmdLine;
		}

		/// <summary>
		/// Callback function when URL options change to update the info status bar at the bottom
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void OnGenericControlValueChanged(object sender, EventArgs e)
		{
			UpdateInfoText();
		}

		/// <summary>
		/// Generically handle adding new items to a combo box for MRU list of items
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void OnGenericComboBoxLeave(object sender, EventArgs e)
		{
			ComboBox ComboBox = sender as ComboBox;
			if (ComboBox != null)
			{
				// if the text isn't already in the list, add it to the top of the list
				if (!ComboBox.Items.Contains(ComboBox.Text))
				{
					ComboBox.Items.Insert(0, ComboBox.Text);
				}

				// max out at 15 items
				if (ComboBox.Items.Count == 16)
				{
					ComboBox.Items.RemoveAt(15);
				}
			}
		}

		#endregion

		#region Tray icon

		private MenuItem MenuItem_LoadMap, MenuItem_ServerListen, MenuItem_ServerDedicated;

		private void AddComboBoxMirror(Menu ParentMenu, string MenuName, ComboBox SourceComboBox)
		{
			// make a new menu for the combo box mirror
			MenuItem NewMenuGroup = new MenuItem(MenuName);

			// add it to the parent menu
			ParentMenu.MenuItems.Add(NewMenuGroup);

			// add the combo box items
			foreach (string Item in SourceComboBox.Items)
			{
				// crete new submenu
				MenuItem NewMenu = new MenuItem(Item, new System.EventHandler(this.GenericComboBoxTraySelect));

				// store the source combo box for the handler to use to set data
				NewMenu.Tag = SourceComboBox;

				// add the menu to the menu group
				NewMenuGroup.MenuItems.Add(NewMenu);
			}
		}

		private void AddButtonMirror(Menu ParentMenu, Button SourceButton, EventHandler EH)
		{
			MenuItem NewMenuItem = new MenuItem(SourceButton.Text, new System.EventHandler(EH));
			NewMenuItem.Tag = SourceButton;
			ParentMenu.MenuItems.Add(NewMenuItem);
		}

		private void SetupTrayIcon()
		{
			// need components for the icon apparently
			this.components = new System.ComponentModel.Container();
			// create the tray icon
			TrayIcon = new NotifyIcon(this.components);
			TrayIcon.Visible = false;
			TrayIcon.Text = VersionStr;
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(GetType());
			TrayIcon.Icon = this.Icon;
			// create the context menu for the tray icon
			TrayMenu = new ContextMenu();

			// local options
			MenuItem LocalMenu = new MenuItem("Local");
			MenuItem_LoadMap = new MenuItem("Load Map");
			LocalMenu.MenuItems.Add(MenuItem_LoadMap);
			AddButtonMirror(LocalMenu, Button_Editor, OnEditorClick);
			TrayMenu.MenuItems.Add(LocalMenu);
			
			// multiplayer options
			MenuItem MultiplayerMenu = new MenuItem("Multiplayer");
			MenuItem_ServerListen = new MenuItem("Server (Listen)");
			MultiplayerMenu.MenuItems.Add(MenuItem_ServerListen);
			MenuItem_ServerDedicated = new MenuItem("Server (Dedicated)");
			MultiplayerMenu.MenuItems.Add(MenuItem_ServerDedicated);
			AddButtonMirror(MultiplayerMenu, Button_AddClient, OnClientClick);
			AddButtonMirror(MultiplayerMenu, Button_ServerClient, OnServerClientClick);
			AddButtonMirror(MultiplayerMenu, Button_KillAll, OnKillAllClick);
			TrayMenu.MenuItems.Add(MultiplayerMenu);

			// cooking options
//			MenuItem CookingMenu = new MenuItem("Console");
			AddButtonMirror(TrayMenu, Button_StartCooking, OnStartCookingClick);
//			TrayMenu.MenuItems.Add(ConsoleMenu);

			// pc options
			MenuItem PCMenu = new MenuItem("PC");
			AddComboBoxMirror(PCMenu, "Config", ComboBox_PCConfiguration);
			AddButtonMirror(PCMenu, Button_Compile, OnCompileClick);
			TrayMenu.MenuItems.Add(PCMenu);

			// console options
			MenuItem ConsoleMenu = new MenuItem("Console");
			AddComboBoxMirror(ConsoleMenu, "Config", ComboBox_ConsoleConfiguration);
			AddButtonMirror(ConsoleMenu, Button_RebootTargets, OnRebootTargetsClick);
			AddButtonMirror(ConsoleMenu, Button_CopyToTargets, OnCopyToTargetsClick);
			TrayMenu.MenuItems.Add(ConsoleMenu);
			
			// global options
			MenuItem GlobalMenu = new MenuItem("Global");
			AddComboBoxMirror(GlobalMenu, "Game", ComboBox_Game.ComboBox);
			AddComboBoxMirror(GlobalMenu, "Platform", ComboBox_Platform.ComboBox);
			TrayMenu.MenuItems.Add(GlobalMenu);


			// exit option
			TrayMenu.MenuItems.Add(new MenuItem("Exit", new System.EventHandler(this.OnExitClick)));

			// assign the menu to the icon
			TrayIcon.ContextMenu = TrayMenu;
			TrayIcon.DoubleClick += new System.EventHandler(this.OnTrayIconDoubleClick);
			// update the maps
			UpdateMaps();
		}

		// Refreshes the maps with the contents of the combo boxes
		private void UpdateMaps()
		{
			MenuItem MapItem = null;
			if (MenuItem_LoadMap != null)
			{
				MenuItem_LoadMap.MenuItems.Clear();
				// add the current text if not already in items
				if (!this.ComboBox_MapnameSP.Items.Contains(this.ComboBox_MapnameSP.Text))
				{
					MapItem = new MenuItem(this.ComboBox_MapnameSP.Text);
					MapItem.Click += new System.EventHandler(this.OnLoadMapClick);
					MenuItem_LoadMap.MenuItems.Add(MapItem);
				}
				foreach (string Item in this.ComboBox_MapnameSP.Items)
				{
					MapItem = new MenuItem(Item);
					MapItem.Click += new System.EventHandler(this.OnLoadMapClick);
					MenuItem_LoadMap.MenuItems.Add(MapItem);
				}
				// add the open file option
				MapItem = new MenuItem("Browse...");
				MapItem.Click += new System.EventHandler(this.OnLoadMapClick);
				MenuItem_LoadMap.MenuItems.Add(MapItem);
			}
			if (MenuItem_ServerListen != null && MenuItem_ServerDedicated != null)
			{
				MenuItem_ServerListen.MenuItems.Clear();
				MenuItem_ServerDedicated.MenuItems.Clear();
				if (!this.ComboBox_MapnameSP.Items.Contains(this.ComboBox_MapnameMP.Text))
				{
					MapItem = new MenuItem(this.ComboBox_MapnameMP.Text);
					MapItem.Click += new System.EventHandler(this.OnServerClick);
					MenuItem_ServerListen.MenuItems.Add(MapItem);
					MapItem = new MenuItem(this.ComboBox_MapnameMP.Text);
					MapItem.Click += new System.EventHandler(this.OnServerDedicatedClick);
					MenuItem_ServerDedicated.MenuItems.Add(MapItem);
				}
				foreach (string Item in this.ComboBox_MapnameMP.Items)
				{
					MapItem = new MenuItem(Item);
					MapItem.Click += new System.EventHandler(this.OnServerClick);
					MenuItem_ServerListen.MenuItems.Add(MapItem);
					MapItem = new MenuItem(Item);
					MapItem.Click += new System.EventHandler(this.OnServerDedicatedClick);
					MenuItem_ServerDedicated.MenuItems.Add(MapItem);
				}
				// add the open file option
				MapItem = new MenuItem("Browse...");
				MapItem.Click += new System.EventHandler(this.OnServerClick);
				MenuItem_ServerListen.MenuItems.Add(MapItem);
				// add the open file option
				MapItem = new MenuItem("Browse...");
				MapItem.Click += new System.EventHandler(this.OnServerDedicatedClick);
				MenuItem_ServerDedicated.MenuItems.Add(MapItem);
			}
		}

		private void UpdateMaps(object sender, EventArgs e)
		{
			UpdateMaps();
		}

		private void OnTrayIconDoubleClick(object sender, EventArgs e)
		{
			this.WindowState = FormWindowState.Normal;
		}

		private void OnSizeChanged(object sender, EventArgs e)
		{
			UpdateMaps();
			TrayIcon.Visible = (this.WindowState == FormWindowState.Minimized);
            this.ShowInTaskbar = true; // @todo: make it an ini setting? !TrayIcon.Visible;
		}

		private void OnExitClick(object sender, EventArgs e)
		{
			this.Close();
		}

		private void GenericComboBoxTraySelect(object sender, EventArgs e)
		{
			// the sender needs to be a menu item
			MenuItem MenuItem = (MenuItem)sender;

			// the tag must be a combo box
			ComboBox SourceComboBox = (ComboBox)MenuItem.Tag;

			// set the combobox to the menu item string
			SourceComboBox.Text = MenuItem.Text;
		}

		#endregion

		#region Key handling

		private void OnUnrealFrontendWindowKeyDown(object sender, KeyEventArgs e)
		{
			if (e.Modifiers == 0 && (int)e.KeyCode >= (int)Keys.F1 && (int)e.KeyCode <= (int)Keys.F12)
			{
				TabControl_Main.SelectedIndex = (int)e.KeyCode - (int)Keys.F1;
				e.Handled = true;
			}
			// manually handle keyboard shortcuts
			else if (e.Modifiers == Keys.Control)
			{
				// default to handled
				e.Handled = true;

				switch (e.KeyValue)
				{
					case 'L':
						if (Button_LoadMap.Enabled)
						{
							OnLoadMapClick(this, e);
						}
						break;

					case 'S':
						if (Button_Server.Enabled)
						{
							OnServerClick(this, e);
						}
						break;

					case 'E':
						if (Button_Editor.Enabled)
						{
							OnEditorClick(this, e);
						}
						break;

					case 'K':
						if (Button_StartCooking.Enabled)
						{
							OnStartCookingClick(this, e);
						}
						break;

					case 'M':
						if (Button_Compile.Enabled)
						{
							OnCompileClick(this, e);
						}
						break;

					case 'Y':
						if (Button_CopyToTargets.Enabled)
						{
							OnCopyToTargetsClick(this, e);
						}
						break;

					case 'R':
						if (Button_RebootTargets.Enabled)
						{
							OnRebootTargetsClick(this, e);
						}
						break;

					default:
						// actually, we didn't handle it
						e.Handled = false;
						break;
				}
			}
		}

		#endregion

		#region GoW specific

		private void OnUseChapterPointChanged(object sender, System.EventArgs e)
		{
			ChapterPointSelect.Enabled = false;
			if (UseChapterPoint.Checked)
			{
				ChapterPointSelect.Enabled = true;
			}
		}

		#endregion

		#endregion

		#region Controls

		#region Global controls

		private void ProcessGameNameChanged()
		{
			// fix up per-game controls based on game
			UpdatePerGameControls(CachedGameName, ComboBox_Game.Text, false);
			CachedGameName = ComboBox_Game.Text;

			if (CachedGameName == "WarGame")
			{
				OnUseChapterPointChanged(null, null);

				ChapterPointSelect.Show();
				UseChapterPoint.Show();
				ComboBox_MapnameSP.Hide();
				Button_BrowseSP.Hide();
				ComboBox_URLOptionsSP.Hide();
				CheckBox_UseCookedMap.Hide();
				ComboBox_MapnameSP.Text = "";
			}
			else
			{
				ChapterPointSelect.Hide();
				UseChapterPoint.Hide();
				ComboBox_MapnameSP.Show();
				Button_BrowseSP.Show();
				ComboBox_URLOptionsSP.Show();
				CheckBox_UseCookedMap.Show();
			}
		}
		
		private void OnGameNameChanged(object sender, System.EventArgs e)
		{
			ProcessGameNameChanged();

			// focus on something else so that the tool strip buttons will get the mouse-over highlights
			TabControl_Main.Focus();
		}

		private void ProcessPlatformChanged()
		{
			// remove existing entries
			ListBox_Targets.Items.Clear();

			//if the current platform does not exist create a dictionary for it
			if(!PlatformDefaults.ContainsKey(ComboBox_Platform.Text))
			{
				PlatformDefaults[ComboBox_Platform.Text] = new Dictionary<string, bool>();
			}

            if(ComboBox_Platform.Text == "PS3 Mod")
            {
                ToolStripButton_Sync.Enabled = false;
                ToolStripButton_Reboot.Enabled = false;
                ComboBox_PCConfiguration.Enabled = false;
                ComboBox_ProjectDir.Enabled = false;
                CheckBox_RunWithCookedData.Enabled = false;

                if(TabControl_Main.TabPages.Contains(Tab_ConsoleSetup))
                {
                    TabControl_Main.TabPages.Remove(Tab_ConsoleSetup);
                }
            }
            else
            {
                if(!TabControl_Main.TabPages.Contains(Tab_ConsoleSetup))
                {
                    TabControl_Main.TabPages.Insert(TabControl_Main.TabPages.IndexOf(Tab_PCSetup) + 1, Tab_ConsoleSetup);
                }

                ToolStripButton_Sync.Enabled = true;
                ToolStripButton_Reboot.Enabled = true;
                ComboBox_PCConfiguration.Enabled = true;
                ComboBox_ProjectDir.Enabled = true;
                CheckBox_RunWithCookedData.Enabled = true;
            }

			if (ComboBox_Platform.Text == "PC")
			{
				Button_CopyToTargets.Enabled = false;
				Button_RebootTargets.Enabled = false;
				Button_AddClient.Enabled = true;
				Button_ServerClient.Enabled = true;
				NumericUpDown_Clients.Enabled = true;
				Button_KillAll.Enabled = true;
				ToolStripButton_ServerAndClients.Enabled = true;
				ToolStripButton_AddClient.Enabled = true;
				ToolStripButton_KillAll.Enabled = true;
				ComboBox_ResolutionSP.Enabled = true;
				ComboBox_ResolutionMP.Enabled = true;
				CheckBox_RCMP.Enabled = true;
				CheckBox_RCSP.Enabled = true;
				CheckBox_ShowLogMP.Enabled = true;
				CheckBox_ShowLogSP.Enabled = true;
			}
			else
			{
				// Only add a list of targets if the CookerTools say it is OK
				if (CookerTools != null && CookerTools.Activate(ComboBox_Platform.Text))
				{
					//get the current platform's dictionary
					Dictionary<string, bool> CurrentPlatformDefaults = PlatformDefaults[ComboBox_Platform.Text];

					// Getlist of all known consoles
					ArrayList ConsoleList = CookerTools.GetKnownConsoles();

					// Initialize the consoles list
					for (int Index = 0; Index < ConsoleList.Count; Index++)
					{
						//if the target doesn't exist in the dictionary add it
						if(!CurrentPlatformDefaults.ContainsKey(ConsoleList[Index] as string))
						{
							CurrentPlatformDefaults[ConsoleList[Index] as string] = Index == 0;
						}

						// Check this item if it is the default
						ListBox_Targets.Items.Add(ConsoleList[Index], CurrentPlatformDefaults[ConsoleList[Index] as string]);
					}
				}

				// gray out the sync button if the platform doesnt need it
				Button_CopyToTargets.Enabled = CookerTools != null && CookerTools.PlatformNeedsToSync();
				Button_RebootTargets.Enabled = true;
				Button_AddClient.Enabled = false;
				Button_ServerClient.Enabled = false;
				NumericUpDown_Clients.Enabled = false;
				Button_KillAll.Enabled = false;
				ToolStripButton_ServerAndClients.Enabled = false;
				ToolStripButton_AddClient.Enabled = false;
				ToolStripButton_KillAll.Enabled = false;
				ComboBox_ResolutionSP.Enabled = false;
				ComboBox_ResolutionMP.Enabled = false;
				CheckBox_RCMP.Enabled = false;
				CheckBox_RCSP.Enabled = false;
				CheckBox_ShowLogMP.Enabled = false;
				CheckBox_ShowLogSP.Enabled = false;
			}

			UpdateInfoText();
		}

		private void OnPlatformChanged(object sender, System.EventArgs e)
		{
			ProcessPlatformChanged();

			// focus on something else so that the tool strip buttons will get the mouse-over highlights
			TabControl_Main.Focus();
		}

		/// <summary>
		/// Copies certain values from across all pages to the status bar so the user
		/// doesn't have to switch tabs just to see important info
		/// </summary>
		private void UpdateInfoText()
		{
			// set each status label accordingly
			ToolStripStatusLabel_PCConfig.Text = ComboBox_PCConfiguration.Text;
			ToolStripStatusLabel_ConsoleConfig.Text = ComboBox_ConsoleConfiguration.Text;
			ToolStripStatusLabel_LocalMap.Text = GetSelectedMapSP();
			if (ToolStripStatusLabel_LocalMap.Text == "")
			{
				ToolStripStatusLabel_LocalMap.Text = "<none>";
			}
			ToolStripStatusLabel_MPMap.Text = ComboBox_MapnameMP.Text;
			if (ToolStripStatusLabel_MPMap.Text == "")
			{
				ToolStripStatusLabel_MPMap.Text = "<none>";
			}
			ToolStripStatusLabel_CookMaps.Text = TextBox_MapsToCook.Text;
			if (ToolStripStatusLabel_CookMaps.Text == "")
			{
				ToolStripStatusLabel_CookMaps.Text = "<none>";
			}

			// get the single player URL
			switch (TabControl_Main.SelectedIndex)
			{
				case 0:
					if (SPOptions != null)
					{
						string GameOptions, EngineOptions;
						GetSPURLOptions(out GameOptions, out EngineOptions);
						ToolStripStatusLabel_URL.Text = GetFinalURL(GameOptions, EngineOptions, ECurrentConfig.SP, false);
					}
					break;

				case 1:
					if (MPOptions != null)
					{
						string GameOptions, EngineOptions;
						GetMPURLOptions(out GameOptions, out EngineOptions);
						ToolStripStatusLabel_URL.Text = GetFinalURL(GameOptions, EngineOptions, ECurrentConfig.MP, false);
					}
					break;

				case 2:
					ToolStripStatusLabel_URL.Text = GetCookingCommandLine();
					break;

				case 3:
					ToolStripStatusLabel_URL.Text = GetCompileCommandLine();
					break;

                case 5:
                    ToolStripStatusLabel_URL.Text = GetPS3CommandLine( CheckBox_FullRebuild.Checked );
                    break;

				default:
					ToolStripStatusLabel_URL.Text = "";
					break;
			}
		}

		#endregion

		#region Local tab

		/// <summary>
		/// Returns the map to use for SP game/editor based on user options
		/// </summary>
		/// <returns>The map to launch with</returns>
		private string GetSelectedMapSP()
		{
			string Map = "";
			if (CheckBox_UseCookedMap.Checked)
			{
				Map = TextBox_MapsToCook.Text;
				// split up multiple maps
				if (Map != "")
				{
					string[] Maps = TextBox_MapsToCook.Text.Split(' ');
					Map = Maps[0];
				}
			}
			else
			{
				Map = ComboBox_MapnameSP.Text;
			}

			return Map;
		}

		private void GetSPURLOptions(out string GameOptions, out string EngineOptions)
		{
			string ExtraGameOptions;
			string ExtraEngineOptions;
			ParseURLOptions(ECurrentConfig.SP, out ExtraGameOptions, out ExtraEngineOptions);

			// If we're in Wargame, use a special commandline
			if (ComboBox_Game.Text == "WarGame")
			{
				if (UseChapterPoint.Checked)
				{
					GameOptions = "wargame_p?loadchapter=" + ChapterPointSelect.Value.ToString();
				}
				else
				{
					GameOptions = "";
				}
			}
			else
			{
				// put together with URL options
				GameOptions = GetSelectedMapSP() + ExtraGameOptions;
			}

			// just use the extra options to start with
			EngineOptions = ExtraEngineOptions;

			// possibly run the PC with cooked data
			if (ComboBox_Platform.Text == "PC")
			{
				EngineOptions += GetOptionByName("RunWithCookedData").GetCommandlineOption();
			}
		}

		/// <summary>
		/// Handles loading a SP map on PC or console
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void OnLoadMapClick(object sender, EventArgs e)
		{
			// determine if the systray menu was used to select the map
			MenuItem Item = sender as MenuItem;
			// if so, browse for a map
			if (Item != null)
			{
				string MapName = Item.Text;
				// check for the browse option
				if (MapName == "Browse...")
				{
					if (!BrowseForMap(this.ComboBox_MapnameSP, out MapName))
					{
						// aborted browse, don't launch
						return;
					}
					else
					{
						UpdateMaps();
					}

				}
				// set the selected map to this one
				ComboBox_MapnameSP.Text = MapName;
			}


			string GameOptions;
			string EngineOptions;
			GetSPURLOptions(out GameOptions, out EngineOptions);

			// run on PC or console passing additional engine (-) options
			LaunchApp(GameOptions, EngineOptions, ECurrentConfig.SP, false);
		}

		private void OnEditorClick(object sender, EventArgs e)
		{
			LaunchApp("editor " + GetSelectedMapSP(), "", ECurrentConfig.Global, true);
		}

		private void OnBrowseSPClick(object sender, EventArgs e)
		{
			string MapName = "";
			if (BrowseForMap(this.ComboBox_MapnameSP, out MapName))
			{
				this.ComboBox_MapnameSP.Text = MapName;
			}
		}

		private void OnUseCookedMapChecked(object sender, EventArgs e)
		{
			ComboBox_MapnameSP.Enabled = !CheckBox_UseCookedMap.Checked;
			Button_BrowseSP.Enabled = !CheckBox_UseCookedMap.Checked;
			UpdateInfoText();
		}

		#endregion

		#region Mutiplayer tab

		private void GetMPURLOptions(out string GameOptions, out string EngineOptions)
		{
			string InitialURL;
			string Options = "";

			// build URL and options based on type
			if (ListBox_ServerType.Items[ListBox_ServerType.TopIndex].ToString() == "Listen")
			{
				int Spacing = ((SimpleIntGameOption)GetOptionByName("WindowSpacing")).Value;
				InitialURL = ComboBox_MapnameMP.Text + "?listen";
				if (ComboBox_Platform.Text == "PC")
				{
					Options = " -posX=" + Spacing + " -posY=" + Spacing;
				}
			}
			else
			{
				InitialURL = "server " + ComboBox_MapnameMP.Text;
			}

			// get extra options
			string ExtraGameOptions;
			string ExtraEngineOptions;
			ParseURLOptions(ECurrentConfig.MP, out ExtraGameOptions, out ExtraEngineOptions);

			// tack on the shared settings
			GameOptions = InitialURL + ExtraGameOptions;
			EngineOptions = Options + ExtraEngineOptions;
		}

		private void OnBrowseMPClick(object sender, EventArgs e)
		{
			string MapName = "";
			if (BrowseForMap(this.ComboBox_MapnameMP, out MapName))
			{
				this.ComboBox_MapnameMP.Text = MapName;
			}
		}

		private void OnKillAllClick(object sender, EventArgs e)
		{
			foreach (Process newProcess in Processes)
			{
				if (newProcess != null)
				{
					newProcess.Kill();
				}
			}
			Processes.Clear();
		}

		private void OnServerClick(object sender, EventArgs e)
		{
			// was it the result of a right-click menu option?
			MenuItem Item = sender as MenuItem;
			if (Item != null)
			{
				string MapName = Item.Text;
				// check for the browse option
				if (MapName == "Browse...")
				{
					if (!BrowseForMap(this.ComboBox_MapnameMP, out MapName))
					{
						// aborted browse, don't launch
						return;
					}
					else
					{
						UpdateMaps();
					}
				}
				this.ComboBox_MapnameMP.Text = MapName;
			}

			NumClients = 0;
			
			string GameOptions;
			string EngineOptions;
			GetMPURLOptions(out GameOptions, out EngineOptions);
			
			// run the mp game
			LaunchApp(GameOptions, EngineOptions, ECurrentConfig.MP, false);
		}

		private void OnServerDedicatedClick(object sender, EventArgs e)
		{
			string MapName;
			// was it the result of a right-click menu option?
			MenuItem Item = sender as MenuItem;
			if (Item != null)
			{
				MapName = Item.Text;
				// check for the browse option
				if (MapName == "Browse...")
				{
					if (!BrowseForMap(this.ComboBox_MapnameMP, out MapName))
					{
						// aborted browse, don't launch
						return;
					}
					else
					{
						UpdateMaps();
					}
				}
			}
			else
			{
				MapName = this.ComboBox_MapnameMP.Text;
			}
			NumClients = 0;

			string GameOptions;
			string EngineOptions;
			ParseURLOptions(ECurrentConfig.SP, out GameOptions, out EngineOptions);

			LaunchApp("server " + MapName + GameOptions, EngineOptions, ECurrentConfig.MP, false);
		}

		private void AddClient(int ResX, int ResY, int Spacing, string URLOptions)
		{
			int PosX = -1, PosY = -1;
			if (NumClients == 0)
			{
				PosX = Spacing;
				PosY = Spacing + ResY + Spacing;
			}
			else if (NumClients == 1)
			{
				PosX = Spacing + ResX + Spacing;
				PosY = Spacing;
			}
			else if (NumClients == 2)
			{
				PosX = Spacing + ResX + Spacing;
				PosY = Spacing + ResY + Spacing;
			}
			LaunchApp("127.0.0.1" + URLOptions, " -posX=" + PosX + " -posY=" + PosY, ECurrentConfig.MP, false);
			NumClients++;
		}

		private void OnClientClick(object sender, EventArgs e)
		{
			string URLOptions = GetMPOptionByName("URLOptions").GetCommandlineOption();
			int ResX, ResY;
			GetMPResolution(out ResX, out ResY);
			int Spacing = ((SimpleIntGameOption)GetOptionByName("WindowSpacing")).Value;
			AddClient(ResX, ResY, Spacing, URLOptions);
		}

		private void OnServerClientClick(object sender, EventArgs e)
		{
			string URLOptions = GetMPOptionByName("URLOptions").GetCommandlineOption();
			int ResX, ResY;
			GetMPResolution(out ResX, out ResY);
			int Spacing = ((SimpleIntGameOption)GetOptionByName("WindowSpacing")).Value;

			if(ListBox_ServerType.Items[ListBox_ServerType.TopIndex].ToString() == "Listen")
			{
				// launch the server
				LaunchApp(this.ComboBox_MapnameMP.Text + "?listen" + URLOptions, " -posX=" + Spacing + " -posY=" + Spacing, ECurrentConfig.MP, false);
			}
			else
			{
				LaunchApp("server " + this.ComboBox_MapnameMP.Text + URLOptions, "", ECurrentConfig.MP, false);
			}

			// stall a bit to prevent thrashing
			Thread.Sleep(10000);

			int DesiredClients = Convert.ToInt32(this.NumericUpDown_Clients.Value);
			NumClients = 0;
			while (NumClients < DesiredClients)
			{
				AddClient(ResX, ResY, Spacing, URLOptions);
				// stall a bit to prevent thrashing
				Thread.Sleep(10000);
			}
		}

		private void NumericUpDown_Clients_ValueChanged(object sender, EventArgs e)
		{
			ToolStripButton_ServerAndClients.Text = "Server + " + NumericUpDown_Clients.Value.ToString() +
				((NumericUpDown_Clients.Value == 1) ? " Client" : " Clients");

			Button_ServerClient.Text = ToolStripButton_ServerAndClients.Text;

			OnGenericComboBoxLeave(sender, e);
		}

		#endregion

		#region Cooking tab

		private string GetCookingCommandLine()
		{
			// Base command
			string CommandLine = "CookPackages -platform=" + ComboBox_Platform.Text;

			if (CheckBox_CookInisIntsOnly.Checked)
			{
				CommandLine += " -inisOnly";
			}
			else
			{
				// Add in map name
				CommandLine += " " + TextBox_MapsToCook.Text;

				if (CheckBox_ForceCookSeekfree.Checked)
				{
					CommandLine += " -recookseekfree";
				}
			}

			// @todo: Make this default for shipped version of this app
            if(CheckBox_CookInUserMode.Checked || ComboBox_Platform.Text == "PS3 Mod")
			{
				CommandLine += " -user -installed";
                if(ComboBox_Platform.Text == "PS3" || ComboBox_Platform.Text == "PS3 Mod")
				{
					CommandLine += " -usermodname=" + ModNameDialog.String;
				}
			}

			// Add in the final release option if necessary
			if (CheckBox_CookFinalReleaseScript.Checked)
			{
				CommandLine += " -final_release";
			}

			// Add in the final release option if necessary
			if (CheckBox_FullCook.Checked)
			{
				CommandLine += " -full";
			}

			// Add in the final release option if necessary
			if (CheckBox_CookAllMaps.Checked)
			{
				CommandLine += " -cookallmaps";
			}

			if (ComboBox_CookingLanguage.Text != "int" && ComboBox_CookingLanguage.Text != "")
			{
				CommandLine += " -languageforcooking=" + ComboBox_CookingLanguage.Text;
			}


			// we always want to have the latest .inis (this stops one from firing off the cooker and then coming back and seeing the "update .inis" message 
			// and then committing seppuku)
			CommandLine += " -updateInisAuto";

			return (CommandLine);
		}

		private void ImportMapList(string filename)
		{
			System.IO.StreamReader streamIn = new System.IO.StreamReader(filename);
			TextBox_MapsToCook.Text = streamIn.ReadToEnd().Replace("\r\n", " ");
			streamIn.Close();
		}

		private void OnImportMapListClick(object sender, EventArgs e)
		{
			OpenFileDialog ofd;
			ofd = new OpenFileDialog();
			ofd.Filter = "TXT Files (*.txt)|*.txt";
			if (ofd.ShowDialog() == DialogResult.OK)
			{
				ImportMapList(ofd.FileName);
			}
		}

		private void OnStartCookingClick(object sender, System.EventArgs e)
		{
			// handle files (TextureFileCache especially) being open on PC when trying to cook
			if (ComboBox_Platform.Text == "PS3" && !CheckBox_CookInisIntsOnly.Checked)
			{
				CacheGameNameAndConsoles();
				// reboot any running PS3s
				CookerTools.RebootAliveTargets();
			}

			// if no running commandlet, then start it
			if (RunningCommandletType == ECommandletType.CT_None)
			{
				if (ComboBox_Platform.Text == "PS3 Mod" || (CheckBox_CookInUserMode.Checked && ComboBox_Platform.Text == "PS3"))
				{
					if (ModNameDialog.ShowDialog() != DialogResult.OK)
					{
						return;
					}
				}

				StartCommandlet(ECommandletType.CT_Cook);
			}
			else
			{
				CheckedStopCommandlet();
			}

			//uncheck the full cook checkbox so the user doesn't accidentaly do a full cook again
			CheckBox_FullCook.Checked = false;
		}


		#endregion

		#region PC setup tab

		private string GetCompileCommandLine()
		{
			// plain make commandline
			string CommandLine = "make";

			if (ComboBox_ScriptConfig.Text == "Debug")
			{
				CommandLine += " -debug";
			}
			else if (ComboBox_ScriptConfig.Text == "Final Release")
			{
				CommandLine += " -final_release";
			}

			// tack on any extras
			CommandLine += GetCompileOptionByName("FullCompile").GetCommandlineOption();
			CommandLine += GetCompileOptionByName("AutoCompileMode").GetCommandlineOption();
            CommandLine += GetCompileOptionByName("QuietMode").GetCommandlineOption();

			return CommandLine;
		}

		private void OnProjectDirChange(object sender, EventArgs e)
		{
			// append a ending "\" if necessary
			string ProjectDir = ComboBox_ProjectDir.Text;
			if (ProjectDir.LastIndexOf("\\") != ProjectDir.Length - 1)
			{
				ComboBox_ProjectDir.Text = ProjectDir + "\\";
			}

			// let shared code update the MRU list
			OnGenericComboBoxLeave(sender, e);

			// set our current directory (needed by loaded DLLs)
			Directory.SetCurrentDirectory(ComboBox_ProjectDir.Text);
		}

		private void OnCompileClick(object sender, EventArgs e)
		{
			// if its not running, start it
			if (RunningCommandletType == ECommandletType.CT_None)
			{
				StartCommandlet(ECommandletType.CT_Compile);
			}
			else
			{
				CheckedStopCommandlet();
			}
		}

		#endregion

		#region Console setup tab

		/// <summary>
		/// Parse the game name and cache the list of checked targets in the console target list
		/// </summary>
		protected void CacheGameNameAndConsoles()
		{
			// get the name from the combo box
			string NewGameName = ComboBox_Game.Text;
			// strip off the trailing Game
			NewGameName = NewGameName.Replace("Game", "");

			// collect the checked consoles
			ArrayList ConsoleNames = new ArrayList();
			foreach (string ConsoleStr in ListBox_Targets.CheckedItems)
			{
				ConsoleNames.Add(ConsoleStr);
			}

			// tell the cooker tools the gamename (removing any spaces
			CookerTools.SetGameAndConsoleInfo(ComboBox_ProjectDir.Text, NewGameName, ConsoleNames, ComboBox_BaseDirectory.Text.Replace(" ", ""));
		}

		private void SetCopyingState(bool On)
		{
			if (On)
			{
				Button_StartCooking.Enabled = false;
				Button_CopyToTargets.Enabled = false;
				Button_RebootTargets.Enabled = false;
				TabControl_Main.Enabled = false;
			}
			else
			{
				Button_StartCooking.Enabled = true;
				Button_CopyToTargets.Enabled = CookerTools.PlatformNeedsToSync();
				Button_RebootTargets.Enabled = true;
				TabControl_Main.Enabled = true;
			}
		}

		private void OnCopyToTargetsClick(object sender, System.EventArgs e)
		{
			// Use the threadsafe version of clear
			LogClear();

			// Copy data to console.
			SyncWithRetry();

			// if the user wants to, run the game after cooking/syncing is finished
			if (CheckBox_LaunchAfterCooking.Checked)
			{
				OnLoadMapClick(null, null);
			}

		}

		/// <summary>
		/// Retry syncing to the target until it succeeds or the user chooses to cancel
		/// </summary>
		private void SyncWithRetry()
		{
			SetCopyingState(true);

			CacheGameNameAndConsoles();

			while (CookerTools.TargetSync() == false)
			{
				// Allow them to cancel if need be
				if (MessageBox.Show(
						"Please make sure your target is on and connected to the network",
						"Can't find target",
						MessageBoxButtons.RetryCancel) == DialogResult.Cancel)
				{
					SetCopyingState(false);
					return;
				}
			}

			AddLine("\r\n[ALL TARGETS PROCESSED]\r\n", Color.Green);
			SetCopyingState(false);
		}

		private void OnRebootTargetsClick(object sender, System.EventArgs e)
		{
			CheckedReboot(false, null);
		}

		#endregion

		#region Help tab

		private void OnTextBoxHelpClicked(object sender, LinkClickedEventArgs e)
		{
			System.Diagnostics.Process.Start(e.LinkText);
		}

		#endregion

		#endregion

		#region PS3 Tab

        /** Currently active PS3 compilation process - null if not running */
        Process RunningPS3MakeProcess = null;

        /** Set in the exit handler to let main thread handle make finishing */
        bool bNeedToHandlePS3MakeCompleted = false;

        /** Was the make successful? */
        bool bPS3MakeSucceeded;

        /** Are we currently doing the clean step? */
        bool bIsMakingClean = false;

		string GetPS3CommandLine( bool bClean )
		{
			string CommandLine;

			// make some options both make and IB will use
			string SharedOptions = " GAMENAME=" + ComboBox_Game.Text.ToUpper() + " BUILDTYPE=";
			if( ComboBox_ConsoleConfiguration.Text == "ReleaseLTCG" )
			{
				SharedOptions += "final_release";
			}
            else if (ComboBox_ConsoleConfiguration.Text == "ReleaseLTCG-DebugConsole")
            {
                SharedOptions += "final_release_debugconsole";
            }
            else
			{
				SharedOptions += ComboBox_ConsoleConfiguration.Text.ToLower();
			}

			if( CheckBox_NoDebugInfo.Checked )
			{
				SharedOptions += " NO_DEBUG_INFO=true";
			}

            if (!CheckBox_StopOnFirstError.Checked)
            {
                SharedOptions += " -k";
            }

            if( ComboBox_AddProfInfo.Text.ToLower() != "none" )
            {
                SharedOptions += " PS3_" + ComboBox_AddProfInfo.Text.ToUpper() + "=true";
            }

			if( bClean )
			{
				SharedOptions += " clean";
			}

			if (ComboBox_SelectPS3Compile.Text.ToLower() == "gcc make")
			{
				CommandLine = "-C ../Development/Src/PS3 PAUSE=true --jobs=" + NumericUpDown_NumberOfThreads.Value.ToString() + SharedOptions;
			}
			else
			{
				string XGEOptions;
				if (ComboBox_SelectPS3Compile.Text.ToLower() == "xge make (remote)")
				{
					XGEOptions = " PCHS= JOBS=20";
				}
				else
				{
					XGEOptions = " JOBS=" + NumericUpDown_NumberOfThreads.Value;
				}

				CommandLine = "/command=\"PS3\\IBMake -C ../Development/Src/PS3 --jobs=1 USE_IB=true" + XGEOptions + SharedOptions + "\" /profile=\"PS3\\IBMake.xml\"";
			}

			return CommandLine;
		}

        public void StartPS3IBMake( bool bClean )
        {
            RunningPS3MakeProcess = new Process();

            // Prepare a ProcessStart structure 
            RunningPS3MakeProcess.StartInfo.FileName = "xgConsole.exe";
            string CommandLine = GetPS3CommandLine( bClean );
            RunningPS3MakeProcess.StartInfo.Arguments = CommandLine;

            // Redirect the output.
            RunningPS3MakeProcess.StartInfo.UseShellExecute = false;
            RunningPS3MakeProcess.StartInfo.RedirectStandardOutput = true;
            RunningPS3MakeProcess.StartInfo.RedirectStandardError = true;
            RunningPS3MakeProcess.StartInfo.CreateNoWindow = true;

            AddLine( "Spawning: xgConsole.exe " + CommandLine, Color.Black );

            // Spawn the process - try to start the process, handling thrown exceptions as a failure.
            try
            {
                RunningPS3MakeProcess.OutputDataReceived += new DataReceivedEventHandler( PrintLog );
                RunningPS3MakeProcess.ErrorDataReceived += new DataReceivedEventHandler( PrintLog );
                RunningPS3MakeProcess.Exited += new EventHandler( ProcessExit );

                RunningPS3MakeProcess.Start();

                // Set priority to high
                RunningPS3MakeProcess.PriorityClass = ProcessPriorityClass.AboveNormal;
                RunningPS3MakeProcess.EnableRaisingEvents = true;

                RunningPS3MakeProcess.BeginOutputReadLine();
            }
            catch
            {
                RunningPS3MakeProcess = null;
            }
        }

        public void StartPS3Make( bool bClean )
        {
            RunningPS3MakeProcess = new Process();
            string Executable = "make.exe";
            string CommandLine = GetPS3CommandLine( bClean );

            // Prepare a ProcessStart structure 
            RunningPS3MakeProcess.StartInfo.FileName = Executable;
            RunningPS3MakeProcess.StartInfo.Arguments = CommandLine;

            // Redirect the output.
            RunningPS3MakeProcess.StartInfo.UseShellExecute = false;
            RunningPS3MakeProcess.StartInfo.CreateNoWindow = false;

            AddLine( "Spawning: " + Executable + " " + CommandLine, Color.Green );

            // Spawn the process - try to start the process, handling thrown exceptions as a failure.
            try
            {
                RunningPS3MakeProcess.Exited += new EventHandler( ProcessExit );

                RunningPS3MakeProcess.Start();

                // Set priority to high
                RunningPS3MakeProcess.PriorityClass = ProcessPriorityClass.AboveNormal;

                RunningPS3MakeProcess.EnableRaisingEvents = true;
            }
            catch
            {
                RunningPS3MakeProcess = null;
            }
        }

        public void PrintLog( object Sender, DataReceivedEventArgs e )
        {
            string Line = e.Data;
            if( Line != null )
            {
                AddLine( Line, Color.DarkGreen );
            }
        }

        public void ProcessExit( object Sender, System.EventArgs e )
        {
            // wast it successful?
            bPS3MakeSucceeded = RunningPS3MakeProcess.ExitCode == 0;

            // note that we aren't running it anymore
            RunningPS3MakeProcess = null;

            // mark that we need to handle t
            bNeedToHandlePS3MakeCompleted = true;
        }

        private void HandlePS3MakeCompleted()
        {
            // if we want to run after compiling, 
            if (!bIsMakingClean && CheckBox_LaunchAfterPS3Compile.Checked && bPS3MakeSucceeded)
            {
                // run the map
                OnLoadMapClick(null, null);
            }

            if (bIsMakingClean)
            {
                // no longer making clean
                bIsMakingClean = false;
            }
            else
            {
                // no longer compiling, restore the button text
                Button_PS3Compile.Text = "Compile";
            }

            // no longer need to handle this
            bNeedToHandlePS3MakeCompleted = false;

        }

        private void Button_PS3Compile_Click( object sender, EventArgs e )
        {
            if (RunningPS3MakeProcess != null)
            {
                if (CheckedStopCommandlet())
                {
                    RunningPS3MakeProcess.Kill();
//                    RunningPS3MakeProcess.WaitForExit();
                }
                return;
            }

            LogClear();

            if( CheckBox_FullRebuild.Checked )
            {
                bIsMakingClean = true;

                if( ComboBox_SelectPS3Compile.Text.ToLower() == "gcc make" )
                {
					StartPS3Make(true);
				}
                else
                {
					StartPS3IBMake(true);
                }

                // wait for make clean to complete
                while (RunningPS3MakeProcess != null)
                {
                    System.Threading.Thread.Sleep( 100 );
                }

				//uncheck the full rebuild check box so the user doesn't accidentaly do a full rebuild again
				CheckBox_FullRebuild.Checked = false;
            }

            if( ComboBox_SelectPS3Compile.Text.ToLower() == "gcc make" )
            {
				StartPS3Make(false);
            }
            else
            {
				StartPS3IBMake(false);
			}

            Button_PS3Compile.Text = "Stop Compile";
        }

        private void ComboBox_SelectPS3Compile_SelectionChangeCommitted( object sender, EventArgs e )
        {
            if( ComboBox_SelectPS3Compile.Text.ToLower() == "gcc make" )
            {
				Label_Threads2.Show();
				NumericUpDown_NumberOfThreads.Show();
			}
			else if (ComboBox_SelectPS3Compile.Text.ToLower() == "xge make (remote)")
			{
				Label_Threads2.Hide();
				NumericUpDown_NumberOfThreads.Hide();
			}
			else
			{
				Label_Threads2.Show();
				NumericUpDown_NumberOfThreads.Show();
			}
        }

		/// <summary>
		/// Converts a 32 bit int into 4 bytes that are in big endian format for PS3
		/// </summary>
		/// <param name="Int"></param>
		/// <returns></returns>
		private Byte[] Int2BytesSwapped(Int32 Int)
		{
			Byte[] Bytes = BitConverter.GetBytes(Int);

			Byte Temp = Bytes[0];
			Bytes[0] = Bytes[3];
			Bytes[3] = Temp;

			Temp = Bytes[1];
			Bytes[1] = Bytes[2];
			Bytes[2] = Temp;

			return Bytes;
		}

		private void ButtonPackageMod_Click(object sender, EventArgs e)
		{
			OpenFileDialog OpenDialog = new OpenFileDialog();

			OpenDialog.InitialDirectory = ComboBox_ProjectDir.Text + "..\\" + ComboBox_Game.Text + "\\CookedPS3";
			OpenDialog.Filter = "All Files (*.*)|*.*";
			OpenDialog.InitialDirectory = SettingsPath + "\\" + ComboBox_Game.Text + "\\Mods\\PS3";
			OpenDialog.Title = "Select files to packaged into a mod";
			OpenDialog.Multiselect = true;

			if (OpenDialog.ShowDialog() == DialogResult.OK)
			{
				if (ModNameDialog.ShowDialog() == DialogResult.OK)
				{
					PackageMod(OpenDialog.FileNames, ModNameDialog.String);
				}
			}
		}

		string ReadIniFile(string Path)
		{
			StreamReader Stream = new StreamReader(Path);
			string Contents = Stream.ReadToEnd();
			Stream.Close();
			return Contents;
		}

		void WriteIniFile(string Path, string Contents)
		{
			StreamWriter Stream = new StreamWriter(Path);
			Stream.Write(Contents);
			Stream.Close();
		}

		int CopyTemplateIniFiles(string ModTag, string ModName, string[] ModFiles, int NumModFiles)
		{
			// the tag is the subdir of Binaries\PS3\ModInis
			DirectoryInfo Dir = new DirectoryInfo(ComboBox_ProjectDir.Text + "\\PS3\\ModInis\\" + ModTag);

			// get the files in the directory
			FileInfo[] Files = Dir.GetFiles("*.ini");

			// create output ini path
			string OutPath = Path.GetDirectoryName(ModFiles[0]) + "\\";

			// process all the .ini files
			foreach (FileInfo File in Files)
			{
				// read template ini
				string Contents = ReadIniFile(File.FullName);

				// @todo: Replace variables!
				if (ModTag == "Map")
				{
					string CookedMap = TextBox_MapsToCook.Text.Split(' ')[0];
					// replace %MAPNAME% get first map cooked
					Contents = Contents.Replace("%MAPNAME%", CookedMap);
					
					// replace %MAPDISPLAYNAME% with map name after '-' (last token when splitting on -)
					string[] MapTokens = CookedMap.Split('-');
					Contents = Contents.Replace("%MAPDISPLAYNAME%", MapTokens[MapTokens.Length - 1]);
				}
				else if (ModTag == "Mutator")
				{
					// replace %MUTATORNAME% with the name of the mutator package
					Contents = Contents.Replace("%MUTATORNAME%", ModName);

					// for %MUTATORCLASSPATH%, by default assume that the package and classname of the mutator are the same
					Contents = Contents.Replace("%MUTATORCLASSPATH%", ModName + "." + ModName);

					// clear %MUTATORGROUP% for now...
					Contents = Contents.Replace("%MUTATORGROUP%", "");
				}

				// write ini to mod dir
				WriteIniFile(OutPath + File.Name, Contents);

				// add the ini to the list of files in the mod
				ModFiles[NumModFiles++] = OutPath + File.Name;
			}

			return NumModFiles;
		}

		void PackageMod(string[] InFiles, string ModName)
		{
			// @todo validate Mod Name (< 127 characters, letters only)

			// filter out unneeded files, with extra room for ini files
			string[] Files = new string[InFiles.Length + 20];
			Int32 NumFiles = 0;

			// track if we found a .ini file
			bool bHasIniFile = false;
			for (int FileIndex = 0; FileIndex < InFiles.Length; FileIndex++)
			{
				// does it have an .ini file?
				if (Path.GetExtension(InFiles[FileIndex]) == ".ini")
				{
					bHasIniFile = true;
				}

				// skip the global cooked info file (the cooker will output it, but we don't need it)
				// skip the dummy .ini file from being put into the mod, since it's just for bHasIniFile checking
				if (Path.GetFileName(InFiles[FileIndex]).ToUpper().StartsWith("GLOBALPERSISTENTCOOKERDATA") == false &&
					Path.GetFileName(InFiles[FileIndex]).ToUpper().StartsWith("_DUMMY.INI") == false)
				{
					Files[NumFiles++] = InFiles[FileIndex];
				}
			}

			// we need files!
			if (NumFiles == 0)
			{
				return;
			}


			// if there was no ini file, bring up the auto-ini-copier dialog
			if (!bHasIniFile)
			{
				ModSelector Dlg = new ModSelector();

				if (Dlg.ShowDialog() == DialogResult.OK)
				{
					if (Dlg.RadioButton_ModNone.Checked)
					{
						// write an empty dummy .ini so we stop getting asked
						WriteIniFile(Path.GetDirectoryName(Files[0]) + "\\_Dummy.ini", "");
					}
					else
					{
						// figure out which radio button is selected
						foreach (Control C in Dlg.Controls)
						{
							RadioButton RadioButton = C as RadioButton;
							if (RadioButton != null && RadioButton.Checked)
							{
								// copy and process the ini files in the matching mod ini source dir
								NumFiles = CopyTemplateIniFiles(RadioButton.Tag.ToString(), ModName, Files, NumFiles);
								break;
							}
						}
					}
				}

				ReviewInis ReviewDlg = new ReviewInis();
				ReviewDlg.LinkLabel_ModDir.Text = Path.GetDirectoryName(Files[0]);
				ReviewDlg.Width = ReviewDlg.LinkLabel_ModDir.Width + ReviewDlg.LinkLabel_ModDir.Left * 2;
				ReviewDlg.ShowDialog();
			}

			// make filenames/paths
			string JamDir = ComboBox_ProjectDir.Text + "..\\" + ComboBox_Game.Text + "\\Mods\\PS3\\";
			string JamFilename = "USERDATA.JAM";

			// create .jam file
			Directory.CreateDirectory(JamDir);
			FileStream JamFile = File.Create(JamDir + JamFilename);

			// storage for writing ints as bytes
			Byte[] IntBytes;

			// write out a dummy offset
			Int32 TableOffset = 0;
			IntBytes = Int2BytesSwapped(TableOffset);

			JamFile.Write(IntBytes, 0, IntBytes.Length);

			// make storage to store sizes for table
			Int32[] FileSizes = new Int32[NumFiles];

			// now jam together all selected files into one
			for (int FileIndex = 0; FileIndex < NumFiles; FileIndex++)
			{
				// error out if a filename is too long to work on PS3
				if (Path.GetFileName(Files[FileIndex]).Length > 127)
				{
					MessageBox.Show("Filename too long - " + Path.GetFileName(Files[FileIndex]), "Bad filename", MessageBoxButtons.OK, MessageBoxIcon.Error);
					JamFile.Close();
					return;
				}

				// jam together into the file
				FileStream InputFile = File.Open(Files[FileIndex], FileMode.Open);
				Int32 FileSize = (Int32)InputFile.Length;
				Byte[] Bytes = new Byte[FileSize];
				InputFile.Read(Bytes, 0, FileSize);
				JamFile.Write(Bytes, 0, FileSize);

				InputFile.Close();

				// store size (in the location it will be used in the next loop)
				FileSizes[FileIndex] = FileSize;
			}

			// remember current offset
			TableOffset = (Int32)JamFile.Position;


			// write out number of files
			IntBytes = Int2BytesSwapped(NumFiles);
			JamFile.Write(IntBytes, 0, IntBytes.Length);

			// convert string to bytes
			Byte[] StringBytes = new UTF8Encoding().GetBytes(ModNameDialog.String);

			// write out length of string
			IntBytes = Int2BytesSwapped(StringBytes.Length);
			JamFile.Write(IntBytes, 0, IntBytes.Length);

			// write out string
			JamFile.Write(StringBytes, 0, StringBytes.Length);

			for (int FileIndex = 0; FileIndex < NumFiles; FileIndex++)
			{
				// write out filesize
				IntBytes = Int2BytesSwapped(FileSizes[FileIndex]);
				JamFile.Write(IntBytes, 0, IntBytes.Length);

				// convert string to bytes
				StringBytes = new UTF8Encoding().GetBytes(Path.GetFileName(Files[FileIndex]));

				// write out length of string
				IntBytes = Int2BytesSwapped(StringBytes.Length);
				JamFile.Write(IntBytes, 0, IntBytes.Length);

				// write out string
				JamFile.Write(StringBytes, 0, StringBytes.Length);
			}

			// remember size
			int JamFileSize = (int)JamFile.Position;

			// go back to start of file and write offset of the table
			JamFile.Position = 0;
			IntBytes = Int2BytesSwapped(TableOffset);
			JamFile.Write(IntBytes, 0, IntBytes.Length);


			JamFile.Close();


			bool bDoneWriting = false;
			while (!bDoneWriting)
			{
				// now, we need to copy it to a media device (memory stick, SD card, etc)
				FolderBrowserDialog MediaDialog = new FolderBrowserDialog();
				MediaDialog.Description = "Select a media device to copy Mod to";
				MediaDialog.ShowNewFolderButton = false;
				MediaDialog.RootFolder = Environment.SpecialFolder.MyComputer;

				DialogResult Res = MediaDialog.ShowDialog();
				if (Res == DialogResult.OK)
				{
					// get the root of the selected drive
					string MediaDrive = Path.GetPathRoot(MediaDialog.SelectedPath);

					if (MediaDrive[1] != ':')
					{
						MessageBox.Show("Please select a local, removable drive", "Invalid drive selected", MessageBoxButtons.OK, MessageBoxIcon.Warning);
					}
					else
					{
						// tell the CookerTools to get values from .xml files						
						CacheGameNameAndConsoles();

						// use the title ID from the .xml file
						string FinalDir = MediaDrive + "PS3\\SAVEDATA\\" + CookerTools.CachedGameSettings.PS3TitleID + "-MODIMPORT\\";

						// copy the .jam file to the media
						try
						{
							Directory.CreateDirectory(FinalDir);
							File.Copy(JamDir + JamFilename, FinalDir + JamFilename, true);
							// if it doesn't throw, we're done
							bDoneWriting = true;
						  }
						catch (Exception /*e*/)
						{
							if (MessageBox.Show("The copy to your media device failed. Check that it has " + ((JamFileSize + 1023) / 1024).ToString() + "K free, is inserted, and is not write-protected.",
								"Copy failed", MessageBoxButtons.RetryCancel, MessageBoxIcon.Error) != DialogResult.Retry)
							{
								bDoneWriting = true;
							}
						}
					}
				}
				else
				{
					bDoneWriting = true;
				}
			}

		}

#endregion
	}
			
}