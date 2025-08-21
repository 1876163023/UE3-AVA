#include "avaLaunch.h"
#include "avaNet.h"

#include "../../External/NProtect/NPGameLib/NPGameLib.h"

#pragma comment(lib, "../../External/NProtect/NPGameLib/NPGameLib.lib")


extern UBOOL	GIsRedduckInternal;

//#define NO_GAMEGUARD

#ifdef NO_GAMEGUARD

UBOOL NPGameInit()   { return true; }
void NPGameExit()	{ }
void NPGameSetHWnd(void *hWnd)  { }
void NPGameSendUserID(const TCHAR *pUserID)	{ debugf(TEXT("NPGameSendUserID(%s)"), pUserID); }
UBOOL NPGameCheck(double delay)	{ return true; }
BOOL CALLBACK NPGameMonCallback(DWORD dwMsg, DWORD dwArg)	{ return TRUE; }

void NPGameProcAuth( DWORD dwIndex, DWORD dwValue1, DWORD dwValue2, DWORD dwValue3 )
{
	debugf(TEXT("NPGameProcAuth(%d,%d,%d,%d) - No Implement"), dwIndex, dwValue1, dwValue2, dwValue3);
}

void NPGameSendAuth( DWORD dwIndex, DWORD dwValue1, DWORD dwValue2, DWORD dwValue3 )
{
	debugf(TEXT("NPGameSendAuth(%d,%d,%d,%d) - No Implement"), dwIndex, dwValue1, dwValue2, dwValue3);
}

DWORD NPGameGetLastError(TCHAR *pMsg)	{ return NPGAMEMON_SUCCESS; }
DWORD NPGameMonGetLastError(TCHAR *pMsg)	{ return NPGAMEMON_SUCCESS; }

#else

#if !FINAL_RELEASE
	extern UBOOL	GIsEditor;
#endif

extern UBOOL	GIsGame;
extern UBOOL	GIsRedduckInternal;

// 기본적으로 '작업관리자'에서 보이도록 한다.
TCHAR NPGTitle[256] = TEXT("AVAKR");
TCHAR NPGTitleTest[256] = TEXT("AVAKRTest");
TCHAR *pNPGTitle = NPGTitleTest;

//! 전역으로 변수 선언.
//CNPGameLib      GNpgl(NPGTitle);
CNPGameLib*		GNpgl = NULL;

//! Check함수가 호출된 마지막 시간.
double			GNPGameLastTime = 0;
//! 마지막으로 난 에러.
DWORD			GLastError = NPGAMEMON_SUCCESS;
//! 
DWORD			GNPGameMonMsg = NPGAMEMON_SUCCESS;
//!
DWORD			GNPGameMonArg = 0;

FILE*			pFile = NULL;

// prototype
BOOL CALLBACK NPGameMonCallback(DWORD dwMsg, DWORD dwArg);
void NPGameSetLastError(DWORD dwResult);
DWORD NPGameGetLastError(TCHAR *pMsg);
DWORD NPGameMonGetLastError(TCHAR *pMsg);

//! 종료하다가 빈번히 nProtect가 제대로 종료가 안되서 추가함.
class AutoDeleteNProtect
{
public:
	~AutoDeleteNProtect()
	{
		if ( GNpgl != NULL )
		{
			delete GNpgl;
			GNpgl = NULL;
		}
	}
} GAutoDeleteNProtect;

UBOOL NPGameInit()
{
	DWORD dwResult;
	TCHAR ExeName[MAX_PATH];
	FString FullName, TitleName;

	GetModuleFileName(NULL, ExeName, ARRAY_COUNT(ExeName));
	FullName = ExeName;
	TitleName = FullName.Right(FullName.Len() - FullName.InStr("\\", TRUE) - 1);

	// 실행파일이 배포 버전인 경우 '작업관리자'에서 보이지 않도록 설정한다.
	if( _tcsicmp(*TitleName, TEXT("ava.exe")) == 0 )
		pNPGTitle = NPGTitle;
	else
		pNPGTitle = NPGTitleTest;

#if !FINAL_RELEASE
	// "NOGAMEGUARD"일때 무시되도록 한다.
	if ( (GIsEditor && !GIsGame) || GIsRedduckInternal || ParseParam(appCmdLine(),TEXT("NOGAMEGUARD")) )
		return true;
#endif

	// 콜백함수 등록.(델파이용이라 필요없다??)
//	SetCallbackToGameMon(NPGameMonCallback);

	GNpgl = new CNPGameLib(pNPGTitle);

	// 성공!!
	if ( (dwResult = GNpgl->Init()) == NPGAMEMON_SUCCESS )
		return true;

	// 에러 설정.
	NPGameSetLastError(dwResult);

//	pFile = _wfopen(TEXT("GameGuard.log"), TEXT("w"));

	return false;
}

