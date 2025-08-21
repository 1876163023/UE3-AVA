/**
 * This component when integrated into a widget allows for scrolling the contents of the widget, i.e. UIList
 * UIScrollbar has build in functionality to autoposition itself within the owner widget depending on its orientation
 *
 * Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved.
 */
class UIScrollbar extends UIObject
	native(UIPrivate)
	hidecategories(Object,UIScreenObject,UIObject,Focus,Presentation,Splitscreen,States)
	notplaceable;

cpptext
{
	/* === UUIScrollbar interface === */
	/**
	 * Changes the background image for this slider.
	 *
	 * @param	NewBackgroundImage		the new surface to use for the slider's background image
	 */
	void SetBackgroundImage( USurface* NewBackgroundImage );

	/**
	 * Returns the size of the scroll-zone (the region between the decrement and increment buttons), along the same orientation as the scrollbar.
	 *
	 * @param	ScrollZoneStart	receives the value of the location of the beginning of the scroll zone, in pixels relative to the scrollbar.
	 *
	 * @return	the height (if the scrollbar's orientation is vertical) or width (if horizontal) of the region between the
	 *			increment and decrement buttons, in pixels.
	 *
	 * @note: noexport for handling the optional out parameter correctly.
	 */
	FLOAT GetScrollZoneExtent( FLOAT* ScrollZoneStart=NULL ) const;

	/**
	 * Verifies that marker can be moved by the given PositionChange value. If the PositionChange is too large
	 * and would cause the marker to extend beyond the increment or decrement buttons, then it will be clamped to a
	 * value by which will the marker can move and not extend beyond its bar region
	 */
	FLOAT GetClampedPositionChange(FLOAT PositionChange);

	/**
	 *	Shifts the position of the marker button by the amount specified, clamps the PositionChange if it would extend the marker pass the increment/decrement buttons
	 *  the direction of shift is based on the sign of the PositionChange and the ScrollbarOrientation setting
	 *
	 *  @param	PositionChange	the amount of pixels that the marker widget will be shifted by, supply
	 *                          negative value to shift marker in opposite direction
	 */
	void UpdateMarkerPosition(FLOAT PositionChange);

	/**
	 * Sets up marker button position based on the value of MarkerPosPercent
	 */
	void ResolveMarkerPosition();

	/**
	 * Sets up marker button bounds based on the value of MarkerSizePercent
	 */
	void ResolveMarkerSize();

	/**
	 * Resolves the NudgeValue into actual pixels
	 */
	void ResolveNudgeSize();

	/**
	 * Responsible for handling of the marker dragging, it reads mouse position and slides the marker in the
	 * appropriate direction
	 */
	void ProcessDragging();

	/**
	 * Responsible for handling paging which is invoked by mouse clicks on the empty bar space
	 */
	void ScrollZoneClicked( const struct FInputEventParameters& EventParms );

	/**
	 * Function overwritten to autoposition the scrollbar within the owner widget
	 *
	 * @param	Face	the face that should be resolved
	 */
	void ResolveFacePosition( EUIWidgetFace Face );

	/**
	 * Called when a property is modified that could potentially affect the widget's position onscreen.
	 */
	void RefreshPosition();

	/**
	 * Called to globally update the formatting of all UIStrings.
	 */
	virtual void RefreshFormatting();

	/**
	 *	Sets a private flag to refresh the bound of the scrollbar
	 */
	void RefreshMarker();

	/**
	 * Render this scrollbar.
	 *
	 * @param	Canvas	the canvas to use for rendering this widget
	 */
	virtual void Render_Widget( FCanvas* Canvas );

	/**
	 * Generates a array of UI Action keys that this widget supports.
	 *
	 * @param	out_KeyNames	Storage for the list of supported keynames.
	 */
	virtual void GetSupportedUIActionKeyNames(TArray<FName> &out_KeyNames );

	/**
	 * Callback that happens the first time the scene is rendered, any widget positioning initialization should be done here.
	 *
	 * By default this function recursively calls itself on all of its children.
	 */
	virtual void PreRenderCallback();

protected:

	/**
	 * Initializes the button and creates the bar image.
	 *
	 * @param	inOwnerScene	the scene to add this widget to.
	 * @param	inOwner			the container widget that will contain this widget.  Will be NULL if the widget
	 *							is being added to the scene's list of children.
	 */
	virtual void Initialize( UUIScene* inOwnerScene, UUIObject* inOwner=NULL );

	/**
	 * Provides a way for widgets to fill their style subscribers array prior to performing any other initialization tasks.
	 *
	 * This version adds the image components (if non-NULL) to the StyleSubscribers array.
	 */
	virtual void InitializeStyleSubscribers();

	/**
	 * Handles input events for this editbox.
	 *
	 * @param	EventParms		the parameters for the input event
	 *
	 * @return	TRUE to consume the key event, FALSE to pass it on.
	 */
	virtual UBOOL ProcessInputKey( const struct FSubscribedInputEventParameters& EventParms );

	/**
	 * Sets up dock links between the scrollbar and its owner widget as well as child buttons of scrollbar.
	 * Docking links are used as a mechanism to autoposition the UUIScrollbar within its owner widget
	 *
	 * @param	bResetLinks		seting flag to TRUE will cause all existing links to be refreshed
	 */
	void SetupDocLinks( UBOOL bResetLinks = FALSE );

	/**
	 * Called when a style reference is resolved successfully.
	 *
	 * @param	ResolvedStyle			the style resolved by the style reference
	 * @param	StyleProperty			the name of the style reference property that was resolved.
	 * @param	ArrayIndex				the array index of the style reference that was resolved.  should only be >0 for style reference arrays.
	 * @param	bInvalidateStyleData	if TRUE, the resolved style is different than the style that was previously resolved by this style reference.
	 */
	void OnStyleResolved( UUIStyle* ResolvedStyle, const struct FStyleReferenceId& StyleProperty, INT ArrayIndex, UBOOL bInvalidateStyleData );

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
	 * Called after this object has been completely de-serialized.  This version migrates values for the deprecated background
	 * image properties over to the BackgroundImageComponent.
	 */
	virtual void PostLoad();
}

