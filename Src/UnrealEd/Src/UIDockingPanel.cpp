/*=============================================================================
	UIDockingPanel.cpp: Wx Panel that lets the user change docking settings for the currently selected widget. 
	Copyright 1998-2007 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"

#include "EngineUIPrivateClasses.h"
#include "UnrealEdPrivateClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineUISequenceClasses.h"

#include "UnObjectEditor.h"
#include "UnUIEditor.h"
#include "UIDockingPanel.h"
#include "ScopedTransaction.h"
#include "ScopedPropertyChange.h"

BEGIN_EVENT_TABLE(WxUIDockingPanel, wxPanel)
	EVT_SPINCTRL(ID_UI_DOCKINGEDITOR_LEFT_PADDING,		WxUIDockingPanel::OnChangeDockingPadding)
	EVT_SPINCTRL(ID_UI_DOCKINGEDITOR_TOP_PADDING,		WxUIDockingPanel::OnChangeDockingPadding)
	EVT_SPINCTRL(ID_UI_DOCKINGEDITOR_RIGHT_PADDING,		WxUIDockingPanel::OnChangeDockingPadding)
	EVT_SPINCTRL(ID_UI_DOCKINGEDITOR_BOTTOM_PADDING,	WxUIDockingPanel::OnChangeDockingPadding)

	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_LEFT_TARGET,WxUIDockingPanel::OnChangeDockingTarget)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_TOP_TARGET,WxUIDockingPanel::OnChangeDockingTarget)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_RIGHT_TARGET,WxUIDockingPanel::OnChangeDockingTarget)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_BOTTOM_TARGET,WxUIDockingPanel::OnChangeDockingTarget)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_LEFT_FACE,WxUIDockingPanel::OnChangeDockingFace)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_TOP_FACE,WxUIDockingPanel::OnChangeDockingFace)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_RIGHT_FACE,WxUIDockingPanel::OnChangeDockingFace)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_BOTTOM_FACE,WxUIDockingPanel::OnChangeDockingFace)
END_EVENT_TABLE()

/** Wx Panel that lets the user change docking settings for the currently selected widget. */
WxUIDockingPanel::WxUIDockingPanel(WxUIEditorBase* InEditor) : 
wxScrolledWindow(InEditor),
UIEditor(InEditor)
{
	wxBoxSizer* BaseVSizer = new wxBoxSizer(wxVERTICAL);
	{
		wxBoxSizer* DockingSetVSizer = new wxBoxSizer(wxVERTICAL);
		{
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
				{
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
					spin_DockPadding[i] = new WxSpinCtrlReal(this, ID_UI_DOCKINGEDITOR_LEFT_PADDING + i, 0.0f, wxDefaultPosition, wxSize(60,-1), -1000.0f, 1000.0f);
					spin_DockPadding[i]->SetHelpText(*DockPaddingHelpText);
					spin_DockPadding[i]->SetToolTip(*DockPaddingToolTip);
					DockFaceHSizer->Add( spin_DockPadding[i], 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5);
				}
				DockFaceVSizer->Add(DockFaceHSizer, 0, wxGROW|wxALL, 0);
			}

		}
		BaseVSizer->Add(DockingSetVSizer, 0, wxGROW|wxALL, 5);

	}
	SetSizer(BaseVSizer);

	// Set a scrollbar for this panel.
	EnableScrolling(false, true);
	SetScrollRate(0,1);

	CurrentWidget = NULL;
}

WxUIDockingPanel::~WxUIDockingPanel()
{
	
}

void WxUIDockingPanel::Serialize( FArchive& Ar )
{
	Ar << CurrentWidget << ValidDockTargets;
}

/** 
 * Sets which widgets are currently selected and updates the panel accordingly.
 *
 * @param SelectedWidgets	The array of currently selected widgets.
 */
