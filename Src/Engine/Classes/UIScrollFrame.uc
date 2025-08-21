/**
 * This  widget defines a region in which its child widgets can be placed. If any of its children lay outside of its
 * defined region then a scroll bar will be made visible to allow the region to be scrolled to the outside widgets.
 *
 * Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved
 *
 */
class UIScrollFrame extends UIContainer
	notplaceable		// notplaceable until it's finished.
	native(UIPrivate);

/** Component for rendering the background image */
var(Image)	editinline	const				UIComp_DrawImage		BackgroundImageComponent;

/**
 * The image that represents the scrollbar background
 */
var deprecated	UITexture Background;
/**
 * the texture atlas coordinates for the scrollbar background. Values of 0 indicate that
 * the texture is not part of an atlas.
 */
var deprecated TextureCoordinates	BackgroundCoordinates;

/**
 * Specifies wheather to render the background texture or not
 */
var(Image)									bool					bDisplayBackground;

/** A scrollbar widget that allows the entire RegionExtent to be scrolled to for the horizontal dimension. */
var editinline const noclear private 		UIScrollbar				ScrollbarHorizontal;

/** A scrollbar widget that allows the entire RegionExtent to be scrolled to for the horizontal dimension. */
var editinline const noclear private 		UIScrollbar				ScrollbarVertical;

/**
 * This struct defines the full region in which all of its child widgets reside within. If this region is greater than the
 * size of this widget in either dimension, scroll bars will be made visible in the corresponding dimension.
 */
var		private								UIScreenValue_Bounds	RegionExtent;

/** Indicates that an event has occured that requires that the region be recalculated. */
var		private	transient					bool					bResolveRegion;

cpptext
{
	/* === UUIScrollFrame interface === */
	/**
	 * Changes the background image for this scroll frame, creating the wrapper UITexture if necessary.
	 *
	 * @param	NewBackgroundImage		the new surface to use for the scroll frame's background image
	 */
	void SetBackgroundImage( USurface* NewBackgroundImage );

	/**
	 * Iterates through this widgets child and determines the outermost extend that will encapsulate all of them.
	 * It will set the RegionExtent member appropriately.
	 */
	void CalculateRegionExtent( );

	/* === UIObject interface === */
	/**
	 * Provides a way for widgets to fill their style subscribers array prior to performing any other initialization tasks.
	 *
	 * This version adds the BackgroundImageComponent (if non-NULL) to the StyleSubscribers array.
	 */
	virtual void InitializeStyleSubscribers();

	/* === UUIScreenObject interface === */
	/**
	 * Initializes the buttons and creates the background image.
	 *
	 * @param	inOwnerScene	the scene to add this widget to.
	 * @param	inOwner			the container widget that will contain this widget.  Will be NULL if the widget
	 *							is being added to the scene's list of children.
	 */
	virtual void Initialize( UUIScene* inOwnerScene, UUIObject* inOwner=NULL );

	/**
	 * Generates a array of UI Action keys that this widget supports.
	 *
	 * @param	out_KeyNames	Storage for the list of supported keynames.
	 */
	virtual void GetSupportedUIActionKeyNames(TArray<FName> &out_KeyNames );

	/**
	 * Evalutes the Position value for the specified face into an actual pixel value.  Should only be
	 * called from UIScene::ResolvePositions.  Any special-case positioning should be done in this function.
	 *
	 * @param	Face	the face that should be resolved
	 */
	virtual void ResolveFacePosition( EUIWidgetFace Face );

	/**
	 * Render this scroll frame.
	 *
	 * @param	Canvas	the FCanvas to use for rendering this widget
	 */
	void Render_Widget( FCanvas* Canvas );

	/**
	 * Allow the widget to do any special rendering after its children have been rendered.
	 * This function draws the scrollframe visible region outline
	 *
	 * @param	Canvas	the canvas to use for rendering this widget
	 */
	virtual void PostRender_Widget( FCanvas* Canvas );

	/**
	 * Called immediately after a child has been added to this screen object.
	 *
	 * @param	WidgetOwner		the screen object that the NewChild was added as a child for
	 * @param	NewChild		the widget that was added
	 */
	virtual void NotifyAddedChild( UUIScreenObject* WidgetOwner, UUIObject* NewChild );

	/**
	 * Called immediately after a child has been removed from this screen object.
	 *
	 * @param	WidgetOwner		the screen object that the widget was removed from.
	 * @param	OldChild		the widget that was removed
	 * @param	ExclusionSet	used to indicate that multiple widgets are being removed in one batch; useful for preventing references
	 *							between the widgets being removed from being severed.
	 */
	virtual void NotifyRemovedChild( UUIScreenObject* WidgetOwner, UUIObject* OldChild, TArray<UUIObject*>* ExclusionSet=NULL );

	/**
	 * Called when a property is modified that could potentially affect the widget's position onscreen.
	 */
	virtual void RefreshPosition();

	/**
	 * Called to globally update the formatting of all UIStrings.
	 */
	virtual void RefreshFormatting();

	/* === UObject interface === */
	/**
	 * Called when a property value from a member struct or array has been changed in the editor, but before the value has actually been modified.
	 */
	virtual void PreEditChange( FEditPropertyChain& PropertyThatChanged );

	/**
	 * Called when a property value from a member struct or array has been changed in the editor.
	 */
	virtual void PostEditChange( FEditPropertyChain& PropertyThatChanged );

	/**
	 * Called after this object has been completely de-serialized.  This version migrates values for the deprecated Background,
	 * BackgroundCoordinates, and PrimaryStyle properties over to the BackgroundImageComponent.
	 */
	virtual void PostLoad();

protected:

	/**
	 * Handles input events for this editbox.
	 *
	 * @param	EventParms		the parameters for the input event
	 *
	 * @return	TRUE to consume the key event, FALSE to pass it on.
	 */
	virtual UBOOL ProcessInputKey( const struct FSubscribedInputEventParameters& EventParms );

	/**
	 * Enables and initializes the scrollbars that need to be visible
	 */
	void RefreshScrollbars();

	/**
	 * Scrolls all of the child widgets by the specified amount in the specified direction.
	 *
	 * @param	Dimension		indicates whether the horizontal or vertical scrollbar should be evaluated.
	 * @param	PositionChange	indicates the amount that the scrollbar has travelled.
	 * @param	bPositionMaxed	indicates that the scrollbar's marker has reached its farthest available position,
	 *                          used to achieve pixel exact scrolling
	 */
	void ScrollRegion(  EUIOrientation Dimension, FLOAT PositionChange, UBOOL bPositionMaxed/*=FALSE*/ );

	/**
	 * Takes position change given in previously defined scrollbar units and translates it into actual pixel values by which the UIScrollframe's
	 * content should be moved. To achieve pixel exact scrolling function also does special bounds evaluation when scrollbar's marker has reached
	 * its rightmost/leftmost position which is indicated by the bPositionMaxed flag.
     *
	 * @param	Dimension		indicates whether the horizontal or vertical scrollbar should be evaluated.
	 * @param	PositionChange	indicates the amount that the scrollbar has travelled.
	 * @param	bPositionMaxed	indicates that the scrollbar's marker has reached its farthest available position,
	 *                          used to achieve pixel exact scrolling
	 *
	 * @return	number of pixels by which the scrollframe contents should be scrolled
	 */
	FLOAT CalculatedPositionChange( EUIOrientation Dimension, FLOAT PositionChange, UBOOL bPositionMaxed);
}