/** Component for rendering the background image */
var(Image)	editinline	const	noclear	UIComp_DrawImage		BackgroundImageComponent;

var private{private} deprecated	UITexture				Background;
var private{private} deprecated	TextureCoordinates		BackgroundCoordinates;

/**
 * Buttons that can be used to increment and decrement the marker of the scrollbar.
 */
var	private		const			UINumericEditBoxButton		IncrementButton;
var	private		const			UINumericEditBoxButton		DecrementButton;

/**
 * The marker which can be manipulated to change the value of the scrollbar
 */
var	private		const			UIScrollbarMarkerButton		MarkerButton;

/**
 * The styles used for the increment, decrement and marker buttons
 */
var		private								UIStyleReference		IncrementStyle;
var		private								UIStyleReference		DecrementStyle;
var		private								UIStyleReference		MarkerStyle;

/**
 * The nudge value indicates how much marker movement will cause one position update "tick",
 * value is stored in pixels.  It can be set to 1 as is the case with the UIScrollframe, or it can be evaluated
 * based on some percentage of the total available size as is the case with the UIList
 */
var		private{private}					float					NudgeValue;

/**
 * The NudgeMultiplier value indicates by how many NudgeValues will the marker move when the increment or decrement button is pressed,
 * NudgeValue is multiplied by NudgeMultiplier to obtain final pixel amount
 */
var()										float					NudgeMultiplier;

/**
 * Values which store current state of the scrollbar's marker
 */
var		private{private} transient			float					NudgePercent;
var		private{private} transient			float					MarkerPosPercent;
var		private{private} transient			float					MarkerSizePercent;

