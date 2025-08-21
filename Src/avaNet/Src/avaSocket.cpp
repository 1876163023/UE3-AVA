/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaNet

	Name: avaSocket.cpp

	Description: Implementation of CavaSocket

***/
#include "avaNet.h"
#include "avaSocket.h"



inline UBOOL is_valid_socket(SOCKET s)
{
	return (s != INVALID_SOCKET);
}


CavaSocket::CavaSocket() : hSocket(INVALID_SOCKET)
{
}

CavaSocket::CavaSocket(SOCKET hSock)
{
	Attach(hSock);
}

CavaSocket::~CavaSocket()
{
	Close();
}

void CavaSocket::Close()
{
	if (IsValid())
	{
		SOCKET sock = Detach();
		closesocket(sock);
	}
}

UBOOL CavaSocket::Shutdown(INT how)
{
	if (IsValid())
	{
		INT err = ::shutdown(hSocket, how);
		return !IsSocketError(err);
	}

	return false;
}

UBOOL CavaSocket::Create(INT nSocketType, INT nProtocolType, INT nAddressFormat)
{
	checkMsg(!IsValid(), "Invalid Socket Handle");

	if (IsValid())	
		return false;

	SOCKET sock = ::socket(nAddressFormat, nSocketType, nProtocolType);
	if ( !is_valid_socket(sock) )
		return false;
	return Attach(sock);
}

UBOOL CavaSocket::Attach(SOCKET hSock)
{
	checkMsg(is_valid_socket(hSock), "Tried to attach invalid socket handle");
	checkMsg(!is_valid_socket(hSocket), "Already attached to valid socket handle");

	if (!is_valid_socket(hSock))
		return false;
	if (is_valid_socket(hSocket))
		return false;

	hSocket = hSock;
	return true;
}

SOCKET CavaSocket::Detach()
{
	checkMsg(is_valid_socket(hSocket), "Tried to detach invalid socket handle");
	SOCKET _hSock = hSocket;
	hSocket = INVALID_SOCKET;
	return _hSock;
}

UBOOL CavaSocket::GetPeerName(FInternetIpAddr &address)
{
	checkMsg(is_valid_socket(hSocket), "API call to invalid socket handle");

	SOCKADDR_IN addr;
	INT len = sizeof(addr);
	INT res = ::getpeername(hSocket, (SOCKADDR*)&addr, &len);

	if (res)
		return false;

	address.SetIp(addr.sin_addr);
	address.SetPort(addr.sin_port);

	return true;
}

UBOOL CavaSocket::GetSockName(FInternetIpAddr &address)
{
	checkMsg(is_valid_socket(hSocket), "API call to invalid socket handle");

	SOCKADDR_IN addr;
	INT len = sizeof(addr);
	INT res = ::getsockname(hSocket, (SOCKADDR*)&addr, &len);

	if (res)
		return false;

	address.SetIp(addr.sin_addr);
	address.SetPort(addr.sin_port);

	return true;
}

UBOOL CavaSocket::SetSockOpt(INT nOptionName, const INT *lpOptionValue, INT nOptionLen, INT nLevel)
{
	checkMsg(lpOptionValue, "Invalid pointer");
	checkMsg(is_valid_socket(hSocket), "API call to invalid socket handle");
#if _MSC_VER
	INT _nRet = ::setsockopt(hSocket, nLevel, nOptionName, (const char*)lpOptionValue, nOptionLen);
#else
	INT _nRet = ::setsockopt(hSocket, nLevel, nOptionName, (const int*)lpOptionValue, nOptionLen);
#endif
	return (0 == _nRet);
}

UBOOL CavaSocket::GetSockOpt(INT nOptionName, INT *lpOptionValue, INT* lpOptionLen, INT nLevel)
{
	checkMsg(lpOptionValue, "Invalid pointer");
	checkMsg(is_valid_socket(hSocket), "API call to invalid socket handle");
#if _MSC_VER
	INT _nRet = ::getsockopt(hSocket, nLevel, nOptionName, (char*)lpOptionValue, (int*)lpOptionLen);
#else
	INT _nRet = ::getsockopt(hSocket, nLevel, nOptionName, (int*)lpOptionValue, (int*)lpOptionLen);
#endif
	return (0 == _nRet);
}

UBOOL CavaSocket::Ioctl(INT cmd, DWORD *arg)
{
	checkMsg(arg, "Invalid pointer");
	checkMsg(is_valid_socket(hSocket), "API call to invalid socket handle");
	INT _nRet = ioctlsocket(hSocket, cmd, arg);
	return (0 == _nRet);
}

UBOOL CavaSocket::Listen(INT nBacklog)
{
	checkMsg(is_valid_socket(hSocket), "API call to invalid socket handle");
	return (::listen(hSocket, nBacklog) != 0);
}

