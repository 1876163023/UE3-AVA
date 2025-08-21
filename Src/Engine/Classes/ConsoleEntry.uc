/**
 * Temporary widget for representing the region that displays the text currently being typed into the console
 *
 * Copyright 2005 Epic Games, Inc. All Rights Reserved
 */
class ConsoleEntry extends UIObject
	native(UIPrivate);

/** displays the text that is currently being typed */
var	transient		UILabel			InputText;

var	transient		UIEditBox		InputBox;

/** the current position of the cursor in InputText's string */
var	transient 		int				CursorPosition;

/** controls whether the underline cursor is rendered */
var()				bool			bRenderCursor;

const ConsolePromptText = "(> ";

cpptext
{
	/**
	 * Render this widget.
	 *
	 * @param	Canvas	the FCanvas to use for rendering this widget
	 */
	virtual void Render_Widget( FCanvas* Canvas ) {}	// do nothing

	/**
	 * Perform any additional rendering after this widget's children have been rendered.
	 *
	 * @param	Canvas	the FCanvas to use for rendering this widget
	 */
	virtual void PostRender_Widget( FCanvas* Canvas );
}

event AddedChild( UIScreenObject WidgetOwner, UIObject NewChild )
{
	if (InputText == None && UILabel(NewChild) != None )
	{
		InputText = UILabel(NewChild);
		InputText.IgnoreMarkup(true);
		SetValue("");
	}
	else if ( InputBox == None && UIEditBox(NewChild) != None )
	{
		InputBox = UIEditBox(NewChild);
		InputBox.IgnoreMarkup(true);
		SetValue("");
	}
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
	if ( InputText == OldChild )
	{
		InputText = None;
	}
	else if ( InputBox == OldChild )
	{
		InputBox = None;
	}
}

event PostInitialize()
{
	Super.PostInitialize();

	InputText = UILabel(FindChild('InputText'));
	if ( InputText != None )
	{
		InputText.IgnoreMarkup(true);
		SetValue("");
	}
	else
	{
		InputBox = UIEditBox(FindChild('InputBox'));
		if ( InputBox != None )
		{
			InputBox.IgnoreMarkup(true);
			SetValue("");
		}
	}
}

function SetValue( string NewValue )
{
	if ( InputText != None )
	{
		InputText.SetValue(ConsolePromptText $ NewValue);
	}
	else if ( InputBox != None )
	{
		InputBox.SetValue(NewValue);
	}
}

DefaultProperties
{
	WidgetTag=ConsoleEntry
	PrimaryStyle=(DefaultStyleTag="ConsoleStyle")

	DefaultStates.Add(class'UIState_Focused')
}