/**
 * Determines scrollbar's thickness
 */
var()										UIScreenValue			BarWidth;

/**
 * Specifies the minimum size for the scrollbar marker.  Useful to prevent the marker from becoming too small if there are lots of items in the list.
 */
var()										UIScreenValue			MinimumMarkerSize;

/**
 * Determines the length of the Increment/Decrement buttons
 */
var()										UIScreenValue			ButtonsExtent;

/**
 * Controls whether this scrollbar is vertical or horizontal
 */
var()										EUIOrientation			ScrollbarOrientation;

/**
 * Specifies wheather to leave extra space between the bottom/right corner of the scrollbar and its owner widget.
 * Extra space prevents ovelapping if both horizontal and vertical scrollbars exist within one widget
 */
var()										bool					bAddCornerPadding;

/** The current position of the mouse cursor, used in dragging handling */
var		transient							UIScreenValue_Position	MousePosition;

/**
 * The accumulated mouse position delta during dragging. The change in mouse position is accumulated until it is greater than the nudge
 * value, at which point it will then call the OnScrollActivity delegate to move the content.
 */
var		private transient					float					MousePositionDelta;

/** A private flag indicating that the marker needs to be repositioned and resized to the bounds of the uiscrollbar */
var		private	transient					bool					bInitializeMarker;

enum ArrowButtonStateType
{
	ARROWBUTTONSTATE_Clear,
	ARROWBUTTONSTATE_ClickedUp,
	ARROWBUTTONSTATE_HeldUp,
	ARROWBUTTONSTATE_ClickedDown,
	ARROWBUTTONSTATE_HeldDown,
};

/** 증가, 감소버튼 (2가지)를 클릭한 상태인지 붙잡고 있는 상태인지(2가지) 가리킴 2 X 2 + 1 = 4 + 1(CLEAR) 상태*/
var		private transient					ArrowButtonStateType	ArrowButtonState;

/** 증가, 감소 버튼을 클릭하거나, 대기 시간이 지나 스크롤이 반복될때의 시간 */
var		private transient					float					ArrowButtonClickTime;

const	PRIVATE_ArrowClickDelay = 0.33;				/** DXUTScrollbar에서 얻어온 클릭 딜레이 */
const	PRIVATE_ArrowClickRepeat = 0.05;			/** DXUTScrollbar에서 얻어온 반복 딜레이 */

/**
 * Returns the position of this scrollbar's top face (if orientation is vertical) or left face (for horizontal), in pixels.
 */
native final function float GetMarkerButtonPosition() const;

/**
 * Returns the size of the scroll-zone (the region between the decrement and increment buttons), along the same orientation as the scrollbar.
 *
 * @param	ScrollZoneStart	receives the value of the location of the beginning of the scroll zone, in pixels relative to the scrollbar.
 *
 * @return	the height (if the scrollbar's orientation is vertical) or width (if horizontal) of the region between the
 *			increment and decrement buttons, in pixels.
 *
 * @note: noexport so that the native implementation can declare the optional out as a * will default value of NULL.
 */
native final noexport function float GetScrollZoneExtent( optional out float ScrollZoneStart ) const;

/**
 * Returns the size of the scroll-zone (the region between the decrement and increment buttons), for the orientation opposite that of the scrollbar.
 *
 * @return	the height (if the scrollbar's orientation is vertical) or width (if horizontal) of the region between the
 *			increment and decrement buttons, in pixels.
 */
native final function float GetScrollZoneWidth();

/**
 * Sets marker's extent to a percentage of scrollbar size, the direction will be vertical or horizontal
 * depending scrollbar's orientation
 *
 *	@param	SizePercentage	determines the size of the marker, value needs to be in the range [ 0 , 1 ] and
 *                          should be equal to the ratio of viewing area to total widget scroll area
 */
native final function SetMarkerSize( float SizePercentage );

