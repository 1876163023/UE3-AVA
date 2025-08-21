#include "avaNet.h"
#include "CMSMessage.h"
#include "RxGateTranslator/Stream.h"
#include "RxGateTranslator/Dump.h"

#pragma warning(disable:4100)



namespace RXCLIENTCMSPROTOCOL
{
	inline BOOL OdlRead(FString &dat, ODLStream *pStream)
	{
		ODL_STRLEN cnt;
		if ( !pStream->Read(cnt) )
			return FALSE;
		TCHAR t;
		for (int i = 0; i < cnt; ++i)
		{
			if ( !pStream->Read(&t, sizeof(TCHAR)) )
				return FALSE;
			dat += t;
		}
		return TRUE;
	}
	inline BOOL OdlWrite(const FString &dat, ODLStream *pStream)
	{
		ODL_STRLEN cnt = dat.Len();
		if ( !pStream->Write(cnt) )
			return FALSE;
		TCHAR *t = (TCHAR*)*dat;
		for (int i = 0; i < cnt; ++i)
		{
			if ( !pStream->Write(&t[i], sizeof(TCHAR)) )
				return FALSE;
		}
		return TRUE;
	}
	inline BOOL OdlDump(const FString &dat, ODLWriter *pWriter)
	{
		pWriter->Print(_T(" value : { %s }"), *dat);
		return TRUE;
	}

	//inline BOOL OdlRead(_LPMSGBUF dat, ODLStream *pStream)
	//{
	//	ODL_STRLEN cnt;
	//	if ( !pStream->Read(cnt) )
	//		return FALSE;
	//	if (cnt > 0)
	//	{
	//		dat->SetLength(cnt);
	//		if ( !pStream->Read(dat->GetData(), cnt) )
	//			return FALSE;
	//	}
	//	return TRUE;
	//}
	//inline BOOL OdlWrite(const _LPMSGBUF dat, ODLStream *pStream)
	//{
	//	ODL_STRLEN cnt = dat->GetLength();
	//	if ( !pStream->Write(cnt) )
	//		return FALSE;
	//	if (cnt > 0)
	//	{
	//		if ( !pStream->Write(dat->GetData(), cnt) )
	//			return FALSE;
	//	}
	//	return TRUE;
	//}
	//inline BOOL OdlDump(const _LPMSGBUF dat, ODLWriter *pWriter)
	//{
	//	pWriter->Print(_T(" value : { %p }"), dat->GetData());
	//	return TRUE;
	//}

	//template<typename T> inline BOOL OdlRead(TArray<T> &dat, ODLStream *pStream)
	//{
	//	ODL_STRLEN cnt;
	//	if ( !pStream->Read(cnt) )
	//		return FALSE;
	//	T t;
	//	for (int i = 0; i < cnt; ++i)
	//	{
	//		if ( !pStream->Read(&t, sizeof(T)) )
	//			return FALSE;
	//		dat.Push(t);
	//	}
	//	return TRUE;
	//}
	//template<typename T> inline BOOL OdlWrite(const TArray<T> &dat, ODLStream *pStream)
	//{
	//	ODL_STRLEN cnt = dat.Num();
	//	if ( !pStream->Write(cnt) )
	//		return FALSE;
	//	for (int i = 0; i < cnt; ++i)
	//	{
	//		if ( !pStream->Write(&dat(i), sizeof(T)) )
	//			return FALSE;
	//	}
	//	return TRUE;
	//}
	//template<typename T> inline BOOL OdlDump(const TArray<T> &dat, ODLWriter *pWriter)
	//{
	//	pWriter->Print(_T(" { Array[%d] }"), dat.Num());
	//	return TRUE;
	//}

	inline BOOL OdlRead(_BUDDYLISTVEC &dat, ODLStream *pStream)
	{
		ODL_STRLEN cnt;
		if ( !pStream->Read(cnt) )
			return FALSE;
		for (int i = 0; i < cnt; ++i)
		{
			_BUDDYINFO t;
			if ( !t.Read(pStream) )
				return FALSE;
			dat.Push(t);
		}
		return TRUE;
	}

	inline BOOL OdlWrite(const _BUDDYLISTVEC &dat, ODLStream *pStream)
	{
		ODL_STRLEN cnt = dat.Num();
		if ( !pStream->Write(cnt) )
			return FALSE;
		for (int i = 0; i < cnt; ++i)
		{
			if ( !dat(i).Write(pStream) )
				return FALSE;
		}
		return TRUE;
	}

	inline BOOL OdlDump(const _BUDDYLISTVEC &dat, ODLWriter *pWriter)
	{
		pWriter->Print(_T(" {"));
		for (int i = 0; i < dat.Num(); ++i)
		{
			dat(i).Dump(pWriter);
			pWriter->Print(_T(" "));
		}
		pWriter->Print(_T("}"));
		return TRUE;
	}

