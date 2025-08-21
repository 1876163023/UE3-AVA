class avaUICheckLabelButton extends UILabelButton
	native;

/** Component for rendering the button background image */
var(Image)	editinline	const	noclear	UIComp_DrawImage			CheckedImageComponent;
var(Image)	editinline	const	noclear	UIComp_DrawString			CheckedStringComponent;
var(Image)	bool													bDrawExclusive;


/**
 * Controls whether this button is considered checked.  When bIsChecked is TRUE, CheckedImage will be rendered over
 * the button background image, using the current style.
 */
var(Value)	private							bool					bIsChecked;
var(Value)									bool					bAutoToggle;


cpptext
{
	/* === UUIScreenObject interface === */
	/**
	 * Render this checkbox.
	 *
	 * @param	Canvas	the FCanvas to use for rendering this widget
	 */
	virtual void Render_Widget( FCanvas* Canvas );

	/**
	 * Changes the checked image for this checkbox, creating the wrapper UITexture if necessary.
	 *
	 * @param	NewImage		the new surface to use for this UIImage
	 */
	virtual void SetCheckImage( class USurface* NewImage );

	/**
	 * Provides a way for widgets to fill their style subscribers array prior to performing any other initialization tasks.
	 *
	 * This version adds the CheckedImageComponent (if non-NULL) to the StyleSubscribers array.
	 */
	virtual void InitializeStyleSubscribers();

	/**
	 * Perform all initialization for this widget. Called on all widgets when a scene is opened,
	 * once the scene has been completely initialized.
	 * For widgets added at runtime, called after the widget has been inserted into its parent's
	 * list of children.
	 *
	 * @param	inOwnerScene	the scene to add this widget to.
	 * @param	inOwner			the container widget that will contain this widget.  Will be NULL if the widget
	 *							is being added to the scene's list of children.
	 */
	virtual void Initialize( UUIScene* inOwnerScene, UUIObject* inOwner=NULL );

	/**
	 * Adds the specified face to the DockingStack for the specified widget
	 *
	 * @param	DockingStack	the docking stack to add this docking node to.  Generally the scene's DockingStack.
	 * @param	Face			the face that should be added
	 *
	 * @return	TRUE if a docking node was added to the scene's DockingStack for the specified face, or if a docking node already
	 *			existed in the stack for the specified face of this widget.
	 */
	virtual UBOOL AddDockingNode( TArray<struct FUIDockingNode>& DockingStack, EUIWidgetFace Face );

	/**
	 * Evalutes the Position value for the specified face into an actual pixel value.  Should only be
	 * called from UIScene::ResolvePositions.  Any special-case positioning should be done in this function.
	 *
	 * @param	Face	the face that should be resolved
	 */
	virtual void ResolveFacePosition( EUIWidgetFace Face );

	/**
	 * Changes this widget's position to the specified value.  This version changes the default value for the bClampValues parameter to TRUE
	 *
	 * @param	LeftFace		the value (in pixels or percentage) to set the left face to
	 * @param	TopFace			the value (in pixels or percentage) to set the top face to
	 * @param	RightFace		the value (in pixels or percentage) to set the right face to
	 * @param	BottomFace		the value (in pixels or percentage) to set the bottom face to
	 * @param	InputType		indicates the format of the input value.  All values will be evaluated as this type.
	 *								EVALPOS_None:
	 *									NewValue will be considered to be in whichever format is configured as the ScaleType for the specified face
	 *								EVALPOS_PercentageOwner:
	 *								EVALPOS_PercentageScene:
	 *								EVALPOS_PercentageViewport:
	 *									Indicates that NewValue is a value between 0.0 and 1.0, which represents the percentage of the corresponding
	 *									base's actual size.
	 *								EVALPOS_PixelOwner
	 *								EVALPOS_PixelScene
	 *								EVALPOS_PixelViewport
	 *									Indicates that NewValue is an actual pixel value, relative to the corresponding base.
	 * @param	bZeroOrigin		FALSE indicates that the value specified includes the origin offset of the viewport.
	 * @param	bClampValues	if TRUE, clamps the values of RightFace and BottomFace so that they cannot be less than the values for LeftFace and TopFace
	 */
	virtual void SetPosition( const FLOAT LeftFace, const FLOAT TopFace, const FLOAT RightFace, const FLOAT BottomFace, EPositionEvalType InputType=EVALPOS_PixelViewport, UBOOL bZeroOrigin=FALSE, UBOOL bClampValues=TRUE )
	{
		Super::SetPosition(LeftFace, TopFace, RightFace, BottomFace, InputType, bZeroOrigin, bClampValues);
	}

	/**
	 * Called when a property is modified that could potentially affect the widget's position onscreen.
	 */
	virtual void RefreshPosition();

	/**
	 * Called to globally update the formatting of all UIStrings.
	 */
	virtual void RefreshFormatting();


protected:

	/**
	 * Handles input events for this checkbox.
	 *
	 * @param	EventParms		the parameters for the input event
	 *
	 * @return	TRUE to consume the key event, FALSE to pass it on.
	 */
	virtual UBOOL ProcessInputKey( const struct FSubscribedInputEventParameters& EventParms );

public:
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
	 * Called after this object has been completely de-serialized.  This version migrates values for the deprecated CheckedImage,
	 * CheckCoordinates, and CheckedStyle properties over to the CheckedImageComponent.
	 */
	virtual void PostLoad();
}

