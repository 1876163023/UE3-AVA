/**
 * Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved.
 */
class Interaction extends UIRoot
	native(UserInterface)
	dependson(IMEComposition)
	transient;

cpptext
{
	/**
	 * Minimal initialization constructor.
	 */
	UInteraction();

	/**
	 * Called when the interaction is added to the GlobalInteractions array.  You must always call Super::Init() so that
	 * unrealscript receives the OnInitialize delegate call.
	 */
	virtual void Init();

	/**
	 * Called once a frame to update the interaction's state.
	 * @param	DeltaTime - The time since the last frame.
	 */
	virtual void Tick(FLOAT DeltaTime)
	{
		eventTick(DeltaTime);
	}

protected:

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
	virtual UBOOL InputKey(INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed=1.f,UBOOL bGamepad=FALSE)
	{
		return FALSE;
	}

	/**
	 * Process an input axis (joystick, thumbstick, or mouse) event received from the viewport.
	 *
	 * @param	ControllerId	the controller that generated this input axis event
	 * @param	Key				the name of the axis that moved  (KEY_MouseX, KEY_XboxTypeS_LeftX, etc.)
	 * @param	Delta			the movement delta for the axis
	 * @param	DeltaTime		the time (in seconds) since the last axis update.
	 *
	 * @return	TRUE to consume the axis event, FALSE to pass it on.
	 */
	virtual UBOOL InputAxis(INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime)
	{
		return FALSE;
	}

	/**
	 * Process a character input event (typing) received from the viewport.
	 *
	 * @param	ControllerId	the controller that generated this character input event
	 * @param	CompStrData		Data of IME Composition String.
	 *
	 * @return	TRUE to consume the key event, FALSE to pass it on.
	 */
	virtual UBOOL InputChar(INT ControllerId, const FInputCompositionStringData& CompStrData )
	{
		return FALSE;
	}

	/**
	 * Process a CandidateWindow from Windows IME(Input Method Editor ) Event 
	 *
	 * @param	ControllerId - The controller which the axis movement is from.
	 *
	 * @return	True to consume the character, false to pass it on.	 
	 */
	virtual UBOOL InputCandidate(INT ControllerId, const FInputCandidateStringData& CandStrData )
	{
		return FALSE;
	}

	/**
	 * Process a CandidateWindow from Windows IME(Input Method Editor ) Event 
	 *
	 * @param	ControllerId - The controller which the axis movement is from.
	 *
	 * @return	True to consume the character, false to pass it on.	 
	 */
	virtual UBOOL InputReadingString(INT ControllerId, const FInputReadingStringData& ReadStrData )
	{
		return FALSE;
	}

public:
}

/**
 * Provides script-only child classes the opportunity to handle input key events received from the viewport.
 * This delegate is ONLY called when input is being routed natively from the GameViewportClient
 * (i.e. NOT when unrealscript calls the InputKey native unrealscript function on this Interaction).
 *
 * @param	ControllerId	the controller that generated this input key event
 * @param	Key				the name of the key which an event occured for (KEY_Up, KEY_Down, etc.)
 * @param	EventType		the type of event which occured (pressed, released, etc.)
 * @param	AmountDepressed	for analog keys, the depression percent.
 *
 * @return	return TRUE to indicate that the input event was handled.  if the return value is TRUE, the input event will not
 *			be processed by this Interaction's native code.
 */
//@todo ronp - input processing optimization
//delegate bool OnReceivedNativeInputKey( int ControllerId, name Key, EInputEvent EventType, optional float AmountDepressed=1.f );

/**
 * Provides script-only child classes the opportunity to handle input axis events received from the viewport.
 * This delegate is ONLY called when input is being routed natively from the GameViewportClient
 * (i.e. NOT when unrealscript calls the InputKey native unrealscript function on this Interaction).
 *
 * @param	ControllerId	the controller that generated this input axis event
 * @param	Key				the name of the axis that moved  (KEY_MouseX, KEY_XboxTypeS_LeftX, etc.)
 * @param	Delta			the movement delta for the axis
 * @param	DeltaTime		the time (in seconds) since the last axis update.
 *
 * @return	return TRUE to indicate that the input event was handled.  if the return value is TRUE, the input event will not
 *			be processed by this Interaction's native code.
 */
//@todo ronp - input processing optimization
//delegate bool OnReceivedNativeInputAxis( int ControllerId, name Key, float Delta, float DeltaTime);

/**
 * Provides script-only child classes the opportunity to handle character input (typing) events received from the viewport.
 * This delegate is ONLY called when input is being routed natively from the GameViewportClient
 * (i.e. NOT when unrealscript calls the InputKey native unrealscript function on this Interaction).
 *
 * @param	ControllerId	the controller that generated this character input event
 * @param	Unicode			the character that was typed
 *
 * @return	return TRUE to indicate that the input event was handled.  if the return value is TRUE, the input event will not
 *			be processed by this Interaction's native code.
 */
