/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */

class avaPlayerInput extends PlayerInput within avaPlayerController;

var float LastDuckTime;
var bool  bHoldDuck;

var Actor.EDoubleClickDir	DCMove;
var localized string		StrChangeMouseSensitivy;

var config array<KeyBind>				ObserverBindings;

// Accessor delegates
delegate bool OnInputKey(int ControllerID, name Key, EInputEvent Event, float AmountDepressed);
delegate bool OnInputAxis(int ControllerID, name Key, float Delta, float DeltaTime);
delegate bool OnInputChar(int ControllerID, InputCompositionStringData CompStrData );

event PlayerInput( float DeltaTime )
{
	local eDoubleClickDir	DoubleClickMove;

	CheckDoubleClickMove( DeltaTime );

	Super.PlayerInput( DeltaTime );

	DoubleClickMove = CheckForDoubleClickMove( DeltaTime/WorldInfo.TimeDilation );
	if ( DoubleClickMove == DCLICK_Forward )
	{
		// Dash
		DoDash();
	}

	if ( aForward <= 0.0 || aStrafe != 0.0 || bRun == 1 || bPressedJump == true )
	{
		CancelDash();
	}

	// Dash 중에는 Turn 이 제한된다.
	if ( avaPawn(Pawn) != None && avaPawn(Pawn).bIsDash )	aTurn *= 0.6;

	// 게임 로직에 의한 마우스 감도 조절...
	aTurn		*=	MouseSensitivityEx;
	aLookup		*=	MouseSensitivityEx;
	
	if ( bAlwaysCrouch == true )
		bDuck = 1;
}

function AlterMouseSensitivity( float Amount )
{
	local float		Old; 
	local string	Msg;
	Local avaOptionSettings OptionSettings;

	Old = int( MouseSensitivity );
	//MouseSensitivity = int( FClamp( MouseSensitivity + Amount, 1, 100 ) );
	//default.MouseSensitivity = int( MouseSensitivity );
	OptionSettings = class'avaOptionSettings'.static.GetDefaultObject();
	OptionSettings.SetMouseSensitivity( MouseSensitivity + Amount );
	if ( Old != OptionSettings.GetMouseSensitivity() )
	{
		// SetMouseSensitivity를 하면 PlayerInput과 OptionSettings가 모두 변경된다.
		SaveConfig();
		OptionSettings.SaveConfig();
		Msg	 =  class'avaStringHelper'.static.Replace( ""$int(Old) , "%d", StrChangeMouseSensitivy );
		Msg	 =  class'avaStringHelper'.static.Replace( ""$int(MouseSensitivity) , "%d", Msg );
		//ClientMessage( Msg );
		MyHUD.Message(PlayerReplicationInfo, Msg, '');
	}
}

exec function IncMouseSensitivity()
{
	AlterMouseSensitivity( +1 );
}

exec function DecMouseSensitivity()
{
	AlterMouseSensitivity( -1 );
}

function ClearNoInputElapsedTime()
{
	NoInputElapsedTime = 0.0;
}

event bool InputAxis(int ControllerId,name Key,float Delta,float DeltaTime)
{
	ClearNoInputElapsedTime();

	if ( OnInputAxis(ControllerID, Key, Delta, DeltaTime) )
	{
		return true;
	}
	else
	{
		return Super.InputAxis(ControllerID, Key, Delta, DeltaTime);
	}
}

event bool InputKey(int ControllerID, name Key, EInputEvent Event, float AmountDepressed = 1.f, bool bGamepad = FALSE)
{
	ClearNoInputElapsedTime();
	
	if ( OnInputKey(ControllerID, Key, Event, AmountDepressed) )
	{
		return true;
	}
	else
	{
		return Super.InputKey(ControllerID, Key, Event, AmountDepressed);
	}
}

event bool InputChar(int ControllerID, InputCompositionStringData CompStrData )
{
	if ( OnInputChar(ControllerID, CompStrData) )
	{
		return true;
	}
	else
	{
		return Super.InputChar(ControllerId, CompStrData);
	}
}

//simulated exec function DoDash()
//{
//	if ( avaPawn(Pawn)!= none )
//	{
////		`log( "avaPlayerInput.DoDash" );
//		bDash = 1;
//	}
//}
//
//simulated exec function CancelDash()
//{
//	if ( avaPawn(Pawn)!= none )
//	{
////		`log( "avaPlayerInput.CancelDash" );
//		bDash = 0;
//	}
//}

simulated exec function Duck()
{
	if ( bIgnoreCrouch ) return;
	if ( avaPawn(Pawn)!= none )
	{
		//if (bHoldDuck)
		//{
		//	bHoldDuck=false;
		//	bDuck=0;
		//	return;
		//}

		bDuck=1;
		//CancelDash();
		//if (WorldInfo.TimeSeconds - LastDuckTime < DoubleClickTime)
		//{
		//	bHoldDuck = true;
		//}

		//LastDuckTime = WorldInfo.TimeSeconds;
	}
}

simulated exec function UnDuck()
{
	if ( bIgnoreCrouch ) return;
	if ( avaPawn(Pawn)!= none && !bHoldDuck )
	{
		bDuck=0;
	}
}

exec function Jump()
{
	if( !IsMoveInputIgnored() )	
	{
		if (bDuck>0)
		{
	 		bDuck = 0;
	 		bHoldDuck = false;
		}
		bPressedJump = true;
	}
}

function CheckDoubleClickMove( float DeltaTime )
{
	DCMove = DCLICK_None;
	if ( ( DoubleClickDir != DCLICK_None ) && ( WorldInfo.TimeSeconds - DoubleClickTimer ) > DoubleClickTime )
	{
//		`log( "Clear Click - " @WorldInfo.TimeSeconds );
		DoubleClickDir		=	DCLICK_None;
		DoubleClickTimer	=	0;
	}

	if ( DoubleClickDir == DCLICK_Forward )
	{
		if ( bEdgeForward == true && bWasForward )
		{
//			`log( "Detect Double Click - " @WorldInfo.TimeSeconds );
			DCMove = DCLICK_Forward;
			DoubleClickDir		= DCLICK_Forward;
			DoubleClickTimer	= WorldInfo.TimeSeconds;
			return;
		}
	}
	else
	{
		if ( bEdgeForward == true && bWasForward )
		{
			DoubleClickDir		= DCLICK_Forward;
			DoubleClickTimer	= WorldInfo.TimeSeconds;
//			`log( "Detect Click - " @DoubleClickTimer );
		}
	}
}

// check for double click move
function Actor.EDoubleClickDir CheckForDoubleClickMove( float DeltaTime )	
{
	return DCMove;
}

function UnSetUserKey( bool bFactoryDefault, optional bool bNotifyChangeToServer )
{
	Super.UnSetUserKey(bFactoryDefault, bNotifyChangeToServer);

	//`Log("UnSetUserKey In "$Self);
	if( bNotifyChangeToServer )
	{
		class'avaNetHandler'.static.GetAvaNetHandler().OptionSaveUserKey( GetUserKeyString() );
		//`Log("NotifyChangeToServer GetUserKeyString()"$GetUserKeyString());
	}
}

exec function SetObserverKeyBinding()
{
	Bindings = ObserverBindings;
}

exec function SetDefaultKeyBinding()
{
	Bindings = DefaultBindings;
}

defaultproperties
{
	bEnableFOVScaling = true
}
