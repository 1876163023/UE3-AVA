//=============================================================================
//  avaBombVolume
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
//  
//	2006/02/09 by OZ
//		1.	C4 ��ġ�� ���� BombVolume
//	2006/02/15 by OZ
//		1.	SeqEvent ó�� �߰�
//	ToDo.
//		1. ��ź�� ������ �þ�� �� ��쿡�� Class �� �ø��� ���� Bomb Type �� �̰��� �߰����ش�.
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

// avaProj_Bomb �� ���ؼ� ȣ��Ǵ� Trigger Event �̴�.
// avaSeqEvent_BombEvent �� �����
// Event ���� 'defused', 'installed', 'exploded' �� ����
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