/**
 * Insert a widget at the specified location
 *
 * @param	NewChild		the widget to insert
 * @param	InsertIndex		the position to insert the widget.  If not specified, the widget is insert at the end of
 *							the list
 * @param	bRenameExisting	controls what happens if there is another widget in this widget's Children list with the same tag as NewChild.
 *							if TRUE, renames the existing widget giving a unique transient name.
 *							if FALSE, does not add NewChild to the list and returns FALSE.
 *
 * @return	the position that that the child was inserted in, or INDEX_NONE if the widget was not inserted
 */
native function int InsertChild( UIObject NewChild, optional int InsertIndex = INDEX_NONE, optional bool bRenameExisting=true );

/**
 * Handler for responding to child widget changing its position, it recalculates the scrollframe's scroll region bounds
 *
 * @param	Sender	Child widget which has been repositioned
 */
native final function OnChildRepositioned( UIScreenObject Sender );

/**
 * Attempts to scroll the scroll frame by the PositionChange value passed by the scrollbar.
 *
 * @return	TRUE if the scroll frame was able to scroll. FALSE if the could not scroll such as if the RegionExtent is not greater than the scroll frame's bounds.
 */
native final function bool ScrollHorizontal( float PositionChange, optional bool bPositionMaxed=false );

/**
 * Attempts to scroll the scroll frame by the PositionChange value passed by the scrollbar.
 *
 * @return	TRUE if the scroll frame was able to scroll. FALSE if the could not scroll such as if the RegionExtent is not greater than the scroll frame's bounds.
 */
native final function bool ScrollVertical( float PositionChange, optional bool bPositionMaxed=false );

/**
 * Initializes the clicked delegates in the increment, decrement and marker buttons.
 * @todo - this is a fix for the issue where delegates don't seem to be getting set properly in defaultproperties blocks.
 */
event Initialized()
{
	Super.Initialized();

	ScrollbarHorizontal.OnScrollActivity = ScrollHorizontal;
	ScrollbarVertical.OnScrollActivity   = ScrollVertical;

	// Private Behavior
	ScrollbarHorizontal.SetPrivateBehavior(PRIVATE_NotDockable | PRIVATE_TreeHidden, true);
	ScrollbarVertical.SetPrivateBehavior(PRIVATE_NotDockable | PRIVATE_TreeHidden, true);
}

/**
 * Called immediately after a child has been added to this screen object.  Sets up NotifyPositionChanged delegate in the added child
 *
 * @param	WidgetOwner		the screen object that the NewChild was added as a child for
 * @param	NewChild		the widget that was added
 */
event AddedChild( UIScreenObject WidgetOwner, UIObject NewChild )
{
	Super.AddedChild(WidgetOwner, NewChild);

	NewChild.NotifyPositionChanged = OnChildRepositioned;
}

/**
 * Called immediately after a child has been removed from this screen object.  Clears the NotifyPositionChanged delegate in the removed child
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

	//@todo ronp - need to add support for delegate comparisons so that we can determine whether this delegate is assigned
	// to a function in one of the widgets contained in the exclusion set
	OldChild.NotifyPositionChanged = None;
}

DefaultProperties
{
	PrimaryStyle=(DefaultStyleTag="DefaultImageStyle",RequiredStyleClass=class'Engine.UIStyle_Image')
	bSupportsPrimaryStyle=false

	// Set the region extent to pixel viewport scaletype.
	RegionExtent=(ScaleType[0]=EVALPOS_PixelViewport,ScaleType[1]=EVALPOS_PixelViewport,ScaleType[2]=EVALPOS_PixelViewport,ScaleType[3]=EVALPOS_PixelViewport)

	// By default display the debug texture
	bDisplayBackground=true

	// Tell the frame to resolve its region extent on creation.
	bResolveRegion=true
}
