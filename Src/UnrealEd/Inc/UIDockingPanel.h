/*=============================================================================
	UIDockingPanel.h: Wx Panel that lets the user change docking settings for the currently selected widget. 
	Copyright © 2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __UIDOCKINGPANEL_H__
#define __UIDOCKINGPANEL_H__

/** Panel that lets the user change docking settings for the currently selected widget. */
class WxUIDockingPanel : public wxScrolledWindow, public FSerializableObject
{
public:
	WxUIDockingPanel(class WxUIEditorBase* InEditor);

	/**
	 * Destructor
	 * Saves the window position
	 */
	virtual ~WxUIDockingPanel();

	// FSerializableObject interface
	virtual void Serialize( FArchive& Ar );

	/** 
	 * Sets which widgets are currently selected and updates the panel accordingly.
	 *
	 * @param SelectedWidgets	The array of currently selected widgets.
	 */
	void SetSelectedWidgets(TArray<UUIObject*> &SelectedWidgets);

	/**
	 * Refreshes the widgets of the docking panel with new values.
	 */
	void RefreshControls();

private:

	/**
	 * Refreshes and validates the list of faces which can be used as the dock target face for the specified face.  Removes any entries that
	 * are invalid for this face (generally caused by restrictions on docking relationships necessary to prevent infinite recursion)
	 *
	 * @param	SourceFace	indicates which of the combo boxes should be validated & updated
	 */
	void UpdateAvailableDockFaces( EUIWidgetFace SourceFace );

	/**
	 * Returns the widget corresponding to the selected item in the "target widget" combo box for the specified face
	 *
	 * @param	SourceFace	indicates which of the "target widget" combo boxes to retrieve the value from
	 */
	UUIScreenObject* GetSelectedDockTarget( EUIWidgetFace SourceFace );

	/**
	 * Returns the UIWidgetFace corresponding to the selected item in the "target face" combo box for the specified face
	 *
	 * @param	SourceFace	indicates which of the "target face" combo boxes to retrieve the value from
	 */
	EUIWidgetFace GetSelectedDockFace( EUIWidgetFace SourceFace );

	/**
	 * Gets the padding value from the "padding" spin control of the specified face
	 *
	 * @param	SourceFace	indicates which of the "padding" spin controls to retrieve the value from
	 */
	FLOAT GetSelectedDockPadding( EUIWidgetFace SourceFace );

	/**
	 * Fills the "dock target" combos with the names of the widgets contained within Container that are valid dock targets for InWidget.
	 */
	void InitializeTargetCombos();

	/**
	* Called when the user changes the value of a "target widget" combo box.  Refreshes and validates the 
	* choices available in the "target face" combo for that source face.
	*/
	void OnChangeDockingTarget( wxCommandEvent& Event );

	/**
	 * Called when the user changes the value of a "target face" combo box.  Refreshes and validates the
	 * choices available in the "target face" combo for all OTHER source faces.
	 */
	void OnChangeDockingFace( wxCommandEvent& Event );

	/**
	 * Called when the user changes the padding for a dock face. 
	 */
	void OnChangeDockingPadding( wxSpinEvent& Event );

	/** Pointer to the UI Editor that owns this panel. */
	WxUIEditorBase* UIEditor;

	/** the widget we're editing */
	UUIObject*			CurrentWidget;

	/** store the list of widgets contained by Container for quick lookup */
	TArray<UUIObject*>	ValidDockTargets;

	/** the localized names for each dock face */
	TArray<FString>		DockFaceStrings;

	/** the labels for each docking set */
	wxStaticText*	lbl_DockingSet[UIFACE_MAX];

	/** the combo for choosing the docking target widget, per face */
	wxComboBox*		cmb_DockTarget[UIFACE_MAX];

	/** the combo for choosing the docking target face, per face */
	wxComboBox*		cmb_DockFace[UIFACE_MAX];

	/** the numeric control for choosing the dock padding for each docking set */
	WxSpinCtrlReal*		spin_DockPadding[UIFACE_MAX];

	/** the OK button */
	wxButton*		btn_OK;

	DECLARE_EVENT_TABLE()
};

#endif