/* === Unrealscript === */
/**
 * Changes the checked image for this checkbox, creating the wrapper UITexture if necessary.
 *
 * @param	NewImage		the new surface to use for this UIImage
 */
final function SetCheckImage( Surface NewImage )
{
	if ( CheckedImageComponent != None )
	{
		CheckedImageComponent.SetImage(NewImage);
	}
}

final function SetCheckString( string NewString )
{
	if ( CheckedStringComponent != None )
	{
		CheckedStringComponent.SetValue(NewString);
	}
}

/**
 * Returns TRUE if this button is in the checked state, FALSE if in the
 */
final function bool IsChecked()
{
	return bIsChecked;
}

/* === Natives === */

native function SetCaption( string NewText );

native function SetValue( bool bShouldBeChecked, optional int PlayerIndex=INDEX_NONE, optional bool bSkipNotification = FALSE );

/* === Kismet action handlers === */
final function OnSetBoolValue( UIAction_SetBoolValue Action )
{
	SetValue(Action.bNewValue);
}

/** === Kismet Action Handlers === */
function OnSetLabelText( UIAction_SetLabelText Action )
{
	SetCaption(Action.NewText);
}

final function OnGetBoolValue( UIAction_GetBoolValue Action )
{
	Action.bNewValue = bIsChecked;
}

/**
 * Resolves this subscriber's data store binding and updates the subscriber with the current value from the data store.
 *
 * @return	TRUE if this subscriber successfully resolved and applied the updated value.
 */
native function bool RefreshSubscriberValue( optional int BindingIndex=INDEX_NONE );

/**
 * Sets the text alignment for the string that the widget is rendering.
 *
 * @param	Horizontal		Horizontal alignment to use for text, UIALIGN_MAX means no change.
 * @param	Vertical		Vertical alignment to use for text, UIALIGN_MAX means no change.
 */
native function SetTextAlignment(EUIAlignment Horizontal, EUIAlignment Vertical);


defaultproperties
{
	Begin Object class=UIComp_DrawImage Name=CheckedImageTemplate
		ImageStyle=(DefaultStyleTag="DefaultCheckbox",RequiredStyleClass=class'Engine.UIStyle_Image')
		StyleResolverTag="Check Image Style"
	End Object
	CheckedImageComponent=CheckedImageTemplate

	Begin Object class=UIComp_DrawString Name=CheckedStringTemplate
		StringStyle=(DefaultStyleTag="DefaultLabelButtonStyle",RequiredStyleClass=class'Engine.UIStyle_Combo')
		StyleResolverTag="Check String Style"
	End Object
	CheckedStringComponent=CheckedStringTemplate

	bAutoToggle=false;
	bDrawExclusive=false;
}