	inline BOOL OdlRead(_PAIRBUDDYLISTVEC &dat, ODLStream *pStream)
	{
		ODL_STRLEN cnt;
		if ( !pStream->Read(cnt) )
			return FALSE;
		for (int i = 0; i < cnt; ++i)
		{
			_PAIRBUDDYINFO t;
			if ( !t.Read(pStream) )
				return FALSE;
			dat.Push(t);
		}
		return TRUE;
	}

	inline BOOL OdlWrite(const _PAIRBUDDYLISTVEC &dat, ODLStream *pStream)
	{
		ODL_STRLEN cnt = dat.Num();
		if ( !pStream->Write(cnt) )
			return FALSE;
		for (int i = 0; i < cnt; ++i)
		{
			if ( !dat(i).Write(pStream) )
				return FALSE;
		}
		return TRUE;
	}

	inline BOOL OdlDump(const _PAIRBUDDYLISTVEC &dat, ODLWriter *pWriter)
	{
		pWriter->Print(_T(" {"));
		for (int i = 0; i < dat.Num(); ++i)
		{
			dat(i).Dump(pWriter);
			pWriter->Print(_T(" "));
		}
		pWriter->Print(_T("}"));
		return TRUE;
	}


	ODLBOOL LOCATIONINFO::Read(ODLStream *pStream)
	{
		if(!OdlRead(clanid,pStream)) return ODLFALSE; 
		if(!OdlRead(channelid,pStream)) return ODLFALSE; 
		if(!OdlRead(servertype,pStream)) return ODLFALSE; 
		return ODLTRUE;
	}
	ODLBOOL LOCATIONINFO::Write(ODLStream *pStream) const
	{
		if(!OdlWrite(clanid, pStream)) return ODLFALSE; 
		if(!OdlWrite(channelid, pStream)) return ODLFALSE; 
		if(!OdlWrite(servertype, pStream)) return ODLFALSE; 
		return ODLTRUE;
	}
	void LOCATIONINFO::Dump(ODLWriter *pWriter) const
	{
		pWriter->Print(_T("<LOCATIONINFO> {"));
		pWriter->Print(_T(" clanid="));
		OdlDump(clanid, pWriter); 
		pWriter->Print(_T(" channelid="));
		OdlDump(channelid, pWriter); 
		pWriter->Print(_T(" servertype="));
		OdlDump(servertype, pWriter); 
		pWriter->Print(_T(" }"));
	}

	ODLBOOL _BUDDYINFO::Read(ODLStream *pStream)
	{
		if(!OdlRead(friendtype,pStream)) return ODLFALSE; 
		if(!OdlRead(userid,pStream)) return ODLFALSE; 
		if(!OdlRead(level,pStream)) return ODLFALSE; 
		if(!OdlRead(beforenickname,pStream)) return ODLFALSE; 
		if(!OdlRead(afternickname,pStream)) return ODLFALSE; 
		if(!OdlRead(bOnlineState,pStream)) return ODLFALSE; 
		if(!OdlRead(clanname,pStream)) return ODLFALSE; 
		if(!OdlRead(loinfo,pStream)) return ODLFALSE; 
		return ODLTRUE;
	}
	ODLBOOL _BUDDYINFO::Write(ODLStream *pStream) const
	{
		if(!OdlWrite(friendtype, pStream)) return ODLFALSE; 
		if(!OdlWrite(userid, pStream)) return ODLFALSE; 
		if(!OdlWrite(level, pStream)) return ODLFALSE; 
		if(!OdlWrite(beforenickname, pStream)) return ODLFALSE; 
		if(!OdlWrite(afternickname, pStream)) return ODLFALSE; 
		if(!OdlWrite(bOnlineState, pStream)) return ODLFALSE; 
		if(!OdlWrite(clanname, pStream)) return ODLFALSE; 
		if(!OdlWrite(loinfo, pStream)) return ODLFALSE; 
		return ODLTRUE;
	}
	void _BUDDYINFO::Dump(ODLWriter *pWriter) const
	{
		pWriter->Print(_T("<_BUDDYINFO> {"));
		pWriter->Print(_T(" friendtype="));
		OdlDump(friendtype, pWriter); 
		pWriter->Print(_T(" userid="));
		OdlDump(userid, pWriter); 
		pWriter->Print(_T(" level="));
		OdlDump(level, pWriter); 
		pWriter->Print(_T(" beforenickname="));
		OdlDump(beforenickname, pWriter); 
		pWriter->Print(_T(" afternickname="));
		OdlDump(afternickname, pWriter); 
		pWriter->Print(_T(" bOnlineState="));
		OdlDump(bOnlineState, pWriter); 
		pWriter->Print(_T(" clanname="));
		OdlDump(clanname, pWriter); 
		pWriter->Print(_T(" loinfo="));
		OdlDump(loinfo, pWriter); 
		pWriter->Print(_T(" }"));
	}


