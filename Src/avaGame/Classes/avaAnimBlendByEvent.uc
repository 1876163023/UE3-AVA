/**
 * Copyright 2006 Ntix Soft, Inc. All Rights Reserved.
 * Event 에 의한 Animation Blending
 * Fire, Reload, BringUp 등등을 위한 구현

	ToDo :	
		Node 에 연결되지 않는 Animation 을 Play 할 경우 Stop Event 를 받지 못하기 때문에 Animation 이 꼬일 수 있다.

  */
class avaAnimBlendByEvent extends avaAnimBlendBase
	Native;

enum EBlendEvent
{
	EBT_None,				// Event 없음
	EBT_Fire,				// Fire Animation
	EBT_AltFire,			// Alt Fire Animation
	EBT_PreReload,
	EBT_Reload,				// Reload Animation
	EBT_PostReload,
	EBT_PullPin,			// Pull Pin Animation
	EBT_BringUp,			// Weapon BringUp
	EBT_QVC,				// Quick Voice Chat 용 수신호 Animation
	EBT_Reserved,			// Reserved Event
	EBT_MountSilencer,		// Mount Silencer Event
	EBT_UnMountSilencer,	// UnMount Silencer Event

};

var() EBlendEvent	ForcedEventType;

var EBlendEvent		PrvEventType;

var int				nEventPlayed;

// Script 의 호출에 의해서 Animation 이 Play 가 되기 때문에 TickAnim 함수는 필요 없지만
// Editor 에서 Play 를 Test 하기 위해서 추가했음.
// Event 에 의한 Animation 이 끝나면 원래대로 돌아와야 하기 때문에 OnChildAnimEnd 에 의한 처리가 필요하다.

// PlayEvent 시 bLoop 가 true 라면, StopEvent Script 를 호출하지 않는다.
// 따라서 Loop 가 되는 Event 라면 반드시 StopEvent 로 Animation 을 Stop 시켜야 한다.

// 
cpptext
{
	virtual void OnChildAnimEnd(UAnimNodeSequence* Child, FLOAT PlayedTime, FLOAT ExcessTime);
	virtual	void TickAnim( float DeltaSeconds, float TotalWeight  );

	virtual INT GetNumSliders() const { return 1; }
	virtual FLOAT GetSliderPosition(INT SliderIndex, INT ValueIndex);
	virtual void HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue);
	virtual FString GetSliderDrawValue(INT SliderIndex);
}


native function PlayAnimByName( name Prefix, name SequenceName, bool bLoop, optional float AnimRate );
//{
//	local AnimNodeSequence ReservedNode;
//
//	ReservedNode = AnimNodeSequence(Children[EBT_Reserved].Anim);
//	if ( ReservedNode != None )
//	{
//		ReservedNode.SetAnim(SequenceName);
//		PlayAnimByEvent( EBT_Reserved, bLoop, AnimRate );
//	}
//}

event function PlayAnimByEvent( EBlendEvent eventType, bool bLoop, optional float AnimRate )
{
	// Quick Voice Chat Message Animation 은 다른 event 의 Animation 을 취소할 수 없다.
	// 이런게 많아지면 Event 에 Priority 를 두도록 하자

	if ( eventType == EBT_QVC && PrvEventType != EBT_None )	return;

	if ( PrvEventType != EBT_None && Children[PrvEventType].Anim != None )
	{
		Children[PrvEventType].Anim.StopAnim();
	}

	if ( Children[eventType].Anim != None )
	{
		if ( AnimRate == 0 )
		{
			AnimRate = 1.0;
		}

		SetActiveChild( eventType, GetBlendTime(eventType) );

		Children[eventType].Anim.PlayAnim( bLoop, AnimRate, 0.0 );

		PrvEventType = eventType;
	}	
}

event StopEvent( int nEventType, optional float SpecialBlendTime )
{
	if ( ActiveChildIndex == EBT_None )	return;
	
	PrvEventType = EBT_None;

	if (SpecialBlendTime == 0.0f)
		SpecialBlendTime = BlendTime;

	SetActiveChild( EBT_None, GetBlendTime(EBT_None) );
}

delegate OnStopEvent( int nEventType );

defaultproperties
{
	Children(0)=(Name="Event-None",Weight=1.0)
	Children(1)=(Name="Event-Fire")
	Children(2)=(Name="Event-AltFire")
	Children(3)=(Name="Event-PreReload")
	Children(4)=(Name="Event-Reload")
	Children(5)=(Name="Event-PostReload")
	Children(6)=(Name="Event-PullPin")
	Children(7)=(Name="Event-BringUp")
	Children(8)=(Name="Event-QVC")
	Children(9)=(Name="Event-Reserved")
	Children(10)=(Name="Event-MountSilencer")
	Children(11)=(Name="Event-UnMountSilencer")
	
	bFixNumChildren=true
	PrvEventType = EBT_None;
}