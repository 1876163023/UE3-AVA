#pragma once


#include "RxGateTranslator/RxMsg.h"
#include "ComDef/Def.h"


#define ODLMsg			RxGate::IRxMsg
#define ODLStream		RxGate::IRxStream
#define ODLWriter		RxGate::IRxWriter
#define ODLID			RxGate::RXMSGID
#define ODLBOOL			BOOL
#define LPODLCTSTR		LPCTSTR
#define ODLTString		FString
#define TRXID_ACCOUNT	DWORD
#define TRXID_SERVER	WORD
#define TRXID_CHANNEL	DWORD
#define TRXID_CLAN		DWORD
#define ODLTRUE			TRUE
#define ODLFALSE		FALSE


namespace RXCLIENTCMSPROTOCOL
{
	typedef WORD ODL_STRLEN;

	inline WORD GetOdlStrLen(const ODLTString &Str) { return sizeof(ODL_STRLEN) + Str.Len() * sizeof(TCHAR); }

	struct LOCATIONINFO : public ODLMsg
	{
		//Member Variable Definition
		DWORD			clanid;
		WORD			channelid;
		WORD			servertype;

		//Member Function Definition
		virtual ODLBOOL Read(ODLStream *pStream);
		virtual ODLID	GetID() const { return 0; }
		virtual ODLBOOL Write(ODLStream *pStream) const;
		virtual void Dump(ODLWriter *pWriter) const;
		virtual LPODLCTSTR GetName() const { return _T("LOCATIONINFO");}
		virtual LPODLCTSTR GetDescription() const { return _T("");}
		virtual WORD	GetBodyLength() const { return sizeof(DWORD) + sizeof(WORD) + sizeof(WORD); }
	};

	struct _BUDDYINFO : public ODLMsg
	{
		//Member Variable Definition
		BYTE	friendtype;
		TRXID_ACCOUNT	userid;
		BYTE	level;
		ODLTString	beforenickname;
		ODLTString	afternickname;
		BYTE	bOnlineState;
		ODLTString	clanname;
		LOCATIONINFO	loinfo;

		//Member Function Definition
		virtual ODLBOOL Read(ODLStream *pStream);
		virtual ODLID	GetID() const { return 0; }
		virtual ODLBOOL Write(ODLStream *pStream) const;
		virtual void Dump(ODLWriter *pWriter) const;
		virtual LPODLCTSTR GetName() const { return _T("_BUDDYINFO");}
		virtual LPODLCTSTR GetDescription() const { return _T("");}
		virtual WORD	GetBodyLength() const { return sizeof(BYTE) + sizeof(TRXID_ACCOUNT) + sizeof(BYTE) + GetOdlStrLen(beforenickname) + GetOdlStrLen(afternickname) +
														sizeof(BYTE) + GetOdlStrLen(clanname) + loinfo.GetBodyLength(); }
	};
	typedef TArray<_BUDDYINFO> _BUDDYLISTVEC;

	struct _PAIRBUDDYINFO : public ODLMsg
	{
		//Member Variable Definition
		BYTE	ftype;
		BYTE	level;
		TRXID_ACCOUNT	userid;
		ODLTString		nickname;
		LOCATIONINFO	loinfo;

		//Member Function Definition
		virtual ODLBOOL Read(ODLStream *pStream);
		virtual ODLID	GetID() const { return 0; }
		virtual ODLBOOL Write(ODLStream *pStream) const;
		virtual void Dump(ODLWriter *pWriter) const;
		virtual LPODLCTSTR GetName() const { return _T("_PAIRBUDDYINFO");}
		virtual LPODLCTSTR GetDescription() const { return _T("");}
		virtual WORD	GetBodyLength() const { return sizeof(BYTE) + sizeof(BYTE) + sizeof(TRXID_ACCOUNT) + GetOdlStrLen(nickname) + loinfo.GetBodyLength(); }
	};
	typedef TArray<_PAIRBUDDYINFO> _PAIRBUDDYLISTVEC;

