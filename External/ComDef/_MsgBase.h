/***

	Copyright (c) Redduck Inc. All rights reserved.

	Project: ComDef

	Name: _MsgBase.h

	Description: Base types, constants, macros used by all message definitions.

***/

#pragma once

#include "Def.h"
#include "MsgDef.h"

// Message category code
enum _MSG_CATEGORY
{
	MC_NONE = 0,
	MC_WEB,
	MC_GAME,
	MC_CHANNEL,
	MC_ROOM,
	MC_INVENTORY,
	MC_BOOTSEQ,
	MC_CLIENT,
	MC_FRIEND,	
	MC_GUILD,
	MC_ADMIN,
	MC_SYSTEM,
	MC_SESSION,
	MC_BUDDYODL,
	MC_END,
};



// Utility macros
#define MSGDEF_NAME(n) static const TCHAR *MsgName() { return __CAT _T("::") _T(#n); }

// Intel base
#define MSGDEF_ID(n) ((__MC << 8) + ID_##n)
#define MSGDEF_TYPE typedef CMsgT<ID, DEF>




//////////////////////////////////////////////////////////////////////////////////////
// stream

namespace PM
{
	struct _STREAM
	{
		WORD offset;

		_STREAM() : offset(0) {}
		LPBYTE GetData(LPBYTE base)
		{
			return base + offset;
		}
	};

	struct _BUF_DATA
	{
		WORD len;
		BYTE buf;
	};

	typedef _STREAM _BUFFER;
	typedef _STREAM _STRING;

	inline size_t _GetStringSize(WCHAR *str)
	{
		return (wcslen(str) + 1) * sizeof(WCHAR);
	}

	template<typename T> inline size_t _GetBufferSize(WORD cnt)
	{
		return sizeof(WORD) + cnt * sizeof(T);
	}


	class _MsgString;

	template <typename T>
	class _MsgBuffer
	{
	private:
		CMsg &msg;
		_BUFFER &stream;

	public:
		explicit _MsgBuffer(_MsgBuffer &src) : msg(src.msg), stream(src.stream) {}
		_MsgBuffer(CMsg &basemsg, _BUFFER &buf) : msg(basemsg), stream(buf) {}

		WORD GetOffset() const { return stream.offset; }
		void SetOffset(WORD offset) { stream.offset = offset; }
		template <typename N> void SetOffsetAfter(_MsgBuffer<N> &buf) { stream.offset = buf.GetOffset() + buf.GetLength() + sizeof(WORD); }
		void SetOffsetAfter(_MsgString &str) { stream.offset = str.GetOffset() + (WORD)PM::_GetStringSize(str); }

		WORD GetLength() const { return reinterpret_cast<_BUF_DATA*>(stream.GetData(msg.GetStreamBase()))->len; }
		void SetLength(WORD len)
		{
			if (stream.offset + sizeof(WORD) + len > msg.GetStreamLength())
				msg.SetStreamLength(stream.offset + sizeof(WORD) + len);
			(reinterpret_cast<_BUF_DATA*>(stream.GetData(msg.GetStreamBase())))->len = len;
		}
		WORD GetCount() const { return GetLength() / sizeof(T); }
		void SetCount(WORD cnt) { SetLength(cnt * sizeof(T)); }

		bool CheckValidity()
		{
			return ((BYTE*)GetBuffer() + GetLength() <= msg.GetBuf()->GetData() + msg.GetBufLength());
		}

		T* GetBuffer(WORD idx = 0)
		{
			return reinterpret_cast<T*>(&(reinterpret_cast<_BUF_DATA*>(stream.GetData(msg.GetStreamBase()))->buf)) + idx;
		}
		void SetBuffer(T *buf, WORD cnt = 1)
		{
			WORD _len = cnt * sizeof(T);
			SetLength(_len);
			if (cnt > 0)
				memcpy(GetBuffer(), buf, _len);
		}
		void SetBuffer(WORD from, T *buf, WORD cnt = 1)
		{
			WORD _len = (from + cnt) * sizeof(T);
			SetLength(_len);
			if (cnt > 0)
				memcpy(GetBuffer(from), buf, cnt * sizeof(T));
		}

