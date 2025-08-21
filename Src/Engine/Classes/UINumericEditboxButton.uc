/**
 * This  widget is a simple extension of the UIButton class with minor changes made specificly for its application
 * in the UINumericEditBox class.
 *
 * Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved
 *
 */
class UINumericEditBoxButton extends UIButton
	native(inherit)
	notplaceable;

cpptext
{
	/* === UUIObject interface === */
	/**
	 * Render this button.
	 *
	 * @param	Canvas	the canvas to use for rendering this widget
	 */
	virtual void Render_Widget( FCanvas* Canvas );
}

