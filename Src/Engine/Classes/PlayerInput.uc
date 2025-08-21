//=============================================================================
// PlayerInput
// Object within playercontroller that manages player input.
// only spawned on client
// Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
//=============================================================================

class PlayerInput extends Input within PlayerController
	config(Input)
	transient
	native(UserInterface);

cpptext
{
	/**
	 * Generates an IE_Released event for each key in the PressedKeys array, then clears the array.  Should be called when another
	 * interaction which swallows some (but perhaps not all) input is activated.
	 */
	virtual void FlushPressedKeys();
}

var globalconfig	bool		bInvertMouse;							/** if true, mouse y axis is inverted from normal FPS mode */

// Double click move flags
var					bool		bWasForward;
var					bool		bWasBack;
var					bool		bWasLeft;
var					bool		bWasRight;
var					bool		bEdgeForward;
var					bool		bEdgeBack;
var					bool		bEdgeLeft;
var					bool 		bEdgeRight;

var					float		DoubleClickTimer;						/** max double click interval for double click move */
var globalconfig	float		DoubleClickTime;						/** stores time of first click for potential double click */

var globalconfig	float		MouseSensitivity;

// Input axes.
var input			float		aBaseX;
var input			float		aBaseY;
var input			float		aBaseZ;
var input			float		aMouseX;
var input			float		aMouseY;
var input			float		aForward;
var input			float		aTurn;
var input			float		aStrafe;
var input			float		aUp;
var input			float		aLookUp;

//
// Joy Raw Input
//
/** Joypad left thumbstick, vertical axis. Range [-1,+1] */
var		transient	float	RawJoyUp;
/** Joypad left thumbstick, horizontal axis. Range [-1,+1] */
var		transient	float	RawJoyRight;
/** Joypad right thumbstick, horizontal axis. Range [-1,+1] */
var		transient	float	RawJoyLookRight;
/** Joypad right thumbstick, vertical axis. Range [-1,+1] */
var		transient	float	RawJoyLookUp;

/** move forward speed scaling */
var()	config		float	MoveForwardSpeed;
/** strafe speed scaling */
var()	config		float	MoveStrafeSpeed;
/** Yaw turn speed scaling */
var()	config		float	LookRightScale;
/** pitch turn speed scaling */
var()	config		float	LookUpScale;


// Input buttons.
var input			byte		bStrafe;
var input			byte		bXAxis;
var input			byte		bYAxis;

// Mouse smoothing control
var globalconfig bool		bEnableMouseSmoothing;			/** if true, mouse smoothing is enabled */

// Zoom Scaling
var bool bEnableFOVScaling;

// Mouse smoothing sample data
var float ZeroTime[2];							/** How long received mouse movement has been zero. */
var float SmoothedMouse[2];						/** Current average mouse movement/sample */
var int MouseSamples;							/** Number of mouse samples since mouse movement has been zero */
var float  MouseSamplingTotal;					/** DirectInput's mouse sampling total time */

var array<byte>		CustomCharTableEnc;

//=============================================================================
// Input related functions.

exec function bool InvertMouse()
{
	bInvertMouse = !bInvertMouse;
	SaveConfig();
	return bInvertMouse;
}

/** Hook called from HUD actor. Gives access to HUD and Canvas */
function DrawHUD( HUD H );

function PreProcessInput(float DeltaTime);
function PostProcessInput(float DeltaTime);

