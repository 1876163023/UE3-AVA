/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaAnimBlendBase extends AnimNodeBlendList
	native;

/** How fast show a given child blend in. */

var(Animation) float BlendTime;

/** Also allow for Blend Overrides */
var(Animation) array<float> ChildBlendTimes;

/** slider position, for animtree editor */
var const	float	SliderPosition;

var()		bool	bBlendByEvent;
var			float	LastTickTime;

cpptext
{
	virtual void GetChildNodes(int childIdx, TArray<UAnimNode*>& Nodes);
	// AnimTree editor interface	
	virtual INT GetNumSliders() const { return 1; }
	virtual FLOAT GetSliderPosition(INT SliderIndex, INT ValueIndex);
	virtual void HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue);
	virtual FString GetSliderDrawValue(INT SliderIndex);
	virtual void TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight );
}

native final function AnimNode FindChildAnimNode(int childIdx, name InNodeName);
native final function AnimNode FindChildAnimNodeByClass(int childidx, class<avaAnimBlendBase> DesiredClass );
native function float GetBlendTime(int ChildIndex, optional bool bGetDefault);

//! 하위 AnimNodeSequence를 배열로 얻어온다(2007/02/01 고광록).
native final function GetAnimSeqNodes(int ChildIndex, out array<AnimNodeSequence> AnimSeqs);

defaultproperties
{
	BlendTime=0.25
}
