/*=============================================================================
	Copyright 2004-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "UnTexAlignTools.h"
#include "ScopedTransaction.h"
#include "Properties.h"
#include "BusyCursor.h"

///////////////////////////////////////////////////////////////////////////////
//
// Local classes.
//
///////////////////////////////////////////////////////////////////////////////

class WxSurfacePropertiesPanel : public wxPanel
{
public:
	WxSurfacePropertiesPanel( wxWindow* InParent );

	/**
	 * Called by WxDlgSurfaceProperties::RefreshPages().
	 */
	void RefreshPage();

private:
	void Pan( INT InU, INT InV );
	void Scale( FLOAT InScaleU, FLOAT InScaleV, UBOOL InRelative );

	void OnU1( wxCommandEvent& In );
	void OnU4( wxCommandEvent& In );
	void OnU16( wxCommandEvent& In );
	void OnU64( wxCommandEvent& In );
	void OnUCustom( wxCommandEvent& In );

	void OnV1( wxCommandEvent& In );
	void OnV4( wxCommandEvent& In );
	void OnV16( wxCommandEvent& In );
	void OnV64( wxCommandEvent& In );
	void OnVCustom( wxCommandEvent& In );

	void OnFlipU( wxCommandEvent& In );
	void OnFlipV( wxCommandEvent& In );
	void OnRot45( wxCommandEvent& In );
	void OnRot90( wxCommandEvent& In );
	void OnRotCustom( wxCommandEvent& In );

	void OnApply( wxCommandEvent& In );
	void OnScaleSimple( wxCommandEvent& In );
	void OnScaleCustom( wxCommandEvent& In );
	void OnLightMapResSelChange( wxCommandEvent& In );
	void OnAcceptsLightsChange( wxCommandEvent& In );
	void OnAcceptsDynamicLightsChange( wxCommandEvent& In );
	void OnForceLightmapChange( wxCommandEvent& In );
	void OnAlignSelChange( wxCommandEvent& In );
	void OnApplyAlign( wxCommandEvent& In );

	//<@ ava specific ; changmin
	// 2007. 8. 14 Secondary Light Source
	void OnSecondaryLightSourceChange( wxCommandEvent &In );
	// 2007. 12. 26 Cast Realtime Sun Shadow
	void OnForceCastSunShadowChange( wxCommandEvent &In );
	//>@ ava

	/**
	 * Sets passed in poly flag on selected surfaces.
	 *
 	 * @param PolyFlag	PolyFlag to toggle on selected surfaces
	 * @param Value		Value to set the flag to.
	 */
	void SetPolyFlag( DWORD PolyFlag, UBOOL Value );

	wxPanel* Panel;

	wxRadioButton *SimpleScaleButton;
	wxRadioButton *CustomScaleButton;

	wxComboBox *SimpleCB;

	wxStaticText *CustomULabel;
	wxStaticText *CustomVLabel;

	wxTextCtrl *CustomUEdit;
	wxTextCtrl *CustomVEdit;

	wxCheckBox *RelativeCheck;
	wxComboBox *LightMapResCombo;

	/** Checkbox for PF_AcceptsLights */
	wxCheckBox*	AcceptsLightsCheck;
	/** Checkbox for PF_AcceptsDynamicLights */
	wxCheckBox*	AcceptsDynamicLightsCheck;
	/** Checkbox for PF_ForceLigthMap */
	wxCheckBox* ForceLightMapCheck;

	//<@ ava specific ;changmin
	wxCheckBox*	SecondaryLightSourceCheck;	// 2007.  8. 13 check box for PF_SecondaryLightSource	
	wxCheckBox* ForceCastSunShadowCheck;	// 2007. 12. 26 check box for PF_ForceCastSunShadow
	//>@ ava

	//<@ ava specific ; 2007. 8. 27 changmin
	wxStaticText	*SurfaceMappingInfo;
	//>@ ava

	UBOOL bUseSimpleScaling;
	WxPropertyWindow* PropertyWindow;
	wxListBox *AlignList;


};


/**
 * WxDlgBindHotkeys
 */
namespace
{
	static const wxColour DlgBindHotkeys_SelectedBackgroundColor(255,218,171);
	static const wxColour DlgBindHotkeys_LightColor(227, 227, 239);
	static const wxColour DlgBindHotkeys_DarkColor(200, 200, 212);
}


class WxKeyBinder : public wxTextCtrl
{
public:
	WxKeyBinder(wxWindow* Parent, class WxDlgBindHotkeys* InParentDlg) : 
	  wxTextCtrl(Parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY),
	  ParentDlg(InParentDlg)
	{

	}

private:

	/** Handler for when a key is pressed or released. */
	void OnKeyEvent(wxKeyEvent& Event);

	/** Handler for when focus is lost */
	void OnKillFocus(wxFocusEvent& Event);

	/** Pointer to the parent of this class. */
	class WxDlgBindHotkeys* ParentDlg;

	DECLARE_EVENT_TABLE()
};


BEGIN_EVENT_TABLE(WxKeyBinder, wxTextCtrl)
	EVT_KEY_DOWN(OnKeyEvent)
	EVT_KEY_UP(OnKeyEvent)
	EVT_KILL_FOCUS(OnKillFocus)
END_EVENT_TABLE()


/** Handler for when a key is pressed or released. */
void WxKeyBinder::OnKeyEvent(wxKeyEvent& Event)
{
	if(ParentDlg->IsBinding())
	{
		if(Event.GetKeyCode() != WXK_SHIFT && Event.GetKeyCode() != WXK_CONTROL && Event.GetKeyCode() != WXK_ALT && Event.GetEventType() == wxEVT_KEY_DOWN)
		{
			FName Key = GApp->GetKeyName(Event);

			if(Key==KEY_Escape)
			{
				ParentDlg->StopBinding();
			}
			else
			{
				ParentDlg->FinishBinding(Event);
			}
		}
		else
		{
			FString BindString = ParentDlg->GenerateBindingText(Event.AltDown(), Event.ControlDown(), Event.ShiftDown(), NAME_None);
			SetValue(*BindString);
		}
	}
}

/** Handler for when focus is lost */
void WxKeyBinder::OnKillFocus(wxFocusEvent& Event)
{
	ParentDlg->StopBinding();
}

BEGIN_EVENT_TABLE(WxDlgBindHotkeys, wxFrame)
	EVT_CLOSE(OnClose)
	EVT_BUTTON(ID_DLGBINDHOTKEYS_LOAD_CONFIG, OnLoadConfig)
	EVT_BUTTON(ID_DLGBINDHOTKEYS_SAVE_CONFIG, OnSaveConfig)
	EVT_BUTTON(ID_DLGBINDHOTKEYS_RESET_TO_DEFAULTS, OnResetToDefaults)
	EVT_BUTTON(wxID_OK, OnOK)
	EVT_TREE_SEL_CHANGED(ID_DLGBINDHOTKEYS_TREE, OnCategorySelected)
	EVT_COMMAND_RANGE(ID_DLGBINDHOTKEYS_BIND_KEY_START, ID_DLGBINDHOTKEYS_BIND_KEY_END, wxEVT_COMMAND_BUTTON_CLICKED, OnBindKey)
	EVT_KEY_DOWN(OnKeyDown)
	EVT_CHAR(OnKeyDown)
END_EVENT_TABLE()

WxDlgBindHotkeys::WxDlgBindHotkeys(wxWindow* InParent) : 
wxFrame(InParent, wxID_ANY, *LocalizeUnrealEd("DlgBindHotkeys_Title"),wxDefaultPosition, wxSize(640,480), wxDEFAULT_FRAME_STYLE | wxWANTS_CHARS),
ItemSize(20),
NumVisibleItems(0),
CommandPanel(NULL),
CurrentlyBindingIdx(-1)
{
	wxSizer* PanelSizer = new wxBoxSizer(wxHORIZONTAL);
	{		
		wxPanel* MainPanel = new wxPanel(this);
		{
			wxSizer* MainSizer = new wxBoxSizer(wxVERTICAL);
			{
				MainSplitter = new wxSplitterWindow(MainPanel);
				{
					CommandPanel = new wxScrolledWindow(MainSplitter);
					CommandCategories = new WxTreeCtrl(MainSplitter, ID_DLGBINDHOTKEYS_TREE, NULL, wxTR_HIDE_ROOT | wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT);
					
					MainSplitter->SplitVertically(CommandCategories, CommandPanel);
					MainSplitter->SetMinimumPaneSize(128);
				}
				MainSizer->Add(MainSplitter, 1, wxEXPAND | wxALL, 2);

				wxBoxSizer* HSizer = new wxBoxSizer(wxHORIZONTAL);
				{
					wxButton* LoadButton = new wxButton(MainPanel, ID_DLGBINDHOTKEYS_LOAD_CONFIG, *LocalizeUnrealEd("LoadConfig"));
					wxButton* SaveButton = new wxButton(MainPanel, ID_DLGBINDHOTKEYS_SAVE_CONFIG, *LocalizeUnrealEd("SaveConfig"));
					wxButton* ResetToDefaults = new wxButton(MainPanel, ID_DLGBINDHOTKEYS_RESET_TO_DEFAULTS, *LocalizeUnrealEd("ResetToDefaults"));
					BindLabel = new wxStaticText(MainPanel, wxID_ANY, TEXT(""));
					BindLabel->GetFont().SetWeight(wxFONTWEIGHT_BOLD);

					HSizer->Add(LoadButton, 0, wxEXPAND | wxALL, 2);
					HSizer->Add(SaveButton, 0, wxEXPAND | wxALL, 2);
					HSizer->Add(ResetToDefaults, 0, wxEXPAND | wxALL, 2);
					HSizer->Add(BindLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 4);
					
					wxBoxSizer* VSizer = new wxBoxSizer(wxVERTICAL);
					{
						wxButton* OKButton = new wxButton(MainPanel, wxID_OK, *LocalizeUnrealEd("OK"));
						VSizer->Add(OKButton, 0, wxALIGN_RIGHT | wxALL, 2);
					}
					HSizer->Add(VSizer,1, wxEXPAND);
				}
				MainSizer->Add(HSizer, 0, wxEXPAND | wxALL, 2);
			}
			MainPanel->SetSizer(MainSizer);
		}
		PanelSizer->Add(MainPanel, 1, wxEXPAND);
	}
	SetSizer(PanelSizer);
	SetAutoLayout(TRUE);

	// Build category tree
	BuildCategories();

	// Load window options.
	LoadSettings();
}

/** Saves settings for this dialog to the INI. */
void WxDlgBindHotkeys::SaveSettings()
{
	FWindowUtil::SavePosSize(TEXT("DlgBindHotkeys"), this);

	GConfig->SetInt(TEXT("DlgBindHotkeys"), TEXT("SplitterPos"), MainSplitter->GetSashPosition(), GEditorUserSettingsIni);

	// Save the key bindings object
	UUnrealEdOptions* Options = GUnrealEd->GetUnrealEdOptions();
	
	if(Options)
	{
		UUnrealEdKeyBindings* KeyBindings = Options->EditorKeyBindings;
		
		if(KeyBindings)
		{
			FString UserKeybindings = appGameConfigDir() + GGameName;
			UserKeybindings += TEXT("EditorUserKeybindings.ini");

			KeyBindings->SaveConfig(CPF_Config, *UserKeybindings);
		}
	}
}

