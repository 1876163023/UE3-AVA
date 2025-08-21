/**
 * Event which is activated when the user a console command.
 * Originator: the PlayerController for the player that typed the console command
 * Instigator: the pawn of the player that typed the console command.
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class SeqEvent_Console extends SequenceEvent;

/** Name to match when entered from the console */
var() Name ConsoleEventName;

/** Description to display when listing summary of all console events */
var() string EventDesc;

defaultproperties
{
	ObjName="Console Event"
	ObjCategory="Misc"
	ConsoleEventName=Default

	EventDesc="No description"

	MaxTriggerCount=0
}