void NPGameExit()
{
	if ( (GIsEditor && !GIsGame) || GIsRedduckInternal || !GNpgl )
		return ;

//	fclose(pFile);

	TCHAR Title[256] = { NULL };
	TCHAR TitleTmp[256] = { NULL };
	TCHAR Msg[4096] = { NULL };
	TCHAR MsgTmp[4096] = { NULL };
	TCHAR Msg2[4096] = { NULL };
	int Error = 0;

	// 게임가드 에러
	if ( GLastError != NPGAMEMON_SUCCESS )
	{
		DWORD dwLastError = NPGameGetLastError(Msg);

		// 이런 경우는 에러 번호를 출력하자.
		if ( dwLastError == NPGAMEMON_COMM_ERROR ||
			 dwLastError == NPGAMEMON_COMM_CLOSE ||
			 dwLastError == NPGAMEMON_INIT_ERROR )
		{
			wsprintf(Title, TEXT("게임가드 에러(%lu)"), dwLastError);
		}
		else
		{
			wsprintf(Title, TEXT("게임가드 에러"));
		}

		// 게임가드 FAQ 페이지를 띄워줌.
		GNpgl->RunFAQ(GLastError);

		Error++;
	}

	// 게임모니터 에러
	if ( GNPGameMonMsg != NPGAMEMON_SUCCESS )
	{
		if ( Error > 0 )
		{
			_tcscpy(TitleTmp, Title);
			_tcscpy(MsgTmp, Msg);

			wsprintf(Title, TEXT("%s, 게임가드 모니터 에러(%lu)"), TitleTmp, NPGameMonGetLastError(Msg2));
			wsprintf(Msg, TEXT("%s\r\n%s"), MsgTmp, Msg2);
		}
		else
		{
			wsprintf(Title, TEXT("게임가드 모니터 에러(%lu)"), NPGameMonGetLastError(Msg));
		}

		Error++;
	}

	if ( GNpgl != NULL )
	{
		delete GNpgl;
		GNpgl = NULL;
	}

	// 포커스를 풀어서 다른 윈도우가 보이도록 해준다.
	if (GEngine && GEngine->GameViewport && GEngine->GameViewport->ViewportFrame)
	{
		//GEngine->GameViewport->ViewportFrame->UnlockFocus();
	}

	// 혹시나 스플래쉬가 있다면 꺼준다.
	{
		extern void appHideSplashEx();
		appHideSplashEx();
	}

	if ( Error > 0 )
	{
		wsprintf(MsgTmp, TEXT("%s\n게임 폴더 안의 %sGameGuard 폴더에 있는 *.erl 파일들을 Game1@inca.co.kr로 첨부하여 메일 보내주시기 바랍니다."), Msg, appBaseDir());

		::MessageBox(NULL, MsgTmp, Title, MB_OK);
	}
}

void NPGameSetHWnd(void *hWnd)
{
	if ( (GIsEditor && !GIsGame) || GIsRedduckInternal || !GNpgl )
		return ;

	GNpgl->SetHwnd((HWND)hWnd);
}

void NPGameSendUserID(const TCHAR *pUserID)
{
	if ( (GIsEditor && !GIsGame) || GIsRedduckInternal || !GNpgl )
		return ;

	if ( _tcslen(pUserID) > 0 )
		GNpgl->Send(pUserID);
}

UBOOL NPGameCheck(double delay)
{
	if ( (GIsEditor && !GIsGame) || GIsRedduckInternal || !GNpgl )
		return true;

	double time = appSeconds();
	DWORD dwResult;

	// 지연시간을 넘어가면 NPGameMon이 실행중인지 체크한다.
	if ( time - GNPGameLastTime > delay )
	{
		GNPGameLastTime = time;

		// 만약 문제가 있다면 false를 리턴하고 에러값을 설정한다.
		if ( (dwResult = GNpgl->Check()) != NPGAMEMON_SUCCESS )
		{
			NPGameSetLastError(dwResult);
			return false;
		}
	}

	return true;
}

static GG_AUTH_DATA GAuthData;
static double GLastTime = 0;
static double GLastRecvTime = 0;