/** Loads settings for this dialog from the INI. */
void WxDlgBindHotkeys::LoadSettings()
{
	FWindowUtil::LoadPosSize(TEXT("DlgBindHotkeys"), this, -1, -1, 640, 480);

	INT SashPos = 256;

	GConfig->GetInt(TEXT("DlgBindHotkeys"), TEXT("SplitterPos"), SashPos, GEditorUserSettingsIni);
	MainSplitter->SetSashPosition(SashPos);

	// Load the user's settings
	UUnrealEdOptions* Options = GUnrealEd->GetUnrealEdOptions();

	if(Options)
	{
		UUnrealEdKeyBindings* KeyBindings = Options->EditorKeyBindings;

		if(KeyBindings)
		{
			// Load user bindings first.
			FString UserKeyBindings = appGameConfigDir() + GGameName;
			UserKeyBindings += TEXT("EditorUserKeyBindings.ini");
			KeyBindings->ReloadConfig(NULL, *UserKeyBindings);

			TArray<FEditorKeyBinding> UserKeys = Options->EditorKeyBindings->KeyBindings;

			// Reload Defaults
			KeyBindings->ReloadConfig();

			// Generate a binding map for quick lookup.
			TMap<FName, INT> BindingMap;

			BindingMap.Empty();

			for(INT BindingIdx=0; BindingIdx<KeyBindings->KeyBindings.Num(); BindingIdx++)
			{
				FEditorKeyBinding &KeyBinding = KeyBindings->KeyBindings(BindingIdx);

				BindingMap.Set(KeyBinding.CommandName, BindingIdx);
			}

			// Merge in user keys
			for(INT KeyIdx=0; KeyIdx<UserKeys.Num();KeyIdx++)
			{
				FEditorKeyBinding &UserKey = UserKeys(KeyIdx);

				INT* BindingIdx = BindingMap.Find(UserKey.CommandName);

				if(BindingIdx && KeyBindings->KeyBindings.IsValidIndex(*BindingIdx))
				{
					FEditorKeyBinding &DefaultBinding = KeyBindings->KeyBindings(*BindingIdx);
					DefaultBinding = UserKey;
				}
				else
				{
					KeyBindings->KeyBindings.AddItem(UserKey);
				}
			}
		}
	}
}

/** Builds the category tree. */
void WxDlgBindHotkeys::BuildCategories()
{
	ParentMap.Empty();
	CommandCategories->DeleteAllItems();
	CommandCategories->AddRoot(TEXT("Root"));
	
	// Add all of the categories to the tree.
	UUnrealEdOptions* Options = GUnrealEd->GetUnrealEdOptions();
	Options->GenerateCommandMap();

	if(Options)
	{
		TArray<FEditorCommandCategory> &Categories = Options->EditorCategories;

		for(INT CategoryIdx=0; CategoryIdx<Categories.Num(); CategoryIdx++)
		{
			FEditorCommandCategory &Category = Categories(CategoryIdx);
			wxTreeItemId ParentId = CommandCategories->GetRootItem();

			if(Category.Parent != NAME_None)
			{
				wxTreeItemId* ParentIdPtr = ParentMap.Find(Category.Parent);

				if(ParentIdPtr)
				{
					ParentId = *ParentIdPtr;
				}
			}

			FString CategoryName = Localize(TEXT("CommandCategoryNames"),*Category.Name.ToString(),TEXT("UnrealEd"));
			wxTreeItemId TreeItem = CommandCategories->AppendItem(ParentId, *CategoryName);
			ParentMap.Set(Category.Name, TreeItem);

			// Create the command map entry for this category.
			TArray<FEditorCommand> Commands;
			for(INT CmdIdx=0; CmdIdx<Options->EditorCommands.Num(); CmdIdx++)
			{
				FEditorCommand &Command = Options->EditorCommands(CmdIdx);

				if(Command.Parent == Category.Name)
				{
					Commands.AddItem(Command);
				}
			}

			if(Commands.Num())
			{
				CommandMap.Set(Category.Name, Commands);
			}
		}
	}
}

/** Builds the command list using the currently selected category. */
void WxDlgBindHotkeys::BuildCommands()
{
	UUnrealEdOptions* Options = GUnrealEd->GetUnrealEdOptions();

	CommandPanel->Freeze();

	// Remove all children from the panel.
	CommandPanel->DestroyChildren();
	
	VisibleBindingControls.Empty();

	FName ParentName;
	const FName* ParentNamePtr = ParentMap.FindKey(CommandCategories->GetSelection());

	if(ParentNamePtr != NULL)
	{
		ParentName = *ParentNamePtr;
		if(ParentName != NAME_None)
		{
			TArray<FEditorCommand> *CommandsPtr = CommandMap.Find(ParentName);

			if(CommandsPtr)
			{
				wxBoxSizer* PanelSizer = new wxBoxSizer(wxVERTICAL);
				{
					// Loop through all commands and add a panel for each one.
					TArray<FEditorCommand> &Commands = *CommandsPtr;

					for(INT CmdIdx=0; CmdIdx<Commands.Num(); CmdIdx++)
					{
						FEditorCommand &Command = Commands(CmdIdx);

						// Add a button, label, and binding box for each command.
						wxPanel* ItemPanel = new wxPanel(CommandPanel);
						{
							FCommandPanel PanelWidgets;
							PanelWidgets.BindingPanel = ItemPanel;
							PanelWidgets.CommandName = Command.CommandName;

							ItemPanel->SetBackgroundColour((CmdIdx%2==0) ? DlgBindHotkeys_LightColor : DlgBindHotkeys_DarkColor);

							FString CommandName = Localize(TEXT("CommandNames"),*Command.CommandName.ToString(),TEXT("UnrealEd"));
							wxBoxSizer* ItemSizer = new wxBoxSizer(wxHORIZONTAL);
							{
								wxBoxSizer* VSizer = new wxBoxSizer(wxVERTICAL);
								{
									wxStaticText* CommandLabel = new wxStaticText(ItemPanel, wxID_ANY, *CommandName);
									CommandLabel->GetFont().SetWeight(wxFONTWEIGHT_BOLD);
									VSizer->Add(CommandLabel, 0, wxEXPAND | wxTOP, 2);

									wxTextCtrl* CurrentBinding = new WxKeyBinder(ItemPanel, this);
									VSizer->Add(CurrentBinding, 0, wxEXPAND | wxBOTTOM, 6);

									// Store reference to the binding widget.
									PanelWidgets.BindingWidget = CurrentBinding;
								}
								ItemSizer->Add(VSizer, 1, wxEXPAND | wxALL, 4);

								wxButton* SetBindingButton = new wxButton(ItemPanel, ID_DLGBINDHOTKEYS_BIND_KEY_START+CmdIdx, *LocalizeUnrealEd("Bind"));
								ItemSizer->Add(SetBindingButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 8);

								PanelWidgets.BindingButton = SetBindingButton;
							}
							ItemPanel->SetSizer(ItemSizer);			

							// Store pointers to each of the controls we created for future use.
							VisibleBindingControls.AddItem(PanelWidgets);
						}
						PanelSizer->Add(ItemPanel, 0, wxEXPAND);
						PanelSizer->RecalcSizes();

						ItemSize = ItemPanel->GetSize().GetHeight();
					}

					VisibleCommands = Commands;
					NumVisibleItems = Commands.Num();
				}
				CommandPanel->SetSizer(PanelSizer);
			}
		}
	}

	// Refresh binding text
	RefreshBindings();

	// Start drawing the command panel again
	CommandPanel->Thaw();

	// Updates the scrollbar and refreshes this window.
	CommandPanel->Layout();
	CommandPanel->Refresh();
	UpdateScrollbar();
	CommandPanel->Refresh();
}

/** Refreshes the binding text for the currently visible binding widgets. */
void WxDlgBindHotkeys::RefreshBindings()
{
	UUnrealEdOptions* Options = GUnrealEd->GetUnrealEdOptions();

	// Loop through all visible items and update the current binding text.
	for(INT BindingIdx=0; BindingIdx<VisibleBindingControls.Num(); BindingIdx++)
	{
		FCommandPanel &BindingWidgets = VisibleBindingControls(BindingIdx);
		wxTextCtrl* CurrentBinding = BindingWidgets.BindingWidget;

		// Set the key binding text.
		FEditorKeyBinding* KeyBinding = Options->GetKeyBinding(BindingWidgets.CommandName);

		FString BindingText;

		if(KeyBinding)
		{
			BindingText = GenerateBindingText(KeyBinding->bAltDown, KeyBinding->bCtrlDown, KeyBinding->bShiftDown, KeyBinding->Key);								
		}
		else
		{
			BindingText = *LocalizeUnrealEd("NotBound");
		}

		CurrentBinding->SetValue(*BindingText);
	}
}

/** Updates the scrollbar for the command view. */
void WxDlgBindHotkeys::UpdateScrollbar()
{
	if(CommandPanel)
	{
		CommandPanel->EnableScrolling(FALSE, TRUE);
		CommandPanel->SetScrollbars(0, ItemSize, 0, NumVisibleItems);
	}
}

/** Starts the binding process for the specified command index. */
void WxDlgBindHotkeys::StartBinding(INT CommandIdx)
{
	CurrentlyBindingIdx = CommandIdx;

	// Set focus to the binding control
	FCommandPanel &BindingWidgets = VisibleBindingControls(CommandIdx);

	// Change the background of the current command
	BindingWidgets.BindingPanel->SetBackgroundColour(DlgBindHotkeys_SelectedBackgroundColor);
	BindingWidgets.BindingPanel->Refresh();

	// Set focus to the binding widget
	wxTextCtrl* BindingControl = BindingWidgets.BindingWidget;
	BindingControl->SetFocus();

	// Disable all binding buttons.
	for(INT ButtonIdx=0; ButtonIdx<VisibleBindingControls.Num(); ButtonIdx++)
	{
		VisibleBindingControls(ButtonIdx).BindingButton->Disable();
	}

	// Show binding text
	BindLabel->SetLabel(*LocalizeUnrealEd("PressKeysToBind"));
}

/** Finishes the binding for the current command using the provided event. */
void WxDlgBindHotkeys::FinishBinding(wxKeyEvent &Event)
{
	// Finish binding the key.
	UUnrealEdOptions* Options = GUnrealEd->GetUnrealEdOptions();

	FName KeyName = GApp->GetKeyName(Event);

	if(KeyName != NAME_None && Options->EditorCommands.IsValidIndex(CurrentlyBindingIdx))
	{
		FName Command = Options->EditorCommands(CurrentlyBindingIdx).CommandName;
		
		Options->BindKey(KeyName, Event.AltDown(), Event.ControlDown(), Event.ShiftDown(), Command);
	}

	StopBinding();
}

/** Stops the binding process for the current command. */
void WxDlgBindHotkeys::StopBinding()
{
	if(CurrentlyBindingIdx != -1)
	{		
		// Reset the background color
		FCommandPanel &BindingWidgets = VisibleBindingControls(CurrentlyBindingIdx);
		BindingWidgets.BindingPanel->SetBackgroundColour((CurrentlyBindingIdx%2==0) ? DlgBindHotkeys_LightColor : DlgBindHotkeys_DarkColor);
		BindingWidgets.BindingPanel->Refresh();

		// Enable all binding buttons.
		for(INT ButtonIdx=0; ButtonIdx<VisibleBindingControls.Num(); ButtonIdx++)
		{
			VisibleBindingControls(ButtonIdx).BindingButton->Enable();
		}

		// Hide binding text
		BindLabel->SetLabel(TEXT(""));

		// Refresh binding text
		RefreshBindings();

		CurrentlyBindingIdx = -1;
	}
}

/**
 * @return Generates a descriptive binding string based on the key combinations provided.
 */