	enum DAT_RXCLIENTCMSPROTOCOL
	{
		DID_MsgAddBuddyReq = 100, //DAT_MsgAddBuddyReq's ID
		DID_MsgAddBuddyAns = 101, //DAT_MsgAddBuddyAns's ID
		DID_MsgDeleteBuddyNtf = 102, //DAT_MsgDeleteBuddyNtf's ID
		DID_MsgGameInviteNtf = 103, //DAT_MsgGameInviteNtf's ID
		DID_MsgClanInviteReq = 104, //DAT_MsgClanInviteReq's ID
		DID_MsgClanInviteAns = 105, //DAT_MsgClanInviteAns's ID
		DID_MsgClanJoinNtf = 106, //DAT_MsgClanJoinNtf's ID
		DID_MsgPrivateChatData = 107, //DAT_MsgPrivateChatData's ID
		DID_MsgLocationInfoReq = 108, //DAT_MsgLocationInfoReq's ID
		DID_MsgLocationInfoAns = 109, //DAT_MsgLocationInfoAns's ID
		DID_MsgBuddyInfoReq = 110, //DAT_MsgBuddyInfoReq's ID
		DID_MsgBuddyInfoAns = 111, //DAT_MsgBuddyInfoAns's ID
		DID_MsgBuddyInfoEndNtf = 112, //DAT_MsgBuddyInfoEndNtf's ID
		DID_MsgBuddyAlarmNtf = 113, //DAT_MsgBuddyAlarmNtf's ID
		DID_MsgPairBuddyinfoReq = 114, //DAT_MsgPairBuddyInfoReq's ID
		DID_MsgPairBuddyinfoAns = 115, //DAT_MsgPairBuddyInfoAns's ID
		DID_MsgUpdateNickNameNtf = 116, //DAT_MsgUpdateNickNameNtf's ID
		DID_MsgPCBUserStartNtf = 117, //DAT_MsgPCBUserStartNtf's ID
		DID_MsgPCBUserTimeExpireNtf = 118, //DAT_MsgPCBUserTimeExpireNtf's ID
		DID_MsgPCBRemainTimeReq = 119, //DAT_MsgPCBRemainTimeReq's ID
		DID_MsgPCBRemainTimeAns = 120, //DAT_MsgPCBRemainTimeAns's ID
		DID_MsgPCBRemainTimeNtf = 121, //DAT_MsgPCBRemainTimeNtf's ID
	};

	struct DAT_MsgAddBuddyReq : public ODLMsg
	{
		//Data ID Definition
		enum { DID = DID_MsgAddBuddyReq};

		//Member Variable Definition
		WORD	BuddyType;
		TRXID_ACCOUNT	FromidAccount;
		TRXID_ACCOUNT	ToidAccount;
		ODLTString	FromNick;


		//Member Function Definition
		virtual ODLBOOL Read(ODLStream *pStream);
		virtual ODLID	GetID() const { return DID; }
		virtual ODLBOOL Write(ODLStream *pStream) const;
		virtual void Dump(ODLWriter* pWriter) const;
		virtual LPODLCTSTR GetName() const { return _T("MsgAddBuddyReq");}
		virtual LPODLCTSTR GetDescription() const { return _T("");}
		virtual WORD	GetBodyLength() const { return sizeof(WORD) + sizeof(TRXID_ACCOUNT) + sizeof(TRXID_ACCOUNT) + GetOdlStrLen(FromNick); }
	};

	struct DAT_MsgAddBuddyAns : public ODLMsg
	{
		//Data ID Definition
		enum { DID = DID_MsgAddBuddyAns};

		//Member Variable Definition
		DWORD	errCode;
		TRXID_ACCOUNT	FromidAccount;
		TRXID_ACCOUNT	ToidAccount;
		ODLTString	FromNick;


		//Member Function Definition
		virtual ODLBOOL Read(ODLStream *pStream);
		virtual ODLID	GetID() const { return DID; }
		virtual ODLBOOL Write(ODLStream *pStream) const;
		virtual void Dump(ODLWriter* pWriter) const;
		virtual LPODLCTSTR GetName() const { return _T("MsgAddBuddyAns");}
		virtual LPODLCTSTR GetDescription() const { return _T("");}
		virtual WORD	GetBodyLength() const { return sizeof(DWORD) + sizeof(TRXID_ACCOUNT) + sizeof(TRXID_ACCOUNT) + GetOdlStrLen(FromNick); }
	};

	struct DAT_MsgDeleteBuddyNtf : public ODLMsg
	{
		//Data ID Definition
		enum { DID = DID_MsgDeleteBuddyNtf};

		//Member Variable Definition
		TRXID_ACCOUNT	sourceidAccount;
		TRXID_ACCOUNT	destidAccount;