void WxUIDockingPanel::SetSelectedWidgets(TArray<UUIObject*> &SelectedWidgets)
{
	// currently we only support changing docking options for 1 widget a time, so only enable the panel if exactly 1 widget is selected.
	if(SelectedWidgets.Num() == 1)
	{
		CurrentWidget = SelectedWidgets(0);
	
		// the widget can never dock to itself, so remove it from the list of valid targets right away
		ValidDockTargets = CurrentWidget->GetScene()->GetChildren(TRUE);
		ValidDockTargets.RemoveItem(CurrentWidget);

		Enable(TRUE);
	}
	else
	{
		Enable(FALSE);
	}

	RefreshControls();
}

/**
 * Refreshes the widgets of the docking panel with new values.
 */
void WxUIDockingPanel::RefreshControls()
{
	// If we are disabled, clear out all controls.
	if(IsEnabled() == FALSE)
	{
		for ( INT FaceIndex = 0; FaceIndex < UIFACE_MAX; FaceIndex++ )
		{
			cmb_DockTarget[FaceIndex]->SetValue(cmb_DockTarget[FaceIndex]->GetString(0));
			cmb_DockFace[FaceIndex]->SetValue(*DockFaceStrings(UIFACE_MAX));
			spin_DockPadding[FaceIndex]->SetValue(0.0f);
		}
	}
	else
	{
		// fill the combos with data
		InitializeTargetCombos();
	}

}

/**
 * Fills the "dock target" combos with the names of the widgets contained within Container that are valid dock targets for InWidget.
 */
