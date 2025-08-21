/***

	Copyright (c) Redduck Inc. All rights reserved.

	Project: ComDef

	Name: MsgDef.h

	Description: Message wrapper class used by all AVA server and clients.

***/

#pragma once


#ifdef _AVA_SERVER
	#include <GateNet/GNBuf.h>
	#include <GNBase/Storage.h>
#endif

#include "MsgBuf.h"

#pragma pack(push)
#pragma pack(2)

struct MSG_HEADER
{
	WORD length;	// (header length) + (body length) + (stream length)
	WORD body_length;
#pragma warning( disable : 4201)
	union
	{
		struct
		{
			BYTE msg_id;
			BYTE msg_cat;
		};
		WORD id;
	};
#pragma warning( default : 4201)
};

#pragma pack(pop)


//ODL MSG HEADER
#pragma pack(push)
#pragma pack(2)
struct ODLMSGHDR
{
	DWORD len;
	WORD id;
};
#pragma pack(pop)




class CMsg
#ifdef _AVA_SERVER
	: public GNStorage<CMsg>
#endif
{
public:
	enum _LENGTH { HEADER_LENGTH = sizeof(MSG_HEADER) };

#ifndef _AVA_SERVER
private:
	bool bRef;
#endif

protected:
	_LPMSGBUF buf;

protected:

#ifdef _AVA_SERVER
	void _FreeBuf()
	{
		buf->Delete();
	}
	void _AssignBuf(const _LPMSGBUF src)
	{
		buf = CreateMsgBuf(src);
	}
#else
	void _FreeBuf()
	{
		if (!bRef)
			buf->Delete();
	}
	void _AssignBuf(const _LPMSGBUF src)
	{
		buf = src;
		bRef = true;
	}
#endif

public:
	CMsg(WORD id, size_t length)	// length = (body length) + (stream length)
#ifndef _AVA_SERVER
		: bRef(false)
#endif
	{
		ASSERT(length > 0);
		buf = CreateMsgBufN((DWORD)length + HEADER_LENGTH);
		ASSERT(buf);
		buf->SetLength((DWORD)length + HEADER_LENGTH);
		ZeroMemory(buf->GetData(), buf->GetLength());
		Header().length = (WORD)(length + HEADER_LENGTH);
		Header().id = id;
	}
	explicit CMsg(const _LPMSGBUF src)
	{
		_AssignBuf(src);
	}
	explicit CMsg(const CMsg& src)
	{
		_AssignBuf(src.buf);
	}
	virtual ~CMsg()
	{
		_FreeBuf();
	}

	_LPMSGBUF GetBuf() const { return buf; }
	void SetBuf(_LPMSGBUF ptr) { ASSERT(ptr->GetLength() >= HEADER_LENGTH); buf = ptr; }
	size_t GetBufLength() const { return buf->GetLength(); }
	void SetBufLength(size_t len) { buf->SetLength((DWORD)len); }
	size_t GetBodyLength() const { if (buf->GetLength() >= GetHeaderLength()) return Header().body_length; else return 0; }

	MSG_HEADER& Header() const { return *(reinterpret_cast<MSG_HEADER*>(buf->GetData())); }
	size_t GetHeaderLength() const { return HEADER_LENGTH; }
	LPBYTE GetStreamBase() const { return buf->GetData() + GetHeaderLength() + GetBodyLength(); }
	size_t GetStreamLength() const { return GetBufLength() - GetHeaderLength() - GetBodyLength(); }
	void SetStreamLength(size_t len) { SetBufLength(GetHeaderLength() + GetBodyLength() + len); }

	CMsg& operator=(const CMsg& src)
	{
		_FreeBuf();
		_AssignBuf(src.buf);

		return *this;
	}

	void Copy(const CMsg& src)
	{
		_FreeBuf();

		int _length = src.buf->GetLength();
		buf = CreateMsgBufN((DWORD)_length);
		buf->SetLength((DWORD)_length);
		//ZeroMemory(buf->GetData(), buf->GetLength());
		//Header().length = (WORD)(src.buf->GetLength());
		//Header().id = src.Header().id;

		memcpy(buf->GetData(),src.buf->GetData(),_length);

#ifndef _AVA_SERVER
		bRef = false;
#endif
	}

	virtual const TCHAR *MsgName() const = 0;
};

template <int ID, typename T>
class CMsgT : public CMsg
{
public:
	enum _LENGTH { BODY_LENGTH = sizeof(T) };

public:
	explicit CMsgT(size_t length = BODY_LENGTH) : CMsg(ID, length)	// length = (body length) + (stream length)
	{
		ASSERT(length >= BODY_LENGTH);
		Header().body_length = BODY_LENGTH;
	}
	explicit CMsgT(const _LPMSGBUF src) : CMsg(src) {}
	explicit CMsgT(const CMsgT<ID, T>& src) : CMsg(src) {}
	virtual ~CMsgT() {}

	CMsgT<ID, T> & operator=(const CMsgT<ID, T> &src)
	{
		_FreeBuf();
		_AssignBuf(src.buf);

		return *this;
	}

	//virtual size_t GetBodyLength() const { return BODY_LENGTH; }
	virtual const TCHAR *MsgName() const { return T::MsgName(); }
	T& Data() const { return *(reinterpret_cast<T*>(buf->GetData() + GetHeaderLength())); }
};