FString WxDlgBindHotkeys::GenerateBindingText(UBOOL bAltDown, UBOOL bCtrlDown, UBOOL bShiftDown, FName Key)
{
	// Build a string describing this key binding.
	FString BindString;

	if(bCtrlDown)
	{
		BindString += TEXT("Ctrl + ");
	}

	if(bAltDown)
	{
		BindString += TEXT("Alt + ");
	}

	if(bShiftDown)
	{
		BindString += TEXT("Shift + ");
	}

	if(Key != NAME_None)
	{
		BindString += Key.ToString();
	}

	return BindString;
}

/** Window closed event handler. */
void WxDlgBindHotkeys::OnClose(wxCloseEvent& Event)
{
	SaveSettings();

	// Hide the dialog
	Hide();

	// Veto the close
	Event.Veto();
}

/** Category selected handler. */
void WxDlgBindHotkeys::OnCategorySelected(wxTreeEvent& Event)
{
	BuildCommands();
}

/** Handler to let the user load a config from a file. */
void WxDlgBindHotkeys::OnLoadConfig(wxCommandEvent& Event)
{
	// Get the filename
	wxChar* FileTypes = TEXT("INI Files (*.ini)|*.ini|All Files (*.*)|*.*");

	WxFileDialog Dlg( this, 
		*LocalizeUnrealEd("LoadKeyConfig"), 
		*appGameConfigDir(),
		TEXT(""),
		FileTypes,
		wxOPEN | wxFILE_MUST_EXIST | wxHIDE_READONLY,
		wxDefaultPosition);

	if(Dlg.ShowModal()==wxID_OK)
	{
		wxString Filename = Dlg.GetPath();
		UUnrealEdOptions* Options = GUnrealEd->GetUnrealEdOptions();

		if(Options)
		{
			UUnrealEdKeyBindings* KeyBindings = Options->EditorKeyBindings;

			if(KeyBindings)
			{
				KeyBindings->ReloadConfig(NULL, Filename);

				// Refresh binding text
				RefreshBindings();
			}
		}
	}
}

/** Handler to let the user save the current config to a file. */
void WxDlgBindHotkeys::OnSaveConfig(wxCommandEvent& Event)
{
	// Get the filename
	wxChar* FileTypes = TEXT("INI Files (*.ini)|*.ini|All Files (*.*)|*.*");

	WxFileDialog Dlg( this, 
		*LocalizeUnrealEd("SaveKeyConfig"), 
		*appGameConfigDir(),
		TEXT(""),
		FileTypes,
		wxSAVE | wxHIDE_READONLY,
		wxDefaultPosition);

	if(Dlg.ShowModal()==wxID_OK)
	{
		wxString Filename = Dlg.GetPath();
		UUnrealEdOptions* Options = GUnrealEd->GetUnrealEdOptions();

		if(Options)
		{
			UUnrealEdKeyBindings* KeyBindings = Options->EditorKeyBindings;

			if(KeyBindings)
			{
				KeyBindings->SaveConfig(CPF_Config, Filename);
			}
		}
	}
}

/** Handler to reset bindings to default. */
void WxDlgBindHotkeys::OnResetToDefaults(wxCommandEvent& Event)
{
	if(wxMessageBox(*LocalizeUnrealEd("AreYouSureYouWantDefaults"), TEXT(""), wxYES_NO | wxCENTRE, this))
	{
		// Load the user's settings
		UUnrealEdOptions* Options = GUnrealEd->GetUnrealEdOptions();

		if(Options)
		{
			UUnrealEdKeyBindings* KeyBindings = Options->EditorKeyBindings;

			if(KeyBindings)
			{
				KeyBindings->ReloadConfig();

				// Refresh binding text
				RefreshBindings();
			}
		}
	}
}

/** Bind key button pressed handler. */
void WxDlgBindHotkeys::OnBindKey(wxCommandEvent& Event)
{
	INT CommandIdx = Event.GetId() - ID_DLGBINDHOTKEYS_BIND_KEY_START;

	if(CommandIdx >=0 && CommandIdx < VisibleCommands.Num())
	{
		StartBinding(CommandIdx);
	}
	else
	{
		CurrentlyBindingIdx = -1;
	}
}

/** OK Button pressed handler. */
void WxDlgBindHotkeys::OnOK(wxCommandEvent &Event)
{
	SaveSettings();

	// Hide the dialog
	Hide();
}

/** Handler for key binding events. */
void WxDlgBindHotkeys::OnKeyDown(wxKeyEvent& Event)
{
	Event.Skip();
}

WxDlgBindHotkeys::~WxDlgBindHotkeys()
{
	
}

///////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------
	WxDlgMove
-----------------------------------------------------------------------------*/

WxDlgMove::WxDlgMove()
{
	const bool bSuccess = wxXmlResource::Get()->LoadDialog( this, GApp->EditorFrame, TEXT("ID_DLG_PKGGROUPS") );
	check( bSuccess );

	SetTitle( *LocalizeUnrealEd("MoveWithReferences") );

	PGNPanel = (wxPanel*)FindWindow( XRCID( "ID_PKGGRPNAME" ) );
	check( PGNPanel != NULL );

	//wxSizer* szr = PGNPanel->GetSizer();

	PGNSizer = new wxBoxSizer(wxHORIZONTAL);
	PGNCtrl = new WxPkgGrpNameTxtCtrl( PGNPanel, -1, PGNSizer );
	const wxRect rc = PGNPanel->GetClientRect();
	PGNCtrl->SetSizer(PGNSizer);
	PGNCtrl->Show();
	PGNCtrl->SetSize( rc );
	PGNCtrl->SetAutoLayout(true);

	PGNPanel->SetAutoLayout(true);

	PGNCtrl->NameLabel->Disable();
	PGNCtrl->NameEdit->Disable();

	FWindowUtil::LoadPosSize( TEXT("DlgMove"), this );
}

WxDlgMove::~WxDlgMove()
{
	FWindowUtil::SavePosSize( TEXT("DlgMove"), this );
}

int WxDlgMove::ShowModal(const FString& InPackage, const FString& InGroup, const FString& InName )
{
	OldPackage = NewPackage = InPackage;
	OldGroup = NewGroup = InGroup;
	OldName = NewName = InName;

	PGNCtrl->PkgCombo->SetValue( *InPackage );
	PGNCtrl->TxtPkgCombo->SetValue( TEXT("") );
	PGNCtrl->GrpEdit->SetValue( *InGroup );
	PGNCtrl->NameEdit->SetValue( *InName );

	return wxDialog::ShowModal();
}

bool WxDlgMove::Validate()
{
	NewPackage = PGNCtrl->PkgCombo->GetValue();
	NewGroup = PGNCtrl->GrpEdit->GetValue();
	NewName = PGNCtrl->NameEdit->GetValue();
	NewTxtPackage = PGNCtrl->TxtPkgCombo->GetValue();
	NewTxtGroup = PGNCtrl->TxtGrpEdit->GetValue();
	IncludeRefs = PGNCtrl->IncludeRefsCheck->GetValue();

	FString Reason;
	if( !FIsValidObjectName( *NewName, Reason ) )
	{
		appMsgf( AMT_OK, *Reason );
		return 0;
	}

	return 1;
}

/*-----------------------------------------------------------------------------
	WxDlgPackageGroupName.
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE(WxDlgPackageGroupName, wxDialog)
	EVT_BUTTON( wxID_OK, WxDlgPackageGroupName::OnOK )
END_EVENT_TABLE()

WxDlgPackageGroupName::WxDlgPackageGroupName()
{
	wxDialog::Create(NULL, wxID_ANY, TEXT("PackageGroupName"), wxDefaultPosition, wxDefaultSize );

	wxBoxSizer* HorizontalSizer = new wxBoxSizer(wxHORIZONTAL);
	{
		wxBoxSizer* InfoStaticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, this, _("Info"));
		{
			
			PGNCtrl = new WxPkgGrpNameCtrl( this, -1, NULL );
			PGNCtrl->SetSizer(PGNCtrl->FlexGridSizer);
			InfoStaticBoxSizer->Add(PGNCtrl, 1, wxEXPAND);
		}
		HorizontalSizer->Add(InfoStaticBoxSizer, 1, wxALIGN_TOP|wxALL|wxEXPAND, 5);
		
		wxBoxSizer* ButtonSizer = new wxBoxSizer(wxVERTICAL);
		{
			wxButton* ButtonOK = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
			ButtonOK->SetDefault();
			ButtonSizer->Add(ButtonOK, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

			wxButton* ButtonCancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
			ButtonSizer->Add(ButtonCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
		}
		HorizontalSizer->Add(ButtonSizer, 0, wxALIGN_TOP|wxALL, 5);
		
	}
	SetSizer(HorizontalSizer);
	
	FWindowUtil::LoadPosSize( TEXT("DlgPackageGroupName"), this );
	GetSizer()->Fit(this);

	FLocalizeWindow( this );
}

WxDlgPackageGroupName::~WxDlgPackageGroupName()
{
	FWindowUtil::SavePosSize( TEXT("DlgPackageGroupName"), this );
}

int WxDlgPackageGroupName::ShowModal(const FString& InPackage, const FString& InGroup, const FString& InName )
{
	Package = InPackage;
	Group = InGroup;
	Name = InName;

	PGNCtrl->PkgCombo->SetValue( *InPackage );
	PGNCtrl->GrpEdit->SetValue( *InGroup );
	PGNCtrl->NameEdit->SetValue( *InName );

	return wxDialog::ShowModal();
}

void WxDlgPackageGroupName::OnOK( wxCommandEvent& In )
{
	Package = PGNCtrl->PkgCombo->GetValue();
	Group = PGNCtrl->GrpEdit->GetValue();
	Name = PGNCtrl->NameEdit->GetValue();

	wxDialog::OnOK( In );
}

/*-----------------------------------------------------------------------------
	WxDlgNewArchetype.
-----------------------------------------------------------------------------*/

bool WxDlgNewArchetype::Validate()
{
	FString	QualifiedName;
	if( Group.Len() )
	{
		QualifiedName = Package + TEXT(".") + Group + TEXT(".") + Name;
	}
	else
	{
		QualifiedName = Package + TEXT(".") + Name;
	}

	// validate that the object name is valid
	FString Reason;
	if( !FIsValidObjectName( *Name, Reason ) || !FIsValidObjectName( *Package, Reason ) || !FIsValidGroupName( *Group, Reason ) || !FIsUniqueObjectName( *QualifiedName, ANY_PACKAGE, Reason ) )
	{
		appMsgf( AMT_OK, *Reason );
		return 0;
	}

	return 1;
}

