/**
 * Base class for all widget types which act as buttons.  Buttons trigger events when
 * they are clicked on or activated using the keyboard.
 * This basic button contains only a background image.
 *
 * Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved.
 */
class UIButton extends UIObject
	native(UIPrivate);

/** Component for rendering the button background image */
var(Image)	editinline	const	noclear	UIComp_DrawImage		BackgroundImageComponent;

/** The background image for this button */
var	deprecated	UITexture ButtonBackground;

/**
 * the texture atlas coordinates for the button background. Values of 0 indicate that
 * the texture is not part of an atlas.
 */
var deprecated	TextureCoordinates	Coordinates;

/** this sound is played when this widget is clicked */
var(Sound)				name						ClickedCue;

cpptext
{
	/* === UIButton interface === */
	/**
	 * Changes the background image for this button, creating the wrapper UITexture if necessary.
	 *
	 * @param	NewImage		the new surface to use for this UIImage
	 */
	virtual void SetImage( class USurface* NewImage );

	/* === UIObject interface === */
	/**
	 * Provides a way for widgets to fill their style subscribers array prior to performing any other initialization tasks.
	 *
	 * This version adds the BackgroundImageComponent (if non-NULL) to the StyleSubscribers array.
	 */
	virtual void InitializeStyleSubscribers();

	/* === UUIScreenObject interface === */
	/**
	 * Generates a array of UI Action keys that this widget supports.
	 *
	 * @param	out_KeyNames	Storage for the list of supported keynames.
	 */
	virtual void GetSupportedUIActionKeyNames(TArray<FName> &out_KeyNames );

	/**
	 * Render this button.
	 *
	 * @param	Canvas	the FCanvas to use for rendering this widget
	 */
	virtual void Render_Widget( FCanvas* Canvas );

protected:
	/**
	 * Handles input events for this button.
	 *
	 * @param	EventParms		the parameters for the input event
	 *
	 * @return	TRUE to consume the key event, FALSE to pass it on.
	 */
	virtual UBOOL ProcessInputKey( const struct FSubscribedInputEventParameters& EventParms );

public:
	/**
	 * Called when a property value from a member struct or array has been changed in the editor, but before the value has actually been modified.
	 */
	virtual void PreEditChange( FEditPropertyChain& PropertyThatChanged );

	/**
	 * Called when a property value from a member struct or array has been changed in the editor.
	 */
	virtual void PostEditChange( FEditPropertyChain& PropertyThatChanged );

	/**
	 * Called after this object has been completely de-serialized.  This version migrates values for the deprecated ButtonBackground,
	 * Coordinates, and PrimaryStyle properties over to the BackgroundImageComponent.
	 */
	virtual void PostLoad();
}

/* === Unrealscript === */
/**
 * Changes the background image for this button, creating the wrapper UITexture if necessary.
 *
 * @param	NewImage		the new surface to use for this UIImage
 */
final function SetImage( Surface NewImage )
{
	if ( BackgroundImageComponent != None )
	{
		BackgroundImageComponent.SetImage(NewImage);
	}
}

// [2006/10/20, YTS] Button Deligates for extensive use
/* == Delegates == */
/**
 * Called when the user clicks on the button.
 *
 * @param	Sender			the button that is submitting the event
 * @param	PlayerIndex		the index of the player that generated the call to this method; used as the PlayerIndex when activating
 *							UIEvents; if not specified, the value of GetBestPlayerIndex() is used instead.
 */
delegate OnMouseClicked( UIButton Sender, int PlayerIndex );

/**
 * Called when the user releases the button.
 *
 * @param	Sender			the button that is submitting the event
 * @param	PlayerIndex		the index of the player that generated the call to this method; used as the PlayerIndex when activating
 *							UIEvents; if not specified, the value of GetBestPlayerIndex() is used instead.
 */
delegate OnMouseReleased( UIButton Sender, int PlayerIndex );


DefaultProperties
{
	PrimaryStyle=(DefaultStyleTag="ButtonBackground",RequiredStyleClass=class'Engine.UIStyle_Image')
	bSupportsPrimaryStyle=false

	// States
	DefaultStates.Add(class'Engine.UIState_Focused')
	DefaultStates.Add(class'Engine.UIState_Active')
	DefaultStates.Add(class'Engine.UIState_Pressed')

	Begin Object class=UIComp_DrawImage Name=BackgroundImageTemplate
		ImageStyle=(DefaultStyleTag="ButtonBackground",RequiredStyleClass=class'Engine.UIStyle_Image')
		StyleResolverTag="Background Image Style"
	End Object
	BackgroundImageComponent=BackgroundImageTemplate

	// Events
	Begin Object Class=UIEvent_OnClick Name=ButtonClickHandler
	End Object

	Begin Object Name=WidgetEventComponent
		DefaultEvents.Add((EventTemplate=ButtonClickHandler,EventState=class'UIState_Focused'))
	End Object

	ClickedCue=Clicked
}
