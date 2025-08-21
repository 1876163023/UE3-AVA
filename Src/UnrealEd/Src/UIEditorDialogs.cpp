/*=============================================================================
	UIEditorDialogs.cpp: UI editor dialog window class implementations.
	Copyright 2005-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// Includes
#include "UnrealEd.h"

#include "EngineUIPrivateClasses.h"
#include "UnrealEdPrivateClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineUISequenceClasses.h"

//#include "Properties.h"
#include "UnObjectEditor.h"
//#include "UnLinkedObjEditor.h"
//#include "Kismet.h"
#include "UnUIEditor.h"
//#include "UnUIEditorProperties.h"
//#include "UnUIStyleEditor.h"
//#include "UIEditorStatusBar.h"
//#include "UIDataStoreBrowser.h"
//#include "UILayerBrowser.h"
//#include "GenericBrowser.h"
//#include "ScopedTransaction.h"
//#include "UnLinkedObjDrawUtils.h"
//#include "BusyCursor.h"
//
//#include "UIDockingPanel.h"
//
//#include "DlgUIListEditor.h"
//#include "DlgUIEvent_MetaObject.h"
//#include "DlgUISkinEditor.h"
//#include "DlgUIEventKeyBindings.h"
//#include "DlgUIWidgetEvents.h"
//
//#include "RenderModifier_SelectedWidgetsOutline.h"
//#include "RenderModifier_TargetWidgetOutline.h"
//#include "RenderModifier_FocusChainHandles.h"
//
//#include "UIWidgetTool_Selection.h"
//#include "UIWidgetTool_FocusChain.h"
//#include "UIWidgetTool_CustomCreate.h"

#include "ScopedPropertyChange.h"


/* ==========================================================================================================
	WxDlgCreateUIStyle
========================================================================================================== */

/** Constructor */
FStyleSelectionData::FStyleSelectionData( const FUIStyleResourceInfo& InResourceInfo )
: ResourceInfo(InResourceInfo), CurrentTemplate(INDEX_NONE)
{
}

/**
 * Adds the specified style to this selection's list of available templates
 */
void FStyleSelectionData::AddStyleTemplate( UUIStyle* InTemplate )
{
	checkSlow(InTemplate->StyleDataClass->IsChildOf(ResourceInfo.UIResource->GetClass()));

	StyleTemplates.AddItem(InTemplate);
}

/**
 * Serializes the references to the style templates for this selection
 */
void FStyleSelectionData::Serialize( FArchive& Ar )
{
	Ar << StyleTemplates;
}


IMPLEMENT_DYNAMIC_CLASS(WxDlgCreateUIStyle,wxDialog)

BEGIN_EVENT_TABLE(WxDlgCreateUIStyle,wxDialog)
	EVT_RADIOBOX(ID_UI_NEWSTYLE_RADIO, WxDlgCreateUIStyle::OnStyleTypeSelected)
	EVT_CHOICE(ID_UI_NEWSTYLE_TEMPLATECOMBO, WxDlgCreateUIStyle::OnStyleTemplateSelected)
END_EVENT_TABLE()

/**
 * Constructor
 * Initializes the available list of style type selections.
 */
WxDlgCreateUIStyle::WxDlgCreateUIStyle()
: StyleClassSelect(NULL), TagEditbox(NULL), FriendlyNameEditbox(NULL), TemplateCombo(NULL)
{
	SceneManager = GUnrealEd->GetBrowserManager()->UISceneManager;
	InitializeStyleCollection();
}

/**
 * Destructor
 * Saves the window position and size to the ini
 */
WxDlgCreateUIStyle::~WxDlgCreateUIStyle()
{
	FWindowUtil::SavePosSize(TEXT("CreateStyleDialog"), this);
}

/**
 * Initialize this dialog.  Must be the first function called after creating the dialog.
 */
void WxDlgCreateUIStyle::Create( wxWindow* InParent, wxWindowID InID/* = ID_UI_NEWSTYLE_DLG*/ )
{
	verify(wxDialog::Create(
		InParent, InID, 
		*LocalizeUI(TEXT("DlgNewStyle_Title")), 
		wxDefaultPosition, wxDefaultSize, 
		wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX|wxDIALOG_MODAL|wxCLIP_CHILDREN
		));

	SetExtraStyle(GetExtraStyle()|wxWS_EX_TRANSIENT|wxWS_EX_BLOCK_EVENTS);

	CreateControls();

	GetSizer()->Fit(this);
	GetSizer()->SetSizeHints(this);
	Centre();

	wxRect DefaultPos = GetRect();
	FWindowUtil::LoadPosSize( TEXT("CreateStyleDialog"), this, DefaultPos.GetLeft(), DefaultPos.GetTop(), 400, DefaultPos.GetHeight() );
}

/**
 * Creates the controls and sizers for this dialog.
 */
