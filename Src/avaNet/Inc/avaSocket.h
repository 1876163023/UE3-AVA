/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaNet

	Name: avaSocket.h

	Description: Declaration of internal socket class

***/
#ifndef _AVASOCKET_H_
#define _AVASOCKET_H_


class CavaSocket
{
public:
	CavaSocket();
	CavaSocket(SOCKET hSock);
	virtual ~CavaSocket();

	UBOOL Create(INT nSocketType = SOCK_STREAM, INT nProtocolType = IPPROTO_TCP, INT nAddressFormat = AF_INET);
	void Close();
	UBOOL Shutdown(INT how);
	UBOOL Attach(SOCKET hSocket);
	SOCKET Detach();

	SOCKET GetHandle() const { return hSocket; }
	operator SOCKET() const	{ return GetHandle(); }

	UBOOL IsValid() const { return GetHandle() != INVALID_SOCKET; }

public:
	UBOOL GetPeerName(FInternetIpAddr &address);
	UBOOL GetSockName(FInternetIpAddr &address);

	UBOOL SetSockOpt(INT nOptionName, const INT *lpOptionValue, INT nOptionLen, INT nLevel = SOL_SOCKET);
	UBOOL GetSockOpt(INT nOptionName, INT *lpOptionValue, INT *lpOptionLen, INT nLevel = SOL_SOCKET);

	UBOOL Ioctl(INT cmd, DWORD *arg);

public:
	UBOOL Listen(INT nBackLog = 5);
	CavaSocket Accept(FInternetIpAddr &addr);
	UBOOL Bind(FInternetIpAddr &addr);

	UBOOL Connect(FInternetIpAddr &addr);

	INT Recv(BYTE* lpBuf, INT nBufLen, INT nFlags = 0);
	INT Send(const BYTE *lpBuf, INT nBufLen, INT nFlags = 0);
	INT RecvFrom(BYTE *lpBuf, INT nBufLen, FInternetIpAddr &addr, INT nFlags = 0);
	INT SendTo(BYTE *lpBuf, INT nBufLen, FInternetIpAddr &addr, INT nFlags = 0);

	INT PollRead(INT timeout = 0);
	INT PollWrite(INT timeout = 0);

	UBOOL SetNonBlocking(UBOOL bIsNonBlocking = TRUE);
	UBOOL SetBroadcast(UBOOL bAllowBroadcast = TRUE);
	UBOOL SetReuseAddr(UBOOL bAllowReuse = TRUE);
	UBOOL SetLinger(UBOOL bShouldLinger = TRUE, INT timeout = 0);

protected:
	SOCKET hSocket;
};


#endif