void NPGameProcAuth( DWORD dwIndex, DWORD dwValue1, DWORD dwValue2, DWORD dwValue3 )
{
	if ( GIsEditor && !GIsGame || GIsRedduckInternal || !GNpgl )
		return ;

	// 이런경우는 이제 없지 않을까?
	if ( GIsRedduckInternal )
	{
		GetAvaNetRequest()->NPGameSendAuth(0, 0, 0, 0);
		return ;
	}

	GLastTime = appSeconds();

//	debugf(TEXT("NPGameProcAuth(0x%08x,0x%08x,0x%08x,0x%08x) - %f seconds"), 
//		   dwIndex, dwValue1, dwValue2, dwValue3, GLastTime - GLastRecvTime);

	GG_AUTH_DATA authData = { dwIndex, dwValue1, dwValue2, dwValue3 };
	GNpgl->Auth2(&authData);

	GAuthData = authData;

	GLastRecvTime = GLastTime;
}

void NPGameSendAuth( DWORD dwIndex, DWORD dwValue1, DWORD dwValue2, DWORD dwValue3 )
{
	if ( (GIsEditor && !GIsGame) || GIsRedduckInternal || !GNpgl )
		return ;

//	debugf(TEXT("NPGameSendAuth(0x%08x,0x%08x,0x%08x,0x%08x) - %f seconds"), 
//		   dwIndex, dwValue1, dwValue2, dwValue3, (appSeconds() - GLastTime));

	GetAvaNetRequest()->NPGameSendAuth(dwIndex, dwValue1, dwValue2, dwValue3);
}

/*! @brief 게임가드 콜백 함수 작성
	@return
		게임 종료시에는 false, 계속 진행할 경우에는 true를 리턴해 줘야 한다.
*/
BOOL CALLBACK NPGameMonCallback(DWORD dwMsg, DWORD dwArg)
{
	if ( (GIsEditor && !GIsGame) || GIsRedduckInternal || !GNpgl )
		return TRUE;

	switch(dwMsg)
	{
		// GameMon과의 통신 채널이 끊어졌습니다. 보통 GameMon이 
		// 비정상적으로 종료되었을 경우이므로 게임도 종료해줍니다.
		case NPGAMEMON_COMM_ERROR:

		// GameMon이 정상적으로 종료되어 보내는 메시지입니다. 게임도 종료해줍니다.
		case NPGAMEMON_COMM_CLOSE:

		// 스피드핵이 감지되었습니다. 게임 종료 처리를 시작하고 
		// 종료 직전에 적절한 메시지를 출력해줍니다.
		case NPGAMEMON_SPEEDHACK:

		// 게임핵이 실행되었지만 성공적으로 강제 종료시켰습니다. 
		// 게임을 계속 진행해도 무방하지만 종료하기를 권장합니다.
		case NPGAMEMON_GAMEHACK_KILLED:

		// 게임핵이 발견되었습니다. 강제 종료가 적합하지 않은 경우이므로 
		// 게임 종료 처리를 시작하고 종료 직전에 적절한 메시지 출력해줍니다.
		case NPGAMEMON_GAMEHACK_DETECT:

		// 게임핵으로 의심되는 프로그램이 실행 중 입니다. 혹은 게임이나 
		// 게임가드가 변조되었습니다. 게임 종료 처리를 시작하고 종료 직전에 
		// 불필요한 프로그램을 종료하고 다시 게임을 해보라는 메시지를 출력해줍니다.
		case NPGAMEMON_GAMEHACK_DOUBT:

		// GameMon 실행 에러입니다. 게임 종료 처리를 시작하고 종료 직전에 
		// 에러코드인 dwArg 값과 함께 적절한 메시지를 출력해 줍니다.
		case NPGAMEMON_INIT_ERROR:
			// 저장.(종료하기 전이나 필요한 부분에서 에러코드에 대한 메시지를 출력)
			GNPGameMonMsg = dwMsg;
			GNPGameMonArg = dwArg;

			GavaNetClient->CloseConnection(Def::EXIT_GAME_GUARD_ERROR);
//			appRequestExit(0);
			return FALSE;

		// 서버로 인증 패킷을 전송. 이때 Callback 함수는 게임가드 프로세서가 호출하므로 동기화가 필수
		case NPGAMEMON_CHECK_CSAUTH2:
		{
			GG_AUTH_DATA *pAuthData = (GG_AUTH_DATA*)dwArg;

			NPGameSendAuth(pAuthData->dwIndex, pAuthData->dwValue1, pAuthData->dwValue2, pAuthData->dwValue3);
			break;
		}
	}

	return TRUE;
}

