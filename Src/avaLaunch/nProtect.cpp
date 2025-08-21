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

// �⺻������ '�۾�������'���� ���̵��� �Ѵ�.
TCHAR NPGTitle[256] = TEXT("AVAKR");
TCHAR NPGTitleTest[256] = TEXT("AVAKRTest");
TCHAR *pNPGTitle = NPGTitleTest;

//! �������� ���� ����.
//CNPGameLib      GNpgl(NPGTitle);
CNPGameLib*		GNpgl = NULL;

//! Check�Լ��� ȣ��� ������ �ð�.
double			GNPGameLastTime = 0;
//! ���������� �� ����.
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

//! �����ϴٰ� ����� nProtect�� ����� ���ᰡ �ȵǼ� �߰���.
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

	// ���������� ���� ������ ��� '�۾�������'���� ������ �ʵ��� �����Ѵ�.
	if( _tcsicmp(*TitleName, TEXT("ava.exe")) == 0 )
		pNPGTitle = NPGTitle;
	else
		pNPGTitle = NPGTitleTest;

#if !FINAL_RELEASE
	// "NOGAMEGUARD"�϶� ���õǵ��� �Ѵ�.
	if ( (GIsEditor && !GIsGame) || GIsRedduckInternal || ParseParam(appCmdLine(),TEXT("NOGAMEGUARD")) )
		return true;
#endif

	// �ݹ��Լ� ���.(�����̿��̶� �ʿ����??)
