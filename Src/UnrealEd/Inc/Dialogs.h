/*=============================================================================
	Copyright ?2004-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __DIALOGS_H__
#define __DIALOGS_H__

/*-----------------------------------------------------------------------------
	WxDlgBindHotkeys
-----------------------------------------------------------------------------*/

/**
 * This class is used to bind hotkeys for the editor.
 */
class WxDlgBindHotkeys : public wxFrame
{
public:
	WxDlgBindHotkeys(wxWindow* InParent);
	~WxDlgBindHotkeys();

	/** Saves settings for this dialog to the INI. */
	void SaveSettings();

	/** Loads settings for this dialog from the INI. */
	void LoadSettings();

	/** Starts the binding process for the current command. */
	void StartBinding(INT CommandIdx);

	/** Finishes the binding for the current command using the provided event. */
	void FinishBinding(wxKeyEvent &Event);

	/** Stops the binding process for the current command. */
	void StopBinding();

	/** @return Whether or not we are currently binding a command. */
	UBOOL IsBinding() 
	{
		return CurrentlyBindingIdx != -1;
	}

	/**
	 * @return Generates a descriptive binding string based on the key combinations provided.
	 */
	FString GenerateBindingText(UBOOL bAltDown, UBOOL bCtrlDown, UBOOL bShiftDown, FName Key);

private:
	
	/** Builds the category tree. */
	void BuildCategories();

	/** Builds the command list using the currently selected category. */
	void BuildCommands();

	/** Refreshes the binding text for the currently visible binding widgets. */
	void RefreshBindings();

	/** Updates the scrollbar for the command view. */
	void UpdateScrollbar();

	/** Window closed event handler. */
	void OnClose(wxCloseEvent& Event);

	/** Category selected handler. */
	void OnCategorySelected(wxTreeEvent& Event);

	/** Handler to let the user load a config from a file. */
	void OnLoadConfig(wxCommandEvent& Event);
	
	/** Handler to let the user save the current config to a file. */
	void OnSaveConfig(wxCommandEvent& Event);
	
	/** Handler to reset bindings to default. */
	void OnResetToDefaults(wxCommandEvent& Event);

	/** Bind key button pressed handler. */
	void OnBindKey(wxCommandEvent& Event);

	/** OK Button pressed handler. */
	void OnOK(wxCommandEvent &Event);

	/** Handler for key binding events. */
	void OnKeyDown(wxKeyEvent& Event);

	/** Which command we are currently binding a key to. */
	INT CurrentlyBindingIdx;

	/** Tree control to display all of the available command categories to bind to. */
	WxTreeCtrl* CommandCategories;

	/** Splitter to seperate tree and command views. */
	wxSplitterWindow* MainSplitter;

	/** Panel to store commands. */
	wxScrolledWindow* CommandPanel;

	/** Currently binding label text. */
	wxStaticText* BindLabel;

	/** Size of 1 command item. */
	INT ItemSize;

	/** Number of currently visible commands. */
	INT NumVisibleItems;

	/** The list of currently visible commands. */
	TArray<FEditorCommand> VisibleCommands;

	/** Struct of controls in a command panel. */
	struct FCommandPanel
	{
		wxTextCtrl* BindingWidget;
		wxButton* BindingButton;
		wxPanel* BindingPanel;
		FName CommandName;
	};

	/** Array of currently visible binding controls. */
	TArray<FCommandPanel> VisibleBindingControls;

	/** A mapping of category names to their array of commands. */
	TMap< FName, TArray<FEditorCommand> >	CommandMap;

	/** Mapping of category names to tree id's. */
	TMap<FName, wxTreeItemId> ParentMap;

	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
	WxDlgMove
-----------------------------------------------------------------------------*/

class WxDlgMove : public wxDialog
{
public:
	WxDlgMove();
	virtual ~WxDlgMove();

	const FString& GetNewPackage() const
	{
		return NewPackage;
	}
	const FString& GetNewGroup() const
	{
		return NewGroup;
	}
	const FString& GetNewName() const
	{
		return NewName;
	}
	const FString& GetNewTxtPackage() const
	{
		return NewTxtPackage;
	}
	const FString& GetNewTxtGroup() const
	{
		return NewTxtGroup;
	}
	const bool GetIncludeRefs() const
	{
		return IncludeRefs;
	}

	int ShowModal(const FString& InPackage, const FString& InGroup, const FString& InName );

	/////////////////////////
	// wxWindow interface.

	virtual bool Validate();

private:
	FString OldPackage, OldGroup, OldName;
	FString NewPackage, NewGroup, NewName, NewTxtPackage, NewTxtGroup;

	bool	IncludeRefs;
	wxBoxSizer* PGNSizer;
	wxPanel* PGNPanel;

public:
	class WxPkgGrpNameTxtCtrl* PGNCtrl;
};

/*-----------------------------------------------------------------------------
	WxDlgPackageGroupName.
-----------------------------------------------------------------------------*/

class WxDlgPackageGroupName : public wxDialog
{
public:
	WxDlgPackageGroupName();
	virtual ~WxDlgPackageGroupName();

	const FString& GetPackage() const
	{
		return Package;
	}
	const FString& GetGroup() const
	{
		return Group;
	}
	const FString& GetObjectName() const
	{
		return Name;
	}

