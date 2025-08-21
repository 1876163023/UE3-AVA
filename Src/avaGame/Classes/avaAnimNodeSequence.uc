/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaAnimNodeSequence extends AnimNodeSequence
	native;

var bool bAutoStart;

/** When a given sequence is finished, it will continue to select new sequences from this list */
var array<name> SeqStack;

/** If true, when the last sequence in the stack is reached, it will be looped */
var bool bLoopLastSequence;

cpptext
{
	virtual void OnAnimEnd(FLOAT PlayedTime, FLOAT ExcessTime);
}

native function PlayAnimation(name Sequence, float SeqRate, bool bSeqLoop);
//! Sequences의 차례로 연속적으로 애니메이션 시켜준다.
native function PlayAnimationSet(array<name> Sequences, float SeqRate, bool bLoopLast);

event OnInit()
{
	Super.OnInit();

	if (bAutoStart)
	{
		PlayAnim(bLooping, Rate);
	}
}

