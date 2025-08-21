/**
 * Contains information about how to present and format text
 *
 * Copyright © 1998-2007 Epic Games, Inc. All Rights Reserved
 */
class UIStyle_Text extends UIStyle_Data
	native(inherit);

/** the font associated with this text style */
var()				Font				StyleFont;

/** attributes to apply to this style's font */
var()				UITextAttributes	Attributes;

/** text alignment within the bounding region */
var()				EUIAlignment		Alignment[EUIOrientation.UIORIENT_MAX];

cpptext
{
	/**
	 * Returns whether the values for this style data match the values from the style specified.
	 *
	 * @param	StyleToCompare	the style to compare this style's values against
	 *
	 * @return	TRUE if StyleToCompare has different values for any style data properties.  FALSE if the specified style is
	 *			NULL, or if the specified style is the same as this one.
	 */
	virtual UBOOL MatchesStyleData( class UUIStyle_Data* StyleToCompare ) const;

	/**
	 * Allows the style to verify that it contains valid data for all required fields.  Called when the owning style is being initialized, after
	 * external references have been resolved.
	 *
	 * This version verifies that this style has a valid font and if not sets the font reference to the default font.
	 */
	virtual void ValidateStyleData();
}

DefaultProperties
{
	UIEditorControlClass="WxStyleTextPropertiesGroup"

	StyleFont=Font'EngineFonts.SmallFont'

	Alignment(UIORIENT_Horizontal)=UIALIGN_Left
	Alignment(UIORIENT_Vertical)=UIALIGN_Center
}
