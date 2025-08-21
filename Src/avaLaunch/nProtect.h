#pragma once

//! NPGame �ʱ�ȭ.(GameGuard�� �����Ѵ�)
extern UBOOL NPGameInit();

//! NPGame ����.(�����ڵ忡 ���� Ȩ�������� ���� ���� ���� �Ѵ�)
extern void NPGameExit();

//! ���� �������� �ڵ��� ������ �ش�.
extern void NPGameSetHWnd(void *hWnd);

//! �α��� ���̵� NPGameMon���� ������.
extern void NPGameSendUserID(const TCHAR *pUserID);

//! �����ð��� ���� ������ 1���� GameGuard�� ���������� Ȯ��.(default=5��)
extern UBOOL NPGameCheck(double delay=5.0);

//! �������� �޾Ƽ� CS���� ���� ������ ���� �Լ�.
extern void NPGameProcAuth( DWORD dwIndex, DWORD dwValue1, DWORD dwValue2, DWORD dwValue3 );

//! CS���� ��⿡�� ���� �����͸� �ٽ� ������ ������ �Լ�.
extern void NPGameSendAuth( DWORD dwIndex, DWORD dwValue1, DWORD dwValue2, DWORD dwValue3 );

//! ���Ӱ��忡�� ���������� �� ����.
extern DWORD NPGameGetLastError(TCHAR *pMsg=NULL);

//! ���Ӹ���Ϳ��� ���������� �� ����.
extern DWORD NPGameMonGetLastError(TCHAR *pMsg=NULL);