	int ShowModal(const FString& InPackage, const FString& InGroup, const FString& InName );

protected:
	FString Package, Group, Name;
	wxBoxSizer* PGNSizer;
	wxPanel* PGNPanel;
	WxPkgGrpNameCtrl* PGNCtrl;

private:
	void OnOK( wxCommandEvent& In );

	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
	WxDlgNewArchetype.
-----------------------------------------------------------------------------*/

class WxDlgNewArchetype : public WxDlgPackageGroupName
{
public:
	/////////////////////////
	// wxWindow interface.

	virtual bool Validate();
};

/*-----------------------------------------------------------------------------
	WxDlgAddSpecial.
-----------------------------------------------------------------------------*/

class WxDlgAddSpecial : public wxDialog
{
public:
	WxDlgAddSpecial();
	virtual ~WxDlgAddSpecial();

private:
	wxCheckBox *PortalCheck, *InvisibleCheck, *TwoSidedCheck,
	//<@ ava specific ; 2006. 11. 27 changmin
		*HintCheck;
	//>@ ava
	wxRadioButton *SolidRadio, *SemiSolidRadio, *NonSolidRadio;

	void OnOK( wxCommandEvent& In );

	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
	WxDlgGenericStringEntry
-----------------------------------------------------------------------------*/

class WxDlgGenericStringEntry : public wxDialog
{
public:
	WxDlgGenericStringEntry();
	virtual ~WxDlgGenericStringEntry();

	const FString& GetEnteredString() const
	{
		return EnteredString;
	}
	const wxTextCtrl& GetStringEntry() const
	{
		return *StringEntry;
	}

	int ShowModal( const TCHAR* DialogTitle, const TCHAR* Caption, const TCHAR* DefaultString );

private:
	FString			EnteredString;
	wxTextCtrl		*StringEntry;
	wxStaticText	*StringCaption;

	void OnOK( wxCommandEvent& In );

	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
	WxDlgSurfaceProperties
-----------------------------------------------------------------------------*/

class WxDlgSurfaceProperties : public wxDialog
{
public:
	WxDlgSurfaceProperties();
	virtual ~WxDlgSurfaceProperties();

	void RefreshPages();

private:
	class WxSurfacePropertiesPanel* PropertiesPanel;

	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
	WxDlgColor.
-----------------------------------------------------------------------------*/

class WxDlgColor : public wxColourDialog
{
public:
	WxDlgColor();
	virtual ~WxDlgColor();

	void LoadColorData( wxColourData* InData );
	void SaveColorData( wxColourData* InData );

	bool Create( wxWindow* InParent, wxColourData* InData );

	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
	WxDlgSelectGroup
-----------------------------------------------------------------------------*/

class WxDlgSelectGroup : public wxDialog
{
public:
	WxDlgSelectGroup( wxWindow* InParent );
	virtual ~WxDlgSelectGroup();

	FString			Package, Group;
	UBOOL			bShowAllPackages;
	UPackage*		RootPackage;
	wxTreeCtrl*		TreeCtrl;

	void UpdateTree();
	void AddChildren( UPackage* InPackage, wxTreeItemId InId );

	int ShowModal( FString InPackageName, FString InGroupName );
	void OnOK( wxCommandEvent& In );
	void OnShowAllPackages( wxCommandEvent& In );

	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
	WxDlgLoadErrors
-----------------------------------------------------------------------------*/

class WxDlgLoadErrors : public wxDialog
{
public:
	WxDlgLoadErrors( wxWindow* InParent );

	wxListBox *PackageList, *ObjectList;

	void Update();
};

/*-----------------------------------------------------------------------------
	WxDlgActorSearch.
-----------------------------------------------------------------------------*/

class WxDlgActorSearch : public wxDialog, public FSerializableObject
{
public:
	WxDlgActorSearch( wxWindow* InParent );
	virtual ~WxDlgActorSearch();

	/**
	 * Empties the search list, releases actor references, etc.
	 */
	void Clear();

	/** Updates the results list using the current filter and search options set. */
	void UpdateResults();

	/** Serializes object references to the specified archive. */
	virtual void Serialize(FArchive& Ar);

	class FActorSearchOptions
	{
	public:
		FActorSearchOptions()
			: Column( 0 )
			, bSortAscending( TRUE )
		{}
		/** The column currently being used for sorting. */
		int Column;
		/** Denotes ascending/descending sort order. */
		UBOOL bSortAscending;
	};

protected:
	wxTextCtrl *SearchForEdit;
	wxRadioButton *StartsWithRadio;
	wxComboBox *InsideOfCombo;
	wxListCtrl *ResultsList;
	wxStaticText *ResultsLabel;

	TArray<AActor*> ReferencedActors;

	FActorSearchOptions SearchOptions;

	/** Wx Event Handlers */
	void OnSearchTextChanged( wxCommandEvent& In );
	void OnColumnClicked( wxListEvent& In );
	void OnItemActivated( wxListEvent& In );
	void OnGoTo( wxCommandEvent& In );
	void OnDelete( wxCommandEvent& In );
	void OnProperties( wxCommandEvent& In );

	/** Refreshes the results list when the window gets focus. */
	void OnActivate( wxActivateEvent& In);
};

#endif // __DIALOGS_H__
