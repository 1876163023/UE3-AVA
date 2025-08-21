/**
 * This widget is used by the UITabControl.  It is always associated with a UITabPage.
 *
 * Copyright 2007 Epic Games, Inc. All Rights Reserved
 */
class UITabButton extends UILabelButton
	native(inherit)
	notplaceable
	HideDropDown;

/*
todo
	- make this widget not selectable in the editor?
	- don't allow the user to bind data fields to this widget (it's handled by the associated tab page)
*/
var()	editconst	editinline	protected	UITabPage	TabPage;

/* == Delegates == */
/**
 * Callback for allowing others to override activation of this button.
 *
 * @param	Sender		the button that is being activated.
 * @param	PlayerIndex	the index [into the Engine.GamePlayers array] for the player that wishes to activate this button.
 */
delegate bool IsActivationAllowed( UITabButton Sender, int PlayerIndex );

/* == Events == */
/**
 * Called immediately after a child has been added to this screen object.
 *
 * This version assigns the TabPage reference to the new child if it's a UITabPage.
 *
 * @param	WidgetOwner		the screen object that the NewChild was added as a child for
 * @param	NewChild		the widget that was added
 */
event AddedChild( UIScreenObject WidgetOwner, UIObject NewChild )
{
	local UITabPage ChildPage;

	Super.AddedChild(WidgetOwner, NewChild);

	if ( WidgetOwner == Self )
	{
		ChildPage = UITabPage(NewChild);
		if ( ChildPage != None )
		{
			if ( TabPage != None && TabPage != ChildPage )
			{
				// remove the old tab page from our Children array
				`log(GetWidgetPathName()@"received new tab page but has existing tab page.  Removing existing page from Children array:" @ TabPage.GetWidgetPathName());
				RemoveChild(TabPage);
			}

			TabPage = ChildPage;
			ChildPage.TabIndex = 0;
		}
	}
}

/**
 * Called immediately after a child has been removed from this screen object.
 *
 * This version clears the TabPage reference, if the child being removed is the current tab page.
 *
 * @param	WidgetOwner		the screen object that the widget was removed from.
 * @param	OldChild		the widget that was removed
 * @param	ExclusionSet	used to indicate that multiple widgets are being removed in one batch; useful for preventing references
 *							between the widgets being removed from being severed.
 *							NOTE: If a value is specified, OldChild will ALWAYS be part of the ExclusionSet, since it is being removed.
 */
event RemovedChild( UIScreenObject WidgetOwner, UIObject OldChild, optional array<UIObject> ExclusionSet )
{
	Super.RemovedChild(WidgetOwner, OldChild, ExclusionSet);

	if ( WidgetOwner == Self && OldChild == TabPage )
	{
		TabPage = None;
	}
}

/**
 * Notification that this widget's parent is about to remove this widget from its children array.  Allows the widget
 * to clean up any references to the old parent.
 *
 * This version clears the tab button's OnClicked delegate.
 *
 * @param	WidgetOwner		the screen object that this widget was removed from.
 */
event RemovedFromParent( UIScreenObject WidgetOwner )
{
	Super.RemovedFromParent(WidgetOwner);

	//@todo ronp - once bool operators for delegates are supported, we can verify that our OnClicked is assigned to a
	// function in the tab control before clearing it.
	OnClicked = None;
}

/* == Natives == */
/**
 * Determines whether this page can be activated.  Calls the IsActivationAllowed delegate to provide other objects
 * a chance to veto the activation of this button.
 *
 * Child classes wishing to perform additional logic for determining whether this button can be activated should hook
 * into the IsActivationAllowed delegate.
 *
 * @param	PlayerIndex	the index [into the Engine.GamePlayers array] for the player that wishes to activate this page.
 *
 * @return	TRUE if this button is allowed to become the active tab button.
 */
native final virtual function bool CanActivateButton( int PlayerIndex );

/**
 * Returns TRUE if this widget has a UIState_TargetedTab object in its StateStack
 */
native final function bool IsTargeted( optional int PlayerIndex=GetBestPlayerIndex() ) const;

/* == UnrealScript == */
/**
 * Simple accessor for getting a reference to this button's page.
 */
function UITabPage GetTabPage()
{
	return TabPage;
}

/* == SequenceAction handlers == */


DefaultProperties
{
	// States
	DefaultStates.Add(class'Engine.UIState_TargetedTab')

	// the tab control handles all navigation between tab buttons, so don't process navigation input events
	Begin Object Name=WidgetEventComponent
		DisabledEventAliases.Add(NextControl)
		DisabledEventAliases.Add(PrevControl)
		DisabledEventAliases.Add(NavFocusUp)
		DisabledEventAliases.Add(NavFocusDown)
		DisabledEventAliases.Add(NavFocusLeft)
		DisabledEventAliases.Add(NavFocusRight)
	End Object

	// Use a custom image style for the background image
	Begin Object Name=BackgroundImageTemplate
		ImageStyle=(DefaultStyleTag="TabButtonBackgroundStyle",RequiredStyleClass=class'Engine.UIStyle_Image')
	End Object

	Begin Object Name=LabelStringRenderer
		StringStyle=(DefaultStyleTag="DefaultTabButtonStringStyle",RequiredStyleClass=class'Engine.UIStyle_Combo')
	End Object
}