void WxDlgCreateUIStyle::CreateControls()
{
	wxBoxSizer* BaseVSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(BaseVSizer);

    wxBoxSizer* PropertiesVSizer = new wxBoxSizer(wxVERTICAL);
    BaseVSizer->Add(PropertiesVSizer, 0, wxGROW|wxALL, 5);

	wxArrayString StyleClassNames;

	// get the labels for the radio button options
	for ( INT i = 0; i < StyleChoices.Num(); i++ )
	{
		FStyleSelectionData& StyleSelection = StyleChoices(i);
		StyleClassNames.Add(*StyleSelection.ResourceInfo.FriendlyName);
	}

	// create the "Select style class" radio button control
	FString StyleClassLabelText = Localize(TEXT("UIEditor"), TEXT("DlgNewStyle_Label_StyleClass"));
	StyleClassSelect = new wxRadioBox( 
		this,								// parent
		ID_UI_NEWSTYLE_RADIO,				// id
		*StyleClassLabelText,				// title
		wxDefaultPosition,					// pos
		wxDefaultSize,						// size
		StyleClassNames,					// choice labels
		1,									// major dimension
		wxRA_SPECIFY_ROWS					// style
		);
	StyleClassSelect->SetToolTip( *Localize(TEXT("UIEditor"), TEXT("DlgNewStyle_Tooltip_StyleClass")) );
	PropertiesVSizer->Add(StyleClassSelect, 0, wxGROW|wxALL, 5);

	// create the properties grouping box
    wxStaticBox* PropertiesGroupBox = new wxStaticBox(this, wxID_ANY, *LocalizeUnrealEd(TEXT("Properties")));
    wxStaticBoxSizer* PropertiesStaticVSizer = new wxStaticBoxSizer(PropertiesGroupBox, wxVERTICAL);
    PropertiesVSizer->Add(PropertiesStaticVSizer, 1, wxGROW|wxALL, 5);

	// create the sizer that will contain the options for creating the new style
    wxFlexGridSizer* PropertiesGridSizer = new wxFlexGridSizer(0, 2, 0, 0);
    PropertiesGridSizer->AddGrowableCol(1);
    PropertiesStaticVSizer->Add(PropertiesGridSizer, 1, wxGROW|wxALL, 5);

	// create the style tag label
    wxStaticText* TagLabel = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("DlgNewStyle_Label_StyleTag")), wxDefaultPosition, wxDefaultSize, 0 );
    PropertiesGridSizer->Add(TagLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	// create the style tag editbox
    TagEditbox = new wxTextCtrl( this, -1, TEXT(""), wxDefaultPosition, wxDefaultSize, 0 );
    TagEditbox->SetMaxLength(64);
	TagEditbox->SetToolTip(*LocalizeUI(TEXT("DlgNewStyle_Tooltip_StyleTag")));
	PropertiesGridSizer->Add(TagEditbox, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	// create the style friendly name label
    wxStaticText* NameLabel = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("DlgNewStyle_Label_StyleName")), wxDefaultPosition, wxDefaultSize, 0 );
    PropertiesGridSizer->Add(NameLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

	// create the style friendly name editbox
    FriendlyNameEditbox = new wxTextCtrl( this, -1, TEXT(""), wxDefaultPosition, wxDefaultSize, 0 );
	FriendlyNameEditbox->SetToolTip(*LocalizeUI(TEXT("DlgNewStyle_Tooltip_StyleName")));
    PropertiesGridSizer->Add(FriendlyNameEditbox, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	// create the template combo label
    wxStaticText* TemplateLabel = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("DlgNewStyle_Label_StyleTemplate")), wxDefaultPosition, wxDefaultSize, 0 );
    PropertiesGridSizer->Add(TemplateLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxArrayString TemplateComboStrings;
	TemplateComboStrings.Add(*LocalizeUnrealEd(TEXT("None")));

	// create the combo box that will display the list of UIStyles that can be used as the archetype for this new style
    TemplateCombo = new wxChoice( this, ID_UI_NEWSTYLE_TEMPLATECOMBO, wxDefaultPosition, wxDefaultSize, TemplateComboStrings );
	TemplateCombo->SetToolTip(*LocalizeUI(TEXT("DlgNewStyle_Tooltip_StyleTemplate")));
    PropertiesGridSizer->Add(TemplateCombo, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	// setup the sizer for the OK/Cancel buttons
    wxBoxSizer* ButtonHSizer = new wxBoxSizer(wxHORIZONTAL);
    PropertiesVSizer->Add(ButtonHSizer, 0, wxALIGN_RIGHT|wxALL, 5);

    wxButton* OKButton = new wxButton( this, wxID_OK, *LocalizeUnrealEd(TEXT("&OK")), wxDefaultPosition, wxDefaultSize, 0 );
    ButtonHSizer->Add(OKButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* CancelButton = new wxButton( this, wxID_CANCEL, *LocalizeUnrealEd(TEXT("&Cancel")), wxDefaultPosition, wxDefaultSize, 0 );
    ButtonHSizer->Add(CancelButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	TagEditbox->SetValidator( WxNameTextValidator(&StyleTagString,VALIDATE_ObjectName) );

	FLocalizeWindow(this,TRUE);
}

/**
 * Initializes the list of available styles, along with the templates available to each one.
 */
void WxDlgCreateUIStyle::InitializeStyleCollection()
{
	check(SceneManager != NULL);

	for ( INT i = 0; i < SceneManager->UIStyleResources.Num(); i++ )
	{
		FStyleSelectionData* StyleSelection = new FStyleSelectionData(SceneManager->UIStyleResources(i));
		StyleChoices.AddRawItem(StyleSelection);
	}

	TArray<UUIStyle*> SkinStyles;
	SceneManager->ActiveSkin->GetAvailableStyles(SkinStyles, FALSE);

	// iterate through the list of available styles in the currently active skin
	//@fixme - this should actually use the style lookup table (by calling GetAvailableStyles(SkinStyles,TRUE)),
	// but then users would be able to use styles from base skins as the archetype for new skins....this is fine,
	// but the ReplaceStyle functionality needs to then fixup those style references to point the corresponding style
	// in the current skin (i.e. the style that is replacing the style that is about to be used for this new style's archetype)
	for ( INT i = 0; i < SkinStyles.Num(); i++ )
	{
		UUIStyle* StyleObj = SkinStyles(i);
		if ( !StyleObj->IsTemplate(RF_ClassDefaultObject) )
		{
			INT StyleDataIndex;

			// if this style object belongs to this style type's class, add the object to the style type's list of available templates
			if ( GetSelectionData(StyleObj,StyleDataIndex) )
			{
				StyleChoices(StyleDataIndex).AddStyleTemplate(StyleObj);
			}
		}
	}
}

/**
 * Retrieves the style selection data associated with the specified style object's class
 *
 * @param	StyleObject			the style object to find selection data for
 * @param	out_SelectionIndex	if the return value is TRUE, will be filled with the index corresponding to the
 *								selection data for the specified style object's class.  If the return value is FALSE,
 *								this value is left unchanged.
 *
 * @return	TRUE if selection data was successfully found for the specified style class
 */
UBOOL WxDlgCreateUIStyle::GetSelectionData( UUIStyle* StyleObject, INT& out_SelectionIndex )
{
	UBOOL bResult = FALSE;
	if ( StyleObject != NULL )
	{
		for ( INT i = 0; i < StyleChoices.Num(); i++ )
		{
			FStyleSelectionData& Choice = StyleChoices(i);
			if ( StyleObject->StyleDataClass->IsChildOf(Choice.ResourceInfo.UIResource->GetClass()) )
			{
				out_SelectionIndex = i;
				bResult = TRUE;
				break;
			}
		}
	}

	return bResult;
}

/**
 * Displays this dialog.
 *
 * @param	InStyleTag		the default value for the "Unique Name" field
 * @param	InFriendlyName	the default value for the "Friendly Name" field
 * @param	InTemplate		the style template to use for the new style
 */
int WxDlgCreateUIStyle::ShowModal( const FString& InStyleTag, const FString& InFriendlyName, UUIStyle* InTemplate/*=NULL*/ )
{
	TagEditbox->SetValue( *InStyleTag );
	FriendlyNameEditbox->SetValue( *InFriendlyName );

	INT StyleTypeIndex=0;
	if ( InTemplate != NULL )
	{
		// if InTemplate is not NULL, it means the user selected the "create style from selected" option...
		// so disable the style class radio button since we must use the class of the template
		if ( GetSelectionData(InTemplate,StyleTypeIndex) )
		{
			// disable the style class radio button if the user specified a t
			StyleClassSelect->SetSelection(StyleTypeIndex);
			StyleClassSelect->Enable(false);
			StyleChoices(StyleTypeIndex).SetCurrentTemplate( InTemplate );
		}
	}

	PopulateStyleTemplateCombo(StyleTypeIndex);

	return wxDialog::ShowModal();
}

/**
 * Populates the style template combo with the list of UIStyle objects which can be used as an archetype for the new style.
 * Only displays the styles that correspond to the selected style type.
 *
 * @param	SelectedStyleIndex	the index of the selected style; corresponds to an element in the StyleChoices array
 */
void WxDlgCreateUIStyle::PopulateStyleTemplateCombo( INT SelectedStyleIndex/*=INDEX_NONE*/ )
{
	// clear all existing items
	TemplateCombo->Clear();

	// add the "None" item
	TemplateCombo->Append( *LocalizeUnrealEd(TEXT("None")) );

	INT TemplateIndex = INDEX_NONE;
	if ( StyleChoices.IsValidIndex(SelectedStyleIndex) )
	{
		TArray<UUIStyle*> StyleTemplates;
		StyleChoices(SelectedStyleIndex).GetStyleTemplates(StyleTemplates);
		for ( INT i = 0; i < StyleTemplates.Num(); i++ )
		{
			// add the data for each template for this style type to the template combo
			TemplateCombo->Append( *StyleTemplates(i)->GetPathName(), StyleTemplates(i) );
		}

		TemplateIndex = StyleChoices(SelectedStyleIndex).GetCurrentTemplate();
	}

	TemplateCombo->SetSelection( TemplateIndex + 1 );
}

/**
 * This handler is called when the user selects an item in the radiobox.
 */
void WxDlgCreateUIStyle::OnStyleTypeSelected( wxCommandEvent& Event )
{
	PopulateStyleTemplateCombo(Event.GetSelection());
}

/**
 * This handler is called when the user selects an item in the choice control.
 */
void WxDlgCreateUIStyle::OnStyleTemplateSelected( wxCommandEvent& Event )
{
	INT StyleTypeIndex = StyleClassSelect->GetSelection();
	check(StyleChoices.IsValidIndex(StyleTypeIndex));

	// set the currently selected style type's CurrentTemplate index to the newly selected template
	StyleChoices(StyleTypeIndex).SetCurrentTemplate( (UUIStyle*)Event.GetClientData() );
}

/**
 * Retrieve the style class that was selected by the user
 */
UClass* WxDlgCreateUIStyle::GetStyleClass() const
{
	INT StyleTypeIndex = StyleClassSelect->GetSelection();
	check(StyleChoices.IsValidIndex(StyleTypeIndex));

	return StyleChoices(StyleTypeIndex).ResourceInfo.UIResource->GetClass();
}

/**
 * Retrieve the style template that was selected by the user
 */
UUIStyle* WxDlgCreateUIStyle::GetStyleTemplate() const
{
	UUIStyle* Result = NULL;
	if ( TemplateCombo != NULL )
	{
		Result = (UUIStyle*)TemplateCombo->GetClientData(TemplateCombo->GetSelection());
	}

	return Result;
}

/* ==========================================================================================================
	WxDlgCreateUISkin
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxDlgCreateUISkin,wxDialog)

BEGIN_EVENT_TABLE(WxDlgCreateUISkin,wxDialog)
	EVT_TEXT(ID_UI_NEWSKIN_EDIT_PKGNAME,WxDlgCreateUISkin::OnTextEntered)
	EVT_TEXT(ID_UI_NEWSKIN_EDIT_SKINNAME,WxDlgCreateUISkin::OnTextEntered)
END_EVENT_TABLE()

void WxDlgCreateUISkin::Create( wxWindow* InParent, wxWindowID InID )
{
	verify(wxDialog::Create(InParent,InID,*LocalizeUI(TEXT("DlgNewSkin_Title")),wxDefaultPosition,wxSize(400,178),wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX|wxCLIP_CHILDREN));

	SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);

	CreateControls();

	GetSizer()->Fit(this);
	GetSizer()->SetSizeHints(this);
	Centre();
}

/**
 * Creates the controls for this window
 */
void WxDlgCreateUISkin::CreateControls()
{
	wxBoxSizer* BaseVSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(BaseVSizer);

	wxStaticBox* OptionsGroupBox = new wxStaticBox(this, wxID_ANY, TEXT("Options"));
	wxStaticBoxSizer* OptionsVSizer = new wxStaticBoxSizer(OptionsGroupBox, wxVERTICAL);
	BaseVSizer->Add(OptionsVSizer, 0, wxGROW|wxALL, 5);

	wxFlexGridSizer* OptionsGridSizer = new wxFlexGridSizer(0, 2, 0, 0);
	OptionsGridSizer->AddGrowableCol(1);
	OptionsVSizer->Add(OptionsGridSizer, 1, wxGROW|wxALL, 5);

	//@todo - localize
	wxStaticText* PackageNameLabel = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("DlgNewSkin_Label_PackageName")), wxDefaultPosition, wxDefaultSize, 0 );
	OptionsGridSizer->Add(PackageNameLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

	PackageNameEdit = new WxTextCtrl( this, ID_UI_NEWSKIN_EDIT_PKGNAME, TEXT(""), wxDefaultPosition, wxDefaultSize, 0 );
	OptionsGridSizer->Add(PackageNameEdit, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxStaticText* SkinNameLabel = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("DlgNewSkin_Label_SkinName")), wxDefaultPosition, wxDefaultSize, 0 );
	OptionsGridSizer->Add(SkinNameLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

	SkinNameEdit = new WxTextCtrl( this, ID_UI_NEWSKIN_EDIT_SKINNAME, TEXT(""), wxDefaultPosition, wxDefaultSize, 0 );
	OptionsGridSizer->Add(SkinNameEdit, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxStaticText* TemplateLabel = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("DlgNewStyle_Label_StyleTemplate")), wxDefaultPosition, wxDefaultSize, 0 );
	OptionsGridSizer->Add(TemplateLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

	SkinTemplateCombo = new wxChoice( this, ID_UI_NEWSKIN_COMBO_TEMPLATE, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	OptionsGridSizer->Add(SkinTemplateCombo, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxBoxSizer* ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	BaseVSizer->Add(ButtonSizer, 0, wxALIGN_RIGHT|wxALL, 2);

	OKButton = new wxButton( this, wxID_OK, *LocalizeUnrealEd(TEXT("&OK")) );
	ButtonSizer->Add(OKButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxButton* CancelButton = new wxButton( this, wxID_CANCEL, *LocalizeUnrealEd(TEXT("&Cancel")) );
	ButtonSizer->Add(CancelButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	PackageNameEdit->SetValidator( WxNameTextValidator(&PackageNameString,VALIDATE_PackageName) );
	SkinNameEdit->SetValidator( WxNameTextValidator(&SkinNameString,VALIDATE_ObjectName) );
}

/**
 * Displays this dialog.
 *
 * @param	InPackageName	the default value for the "Package Name" field
 * @param	InSkinName		the default value for the "Skin Name" field
 * @param	InTemplate		the template to use for the new skin
 */
INT WxDlgCreateUISkin::ShowModal( const FString& InPackageName, const FString& InSkinName, UUISkin* InTemplate )
{
	PackageNameString = *InPackageName;
	SkinNameString = *InSkinName;

	// for now, only add the specified template
	INT Index = SkinTemplateCombo->Append( *InTemplate->Tag.ToString(), InTemplate );
	SkinTemplateCombo->SetSelection(Index);
	SkinTemplateCombo->Enable(FALSE);

	return wxDialog::ShowModal();
}

/** This handler is called when the user types text in the text control */
void WxDlgCreateUISkin::OnTextEntered( wxCommandEvent& Event )
{
	if ( OKButton != NULL )
	{
		// only enable the OK button if both edit boxes contain text
		OKButton->Enable( PackageNameEdit->GetLineLength(0) > 0 && SkinNameEdit->GetLineLength(0) > 0 );
	}

	Event.Skip();
}

bool WxDlgCreateUISkin::Validate()
{
	bool bResult = wxDialog::Validate();
	if ( bResult )
	{
		FString PackageName = PackageNameString.c_str();
		FString SkinName = SkinNameString.c_str();

		FString QualifiedSkinName = PackageName + TEXT(".") + SkinName;
		FString Reason;
		if ( !FIsUniqueObjectName(*QualifiedSkinName,ANY_PACKAGE,Reason) )
		{
			//@todo - don't rely on FIsUniqueObjectName....search through all packages
			appMsgf(AMT_OK, *Reason);
			bResult = false;
		}
	}

	return bResult;
}

/**
 * Returns the UUISkin corresponding to the selected item in the template combobox
 */
UUISkin* WxDlgCreateUISkin::GetSkinTemplate()
{
	return (UUISkin*)SkinTemplateCombo->GetClientData(SkinTemplateCombo->GetSelection());
}

/* ==========================================================================================================
	WxDlgDockingEditor
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxDlgDockingEditor,wxDialog)

BEGIN_EVENT_TABLE(WxDlgDockingEditor,wxDialog)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_LEFT_TARGET,WxDlgDockingEditor::OnChangeDockingTarget)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_TOP_TARGET,WxDlgDockingEditor::OnChangeDockingTarget)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_RIGHT_TARGET,WxDlgDockingEditor::OnChangeDockingTarget)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_BOTTOM_TARGET,WxDlgDockingEditor::OnChangeDockingTarget)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_LEFT_FACE,WxDlgDockingEditor::OnChangeDockingFace)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_TOP_FACE,WxDlgDockingEditor::OnChangeDockingFace)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_RIGHT_FACE,WxDlgDockingEditor::OnChangeDockingFace)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_BOTTOM_FACE,WxDlgDockingEditor::OnChangeDockingFace)
END_EVENT_TABLE()

void WxDlgDockingEditor::Serialize( FArchive& Ar )
{
	Ar << CurrentWidget << ValidDockTargets;
}

/**
 * Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
 *
 * @param	InParent				the window that opened this dialog
 * @param	InID					the ID to use for this dialog
 * @param	AdditionalButtonIds		additional buttons that should be added to the dialog....currently only ID_CANCEL_ALL is supported
 */
void WxDlgDockingEditor::Create( wxWindow* InParent, wxWindowID InID/*=ID_UI_DOCKINGEDITOR_DLG*/, long AdditionalButtonIds/*=0*/ )
{
	SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
	verify(wxDialog::Create( InParent, InID, *LocalizeUI(TEXT("DlgDockingEditor_Title")), wxDefaultPosition, wxSize(600,300), wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL|wxCLIP_CHILDREN|wxTAB_TRAVERSAL ));

	CreateControls(AdditionalButtonIds);

	GetSizer()->Fit(this);
	GetSizer()->SetSizeHints(this);
	Centre();

	wxRect DefaultPos = GetRect();
	FWindowUtil::LoadPosSize( TEXT("DockingEditorDialog"), this, DefaultPos.GetLeft(), DefaultPos.GetTop(), 400, DefaultPos.GetHeight() );
}

/**
 * Destructor
 * Saves the window position
 */
WxDlgDockingEditor::~WxDlgDockingEditor()
{
	FWindowUtil::SavePosSize(TEXT("DockingEditorDialog"), this);
}

/**
 * Creates the controls for this window
 */
void WxDlgDockingEditor::CreateControls( long AdditionalButtonIds/*=0*/ )
{
	wxBoxSizer* BaseVSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(BaseVSizer);

	wxBoxSizer* DockingSetVSizer = new wxBoxSizer(wxVERTICAL);
	BaseVSizer->Add(DockingSetVSizer, 0, wxGROW|wxALL, 5);

	// retrieve the text for the docking set labels from the loc file
	FString tmpString = LocalizeUI( TEXT("DlgDockingEditor_Label_DockTarget") );
	tmpString.ParseIntoArray(&DockFaceStrings,TEXT(","),TRUE);
	if ( DockFaceStrings.Num() != 4 )
	{
		DockFaceStrings.Empty(4);
		DockFaceStrings.AddItem(LocalizeUI("UIEditor_FaceText[0]"));
		DockFaceStrings.AddItem(LocalizeUI("UIEditor_FaceText[1]"));
		DockFaceStrings.AddItem(LocalizeUI("UIEditor_FaceText[2]"));
		DockFaceStrings.AddItem(LocalizeUI("UIEditor_FaceText[3]"));
	}

	FString NoneText = *LocalizeUnrealEd(TEXT("None"));
	DockFaceStrings.AddItem(NoneText);
	FString DockTargetToolTip = LocalizeUI(TEXT("DlgDockingEditor_ToolTip_DockTarget"));
	FString DockFaceToolTip = LocalizeUI(TEXT("DlgDockingEditor_ToolTip_DockFace"));
	FString DockPaddingToolTip = LocalizeUI(TEXT("DlgDockingEditor_ToolTip_DockPadding"));
	FString DockTargetHelpText = LocalizeUI(TEXT("DlgDockingEditor_HelpText_DockTarget"));
	FString DockFaceHelpText = LocalizeUI(TEXT("DlgDockingEditor_HelpText_DockFace"));
	FString DockPaddingHelpText = LocalizeUI(TEXT("DlgDockingEditor_HelpText_DockPadding"));

	// add the controls for each face
	for ( INT i = 0; i < UIFACE_MAX; i++ )
	{
		wxBoxSizer* DockFaceVSizer = new wxBoxSizer(wxVERTICAL);
		DockingSetVSizer->Add(DockFaceVSizer, 0, wxGROW|wxALL, 0);

		// add the label
		lbl_DockingSet[i] = new wxStaticText( this, wxID_STATIC, *(DockFaceStrings(i) + TEXT(":")) );
		DockFaceVSizer->Add(lbl_DockingSet[i], 0, wxGROW|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

		wxBoxSizer* DockFaceHSizer = new wxBoxSizer(wxHORIZONTAL);
		DockFaceVSizer->Add(DockFaceHSizer, 0, wxGROW|wxALL, 0);

		// add the combo that displays the list of widgets that can be assigned as a dock target
		cmb_DockTarget[i] = new wxComboBox( this, ID_UI_DOCKINGEDITOR_LEFT_TARGET + i, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY | wxCB_DROPDOWN );
		cmb_DockTarget[i]->SetHelpText(*DockTargetHelpText);
		cmb_DockTarget[i]->SetToolTip(*DockTargetToolTip);
		DockFaceHSizer->Add(cmb_DockTarget[i], 1, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5);

		wxString cmb_DockFace_Strings[] =
		{
			*DockFaceStrings(0),
			*DockFaceStrings(1),
			*DockFaceStrings(2),
			*DockFaceStrings(3),
			*DockFaceStrings(4)
		};
		// add the combo that displays the list of faces that can be assigned to this docking set
		cmb_DockFace[i] = new wxComboBox( this, ID_UI_DOCKINGEDITOR_LEFT_FACE + i, wxEmptyString, wxDefaultPosition, wxDefaultSize, 5, cmb_DockFace_Strings, wxCB_READONLY | wxCB_DROPDOWN );
		cmb_DockFace[i]->SetValue(cmb_DockTarget[i]->GetString(0));
		cmb_DockFace[i]->SetHelpText(*DockFaceHelpText);
		cmb_DockFace[i]->SetToolTip(*DockFaceToolTip);
		DockFaceHSizer->Add(cmb_DockFace[i], 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5);

		// add a spin control that allows the user to modify the padding value for this face
		spin_DockPadding[i] = new wxSpinCtrl(this, ID_UI_DOCKINGEDITOR_LEFT_PADDING + i, TEXT("0"), wxDefaultPosition, wxSize(60,-1), wxSP_ARROW_KEYS, -1000, 1000);
		spin_DockPadding[i]->SetHelpText(*DockPaddingHelpText);
		spin_DockPadding[i]->SetToolTip(*DockPaddingToolTip);
		DockFaceHSizer->Add( spin_DockPadding[i], 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5);
	}

	wxBoxSizer* ButtonHSizer = new wxBoxSizer(wxHORIZONTAL);
	BaseVSizer->Add(ButtonHSizer, 0, wxALIGN_RIGHT|wxALL, 5);

	// add the OK and cancel buttons
	btn_OK = new wxButton( this, wxID_OK, *LocalizeUnrealEd(TEXT("&OK")) );
	btn_OK->SetDefault();
	ButtonHSizer->Add(btn_OK, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxButton* btn_Cancel = new wxButton( this, wxID_CANCEL, *LocalizeUnrealEd(TEXT("&Cancel")) );
	ButtonHSizer->Add(btn_Cancel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}

/**
 * Initializes the values of the controls in this dialog, the displays the dialog.
 *
 * @param	Container	the object which contains InWidget
 * @param	InWidget	the widget to edit the docking sets for
 */
INT WxDlgDockingEditor::ShowModal( UUIScreenObject* Container, UUIObject* InWidget )
{
	check(Container);
	check(InWidget);

	FString WindowTitle = LocalizeUI(TEXT("DlgDockingEditor_Title")) + TEXT(": ") + InWidget->GetTag().ToString();
	SetTitle(*WindowTitle);

	// InWidget must be contained within Container
	verify(Container==InWidget->GetScene() || InWidget->IsContainedBy(Cast<UUIObject>(Container)));

	// the widget can never dock to itself, so remove it from the list of valid targets right away
	ValidDockTargets = Container->GetChildren(TRUE);
	ValidDockTargets.RemoveItem(InWidget);

	CurrentWidget = InWidget;

	// fill the combos with data
	InitializeTargetCombos();

	// display the dialog
	return wxDialog::ShowModal();
}

/**
 * Fills the "dock target" combos with the names of the widgets contained within Container that are valid dock targets for InWidget.
 */
void WxDlgDockingEditor::InitializeTargetCombos()
{
	UUIScene* OwnerScene = CurrentWidget->GetScene();
	for ( INT FaceIndex = 0; FaceIndex < UIFACE_MAX; FaceIndex++ )
	{
		EUIWidgetFace CurrentFace = (EUIWidgetFace)FaceIndex;
		cmb_DockTarget[CurrentFace]->Freeze();

		// clear the existing contents
		cmb_DockTarget[CurrentFace]->Clear();

		// Add the 'none' item
		INT SelectionIndex=0;
		cmb_DockTarget[CurrentFace]->Append(*LocalizeUnrealEd(TEXT("None")), (void*)NULL);

		// add the item for the scene
		FString SceneTag = FString::Printf(TEXT("%s (%s)"), *OwnerScene->GetTag().ToString(), *LocalizeUI("UIEditor_Scene"));
		INT idx = cmb_DockTarget[FaceIndex]->Append(*SceneTag, OwnerScene);
		if ( CurrentWidget->DockTargets.IsDocked(CurrentFace,FALSE) && CurrentWidget->DockTargets.GetDockTarget(CurrentFace) == NULL )
		{
			// if the widget is docked, but GetDockTarget returns NULL, the widget is docked to the scene
			SelectionIndex = idx;
		}

		for ( INT ChildIndex = 0; ChildIndex < ValidDockTargets.Num(); ChildIndex++ )
		{
			UUIObject* Child = ValidDockTargets(ChildIndex);
			idx = cmb_DockTarget[CurrentFace]->Append( *Child->GetTag().ToString(), Child );
			if ( CurrentWidget->DockTargets.GetDockTarget(CurrentFace) == Child )
			{
				SelectionIndex = idx;
			}
		}

		cmb_DockTarget[CurrentFace]->Thaw();

		// slight hack here because SetSelection doesn't update the combo's m_selectionOld member, which causes
		// the combo to generate two "selection changed" events the first time the user selects an item
		//cmb_DockTarget[FaceIndex]->SetSelection(SelectionIndex);
		wxString SelectedWidgetString = cmb_DockTarget[FaceIndex]->GetString(SelectionIndex);
		cmb_DockTarget[FaceIndex]->SetValue(SelectedWidgetString);

		// set the value of the face combo
		cmb_DockFace[FaceIndex]->SetValue(*DockFaceStrings(CurrentWidget->DockTargets.GetDockFace(CurrentFace)));

		// set the value of the padding control
		spin_DockPadding[FaceIndex]->SetValue(appTrunc(CurrentWidget->DockTargets.DockPadding[FaceIndex]));
	}

	for ( INT FaceIndex = 0; FaceIndex < UIFACE_MAX; FaceIndex++ )
	{
		UpdateAvailableDockFaces((EUIWidgetFace)FaceIndex);
	}
}

/**
 * Refreshes and validates the list of faces which can be used as the dock target face for the specified face.  Removes any entries that
 * are invalid for this face (generally caused by restrictions on docking relationships necessary to prevent infinite recursion)
 *
 * @param	SourceFace	indicates which of the combo boxes should be validated & updated
 */
void WxDlgDockingEditor::UpdateAvailableDockFaces( EUIWidgetFace SourceFace )
{
	UUIScreenObject* CurrentSelectedTarget = GetSelectedDockTarget(SourceFace);
	UUIObject* CurrentlySelectedWidget = Cast<UUIObject>(CurrentSelectedTarget);

	TArray<FString> ValidDockFaces = DockFaceStrings;
	if ( CurrentSelectedTarget != NULL )
	{
		for ( INT i = 0; i < UIFACE_MAX; i++ )
		{
			EUIWidgetFace CurrentFace = (EUIWidgetFace)i;
			if ( CurrentlySelectedWidget != NULL )
			{
				EUIWidgetFace DockFace = CurrentlySelectedWidget->DockTargets.GetDockFace(CurrentFace);

				if ( CurrentlySelectedWidget->DockTargets.GetDockTarget(CurrentFace) == CurrentWidget && CurrentlySelectedWidget->DockTargets.IsDocked(CurrentFace) )
				{
					UBOOL bTargetIsBoundToThisFace = DockFace == SourceFace;
					UBOOL bTargetIsBoundToDependentFace = (CurrentFace == UIFACE_Left || CurrentFace==UIFACE_Top) && DockFace == CurrentFace + 2;
					if ( bTargetIsBoundToThisFace || bTargetIsBoundToDependentFace )
					{
						// not allowed to create a circular relationship between docking sets, so if
						// the currently selected target is docked to this widget's SourceFace, we must remove the
						// string corresponding to the opposite face on the target widget
						ValidDockFaces.RemoveItem(DockFaceStrings(CurrentFace));
						if ( CurrentFace == UIFACE_Left || CurrentFace == UIFACE_Top )
						{
							// if the face currently being evaluated in the target is the left or top face, it also illegal to
							// dock to the opposing side of the target, since that face is dependent on this one
							ValidDockFaces.RemoveItem(DockFaceStrings(CurrentFace+2));
						}
					}
				}
			}

			if ( CurrentFace == SourceFace )
			{
				continue;
			}

			UUIScreenObject* TargetObject = GetSelectedDockTarget((EUIWidgetFace)i);
			EUIWidgetFace TargetFace = GetSelectedDockFace((EUIWidgetFace)i);
			if ( TargetFace != UIFACE_MAX )
			{
				// this the target for SourceFace is also the target for another face,
				if ( TargetObject == CurrentSelectedTarget )
				{
					// not allowed to bind more than one face of source widget to the same face of the target widget
					ValidDockFaces.RemoveItem(DockFaceStrings(TargetFace));
				}
			}
		}
	}

	// save the current selection
	FString CurrentSelectedFaceString;
	if ( cmb_DockFace[SourceFace]->GetCount() > 0 )
	{
		CurrentSelectedFaceString = cmb_DockFace[SourceFace]->GetValue().c_str();
	}

	// get the index for the currently selected face
	INT CurrentSelectedFaceIndex = CurrentSelectedFaceString.Len()
		? ValidDockFaces.FindItemIndex(CurrentSelectedFaceString)
		: INDEX_NONE;

	if ( CurrentSelectedFaceIndex == INDEX_NONE )
	{
		// if the selected face is no longer part of the valid set, change it to None
		CurrentSelectedFaceIndex = ValidDockFaces.Num() - 1;
	}

	// freeze the combo to eliminate flicker while we update the combo's items
	cmb_DockFace[SourceFace]->Freeze();

	// now clear the existing entries
	cmb_DockFace[SourceFace]->Clear();
	for ( INT i = 0; i < ValidDockFaces.Num(); i++ )
	{
		// add the remaining choices
		cmb_DockFace[SourceFace]->Append(*ValidDockFaces(i)); 
	}

	cmb_DockFace[SourceFace]->Thaw();

	// restore the selected item
	cmb_DockFace[SourceFace]->SetValue(*ValidDockFaces(CurrentSelectedFaceIndex));
}

/**
 * Returns the widget corresponding to the selected item in the "target widget" combo box for the specified face
 *
 * @param	SourceFace	indicates which of the "target widget" combo boxes to retrieve the value from
 */
UUIScreenObject* WxDlgDockingEditor::GetSelectedDockTarget( EUIWidgetFace SourceFace )
{
	UUIScreenObject* Result = NULL;
	if ( cmb_DockTarget[SourceFace]->GetCount() > 0 )
	{
		Result = (UUIScreenObject*)cmb_DockTarget[SourceFace]->GetClientData(cmb_DockTarget[SourceFace]->GetSelection());
	}
	return Result;
}

/**
 * Returns the UIWidgetFace corresponding to the selected item in the "target face" combo box for the specified face
 *
 * @param	SourceFace	indicates which of the "target face" combo boxes to retrieve the value from
 */
EUIWidgetFace WxDlgDockingEditor::GetSelectedDockFace( EUIWidgetFace SourceFace )
{
	wxString Selection;
	if ( cmb_DockFace[SourceFace]->GetCount() > 0 )
	{
		Selection = cmb_DockFace[SourceFace]->GetValue();
	}
	INT Index = Selection.Len() > 0
		? DockFaceStrings.FindItemIndex(Selection.c_str())
		: UIFACE_MAX;

	EUIWidgetFace Result = (EUIWidgetFace)Index;
	return Result;
}

/**
 * Gets the padding value from the "padding" spin control of the specified face
 *
 * @param	SourceFace	indicates which of the "padding" spin controls to retrieve the value from
 */
FLOAT WxDlgDockingEditor::GetSelectedDockPadding( EUIWidgetFace SourceFace )
{
	FLOAT Result = spin_DockPadding[SourceFace]->GetValue();
	return Result;
}

/**
 * Called when the user changes the value of a "target widget" combo box.  Refreshes and validates the 
 * choices available in the "target face" combo for that source face.
 */
void WxDlgDockingEditor::OnChangeDockingTarget( wxCommandEvent& Event )
{
	EUIWidgetFace SourceFace = (EUIWidgetFace)(Event.GetId() - ID_UI_DOCKINGEDITOR_LEFT_TARGET);
	UpdateAvailableDockFaces(SourceFace);
}

/**
 * Called when the user changes the value of a "target face" combo box.  Refreshes and validates the
 * choices available in the "target face" combo for all OTHER source faces.
 */
void WxDlgDockingEditor::OnChangeDockingFace( wxCommandEvent& Event )
{
	EUIWidgetFace SourceFace = (EUIWidgetFace)(Event.GetId() - ID_UI_DOCKINGEDITOR_LEFT_FACE);
	for ( INT i = 0; i < UIFACE_MAX; i++ )
	{
		if ( i != SourceFace )
		{
			UpdateAvailableDockFaces((EUIWidgetFace)i);
		}
	}
}


/**
 * Applies changes made in the dialog to the widget being edited.  Called from the owner window when ShowModal()
 * returns the OK code.
 *
 * @return	TRUE if all changes were successfully applied
 */
UBOOL WxDlgDockingEditor::SaveChanges()
{
	UBOOL bSuccess=FALSE;

	FScopedObjectStateChange DockingChangeNotifier(CurrentWidget);

	// apply the changes
	for ( INT FaceIndex = 0; FaceIndex < UIFACE_MAX; FaceIndex++ )
	{
		UUIScreenObject* TargetWidget = GetSelectedDockTarget((EUIWidgetFace)FaceIndex);
		EUIWidgetFace TargetFace = GetSelectedDockFace((EUIWidgetFace)FaceIndex);
		FLOAT Padding = GetSelectedDockPadding((EUIWidgetFace)FaceIndex);

		bSuccess = CurrentWidget->SetDockTarget((BYTE)FaceIndex, TargetWidget, TargetFace, Padding) && bSuccess;
	}

	if ( !bSuccess )
	{
		DockingChangeNotifier.CancelEdit();
	}

	return bSuccess;
}
