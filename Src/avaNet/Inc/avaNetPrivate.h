/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaNet

	Name: avaNetPrivate.h

	Description: Local declarations

***/
#ifndef __AVANETPRIVATE_H__
#define __AVANETPRIVATE_H__



#define EMERGENCY_CHECK_PERIOD		60.0
#define LOADING_CHECK_TIME			90

#define DEFAULT_PLAYERS_PER_CHANNEL 1000

#define CFG_SECTION					TEXT("avaNet")

#define CFG_TIMEOUTSEC				TEXT("TimeOutSec")

#define CFG_SERVERLIST				TEXT("ServerList")
#define CFG_P2PSERVERLIST			TEXT("P2PServerList")
#define CFG_RTTTSERVERLIST			TEXT("RtttServerList")
#define CFG_CMSADDRESS				TEXT("CMSAddress")
#define CFG_NEWSERVERADDRESS		TEXT("NewServerAddress")
#define CFG_CMSADDRESS				TEXT("CMSAddress")
#define CFG_P2PLOCALPORT			TEXT("P2PLocalPort")

#define CFG_LASTCHANNEL				TEXT("LastChannel")
#define CFG_PLAYERS_PER_CHANNEL		TEXT("PlayersPerChannel")

#define CFG_HOSTADDRESS				TEXT("HostAddress")

#define CFG_RTTRECHECKTHRESHOLD		TEXT("RttRecheckThreshold")
#define CFG_RTTRECHECKTIME			TEXT("RttRecheckTime")

#define CFG_USERSN					TEXT("UserSN")
#define CFG_USERID					TEXT("UserID")
#define CFG_USERPASSWORD			TEXT("UserPassword")

#define CFG_KEYSTRING				TEXT("KeyString")

#define CFG_SHOPDESC				TEXT("ShopDescFile")
#define CFG_ITEMDESC				TEXT("ItemDescFile")
#define CFG_SLOTDESC				TEXT("SlotDescFile")
#define CFG_W1						TEXT("W1")
#define CFG_W1_IGNORE				TEXT("W1_Ignore")
#define CFG_W2						TEXT("W2")
#define CFG_W2_IGNORE				TEXT("W2_Ignore")
#define CFG_ROOMNAME				TEXT("RoomNameFile")
#define CFG_TICKERMSG				TEXT("TickerMsgFile")
#define CFG_AWARDDESC				TEXT("AwardDescFile")
#define CFG_SKILLDESC				TEXT("SkillDescFile")

#define CFG_PACKETPROFILE			TEXT("EnablePacketProfile")
#define CFG_PACKETPROFILEPATH		TEXT("PacketProfilePath")

#define CFG_BTTESTPROFILE			TEXT("EnableBTTestProfile")
#define CFG_BTTESTPROFILEPATH		TEXT("BTTestProfilePath")

#define CFG_ADDRLIST_SEP			TEXT(",")
#define CFG_ADDRPORT_SEP			TEXT(":")



#define CREATE_RXADDRESS(_A, _C)				\
	RxGate::RXNERVE_ADDRESS _A;					\
	_A.addrType = 0;							\
	WideCharToMultiByte(CP_ACP, 0, *((_C).GetRxAddress()), -1, (LPSTR)_A.address, RXNERVE_ADDRESS_SIZE, NULL, NULL);

inline void CreateCHMAddress(RxGate::RXNERVE_ADDRESS &Addr, WORD idChannel)
{
	Addr.addrType = 0;
	char addrbuf[RXNERVE_ADDRESS_SIZE + 1];
	appMemzero(addrbuf, RXNERVE_ADDRESS_SIZE + 1);
	sprintf(addrbuf, "CHM%d", idChannel);
	appMemcpy(Addr.address, addrbuf, RXNERVE_ADDRESS_SIZE);
}

inline void CreateGCSAddress(RxGate::RXNERVE_ADDRESS &Addr, WORD idChannel)
{
	Addr.addrType = 0;
	char addrbuf[RXNERVE_ADDRESS_SIZE + 1];
	appMemzero(addrbuf, RXNERVE_ADDRESS_SIZE + 1);
	sprintf(addrbuf, "GCS%d", idChannel);
	appMemcpy(Addr.address, addrbuf, RXNERVE_ADDRESS_SIZE);
}



#define BREAK_SECTION_BEGIN() do
#define BREAK_SECTION_END() while (0);


template <class T>
class SelfPtrT
{
private:
	SelfPtrT(const SelfPtrT&);
	SelfPtrT& operator=(const SelfPtrT&);

public:
	typedef T _PtrClass;

public:
	SelfPtrT(T* pt = NULL)
	{	
		ptr = pt;
	}
	~SelfPtrT()
	{
		if (ptr)
		{
			ptr->Delete();
			ptr = NULL;
		}
	}
public:
	operator T*() const
	{
		return (T*)ptr;
	}
	T& operator*() const
	{
		return *ptr;
	}
	T* operator->() const
	{
		return (T*)ptr;
	}
	T* operator=(T* p)
	{
		if (ptr == p)	return ptr;
		if (ptr)	
		{
			ptr->Delete();
			ptr = NULL;
		}
		ptr = p;
		return p;
	}
	operator bool() const
	{
		return ptr != NULL;
	}
	bool operator!() const
	{
		return (!ptr);
	}
	bool operator<(T* pt) const
	{
		return (ptr < pt);
	}
	bool operator==(T* pt) const
	{
		return (pt == ptr);
	}
	void Attach(T* p)
	{
		ptr = p;
	}
	T* Detach()
	{
		T* p = ptr;
		ptr = NULL;
		return p;
	}

public:
	T* ptr;
};


typedef SelfPtrT<CMsgBuf> ScopedMsgBufPtr;


//#define _HAPPY_NEW_YEAR_EVENT
#define _GAME_ITEM_PROMOTION


#endif
