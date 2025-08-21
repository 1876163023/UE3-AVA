/**
 * Script���� ����ϴ� SetTimer�� ����� ����� �Ѵ�.
 *
 * Start�ϸ� �� �ֱ⸶�� Out�� ƨ���ش�.
 * Stop�ϸ� ���� �����ϴ� Timer�� ������Ų��.
 *
 * 1�� ���� 5�� �̷������� ����������� ������ ������ ����ϸ� �˸��� ������ ���δ�.
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
