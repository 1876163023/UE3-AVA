/*=============================================================================
  avaVolume_MGInstall
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
  
	2006/06/09 by OZ

		MachineGun �� ��ġ�� �� �ִ����� ǥ���� �ֱ� ���� Volume �̴�.

ToDo.

	1. ��ġ����
		1-1. ��ġ������ ���⼺�� ������.
		1-2. ��ġ�� �� �ִ� Posture �� ������ �� �ִ�(�ɾƼ� ��ġ�� ���� ��ġ��..)
		1-3. ��ġ�� Instigator �� ��ġ������ ������ �ڵ����� ���ϰ� �ȴ�.
		1-4. ��ġ������ MaxPitch �� MinPitch �� ���� ���� �ִ�.
		1-5. Ư�� Team �� ��ġ�� ���� �ִ�.

=============================================================================*/
class avaVolume_MGInstall extends TriggerVolume;

var()	ETeamType			TeamIdx;			// Install Volume �� ����� �� �ִ� Team Index �̴�.
var()	bool				bCrouch;			// true �̸� ���� ���¿����� ����� �� �ִ�.