// Postprocess the player's input.
event PlayerInput( float DeltaTime )
{
	local float FOVScale, TimeScale;

	// Save Raw values
	RawJoyUp		= aBaseY;
	RawJoyRight		= aStrafe;
	RawJoyLookRight	= aTurn;
	RawJoyLookUp	= aLookUp;

	// PlayerInput shouldn't take timedilation into account
	DeltaTime /= WorldInfo.TimeDilation;

	PreProcessInput( DeltaTime );

	// Scale to game speed
	TimeScale = 100.f*DeltaTime;
	aBaseY		*= TimeScale * MoveForwardSpeed;
	aStrafe		*= TimeScale * MoveStrafeSpeed;
	aUp			*= TimeScale * MoveStrafeSpeed;
	aTurn		*= TimeScale * LookRightScale;
	aLookUp		*= TimeScale * LookUpScale;

	PostProcessInput( DeltaTime );

	ProcessInputMatching(DeltaTime);

	// Check for Double click movement.
	bEdgeForward	= (bWasForward	^^ (aBaseY	> 0));
	bEdgeBack		= (bWasBack		^^ (aBaseY	< 0));
	bEdgeLeft		= (bWasLeft		^^ (aStrafe < 0));
	bEdgeRight		= (bWasRight	^^ (aStrafe > 0));
	bWasForward		= (aBaseY	> 0);
	bWasBack		= (aBaseY	< 0);
	bWasLeft		= (aStrafe	< 0);
	bWasRight		= (aStrafe	> 0);

	// Take FOV into account (lower FOV == less sensitivity).

	if ( bEnableFOVScaling )
	{
		FOVScale = GetFOVAngle() * 0.01111; // 0.01111 = 1 / 90.0
	}
	else
	{
		FOVScale = 1.0;
	}

	// Apply mouse sensitivity.
	aMouseX			*= MouseSensitivity * FOVScale;
	aMouseY			*= MouseSensitivity * FOVScale;

	// mouse smoothing
	if ( bEnableMouseSmoothing )
	{
		aMouseX = SmoothMouse(aMouseX, DeltaTime,bXAxis,0);
		aMouseY = SmoothMouse(aMouseY, DeltaTime,bYAxis,1);
	}

	aLookUp			*= FOVScale;
	aTurn			*= FOVScale;

	// Turning and strafing share the same axis.
	if( bStrafe > 0 )
		aStrafe		+= aBaseX + aMouseX;
	else
		aTurn		+= aBaseX + aMouseX;

	// Look up/down.
	aLookup += aMouseY;
	if (bInvertMouse)
	{
		aLookup *= -1.f;
	}

	// Forward/ backward movement
	aForward		+= aBaseY;

	// Handle walking.
	HandleWalking();

	// ignore move input
	// Do not clear RawJoy flags, as we still want to be able to read input.
	if( IsMoveInputIgnored() )
	{
		aForward	= 0.f;
		aStrafe		= 0.f;
		aUp			= 0.f;
	}

	// ignore look input
	// Do not clear RawJoy flags, as we still want to be able to read input.
	if( IsLookInputIgnored() )
	{
		aTurn		= 0.f;
		aLookup		= 0.f;
	}
}

// check for double click move
function Actor.EDoubleClickDir CheckForDoubleClickMove(float DeltaTime)
{
	local Actor.EDoubleClickDir DoubleClickMove, OldDoubleClick;

	if ( DoubleClickDir == DCLICK_Active )
		DoubleClickMove = DCLICK_Active;
	else
		DoubleClickMove = DCLICK_None;
	if (DoubleClickTime > 0.0)
	{
		if ( DoubleClickDir == DCLICK_Active )
		{
			if ( (Pawn != None) && (Pawn.Physics == PHYS_Walking) )
			{
				DoubleClickTimer = 0;
				DoubleClickDir = DCLICK_Done;
			}
		}
		else if ( DoubleClickDir != DCLICK_Done )
		{
			OldDoubleClick = DoubleClickDir;
			DoubleClickDir = DCLICK_None;

			if (bEdgeForward && bWasForward)
				DoubleClickDir = DCLICK_Forward;
			else if (bEdgeBack && bWasBack)
				DoubleClickDir = DCLICK_Back;
			else if (bEdgeLeft && bWasLeft)
				DoubleClickDir = DCLICK_Left;
			else if (bEdgeRight && bWasRight)
				DoubleClickDir = DCLICK_Right;

			if ( DoubleClickDir == DCLICK_None)
				DoubleClickDir = OldDoubleClick;
			else if ( DoubleClickDir != OldDoubleClick )
				DoubleClickTimer = DoubleClickTime + 0.5 * DeltaTime;
			else
				DoubleClickMove = DoubleClickDir;
		}

		if (DoubleClickDir == DCLICK_Done)
		{
			DoubleClickTimer = FMin(DoubleClickTimer-DeltaTime,0);
			if (DoubleClickTimer < -0.35)
			{
				DoubleClickDir = DCLICK_None;
				DoubleClickTimer = DoubleClickTime;
			}
		}
		else if ((DoubleClickDir != DCLICK_None) && (DoubleClickDir != DCLICK_Active))
		{
			DoubleClickTimer -= DeltaTime;
			if (DoubleClickTimer < 0)
			{
				DoubleClickDir = DCLICK_None;
				DoubleClickTimer = DoubleClickTime;
			}
		}
	}
	return DoubleClickMove;
}

