/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaNet

	Name: avaConnection.cpp

	Description: Implementation of avaConnection

***/
#include "avaNet.h"
#include "avaNetEventHandler.h"
#include "avaConnection.h"

#define AVA_BUFSIZE 1024


#if _MSC_VER
	#define _GetLastError() WSAGetLastError()
#else
	#define _GetLastError() errno
#endif


#ifndef ASSERT
	#define ASSERT check
#endif




CavaConnection::CavaConnection() : ConnState(CS_None), sendbuf(NULL), recvbuf(NULL), bInTick(FALSE)
{
}

CavaConnection::~CavaConnection()
{
	Destroy();
}

void CavaConnection::Destroy()
{
	if (sock.IsValid() && ConnState > CS_Initialized)
	{
		ProcRecv();
		while (FlushSendBuffer());
		sock.Close();
	}

	if (sendbuf)
	{
		sendbuf->Delete();
		sendbuf = NULL;
	}
	if (recvbuf)
	{
		recvbuf->Delete();
		recvbuf = NULL;
	}

	//Super::Destroy();
}

UBOOL CavaConnection::Initialize()
{
	ASSERT(GavaNetClient && GavaNetClient->GetEventHandler());

	if (!sock.Create())
		return FALSE;

	if (!sock.SetLinger() /*|| !sock.SetNonBlocking()*/)
	{
		sock.Close();
		return FALSE;
	}

	sendbuf = CreateMsgBufN(AVA_BUFSIZE * 4);
	recvbuf = CreateMsgBufN(AVA_BUFSIZE * 4);
	if (!sendbuf || !recvbuf)
		return FALSE;

	ConnState = CS_Initialized;

	return TRUE;
}

void CavaConnection::Tick()
{
	if (!sock.IsValid())
		return;

	//if (InterlockedCompareExchange((LONG*)&bInTick, TRUE, FALSE))
	//	return;
	//if (bInTick)
	//	return;
	//bInTick = TRUE;

	switch (ConnState)
	{
	case CS_None:
		return;

	case CS_Initialized:
		break;

	case CS_Connecting:
		ProcConnect();
		ProcRecv();
		ProcRecvBuffer();
		break;

	case CS_Connected:
		ProcRecv();
		ProcRecvBuffer();
		FlushSendBuffer();
		break;

	case CS_ClosePending:
		_LOG(TEXT("CS_ClosePending"));
		ProcRecv();
		ProcRecvBuffer();
		if (!FlushSendBuffer())
		{
			if (sock.Shutdown(2))
				ConnState = CS_Closing;
		}
		break;
	//}

	//if (sock.IsValid() && NetClient)
	//{
	//	if (sock.PollRead() > 0)
	//	{
	//		DWORD numbytes;
	//		sock.Ioctl(FIONREAD, &numbytes);
	//		if ( numbytes == 0 || (IsSocketError(numbytes) && _GetLastError() != SE_EWOULDBLOCK) )
	case CS_Closing:
			{
				_LOG(TEXT("CS_Closing"));

				ProcRecvBuffer();

				// disconnect
				ConnState = CS_Initialized;
				sock.Close();

				//if (GavaNetClient->bPendingConnection)
				//	GavaNetClient->AutoConnect();
				//else
				if (GavaNetClient->PendingConnections.Num() == 0)
					if (NetClient->GetEventHandler())
						NetClient->GetEventHandler()->Disconnected(this);
			}
			break;
	}
	//	}
	//}

	//bInTick = FALSE;
	//InterlockedExchange((LONG*)&bInTick, FALSE);
}

void CavaConnection::ProcConnect()
{
	if ( !sock.IsValid() )
		return;

	INT err = sock.PollWrite();
	if ( IsSocketError(err) )
	{
		if (NetClient && NetClient->GetEventHandler())
			NetClient->GetEventHandler()->Error(this, EavaE_Socket_FailedToConnect);
		Disconnect();
		return;
	}
	if (err == 0)
		return;

	ConnState = CS_Connected;
	NetClient->GetEventHandler()->Connected(this);
}

void CavaConnection::ProcRecv()
{
	if ( !sock.IsValid() )
		return;
	if ( !recvbuf || !NetClient )
		return;

	INT err = sock.PollRead();
	if ( IsSocketError(err) )
	{
		_LOG(TEXT("sock.PollRead() failed, err = %d"), err);
		if (NetClient->GetEventHandler())
			NetClient->GetEventHandler()->Error(this, EavaE_Socket_FailedToRecv);
		Disconnect();
		return;
	}

	if (err > 0)
	{
		BYTE buf[AVA_BUFSIZE];
		INT readlen = 0;
		while (err > 0)
		{
			readlen = sock.Recv(buf, AVA_BUFSIZE);
			if ( IsSocketError(readlen) )
			{
				_LOG(TEXT("sock.Recv() failed, readlen = %d"), readlen);
				if ( _GetLastError() != SE_EWOULDBLOCK )
				{
					_LOG(TEXT("GetLastError() is not EWOULDBLOCK"));
					err = -1;
					break;
				}
				break;
			}
			if (readlen == 0)
			{
				_LOG(TEXT("sock.Recv() returned 0, disconnecting..."));
				Disconnect();
				break;
			}

			recvbuf->AddRight((LPVOID)buf, readlen);

			err = sock.PollRead();
		}

		if (err < 0)
		{
			if (NetClient->GetEventHandler())
				NetClient->GetEventHandler()->Error(this, EavaE_Socket_FailedToRecv);
			Disconnect();
		}
	}
}

