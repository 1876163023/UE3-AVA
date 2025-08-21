//=============================================================================
//  avaBombVolume
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
//  
//	2006/02/09 by OZ
//		1.	C4 설치를 위한 BombVolume
//	2006/02/15 by OZ
//		1.	SeqEvent 처리 추가
//	ToDo.
//		1. 폭탄의 종류가 늘어나게 될 경우에는 Class 를 늘리지 말고 Bomb Type 을 이곳에 추가해준다.
//=============================================================================
class avaBombVolume extends TriggerVolume;

var()	int	TeamIdx;

simulated event Touch(Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal)
{
	if ( avaPawn( Other ) == None )	return;
	avaPawn( Other ).TouchBombVolume( self );
}

simulated event UnTouch( Actor Other )
{
	if ( avaPawn( Other ) == None )	return;
	avaPawn( Other ).UnTouchBombVolume( self );
}

// avaProj_Bomb 에 의해서 호출되는 Trigger Event 이다.
// avaSeqEvent_BombEvent 로 연결됨
// Event 에는 'defused', 'installed', 'exploded' 가 있음
function TriggerByBomb( name Event, Controller EventInstigator )
{
	local	avaSeqEvent_BombEvent	BombEvent;
	local	int i;
	for ( i=0 ; i<GeneratedEvents.Length; ++i )
	{
		BombEvent = avaSeqEvent_BombEvent( GeneratedEvents[i] );
		if ( BombEvent != none )
		{
			BombEvent.Trigger( Event, EventInstigator );
		}
	}
}

defaultproperties
{
	bColored=true
	BrushColor=(R=255,G=0,B=0,A=255)
	SupportedEvents.Add(class'avaSeqEvent_BombEvent')
	TeamIdx = 255
}