/*-----------------------------------------------------------------------------
	WxDlgAddSpecial.
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE(WxDlgAddSpecial, wxDialog)
	EVT_BUTTON( wxID_OK, WxDlgAddSpecial::OnOK )
END_EVENT_TABLE()

WxDlgAddSpecial::WxDlgAddSpecial()
{
	const bool bSuccess = wxXmlResource::Get()->LoadDialog( this, GApp->EditorFrame, TEXT("ID_DLG_ADDSPECIAL") );
	check( bSuccess );

	PortalCheck = (wxCheckBox*)FindWindow( XRCID( "IDCK_PORTAL" ) );
	check( PortalCheck != NULL );
	InvisibleCheck = (wxCheckBox*)FindWindow( XRCID( "IDCK_INVISIBLE" ) );
	check( InvisibleCheck != NULL );
	TwoSidedCheck = (wxCheckBox*)FindWindow( XRCID( "IDCK_TWOSIDED" ) );
	check( TwoSidedCheck != NULL );
	SolidRadio = (wxRadioButton*)FindWindow( XRCID( "IDRB_SOLID" ) );
	check( SolidRadio != NULL );
	SemiSolidRadio = (wxRadioButton*)FindWindow( XRCID( "IDRB_SEMISOLID" ) );
	check( SemiSolidRadio != NULL );
	NonSolidRadio = (wxRadioButton*)FindWindow( XRCID( "IDRB_NONSOLID" ) );
	check( NonSolidRadio != NULL );

	//<@ ava specific ; 2006. 11. 27 changmin
	HintCheck = (wxCheckBox*)FindWindow( XRCID( "IDCK_HINT" ) );
	check( HintCheck != NULL );
	//>@ ava

	FWindowUtil::LoadPosSize( TEXT("DlgAddSpecial"), this );
	FLocalizeWindow( this );
}

WxDlgAddSpecial::~WxDlgAddSpecial()
{
	FWindowUtil::SavePosSize( TEXT("DlgAddSpecial"), this );
}

void WxDlgAddSpecial::OnOK( wxCommandEvent& In )
{
	INT Flags = 0;

	if( PortalCheck->GetValue() )		Flags |= PF_Portal;
	if( InvisibleCheck->GetValue() )	Flags |= PF_Invisible;
	if( TwoSidedCheck->GetValue() )		Flags |= PF_TwoSided;
	if( SemiSolidRadio->GetValue() )	Flags |= PF_Semisolid;
	if( NonSolidRadio->GetValue() )		Flags |= PF_NotSolid;

	//<@ ava specific ; 2006. 11. 27 changmin
	if( HintCheck->GetValue())			Flags |= PF_Hint;
	//>@ ava

	GUnrealEd->Exec( *FString::Printf(TEXT("BRUSH ADD FLAGS=%d"), Flags));

	wxDialog::OnOK( In );
}

/*-----------------------------------------------------------------------------
	WxDlgGenericStringEntry.
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE(WxDlgGenericStringEntry, wxDialog)
	EVT_BUTTON( wxID_OK, WxDlgGenericStringEntry::OnOK )
END_EVENT_TABLE()

WxDlgGenericStringEntry::WxDlgGenericStringEntry()
{
	const bool bSuccess = wxXmlResource::Get()->LoadDialog( this, GApp->EditorFrame, TEXT("ID_GENERICSTRINGENTRY") );
	check( bSuccess );

	StringEntry = (wxTextCtrl*)FindWindow( XRCID( "IDEC_STRINGENTRY" ) );
	check( StringEntry != NULL );
	StringCaption = (wxStaticText*)FindWindow( XRCID( "IDEC_STRINGCAPTION" ) );
	check( StringCaption != NULL );

	ADDEVENTHANDLER( XRCID("IDEC_STRINGENTRY"), wxEVT_COMMAND_TEXT_ENTER, &WxDlgGenericStringEntry::OnOK );

	EnteredString = FString( TEXT("") );

	FWindowUtil::LoadPosSize( TEXT("DlgGenericStringEntry"), this );
}

WxDlgGenericStringEntry::~WxDlgGenericStringEntry()
{
	FWindowUtil::SavePosSize( TEXT("DlgGenericStringEntry"), this );
}

int WxDlgGenericStringEntry::ShowModal(const TCHAR* DialogTitle, const TCHAR* Caption, const TCHAR* DefaultString)
{
	SetTitle( DialogTitle );
	StringCaption->SetLabel( Caption );

	FLocalizeWindow( this );

	StringEntry->SetValue( DefaultString );

	return wxDialog::ShowModal();
}

void WxDlgGenericStringEntry::OnOK( wxCommandEvent& In )
{
	EnteredString = StringEntry->GetValue();

	wxDialog::OnOK( In );
}

/*-----------------------------------------------------------------------------
	XDlgSurfPropPage1.
-----------------------------------------------------------------------------*/