/**
 * Iterates through all InputRequests on the PlayerController and
 * checks to see if a new input has been matched, or if the entire
 * match sequence should be reset.
 *
 * @param	DeltaTime - time since last tick
 */
final function ProcessInputMatching(float DeltaTime)
{
	local float Value;
	local int i,MatchIdx;
	local bool bMatch;
	// iterate through each request,
	for (i = 0; i < InputRequests.Length; i++)
	{
		// if we have a valid match idx
		if (InputRequests[i].MatchActor != None &&
			InputRequests[i].MatchIdx >= 0 &&
			InputRequests[i].MatchIdx < InputRequests[i].Inputs.Length)
		{
			MatchIdx = InputRequests[i].MatchIdx;
			// if we've exceeded the delta,
			// ignore the delta for the first match
			if (MatchIdx != 0 &&
				WorldInfo.TimeSeconds - InputRequests[i].LastMatchTime >= InputRequests[i].Inputs[MatchIdx].TimeDelta)
			{
				// reset this match
				InputRequests[i].LastMatchTime = 0.f;
				InputRequests[i].MatchIdx = 0;

				// fire off the cancel event
				if (InputRequests[i].FailedFuncName != 'None')
				{
					InputRequests[i].MatchActor.SetTimer(0.01f, false, InputRequests[i].FailedFuncName );
				}
			}
			else
			{
				// grab the current input value of the matching type
				Value = 0.f;
				switch (InputRequests[i].Inputs[MatchIdx].Type)
				{
				case IT_XAxis:
					Value = aStrafe;
					break;
				case IT_YAxis:
					Value = aBaseY;
					break;
				}
				// check to see if this matches
				switch (InputRequests[i].Inputs[MatchIdx].Action)
				{
				case IMA_GreaterThan:
					bMatch = Value >= InputRequests[i].Inputs[MatchIdx].Value;
					break;
				case IMA_LessThan:
					bMatch = Value <= InputRequests[i].Inputs[MatchIdx].Value;
					break;
				}
				if (bMatch)
				{
					// mark it as matched
					InputRequests[i].LastMatchTime = WorldInfo.TimeSeconds;
					InputRequests[i].MatchIdx++;
					// check to see if we've matched all inputs
					if (InputRequests[i].MatchIdx >= InputRequests[i].Inputs.Length)
					{
						// fire off the event
						if (InputRequests[i].MatchFuncName != 'None')
						{
							InputRequests[i].MatchActor.SetTimer(0.01f,false,InputRequests[i].MatchFuncName);
						}
						// reset this match
						InputRequests[i].LastMatchTime = 0.f;
						InputRequests[i].MatchIdx = 0;
						// as well as all others
					}
				}
			}
		}
	}
}

//*************************************************************************************
// Normal gameplay execs
// Type the name of the exec function at the console to execute it

exec function Jump()
{
	if ( WorldInfo.Pauser == PlayerReplicationInfo )
		SetPause( False );
	else
		bPressedJump = true;
}

exec function SmartJump()
{
	Jump();
}

//*************************************************************************************
// Mouse smoothing

event bool InputAxis(int ControllerId,name Key,float Delta,float DeltaTime)
{
	//@todo steve:  there must be some way of directly getting the mouse sampling rate from directinput
	if ( Key == 'mousex' )
	{
		// calculate sampling time
		// make sure not first non-zero sample
		if ( SmoothedMouse[0] > 0 )
		{
			// not first non-zero
			MouseSamplingTotal += DeltaTime;
			MouseSamples++;
		}
	}
	return super.InputAxis(ControllerId, Key, Delta, DeltaTime);
}

exec function ClearSmoothing()
{
	local int i;

	for ( i=0; i<2; i++ )
	{
		//`Log(i$" zerotime "$zerotime[i]$" smoothedmouse "$SmoothedMouse[i]);
		ZeroTime[i] = 0;
		SmoothedMouse[i] = 0;
	}
	//`Log("MouseSamplingTotal "$MouseSamplingTotal$" MouseSamples "$MouseSamples);
    	MouseSamplingTotal = Default.MouseSamplingTotal;
	MouseSamples = Default.MouseSamples;
}