//! 내부 함수.
void NPGameSetLastError(DWORD dwResult)
{
	if ( (GIsEditor && !GIsGame) || GIsRedduckInternal || !GNpgl )
		return ;

	GLastError = dwResult;
}

DWORD NPGameGetLastError(TCHAR *pMsg)
{
	if ( (GIsEditor && !GIsGame) || GIsRedduckInternal || !GNpgl )
		return NPGAMEMON_SUCCESS;

	if ( pMsg != NULL )
	{
		switch(GLastError)
		{
		case NPGAMEMON_SUCCESS:
			_tcscpy(pMsg, TEXT("아무런 문제도 없습니다"));
			break;

		case NPGAMEMON_ERROR_EXIST:
			_tcscpy(pMsg, TEXT("게임가드가 실행 중 입니다. 잠시 후나 재부팅 후에 다시 실행해보시기 바랍니다."));
			break;
		case NPGAMEMON_ERROR_GAME_EXIST:
			_tcscpy(pMsg, TEXT("게임이 중복 실행되었거나 게임가드가 이미 실행 중 입니다. 게임 종료 후 다시 실행해보시기 바랍니다."));
			break;
		case NPGAMEMON_ERROR_INIT:
			_tcscpy(pMsg, TEXT("게임가드 초기화 에러입니다. 재부팅 후 다시 실행해보거나 충돌할 수 있는 다른 프로그램들을 종료한 후 실행해 보시기 바랍니다."));
			break;
		case NPGAMEMON_ERROR_AUTH_GAMEGUARD:
		case NPGAMEMON_ERROR_NFOUND_GG:
		case NPGAMEMON_ERROR_AUTH_INI:
		case NPGAMEMON_ERROR_NFOUND_INI:
			_tcscpy(pMsg, TEXT("게임가드 파일이 없거나 변조되었습니다. 게임가드 셋업 파일을 설치해보시기 바랍니다."));
			break;
		case NPGAMEMON_ERROR_CRYPTOAPI:
			_tcscpy(pMsg, TEXT("윈도우의 일부 시스템 파일이 손상되었습니다. 인터넷 익스플로러(IE)를 다시 설치해보시기 바랍니다."));
			break;
		case NPGAMEMON_ERROR_EXECUTE:
			_tcscpy(pMsg, TEXT("게임가드 실행에 실패했습니다. 게임가드 셋업 파일을 다시 설치해보시기 바랍니다."));
			break;
		case NPGAMEMON_ERROR_ILLEGAL_PRG:
			_tcscpy(pMsg, TEXT("불법 프로그램이 발견되었습니다. 불필요한 프로그램을 종료한 후 다시 실행해보시기 바랍니다."));
			break;
		case NPGMUP_ERROR_ABORT:
			_tcscpy(pMsg, TEXT("게임가드 업데이트를 취소하셨습니다. 접속이 계속 되지 않을 경우 인터넷 및 개인 방화벽 설정을 조정해 보시기 바랍니다."));
			break;
		case NPGMUP_ERROR_CONNECT:
			_tcscpy(pMsg, TEXT("게임가드 업데이트 서버 접속에 실패하였습니다. 잠시 후 다시 접속하거나, 네트웍 상태를 점검해봅니다."));
			break;
//			case HOOK_TIMEOUT:
//				_tcscpy(pMsg, TEXT("바이러스나 스파이웨어로 인해 후킹이 실패하였습니다. 최신백신을 받으신 후 컴퓨터 전체검사를 해봅니다."));
//				break;
		case NPGAMEMON_ERROR_GAMEGUARD:
			_tcscpy(pMsg, TEXT("게임가드 초기화 에러 또는 구버젼의 게임가드 파일입니다. 게임가드 셋업파일을 다시 설치하고 게임을 실행해봅니다."));
			break;
//			case NPGMUP_ERROR_PARAM:
//			  _tcscpy(pMsg, TEXT("ini 파일이 없거나 변조되었습니다. 게임가드 셋업 파일을 설치하면 해결할 수 있습니다."));
//				break;
//			case NPGMUP_ERROR_INIT:
//				_tcscpy(pMsg, TEXT("npgmup.des 초기화 에러입니다. 게임가드폴더를 삭제후 다시 게임실행을 해봅니다."));
//				break;
		case NPGMUP_ERROR_DOWNCFG:
			_tcscpy(pMsg, TEXT("게임가드 업데이트 서버 접속에 실패하였습니다. 잠시 후 재시도 해보거나, 개인 방화벽이 있다면 설정을 조정해 보시기 바랍니다."));
			break;
		case NPGMUP_ERROR_AUTH:
			_tcscpy(pMsg, TEXT("게임가드 업데이트를 완료하지 못 했습니다. 바이러스 백신을 일시 중시 시킨 후 재시도 해보시거나, PC 관리 프로그램을 사용하시면 설정을 조정해 보시기 바랍니다."));
			break;
		case NPGAMEMON_ERROR_NPSCAN:
			_tcscpy(pMsg, TEXT("바이러스 및 해킹툴 검사 모듈 로딩에 실패 했습니다. 메모리 부족이거나 바이러스에 의한 감염일 수 있습니다."));
			break;
		default:
			// 적절한 종료 메시지 출력
			_tcscpy(pMsg, TEXT("게임가드 실행 중 에러가 발생하였습니다."));
			break;
		}
	}

	return GLastError;
}