void CavaConnection::ProcRecvBuffer()
{
	if ( !recvbuf || !NetClient )
		return;

	INT len = 0;
	while (recvbuf->GetLength() > 0)
	{
		len = NetClient->GetEventHandler()->CheckMsgLength(this, recvbuf->GetData(), recvbuf->GetLength());
		//_LOG(*FString::Printf(TEXT("Msg length = %d, Buffer length = %d, Buffer cap = %d"), len, recvbuf->GetLength(), recvbuf->GetCapacity()));

		if (len == 0)
			break;

		NetClient->GetEventHandler()->MsgReceived(this, recvbuf->GetData(), len);
		recvbuf->RemoveLeft(len);
	}

	if (recvbuf->GetLength() == 0)
		recvbuf->Clear();
}

UBOOL CavaConnection::FlushSendBuffer()
{
	if ( !sock.IsValid() )
		return FALSE;
	if (!sendbuf)
		return FALSE;

	if (ConnState == CS_Connected || ConnState == CS_ClosePending)
	{
		INT count = ::Min<INT>(sendbuf->GetLength(), AVA_BUFSIZE);
		if (count == 0)
			return FALSE;
		INT sent;

		while (count > 0)
		{
			sent = sock.Send(sendbuf->GetData(), count);

			if ( IsSocketError(sent) )
			{
				_LOG(TEXT("sock.Send() failed, sent = %d"), sent);
				if (NetClient && NetClient->GetEventHandler())
					NetClient->GetEventHandler()->Error(this, EavaE_Socket_FailedToSend);
				Disconnect();
				return FALSE;
			}

			sendbuf->RemoveLeft(sent);
			count = ::Min<INT>(sendbuf->GetLength(), AVA_BUFSIZE);
		}

		sendbuf->Clear();

		return TRUE;
	}

	return FALSE;
}

UBOOL CavaConnection::Connect(FInternetIpAddr Addr)
{
	if (!sendbuf)
		return FALSE;
	if (ConnState != CS_Initialized)
		return FALSE;

	if (!sock.IsValid())
	{
		if (!sock.Create())
			return FALSE;

		if (!sock.SetLinger() /*|| !sock.SetNonBlocking()*/)
		{
			sock.Close();
			return FALSE;
		}
	}

	UBOOL res = sock.Connect(Addr);
	if (!res)
	{
		int err = _GetLastError();
#if __linux__
		bool ignoreError = ((err == EINPROGRESS) || (err == EWOULDBLOCK));
#else
		bool ignoreError = (err == WSAEWOULDBLOCK);
#endif

		if (ignoreError)
		{
			res = TRUE;
		}
	}

	if (res)
	{
		ConnState = CS_Connecting;
		RemoteAddress = Addr;
		sendbuf->Clear();
	}

	return res;
}

void CavaConnection::Disconnect()
{
	if (ConnState > CS_Connected || !sock.IsValid())
		return;

	_LOG(TEXT("Disconnecting current connection..."));

	RemoteAddress.SetIp(0);
	RemoteAddress.SetPort(0);
	ConnState = CS_ClosePending;
}

UBOOL CavaConnection::IsConnected()
{
	return (sock.IsValid() && ConnState == CS_Connected);
}

INT CavaConnection::Send(_LPMSGBUF buf, UBOOL bForceSend)
{
	if (!buf)
		return 0;
	return Send(buf->GetData(), buf->GetLength(), bForceSend);
}

INT CavaConnection::Send(const BYTE *buf, INT len, UBOOL bForceSend)
{
	if (!buf || len == 0)
		return 0;
	if (!sendbuf || !NetClient)
		return 0;
	if (ConnState != CS_Connected)
		return 0;

	//FString str;
	//TCHAR x[3];
	//str = TEXT("Message Sent : ");
	//BYTE *b = (BYTE*)buf;
	//BYTE *dest = (BYTE*)buf + len;
	//for (; b < dest; ++b)
	//{
	//	swprintf(x, TEXT("%0x "), *b);
	//	str += x;
	//}
	//GavaNetClient->LogConsole(*str);

	if (bForceSend)
	{
		INT sent = sock.Send(buf, len);
		if ( IsSocketError(sent) )
		{
			if (NetClient->GetEventHandler())
				NetClient->GetEventHandler()->Error(this, EavaE_Socket_FailedToSend);
			Disconnect();
			return 0;
		}
		return sent;
	}
	else
	{
		sendbuf->AddRight((LPVOID)buf, len);
	}

	NetClient->GetEventHandler()->MsgSent(this, buf, len);

	return len;
}