/** SmoothMouse()
Smooth mouse movement, because mouse sampling doesn't match up with tick time.
 * @note: if we got sample event for zero mouse samples (so we
			didn't have to guess whether a 0 was caused by no sample occuring during the tick (at high frame rates) or because the mouse actually stopped)
 * @param: aMouse is the mouse axis movement received from DirectInput
 * @param: DeltaTime is the tick time
 * @param: SampleCount is the number of mouse samples received from DirectInput
 * @param: Index is 0 for X axis, 1 for Y axis
 * @return the smoothed mouse axis movement
 */
function float SmoothMouse(float aMouse, float DeltaTime, out byte SampleCount, int Index)
{
	local float MouseSamplingTime;

	if (DeltaTime < 0.25)
	{
		MouseSamplingTime = MouseSamplingTotal/MouseSamples;

		if ( aMouse == 0 )
		{
			// no mouse movement received
			ZeroTime[Index] += DeltaTime;
			if ( ZeroTime[Index] < MouseSamplingTime )
			{
				// zero mouse movement is possibly because less than the mouse sampling interval has passed
				aMouse = SmoothedMouse[Index] * DeltaTime/MouseSamplingTime;
			}
			else
			{
				SmoothedMouse[Index] = 0;
			}
		}
		else
		{
			ZeroTime[Index] = 0;
			if ( SmoothedMouse[Index] != 0 )
			{
				// this isn't the first tick with non-zero mouse movement
				if ( DeltaTime < MouseSamplingTime * (SampleCount + 1) )
				{
					// smooth mouse movement so samples/tick is constant
					aMouse = aMouse * DeltaTime/(MouseSamplingTime * SampleCount);
				}
				else
				{
					// fewer samples, so going slow
					// use number of samples we should have had for sample count
					SampleCount = DeltaTime/MouseSamplingTime;
				}
			}
			SmoothedMouse[Index] = aMouse/SampleCount;
		}
	}
	else
	{
		// if we had an abnormally long frame, clear everything so it doesn't distort the results
		ClearSmoothing();
	}
	SampleCount = 0;
	return aMouse;
}

function bool IsSpecificCommand( KeyBind inKeyBind, bool OnRelease = true, bool Axis = true, bool Button = true, bool Toggle = true, bool Count = true )
{
	Local bool bResult;
	Local string CapsCmd;

	bResult = false;
	CapsCmd = Caps(inKeyBind.Command);
	if( Axis && InStr( CapsCmd, "AXIS") != INDEX_NONE )
		bResult = true;
	else if( OnRelease && InStr( CapsCmd,"ONRELEASE") != INDEX_NONE )
		bResult = true;
	else if( Button && InStr( CapsCmd,"BUTTON") != INDEX_NONE )
		bResult = true;
	else if( Toggle && InStr( CapsCmd,"TOGGLE") != INDEX_NONE )
		bResult = true;
	else if( Count && InStr(CapsCmd,"COUNT") != INDEX_NONE )
		bResult = true;

	return bResult;
}

function bool GetKeyBindByIndex( int BindingIndex, out KeyBind outKeyBind, optional bool bRecursive = false)
{
	Local int FindIndex;
	if( 0 <= BindingIndex && BindingIndex < Bindings.Length )
	{
		outKeyBind = Bindings[BindingIndex];
		if( bRecursive )
		{
			FindIndex = Bindings.Find('Name',name(outKeyBind.Command));

			while( FindIndex != INDEX_NONE )
			{
				outKeyBind = Bindings[FindIndex];

				FindIndex = Bindings.Find('Name',name(outKeyBind.Command));
			}
		}
		return true;
	}
	return false;
}

function bool HasBindingName( coerce name BindingName , optional out KeyBind KeyBinding)
{
	Local int FindIndex;

	FindIndex = Bindings.find( 'Name', BindingName);
	if( FindIndex >= 0 )
		 KeyBinding = Bindings[FindIndex];

	return FindIndex >= 0;
}

function SetUserKey( int BindingIndex , name NewBindName , bool bSaveConfig = false)
{
	//`Log("SetUserKey("@UserKeyAlias$","@NewBindName$","@bSaveConfig);
	if( 0 <= BindingIndex && BindingIndex < Bindings.Length )
	{
		Bindings[BindingIndex].Name = NewBindName;
	}

	if( bSaveConfig )
	{
		SaveConfig();
	}
}

