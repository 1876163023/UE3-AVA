#pragma once


#include "Def.h"
#include "Base.h"
#include "ComDef/MsgBuf.h"

namespace RxGate
{

	typedef WORD RXMSGID;

	#pragma pack(push)
	#pragma pack(2)
	struct _MSGHDR
	{
		DWORD len;
		RXMSGID id;
	};
	#pragma pack(pop)


	LONG __stdcall MsgDataCheck(_LPMSGBUF lpBuf);
	_MSGHDR* GetMsgHdr(_LPMSGBUF lpBuf);


	struct IRxMsg
	{
		virtual BOOL Read(IRxStream *pStream) = 0;
		virtual RXMSGID GetID() const = 0;
		virtual BOOL Write(IRxStream *pStream) const = 0;
		virtual void Dump(IRxWriter* pWriter) const = 0;
		virtual LPCTSTR GetName() const  = 0;
		virtual LPCTSTR GetDescription() const  = 0;
		virtual WORD	GetBodyLength() const = 0;
	};

}