		//Member Function Definition
		virtual ODLBOOL Read(ODLStream *pStream);
		virtual ODLID	GetID() const { return DID; }
		virtual ODLBOOL Write(ODLStream *pStream) const;
		virtual void Dump(ODLWriter* pWriter) const;
		virtual LPODLCTSTR GetName() const { return _T("MsgDeleteBuddyNtf");}
		virtual LPODLCTSTR GetDescription() const { return _T("");}
		virtual WORD	GetBodyLength() const { return sizeof(TRXID_ACCOUNT) + sizeof(TRXID_ACCOUNT); }
	};

	struct DAT_MsgGameInviteNtf : public ODLMsg
	{
		//Data ID Definition
		enum { DID = DID_MsgGameInviteNtf};

		//Member Variable Definition
		TRXID_ACCOUNT	FromidAccount;
		ODLTString	Fromnickname;
		TRXID_ACCOUNT	ToidAccount;
		LOCATIONINFO	Fromlocationinfo;
		ODLTString		roomname;
		BYTE			roomid;


		//Member Function Definition
		virtual ODLBOOL Read(ODLStream *pStream);
		virtual ODLID	GetID() const { return DID; }
		virtual ODLBOOL Write(ODLStream *pStream) const;
		virtual void Dump(ODLWriter* pWriter) const;
		virtual LPODLCTSTR GetName() const { return _T("MsgGameInviteNtf");}
		virtual LPODLCTSTR GetDescription() const { return _T("");}
		virtual WORD	GetBodyLength() const { return sizeof(TRXID_ACCOUNT) + GetOdlStrLen(Fromnickname) + sizeof(TRXID_ACCOUNT) + Fromlocationinfo.GetBodyLength() +
														GetOdlStrLen(roomname) + sizeof(BYTE) + sizeof(TRXID_CHANNEL); }
	};

	struct DAT_MsgClanInviteReq : public ODLMsg
	{
	//Data ID Definition
		enum { DID = DID_MsgClanInviteReq};

	//Member Variable Definition
		TRXID_ACCOUNT	FromidAccount;
		ODLTString	Fromnickname;
		TRXID_ACCOUNT	ToidAccount;
		TRXID_CLAN	clanid;
		ODLTString	clanname;
	

		//Member Function Definition
		virtual ODLBOOL Read(ODLStream *pStream);
		virtual ODLID	GetID() const { return DID; }
		virtual ODLBOOL Write(ODLStream *pStream) const;
		virtual void Dump(ODLWriter* pWriter) const;
		virtual LPODLCTSTR GetName() const { return _T("MsgClanInviteReq");}
		virtual LPODLCTSTR GetDescription() const { return _T("");}
		virtual WORD	GetBodyLength() const { return sizeof(TRXID_ACCOUNT) + GetOdlStrLen(Fromnickname) + sizeof(TRXID_ACCOUNT) + sizeof(TRXID_CLAN) + GetOdlStrLen(clanname); }
	};

	struct DAT_MsgClanInviteAns : public ODLMsg
	{
	//Data ID Definition
		enum { DID = DID_MsgClanInviteAns};

	//Member Variable Definition
		DWORD	errCode;
		TRXID_ACCOUNT	FromidAccount;
		TRXID_ACCOUNT	ToidAccount;
	

		//Member Function Definition
		virtual ODLBOOL Read(ODLStream *pStream);
		virtual ODLID	GetID() const { return DID; }
		virtual ODLBOOL Write(ODLStream *pStream) const;
		virtual void Dump(ODLWriter* pWriter) const;
		virtual LPODLCTSTR GetName() const { return _T("MsgClanInviteAns");}
		virtual LPODLCTSTR GetDescription() const { return _T("");}
		virtual WORD	GetBodyLength() const { return sizeof(DWORD) + sizeof(TRXID_ACCOUNT) + sizeof(TRXID_ACCOUNT); }
	};

	struct DAT_MsgClanJoinNtf : public ODLMsg
	{
	//Data ID Definition
		enum { DID = DID_MsgClanJoinNtf};

	//Member Variable Definition
		TRXID_ACCOUNT	ToidAccount;
		RxGate::RXNERVE_ADDRESS gcsaddr;
		TRXID_CLAN		clanid;
	