		_MsgBuffer& operator=(const T *buf) { SetBuffer(buf); return *this; }
		_MsgBuffer& operator=(T buf) { SetBuffer(&buf); return *this; }
		_MsgBuffer& operator=(const _MsgBuffer &src) { msg = src.msg; stream = src.stream; return *this; }
		T* operator->() { return GetBuffer(); }
		T& operator*() { return *(GetBuffer()); }
		T& operator[](WORD idx) { return *GetBuffer(idx); }
		T& operator[](int idx) { return *GetBuffer((WORD)idx); }
		operator T*() { return GetBuffer(); }
	};

	class _MsgString
	{
	private:
		CMsg &msg;
		_STRING &stream;

	public:
		explicit _MsgString(const _MsgString &src) : msg(src.msg), stream(src.stream) {}
		_MsgString(CMsg &basemsg, _STRING &str) : msg(basemsg), stream(str) {}

		bool CheckValidity()
		{
			return ((BYTE*)GetString() + GetLength() <= msg.GetBuf()->GetData() + msg.GetBufLength());
		}

		WORD GetOffset() const { return stream.offset; }
		void SetOffset(WORD offset) { stream.offset = offset; }
		template <typename N> void SetOffsetAfter(_MsgBuffer<N> &buf) { stream.offset = buf.GetOffset() + buf.GetLength() + sizeof(WORD); }
		void SetOffsetAfter(_MsgString &str) { stream.offset = str.GetOffset() + (WORD)PM::_GetStringSize(str); }

		WCHAR* GetString() { return reinterpret_cast<WCHAR*>(stream.GetData(msg.GetStreamBase())); }
		size_t GetLength() { return wcslen(GetString()); }
		WCHAR* SetString(const WCHAR *str)
		{
			size_t _len = wcslen(str);
			if (stream.offset + (_len + 1) * sizeof(WCHAR) > msg.GetStreamLength())
				msg.SetStreamLength(stream.offset + (_len + 1) * sizeof(WCHAR));
			wcsncpy(GetString(), str, _len);
			GetString()[_len] = 0;
			return *this;
		}

		operator LPWSTR() { return GetString(); }
		_MsgString& operator=(const WCHAR* str) { SetString(str); return *this; }
		_MsgString& operator=(const _MsgString &src) { msg = src.msg; stream = src.stream; return *this; }
	};

	inline WORD _GetBufferLength(CMsg &msg, _BUFFER &buf)
	{
		return reinterpret_cast<_BUF_DATA*>(buf.GetData(msg.GetStreamBase()))->len;
	}

	inline LPBYTE _GetBuffer(CMsg &msg, _BUFFER &buf)
	{
		return &(reinterpret_cast<_BUF_DATA*>(buf.GetData(msg.GetStreamBase()))->buf);
	}

	inline WORD _GetStringLength(CMsg &msg, _STRING &str)
	{
		return (WORD)(wcslen(reinterpret_cast<WCHAR*>(str.GetData(msg.GetStreamBase()))) + 1) * sizeof(WCHAR);
	}

	inline WCHAR* _GetString(CMsg &msg, _STRING &str)
	{
		return reinterpret_cast<WCHAR*>(str.GetData(msg.GetStreamBase()));
	}

	inline void _CopyBuffer(CMsg &msgTo, _BUFFER &bufTo, CMsg &msgFrom, _BUFFER &bufFrom)
	{
		WORD _len = _GetBufferLength(msgFrom, bufFrom) + sizeof(WORD);
		if ( bufTo.offset + _len > (WORD)msgTo.GetStreamLength() )
			msgTo.SetStreamLength(bufTo.offset + _len);

		memcpy(bufTo.GetData(msgTo.GetStreamBase()), bufFrom.GetData(msgFrom.GetStreamBase()), _len);
	}

	template <typename T> inline void _CopyBuffer(_MsgBuffer<T> &to, _MsgBuffer<T> &from)
	{
		to.SetBuffer(from.GetBuffer(), from.GetCount());
	}

};	// namespace PM



// Null message
namespace PM
{
	namespace NONE
	{
		enum _ID
		{
			ID_NONE = 0
		};

		namespace NONE
		{
			enum _ID { ID = 0 };
			struct DEF
			{
				static const TCHAR *MsgName() { return _T("UNKNOWN"); }
			};

			MSGDEF_TYPE TMSG;
		};
	};
};

