/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaNet

	Name: avaMsgProc.cpp

	Description: Implementation of message processors

***/
#include "avaNet.h"

#define ASSERT check

#include "ComDef/Def.h"

using namespace Def;

#include "ComDef/_MsgBase.h"
#include "avaMsgProc.h"


void PM::Proc(_LPMSGBUF pData)
{
	MSG_HEADER *pHeader = (MSG_HEADER*)pData->GetData();

	switch (pHeader->msg_cat)
	{
		case MC_CLIENT:		CLIENT::Proc(pData); break;
		case MC_CHANNEL:	CHANNEL::Proc(pData); break;
		case MC_ROOM:		ROOM::Proc(pData); break;
		case MC_GAME:		GAME::Proc(pData); break;
		case MC_INVENTORY:	INVENTORY::Proc(pData); break;
		case MC_GUILD:		GUILD::Proc(pData); break;
		case MC_ADMIN:		ADMIN::Proc(pData); break;
	default:
		_LOG(TEXT("Invalid message received. Category = %d, ID = %d"), pHeader->msg_cat, pHeader->msg_id);
	}
}


void PM::ProcTimeOut(const BYTE *Buffer, INT BufferLen)
{
	if (BufferLen < sizeof(MSG_HEADER))
		return;

	MSG_HEADER *pHeader = (MSG_HEADER*)Buffer;
	switch (pHeader->msg_cat)
	{
		case MC_CLIENT:		CLIENT::ProcTimeOut(Buffer, BufferLen); break;
		case MC_CHANNEL:	CHANNEL::ProcTimeOut(Buffer, BufferLen); break;
		case MC_ROOM:		ROOM::ProcTimeOut(Buffer, BufferLen); break;
		case MC_INVENTORY:	INVENTORY::ProcTimeOut(Buffer, BufferLen); break;
		case MC_GUILD:		GUILD::ProcTimeOut(Buffer, BufferLen); break;
	default:
		_LOG(TEXT("Some message timed out. Category = %d, ID = %d"), pHeader->msg_cat, pHeader->msg_id);
	}
}