		//Member Function Definition
		virtual ODLBOOL Read(ODLStream *pStream);
		virtual ODLID	GetID() const { return DID; }
		virtual ODLBOOL Write(ODLStream *pStream) const;
		virtual void Dump(ODLWriter* pWriter) const;
		virtual LPODLCTSTR GetName() const { return _T("MsgClanJoinNtf");}
		virtual LPODLCTSTR GetDescription() const { return _T("");}
		virtual WORD	GetBodyLength() const { return sizeof(TRXID_ACCOUNT) + sizeof(RxGate::RXNERVE_ADDRESS) + sizeof(TRXID_CLAN); }
	};

	struct DAT_MsgPrivateChatData : public ODLMsg
	{
		//Data ID Definition
		enum { DID = DID_MsgPrivateChatData};

		//Member Variable Definition
		TRXID_ACCOUNT	FromidAccount;
		ODLTString	Fromnickname;
		TRXID_ACCOUNT	ToidAccount;
		ODLTString	chatdata;


		//Member Function Definition
		virtual ODLBOOL Read(ODLStream *pStream);
		virtual ODLID	GetID() const { return DID; }
		virtual ODLBOOL Write(ODLStream *pStream) const;
		virtual void Dump(ODLWriter* pWriter) const;
		virtual LPODLCTSTR GetName() const { return _T("MsgPrivateChatData");}
		virtual LPODLCTSTR GetDescription() const { return _T("");}
		virtual WORD	GetBodyLength() const { return sizeof(TRXID_ACCOUNT) + GetOdlStrLen(Fromnickname) + sizeof(TRXID_ACCOUNT) + GetOdlStrLen(chatdata); }
	};

	struct DAT_MsgLocationInfoReq : public ODLMsg
	{
		//Data ID Definition
		enum { DID = DID_MsgLocationInfoReq};

		//Member Variable Definition
		TRXID_ACCOUNT	srcidAccount;
		TRXID_ACCOUNT	destidAccount;


		//Member Function Definition
		virtual ODLBOOL Read(ODLStream *pStream);
		virtual ODLID	GetID() const { return DID; }
		virtual ODLBOOL Write(ODLStream *pStream) const;
		virtual void Dump(ODLWriter* pWriter) const;
		virtual LPODLCTSTR GetName() const { return _T("MsgLocationInfoReq");}
		virtual LPODLCTSTR GetDescription() const { return _T("");}
		virtual WORD	GetBodyLength() const { return sizeof(srcidAccount) + sizeof(destidAccount); }
	};

	struct DAT_MsgLocationInfoAns : public ODLMsg
	{
		//Data ID Definition
		enum { DID = DID_MsgLocationInfoAns};

		//Member Variable Definition
		DWORD	errCode;
		TRXID_ACCOUNT	idAccount;
		LOCATIONINFO	loinfo;


		//Member Function Definition
		virtual ODLBOOL Read(ODLStream *pStream);
		virtual ODLID	GetID() const { return DID; }
		virtual ODLBOOL Write(ODLStream *pStream) const;
		virtual void Dump(ODLWriter* pWriter) const;
		virtual LPODLCTSTR GetName() const { return _T("MsgLocationInfoAns");}
		virtual LPODLCTSTR GetDescription() const { return _T("");}
		virtual WORD	GetBodyLength() const { return sizeof(DWORD) + sizeof(TRXID_ACCOUNT) + loinfo.GetBodyLength(); }
	};

	struct DAT_MsgBuddyInfoReq : public ODLMsg
	{
		//Data ID Definition
		enum { DID = DID_MsgBuddyInfoReq};

		//Member Variable Definition
		ODLBOOL	bconnectAlram;
		TRXID_ACCOUNT	idAccount;


		//Member Function Definition
		virtual ODLBOOL Read(ODLStream *pStream);
		virtual ODLID	GetID() const { return DID; }
		virtual ODLBOOL Write(ODLStream *pStream) const;
		virtual void Dump(ODLWriter* pWriter) const;
		virtual LPODLCTSTR GetName() const { return _T("MsgBuddyInfoReq");}
		virtual LPODLCTSTR GetDescription() const { return _T("");}
		virtual WORD	GetBodyLength() const { return sizeof(ODLBOOL) + sizeof(TRXID_ACCOUNT); }
	};

	struct DAT_MsgBuddyInfoAns : public ODLMsg
	{
		//Data ID Definition
		enum { DID = DID_MsgBuddyInfoAns};

		//Member Variable Definition
		UINT	count;
		_BUDDYLISTVEC	ava_friend;