void WxUIDockingPanel::InitializeTargetCombos()
{
	for ( INT FaceIndex = 0; FaceIndex < UIFACE_MAX; FaceIndex++ )
	{
		EUIWidgetFace Face = (EUIWidgetFace)FaceIndex;

		cmb_DockTarget[FaceIndex]->Freeze();

		// clear the existing contents
		cmb_DockTarget[FaceIndex]->Clear();

		INT SelectionIndex=0;

		// add the 'None' item
		cmb_DockTarget[FaceIndex]->Append(*LocalizeUnrealEd(TEXT("None")), (void*)NULL);

		FString SceneTag = FString::Printf(TEXT("%s (%s)"), *UIEditor->OwnerScene->GetTag().ToString(), *LocalizeUI("UIEditor_Scene"));
		INT idx = cmb_DockTarget[FaceIndex]->Append(*SceneTag, UIEditor->OwnerScene);
		if ( CurrentWidget->DockTargets.IsDocked(Face,FALSE) && CurrentWidget->DockTargets.GetDockTarget(Face) == NULL )
		{
			// if the widget is docked, but GetDockTarget returns NULL, the widget is docked to the scene
			SelectionIndex = idx;
		}

		for ( INT ChildIndex = 0; ChildIndex < ValidDockTargets.Num(); ChildIndex++ )
		{
			UUIObject* Child = ValidDockTargets(ChildIndex);
			idx = cmb_DockTarget[FaceIndex]->Append( *Child->GetTag().ToString(), Child );
			if ( CurrentWidget->DockTargets.GetDockTarget(Face) == Child )
			{
				SelectionIndex = idx;
			}
		}

		cmb_DockTarget[FaceIndex]->Thaw();

		// slight hack here because SetSelection doesn't update the combo's m_selectionOld member, which causes
		// the combo to generate two "selection changed" events the first time the user selects an item
		//cmb_DockTarget[FaceIndex]->SetSelection(SelectionIndex);
		wxString SelectedWidgetString = cmb_DockTarget[FaceIndex]->GetString(SelectionIndex);
		cmb_DockTarget[FaceIndex]->SetValue(SelectedWidgetString);

		// set the value of the face combo
		cmb_DockFace[FaceIndex]->SetValue(*DockFaceStrings(CurrentWidget->DockTargets.GetDockFace(Face)));

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
void WxUIDockingPanel::UpdateAvailableDockFaces( EUIWidgetFace SourceFace )
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
						ValidDockFaces.RemoveItem(DockFaceStrings(i));
						if ( CurrentFace == UIFACE_Left || CurrentFace == UIFACE_Top )
						{
							// if the face currently being evaluated in the target is the left or top face, it also illegal to
							// dock to the opposing side of the target, since that face is dependent on this one
							ValidDockFaces.RemoveItem(DockFaceStrings(i+2));
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
	{
		// now clear the existing entries
		cmb_DockFace[SourceFace]->Clear();
		for ( INT i = 0; i < ValidDockFaces.Num(); i++ )
		{
			// add the remaining choices
			cmb_DockFace[SourceFace]->Append(*ValidDockFaces(i)); 
		}

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
UUIScreenObject* WxUIDockingPanel::GetSelectedDockTarget( EUIWidgetFace SourceFace )
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
EUIWidgetFace WxUIDockingPanel::GetSelectedDockFace( EUIWidgetFace SourceFace )
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
FLOAT WxUIDockingPanel::GetSelectedDockPadding( EUIWidgetFace SourceFace )
{
	FLOAT Result = spin_DockPadding[SourceFace]->GetValue();

	return Result;
}

/**
 * Called when the user changes the value of a "target widget" combo box.  Refreshes and validates the 
 * choices available in the "target face" combo for that source face.
 */
void WxUIDockingPanel::OnChangeDockingTarget( wxCommandEvent& Event )
{
	EUIWidgetFace SourceFace = (EUIWidgetFace)(Event.GetId() - ID_UI_DOCKINGEDITOR_LEFT_TARGET);
	UpdateAvailableDockFaces(SourceFace);

	UUIScreenObject* TargetWidget = GetSelectedDockTarget(SourceFace);
	EUIWidgetFace TargetFace = GetSelectedDockFace(SourceFace);
	FLOAT Padding = GetSelectedDockPadding(SourceFace);

	FScopedTransaction Transaction(*LocalizeUI(TEXT("TransEditDocking")));
	FScopedObjectStateChange DockTargetChangeNotifier(CurrentWidget);

	if ( !CurrentWidget->SetDockTarget((BYTE)SourceFace, TargetWidget, TargetFace, Padding) )
	{
		DockTargetChangeNotifier.CancelEdit();
	}
}

/**
 * Called when the user changes the value of a "target face" combo box.  Refreshes and validates the
 * choices available in the "target face" combo for all OTHER source faces.
 */
void WxUIDockingPanel::OnChangeDockingFace( wxCommandEvent& Event )
{
	EUIWidgetFace SourceFace = (EUIWidgetFace)(Event.GetId() - ID_UI_DOCKINGEDITOR_LEFT_FACE);
	for ( INT i = 0; i < UIFACE_MAX; i++ )
	{
		if ( i != SourceFace )
		{
			UpdateAvailableDockFaces((EUIWidgetFace)i);
		}
	}

	UUIScreenObject* TargetWidget = GetSelectedDockTarget(SourceFace);
	EUIWidgetFace TargetFace = GetSelectedDockFace(SourceFace);
	FLOAT Padding = GetSelectedDockPadding(SourceFace);

	FScopedTransaction Transaction(*LocalizeUI(TEXT("TransEditDocking")));
	FScopedObjectStateChange DockFaceChangeNotifier(CurrentWidget);

	if ( !CurrentWidget->SetDockTarget((BYTE)SourceFace, TargetWidget, TargetFace, Padding) )
	{
		DockFaceChangeNotifier.CancelEdit();
	}
}

/**
 * Called when the user changes the padding for a dock face. 
 */
void WxUIDockingPanel::OnChangeDockingPadding( wxSpinEvent& Event )
{
	if(CurrentWidget != NULL)
	{
		FScopedTransaction Transaction(*LocalizeUI(TEXT("TransEditDocking")));
		FScopedObjectStateChange DockPaddingChangeNotifier(CurrentWidget);

		EUIWidgetFace SourceFace = (EUIWidgetFace)(Event.GetId() - ID_UI_DOCKINGEDITOR_LEFT_PADDING);

		UUIScreenObject* TargetWidget = GetSelectedDockTarget(SourceFace);
		EUIWidgetFace TargetFace = GetSelectedDockFace(SourceFace);
		FLOAT Padding = GetSelectedDockPadding(SourceFace);

		if ( !CurrentWidget->SetDockTarget((BYTE)SourceFace, TargetWidget, TargetFace, Padding) )
		{
			DockPaddingChangeNotifier.CancelEdit();
		}
	}
}











//EOF




