/*=============================================================================
	GroupBrowser.h: UnrealEd's group browser.
	Copyright © 2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __GROUPBROWSER_H__
#define __GROUPBROWSER_H__

// Forward declarations.
class WxListView;

class WxGroupBrowser : public WxBrowser
{
	DECLARE_DYNAMIC_CLASS(WxGroupBrowser);

public:
	WxGroupBrowser();

	/**
	 * Forwards the call to our base class to create the window relationship.
	 * Creates any internally used windows after that
	 *
	 * @param DockID the unique id to associate with this dockable window
	 * @param FriendlyName the friendly name to assign to this window
	 * @param Parent the parent of this window (should be a Notebook)
	 */
	virtual void Create(INT DockID,const TCHAR* FriendlyName,wxWindow* Parent);

	/**
	 * Loops through all the actors in the world and assembles a list of unique group names.
	 * Actors can belong to multiple groups by separating the group names with commas.
	 */
	virtual void Update();

	virtual void Activated();

	/**
	 * Returns the key to use when looking up values.
	 */
	virtual const TCHAR* GetLocalizationKey() const;

protected:
	/**
	 * When FALSE, we don't process toggle messages for the checked list box.  This is necessary
	 * at certain times when processing things in batch.
	 */
	UBOOL bAllowToggling;

	/**
	 * TRUE if an update was requested while the browser tab wasn't active.
	 * The browser will Update() the next time this browser tab is Activated().
	 */
	UBOOL bUpdateOnActivated;

	wxCheckListBox*		GroupList;
	WxListView*			ActorList;
	wxSplitterWindow*	SplitterWindow;

	/**
	 * Selects/deselects actors by group.
	 * @return		TRUE if at least one actor was selected.
	 */
	UBOOL SelectActors(UBOOL InSelect);

	/** Loops through all actors in the world and updates their visibility based on which groups are checked. */
	void UpdateActorVisibility();

	/** Populates the actor list with actors in the selected groups. */
	void PopulateActorList();

private:
	////////////////////
	// Wx events.

	/** Adds level-selected actors to the selected grops. */
	void OnAddToGroup( wxCommandEvent& In );
	/** Deletes level-selected actors from the selected groups. */
	void OnDeleteSelectedActorsFromGroups( wxCommandEvent& In );
	/** Called when a list item check box is toggles. */
	void OnToggled( wxCommandEvent& In );
	/** Called when item selection in the group list changes. */
	void OnGroupSelectionChange( wxCommandEvent& In );
	/** Called when a list item in the group browser is double clicked. */
	void OnDoubleClick( wxCommandEvent& In );
	/** Creates a new group. */
	void OnNewGroup( wxCommandEvent& In );
	/** Deletes a group. */
	void OnDeleteGroup( wxCommandEvent& In );
	/** Presents a context menu for group list items. */
	void OnRightButtonDown( wxMouseEvent& In );
	/** Handler for IDM_RefreshBrowser events; updates the browser contents. */
	void OnRefresh( wxCommandEvent& In );
	/** Selects actors in the selected groups. */
	void OnSelect( wxCommandEvent& In );
	/** Deselects actors in the selected groups. */
	void OnDeselect( wxCommandEvent& In );
	/** Sets all groups to visible. */
	void OnAllGroupsVisible( wxCommandEvent& In );
	/** Presents a rename dialog for each selected group. */
	void OnRename( wxCommandEvent& In );
	/** Responds to size events by updating the splitter. */
	void OnSize(wxSizeEvent& In);

	DECLARE_EVENT_TABLE();
};

#endif // __GROUPBROWSER_H__