function SwapUserKey( int BindingIndex, name NewBindName, out int SwappedKeySlot, bool bSaveConfig = false )
{
	Local int FindIndex;
	Local name SwappedName;

	FindIndex = Bindings.find( 'Name', NewBindName );
	if( FindIndex >= 0 && 0 <= BindingIndex && BindingIndex < Bindings.Length )
	{
		SwappedName = Bindings[BindingIndex].Name;
		SwappedKeySlot = Bindings[FindIndex].Slot;
		Bindings[BindingIndex].Name = NewBindName;
		Bindings[FindIndex].Name = SwappedName;
	}
}

function UnSetUserKey( bool bFactoryDefault, optional bool bNotifyChangeToServer )
{
	//Local string UserKeyString;
	//Local int ch, i;

//	`Log("UnSetUserKey In "$Self);
	if( bFactoryDefault )
	{
		Bindings = DefaultBindings;
	}
	else
	{
		Bindings = Self.class.default.Bindings;
	}

	if( bNotifyChangeToServer )
	{
		SaveConfig();
	}
}

//
//
//function string GetConvStr( byte InChar )
//{
//	Local byte LowByte, HiByte;
//
//	LowByte = InChar % 16;
//	LowByte += LowByte >= 10 ? (0x41 - 10): 0x30;
//	HiByte = InChar / 16;
//	HiByte += HiByte >= 10 ? (0x41 - 10) : 0x30;
//	`Log("In "$InChar$", Low = "$LowByte$", Hi = "$HiByte);
//
//	return Chr( (HiByte << 8) | (LowByte) );
//}
//
//function string GetNormalStr( int AscValue )
//{
//	Local int HiByte;
//	Local int LowByte;
//	
//	HiByte = ((AscValue & 0xff00) >> 8);
//	HiByte = HiByte >= 0x41 ? HiByte - (0x41 + 10) : HiByte - 0x30;
//	HiByte = HiByte * 16;
//
//	LowByte = (AscValue & 0xff);
//	LowByte = LowByte >= 0x41 ? LowByte - ( 0x41 + 10 ) : LowByte - 0x30;
//
//	return Chr( HiByte + LowByte );
//}
//


function string GetUserKeyString()
{
	Local int i;
	Local byte Count;
	Local string UserKeyString;
	Local array<byte> UserKeyBytes;

	UserKeyBytes = GetUserKeyBytes();
	for( i = 1 ; i< UserKeyBytes.Length ; i++ )
	{
		if( UserKeyBytes[i] == 0 )
			UserKeyString $= Chr(0x80);
		else
		{
//			`Log("UserKeyBytes["$i$"] = "$UserKeyBytes[i]$" > "$CustomCharTableEnc[UserKeyBytes[i]]);
			UserKeyString $= Chr(CustomCharTableEnc[UserKeyBytes[i]]);
			Count++;
		}
	}

	assert( Count == UserKeyBytes[0] );
	return ( Count != UserKeyBytes[0] ) ? "" : Chr(Count)$UserKeyString;
}