		//Member Function Definition
		virtual ODLBOOL Read(ODLStream *pStream);
		virtual ODLID	GetID() const { return DID; }
		virtual ODLBOOL Write(ODLStream *pStream) const;
		virtual void Dump(ODLWriter* pWriter) const;
		virtual LPODLCTSTR GetName() const { return _T("MsgBuddyInfoAns");}
		virtual LPODLCTSTR GetDescription() const { return _T("");}
		virtual WORD	GetBodyLength() const { return 0; }
	};

	struct DAT_MsgBuddyInfoEndNtf : public ODLMsg
	{
		//Data ID Definition
		enum { DID = DID_MsgBuddyInfoEndNtf};

		//Member Variable Definition
		TRXID_ACCOUNT	idAccount;


		//Member Function Definition
		virtual ODLBOOL Read(ODLStream *pStream);
		virtual ODLID	GetID() const { return DID; }
		virtual ODLBOOL Write(ODLStream *pStream) const;
		virtual void Dump(ODLWriter* pWriter) const;
		virtual LPODLCTSTR GetName() const { return _T("MsgBuddyInfoEndNtf");}
		virtual LPODLCTSTR GetDescription() const { return _T("");}
		virtual WORD	GetBodyLength() const { return sizeof(TRXID_ACCOUNT); }
	};

	struct DAT_MsgBuddyAlarmNtf : public ODLMsg
	{
		//Data ID Definition
		enum { DID = DID_MsgBuddyAlarmNtf};

		//Member Variable Definition
		_BUDDYINFO			buddyinfo;


		//Member Function Definition
		virtual ODLBOOL Read(ODLStream *pStream);
		virtual ODLID	GetID() const { return DID; }
		virtual ODLBOOL Write(ODLStream *pStream) const;
		virtual void Dump(ODLWriter* pWriter) const;
		virtual LPODLCTSTR GetName() const { return _T("MsgBuddyAlarmNtf");}
		virtual LPODLCTSTR GetDescription() const { return _T("");}
		virtual WORD	GetBodyLength() const { return buddyinfo.GetBodyLength(); }
	};

	struct DAT_MsgPairBuddyinfoReq : public ODLMsg
	{
		//Data ID Definition
		enum { DID = DID_MsgPairBuddyinfoReq};

		//Member Variable Definition
		TRXID_ACCOUNT		idAccount;


		//Member Function Definition
		virtual ODLBOOL Read(ODLStream *pStream);
		virtual ODLID	GetID() const { return DID; }
		virtual ODLBOOL Write(ODLStream *pStream) const;
		virtual void Dump(ODLWriter* pWriter) const;
		virtual LPODLCTSTR GetName() const { return _T("MsgPairBuddyinfoReq");}
		virtual LPODLCTSTR GetDescription() const { return _T("");}
		virtual WORD	GetBodyLength() const { return sizeof(idAccount); }
	};

	struct DAT_MsgPairBuddyinfoAns : public ODLMsg
	{
		//Data ID Definition
		enum { DID = DID_MsgPairBuddyinfoAns};

		//Member Variable Definition
		BYTE				err;
		BYTE				cnt;
		_PAIRBUDDYLISTVEC	pairbuddyinfo;


		//Member Function Definition
		virtual ODLBOOL Read(ODLStream *pStream);
		virtual ODLID	GetID() const { return DID; }
		virtual ODLBOOL Write(ODLStream *pStream) const;
		virtual void Dump(ODLWriter* pWriter) const;
		virtual LPODLCTSTR GetName() const { return _T("MsgPairBuddyinfoAns");}
		virtual LPODLCTSTR GetDescription() const { return _T("");}
		virtual WORD	GetBodyLength() const { return 0; }
	};

	struct DAT_MsgUpdateNickNameNtf : public ODLMsg
	{
	//Data ID Definition
		enum { DID = DID_MsgUpdateNickNameNtf};

	//Member Variable Definition
		TRXID_ACCOUNT	idAccount;
		ODLTString	nickname;
	

		//Member Function Definition
		virtual ODLBOOL Read(ODLStream *pStream);
		virtual ODLID	GetID() const { return DID; }
		virtual ODLBOOL Write(ODLStream *pStream) const;
		virtual void Dump(ODLWriter* pWriter) const;
		virtual LPODLCTSTR GetName() const { return _T("MsgUpdateNickNameNtf");}
		virtual LPODLCTSTR GetDescription() const { return _T("");}
		virtual WORD	GetBodyLength() const { return sizeof(idAccount) + GetOdlStrLen(nickname); }
	};