CavaSocket CavaSocket::Accept(FInternetIpAddr &addr)
{
	checkMsg(is_valid_socket(hSocket), "API call to invalid socket handle");

	INT addrlen = sizeof(SOCKADDR);
	SOCKET sock = ::accept(hSocket, (SOCKADDR*)addr, &addrlen);

	return sock;
}

UBOOL CavaSocket::Bind(FInternetIpAddr &addr)
{
	checkMsg(is_valid_socket(hSocket), "API call to invalid socket handle");

	return (::bind(hSocket, (SOCKADDR*)addr, sizeof(SOCKADDR)) == 0);
}

UBOOL CavaSocket::Connect(FInternetIpAddr &addr)
{
	checkMsg(is_valid_socket(hSocket), "API call to invalid socket handle");

	return (::connect(hSocket, (SOCKADDR*)addr, sizeof(SOCKADDR)) != SOCKET_ERROR);
}

INT CavaSocket::Recv(BYTE *lpBuf, INT nBufLen, INT nFlags)
{
	checkMsg(lpBuf, "Invalid pointer");
	if (!lpBuf)
	{
		return SOCKET_ERROR;
	}

	checkMsg(is_valid_socket(hSocket), "API call to invalid socket handle");
	return ::recv(hSocket, (char *)lpBuf, nBufLen, nFlags);
}

INT CavaSocket::Send(const BYTE *lpBuf, INT nBufLen, INT nFlags)
{
	checkMsg(lpBuf, "Invalid pointer");
	if (!lpBuf)
	{
		return SOCKET_ERROR;
	}
	checkMsg(is_valid_socket(hSocket), "API call to invalid socket handle");
	return ::send(hSocket, (const char *)lpBuf, nBufLen, nFlags);
}

INT CavaSocket::RecvFrom(BYTE *lpBuf, INT nBufLen, FInternetIpAddr &addr, INT nFlags)
{
	checkMsg(lpBuf, "Invalid pointer");

	if (!lpBuf)
	{
		return SOCKET_ERROR;
	}
	checkMsg(is_valid_socket(hSocket), "API call to invalid socket handle");

	INT addrlen = sizeof(SOCKADDR);
	INT len = ::recvfrom(hSocket, (char *)lpBuf, nBufLen, nFlags, (SOCKADDR*)addr, &addrlen);

	return len;
}

INT CavaSocket::SendTo(BYTE *lpBuf, INT nBufLen, FInternetIpAddr &addr, INT nFlags)
{
	checkMsg(lpBuf, "Invalid Pointer");
	if (!lpBuf)
	{
		return SOCKET_ERROR;
	}
	checkMsg(is_valid_socket(hSocket), "API call to invalid socket handle");

	return ::sendto(hSocket, (const char *)lpBuf, nBufLen, nFlags, (SOCKADDR*)addr, sizeof(SOCKADDR));
}

INT CavaSocket::PollRead(INT timeout)
{
	if (!IsValid())
		return SOCKET_ERROR;

	fd_set sockset;
	FD_ZERO(&sockset);
	FD_SET(hSocket, &sockset);

	if (timeout >= 0)
	{
		TIMEVAL seltime = {0, timeout};
		return select(hSocket + 1, &sockset, 0, 0, &seltime);
	}
	else
		return select(hSocket + 1, &sockset, 0, 0, NULL);
}

INT CavaSocket::PollWrite(INT timeout)
{
	if (!IsValid())
		return SOCKET_ERROR;

	fd_set sockset;
	FD_ZERO(&sockset);
	FD_SET(hSocket, &sockset);

	if (timeout >= 0)
	{
		TIMEVAL seltime = {0, timeout};
		return select(hSocket + 1, 0, &sockset, 0, &seltime);
	}
	else
		return select(hSocket + 1, 0, &sockset, 0, NULL);
}

UBOOL CavaSocket::SetNonBlocking(UBOOL bIsNonBlocking)
{
#if __UNIX__
	INT pd_flags;
	pd_flags = fcntl(hSocket, F_GETFL, 0);
	if (bIsNonBlocking)
		pd_flags |= O_NONBLOCK;
	else
		pd_flags &= (~O_NONBLOCK);
	return fcntl(hSocket, F_SETFL, pd_flags) == 0;
#else
	return ioctlsocket(hSocket, FIONBIO, (u_long*)&bIsNonBlocking) == 0;
#endif
}

UBOOL CavaSocket::SetBroadcast(UBOOL bAllowBroadcast)
{
	return SetSockOpt(SO_BROADCAST, &bAllowBroadcast, sizeof(UBOOL));
}

UBOOL CavaSocket::SetReuseAddr(UBOOL bAllowReuse)
{
	return SetSockOpt(SO_REUSEADDR, &bAllowReuse, sizeof(UBOOL));
}

UBOOL CavaSocket::SetLinger(UBOOL bShouldLinger, INT timeout)
{
	LINGER lin;
	lin.l_onoff = bShouldLinger;
	lin.l_linger = timeout;

	return SetSockOpt(SO_LINGER, (const INT*)&lin, sizeof(lin));
}