WxSurfacePropertiesPanel::WxSurfacePropertiesPanel( wxWindow* InParent )
	:	wxPanel( InParent, -1 ),
		bUseSimpleScaling( TRUE )
{
	Panel = (wxPanel*)wxXmlResource::Get()->LoadPanel( this, TEXT("ID_SURFPROP_PANROTSCALE") );
	check( Panel != NULL );
	SimpleCB = (wxComboBox*)FindWindow( XRCID( "IDCB_SIMPLE" ) );
	check( SimpleCB != NULL );
	CustomULabel = (wxStaticText*)FindWindow( XRCID( "IDSC_CUSTOM_U" ) );
	check( CustomULabel != NULL );
	CustomVLabel = (wxStaticText*)FindWindow( XRCID( "IDSC_CUSTOM_V" ) );
	check( CustomVLabel != NULL );
	CustomUEdit = (wxTextCtrl*)FindWindow( XRCID( "IDEC_CUSTOM_U" ) );
	check( CustomUEdit != NULL );
	CustomVEdit = (wxTextCtrl*)FindWindow( XRCID( "IDEC_CUSTOM_V" ) );
	check( CustomVEdit != NULL );
	SimpleScaleButton = (wxRadioButton*)FindWindow( XRCID( "IDRB_SIMPLE" ) );
	check( SimpleScaleButton != NULL );
	CustomScaleButton = (wxRadioButton*)FindWindow( XRCID( "IDRB_CUSTOM" ) );
	check( CustomScaleButton != NULL );
	RelativeCheck = (wxCheckBox*)FindWindow( XRCID( "IDCK_RELATIVE" ) );
	check( RelativeCheck != NULL );
	LightMapResCombo = (wxComboBox*)FindWindow( XRCID( "IDCB_LIGHTMAPRES" ) );
	check( LightMapResCombo != NULL );
	AcceptsLightsCheck = (wxCheckBox*)FindWindow( XRCID( "IDCK_ACCEPTSLIGHTS" ) );
	check( AcceptsLightsCheck != NULL );
	AcceptsDynamicLightsCheck = (wxCheckBox*)FindWindow( XRCID( "IDCK_ACCEPTSDYNAMICLIGHTS" ) );
	check( AcceptsDynamicLightsCheck != NULL );
	ForceLightMapCheck = (wxCheckBox*)FindWindow( XRCID( "IDCK_FORCELIGHTMAP" ) );
	check( ForceLightMapCheck != NULL );

	//<@ ava specific ; changmin
	// 2007. 8. 14
	SecondaryLightSourceCheck = (wxCheckBox*)FindWindow( XRCID( "IDCK_SECONDARYLIGHTSOURCE" ) );
	check( SecondaryLightSourceCheck != NULL );
	// 2007. 12. 26
	ForceCastSunShadowCheck = (wxCheckBox*)FindWindow( XRCID("IDCK_FORCECASTSUNSHADOW") );
	check( ForceCastSunShadowCheck != NULL )
	//>@ ava

	//<@ ava specific ; 2007. 8. 27 changmin
	SurfaceMappingInfo = (wxStaticText*)FindWindow( XRCID("AvaID_MappingInfo") );
	check( SurfaceMappingInfo != NULL );
	//>@ ava

	SimpleScaleButton->SetValue( 1 );

	ADDEVENTHANDLER( XRCID("IDPB_U_1"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnU1 );
	ADDEVENTHANDLER( XRCID("IDPB_U_4"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnU4 );
	ADDEVENTHANDLER( XRCID("IDPB_U_16"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnU16 );
	ADDEVENTHANDLER( XRCID("IDPB_U_64"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnU64 );
	ADDEVENTHANDLER( XRCID("IDPB_U_CUSTOM"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnUCustom );

	ADDEVENTHANDLER( XRCID("IDPB_V_1"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnV1 );
	ADDEVENTHANDLER( XRCID("IDPB_V_4"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnV4 );
	ADDEVENTHANDLER( XRCID("IDPB_V_16"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnV16 );
	ADDEVENTHANDLER( XRCID("IDPB_V_64"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnV64 );
	ADDEVENTHANDLER( XRCID("IDPB_V_CUSTOM"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnVCustom );

	ADDEVENTHANDLER( XRCID("IDPB_ROT_45"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnRot45 );
	ADDEVENTHANDLER( XRCID("IDPB_ROT_90"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnRot90 );
	ADDEVENTHANDLER( XRCID("IDPB_ROT_CUSTOM"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnRotCustom );

	ADDEVENTHANDLER( XRCID("IDPB_FLIP_U"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnFlipU );
	ADDEVENTHANDLER( XRCID("IDPB_FLIP_V"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnFlipV );
	
	ADDEVENTHANDLER( XRCID("IDPB_APPLY"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnApply );
	ADDEVENTHANDLER( XRCID("IDRB_SIMPLE"), wxEVT_COMMAND_RADIOBUTTON_SELECTED, &WxSurfacePropertiesPanel::OnScaleSimple );
	ADDEVENTHANDLER( XRCID("IDRB_CUSTOM"), wxEVT_COMMAND_RADIOBUTTON_SELECTED, &WxSurfacePropertiesPanel::OnScaleCustom );
	ADDEVENTHANDLER( XRCID("IDCB_LIGHTMAPRES"), wxEVT_COMMAND_COMBOBOX_SELECTED, &WxSurfacePropertiesPanel::OnLightMapResSelChange );
	ADDEVENTHANDLER( XRCID("IDCK_ACCEPTSLIGHTS"), wxEVT_COMMAND_CHECKBOX_CLICKED, &WxSurfacePropertiesPanel::OnAcceptsLightsChange );
	ADDEVENTHANDLER( XRCID("IDCK_ACCEPTSDYNAMICLIGHTS"), wxEVT_COMMAND_CHECKBOX_CLICKED, &WxSurfacePropertiesPanel::OnAcceptsDynamicLightsChange );
	ADDEVENTHANDLER( XRCID("IDCK_FORCELIGHTMAP"), wxEVT_COMMAND_CHECKBOX_CLICKED, &WxSurfacePropertiesPanel::OnForceLightmapChange );

	//<@ ava specific ;changmin
	// 2007. 8. 14 : Secondary Light source
	ADDEVENTHANDLER( XRCID("IDCK_SECONDARYLIGHTSOURCE"), wxEVT_COMMAND_CHECKBOX_CLICKED, &WxSurfacePropertiesPanel::OnSecondaryLightSourceChange );
	// 2007. 12. 26 : Cast Realtime Sun Shadow
	ADDEVENTHANDLER( XRCID("IDCK_FORCECASTSUNSHADOW"), wxEVT_COMMAND_CHECKBOX_CLICKED, &WxSurfacePropertiesPanel::OnForceCastSunShadowChange );

	//>@ ava

	CustomULabel->Disable();
	CustomVLabel->Disable();
	CustomUEdit->Disable();
	CustomVEdit->Disable();

	
	// Setup alignment properties.
	AlignList = (wxListBox*)FindWindow( XRCID( "IDLB_ALIGNMENT" ) );
	check( AlignList );

	ADDEVENTHANDLER( XRCID("IDLB_ALIGNMENT"), wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, &WxSurfacePropertiesPanel::OnApplyAlign );
	ADDEVENTHANDLER( XRCID("IDLB_ALIGNMENT"), wxEVT_COMMAND_LISTBOX_SELECTED, &WxSurfacePropertiesPanel::OnAlignSelChange );
	ADDEVENTHANDLER( XRCID("IDPB_APPLYALIGN"), wxEVT_COMMAND_BUTTON_CLICKED, &WxSurfacePropertiesPanel::OnApplyAlign);

	// Initialize controls.
	for( INT x = 0 ; x < GTexAlignTools.Aligners.Num() ; ++x )
	{
		AlignList->Append( *GTexAlignTools.Aligners(x)->Desc );
	}
	AlignList->SetSelection( 0 );



	wxPanel* PropertyPanel;
	PropertyPanel = (wxPanel*)FindWindow( XRCID( "ID_PROPERTYWINDOW_PANEL" ) );
	check( PropertyPanel );

	wxStaticBoxSizer* PropertySizer = new wxStaticBoxSizer(wxVERTICAL, PropertyPanel, TEXT("Options"));
	{
		PropertyWindow = new WxPropertyWindow;
		PropertyWindow->Create( PropertyPanel, GUnrealEd );
		
		wxSize PanelSize = PropertyPanel->GetSize();
		PanelSize.SetWidth(PanelSize.GetWidth() - 5);
		PanelSize.SetHeight(PanelSize.GetHeight() - 15);
		PropertyWindow->SetMinSize(PanelSize);

		PropertySizer->Add(PropertyWindow, 1, wxEXPAND);
	}
	PropertyPanel->SetSizer(PropertySizer);
	PropertySizer->Fit(PropertyPanel);
}

void WxSurfacePropertiesPanel::Pan( INT InU, INT InV )
{
	const FLOAT Mod = GetAsyncKeyState(VK_SHIFT) & 0x8000 ? -1.f : 1.f;
	GUnrealEd->Exec( *FString::Printf( TEXT("POLY TEXPAN U=%f V=%f"), InU * Mod, InV * Mod ) );
}

void WxSurfacePropertiesPanel::Scale( FLOAT InScaleU, FLOAT InScaleV, UBOOL InRelative )
{
	if( InScaleU == 0.f )
	{
		InScaleU = 1.f;
	}
	if( InScaleV == 0.f )
	{
		InScaleV = 1.f;
	}

	InScaleU = 1.0f / InScaleU;
	InScaleV = 1.0f / InScaleV;

	GUnrealEd->Exec( *FString::Printf( TEXT("POLY TEXSCALE %s UU=%f VV=%f"), InRelative?TEXT("RELATIVE"):TEXT(""), InScaleU, InScaleV ) );
}


void WxSurfacePropertiesPanel::RefreshPage()
{
	// Find the shadowmap scale on the selected surfaces.

	FLOAT	ShadowMapScale				= 0.0f;
	UBOOL	bValidScale					= FALSE;
	// Keep track of how many surfaces are selected and how many have the options set.
	INT		AcceptsLightsCount			= 0;
	INT		AcceptsDynamicLightsCount	= 0;
	INT		ForceLightMapCount			= 0;
	INT		SelectedSurfaceCount		= 0;

	//<@ ava specific ;  changmin
	// 2007. 8. 14 ; Secondray Light Source
	INT		SecondaryLightSourceCount	= 0;
	// 2007. 12. 26 ; Cast Realtime Sun Shadow
	INT		ForceCastSunShadowCount		= 0;
	//>@ ava

	//<@ ava specific ; 2007. 8. 27 changmin
	FLOAT	Ava_UScale = 1.0f;
	FLOAT	Ava_VScale = 1.0f;
	FLOAT	Ava_BoxMappingDegree = 0.0f;
	FLOAT	Ava_DefaultMappingDegree = 0.0f;
	FLOAT	Ava_PlanerMappingDegree = 0.0f;
	UBOOL	Ava_bFlipBox = FALSE;
	UBOOL	Ava_bFlipDefault = FALSE;
	UBOOL	Ava_bFlipPlaner = FALSE;
	UBOOL	Ava_bPlanerMappingOnly = FALSE;
	//>@ ava

	if ( GWorld )
	{
		for ( INT LevelIndex = 0 ; LevelIndex < GWorld->Levels.Num() ; ++LevelIndex )
		{
			const ULevel* Level = GWorld->Levels(LevelIndex);
			const UModel* Model = Level->Model;
			for(INT SurfaceIndex = 0; SurfaceIndex < Model->Surfs.Num();SurfaceIndex++)
			{
				const FBspSurf&	Surf =  Model->Surfs(SurfaceIndex);

				if(Surf.PolyFlags & PF_Selected)
				{
					if(SelectedSurfaceCount == 0)
					{
						ShadowMapScale = Surf.ShadowMapScale;
						bValidScale = TRUE;

						//<@ ava specific ; 2007. 8. 27 changmin
						FVector TextureU( Model->Vectors(Surf.vTextureU) );
						FVector TextureV( Model->Vectors(Surf.vTextureV) );

						Ava_UScale = TextureU.Size();
						Ava_VScale = TextureV.Size();

						const FVector TangentNormal = TextureU.SafeNormal() ^ TextureV.SafeNormal();

						if( Abs(TangentNormal | FVector( 1, 0, 0 )) >= 0.9999f
						||	Abs(TangentNormal | FVector( 0, 1, 0 )) >= 0.9999f
						||	Abs(TangentNormal | FVector( 0, 0, 1 )) >= 0.9999f )
						{
							Ava_bPlanerMappingOnly = TRUE;
						}

						const FVector Normal( Model->Vectors(Surf.vNormal) );
						//if( (TangentNormal | Normal) < 0.9999f )
						//{
						//	Ava_bPlanerMappingOnly = TRUE;
						//}

						if( Ava_bPlanerMappingOnly )
						{
							FVector U, V;
							INT Axis = TAXIS_X;
							if( Abs(Normal.Y) >= 0.5f )
							{
								Axis = TAXIS_Y;
							}
							else if ( Abs(Normal.Z) >= 0.5f )
							{
								Axis = TAXIS_Z;
							}

							UBOOL bFlip = FALSE;
							if( Axis == TAXIS_X && Normal.X < 0 )
							{
								bFlip = TRUE;
							}
							if( Axis == TAXIS_Y && Normal.Y < 0 )
							{
								bFlip = TRUE;
							}
							if( Axis == TAXIS_Z && Normal.Z < 0 )
							{
								bFlip = TRUE;
							}
							if( Axis == TAXIS_X )
							{
								U = FVector(0, (bFlip ? 1 : -1) ,0);
								V = FVector(0,0,-1);
							}
							else if( Axis == TAXIS_Y )
							{
								U = FVector((bFlip ? -1 : 1),0,0);
								V = FVector(0,0,-1);
							}
							else
							{
								U = FVector((bFlip ? 1 : -1),0,0);
								V = FVector(0,-1,0);
							}

							Ava_bFlipPlaner = ( (U ^ V) | (TextureU.SafeNormal() ^ TextureV.SafeNormal()) ) < 0.0f;
							if( Ava_bFlipPlaner )
							{
								U *= -1.0f;
							}
							UBOOL PositiveV = (V | TextureU.SafeNormal()) >= 0.0f;
							FLOAT Degree = appAcos( Clamp<FLOAT>( U | TextureU.SafeNormal(), -1.0f, 1.0f ) ) * 180.0f / M_PI;
							if( PositiveV && Degree > 0.5f )
							{
								Ava_PlanerMappingDegree = 360.0f - Degree;
							}
							else
							{
								Ava_PlanerMappingDegree = Degree;
							}

						}
						else
						{
							// Box mapping 회전값...
							FVector U, V;
							Normal.FindBestAxisVectors( V, U );
							U *= -1.0;
							V *= -1.0;

							UBOOL bFlip = FALSE;
							Ava_bFlipBox = bFlip = ( (U ^ V) | (TextureU.SafeNormal() ^ TextureV.SafeNormal()) ) < 0.0f;
							if( bFlip )
							{
								U *= -1.0f;
							}

							UBOOL PositiveV = (V | TextureU.SafeNormal()) >= 0.0f;
							FLOAT Degree = appAcos( Clamp<FLOAT>( U | TextureU.SafeNormal(), -1.0f, 1.0f ) ) * 180.0f / M_PI;
							if( PositiveV && Degree > 0.5f )
							{
								Ava_BoxMappingDegree = 360.0f - Degree;
							}
							else
							{
								Ava_BoxMappingDegree = Degree;
							}

							// Default mapping 회전값...
							// Defaut 랑 Box Mapping은 서로 호환가능하므로...default mapping 제외..
							FPoly EdPoly;
							GEditor->polyFindMaster( GWorld->Levels(LevelIndex)->Model, SurfaceIndex, EdPoly );
							EdPoly.Base = FVector(0,0,0);
							EdPoly.TextureU = FVector(0,0,0);
							EdPoly.TextureV = FVector(0,0,0);
							EdPoly.Finalize( NULL, 0 );
							EdPoly.Transform( FVector(0,0,0), FVector(0,0,0) );
							Ava_bFlipDefault = bFlip = ((EdPoly.TextureU ^ EdPoly.TextureV) | ( TextureU.SafeNormal() ^ TextureV.SafeNormal() )) < 0.0f;
							if( bFlip )
							{
								EdPoly.TextureU *= -1.0f;
							}

							PositiveV = ( EdPoly.TextureV | TextureU.SafeNormal() ) >= 0.0f;
							Degree = appAcos(Clamp<FLOAT>( EdPoly.TextureU.SafeNormal() | TextureU.SafeNormal(), -1.0f, 1.0f ) ) * 180.0f / M_PI;
							if( PositiveV && Degree > 0.5f )
							{
								Ava_DefaultMappingDegree = 360.0f - Degree;
							}
							else
							{
								Ava_DefaultMappingDegree = Degree;
							}
						}
						//>@ ava
					}
					else if(bValidScale && ShadowMapScale != Surf.ShadowMapScale)
					{
						bValidScale = FALSE;
					}

					// Keep track of selected surface count and how many have the particular options set so we can display
					// the correct state of checked, unchecked or mixed.
					SelectedSurfaceCount++;
					if( Surf.PolyFlags & PF_AcceptsLights )
					{
						AcceptsLightsCount++;
					}
					if( Surf.PolyFlags & PF_AcceptsDynamicLights )
					{
						AcceptsDynamicLightsCount++;
					}
					if( Surf.PolyFlags & PF_ForceLightMap )
					{
						ForceLightMapCount++;
					}

					//<@ ava specific ; changmin
					// 2007. 8. 14 ; Secondary Light Source
					if( Surf.PolyFlags & PF_SecondaryLightSource )
					{
						SecondaryLightSourceCount++;
					}
					// 2007. 12. 26 ; Cast Realtime Sun Shadow
					if( Surf.PolyFlags & PF_ForceCastSunShadow )
					{
						ForceCastSunShadowCount++;
					}
					//>@ ava
				}
			}
		}
	}

	// Select the appropriate scale.
	INT	ScaleItem = INDEX_NONE;

	if( bValidScale )
	{
		const FString ScaleString = FString::Printf(TEXT("%.1f"),ShadowMapScale);

		ScaleItem = LightMapResCombo->FindString(*ScaleString);

		if(ScaleItem == INDEX_NONE)
		{
			ScaleItem = LightMapResCombo->GetCount();
			LightMapResCombo->Append( *ScaleString );
		}
	}

	LightMapResCombo->SetSelection( ScaleItem );

	// Set AcceptsLights state.
	if( AcceptsLightsCount == 0 )
	{
		AcceptsLightsCheck->Set3StateValue( wxCHK_UNCHECKED );
	}
	else if( AcceptsLightsCount == SelectedSurfaceCount )
	{
		AcceptsLightsCheck->Set3StateValue( wxCHK_CHECKED );
	}
	else
	{
		AcceptsLightsCheck->Set3StateValue( wxCHK_UNDETERMINED );
	}

	// Set AcceptsDynamicLights state.
	if( AcceptsDynamicLightsCount == 0 )
	{
		AcceptsDynamicLightsCheck->Set3StateValue( wxCHK_UNCHECKED );
	}
	else if( AcceptsDynamicLightsCount == SelectedSurfaceCount )
	{
		AcceptsDynamicLightsCheck->Set3StateValue( wxCHK_CHECKED );
	}
	else
	{
		AcceptsDynamicLightsCheck->Set3StateValue( wxCHK_UNDETERMINED );
	}

	// Set ForceLightMap state.
	if( ForceLightMapCount == 0 )
	{
		ForceLightMapCheck->Set3StateValue( wxCHK_UNCHECKED );
	}
	else if( ForceLightMapCount == SelectedSurfaceCount )
	{
		ForceLightMapCheck->Set3StateValue( wxCHK_CHECKED );
	}
	else
	{
		ForceLightMapCheck->Set3StateValue( wxCHK_UNDETERMINED );
	}

	//<@ ava specific ; changmin
	// 2007. 8. 14 ; Secondary Light Source
	if( SecondaryLightSourceCount == 0 )
	{
		SecondaryLightSourceCheck->Set3StateValue( wxCHK_UNCHECKED );
	}
	else if( SecondaryLightSourceCount == SelectedSurfaceCount )
	{
		SecondaryLightSourceCheck->Set3StateValue( wxCHK_CHECKED );
	}
	else
	{
		SecondaryLightSourceCheck->Set3StateValue( wxCHK_UNDETERMINED );
	}
	// 2007. 12. 26 ; Cast Realtime Sun Shadow
	if( ForceCastSunShadowCount == 0 )
	{
		ForceCastSunShadowCheck->Set3StateValue( wxCHK_UNCHECKED );
	}
	else if( ForceCastSunShadowCount == SelectedSurfaceCount )
	{
		ForceCastSunShadowCheck->Set3StateValue( wxCHK_CHECKED );
	}
	else
	{
		ForceCastSunShadowCheck->Set3StateValue( wxCHK_UNDETERMINED );
	}
	//>@ ava

	//<@ ava specific ; 2007. 8. 27 changmin
	if( SelectedSurfaceCount > 0 )
	{
		if( Ava_bPlanerMappingOnly )
		{
			SurfaceMappingInfo->SetLabel( *FString::Printf(*LocalizeUnrealEd(TEXT("Ava_PlanerSurfaceMappingInfoF")),
				1.0f/Ava_UScale, 1.0f/Ava_VScale, Ava_UScale, Ava_VScale,
				Ava_bFlipPlaner ? TEXT("(FlipU) and Custum") : TEXT("Custum"), Ava_PlanerMappingDegree) ) ;
		}
		else
		{
			SurfaceMappingInfo->SetLabel( *FString::Printf(*LocalizeUnrealEd(TEXT("Ava_BoxSurfaceMappingInfoF")),
				1.0f/Ava_UScale, 1.0f/Ava_VScale, Ava_UScale, Ava_VScale,
				Ava_bFlipBox ? TEXT("(FlipU) and Custum") : TEXT("Custum"), Ava_BoxMappingDegree ) );
		}
	}
	//>@ ava

	// Refresh property window.
	PropertyWindow->SetObject( GTexAlignTools.Aligners( AlignList->GetSelection() ), 1,0,0 );
}


void WxSurfacePropertiesPanel::OnU1( wxCommandEvent& In )
{
	Pan( 1, 0 );
}

void WxSurfacePropertiesPanel::OnU4( wxCommandEvent& In )
{
	Pan( 4, 0 );
}

void WxSurfacePropertiesPanel::OnU16( wxCommandEvent& In )
{
	Pan( 16, 0 );
}

void WxSurfacePropertiesPanel::OnU64( wxCommandEvent& In )
{
	Pan( 64, 0 );
}

void WxSurfacePropertiesPanel::OnUCustom( wxCommandEvent& In )
{
	wxTextEntryDialog Dlg(this, *LocalizeUnrealEd(TEXT("UPanAmount")), *LocalizeUnrealEd(TEXT("PanU")));

	if(Dlg.ShowModal() == wxID_OK)
	{
		const wxString StrValue = Dlg.GetValue();
		const INT PanValue = appAtoi(StrValue);

		Pan( PanValue, 0 );
	}

}

void WxSurfacePropertiesPanel::OnV1( wxCommandEvent& In )
{
	Pan( 0, 1 );
}

void WxSurfacePropertiesPanel::OnV4( wxCommandEvent& In )
{
	Pan( 0, 4 );
}

void WxSurfacePropertiesPanel::OnV16( wxCommandEvent& In )
{
	Pan( 0, 16 );
}

void WxSurfacePropertiesPanel::OnV64( wxCommandEvent& In )
{
	Pan( 0, 64 );
}

void WxSurfacePropertiesPanel::OnVCustom( wxCommandEvent& In )
{
	wxTextEntryDialog Dlg(this, *LocalizeUnrealEd(TEXT("VPanAmount")), *LocalizeUnrealEd(TEXT("PanV")));

	if(Dlg.ShowModal() == wxID_OK)
	{
		const wxString StrValue = Dlg.GetValue();
		const INT PanValue = appAtoi(StrValue);

		Pan( 0, PanValue );
	}
}

void WxSurfacePropertiesPanel::OnFlipU( wxCommandEvent& In )
{
	GUnrealEd->Exec( TEXT("POLY TEXMULT UU=-1 VV=1") );
}

void WxSurfacePropertiesPanel::OnFlipV( wxCommandEvent& In )
{
	GUnrealEd->Exec( TEXT("POLY TEXMULT UU=1 VV=-1") );
}

void WxSurfacePropertiesPanel::OnRot45( wxCommandEvent& In )
{
	const FLOAT Mod = GetAsyncKeyState(VK_SHIFT) & 0x8000 ? -1.f : 1.f;
	const FLOAT UU = 1.0f / appSqrt(2.f);
	const FLOAT VV = 1.0f / appSqrt(2.f);
	const FLOAT UV = (1.0f / appSqrt(2.f)) * Mod;
	const FLOAT VU = -(1.0f / appSqrt(2.f)) * Mod;
	GUnrealEd->Exec( *FString::Printf( TEXT("POLY TEXMULT UU=%f VV=%f UV=%f VU=%f"), UU, VV, UV, VU ) );
}

void WxSurfacePropertiesPanel::OnRot90( wxCommandEvent& In )
{
	const FLOAT Mod = GetAsyncKeyState(VK_SHIFT) & 0x8000 ? -1 : 1;
	const FLOAT UU = 0.f;
	const FLOAT VV = 0.f;
	const FLOAT UV = 1.f * Mod;
	const FLOAT VU = -1.f * Mod;
	GUnrealEd->Exec( *FString::Printf( TEXT("POLY TEXMULT UU=%f VV=%f UV=%f VU=%f"), UU, VV, UV, VU ) );
}

void WxSurfacePropertiesPanel::OnRotCustom( wxCommandEvent& In )
{
	wxTextEntryDialog Dlg(this, *LocalizeUnrealEd(TEXT("RotationAmount")), *LocalizeUnrealEd(TEXT("Rotation")));

	if(Dlg.ShowModal() == wxID_OK)
	{
		wxString StrValue = Dlg.GetValue();
		const FLOAT RotateDegrees = appAtof(StrValue);
		const FLOAT RotateRadians = RotateDegrees / 180.0f * PI;

		const FLOAT UU = cos(RotateRadians);
		const FLOAT VV = UU;
		const FLOAT UV = -sin(RotateRadians);
		const FLOAT VU = sin(RotateRadians);
		GUnrealEd->Exec( *FString::Printf( TEXT("POLY TEXMULT UU=%f VV=%f UV=%f VU=%f"), UU, VV, UV, VU ) );
	}


}

void WxSurfacePropertiesPanel::OnApply( wxCommandEvent& In )
{
	FLOAT UScale, VScale;

	if( bUseSimpleScaling )
	{
		UScale = VScale = appAtof( SimpleCB->GetValue() );
	}
	else
	{
		UScale = appAtof( CustomUEdit->GetValue() );
		VScale = appAtof( CustomVEdit->GetValue() );
	}

	const UBOOL bRelative = RelativeCheck->GetValue();
	Scale( UScale, VScale, bRelative );
}

void WxSurfacePropertiesPanel::OnScaleSimple( wxCommandEvent& In )
{
	bUseSimpleScaling = TRUE;

	CustomULabel->Disable();
	CustomVLabel->Disable();
	CustomUEdit->Disable();
	CustomVEdit->Disable();
	SimpleCB->Enable();
}

void WxSurfacePropertiesPanel::OnScaleCustom( wxCommandEvent& In )
{
	bUseSimpleScaling = FALSE;

	CustomULabel->Enable();
	CustomVLabel->Enable();
	CustomUEdit->Enable();
	CustomVEdit->Enable();
	SimpleCB->Disable();
}

void WxSurfacePropertiesPanel::OnLightMapResSelChange( wxCommandEvent& In )
{
	const FLOAT ShadowMapScale = appAtof(LightMapResCombo->GetString(LightMapResCombo->GetSelection()));

	for ( INT LevelIndex = 0 ; LevelIndex < GWorld->Levels.Num() ; ++LevelIndex )
	{
		ULevel* Level = GWorld->Levels(LevelIndex);
		UModel* Model = Level->Model;
		for(INT SurfaceIndex = 0;SurfaceIndex < Model->Surfs.Num();SurfaceIndex++)
		{
			FBspSurf&	Surf = Model->Surfs(SurfaceIndex);

			if(Surf.PolyFlags & PF_Selected)
			{
				Surf.Actor->Brush->Polys->Element(Surf.iBrushPoly).ShadowMapScale = ShadowMapScale;
				Surf.ShadowMapScale = ShadowMapScale;
			}
		}
	}

	GWorld->MarkPackageDirty();
	GCallbackEvent->Send( CALLBACK_LevelDirtied );
}

/**
 * Sets passed in poly flag on selected surfaces.
 *
 * @param PolyFlag	PolyFlag to toggle on selected surfaces
 * @param Value		Value to set the flag to.
 */
void WxSurfacePropertiesPanel::SetPolyFlag( DWORD PolyFlag, UBOOL Value )
{
	//<@ ava specific ; 2007. 8. 14 changmin
	// Add Secondary Light Source
	// Surface property는 PF_ModelCompoentMask와 SecondaryLightSource Flag가 저장된다.
	check( PolyFlag & ( PF_ModelComponentMask | PF_SecondaryLightSource | PF_ForceCastSunShadow ) );
	//>@ ava
	//check( PolyFlag & PF_ModelComponentMask );
	{
		const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("ToggleSurfaceFlags")) );
		for ( INT LevelIndex = 0 ; LevelIndex < GWorld->Levels.Num() ; ++LevelIndex )
		{
			ULevel* Level = GWorld->Levels(LevelIndex);
			UModel* Model = Level->Model;
			Model->ModifySelectedSurfs( TRUE );

			for( INT SurfaceIndex=0; SurfaceIndex<Model->Surfs.Num(); SurfaceIndex++ )
			{
				FBspSurf& Surf = Model->Surfs(SurfaceIndex);
				if( Surf.PolyFlags & PF_Selected )
				{
					if(Value)
					{
						Surf.PolyFlags |= PolyFlag;
					}
					else
					{
						Surf.PolyFlags &= ~PolyFlag;
					}

					// Propagate toggled flags to poly.
					GEditor->polyUpdateMaster( Model, SurfaceIndex, TRUE );
				}
			}
		}
	}

	GWorld->MarkPackageDirty();
	GCallbackEvent->Send( CALLBACK_LevelDirtied );
}

void WxSurfacePropertiesPanel::OnAcceptsLightsChange( wxCommandEvent& In )
{
	SetPolyFlag( PF_AcceptsLights,  In.IsChecked());
}
void WxSurfacePropertiesPanel::OnAcceptsDynamicLightsChange( wxCommandEvent& In )
{
	SetPolyFlag( PF_AcceptsDynamicLights,  In.IsChecked() );
}
void WxSurfacePropertiesPanel::OnForceLightmapChange( wxCommandEvent& In )
{
	SetPolyFlag( PF_ForceLightMap,  In.IsChecked() );
}

//<@ ava specific ; changmin
// 2007. 8. 14 ; Secondary Light Source
void WxSurfacePropertiesPanel::OnSecondaryLightSourceChange( wxCommandEvent& In )
{
	SetPolyFlag( PF_SecondaryLightSource, In.IsChecked() );
}
// 2007. 12. 26 ; Add Cast Realtime Sun Shadow
void WxSurfacePropertiesPanel::OnForceCastSunShadowChange( wxCommandEvent& In )
{
	SetPolyFlag( PF_ForceCastSunShadow, In.IsChecked() );
}
//>@ ava



void WxSurfacePropertiesPanel::OnAlignSelChange( wxCommandEvent& In )
{
	RefreshPage();
}

void WxSurfacePropertiesPanel::OnApplyAlign( wxCommandEvent& In )
{
	UTexAligner* SelectedAligner = GTexAlignTools.Aligners( AlignList->GetSelection() );
	for ( INT LevelIndex = 0 ; LevelIndex < GWorld->Levels.Num() ; ++LevelIndex )
	{
		ULevel* Level = GWorld->Levels(LevelIndex);
		SelectedAligner->Align( TEXALIGN_None, Level->Model );
	}
}

/*-----------------------------------------------------------------------------
	WxDlgSurfaceProperties.
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE(WxDlgSurfaceProperties, wxDialog)
END_EVENT_TABLE()

WxDlgSurfaceProperties::WxDlgSurfaceProperties() : 
wxDialog(GApp->EditorFrame, wxID_ANY, TEXT("SurfaceProperties"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxSYSTEM_MENU | wxCAPTION)
{

	wxBoxSizer* DialogSizer = new wxBoxSizer(wxVERTICAL);
	{
		PropertiesPanel = new WxSurfacePropertiesPanel( this );
		DialogSizer->Add( PropertiesPanel, 1, wxEXPAND );
	}
	SetSizer(DialogSizer);

	
	FWindowUtil::LoadPosSize( TEXT("SurfaceProperties"), this );

	Fit();
	RefreshPages();
	FLocalizeWindow( this );
}

WxDlgSurfaceProperties::~WxDlgSurfaceProperties()
{
	FWindowUtil::SavePosSize( TEXT("SurfaceProperties"), this );
}

void WxDlgSurfaceProperties::RefreshPages()
{
	PropertiesPanel->RefreshPage();
}

/*-----------------------------------------------------------------------------
	WxDlgColor.
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE(WxDlgColor, wxColourDialog)
EVT_BUTTON( wxID_OK, WxDlgColor::OnOK )
END_EVENT_TABLE()

WxDlgColor::WxDlgColor()
{
}

WxDlgColor::~WxDlgColor()
{
	wxColourData cd = GetColourData();
	SaveColorData( &cd );
}

bool WxDlgColor::Create( wxWindow* InParent, wxColourData* InData )
{
	LoadColorData( InData );

	return wxColourDialog::Create( InParent, InData );
}

/**
* Loads custom color data from the INI file.
*
* @param	InData	The color data structure to load into
*/

void WxDlgColor::LoadColorData( wxColourData* InData )
{
	for( INT x = 0 ; x < 16 ; ++x )
	{
		const FString Key = FString::Printf( TEXT("Color%d"), x );
		INT Color = wxColour(255,255,255).GetPixel();
		GConfig->GetInt( TEXT("ColorDialog"), *Key, Color, GEditorIni );
		InData->SetCustomColour( x, wxColour( Color ) );
	}
}

/**
* Saves custom color data to the INI file.
*
* @param	InData	The color data structure to save from
*/

void WxDlgColor::SaveColorData( wxColourData* InData )
{
	for( INT x = 0 ; x < 16 ; ++x )
	{
		const FString Key = FString::Printf( TEXT("Color%d"), x );
		GConfig->SetInt( TEXT("ColorDialog"), *Key, InData->GetCustomColour(x).GetPixel(), GEditorIni );
	}
}

/*-----------------------------------------------------------------------------
	WxDlgSelectGroup.
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE(WxDlgSelectGroup, wxDialog)
	EVT_BUTTON( wxID_OK, WxDlgSelectGroup::OnOK )
END_EVENT_TABLE()

WxDlgSelectGroup::WxDlgSelectGroup( wxWindow* InParent )
{
	const bool bSuccess = wxXmlResource::Get()->LoadDialog( this, InParent, TEXT("ID_DLG_SELECTGROUP") );
	check( bSuccess );

	TreeCtrl = wxDynamicCast( FindWindow( XRCID( "IDTC_GROUPS" ) ), wxTreeCtrl );
	check( TreeCtrl != NULL );

	bShowAllPackages = 0;
	ADDEVENTHANDLER( XRCID("IDCK_SHOW_ALL"), wxEVT_COMMAND_CHECKBOX_CLICKED , &WxDlgSelectGroup::OnShowAllPackages );

	FLocalizeWindow( this );
}

WxDlgSelectGroup::~WxDlgSelectGroup()
{
}

int WxDlgSelectGroup::ShowModal( FString InPackageName, FString InGroupName )
{
	// Find the package objects

	RootPackage = Cast<UPackage>(GEngine->StaticFindObject( UPackage::StaticClass(), ANY_PACKAGE, *InPackageName ) );

	if( RootPackage == NULL )
	{
		bShowAllPackages = 1;
	}

	UpdateTree();

	return wxDialog::ShowModal();
}

/**
* Loads the tree control with a list of groups and/or packages.
*/

void WxDlgSelectGroup::UpdateTree()
{
	UPackage* Root = RootPackage;
	if( bShowAllPackages )
	{
		Root = NULL;
	}

	// Load tree control with the groups associated with this package.

	TreeCtrl->DeleteAllItems();

	const wxTreeItemId id = TreeCtrl->AddRoot( Root ? *Root->GetName() : TEXT("Packages"), -1, -1, new WxTreeObjectWrapper( Root ) );

	AddChildren( Root, id );

	TreeCtrl->SortChildren( id );
	TreeCtrl->Expand( id );
}

/**
* Adds the child packages of InPackage to the tree.
*
* @param	InPackage	The package to add children of
* @param	InId		The tree item id to add the children underneath
*/

void WxDlgSelectGroup::AddChildren( UPackage* InPackage, wxTreeItemId InId )
{
	for( TObjectIterator<UPackage> It ; It ; ++It )
	{
		if( It->GetOuter() == InPackage )
		{
			UPackage* pkg = *It;
			wxTreeItemId id = TreeCtrl->AppendItem( InId, *It->GetName(), -1, -1, new WxTreeObjectWrapper( pkg ) );

			AddChildren( pkg, id );
			TreeCtrl->SortChildren( id );
		}
	}
}

void WxDlgSelectGroup::OnOK( wxCommandEvent& In )
{
	WxTreeObjectWrapper* ItemData = (WxTreeObjectWrapper*) TreeCtrl->GetItemData(TreeCtrl->GetSelection());

	if( ItemData )
	{
		UPackage* pkg = ItemData->GetObjectChecked<UPackage>();
		FString Prefix;

		Group = TEXT("");

		while( pkg->GetOuter() )
		{
			Prefix = pkg->GetName();
			if( Group.Len() )
			{
				Prefix += TEXT(".");
			}

			Group = Prefix + Group;

			pkg = (UPackage*)(pkg->GetOuter());
		}

		Package = pkg->GetName();
	}
	wxDialog::OnOK( In );
}

void WxDlgSelectGroup::OnShowAllPackages( wxCommandEvent& In )
{
	bShowAllPackages = In.IsChecked();

	UpdateTree();
}

/*-----------------------------------------------------------------------------
	WxDlgLoadErrors.
-----------------------------------------------------------------------------*/

WxDlgLoadErrors::WxDlgLoadErrors( wxWindow* InParent )
{
	const bool bSuccess = wxXmlResource::Get()->LoadDialog( this, InParent, TEXT("ID_DLG_LOAD_ERRORS") );
	check( bSuccess );

	PackageList = wxDynamicCast( FindWindow( XRCID( "IDLB_FILES" ) ), wxListBox );
	check( PackageList != NULL );
	ObjectList = wxDynamicCast( FindWindow( XRCID( "IDLB_OBJECTS" ) ), wxListBox );
	check( ObjectList != NULL );

	FLocalizeWindow( this );
}

void WxDlgLoadErrors::Update()
{
	PackageList->Clear();
	ObjectList->Clear();

	// Use Windows methods since wxWindows doesn't give us what we need
	HWND hWnd = (HWND)GetHandle();
	HDC hDC = ::GetDC(hWnd);
	DWORD MaxWidth = 0;

	for( INT x = 0 ; x < GEdLoadErrors.Num() ; ++x )
	{
		if( GEdLoadErrors(x).Type == FEdLoadError::TYPE_FILE )
		{
			PackageList->Append( *GEdLoadErrors(x).Desc );
		}
		else
		{
			ObjectList->Append( *GEdLoadErrors(x).Desc );
			// Figure out the size of the string
			DWORD StrWidth = LOWORD(::GetTabbedTextExtent(hDC,*GEdLoadErrors(x).Desc,
				GEdLoadErrors(x).Desc.Len(),0,NULL));
			if (MaxWidth < StrWidth)
			{
				MaxWidth = StrWidth;
			}
		}
	}
	// Done with the DC so let it go
	::ReleaseDC(hWnd,hDC);
	// Get the object list rect so we know how much to change the dialog size by
	wxRect ObjectListRect = ObjectList->GetRect();
	wxSize DialogSize = GetClientSize();
	// If the dialog's right edge is less than our needed size, grow the dialog
	INT AdjustBy = (ObjectListRect.GetLeft() + MaxWidth + 8) - DialogSize.GetWidth();
	if (AdjustBy > 0)
	{
		SetSize(DialogSize.GetWidth() + AdjustBy,DialogSize.GetHeight());
	}
	Center();
}

/*-----------------------------------------------------------------------------
	WxDlgActorSearch.
-----------------------------------------------------------------------------*/
namespace
{
	enum EActorSearchFields
	{
		ASF_Name,
		ASF_Level,
		ASF_Group,
		ASF_PathName,
		ASF_Tag
	};
}


WxDlgActorSearch::WxDlgActorSearch( wxWindow* InParent )
{
	const bool bSuccess = wxXmlResource::Get()->LoadDialog( this, InParent, TEXT("ID_DLG_ACTOR_SEARCH") );
	check( bSuccess );

	SearchForEdit = (wxTextCtrl*)FindWindow( XRCID( "ID_SEARCH_FOR" ) );
	check( SearchForEdit != NULL );
	StartsWithRadio = (wxRadioButton*)FindWindow( XRCID( "ID_STARTING_WITH" ) );
	check( StartsWithRadio != NULL );
	ResultsList = (wxListCtrl*)FindWindow( XRCID( "ID_RESULTS" ) );
	check( ResultsList != NULL );
	ResultsLabel = (wxStaticText*)FindWindow( XRCID( "ID_RESULTS_TEXT" ) );
	check( ResultsLabel != NULL );
	InsideOfCombo = (wxComboBox*)FindWindow( XRCID( "IDCB_INSIDEOF" ) );
	check( InsideOfCombo != NULL );

	InsideOfCombo->Append( *LocalizeUnrealEd("Name") );
	InsideOfCombo->Append( *LocalizeUnrealEd("Level") );
	InsideOfCombo->Append( *LocalizeUnrealEd("Group") );
	InsideOfCombo->Append( *LocalizeUnrealEd("PathName") );
	InsideOfCombo->Append( *LocalizeUnrealEd("Tag") );
	InsideOfCombo->SetSelection( 0 );

	StartsWithRadio->SetValue( 1 );

	ResultsList->InsertColumn( ASF_Name, *LocalizeUnrealEd("Name"), wxLIST_FORMAT_LEFT, 128 );
	ResultsList->InsertColumn( ASF_Level, *LocalizeUnrealEd("Level"), wxLIST_FORMAT_LEFT, 128 );
	ResultsList->InsertColumn( ASF_Group, *LocalizeUnrealEd("Group"), wxLIST_FORMAT_LEFT, 128 );
	ResultsList->InsertColumn( ASF_PathName, *LocalizeUnrealEd("PathName"), wxLIST_FORMAT_LEFT, 300 );
	ResultsList->InsertColumn( ASF_Tag, *LocalizeUnrealEd("Tag"), wxLIST_FORMAT_LEFT, 128 );

	ADDEVENTHANDLER( XRCID("ID_SEARCH_FOR"), wxEVT_COMMAND_TEXT_UPDATED, &WxDlgActorSearch::OnSearchTextChanged );
	ADDEVENTHANDLER( XRCID("ID_STARTING_WITH"), wxEVT_COMMAND_RADIOBUTTON_SELECTED, &WxDlgActorSearch::OnSearchTextChanged );
	ADDEVENTHANDLER( XRCID("ID_CONTAINING"), wxEVT_COMMAND_RADIOBUTTON_SELECTED, &WxDlgActorSearch::OnSearchTextChanged );
	wxEvtHandler* eh = GetEventHandler();
	eh->Connect( XRCID("ID_RESULTS"), wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler(WxDlgActorSearch::OnColumnClicked) );
	eh->Connect( XRCID("ID_RESULTS"), wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxListEventHandler(WxDlgActorSearch::OnItemActivated) );
	ADDEVENTHANDLER( XRCID("IDPB_GOTOACTOR"), wxEVT_COMMAND_BUTTON_CLICKED, &WxDlgActorSearch::OnGoTo )
	ADDEVENTHANDLER( XRCID("IDPB_DELETEACTOR"), wxEVT_COMMAND_BUTTON_CLICKED, &WxDlgActorSearch::OnDelete )
	ADDEVENTHANDLER( XRCID("IDPB_PROPERTIES"), wxEVT_COMMAND_BUTTON_CLICKED, &WxDlgActorSearch::OnProperties )
	eh->Connect( this->GetId(), wxEVT_ACTIVATE, wxActivateEventHandler(WxDlgActorSearch::OnActivate) );

	FWindowUtil::LoadPosSize( TEXT("DlgActorSearch"), this, 256, 256, 450, 400 );

	UpdateResults();

	FLocalizeWindow( this );
}

WxDlgActorSearch::~WxDlgActorSearch()
{
	FWindowUtil::SavePosSize( TEXT("DlgActorSearch"), this );
}

void WxDlgActorSearch::OnSearchTextChanged( wxCommandEvent& In )
{
	UpdateResults();
}

void WxDlgActorSearch::OnColumnClicked( wxListEvent& In )
{
	const int Column = In.GetColumn();

	if( Column > -1 )
	{
		if( Column == SearchOptions.Column )
		{
			// Clicking on the same column will flip the sort order
			SearchOptions.bSortAscending = !SearchOptions.bSortAscending;
		}
		else
		{
			// Clicking on a new column will set that column as current and reset the sort order.
			SearchOptions.Column = In.GetColumn();
			SearchOptions.bSortAscending = TRUE;
		}
	}

	UpdateResults();
}

void WxDlgActorSearch::OnItemActivated( wxListEvent& In )
{
	wxCommandEvent Event;
	OnGoTo( Event );
}

/**
 * Empties the search list, releases actor references, etc.
 */
void WxDlgActorSearch::Clear()
{
	ResultsList->DeleteAllItems();
	ReferencedActors.Empty();
	ResultsLabel->SetLabel( *FString::Printf( *LocalizeUnrealEd("F_ObjectsFound"), 0 ) );
}

/**
 * Global temp storage mapping actors to the strings that should be used for sorting.
 * Non-NULL only during an actor search update, while actors are being sorted.
 */
static TMap<AActor*, FString> *GActorString = NULL;

/** Used by WxActorResultsListSort.  A global flag indicating whether actor search results should be sorted ascending or descending. */
static UBOOL GSortAscending = FALSE;

/**
 * Orders items in the actor search dialog's list view.
 */
static int wxCALLBACK WxActorResultsListSort(long InItem1, long InItem2, long InSortData)
{
	AActor* A = (AActor*)InItem1;
	AActor* B = (AActor*)InItem2;

	const FString* StringA = GActorString->Find( A );
	const FString* StringB = GActorString->Find( B );

	return appStricmp( *(*StringA), *(*StringB) ) * GSortAscending ? 1 : -1;
}

/**
 * Updates the contents of the results list.
 */
void WxDlgActorSearch::UpdateResults()
{
	const FScopedBusyCursor BusyCursor;
	ResultsList->Freeze();
	{
		// Empties the search list, releases actor references, etc.
		Clear();

		// Get the text to search with.  If it's empty, leave.

		FString SearchText = (const TCHAR*)SearchForEdit->GetValue();
		SearchText = SearchText.ToUpper();

		// Looks through all actors and see which ones meet the search criteria
		const INT SearchFields = InsideOfCombo->GetSelection();

		TMap<AActor*, FString> ActorStringMap;
		for( FActorIterator It; It; ++It )
		{
			AActor* Actor = *It;
			// skip transient actors (path building scout, etc)
			if ( !Actor->HasAnyFlags(RF_Transient) )
			{
				UBOOL bFoundItem = FALSE;

				FString CompString;
				switch( SearchFields )
				{
				case ASF_Name:		// Name
					CompString = Actor->GetName();
					break;
				case ASF_Level:
					CompString = Actor->GetOutermost()->GetName();
					break;
				case ASF_Tag:		// Tag
					CompString = Actor->Tag.ToString();
					break;
				case ASF_PathName:	// Path Name
					CompString = Actor->GetPathName();
					break;
				case ASF_Group:		// Group
					CompString = Actor->Group.ToString();
					break;
				default:
					break;
				}

				if( SearchText.Len() == 0 )
				{
					// If there is no search text, show all actors.
					bFoundItem = TRUE;
				}
				else
				{
					// Starts with/contains.
					if( StartsWithRadio->GetValue() )
					{
						if( CompString.ToUpper().StartsWith( SearchText ) )
						{
							bFoundItem = TRUE;
						}
					}
					else
					{
						if( CompString.ToUpper().InStr( SearchText ) != INDEX_NONE )
						{
							bFoundItem = TRUE;
						}
					}
				}

				// If the actor matches the criteria, add it to the list.
				if( bFoundItem )
				{
					ActorStringMap.Set( Actor, *CompString );
					const LONG Idx = ResultsList->InsertItem( 0, *Actor->GetName() );
					ResultsList->SetItem( Idx, ASF_Level, *Actor->GetOutermost()->GetName() );
					ResultsList->SetItem( Idx, ASF_PathName, *Actor->GetPathName() );
					ResultsList->SetItem( Idx, ASF_Group, *Actor->Group.ToString() );
					ResultsList->SetItem( Idx, ASF_Tag, *Actor->Tag.ToString() );
					ResultsList->SetItemData( Idx, (LONG)(void*)Actor );
					ReferencedActors.AddItem( Actor );
				}
			}
		}

		// Sort based on the current options
		GActorString = &ActorStringMap;
		GSortAscending = SearchOptions.bSortAscending;
		ResultsList->SortItems( WxActorResultsListSort, (LONG)&SearchOptions );
		GActorString = NULL;
	}
	ResultsList->Thaw();

	ResultsLabel->SetLabel( *FString::Printf( *LocalizeUnrealEd("F_ObjectsFound"), ResultsList->GetItemCount() ) );
}

/** Serializes object references to the specified archive. */
void WxDlgActorSearch::Serialize(FArchive& Ar)
{
	Ar << ReferencedActors;
}

void WxDlgActorSearch::OnGoTo( wxCommandEvent& In )
{
	long ItemIndex = ResultsList->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
	AActor* Actor = (AActor*)(ResultsList->GetItemData( ItemIndex ));

	const FScopedTransaction Transaction( *LocalizeUnrealEd("ActorSearchGoto") );

	// Deselect all actors, then focus on the first actor in the list, and simply select the rest.
	GEditor->Exec(TEXT("ACTOR SELECT NONE"));

	while( ItemIndex != -1 )
	{
		Actor = (AActor*)(ResultsList->GetItemData( ItemIndex ));
		if ( Actor )
		{
			GEditor->SelectActor( Actor, TRUE, NULL, FALSE, TRUE );
		}

		// Advance list iterator.
		ItemIndex = ResultsList->GetNextItem(ItemIndex, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	}

	GEditor->NoteSelectionChange();
	GEditor->Exec( TEXT("CAMERA ALIGN") );
}

void WxDlgActorSearch::OnDelete( wxCommandEvent& In )
{
	GEditor->Exec(TEXT("ACTOR SELECT NONE"));

	long ItemIndex = ResultsList->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );

	while( ItemIndex != -1 )
	{
		AActor* Actor = (AActor*)ResultsList->GetItemData( ItemIndex );
		if ( Actor )
		{
			GEditor->SelectActor( Actor, TRUE, NULL, FALSE, TRUE );
		}
		ItemIndex = ResultsList->GetNextItem( ItemIndex, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
	}

	GEditor->Exec( TEXT("DELETE") );
	UpdateResults();
}

void WxDlgActorSearch::OnProperties(wxCommandEvent& In )
{
	long ItemIndex = ResultsList->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
	AActor* Actor = (AActor*)(ResultsList->GetItemData( ItemIndex ));

	const FScopedTransaction Transaction( *LocalizeUnrealEd("ActorSearchSelectActors") );

	// Deselect all actors, then select the actors that are selected in our list, then 
	GEditor->Exec(TEXT("ACTOR SELECT NONE"));

	while( ItemIndex != -1 )
	{
		Actor = (AActor*)(ResultsList->GetItemData( ItemIndex ));
		if ( Actor )
		{
			GEditor->SelectActor( Actor, TRUE, NULL, FALSE, TRUE );
		}

		// Advance list iterator.
		ItemIndex = ResultsList->GetNextItem(ItemIndex, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	}

	GEditor->NoteSelectionChange();
	GEditor->Exec( TEXT("EDCALLBACK ACTORPROPS") );
}

/** Refreshes the results list when the window gets focus. */
void WxDlgActorSearch::OnActivate(wxActivateEvent& In)
{
	const UBOOL bActivated = In.GetActive();
	if(bActivated == TRUE)
	{
		UpdateResults();
	}
}
