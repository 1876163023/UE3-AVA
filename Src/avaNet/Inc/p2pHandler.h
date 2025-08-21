/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaNet

	Name: p2pHandler.h

	Description: Implements p2p connection handler -- declared in IpDrv project -- to support hole-punching mechanism.

***/
#ifndef _P2PHANDLER_H_
#define _P2PHANDLER_H_



class CavaP2PHandler : public IP2PHandler
{
public:
	virtual void OnInitListen(FSocket *Socket);
	virtual void OnInitConnect(FSocket *Socket, FURL &ConnectURL);
	virtual void OnTick();
	virtual bool OnRecvFrom(char* Data, INT BytesRead, FInternetIpAddr &RecvFrom);
};


#endif