/**
 * Sets marker's top or left face position to start at some percentage of total scrollbar extent
 *
 * @param	PositionPercentage	determines where the marker should start, value needs to be in the range [ 0 , 1 ] and
 *                              should correspond to the position of the topmost or leftmost item in the viewing area
 */
native final function SetMarkerPosition( float PositionPercentage );

/**
 * Sets the amount by which the marker will move to cause one position tick
 *
 * @param	NudgePercentage 	percentage of total scrollbar area which will amount to one tick
 *                              value needs to be in the range [ 0 , 1 ]
 */
native final function SetNudgeSizePercent( float NudgePercentage );

/**
 * Sets the amount by which the marker will move to cause one position tick
 *
 * @param	NudgePixels 	Number of pixels by which the marker will need to be moved to cause one tick
 */
native final function SetNudgeSizePixels( float NudgePixels );

/**
 *	Sets the value of the bAddCornerPadding flag
 */
native final function EnableCornerPadding( bool FlagValue );

/**
 * Increments marker position by the amount of nudges specified in the NudgeMultiplier.  Increment direction is either down or right,
 * depending on scrollbar's orientation.
 *
 * @param	EventObject	Object that issued the event.
 * @param	PlayerIndex	Player that performed the action that issued the event.
 */
native final function ScrollIncrement( UIScreenObject Sender, int PlayerIndex );

/**
 * Decrements marker position by the amount of nudges specified in the NudgeMultiplier.  Decrement direction is either up or left,
 * depending on marker's orientation
 *
 * @param	EventObject	Object that issued the event.
 * @param	PlayerIndex	Player that performed the action that issued the event.
 */
native final function ScrollDecrement( UIScreenObject Sender, int PlayerIndex );

/**
 * 증가, 감소 버튼을 눌렀다 땠을 때 해당 상태를 업데이트해주기 위해 사용
 *
 * @param	EventObject	Object that issued the event.
 * @param	PlayerIndex	Player that performed the action that issued the event.
 */
final function ScrollRelease( UIScreenObject Sender, int PlayerIndex )
{
	ArrowButtonState = ARROWBUTTONSTATE_Clear;
}

/**
 * Initiates mouse drag scrolling. The scroll bar marker will slide on its axis with the mouse cursor
 * until DragScrollEnd is called.
 *
 * @param	EventObject	Object that issued the event.
 * @param	PlayerIndex	Player that performed the action that issued the event.
 */
native final function DragScrollBegin( UIScreenObject Sender, int PlayerIndex );

/**
 * Terminates mouse drag scrolling.
 *
 * @param EventObject	Object that issued the event.
 * @param PlayerIndex	Player that performed the action that issued the event.
 *
 * @return	return TRUE to prevent the kismet OnClick event from firing.
 */
native final function bool DragScrollEnd( UIScreenObject Sender, int PlayerIndex );

/**
 *	Called during the dragging process
 *
 * @param	EventObject	Object that issued the event.
 * @param	PlayerIndex	Player that performed the action that issued the event.
 */
native final function DragScroll( UIScrollbarMarkerButton Sender, int PlayerIndex );

/** === Delegates === */
/**
 * Delegate invoked on scrolling activity
 *
 * @param	PositionChange	the number of nudge values by which the marker was moved
 * @param	bPositionMaxed	indicates that the scrollbar's marker has reached its farthest available position,
 *                          used to obtain pixel exact scrolling
 */
delegate bool OnScrollActivity( float PositionChange, optional bool bPositionMaxed=false );

/**
 * Called when the user clicks anywhere in the scrollbar other than on one of the buttons.
 *
 * @param	Sender			the scrollbar that was clicked.
 * @param	PositionPerc	a value from 0.0 - 1.0, representing the location of the click within the region between the increment
 *							and decrement buttons.  Values closer to 0.0 means that the user clicked near the decrement button; values closer
 *							to 1.0 are nearer the increment button.
 * @param	PlayerIndex		Player that performed the action that issued the event.
 */
