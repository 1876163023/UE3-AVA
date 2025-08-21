/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaNet

	Name: avaMsgSend.cpp

	Description: Implementation of message senders

***/
#include "avaNet.h"

using namespace Def;


#define ASSERT check

#include "avaMsgSend.h"
#include "avaConnection.h"
#include "avaNetStateController.h"

#include "ComDef/MsgDef.h"
#include "ComDef/MsgDefClient.h"
#include "ComDef/MsgDefChannel.h"
#include "ComDef/MsgDefRoom.h"
#include "ComDef/MsgDefGame.h"
#include "ComDef/MsgDefInventory.h"
#include "ComDef/MsgDefAdmin.h"
#include "ComDef/MsgDefGuild.h"
#include "RxGateTranslator/RxGateTranslator.h"



void _SendMsgToAddr(_LPMSGBUF pBuf, DWORD Key, WORD id, const TCHAR *MsgName, RxGate::LPRXNERVE_ADDRESS pAddr, UBOOL bForceDirect, DOUBLE PendingTime/* = 0.0*/)
{
	{
		ScopedMsgBufPtr MsgBuf = CreateMsgBufN(1024);
		check(MsgBuf);
		if ( RxGateTranslator::MsgData(MsgBuf, Key, &GavaNetClient->CurrentClientAddress, pAddr, pBuf) )
		{
			GavaNetClient->CurrentConnection()->Send(MsgBuf, bForceDirect);
			_LOG(TEXT("[%s] sent to %s."), MsgName, pAddr->address);
		}
	}
	if (PendingTime > 0.0)
	{
		FPendingMsg *pMsg = GavaNetClient->PendingMsgs.Find(id);
		if (pMsg)
		{
			_LPMSGBUF buf = (_LPMSGBUF)pMsg->pMsg;
			buf->Clear();
			buf->AddRight(pBuf->GetData(), pBuf->GetLength());
			pMsg->SetCheckTime(PendingTime);
		}
		else
		{
			GavaNetClient->PendingMsgs.Set(id, FPendingMsg((BYTE*)CreateMsgBuf(pBuf), PendingTime));
		}
	}
}

void _SendMsgTo(_LPMSGBUF pBuf, WORD Key, WORD id, const TCHAR *MsgName, UBOOL bForceDirect, DOUBLE PendingTime/* = 0.0*/)
{
	{
		ScopedMsgBufPtr MsgBuf = CreateMsgBufN(1024);
		check(MsgBuf);
		if ( RxGateTranslator::MsgSessionData(MsgBuf, Key, pBuf) )
		{
			GavaNetClient->CurrentConnection()->Send(MsgBuf, bForceDirect);
			_LOG(TEXT("[%s] sent."), MsgName);
		}
	}
	if (PendingTime > 0.0)
	{
		FPendingMsg *pMsg = GavaNetClient->PendingMsgs.Find(id);
		if (pMsg)
		{
			_LPMSGBUF buf = (_LPMSGBUF)pMsg->pMsg;
			buf->Clear();
			buf->AddRight(pBuf->GetData(), pBuf->GetLength());
			pMsg->SetCheckTime(PendingTime);
		}
		else
		{
			GavaNetClient->PendingMsgs.Set(id, FPendingMsg((BYTE*)CreateMsgBuf(pBuf), PendingTime));
		}
	}
}



void ProcMsgSendError(WORD MsgID, const FString Err)
{
	if (!GavaNetClient->bStarted)
		return;

	BYTE Category = (MsgID >> 8);
	BYTE id = (MsgID & 0xff);

	switch (Category)
	{
	case MC_CLIENT:		ProcMsgSendErrorClient(MsgID, Err); break;
	case MC_CHANNEL:	ProcMsgSendErrorChannel(MsgID, Err); break;
	case MC_ROOM:		ProcMsgSendErrorRoom(MsgID, Err); break;
	case MC_GAME:		ProcMsgSendErrorGame(MsgID, Err); break;
	case MC_INVENTORY:	ProcMsgSendErrorInventory(MsgID, Err); break;
	case MC_GUILD:		ProcMsgSendErrorGuild(MsgID, Err); break;
	case MC_ADMIN:		ProcMsgSendErrorAdmin(MsgID, Err); break;
	default:
		_LOG(TEXT("Failed to send some message. Category = %d, ID = %d"), Category, id);
	}
}