function SetUserKeyString( string UserKeyString )
{
	Local int i, ch, UserKeyLen, FindIndex, Count, VirtualKeyCode;
	Local name NameToBind;
	Local KeyBind KeyBindFound;

	if( Len(UserKeyString) == 0 || Asc(Left(UserKeyString, 1)) == 0 )
		return;

	UserKeyLen = Len(UserKeyString);
	for( i = 1 ; i < UserKeyLen ; i++ )
	{
		ch = Asc(Mid(UserKeyString,i));
		FindIndex = Bindings.find('Slot', i);
		VirtualKeyCode = CustomCharTableEnc.Find(ch);
		if( FindIndex >= 0 && (0 <= ch && ch < 0x80) && VirtualKeyCode != INDEX_NONE)
		{
//			`Log("Pre Name = "$Bindings[FindIndex].Name$", Post = "$GetKeyNameByCode(VirtualKeyCode));
			NameToBind = GetKeyNameByCode(VirtualKeyCode);
//			`Log("NameToBind = "$NameToBind);
			if( HasBindingName( NameToBind, KeyBindFound ) && KeyBindFound.Slot == 0)
			{
				//`Log("NameToBind = "$NameToBind$", Slot = "$KeyBind.Slot);
				UnSetUserKey(true,true);
				return;
			}
			Bindings[FindIndex].Name = NameToBind;
			Count++;
		}
	}

	ch = Asc(Left(UserKeyString,1));
	if( Count != ch )
		`warn("UserKey String mismatch : in "$ch$", changed "$Count);
	SaveConfig();
}

function array<byte> GetUserKeyBytes()
{
	Local int i, Code;
	Local byte Count;
	Local array<byte> KeyBytes;

	for( i = Bindings.Length - 1 ; i >= 0 ; i-- )
	{
		// Code 반환값이 0이면 해당하는 KeyCode를 찾지 못한것이다.
		Code = GetKeyCodeByName(Bindings[i].Name);

		// 키코드 0x00은 쓰이지 않는다. 
		// Slot 0번은 할당한 갯수를 저장하므로 쓰이지 않느다.
		if( Code != 0 && Bindings[i].Slot != 0 )
		{
			KeyBytes[Bindings[i].Slot] = Code;
			Count++;
		}
	}
	
	KeyBytes[0] = Count;
	return KeyBytes;
}

function SetUserKeyByte( array<byte> KeyBytes )
{
	Local int i, FindIndex;
	Local byte Count;

	for ( i = 1 ; i < KeyBytes.Length ; i++ )
	{
		FindIndex = Bindings.Find('Slot', i);
		if( FindIndex >= 0 && KeyBytes[i] != 0)
		{
			Bindings[FindIndex].Name = GetKeyNameByCode(KeyBytes[i]);
			Count++;
		}
	}

	// 지정된 키 갯수와 할당한 키 갯수가 같은지 확인
	assert( KeyBytes.Length > 0 && Count == KeyBytes[0]);
}

native final function byte GetKeyCodeByName( name KeyName );
native final function name GetKeyNameByCode( byte KeyCode );

defaultproperties
{
    MouseSamplingTotal=+0.0083
	MouseSamples=1

	// 서버팀에서 원하는 캐릭터 변환테이블
	// 변환하는 인덱스는 Winuser.h의 VK시리즈(가상키 시리즈)의 값이며
	// 변환되어 나오는값은 0~128 (124제외)의 값으로 서버에서 문자열로 받아들일 수 있는 값이다.
	// 서버에서는 0~128(124제외)외에 키값을 허용하지 않으므로 더이상의 추가는 불가능하다.
	CustomCharTableEnc(0)=0
	CustomCharTableEnc(1)=1
	CustomCharTableEnc(2)=2
	CustomCharTableEnc(3)=3
	CustomCharTableEnc(4)=4
	CustomCharTableEnc(5)=5
	CustomCharTableEnc(6)=6
	CustomCharTableEnc(7)=0
	CustomCharTableEnc(8)=7
	CustomCharTableEnc(9)=8
	CustomCharTableEnc(10)=0
	CustomCharTableEnc(11)=0
	CustomCharTableEnc(12)=9
	CustomCharTableEnc(13)=10
	CustomCharTableEnc(14)=0
	CustomCharTableEnc(15)=0
	CustomCharTableEnc(16)=11
	CustomCharTableEnc(17)=12
	CustomCharTableEnc(18)=13
	CustomCharTableEnc(19)=14
	CustomCharTableEnc(20)=15
	CustomCharTableEnc(21)=16
	CustomCharTableEnc(22)=0
	CustomCharTableEnc(23)=17
	CustomCharTableEnc(24)=18
	CustomCharTableEnc(25)=19
	CustomCharTableEnc(26)=0
	CustomCharTableEnc(27)=20
	CustomCharTableEnc(28)=21
	CustomCharTableEnc(29)=22
	CustomCharTableEnc(30)=23
	CustomCharTableEnc(31)=24
	CustomCharTableEnc(32)=25
	CustomCharTableEnc(33)=26
	CustomCharTableEnc(34)=27
	CustomCharTableEnc(35)=28
	CustomCharTableEnc(36)=29
	CustomCharTableEnc(37)=30
	CustomCharTableEnc(38)=31
	CustomCharTableEnc(39)=32
	CustomCharTableEnc(40)=33
	CustomCharTableEnc(41)=0
	CustomCharTableEnc(42)=34
	CustomCharTableEnc(43)=0
	CustomCharTableEnc(44)=35
	CustomCharTableEnc(45)=36
	CustomCharTableEnc(46)=37
	CustomCharTableEnc(47)=0
	CustomCharTableEnc(48)=38
	CustomCharTableEnc(49)=39
	CustomCharTableEnc(50)=40
	CustomCharTableEnc(51)=41
	CustomCharTableEnc(52)=42
	CustomCharTableEnc(53)=43
	CustomCharTableEnc(54)=44
	CustomCharTableEnc(55)=45
	CustomCharTableEnc(56)=46
	CustomCharTableEnc(57)=47
	CustomCharTableEnc(58)=0
	CustomCharTableEnc(59)=0
	CustomCharTableEnc(60)=0
	CustomCharTableEnc(61)=0
	CustomCharTableEnc(62)=0
	CustomCharTableEnc(63)=0
	CustomCharTableEnc(64)=0
	CustomCharTableEnc(65)=48
	CustomCharTableEnc(66)=49
	CustomCharTableEnc(67)=50
	CustomCharTableEnc(68)=51
	CustomCharTableEnc(69)=52
	CustomCharTableEnc(70)=53
	CustomCharTableEnc(71)=54
	CustomCharTableEnc(72)=55
	CustomCharTableEnc(73)=56
	CustomCharTableEnc(74)=57
	CustomCharTableEnc(75)=58
	CustomCharTableEnc(76)=59
	CustomCharTableEnc(77)=60
	CustomCharTableEnc(78)=61
	CustomCharTableEnc(79)=62
	CustomCharTableEnc(80)=63
	CustomCharTableEnc(81)=64
	CustomCharTableEnc(82)=65
	CustomCharTableEnc(83)=66
	CustomCharTableEnc(84)=67
	CustomCharTableEnc(85)=68
	CustomCharTableEnc(86)=69
	CustomCharTableEnc(87)=70
	CustomCharTableEnc(88)=71
	CustomCharTableEnc(89)=72
	CustomCharTableEnc(90)=73
	CustomCharTableEnc(91)=74
	CustomCharTableEnc(92)=75
	CustomCharTableEnc(93)=76
	CustomCharTableEnc(94)=77
	CustomCharTableEnc(95)=78
	CustomCharTableEnc(96)=79
	CustomCharTableEnc(97)=80
	CustomCharTableEnc(98)=81
	CustomCharTableEnc(99)=82
	CustomCharTableEnc(100)=83
	CustomCharTableEnc(101)=84
	CustomCharTableEnc(102)=85
	CustomCharTableEnc(103)=86
	CustomCharTableEnc(104)=87
	CustomCharTableEnc(105)=88
	CustomCharTableEnc(106)=89
	CustomCharTableEnc(107)=90
	CustomCharTableEnc(108)=0
	CustomCharTableEnc(109)=91
	CustomCharTableEnc(110)=92
	CustomCharTableEnc(111)=93
	CustomCharTableEnc(112)=94
	CustomCharTableEnc(113)=95
	CustomCharTableEnc(114)=96
	CustomCharTableEnc(115)=97
	CustomCharTableEnc(116)=98
	CustomCharTableEnc(117)=99
	CustomCharTableEnc(118)=100
	CustomCharTableEnc(119)=101
	CustomCharTableEnc(120)=102
	CustomCharTableEnc(121)=103
	CustomCharTableEnc(122)=104
	CustomCharTableEnc(123)=105
	CustomCharTableEnc(124)=0
	CustomCharTableEnc(125)=0
	CustomCharTableEnc(126)=0
	CustomCharTableEnc(127)=0
	CustomCharTableEnc(128)=0
	CustomCharTableEnc(129)=0
	CustomCharTableEnc(130)=0
	CustomCharTableEnc(131)=0
	CustomCharTableEnc(132)=0
	CustomCharTableEnc(133)=0
	CustomCharTableEnc(134)=0
	CustomCharTableEnc(135)=0
	CustomCharTableEnc(136)=126			// MouseScrollUp
	CustomCharTableEnc(137)=127			// MouseScrollDown
	CustomCharTableEnc(138)=0
	CustomCharTableEnc(139)=0
	CustomCharTableEnc(140)=0
	CustomCharTableEnc(141)=0
	CustomCharTableEnc(142)=0
	CustomCharTableEnc(143)=0
	CustomCharTableEnc(144)=106
	CustomCharTableEnc(145)=107
	CustomCharTableEnc(146)=0
	CustomCharTableEnc(147)=0
	CustomCharTableEnc(148)=0
	CustomCharTableEnc(149)=0
	CustomCharTableEnc(150)=0
	CustomCharTableEnc(151)=0
	CustomCharTableEnc(152)=0
	CustomCharTableEnc(153)=0
	CustomCharTableEnc(154)=0
	CustomCharTableEnc(155)=0
	CustomCharTableEnc(156)=0
	CustomCharTableEnc(157)=0
	CustomCharTableEnc(158)=0
	CustomCharTableEnc(159)=0
	CustomCharTableEnc(160)=108
	CustomCharTableEnc(161)=109
	CustomCharTableEnc(162)=110
	CustomCharTableEnc(163)=111
	CustomCharTableEnc(164)=112
	CustomCharTableEnc(165)=113
	CustomCharTableEnc(166)=0
	CustomCharTableEnc(167)=0
	CustomCharTableEnc(168)=0
	CustomCharTableEnc(169)=0
	CustomCharTableEnc(170)=0
	CustomCharTableEnc(171)=0
	CustomCharTableEnc(172)=0
	CustomCharTableEnc(173)=0
	CustomCharTableEnc(174)=0
	CustomCharTableEnc(175)=0
	CustomCharTableEnc(176)=0
	CustomCharTableEnc(177)=0
	CustomCharTableEnc(178)=0
	CustomCharTableEnc(179)=0
	CustomCharTableEnc(180)=0
	CustomCharTableEnc(181)=0
	CustomCharTableEnc(182)=0
	CustomCharTableEnc(183)=0
	CustomCharTableEnc(184)=0
	CustomCharTableEnc(185)=0
	CustomCharTableEnc(186)=114
	CustomCharTableEnc(187)=115
	CustomCharTableEnc(188)=116
	CustomCharTableEnc(189)=117
	CustomCharTableEnc(190)=118
	CustomCharTableEnc(191)=119
	CustomCharTableEnc(192)=120
	CustomCharTableEnc(193)=0
	CustomCharTableEnc(194)=0
	CustomCharTableEnc(195)=0
	CustomCharTableEnc(196)=0
	CustomCharTableEnc(197)=0
	CustomCharTableEnc(198)=0
	CustomCharTableEnc(199)=0
	CustomCharTableEnc(200)=0
	CustomCharTableEnc(201)=0
	CustomCharTableEnc(202)=0
	CustomCharTableEnc(203)=0
	CustomCharTableEnc(204)=0
	CustomCharTableEnc(205)=0
	CustomCharTableEnc(206)=0
	CustomCharTableEnc(207)=0
	CustomCharTableEnc(208)=0
	CustomCharTableEnc(209)=0
	CustomCharTableEnc(210)=0
	CustomCharTableEnc(211)=0
	CustomCharTableEnc(212)=0
	CustomCharTableEnc(213)=0
	CustomCharTableEnc(214)=0
	CustomCharTableEnc(215)=0
	CustomCharTableEnc(216)=0
	CustomCharTableEnc(217)=0
	CustomCharTableEnc(218)=0
	CustomCharTableEnc(219)=121
	CustomCharTableEnc(220)=122
	CustomCharTableEnc(221)=123
	CustomCharTableEnc(222)=125
	CustomCharTableEnc(223)=0
	CustomCharTableEnc(224)=0
	CustomCharTableEnc(225)=0
	CustomCharTableEnc(226)=0
	CustomCharTableEnc(227)=0
	CustomCharTableEnc(228)=0
	CustomCharTableEnc(229)=0
	CustomCharTableEnc(230)=0
	CustomCharTableEnc(231)=0
	CustomCharTableEnc(232)=0
	CustomCharTableEnc(233)=0
	CustomCharTableEnc(234)=0
	CustomCharTableEnc(235)=0
	CustomCharTableEnc(236)=0
	CustomCharTableEnc(237)=0
	CustomCharTableEnc(238)=0
	CustomCharTableEnc(239)=0
	CustomCharTableEnc(240)=0
	CustomCharTableEnc(241)=0
	CustomCharTableEnc(242)=0
	CustomCharTableEnc(243)=0
	CustomCharTableEnc(244)=0
	CustomCharTableEnc(245)=0
	CustomCharTableEnc(246)=0
	CustomCharTableEnc(247)=0
	CustomCharTableEnc(248)=0
	CustomCharTableEnc(249)=0
	CustomCharTableEnc(250)=0
	CustomCharTableEnc(251)=0
	CustomCharTableEnc(252)=0
	CustomCharTableEnc(253)=0
	CustomCharTableEnc(254)=0
	CustomCharTableEnc(255)=0
}