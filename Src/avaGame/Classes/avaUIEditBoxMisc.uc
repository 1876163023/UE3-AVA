class avaUIEditBoxMisc extends UIEditBox
	native;

cpptext
{
	/**
	 * Handles input events for this editbox.
	 *
	 * @param	EventParms		the parameters for the input event
	 *
	 * @return	TRUE to consume the key event, FALSE to pass it on.
	 */
	virtual UBOOL ProcessInputKey( const struct FSubscribedInputEventParameters& EventParms );

	/**
	 * Adds the specified character to the editbox's text field, if the text field is currently eligible to receive
	 * new characters
	 *
	 * Only called if this widget is in the owning scene's InputSubscriptions map for the KEY_Unicode key.
	 *
	 * @param	PlayerIndex		index [into the Engine.GamePlayers array] of the player that generated this event
	 * @param	Character		the character that was received
	 *
	 * @return	TRUE to consume the character, false to pass it on.
	 */
	virtual UBOOL ProcessInputChar( INT PlayerIndex, const FInputCompositionStringData& CompStrData );

	/**
	 * Called when a property value from a member struct or array has been changed in the editor.
	 */
	virtual void PostEditChange( FEditPropertyChain& PropertyThatChanged );
}