	struct DAT_MsgPCBUserStartNtf : public ODLMsg
	{
	//Data ID Definition
		enum { DID = DID_MsgPCBUserStartNtf};

	//Member Variable Definition
		DWORD			service_type;
		DWORD			IP;
		TRXID_ACCOUNT	idAccount;
		ODLTString		pmangID;
	

		//Member Function Definition
		virtual ODLBOOL Read(ODLStream *pStream);
		virtual ODLID	GetID() const { return DID; }
		virtual ODLBOOL Write(ODLStream *pStream) const;
		virtual void Dump(ODLWriter* pWriter) const;
		virtual LPODLCTSTR GetName() const { return _T("MsgPCBUserStartNtf");}
		virtual LPODLCTSTR GetDescription() const { return _T("");}
		virtual WORD	GetBodyLength() const { return sizeof(service_type) + sizeof(IP) + sizeof(idAccount) + GetOdlStrLen(pmangID); }
	};

	struct DAT_MsgPCBUserTimeExpireNtf : public ODLMsg
	{
	//Data ID Definition
		enum { DID = DID_MsgPCBUserTimeExpireNtf};

	//Member Variable Definition
		BYTE			ret;	// 0: OK, 1: Expire
	

		//Member Function Definition
		virtual ODLBOOL Read(ODLStream *pStream);
		virtual ODLID	GetID() const { return DID; }
		virtual ODLBOOL Write(ODLStream *pStream) const;
		virtual void Dump(ODLWriter* pWriter) const;
		virtual LPODLCTSTR GetName() const { return _T("MsgPCBUserTimeExpireNtf");}
		virtual LPODLCTSTR GetDescription() const { return _T("");}
		virtual WORD	GetBodyLength() const { return sizeof(ret); }
	};

	struct DAT_MsgPCBRemainTimeReq : public ODLMsg
	{
	//Data ID Definition
		enum { DID = DID_MsgPCBRemainTimeReq};

	//Member Variable Definition
		TRXID_ACCOUNT	idAccount;
		DWORD	IP;
	

		//Member Function Definition
		virtual ODLBOOL Read(ODLStream *pStream);
		virtual ODLID	GetID() const { return DID; }
		virtual ODLBOOL Write(ODLStream *pStream) const;
		virtual void Dump(ODLWriter* pWriter) const;
		virtual LPODLCTSTR GetName() const { return _T("MsgPCBRemainTimeReq");}
		virtual LPODLCTSTR GetDescription() const { return _T("");}
		virtual WORD	GetBodyLength() const { return sizeof(idAccount) + sizeof(IP); }
	};

	struct DAT_MsgPCBRemainTimeAns : public ODLMsg
	{
	//Data ID Definition
		enum { DID = DID_MsgPCBRemainTimeAns};

	//Member Variable Definition
		ODLTString	strtime;
	

		//Member Function Definition
		virtual ODLBOOL Read(ODLStream *pStream);
		virtual ODLID	GetID() const { return DID; }
		virtual ODLBOOL Write(ODLStream *pStream) const;
		virtual void Dump(ODLWriter* pWriter) const;
		virtual LPODLCTSTR GetName() const { return _T("MsgPCBRemainTimeAns");}
		virtual LPODLCTSTR GetDescription() const { return _T("");}
		virtual WORD	GetBodyLength() const { return GetOdlStrLen(strtime); }
	};

	struct DAT_MsgPCBRemainTimeNtf : public ODLMsg
	{
	//Data ID Definition
		enum { DID = DID_MsgPCBRemainTimeNtf};

	//Member Variable Definition
		ODLTString	strtime;
	

		//Member Function Definition
		virtual ODLBOOL Read(ODLStream *pStream);
		virtual ODLID	GetID() const { return DID; }
		virtual ODLBOOL Write(ODLStream *pStream) const;
		virtual void Dump(ODLWriter* pWriter) const;
		virtual LPODLCTSTR GetName() const { return _T("MsgPCBRemainTimeNtf");}
		virtual LPODLCTSTR GetDescription() const { return _T("");}
		virtual WORD	GetBodyLength() const { return GetOdlStrLen(strtime); }
	};
}