delegate OnClickedScrollZone( UIScrollbar Sender, float PositionPerc, int PlayerIndex );

/**
 * Initializes the clicked delegates in the increment, decrement and marker buttons.
 * @todo - this is a fix for the issue where delegates don't seem to be getting set properly in defaultproperties blocks.
 */
event Initialized()
{
	local byte ButtonMask;
	local byte ButtonMask_Disable;

	Super.Initialized();

	IncrementButton.OnPressed = ScrollIncrement;
	IncrementButton.OnPressRepeat = ScrollIncrement;
	IncrementButton.OnPressRelease = ScrollRelease;

	DecrementButton.OnPressed = ScrollDecrement;
	DecrementButton.OnPressRepeat = ScrollDecrement;
	DecrementButton.OnPressRelease = ScrollRelease;

	MarkerButton.OnPressed = DragScrollBegin;
	MarkerButton.OnClicked = DragScrollEnd;

	MarkerButton.OnButtonDragged = DragScroll;

	// Private Behavior
	SetPrivateBehavior( PRIVATE_NotFocusable | PRIVATE_NotRotatable, true );

	ButtonMask = PRIVATE_NotFocusable | PRIVATE_NotDockable | PRIVATE_TreeHidden | PRIVATE_NotEditorSelectable | PRIVATE_ManagedStyle;
	IncrementButton.SetPrivateBehavior(ButtonMask, true);
	DecrementButton.SetPrivateBehavior(ButtonMask, true);
	MarkerButton.SetPrivateBehavior(ButtonMask, true);

	ButtonMask_Disable = PRIVATE_ManagedStyle | PRIVATE_NotEditorSelectable | PRIVATE_TreeHidden;
	IncrementButton.SetPrivateBehavior(ButtonMask_Disable, false);
	DecrementButton.SetPrivateBehavior(ButtonMask_Disable, false);
	MarkerButton.SetPrivateBehavior(ButtonMask_Disable, false);
}

DefaultProperties
{
	NudgeValue=1
	NudgeMultiplier=1.0f
	NudgePercent=0.0f
	MarkerPosPercent=0.0f
	MarkerSizePercent=1.0f

	ScrollbarOrientation=UIORIENT_Vertical

	BarWidth=(Value=16,ScaleType=EVALPOS_PixelViewport,Orientation=UIORIENT_Horizontal)
	ButtonsExtent=(Value=16,ScaleType=EVALPOS_PixelViewport,Orientation=UIORIENT_Vertical)
	MinimumMarkerSize=(Value=12,ScaleType=EVALPOS_PixelViewport,Orientation=UIORIENT_Vertical)

	bAddCornerPadding=true

	// Styles
	PrimaryStyle=(DefaultStyleTag="DefaultScrollZoneStyle",RequiredStyleClass=class'Engine.UIStyle_Image')
	IncrementStyle=(DefaultStyleTag="DefaultIncrementButtonStyle",RequiredStyleClass=class'Engine.UIStyle_Image')
	DecrementStyle=(DefaultStyleTag="DefaultDecrementButtonStyle",RequiredStyleClass=class'Engine.UIStyle_Image')
	MarkerStyle=(DefaultStyleTag="DefaultScrollBarStyle",RequiredStyleClass=class'Engine.UIStyle_Image')
	bSupportsPrimaryStyle=false

	Begin Object Class=UIComp_DrawImage Name=ScrollBarBackgroundImageTemplate
		ImageStyle=(DefaultStyleTag="DefaultScrollZoneStyle",RequiredStyleClass=class'Engine.UIStyle_Image')
		StyleResolverTag="Background Image Style"
	End Object
	BackgroundImageComponent=ScrollBarBackgroundImageTemplate

	// States
	DefaultStates.Add(class'Engine.UIState_Focused')
	DefaultStates.Add(class'Engine.UIState_Pressed')
	DefaultStates.Add(class'Engine.UIState_Active')
}