	ODLBOOL _PAIRBUDDYINFO::Read(ODLStream *pStream)
	{
		if(!OdlRead(ftype,pStream)) return ODLFALSE; 
		if(!OdlRead(level,pStream)) return ODLFALSE; 
		if(!OdlRead(userid,pStream)) return ODLFALSE; 
		if(!OdlRead(nickname, pStream)) return ODLFALSE; 
		if(!OdlRead(loinfo,pStream)) return ODLFALSE; 
		return ODLTRUE;
	}
	ODLBOOL _PAIRBUDDYINFO::Write(ODLStream *pStream) const
	{
		if(!OdlWrite(ftype, pStream)) return ODLFALSE; 
		if(!OdlWrite(level, pStream)) return ODLFALSE; 
		if(!OdlWrite(userid, pStream)) return ODLFALSE; 
		if(!OdlWrite(nickname, pStream)) return ODLFALSE; 
		if(!OdlWrite(loinfo, pStream)) return ODLFALSE; 
		return ODLTRUE;
	}
	void _PAIRBUDDYINFO::Dump(ODLWriter *pWriter) const
	{
		pWriter->Print(_T("<_BUDDYINFO> {"));
		pWriter->Print(_T(" ftype="));
		OdlDump(ftype, pWriter); 
		pWriter->Print(_T(" level="));
		OdlDump(level, pWriter); 
		pWriter->Print(_T(" userid="));
		OdlDump(userid, pWriter); 
		pWriter->Print(_T(" nickname="));
		OdlDump(nickname, pWriter); 
		pWriter->Print(_T(" loinfo="));
		OdlDump(loinfo, pWriter); 
		pWriter->Print(_T(" }"));
	}


