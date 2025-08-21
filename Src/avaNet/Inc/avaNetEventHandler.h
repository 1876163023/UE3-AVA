/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaNet

	Name: avaNetEventHandler.h

	Description: Derivation of interface IavaNetEventHandlerInterface

***/
#ifndef __AVAEVENTHANDLER_H__
#define __AVAEVENTHANDLER_H__



enum _AVANET_ERROR_CODE
{
	EavaE_None = 0,
	EavaE_Socket_FailedToConnect,
	EavaE_Socket_FailedToRecv,
	EavaE_Socket_FailedToSend,
	EavaE_Socket_NoConnection,
	EavaE_Session_FailedToConnect,
	EavaE_Session_FailedToCreate,
	EavaE_Session_FailedToChange,
	EavaE_Client_InvalidState,
};


#include "UnNetErrorHandler.h"

class CavaUNErrorHandler : public IUnNetErrorHandler
{
public:
	void OnFailure();
	void OnBrawl();
	void OnPackageMismatch(const TCHAR *PackageName);
	void OnPackageNotFound(const TCHAR *PackageName);
	void OnConnectionError(const TCHAR *Error);
};



class IavaNetEventHandlerInterface
{
public:
    virtual void Connected(CavaConnection*) = 0;
    virtual void Disconnected(CavaConnection*) = 0;
    virtual void MsgSent(CavaConnection*, const BYTE*, INT) = 0;
    virtual void MsgReceived(CavaConnection*, const BYTE*, INT) = 0;
	virtual void MsgTimeOut(CavaConnection*, const BYTE*, INT) = 0;
    virtual void Error(CavaConnection*, INT) = 0;
	virtual INT CheckMsgLength(CavaConnection*, const BYTE*, INT) = 0;
};



class CavaNetEventHandler : public IavaNetEventHandlerInterface, public FCallbackEventDevice
{
public:
	static DOUBLE NoticedTime;
	static INT NoticedProgress;
	static INT NoticedStep;

public:
	// IavaNetEventHandlerInterface
	void Connected(CavaConnection* Connection);
	void Disconnected(CavaConnection* Connection);
	void MsgSent(CavaConnection* Connection, const BYTE *Buffer, INT BufferLen);
	void MsgReceived(CavaConnection* Connection, const BYTE *Buffer, INT BufferLen);
	void MsgTimeOut(CavaConnection* Connection, const BYTE *Buffer, INT BufferLen);
	void Error(CavaConnection* Connection, INT Code);
	INT CheckMsgLength(CavaConnection* Connection, const BYTE *Buffer, INT BufferLen);

	// FCallbackEventDevice
	void Send(ECallbackEventType InType);

	// Misc
	static void StartLoadingProgress(UBOOL bNotify = TRUE);
	static void FinishLoadingProgress(UBOOL bNotify = TRUE);
	static void LoadingProgressCallback(INT idStep, INT CurStep, INT MaxStep, bool bTickNetClient); // static void LoadingProgressCallback(const TCHAR *StepName, FLOAT Sec, INT CurStep, INT MaxStep);	// [!] 20070510 dEAthcURe|LP
};



#endif