//@todo ronp - input processing optimization
//delegate bool OnReceivedNativeInputChar( int ControllerId, string Unicode );


/**
 * Process an input key event routed through unrealscript from another object.  Child classes may override this function to process
 * input key events in unrealscript.  Default behavior is to call this Interaction's C++ InputKey method.
 *
 * @param	ControllerId	the controller that generated this input key event
 * @param	Key				the name of the key which an event occured for (KEY_Up, KEY_Down, etc.)
 * @param	EventType		the type of event which occured (pressed, released, etc.)
 * @param	AmountDepressed	for analog keys, the depression percent.
 *
 * @return	true to consume the key event, false to pass it on.
 */
//@todo ronp - input processing optimization
native noexport event bool InputKey( int ControllerId, name Key, EInputEvent EventType, optional float AmountDepressed=1.f, optional bool bGamepad=FALSE );

/**
 * Process an input axis event routed through unrealscript from another object.  Child classes may override this function to process
 * input axis events in unrealscript.  Default behavior is to call this Interaction's C++ InputAxis method.
 *
 * @param	ControllerId	the controller that generated this input axis event
 * @param	Key				the name of the axis that moved  (KEY_MouseX, KEY_XboxTypeS_LeftX, etc.)
 * @param	Delta			the movement delta for the axis
 * @param	DeltaTime		the time (in seconds) since the last axis update.
 *
 * @return	True to consume the axis movement, false to pass it on.
 */
//@todo ronp - input processing optimization
native noexport event bool InputAxis( int ControllerId, name Key, float Delta, float DeltaTime );

/**
 * Process a character input event (typing) routed through unrealscript from another object.  Child classes may override this function
 * to process character input in unrealscript.  Default behavior is to call this Interaction's C++ InputChar method.
 *
 * @param	ControllerId	the controller that generated this character input event
 * @param	Unicode			the character that was typed
 *
 * @return	True to consume the character, false to pass it on.
 */
//@todo ronp - input processing optimization
native noexport event bool InputChar( int ControllerId, InputCompositionStringData CompStrData );


/**
 * Process a Candidate Window from IME event. 
 * Child classes may override this function to draw Candidate Window of IME
 *
 * @param	ControllerId - The controller which the axis movement is from.
 * @param	CandStrData - Candidate String List, Page Start Index, Page Size, ...
 *
 * @return	True to consume the character, false to pass it on.	 
 */
native noexport event bool InputCandidate( int ControllerId, InputCandidateStringData CandStrData );

/**
 * IME ReadingString Notification
 * ReadingString은 Traditional Chinese, Simplified Chinese에만 쓰임
 * 중국어에서는 중국어를 입력하기에 앞서 발음나는대로 영어로 적기때문에 
 * 현재 적어넣은 영어를 읽는대로(ReadingString) 표현해줘야함
 *
 * @param	Viewport - The viewport which the axis movement is from.
 * @param	ControllerId - The controller which the axis movement is from.
 * @param	ReadStrData	-  적어넣은 영어를 읽는대로 표현한 String정보
 * @return	True to consume the character, false to pass it on.	 
 */
native noexport event bool InputReadingString( int ControllerId, InputReadingStringData ReadStrData );

/**
 * Called once a frame to update the interaction's state.
 * @param	DeltaTime - The time since the last frame.
 */
event Tick(float DeltaTime);

/**
 * Called when the interaction is added to the GlobalInteractions array.
 */
native final noexport function Init();

/**
 * Called from native Init() after native initialization has been performed.
 */
delegate OnInitialize();

/**
 * default handler for OnInitialize delegate.  Here so that child classes can override the default behavior easily
 */
function Initialized();

/**
 * Called when the current map is being unloaded.  Cleans up any references which would prevent garbage collection.
 */
function NotifyGameSessionEnded();

/**
 * Called when a new player has been added to the list of active players (i.e. split-screen join)
 *
 * @param	PlayerIndex		the index [into the GamePlayers array] where the player was inserted
 * @param	AddedPlayer		the player that was added
 */
function NotifyPlayerAdded( int PlayerIndex, LocalPlayer AddedPlayer );

/**
 * Called when a player has been removed from the list of active players (i.e. split-screen players)
 *
 * @param	PlayerIndex		the index [into the GamePlayers array] where the player was located
 * @param	RemovedPlayer	the player that was removed
 */
function NotifyPlayerRemoved( int PlayerIndex, LocalPlayer RemovedPlayer );

defaultproperties
{
	OnInitialize=Initialized
}