	ODLBOOL DAT_MsgAddBuddyReq::Read(ODLStream *pStream)
	{
		if(!OdlRead(BuddyType,pStream)) return ODLFALSE; 
		if(!OdlRead(FromidAccount,pStream)) return ODLFALSE; 
		if(!OdlRead(ToidAccount,pStream)) return ODLFALSE; 
		if(!OdlRead(FromNick,pStream)) return ODLFALSE; 

		return ODLTRUE;	
	}
	ODLBOOL DAT_MsgAddBuddyReq::Write(ODLStream *pStream) const
	{
		if(!OdlWrite(BuddyType, pStream)) return ODLFALSE; 
		if(!OdlWrite(FromidAccount, pStream)) return ODLFALSE; 
		if(!OdlWrite(ToidAccount, pStream)) return ODLFALSE; 
		if(!OdlWrite(FromNick, pStream)) return ODLFALSE; 

		return ODLTRUE;	
	}
	void DAT_MsgAddBuddyReq::Dump(ODLWriter* pWriter ) const
	{
		pWriter->Print(_T("<MsgAddBuddyReq> {"));
		pWriter->Print(_T(" BuddyType="));
		OdlDump(BuddyType, pWriter); 
		pWriter->Print(_T(" FromidAccount="));
		OdlDump(FromidAccount, pWriter); 
		pWriter->Print(_T(" ToidAccount="));
		OdlDump(ToidAccount, pWriter); 
		pWriter->Print(_T(" FromNick="));
		OdlDump(FromNick, pWriter); 

		pWriter->Print(_T(" }"));
	}
	ODLBOOL DAT_MsgAddBuddyAns::Read(ODLStream *pStream)
	{
		if(!OdlRead(errCode,pStream)) return ODLFALSE; 
		if(!OdlRead(FromidAccount,pStream)) return ODLFALSE; 
		if(!OdlRead(ToidAccount,pStream)) return ODLFALSE; 
		if(!OdlRead(FromNick,pStream)) return ODLFALSE; 

		return ODLTRUE;	
	}
	ODLBOOL DAT_MsgAddBuddyAns::Write(ODLStream *pStream) const
	{
		if(!OdlWrite(errCode, pStream)) return ODLFALSE; 
		if(!OdlWrite(FromidAccount, pStream)) return ODLFALSE; 
		if(!OdlWrite(ToidAccount, pStream)) return ODLFALSE; 
		if(!OdlWrite(FromNick, pStream)) return ODLFALSE; 

		return ODLTRUE;	
	}
	void DAT_MsgAddBuddyAns::Dump(ODLWriter* pWriter ) const
	{
		pWriter->Print(_T("<MsgAddBuddyAns> {"));
		pWriter->Print(_T(" errCode="));
		OdlDump(errCode, pWriter); 
		pWriter->Print(_T(" FromidAccount="));
		OdlDump(FromidAccount, pWriter); 
		pWriter->Print(_T(" ToidAccount="));
		OdlDump(ToidAccount, pWriter); 
		pWriter->Print(_T(" Fromnickname="));
		OdlDump(FromNick, pWriter); 

		pWriter->Print(_T(" }"));
	}
	ODLBOOL DAT_MsgDeleteBuddyNtf::Read(ODLStream *pStream)
	{
		if(!OdlRead(sourceidAccount,pStream)) return ODLFALSE; 
		if(!OdlRead(destidAccount,pStream)) return ODLFALSE; 

		return ODLTRUE;	
	}
	ODLBOOL DAT_MsgDeleteBuddyNtf::Write(ODLStream *pStream) const
	{
		if(!OdlWrite(sourceidAccount, pStream)) return ODLFALSE; 
		if(!OdlWrite(destidAccount, pStream)) return ODLFALSE; 

		return ODLTRUE;	
	}
	void DAT_MsgDeleteBuddyNtf::Dump(ODLWriter* pWriter ) const
	{
		pWriter->Print(_T("<MsgDeleteBuddyNtf> {"));
		pWriter->Print(_T(" sourceidAccount="));
		OdlDump(sourceidAccount, pWriter); 
		pWriter->Print(_T(" destidAccount="));
		OdlDump(destidAccount, pWriter); 

		pWriter->Print(_T(" }"));
	}
	ODLBOOL DAT_MsgGameInviteNtf::Read(ODLStream *pStream)
	{
		if(!OdlRead(FromidAccount,pStream)) return ODLFALSE; 
		if(!OdlRead(Fromnickname,pStream)) return ODLFALSE; 
		if(!OdlRead(ToidAccount,pStream)) return ODLFALSE; 
		if(!OdlRead(Fromlocationinfo,pStream)) return ODLFALSE; 
		if(!OdlRead(roomname,pStream)) return ODLFALSE; 
		if(!OdlRead(roomid,pStream)) return ODLFALSE; 

		return ODLTRUE;	
	}
	ODLBOOL DAT_MsgGameInviteNtf::Write(ODLStream *pStream) const
	{
		if(!OdlWrite(FromidAccount, pStream)) return ODLFALSE; 
		if(!OdlWrite(Fromnickname, pStream)) return ODLFALSE; 
		if(!OdlWrite(ToidAccount, pStream)) return ODLFALSE; 
		if(!OdlWrite(Fromlocationinfo, pStream)) return ODLFALSE; 
		if(!OdlWrite(roomname,pStream)) return ODLFALSE; 
		if(!OdlWrite(roomid,pStream)) return ODLFALSE; 

		return ODLTRUE;	
	}
	void DAT_MsgGameInviteNtf::Dump(ODLWriter* pWriter ) const
	{
		pWriter->Print(_T("<MsgGameInviteNtf> {"));
		pWriter->Print(_T(" FromidAccount="));
		OdlDump(FromidAccount, pWriter); 
		pWriter->Print(_T(" Fromnickname="));
		OdlDump(Fromnickname, pWriter); 
		pWriter->Print(_T(" ToidAccount="));
		OdlDump(ToidAccount, pWriter); 
		pWriter->Print(_T(" Fromlocationinfo="));
		OdlDump(Fromlocationinfo, pWriter); 
		pWriter->Print(_T(" roomname="));
		OdlDump(roomname, pWriter); 
		pWriter->Print(_T(" roomid="));
		OdlDump(roomid, pWriter); 

		pWriter->Print(_T(" }"));
	}
	ODLBOOL DAT_MsgClanInviteReq::Read(ODLStream *pStream)
	{
		if(!OdlRead(FromidAccount,pStream)) return ODLFALSE; 
		if(!OdlRead(Fromnickname,pStream)) return ODLFALSE; 
		if(!OdlRead(ToidAccount,pStream)) return ODLFALSE; 
		if(!OdlRead(clanid,pStream)) return ODLFALSE; 
		if(!OdlRead(clanname,pStream)) return ODLFALSE; 
	
		return ODLTRUE;	
	}
	ODLBOOL DAT_MsgClanInviteReq::Write(ODLStream *pStream) const
	{
		if(!OdlWrite(FromidAccount, pStream)) return ODLFALSE; 
		if(!OdlWrite(Fromnickname, pStream)) return ODLFALSE; 
		if(!OdlWrite(ToidAccount, pStream)) return ODLFALSE; 
		if(!OdlWrite(clanid, pStream)) return ODLFALSE; 
		if(!OdlWrite(clanname, pStream)) return ODLFALSE; 
	
		return ODLTRUE;	
	}
	void DAT_MsgClanInviteReq::Dump(ODLWriter* pWriter ) const
	{
		pWriter->Print(_T("<MsgClanInviteReq> {"));
		pWriter->Print(_T(" FromidAccount="));
		OdlDump(FromidAccount, pWriter); 
		pWriter->Print(_T(" Fromnickname="));
		OdlDump(Fromnickname, pWriter); 
		pWriter->Print(_T(" ToidAccount="));
		OdlDump(ToidAccount, pWriter); 
		pWriter->Print(_T(" clanid="));
		OdlDump(clanid, pWriter); 
		pWriter->Print(_T(" clanname="));
		OdlDump(clanname, pWriter); 
	
		pWriter->Print(_T(" }"));
	}
	ODLBOOL DAT_MsgClanInviteAns::Read(ODLStream *pStream)
	{
		if(!OdlRead(errCode,pStream)) return ODLFALSE; 
		if(!OdlRead(FromidAccount,pStream)) return ODLFALSE; 
		if(!OdlRead(ToidAccount,pStream)) return ODLFALSE; 
	
		return ODLTRUE;	
	}
	ODLBOOL DAT_MsgClanInviteAns::Write(ODLStream *pStream) const
	{
		if(!OdlWrite(errCode, pStream)) return ODLFALSE; 
		if(!OdlWrite(FromidAccount, pStream)) return ODLFALSE; 
		if(!OdlWrite(ToidAccount, pStream)) return ODLFALSE; 
	
		return ODLTRUE;	
	}
	void DAT_MsgClanInviteAns::Dump(ODLWriter* pWriter ) const
	{
		pWriter->Print(_T("<MsgClanInviteAns> {"));
		pWriter->Print(_T(" errCode="));
		OdlDump(errCode, pWriter); 
		pWriter->Print(_T(" FromidAccount="));
		OdlDump(FromidAccount, pWriter); 
		pWriter->Print(_T(" ToidAccount="));
		OdlDump(ToidAccount, pWriter); 
	
		pWriter->Print(_T(" }"));
	}
	ODLBOOL DAT_MsgClanJoinNtf::Read(ODLStream *pStream)
	{
		if(!OdlRead(ToidAccount,pStream)) return ODLFALSE; 
		if(!OdlRead(gcsaddr,pStream)) return ODLFALSE; 
		if(!OdlRead(clanid,pStream)) return ODLFALSE; 
	
		return ODLTRUE;	
	}
	ODLBOOL DAT_MsgClanJoinNtf::Write(ODLStream *pStream) const
	{
		if(!OdlWrite(ToidAccount, pStream)) return ODLFALSE; 
		if(!OdlWrite(gcsaddr,pStream)) return ODLFALSE; 
		if(!OdlWrite(clanid,pStream)) return ODLFALSE; 
	
		return ODLTRUE;	
	}
	void DAT_MsgClanJoinNtf::Dump(ODLWriter* pWriter ) const
	{
		pWriter->Print(_T("<MsgClanJoinNtf> {"));
		pWriter->Print(_T(" ToidAccount="));
		OdlDump(ToidAccount, pWriter); 
		pWriter->Print(_T(" gcsaddr="));
		OdlDump(gcsaddr, pWriter); 
		pWriter->Print(_T(" clanid="));
		OdlDump(clanid, pWriter); 
	
		pWriter->Print(_T(" }"));
	}
	ODLBOOL DAT_MsgPrivateChatData::Read(ODLStream *pStream)
	{
		if(!OdlRead(FromidAccount,pStream)) return ODLFALSE; 
		if(!OdlRead(Fromnickname,pStream)) return ODLFALSE; 
		if(!OdlRead(ToidAccount,pStream)) return ODLFALSE; 
		if(!OdlRead(chatdata,pStream)) return ODLFALSE; 

		return ODLTRUE;	
	}
	ODLBOOL DAT_MsgPrivateChatData::Write(ODLStream *pStream) const
	{
		if(!OdlWrite(FromidAccount, pStream)) return ODLFALSE; 
		if(!OdlWrite(Fromnickname, pStream)) return ODLFALSE; 
		if(!OdlWrite(ToidAccount, pStream)) return ODLFALSE; 
		if(!OdlWrite(chatdata, pStream)) return ODLFALSE; 

		return ODLTRUE;	
	}
	void DAT_MsgPrivateChatData::Dump(ODLWriter* pWriter ) const
	{
		pWriter->Print(_T("<MsgPrivateChatData> {"));
		pWriter->Print(_T(" FromidAccount="));
		OdlDump(FromidAccount, pWriter); 
		pWriter->Print(_T(" Fromnickname="));
		OdlDump(Fromnickname, pWriter); 
		pWriter->Print(_T(" ToidAccount="));
		OdlDump(ToidAccount, pWriter); 
		pWriter->Print(_T(" chatdata="));
		OdlDump(chatdata, pWriter); 

		pWriter->Print(_T(" }"));
	}
	ODLBOOL DAT_MsgLocationInfoReq::Read(ODLStream *pStream)
	{
		if(!OdlRead(srcidAccount,pStream)) return ODLFALSE; 
		if(!OdlRead(destidAccount,pStream)) return ODLFALSE; 

		return ODLTRUE;	
	}
	ODLBOOL DAT_MsgLocationInfoReq::Write(ODLStream *pStream) const
	{
		if(!OdlWrite(srcidAccount, pStream)) return ODLFALSE; 
		if(!OdlWrite(destidAccount, pStream)) return ODLFALSE; 

		return ODLTRUE;	
	}
	void DAT_MsgLocationInfoReq::Dump(ODLWriter* pWriter ) const
	{
		pWriter->Print(_T("<MsgLocationInfoReq> {"));
		pWriter->Print(_T(" srcidAccount="));
		OdlDump(srcidAccount, pWriter); 
		pWriter->Print(_T(" destidAccount="));
		OdlDump(destidAccount, pWriter); 

		pWriter->Print(_T(" }"));
	}
	ODLBOOL DAT_MsgLocationInfoAns::Read(ODLStream *pStream)
	{
		if(!OdlRead(errCode,pStream)) return ODLFALSE; 
		if(!OdlRead(idAccount,pStream)) return ODLFALSE; 
		if(!OdlRead(loinfo,pStream)) return ODLFALSE; 

		return ODLTRUE;	
	}
	ODLBOOL DAT_MsgLocationInfoAns::Write(ODLStream *pStream) const
	{
		if(!OdlWrite(errCode, pStream)) return ODLFALSE; 
		if(!OdlWrite(idAccount, pStream)) return ODLFALSE; 
		if(!OdlWrite(loinfo, pStream)) return ODLFALSE; 

		return ODLTRUE;	
	}
	void DAT_MsgLocationInfoAns::Dump(ODLWriter* pWriter ) const
	{
		pWriter->Print(_T("<MsgLocationInfoAns> {"));
		pWriter->Print(_T(" errCode="));
		OdlDump(errCode, pWriter); 
		pWriter->Print(_T(" idAccount="));
		OdlDump(idAccount, pWriter); 
		pWriter->Print(_T(" loinfo="));
		OdlDump(loinfo, pWriter); 

		pWriter->Print(_T(" }"));
	}
	ODLBOOL DAT_MsgBuddyInfoReq::Read(ODLStream *pStream)
	{
		if(!OdlRead(bconnectAlram,pStream)) return ODLFALSE; 
		if(!OdlRead(idAccount,pStream)) return ODLFALSE; 

		return ODLTRUE;	
	}
	ODLBOOL DAT_MsgBuddyInfoReq::Write(ODLStream *pStream) const
	{
		if(!OdlWrite(bconnectAlram, pStream)) return ODLFALSE; 
		if(!OdlWrite(idAccount, pStream)) return ODLFALSE; 

		return ODLTRUE;	
	}
	void DAT_MsgBuddyInfoReq::Dump(ODLWriter* pWriter ) const
	{
		pWriter->Print(_T("<MsgBuddyInfoReq> {"));
		pWriter->Print(_T(" bconnectAlram="));
		OdlDump(bconnectAlram, pWriter); 
		pWriter->Print(_T(" idAccount="));
		OdlDump(idAccount, pWriter); 

		pWriter->Print(_T(" }"));
	}
	ODLBOOL DAT_MsgBuddyInfoAns::Read(ODLStream *pStream)
	{
		if(!OdlRead(count,pStream)) return ODLFALSE; 
		if(!OdlRead(ava_friend,pStream)) return ODLFALSE; 

		return ODLTRUE;	
	}
	ODLBOOL DAT_MsgBuddyInfoAns::Write(ODLStream *pStream) const
	{
		if(!OdlWrite(count, pStream)) return ODLFALSE; 
		if(!OdlWrite(ava_friend, pStream)) return ODLFALSE; 

		return ODLTRUE;	
	}
	void DAT_MsgBuddyInfoAns::Dump(ODLWriter* pWriter ) const
	{
		pWriter->Print(_T("<MsgBuddyInfoAns> {"));
		pWriter->Print(_T(" count="));
		OdlDump(count, pWriter); 
		pWriter->Print(_T(" ava_friend="));
		OdlDump(ava_friend, pWriter); 

		pWriter->Print(_T(" }"));
	}
	ODLBOOL DAT_MsgBuddyInfoEndNtf::Read(ODLStream *pStream)
	{
		if(!OdlRead(idAccount,pStream)) return ODLFALSE; 

		return ODLTRUE;	
	}
	ODLBOOL DAT_MsgBuddyInfoEndNtf::Write(ODLStream *pStream) const
	{
		if(!OdlWrite(idAccount, pStream)) return ODLFALSE; 

		return ODLTRUE;	
	}
	void DAT_MsgBuddyInfoEndNtf::Dump(ODLWriter* pWriter ) const
	{
		pWriter->Print(_T("<MsgBuddyInfoEndNtf> {"));
		pWriter->Print(_T(" idAccount="));
		OdlDump(idAccount, pWriter); 

		pWriter->Print(_T(" }"));
	}
	ODLBOOL DAT_MsgBuddyAlarmNtf::Read(ODLStream *pStream)
	{
		if(!OdlRead(buddyinfo,pStream)) return ODLFALSE; 

		return ODLTRUE;	
	}
	ODLBOOL DAT_MsgBuddyAlarmNtf::Write(ODLStream *pStream) const
	{
		if(!OdlWrite(buddyinfo, pStream)) return ODLFALSE; 

		return ODLTRUE;	
	}
	void DAT_MsgBuddyAlarmNtf::Dump(ODLWriter* pWriter ) const
	{
		pWriter->Print(_T("<MsgBuddyAlarmNtf> {"));
		pWriter->Print(_T(" buddyinfo="));
		buddyinfo.Dump(pWriter);

		pWriter->Print(_T(" }"));
	}
	ODLBOOL DAT_MsgPairBuddyinfoReq::Read(ODLStream *pStream)
	{
		if(!OdlRead(idAccount,pStream)) return ODLFALSE; 

		return ODLTRUE;	
	}
	ODLBOOL DAT_MsgPairBuddyinfoReq::Write(ODLStream *pStream) const
	{
		if(!OdlWrite(idAccount, pStream)) return ODLFALSE; 

		return ODLTRUE;	
	}
	void DAT_MsgPairBuddyinfoReq::Dump(ODLWriter* pWriter ) const
	{
		pWriter->Print(_T("<MsgPairBuddyinfoReq> {"));
		pWriter->Print(_T(" idAccount="));
		OdlDump(idAccount, pWriter); 

		pWriter->Print(_T(" }"));
	}
	ODLBOOL DAT_MsgPairBuddyinfoAns::Read(ODLStream *pStream)
	{
		if(!OdlRead(err,pStream)) return ODLFALSE; 
		if(!OdlRead(cnt,pStream)) return ODLFALSE; 
		if(!OdlRead(pairbuddyinfo,pStream)) return ODLFALSE; 

		return ODLTRUE;	
	}
	ODLBOOL DAT_MsgPairBuddyinfoAns::Write(ODLStream *pStream) const
	{
		if(!OdlWrite(err, pStream)) return ODLFALSE; 
		if(!OdlWrite(cnt, pStream)) return ODLFALSE; 
		if(!OdlWrite(pairbuddyinfo,pStream)) return ODLFALSE; 

		return ODLTRUE;	
	}
	void DAT_MsgPairBuddyinfoAns::Dump(ODLWriter* pWriter ) const
	{
		pWriter->Print(_T("<MsgPairBuddyinfoAns> {"));
		pWriter->Print(_T(" err="));
		OdlDump(err, pWriter); 
		pWriter->Print(_T(" cnt="));
		OdlDump(cnt, pWriter); 
		pWriter->Print(_T(" pairbuddyinfo="));
		OdlDump(pairbuddyinfo, pWriter); 

		pWriter->Print(_T(" }"));
	}
	ODLBOOL DAT_MsgUpdateNickNameNtf::Read(ODLStream *pStream)
	{
		if(!OdlRead(idAccount,pStream)) return ODLFALSE; 
		if(!OdlRead(nickname,pStream)) return ODLFALSE; 

		return ODLTRUE;	
	}
	ODLBOOL DAT_MsgUpdateNickNameNtf::Write(ODLStream *pStream) const
	{
		if(!OdlWrite(idAccount,pStream)) return ODLFALSE; 
		if(!OdlWrite(nickname,pStream)) return ODLFALSE; 

		return ODLTRUE;	
	}
	void DAT_MsgUpdateNickNameNtf::Dump(ODLWriter* pWriter ) const
	{
		pWriter->Print(_T("<MsgUpdateNickNameNtf> {"));
		pWriter->Print(_T(" idAccount="));
		OdlDump(idAccount, pWriter); 
		pWriter->Print(_T(" nickname="));
		OdlDump(nickname, pWriter); 

		pWriter->Print(_T(" }"));
	}
	ODLBOOL DAT_MsgPCBUserStartNtf::Read(ODLStream *pStream)
	{
		if(!OdlRead(service_type,pStream)) return ODLFALSE; 
		if(!OdlRead(IP,pStream)) return ODLFALSE; 
		if(!OdlRead(idAccount,pStream)) return ODLFALSE; 
		if(!OdlRead(pmangID,pStream)) return ODLFALSE; 
	
		return ODLTRUE;	
	}
	ODLBOOL DAT_MsgPCBUserStartNtf::Write(ODLStream *pStream) const
	{
		if(!OdlWrite(service_type, pStream)) return ODLFALSE; 
		if(!OdlWrite(IP, pStream)) return ODLFALSE; 
		if(!OdlWrite(idAccount, pStream)) return ODLFALSE; 
		if(!OdlWrite(pmangID, pStream)) return ODLFALSE; 
	
		return ODLTRUE;	
	}
	void DAT_MsgPCBUserStartNtf::Dump(ODLWriter* pWriter ) const
	{
		pWriter->Print(_T("<MsgPCBUserStartNtf> {"));
		pWriter->Print(_T(" service_type="));
		OdlDump(service_type, pWriter); 
		pWriter->Print(_T(" IP="));
		OdlDump(IP, pWriter); 
		pWriter->Print(_T(" idAccount="));
		OdlDump(idAccount, pWriter); 
		pWriter->Print(_T(" pmangID="));
		OdlDump(pmangID, pWriter); 
	
		pWriter->Print(_T(" }"));
	}
	ODLBOOL DAT_MsgPCBUserTimeExpireNtf::Read(ODLStream *pStream)
	{
		if(!OdlRead(ret,pStream)) return ODLFALSE; 
	
		return ODLTRUE;	
	}
	ODLBOOL DAT_MsgPCBUserTimeExpireNtf::Write(ODLStream *pStream) const
	{
		if(!OdlWrite(ret, pStream)) return ODLFALSE; 
	
		return ODLTRUE;	
	}
	void DAT_MsgPCBUserTimeExpireNtf::Dump(ODLWriter* pWriter ) const
	{
		pWriter->Print(_T("<MsgPCBUserTimeExpireNtf> {"));
		pWriter->Print(_T(" ret="));
		OdlDump(ret, pWriter); 
	
		pWriter->Print(_T(" }"));
	}
	ODLBOOL DAT_MsgPCBRemainTimeReq::Read(ODLStream *pStream)
	{
		if(!OdlRead(idAccount,pStream)) return ODLFALSE; 
		if(!OdlRead(IP,pStream)) return ODLFALSE; 
	
		return ODLTRUE;	
	}
	ODLBOOL DAT_MsgPCBRemainTimeReq::Write(ODLStream *pStream) const
	{
		if(!OdlWrite(idAccount, pStream)) return ODLFALSE; 
		if(!OdlWrite(IP, pStream)) return ODLFALSE; 
	
		return ODLTRUE;	
	}
	void DAT_MsgPCBRemainTimeReq::Dump(ODLWriter* pWriter ) const
	{
		pWriter->Print(_T("<MsgPCBRemainTimeReq> {"));
		pWriter->Print(_T(" idAccount="));
		OdlDump(idAccount, pWriter); 
		pWriter->Print(_T(" IP="));
		OdlDump(IP, pWriter); 
	
		pWriter->Print(_T(" }"));
	}
	ODLBOOL DAT_MsgPCBRemainTimeAns::Read(ODLStream *pStream)
	{
		if(!OdlRead(strtime,pStream)) return ODLFALSE; 
	
		return ODLTRUE;	
	}
	ODLBOOL DAT_MsgPCBRemainTimeAns::Write(ODLStream *pStream) const
	{
		if(!OdlWrite(strtime, pStream)) return ODLFALSE; 
	
		return ODLTRUE;	
	}
	void DAT_MsgPCBRemainTimeAns::Dump(ODLWriter* pWriter ) const
	{
		pWriter->Print(_T("<MsgPCBRemainTimeAns> {"));
		pWriter->Print(_T(" strtime="));
		OdlDump(strtime, pWriter); 
	
		pWriter->Print(_T(" }"));
	}
	ODLBOOL DAT_MsgPCBRemainTimeNtf::Read(ODLStream *pStream)
	{
		if(!OdlRead(strtime,pStream)) return ODLFALSE; 
	
		return ODLTRUE;	
	}
	ODLBOOL DAT_MsgPCBRemainTimeNtf::Write(ODLStream *pStream) const
	{
		if(!OdlWrite(strtime, pStream)) return ODLFALSE; 
	
		return ODLTRUE;	
	}
	void DAT_MsgPCBRemainTimeNtf::Dump(ODLWriter* pWriter ) const
	{
		pWriter->Print(_T("<MsgPCBRemainTimeNtf> {"));
		pWriter->Print(_T(" strtime="));
		OdlDump(strtime, pWriter); 
	
		pWriter->Print(_T(" }"));
	}
};
