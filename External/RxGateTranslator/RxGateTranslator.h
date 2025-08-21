#pragma once


//#ifdef _AVA_NET
//
//	#ifdef _DEBUG
//		#pragma comment(lib, "RxGateTranslator_ud.lib")
//		#pragma message("Linking RxGateTranslator_ud.lib")
//	#else
//		#pragma comment(lib, "RxGateTranslator_u.lib")
//		#pragma message("Linking RxGateTranslator_u.lib")
//	#endif
//
//#else
//
//	#ifdef _AVA_SERVER
//
//		#ifdef _DEBUG
//			#pragma comment(lib, "RxGateTranslator_sd.lib")
//			#pragma message("Linking RxGateTranslator_sd.lib")
//		#else
//			#pragma comment(lib, "RxGateTranslator_s.lib")
//			#pragma message("Linking RxGateTranslator_s.lib")
//		#endif
//
//	#else
//
//		#ifdef _DEBUG
//			#pragma comment(lib, "RxGateTranslator_d.lib")
//			#pragma message("Linking RxGateTranslator_d.lib")
//		#else
//			#pragma comment(lib, "RxGateTranslator.lib")
//			#pragma message("Linking RxGateTranslator.lib")
//		#endif
//
//	#endif
//
//#endif

#include "Def.h"
#include "ComDef/MsgBuf.h"


class IRxGateTranslateProc
{
public:
	// RxGate -> client
	virtual void MsgConnectionAns(void *info, WORD errCode, DWORD clientKey, DWORD clientIP, RxGate::LPRXNERVE_ADDRESS pGateAddr) = 0;
	virtual void MsgCreateSessionAns(void *info, WORD errCode, WORD sessionKey, RxGate::LPRXNERVE_ADDRESS pServerAddr) = 0;
	virtual void MsgJoinGroupNtf(void *info, WORD sessionKey, RxGate::LPRXNERVE_ADDRESS pServerAddr) = 0;
	virtual void MsgExitGroupNtf(void *info, WORD sessionKey, RxGate::LPRXNERVE_ADDRESS pServerAddr) = 0;
	virtual void MsgChangeSessionAns(void *info, WORD errCode, WORD reqSessionKey, WORD newSessionKey, RxGate::LPRXNERVE_ADDRESS pServerAddr) = 0;
	virtual void MsgGroupData(void *info, RxGate::LPRXNERVE_ADDRESS pGroupAddr, _LPMSGBUF pData) = 0;
	virtual void MsgCloseSessionNtf(void *info, WORD sessionKey, WORD reasonCode) = 0;

	virtual void MsgGameGuardAuthReq(void *Info, DWORD clientKey, DWORD dwIndex, DWORD dwValue1, DWORD dwValue2, DWORD dwValue3) = 0;
	virtual void MsgGameGuardErrorNtf(void *Info) = 0;

	// RxGate -> server
	//virtual void MsgSessionInfoNtf(void *info, DWORD clientKey, WORD sessionKey,LONG clientaddr) = 0;
	virtual void MsgSessionInfoReq(void *info, DWORD clientKey, WORD sessionKey,LONG clientaddr) = 0;
	virtual void MsgJoinGroupAns(void *info, BOOL join, WORD sessionKey, RxGate::LPRXNERVE_ADDRESS pGroupAddr) = 0;
	virtual void MsgExitGroupAns(void *info,WORD errCode) = 0;
	virtual void MsgChangeSessionNtf(void *info, RxGate::LPRXNERVE_ADDRESS pServerAddr) = 0;
 	virtual void MsgData(void *info, DWORD errCode, DWORD msgTag, DWORD clientKey, RxGate::LPRXNERVE_ADDRESS pServerAddr, _LPMSGBUF pData, RxGate::LPRXNERVE_ADDRESS pSrcAddr) = 0;

	virtual void MsgGameGuardAuthAns(void *Info, DWORD clientKey, DWORD dwIndex, DWORD dwValue1, DWORD dwValue2, DWORD dwValue3) = 0;

	// RxGate -> client, server
	virtual void MsgSessionData(void *info, WORD sessionKey, _LPMSGBUF pData) = 0;
};

namespace RxGateTranslator
{
	enum { BLOCKSIZE = 1024 };

	// client -> RxGate
	BOOL MsgConnectionReq(_LPMSGBUF buf, DWORD usn);
	BOOL MsgCreateSessionReq(_LPMSGBUF buf, RxGate::LPRXNERVE_ADDRESS pServerAddr);
	BOOL MsgChangeSessionReq(_LPMSGBUF buf, DWORD clientKey, WORD sessionKey, RxGate::LPRXNERVE_ADDRESS pServerAddr);
	BOOL MsgCloseSessionNtf(_LPMSGBUF buf, DWORD clientKey);
	BOOL MsgKeepAliveCheck(_LPMSGBUF buf);
	BOOL MsgData(_LPMSGBUF buf, DWORD clientKey, RxGate::LPRXNERVE_ADDRESS srcAddr,RxGate::LPRXNERVE_ADDRESS destAddr, _LPMSGBUF pData);
	BOOL MsgData(_LPMSGBUF buf, DWORD clientKey, RxGate::LPRXNERVE_ADDRESS srcAddr,RxGate::LPRXNERVE_ADDRESS destAddr, LPBYTE pData, DWORD dataLen);

	BOOL MsgGameGuardAuthReq(_LPMSGBUF buf, DWORD clientKey, DWORD dwIndex, DWORD dwValue1, DWORD dwValue2, DWORD dwValue3);

	BOOL MsgGameCloseNtf(_LPMSGBUF buf, DWORD exitCode);

	// server -> RxGate
	BOOL MsgSessionInfoAns(_LPMSGBUF buf, DWORD clientKey, WORD sessionKey);
	BOOL MsgJoinGroupReq(_LPMSGBUF buf, WORD sessionKey, RxGate::LPRXNERVE_ADDRESS pGroupAddr);
	BOOL MsgExitGroupReq(_LPMSGBUF buf, RxGate::LPRXNERVE_ADDRESS pGroupAddr);
	BOOL MsgCloseConnectionNtf(_LPMSGBUF buf, DWORD clientKey);
	BOOL MsgCloseSessionNtf(_LPMSGBUF buf, WORD sessionKey, WORD reasonCode);
	BOOL MsgGroupData(_LPMSGBUF buf, RxGate::LPRXNERVE_ADDRESS pGroupAddr, _LPMSGBUF pData);
	BOOL MsgGroupData(_LPMSGBUF buf, RxGate::LPRXNERVE_ADDRESS pGroupAddr, LPBYTE pData, DWORD dataLen);

	BOOL MsgGameGuardAuthAns(_LPMSGBUF buf, DWORD clientKey, DWORD dwIndex, DWORD dwValue1, DWORD dwValue2, DWORD dwValue3);

	// client, server -> RxGate
	BOOL MsgSessionData(_LPMSGBUF buf, WORD sessionKey, _LPMSGBUF pData);
	BOOL MsgSessionData(_LPMSGBUF buf, WORD sessionKey, LPBYTE pData, DWORD dataLen);

	// process received message
	BOOL Proc(void *info, IRxGateTranslateProc *pProc, _LPMSGBUF buf, RxGate::LPRXNERVE_ADDRESS pSrcAddr = NULL);
	BOOL Proc(void *info, IRxGateTranslateProc *pProc, LPBYTE buf, DWORD bufLen, RxGate::LPRXNERVE_ADDRESS pSrcAddr = NULL);
}