DWORD NPGameMonGetLastError(TCHAR *pMsg)
{
	TCHAR Info[512] = { NULL };

	if ( (GIsEditor && !GIsGame) || GIsRedduckInternal || !GNpgl )
		return NPGAMEMON_SUCCESS;

	switch(GNPGameMonMsg)
	{
		// GameMon과의 통신 채널이 끊어졌습니다. 보통 GameMon이 
		// 비정상적으로 종료되었을 경우이므로 게임도 종료해줍니다.
		case NPGAMEMON_COMM_ERROR:
			wsprintf(pMsg, TEXT("게임가드 에러!"));
			break;

		// GameMon이 정상적으로 종료되어 보내는 메시지입니다. 게임도 종료해줍니다.
		case NPGAMEMON_COMM_CLOSE:
			wsprintf(pMsg, TEXT("게임가드가 종료 되었습니다.!"));
			break;

		// GameMon 실행 에러입니다. 게임 종료 처리를 시작하고 종료 직전에 
		// 에러코드인 dwArg 값과 함께 적절한 메시지를 출력해 줍니다.
		case NPGAMEMON_INIT_ERROR:
			wsprintf(pMsg, TEXT("게임가드 초기화 에러 : %lu"), GNPGameMonArg);
			break;

		// 스피드핵이 감지되었습니다. 게임 종료 처리를 시작하고 
		// 종료 직전에 적절한 메시지를 출력해줍니다.
		case NPGAMEMON_SPEEDHACK:
			wsprintf(pMsg, TEXT("스피드핵이 감지 되었습니다."));
			break;

		// 게임핵이 실행되었지만 성공적으로 강제 종료시켰습니다. 
		// 게임을 계속 진행해도 무방하지만 종료하기를 권장합니다.
		case NPGAMEMON_GAMEHACK_KILLED:
#if UNICODE
			MultiByteToWideChar( CP_ACP, 0, GNpgl->GetInfo(), -1, Info, ARRAY_COUNT(Info) );
			wsprintf(pMsg, TEXT("게임핵이 발견 되었습니다.\r\n%s"), Info);
#else
			wsprintf(pMsg, TEXT("게임핵이 발견 되었습니다.\r\n%s"), GNpgl->GetInfo());
#endif
			break;

		// 게임핵이 발견되었습니다. 강제 종료가 적합하지 않은 경우이므로 
		// 게임 종료 처리를 시작하고 종료 직전에 적절한 메시지 출력해줍니다.
		case NPGAMEMON_GAMEHACK_DETECT:
#if UNICODE
			MultiByteToWideChar( CP_ACP, 0, GNpgl->GetInfo(), -1, Info, ARRAY_COUNT(Info) );
			wsprintf(pMsg, TEXT("게임핵이 발견 되었습니다.\r\n%s"), Info);
#else
			wsprintf(pMsg, TEXT("게임핵이 발견 되었습니다.\r\n%s"), GNpgl->GetInfo());
#endif
			break;

		// 게임핵으로 의심되는 프로그램이 실행 중 입니다. 혹은 게임이나 
		// 게임가드가 변조되었습니다. 게임 종료 처리를 시작하고 종료 직전에 
		// 불필요한 프로그램을 종료하고 다시 게임을 해보라는 메시지를 출력해줍니다.
		case NPGAMEMON_GAMEHACK_DOUBT:
			wsprintf(pMsg, TEXT("게임핵으로 의심되는 프로그램이 실행 중 입니다. 혹은 게임이나 게임가드가 변조 되었습니다."));
			break;
	}

	return GNPGameMonMsg;
}

#endif
