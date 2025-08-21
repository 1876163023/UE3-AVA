/*=============================================================================
  avaVolume_MGInstall
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
  
	2006/06/09 by OZ

		MachineGun 을 거치할 수 있는지를 표시해 주기 위한 Volume 이다.

ToDo.

	1. 거치볼륨
		1-1. 거치볼륨은 방향성을 가진다.
		1-2. 거치할 수 있는 Posture 를 지정할 수 있다(앉아서 거치냐 서서 거치냐..)
		1-3. 거치시 Instigator 는 거치볼륨의 방향을 자동으로 향하게 된다.
		1-4. 거치볼륨은 MaxPitch 와 MinPitch 를 가질 수도 있다.
		1-5. 특정 Team 만 거치할 수도 있다.

=============================================================================*/
class avaVolume_MGInstall extends TriggerVolume;

var()	ETeamType			TeamIdx;			// Install Volume 을 사용할 수 있는 Team Index 이다.
var()	bool				bCrouch;			// true 이면 앉은 상태에서만 사용할 수 있다.


