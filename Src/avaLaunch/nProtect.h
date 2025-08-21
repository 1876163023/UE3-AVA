#pragma once

//! NPGame 초기화.(GameGuard를 실행한다)
extern UBOOL NPGameInit();

//! NPGame 종료.(에러코드에 대한 홈페이지를 띄우는 등의 일을 한다)
extern void NPGameExit();

//! 메인 윈도우의 핸들을 설정해 준다.
extern void NPGameSetHWnd(void *hWnd);

//! 로그인 아이디를 NPGameMon으로 보낸다.
extern void NPGameSendUserID(const TCHAR *pUserID);

//! 지연시간이 지날 때마다 1번씩 GameGuard가 실행중인지 확인.(default=5초)
extern UBOOL NPGameCheck(double delay=5.0);

//! 서버에서 받아서 CS인증 모듈로 보내기 위한 함수.
extern void NPGameProcAuth( DWORD dwIndex, DWORD dwValue1, DWORD dwValue2, DWORD dwValue3 );

//! CS인증 모듈에서 받은 데이터를 다시 서버로 보내는 함수.
extern void NPGameSendAuth( DWORD dwIndex, DWORD dwValue1, DWORD dwValue2, DWORD dwValue3 );

//! 게임가드에서 마지막으로 난 에러.
extern DWORD NPGameGetLastError(TCHAR *pMsg=NULL);

//! 게임모니터에서 마지막으로 난 에러.
extern DWORD NPGameMonGetLastError(TCHAR *pMsg=NULL);