//	SetCallbackToGameMon(NPGameMonCallback);

	GNpgl = new CNPGameLib(pNPGTitle);

	// ����!!
	if ( (dwResult = GNpgl->Init()) == NPGAMEMON_SUCCESS )
		return true;

	// ���� ����.
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

	// ���Ӱ��� ����
	if ( GLastError != NPGAMEMON_SUCCESS )
	{
		DWORD dwLastError = NPGameGetLastError(Msg);

		// �̷� ���� ���� ��ȣ�� �������.
		if ( dwLastError == NPGAMEMON_COMM_ERROR ||
			 dwLastError == NPGAMEMON_COMM_CLOSE ||
			 dwLastError == NPGAMEMON_INIT_ERROR )
		{
			wsprintf(Title, TEXT("���Ӱ��� ����(%lu)"), dwLastError);
		}
		else
		{
			wsprintf(Title, TEXT("���Ӱ��� ����"));
		}

		// ���Ӱ��� FAQ �������� �����.
		GNpgl->RunFAQ(GLastError);

		Error++;
	}

	// ���Ӹ���� ����
	if ( GNPGameMonMsg != NPGAMEMON_SUCCESS )
	{
		if ( Error > 0 )
		{
			_tcscpy(TitleTmp, Title);
			_tcscpy(MsgTmp, Msg);

			wsprintf(Title, TEXT("%s, ���Ӱ��� ����� ����(%lu)"), TitleTmp, NPGameMonGetLastError(Msg2));
			wsprintf(Msg, TEXT("%s\r\n%s"), MsgTmp, Msg2);
		}
		else
		{
			wsprintf(Title, TEXT("���Ӱ��� ����� ����(%lu)"), NPGameMonGetLastError(Msg));
		}

		Error++;
	}

	if ( GNpgl != NULL )
	{
		delete GNpgl;
		GNpgl = NULL;
	}

	// ��Ŀ���� Ǯ� �ٸ� �����찡 ���̵��� ���ش�.
	if (GEngine && GEngine->GameViewport && GEngine->GameViewport->ViewportFrame)
	{
		//GEngine->GameViewport->ViewportFrame->UnlockFocus();
	}

	// Ȥ�ó� ���÷����� �ִٸ� ���ش�.
	{
		extern void appHideSplashEx();
		appHideSplashEx();
	}

	if ( Error > 0 )
	{
		wsprintf(MsgTmp, TEXT("%s\n���� ���� ���� %sGameGuard ������ �ִ� *.erl ���ϵ��� Game1@inca.co.kr�� ÷���Ͽ� ���� �����ֽñ� �ٶ��ϴ�."), Msg, appBaseDir());

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

	// �����ð��� �Ѿ�� NPGameMon�� ���������� üũ�Ѵ�.
	if ( time - GNPGameLastTime > delay )
	{
		GNPGameLastTime = time;

		// ���� ������ �ִٸ� false�� �����ϰ� �������� �����Ѵ�.
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

	// �̷����� ���� ���� ������?
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

/*! @brief ���Ӱ��� �ݹ� �Լ� �ۼ�
	@return
		���� ����ÿ��� false, ��� ������ ��쿡�� true�� ������ ��� �Ѵ�.
*/
BOOL CALLBACK NPGameMonCallback(DWORD dwMsg, DWORD dwArg)
{
	if ( (GIsEditor && !GIsGame) || GIsRedduckInternal || !GNpgl )
		return TRUE;

	switch(dwMsg)
	{
		// GameMon���� ��� ä���� ���������ϴ�. ���� GameMon�� 
		// ������������ ����Ǿ��� ����̹Ƿ� ���ӵ� �������ݴϴ�.
		case NPGAMEMON_COMM_ERROR:

		// GameMon�� ���������� ����Ǿ� ������ �޽����Դϴ�. ���ӵ� �������ݴϴ�.
		case NPGAMEMON_COMM_CLOSE:

		// ���ǵ����� �����Ǿ����ϴ�. ���� ���� ó���� �����ϰ� 
		// ���� ������ ������ �޽����� ������ݴϴ�.
		case NPGAMEMON_SPEEDHACK:

		// �������� ����Ǿ����� ���������� ���� ������׽��ϴ�. 
		// ������ ��� �����ص� ���������� �����ϱ⸦ �����մϴ�.
		case NPGAMEMON_GAMEHACK_KILLED:

		// �������� �߰ߵǾ����ϴ�. ���� ���ᰡ �������� ���� ����̹Ƿ� 
		// ���� ���� ó���� �����ϰ� ���� ������ ������ �޽��� ������ݴϴ�.
		case NPGAMEMON_GAMEHACK_DETECT:

		// ���������� �ǽɵǴ� ���α׷��� ���� �� �Դϴ�. Ȥ�� �����̳� 
		// ���Ӱ��尡 �����Ǿ����ϴ�. ���� ���� ó���� �����ϰ� ���� ������ 
		// ���ʿ��� ���α׷��� �����ϰ� �ٽ� ������ �غ���� �޽����� ������ݴϴ�.
		case NPGAMEMON_GAMEHACK_DOUBT:

		// GameMon ���� �����Դϴ�. ���� ���� ó���� �����ϰ� ���� ������ 
		// �����ڵ��� dwArg ���� �Բ� ������ �޽����� ����� �ݴϴ�.
		case NPGAMEMON_INIT_ERROR:
			// ����.(�����ϱ� ���̳� �ʿ��� �κп��� �����ڵ忡 ���� �޽����� ���)
			GNPGameMonMsg = dwMsg;
			GNPGameMonArg = dwArg;

			GavaNetClient->CloseConnection(Def::EXIT_GAME_GUARD_ERROR);
//			appRequestExit(0);
			return FALSE;

		// ������ ���� ��Ŷ�� ����. �̶� Callback �Լ��� ���Ӱ��� ���μ����� ȣ���ϹǷ� ����ȭ�� �ʼ�
		case NPGAMEMON_CHECK_CSAUTH2:
		{
			GG_AUTH_DATA *pAuthData = (GG_AUTH_DATA*)dwArg;

			NPGameSendAuth(pAuthData->dwIndex, pAuthData->dwValue1, pAuthData->dwValue2, pAuthData->dwValue3);
			break;
		}
	}

	return TRUE;
}

//! ���� �Լ�.
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
			_tcscpy(pMsg, TEXT("�ƹ��� ������ �����ϴ�"));
			break;

		case NPGAMEMON_ERROR_EXIST:
			_tcscpy(pMsg, TEXT("���Ӱ��尡 ���� �� �Դϴ�. ��� �ĳ� ����� �Ŀ� �ٽ� �����غ��ñ� �ٶ��ϴ�."));
			break;
		case NPGAMEMON_ERROR_GAME_EXIST:
			_tcscpy(pMsg, TEXT("������ �ߺ� ����Ǿ��ų� ���Ӱ��尡 �̹� ���� �� �Դϴ�. ���� ���� �� �ٽ� �����غ��ñ� �ٶ��ϴ�."));
			break;
		case NPGAMEMON_ERROR_INIT:
			_tcscpy(pMsg, TEXT("���Ӱ��� �ʱ�ȭ �����Դϴ�. ����� �� �ٽ� �����غ��ų� �浹�� �� �ִ� �ٸ� ���α׷����� ������ �� ������ ���ñ� �ٶ��ϴ�."));
			break;
		case NPGAMEMON_ERROR_AUTH_GAMEGUARD:
		case NPGAMEMON_ERROR_NFOUND_GG:
		case NPGAMEMON_ERROR_AUTH_INI:
		case NPGAMEMON_ERROR_NFOUND_INI:
			_tcscpy(pMsg, TEXT("���Ӱ��� ������ ���ų� �����Ǿ����ϴ�. ���Ӱ��� �¾� ������ ��ġ�غ��ñ� �ٶ��ϴ�."));
			break;
		case NPGAMEMON_ERROR_CRYPTOAPI:
			_tcscpy(pMsg, TEXT("�������� �Ϻ� �ý��� ������ �ջ�Ǿ����ϴ�. ���ͳ� �ͽ��÷η�(IE)�� �ٽ� ��ġ�غ��ñ� �ٶ��ϴ�."));
			break;
		case NPGAMEMON_ERROR_EXECUTE:
			_tcscpy(pMsg, TEXT("���Ӱ��� ���࿡ �����߽��ϴ�. ���Ӱ��� �¾� ������ �ٽ� ��ġ�غ��ñ� �ٶ��ϴ�."));
			break;
		case NPGAMEMON_ERROR_ILLEGAL_PRG:
			_tcscpy(pMsg, TEXT("�ҹ� ���α׷��� �߰ߵǾ����ϴ�. ���ʿ��� ���α׷��� ������ �� �ٽ� �����غ��ñ� �ٶ��ϴ�."));
			break;
		case NPGMUP_ERROR_ABORT:
			_tcscpy(pMsg, TEXT("���Ӱ��� ������Ʈ�� ����ϼ̽��ϴ�. ������ ��� ���� ���� ��� ���ͳ� �� ���� ��ȭ�� ������ ������ ���ñ� �ٶ��ϴ�."));
			break;
		case NPGMUP_ERROR_CONNECT:
			_tcscpy(pMsg, TEXT("���Ӱ��� ������Ʈ ���� ���ӿ� �����Ͽ����ϴ�. ��� �� �ٽ� �����ϰų�, ��Ʈ�� ���¸� �����غ��ϴ�."));
			break;
//			case HOOK_TIMEOUT:
//				_tcscpy(pMsg, TEXT("���̷����� �����̿���� ���� ��ŷ�� �����Ͽ����ϴ�. �ֽŹ���� ������ �� ��ǻ�� ��ü�˻縦 �غ��ϴ�."));
//				break;
		case NPGAMEMON_ERROR_GAMEGUARD:
			_tcscpy(pMsg, TEXT("���Ӱ��� �ʱ�ȭ ���� �Ǵ� �������� ���Ӱ��� �����Դϴ�. ���Ӱ��� �¾������� �ٽ� ��ġ�ϰ� ������ �����غ��ϴ�."));
			break;
//			case NPGMUP_ERROR_PARAM:
//			  _tcscpy(pMsg, TEXT("ini ������ ���ų� �����Ǿ����ϴ�. ���Ӱ��� �¾� ������ ��ġ�ϸ� �ذ��� �� �ֽ��ϴ�."));
//				break;
//			case NPGMUP_ERROR_INIT:
//				_tcscpy(pMsg, TEXT("npgmup.des �ʱ�ȭ �����Դϴ�. ���Ӱ��������� ������ �ٽ� ���ӽ����� �غ��ϴ�."));
//				break;
		case NPGMUP_ERROR_DOWNCFG:
			_tcscpy(pMsg, TEXT("���Ӱ��� ������Ʈ ���� ���ӿ� �����Ͽ����ϴ�. ��� �� ��õ� �غ��ų�, ���� ��ȭ���� �ִٸ� ������ ������ ���ñ� �ٶ��ϴ�."));
			break;
		case NPGMUP_ERROR_AUTH:
			_tcscpy(pMsg, TEXT("���Ӱ��� ������Ʈ�� �Ϸ����� �� �߽��ϴ�. ���̷��� ����� �Ͻ� �߽� ��Ų �� ��õ� �غ��ðų�, PC ���� ���α׷��� ����Ͻø� ������ ������ ���ñ� �ٶ��ϴ�."));
			break;
		case NPGAMEMON_ERROR_NPSCAN:
			_tcscpy(pMsg, TEXT("���̷��� �� ��ŷ�� �˻� ��� �ε��� ���� �߽��ϴ�. �޸� �����̰ų� ���̷����� ���� ������ �� �ֽ��ϴ�."));
			break;
		default:
			// ������ ���� �޽��� ���
			_tcscpy(pMsg, TEXT("���Ӱ��� ���� �� ������ �߻��Ͽ����ϴ�."));
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
		// GameMon���� ��� ä���� ���������ϴ�. ���� GameMon�� 
		// ������������ ����Ǿ��� ����̹Ƿ� ���ӵ� �������ݴϴ�.
		case NPGAMEMON_COMM_ERROR:
			wsprintf(pMsg, TEXT("���Ӱ��� ����!"));
			break;

		// GameMon�� ���������� ����Ǿ� ������ �޽����Դϴ�. ���ӵ� �������ݴϴ�.
		case NPGAMEMON_COMM_CLOSE:
			wsprintf(pMsg, TEXT("���Ӱ��尡 ���� �Ǿ����ϴ�.!"));
			break;

		// GameMon ���� �����Դϴ�. ���� ���� ó���� �����ϰ� ���� ������ 
		// �����ڵ��� dwArg ���� �Բ� ������ �޽����� ����� �ݴϴ�.
		case NPGAMEMON_INIT_ERROR:
			wsprintf(pMsg, TEXT("���Ӱ��� �ʱ�ȭ ���� : %lu"), GNPGameMonArg);
			break;

		// ���ǵ����� �����Ǿ����ϴ�. ���� ���� ó���� �����ϰ� 
		// ���� ������ ������ �޽����� ������ݴϴ�.
		case NPGAMEMON_SPEEDHACK:
			wsprintf(pMsg, TEXT("���ǵ����� ���� �Ǿ����ϴ�."));
			break;

		// �������� ����Ǿ����� ���������� ���� ������׽��ϴ�. 
		// ������ ��� �����ص� ���������� �����ϱ⸦ �����մϴ�.
		case NPGAMEMON_GAMEHACK_KILLED:
#if UNICODE
			MultiByteToWideChar( CP_ACP, 0, GNpgl->GetInfo(), -1, Info, ARRAY_COUNT(Info) );
			wsprintf(pMsg, TEXT("�������� �߰� �Ǿ����ϴ�.\r\n%s"), Info);
#else
			wsprintf(pMsg, TEXT("�������� �߰� �Ǿ����ϴ�.\r\n%s"), GNpgl->GetInfo());
#endif
			break;

		// �������� �߰ߵǾ����ϴ�. ���� ���ᰡ �������� ���� ����̹Ƿ� 
		// ���� ���� ó���� �����ϰ� ���� ������ ������ �޽��� ������ݴϴ�.
		case NPGAMEMON_GAMEHACK_DETECT:
#if UNICODE
			MultiByteToWideChar( CP_ACP, 0, GNpgl->GetInfo(), -1, Info, ARRAY_COUNT(Info) );
			wsprintf(pMsg, TEXT("�������� �߰� �Ǿ����ϴ�.\r\n%s"), Info);
#else
			wsprintf(pMsg, TEXT("�������� �߰� �Ǿ����ϴ�.\r\n%s"), GNpgl->GetInfo());
#endif
			break;

		// ���������� �ǽɵǴ� ���α׷��� ���� �� �Դϴ�. Ȥ�� �����̳� 
		// ���Ӱ��尡 �����Ǿ����ϴ�. ���� ���� ó���� �����ϰ� ���� ������ 
		// ���ʿ��� ���α׷��� �����ϰ� �ٽ� ������ �غ���� �޽����� ������ݴϴ�.
		case NPGAMEMON_GAMEHACK_DOUBT:
			wsprintf(pMsg, TEXT("���������� �ǽɵǴ� ���α׷��� ���� �� �Դϴ�. Ȥ�� �����̳� ���Ӱ��尡 ���� �Ǿ����ϴ�."));
			break;
	}

	return GNPGameMonMsg;
}

#endif
