/**
 * Copyright 2006 Ntix Soft, Inc. All Rights Reserved.
 * Event �� ���� Animation Blending
 * Fire, Reload, BringUp ����� ���� ����

	ToDo :	
		Node �� ������� �ʴ� Animation �� Play �� ��� Stop Event �� ���� ���ϱ� ������ Animation �� ���� �� �ִ�.

  */
class avaAnimBlendByEvent extends avaAnimBlendBase
	Native;

enum EBlendEvent
{
	EBT_None,				// Event ����
	EBT_Fire,				// Fire Animation
	EBT_AltFire,			// Alt Fire Animation
	EBT_PreReload,
	EBT_Reload,				// Reload Animation
	EBT_PostReload,
	EBT_PullPin,			// Pull Pin Animation
	EBT_BringUp,			// Weapon BringUp
	EBT_QVC,				// Quick Voice Chat �� ����ȣ Animation
	EBT_Reserved,			// Reserved Event
	EBT_MountSilencer,		// Mount Silencer Event
	EBT_UnMountSilencer,	// UnMount Silencer Event

};

var() EBlendEvent	ForcedEventType;

var EBlendEvent		PrvEventType;

var int				nEventPlayed;

// Script �� ȣ�⿡ ���ؼ� Animation �� Play �� �Ǳ� ������ TickAnim �Լ��� �ʿ� ������
// Editor ���� Play �� Test �ϱ� ���ؼ� �߰�����.
// Event �� ���� Animation �� ������ ������� ���ƿ;� �ϱ� ������ OnChildAnimEnd �� ���� ó���� �ʿ��ϴ�.

// PlayEvent �� bLoop �� true ���, StopEvent Script �� ȣ������ �ʴ´�.
// ���� Loop �� �Ǵ� Event ��� �ݵ�� StopEvent �� Animation �� Stop ���Ѿ� �Ѵ�.

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
	// Quick Voice Chat Message Animation �� �ٸ� event �� Animation �� ����� �� ����.
	// �̷��� �������� Event �� Priority �� �ε��� ����

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