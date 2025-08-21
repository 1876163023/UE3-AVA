/*=============================================================================
	UnIn.cpp: Unreal input system.
	Copyright 1997-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineUserInterfaceClasses.h"

IMPLEMENT_CLASS(UInput);
IMPLEMENT_CLASS(UPlayerInput);

//
//	UInput::FindButtonName - Find a button.
//
BYTE* UInput::FindButtonName( const TCHAR* ButtonName )
{
	FName Button( ButtonName, FNAME_Find );
	if( Button == NAME_None )
		return NULL;

	BYTE* Ptr = (BYTE*) NameToPtr.FindRef( Button );
	if( Ptr == NULL )
	{
		for(const UObject* Object = this;Object;Object = Object->GetOuter())
		{
			for( UProperty* Property = Object->GetClass()->PropertyLink; Property; Property = Property->PropertyLinkNext )
			{
				if( (Property->PropertyFlags & CPF_Input) != 0 && Property->GetFName()==Button && Property->IsA(UByteProperty::StaticClass()) )
				{
					Ptr = (BYTE*)Object + Property->Offset;
					NameToPtr.Set( Button, Ptr );
					return Ptr;
				}
			}
		}
	}

	return Ptr;
}

//
//	UInput::FindAxisName - Find an axis.
//
FLOAT* UInput::FindAxisName( const TCHAR* ButtonName )
{
	FName Button( ButtonName, FNAME_Find );
	if( Button == NAME_None )
		return NULL;

	FLOAT* Ptr = (FLOAT*) NameToPtr.FindRef( Button );
	if( Ptr == NULL )
	{
		for(const UObject* Object = this;Object;Object = Object->GetOuter())
		{
			for ( UProperty* Property = Object->GetClass()->PropertyLink; Property; Property = Property->PropertyLinkNext )
			{
				if ( (Property->PropertyFlags&CPF_Input) != 0 && Property->GetFName() == Button && Property->IsA(UFloatProperty::StaticClass()) )
				{
					Ptr = (FLOAT*) ((BYTE*) Object + Property->Offset);
					NameToPtr.Set( Button, Ptr );
					return Ptr;
				}
			}
		}
	}

	return Ptr;
}

//
//	UInput::GetBind
//
FString UInput::GetBind(FName Key) const
{
	UBOOL	Control = PressedKeys.FindItemIndex(KEY_LeftControl) != INDEX_NONE || PressedKeys.FindItemIndex(KEY_RightControl) != INDEX_NONE,
			Shift = PressedKeys.FindItemIndex(KEY_LeftShift) != INDEX_NONE || PressedKeys.FindItemIndex(KEY_RightShift) != INDEX_NONE,
			Alt = PressedKeys.FindItemIndex(KEY_LeftAlt) != INDEX_NONE || PressedKeys.FindItemIndex(KEY_RightAlt) != INDEX_NONE;

	for(INT BindIndex = Bindings.Num()-1; BindIndex >= 0; BindIndex--)
	{
		const FKeyBind&	Bind = Bindings(BindIndex);
		if(Bind.Name == Key && (!Bind.Control || Control) && (!Bind.Shift || Shift) && (!Bind.Alt || Alt))
			return Bindings(BindIndex).Command;
	}

	return TEXT("");
}

// Checks to see if InKey is pressed down

UBOOL UInput::IsPressed( FName InKey ) const
{
	return ( PressedKeys.FindItemIndex( InKey ) != INDEX_NONE );
}

UBOOL UInput::IsCtrlPressed() const
{
	return ( IsPressed( KEY_LeftControl ) || IsPressed( KEY_RightControl ) );
}

UBOOL UInput::IsShiftPressed() const
{
	return ( IsPressed( KEY_LeftShift ) || IsPressed( KEY_RightShift ) );
}

UBOOL UInput::IsAltPressed() const
{
	return ( IsPressed( KEY_LeftAlt ) || IsPressed( KEY_RightAlt ) );
}

//
//	UInput::ExecInputCommands - Execute input commands.
//

void UInput::ExecInputCommands( const TCHAR* Cmd, FOutputDevice& Ar )
{
	TCHAR Line[256];
	while( ParseLine( &Cmd, Line, ARRAY_COUNT(Line)) )
	{
		const TCHAR* Str = Line;
		if(CurrentEvent == IE_Pressed || (CurrentEvent == IE_Released && ParseCommand(&Str,TEXT("OnRelease"))))
		{
			APlayerController*	Actor = Cast<APlayerController>(GetOuter());

			if(ScriptConsoleExec(Str,Ar,this))
				continue;
			else if(Exec(Str,Ar))
				continue;
			else if(Actor && Actor->Player && Actor->Player->Exec(Str,Ar))
				continue;
		}
		else
			Exec(Str,Ar);
	}
}

//
//	UInput::Exec - Execute a command.
//
UBOOL UInput::Exec(const TCHAR* Str,FOutputDevice& Ar)
{
	TCHAR Temp[256];
	static UBOOL InAlias=0;

	if( ParseCommand( &Str, TEXT("BUTTON") ) )
	{
		// Normal button.
		BYTE* Button;
		if( ParseToken( Str, Temp, ARRAY_COUNT(Temp), 0 ) )
		{
			if	( (Button=FindButtonName(Temp))!=NULL )
			{
				if( CurrentEvent == IE_Pressed )
					*Button = 1;
				else if( CurrentEvent == IE_Released && *Button )
					*Button = 0;
			}
			else Ar.Log( TEXT("Bad Button command") );
		}
		else Ar.Log( TEXT("Bad Button command") );
		return 1;
	}
	else if( ParseCommand( &Str, TEXT("PULSE") ) )
	{
		// Normal button.
		BYTE* Button;
		if( ParseToken( Str, Temp, ARRAY_COUNT(Temp), 0 ) )
		{
			if	( (Button=FindButtonName(Temp))!=NULL )
			{
				if( CurrentEvent == IE_Pressed )
					*Button = 1;
			}
			else Ar.Log( TEXT("Bad Button command") );
		}
		else Ar.Log( TEXT("Bad Button command") );
		return 1;
	}
	else if( ParseCommand( &Str, TEXT("TOGGLE") ) )
	{
		// Toggle button.
		BYTE* Button;
		if
		(	ParseToken( Str, Temp, ARRAY_COUNT(Temp), 0 )
		&&	((Button=FindButtonName(Temp))!=NULL) )
		{
			if( CurrentEvent == IE_Pressed )
				*Button ^= 0x80;
		}
		else Ar.Log( TEXT("Bad Toggle command") );
		return 1;
	}
	else if( ParseCommand( &Str, TEXT("AXIS") ) )
	{
		// Axis movement.
		FLOAT* Axis;
		if(	
			ParseToken( Str, Temp, ARRAY_COUNT(Temp), 0 ) 
		&& (Axis=FindAxisName(Temp))!=NULL )
		{
			if( CurrentEvent == IE_Axis )
			{
				FLOAT	Speed			= 1.f, 
						DeadZone		= 0.f,
						AbsoluteAxis	= 0.f;
				UBOOL	Invert			= 1;

				Parse( Str, TEXT("SPEED=")			, Speed			);
				Parse( Str, TEXT("INVERT=")			, Invert		);
				Parse( Str, TEXT("DEADZONE=")		, DeadZone		);
				Parse( Str, TEXT("ABSOLUTEAXIS=")	, AbsoluteAxis	);

				// Axis is expected to be in -1 .. 1 range if dead zone is used.
				if( DeadZone > 0.f && DeadZone < 1.f )
				{
					// We need to translate and scale the input to the +/- 1 range after removing the dead zone.
					if( CurrentDelta > 0 )
					{
						CurrentDelta = Max( 0.f, CurrentDelta - DeadZone ) / (1.f - DeadZone);
					}
					else
					{
						CurrentDelta = -Max( 0.f, -CurrentDelta - DeadZone ) / (1.f - DeadZone);
					}
				}
				
				// Absolute axis like joysticks need to be scaled by delta time in order to be framerate independent.
				if( AbsoluteAxis )
				{
					Speed *= CurrentDeltaTime * AbsoluteAxis;
				}
				*Axis += Speed * Invert * CurrentDelta;
			}
		}
		else Ar.Logf( TEXT("%s Bad Axis command"),Str );
		return 1;
	}
	else if ( ParseCommand( &Str, TEXT("COUNT") ) )
	{
		BYTE *Count;
		if
		(	ParseToken( Str, Temp, ARRAY_COUNT(Temp), 0 ) 
		&& (Count=FindButtonName(Temp))!=NULL )
		{
			*Count += 1;
		}
		else Ar.Logf( TEXT("%s Bad Count command"),Str );
		return 1;
	}
	else if( ParseCommand( &Str, TEXT("KEYBINDING") ) && ParseToken( Str, Temp, ARRAY_COUNT(Temp), 0 ) )
	{
		FName	KeyName(Temp,FNAME_Find);
		if(KeyName != NAME_None)
		{
			for(UINT BindIndex = 0;BindIndex < (UINT)Bindings.Num();BindIndex++)
			{
				if(Bindings(BindIndex).Name == KeyName)
				{
					Ar.Logf(TEXT("%s"),*Bindings(BindIndex).Command);
					break;
				}
			}
		}

		return 1;
	}
	else if( !InAlias && ParseToken( Str, Temp, ARRAY_COUNT(Temp), 0 ) )
	{
		FName	KeyName(Temp,FNAME_Find);
		if(KeyName != NAME_None)
		{
			for(INT BindIndex = Bindings.Num() - 1; BindIndex >= 0; BindIndex--)
			{
				if(Bindings(BindIndex).Name == KeyName)
				{
					InAlias = 1;
					ExecInputCommands(*Bindings(BindIndex).Command,Ar);
					InAlias = 0;
					return 1;
				}
			}
		}
	}

	return 0;
}

//
//	UInput::InputKey
//
UBOOL UInput::InputKey(INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed,UBOOL bGamepad)
{
	switch(Event)
	{
	case IE_Pressed:
		if(PressedKeys.FindItemIndex(Key) != INDEX_NONE)
		{
			debugf(NAME_Input, TEXT("Received pressed event for key %s that was already pressed (%s)"), *Key.ToString(), *GetFullName());
			return FALSE;
		}
		PressedKeys.AddUniqueItem(Key);
		break;

	case IE_Released:
		if(!PressedKeys.RemoveItem(Key))
		{
			debugf(NAME_Input, TEXT("Received released event for key %s but key was never pressed (%s)"), *Key.ToString(), *GetFullName());
			return FALSE;
		}
		break;
	default:
		break;
	};

	CurrentEvent		= Event;
	CurrentDelta		= 0.0f;
	CurrentDeltaTime	= 0.0f;

	const FString Command = GetBind(Key);
	if(Command.Len())
	{
		ExecInputCommands(*Command,*GLog);
		return TRUE;
	}
	else
	{
		return Super::InputKey(ControllerId,Key,Event,AmountDepressed,bGamepad);
	}
}

//
//	UInput::InputAxis
//
UBOOL UInput::InputAxis(INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime)
{
	CurrentEvent		= IE_Axis;
	CurrentDelta		= Delta;
	CurrentDeltaTime	= DeltaTime;

	const FString Command = GetBind(Key);
	if(Command.Len())
	{
		ExecInputCommands(*Command,*GLog);
		return TRUE;
	}
	else
	{
		return Super::InputAxis(ControllerId,Key,Delta,DeltaTime);
	}
}

//
//	UInput::Tick - Read input for the viewport.
//
void UInput::Tick(FLOAT DeltaTime)
{
	if(DeltaTime != -1.f)
	{
		// Update held keys with IE_Repeat.
		for(UINT PressedIndex = 0;PressedIndex < (UINT)PressedKeys.Num();PressedIndex++)
		{
			// calling InputAxis here is intentional - we just want to execute the same stuff that happens in InputAxis, even though the PressedKeys array
			// is only filled by InputKey.
			InputAxis(0,PressedKeys(PressedIndex),1,DeltaTime);
		}
	}
	else
	{
		// Initialize axis array if needed.
		if( !AxisArray.Num() )
		{
			for( UProperty* Property = GetClass()->PropertyLink; Property; Property = Property->PropertyLinkNext )
			{
				if( (Property->PropertyFlags & CPF_Input) != 0 && Property->IsA(UFloatProperty::StaticClass()) )
				{
					for ( INT ArrayIndex = 0; ArrayIndex < Property->ArrayDim; ArrayIndex++ )
					{
						AxisArray.AddUniqueItem( (FLOAT*) ((BYTE*)this + Property->Offset + ArrayIndex * Property->ElementSize) );
					}
				}
			}
		}

		// Reset axis.
		for( INT i=0; i<AxisArray.Num(); i++ )
		{
			*AxisArray(i) = 0;
		}
	}

	Super::Tick(DeltaTime);
}

//
//	UInput::ResetInput - Reset the input system's state.
//
void UInput::ResetInput()
{
	FlushPressedKeys();

	// Reset all input variables.
	for( UProperty* Property = GetClass()->PropertyLink; Property; Property = Property->PropertyLinkNext )
	{
		if ( (Property->PropertyFlags&CPF_Input) != 0 )
		{
			for ( INT ArrayIndex = 0; ArrayIndex < Property->ArrayDim; ArrayIndex++ )
			{
				Property->ClearValue( (BYTE*)this + Property->Offset + ArrayIndex * Property->ElementSize );
			}
		}
	}
}


/* ==========================================================================================================
	UPlayerInput
========================================================================================================== */
/**
 * Generates an IE_Released event for each key in the PressedKeys array, then clears the array.  Should be called when another
 * interaction which swallows some (but perhaps not all) input is activated.
 */
void UPlayerInput::FlushPressedKeys()
{
	APlayerController* PlayerOwner = GetOuterAPlayerController();
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(GetOuterAPlayerController()->Player);
	if ( LocalPlayer != NULL )
	{
		TArray<FName> PressedKeyCopy = PressedKeys;
		for ( INT KeyIndex = 0; KeyIndex < PressedKeyCopy.Num(); KeyIndex++ )
		{
			FName Key = PressedKeyCopy(KeyIndex);

			// simulate a release event for this key so that the PlayerInput code can perform any cleanup required
			eventInputKey(LocalPlayer->ControllerId, Key, IE_Released, 0);
		}
	}

	Super::FlushPressedKeys();
}

BYTE UPlayerInput::GetKeyCodeByName( FName KeyName )
{
	return GEngine && GEngine->Client ? GEngine->Client->GetKeyCode( KeyName ) : NULL;
}

FName UPlayerInput::GetKeyNameByCode( BYTE KeyCode )
{
	return GEngine && GEngine->Client ? GEngine->Client->GetVirtualKeyName( KeyCode ) : NULL;
}

/// EOF



