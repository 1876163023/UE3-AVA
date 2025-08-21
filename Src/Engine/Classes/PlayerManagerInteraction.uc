/**
 * Responsible for routing input events from the GameViewportClient to the
 * appropriate player.
 *
 * Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved.
 */
class PlayerManagerInteraction extends Interaction
	within GameViewportClient;


/** @name Interaction interface. */

event bool InputKey(int ControllerId,name Key,EInputEvent Event,float AmountDepressed = 1.f, bool bGamepad = FALSE)
{
	local LocalPlayer TargetPlayer;
	local PlayerController PC;
	local int InteractionIndex;

	// Find the player which ControllerId references.
	TargetPlayer = FindPlayerByControllerId(ControllerId);
	if( TargetPlayer != None )
	{
		PC = TargetPlayer.Actor;
		if ( PC != None )
		{
			// Pass the input on to the target player's interactions.
			for(InteractionIndex = 0;InteractionIndex < PC.Interactions.Length;InteractionIndex++)
			{
				if(PC.Interactions[InteractionIndex].InputKey(ControllerId,Key,Event,AmountDepressed,bGamepad))
				{
					return true;
				}
			}
		}
	}

	return false;
}

event bool InputAxis(int ControllerId,name Key,float Delta,float DeltaTime)
{
	local LocalPlayer TargetPlayer;
	local int InteractionIndex;

	// Find the player which ControllerId references.
	TargetPlayer = FindPlayerByControllerId(ControllerId);
	if( TargetPlayer != None && TargetPlayer.Actor != None )
	{
		// Pass the input on to the target player's interactions.
		for(InteractionIndex = 0;InteractionIndex < TargetPlayer.Actor.Interactions.Length;InteractionIndex++)
		{
			if(TargetPlayer.Actor.Interactions[InteractionIndex].InputAxis(ControllerId,Key,Delta,DeltaTime))
			{
				return true;
			}
		}
	}

	return false;
}

event bool InputChar(int ControllerId, InputCompositionStringData CompStrData )
{
	local LocalPlayer TargetPlayer;
	local int InteractionIndex;

	// Find the player which ControllerId references.
	TargetPlayer = FindPlayerByControllerId(ControllerId);
	if( TargetPlayer != None && TargetPlayer.Actor != None )
	{
		// Pass the input on to the target player's interactions.
		for(InteractionIndex = 0;InteractionIndex < TargetPlayer.Actor.Interactions.Length;InteractionIndex++)
		{
			if(TargetPlayer.Actor.Interactions[InteractionIndex].InputChar(ControllerId, CompStrData))
			{
				return true;
			}
		}
	}

	return false;
}
