/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaNet

	Name: avaConnection.h

	Description: Connection management class for AVA clients.

***/

#ifndef __AVACONNECTION_H__
#define __AVACONNECTION_H__



#include "ComDef/Def.h"
#include "ComDef/MsgBuf.h"
//#include "RxGateTranslator/RxGateTranslator.h"


enum EavaConnState
{
	CS_None,
	CS_Initialized,
	CS_Connecting,
	CS_Connected,
	CS_ClosePending,
	CS_Closing
};

//class CavaMsg;

class CavaConnection
{
public:
	CavaConnection();
	virtual ~CavaConnection();

	virtual void Destroy();

protected:
	CavaSocket sock;
	//CavaBuf recvbuf;
	//CavaBuf sendbuf;
	_LPMSGBUF recvbuf;
	_LPMSGBUF sendbuf;
	CavaNetClient *NetClient;

	UBOOL bInTick;

public:
	EavaConnState ConnState;
	FInternetIpAddr RemoteAddress;

protected:
	void ProcConnect();
	void ProcRecv();
	void ProcRecvBuffer();
	UBOOL FlushSendBuffer();

public:
	void SetNetClient(CavaNetClient *Client)
	{
		NetClient = Client;
	}
	CavaNetClient* GetNetClient()
	{
		return NetClient;
	}

	UBOOL Initialize();

	void Tick();

	UBOOL Connect(FInternetIpAddr addr);
	void Disconnect();
	UBOOL IsConnected();

	//INT Send(CavaMsg *msg);
	INT Send(_LPMSGBUF buf, UBOOL bForceSend = FALSE);
	INT Send(const BYTE* buf, INT len, UBOOL bForceSend = FALSE);
};



#endif

