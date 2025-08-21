/**
 * Script에서 사용하는 SetTimer와 비슷한 기능을 한다.
 *
 * Start하면 매 주기마다 Out을 튕겨준다.
 * Stop하면 현재 동작하던 Timer를 중지시킨다.
 *
 * 1초 내지 5초 이런식으로 갱신해줘야할 정보가 있을때 사용하면 알맞을 것으로 보인다.
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaSeqAct_SetTimer extends SequenceAction
	native;

cpptext
{
	void Activated()
	{
		// reset the activation time
		TimeElapsed = 0.f;
		PeriodChecker = 0.f;
	}

	UBOOL UpdateOp(FLOAT DeltaTime)
	{
		// check for stop
		if (InputLinks(1).bHasImpulse)
		{
			// finish the op
			return TRUE;
		}
		else
		{
			// update the current time
			TimeElapsed += DeltaTime;
			PeriodChecker += DeltaTime;
			if( PeriodChecker > TimerPeriod )
			{
				OutputLinks(0).bHasImpulse = TRUE;
				PeriodChecker = 0.f;
			}
			else
			{
				OutputLinks(0).bHasImpulse = FALSE;
			}
			// and force any attached variables to get the new value
			PopulateLinkedVariableValues();
		}
		return FALSE;
	}
};

var transient float PeriodChecker;

/** Amount of time this timer has been active */
var() float TimeElapsed;
var() float TimerPeriod;

/**
 * Determines whether this class should be displayed in the list of available ops in the UI's kismet editor.
 *
 * @param	TargetObject	the widget that this SequenceObject would be attached to.
 *
 * @return	TRUE if this sequence object should be available for use in the UI kismet editor
 */
event bool IsValidUISequenceObject( optional UIScreenObject TargetObject )
{
	return true;
}

defaultproperties
{
	ObjName="SetTimer(Periodic)"
	ObjCategory="Misc"

	bLatentExecution=TRUE
	bAutoActivateOutputLinks=true

	InputLinks(0)=(LinkDesc="Start")
	InputLinks(1)=(LinkDesc="Stop")

	VariableLinks.Empty
	VariableLinks(0)=(LinkDesc="TimeElapsed",ExpectedType=class'SeqVar_Float',PropertyName=TimeElapsed)
	VariableLinks(1)=(LinkDesc="Period",ExpectedType=class'SeqVar_Float',PropertyName=TimerPeriod)
}
