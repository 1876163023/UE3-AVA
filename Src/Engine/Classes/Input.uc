//=============================================================================
// Input
// Object that maps key events to key bindings
// Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved.
//=============================================================================

class Input extends Interaction
	native(UserInterface)
	config(Input)
    transient;

struct native KeyBind
{
	var config name		Group;
	var config string	UserKeyAlias;
	var config name		Name;
	var config string	Command;
	var config bool		Control,
						Shift,
						Alt;
	var config byte		Slot;
structcpptext
{
	FKeyBind()
	: UserKeyAlias()
	, Name()
	, Control(0)
	, Shift(0)
	, Alt(0)
	, Slot(0)
	{}
}
};

var config array<KeyBind>				Bindings;
var config array<KeyBind>				DefaultBindings<ToolTip= FactoryDefault >;

/** list of keys which this interaction handled a pressed event for */
var const array<name>					PressedKeys;

var const EInputEvent					CurrentEvent;
var const float							CurrentDelta;
var const float							CurrentDeltaTime;

var native const Map{FName,void*}		NameToPtr;
var native const init array<pointer>	AxisArray{FLOAT};

cpptext
{
	// UInteraction interface.

	/**
	 * Process an input key event received from the viewport.
	 *
	 * @param	ControllerId	gamepad/controller that generated this input event
	 * @param	Key				the name of the key which an event occured for (KEY_Up, KEY_Down, etc.)
	 * @param	Event			the type of event which occured (pressed, released, etc.)
	 * @param	AmountDepressed	(analog keys only) the depression percent.
	 * @param	bGamepad - input came from gamepad (ie xbox controller)
	 *
	 * @return	TRUE to consume the key event, FALSE to pass it on.
	 */
	virtual UBOOL InputKey(INT ControllerId, FName Key, enum EInputEvent Event, FLOAT AmountDepressed = 1.f, UBOOL bGamepad = FALSE );
	virtual UBOOL InputAxis(INT ControllerId, FName Key, FLOAT Delta, FLOAT DeltaTime);
	virtual void Tick(FLOAT DeltaTime);
	UBOOL IsPressed( FName InKey ) const;
	UBOOL IsCtrlPressed() const;
	UBOOL IsShiftPressed() const;
	UBOOL IsAltPressed() const;

	// UInput interface.
	virtual UBOOL Exec(const TCHAR* Cmd,FOutputDevice& Ar);
	virtual void ResetInput();

	/**
	 * Clears the PressedKeys array.  Should be called when another interaction which swallows some (but perhaps not all) input is activated.
	 */
	virtual void FlushPressedKeys()
	{
		PressedKeys.Empty();
	}

	// Protected.

	BYTE* FindButtonName(const TCHAR* ButtonName);
	FLOAT* FindAxisName(const TCHAR* ButtonName);
	FString GetBind(FName Key) const;
	void ExecInputCommands(const TCHAR* Cmd,class FOutputDevice& Ar);
}

exec function SetBind(name BindName,string Command)
{
	local KeyBind	NewBind;
	local int		BindIndex;

	for(BindIndex = 0;BindIndex < Bindings.Length;BindIndex++)
		if(Bindings[BindIndex].Name == BindName)
		{
			Bindings[BindIndex].Command = Command;
			SaveConfig();
			return;
		}

	NewBind.Name = BindName;
	NewBind.Command = Command;
	Bindings[Bindings.Length] = NewBind;
	SaveConfig();
}