#pragma once


#include "RxMsg.h"

#ifdef _AVA_SERVER
	#include <GNBase/Storage.h>
#endif


namespace RxGate
{

	#define _ODL_SERIALIZE(TYPE, OUTFMT) \
		inline BOOL OdlRead(TYPE &dat, IRxStream *pStream) \
		{ return pStream->Read(dat); } \
		inline BOOL OdlWrite(const TYPE &dat, IRxStream *pStream) \
		{ return pStream->Write(dat); } \
		inline BOOL OdlRead(TYPE *dat, WORD size, IRxStream *pStream) \
		{ return pStream->Read(dat, size); } \
		inline BOOL OdlWrite(const TYPE *dat, WORD size, IRxStream *pStream) \
		{ return pStream->Write(dat, size); } 

	_ODL_SERIALIZE(char, "%d");
	_ODL_SERIALIZE(short, "%d");
	_ODL_SERIALIZE(long, "%d");
	_ODL_SERIALIZE(int, "%d");
	_ODL_SERIALIZE(__int64, "%ld");
	_ODL_SERIALIZE(unsigned char, "%u");
	_ODL_SERIALIZE(unsigned short, "%u");
	_ODL_SERIALIZE(unsigned long, "%u");
	_ODL_SERIALIZE(unsigned int, "%u");
	_ODL_SERIALIZE(unsigned __int64, "%lu");
	_ODL_SERIALIZE(float, "%f");
	_ODL_SERIALIZE(double, "%f");

	template <typename T>
	inline BOOL OdlRead(T *dat, WORD size, IRxStream *pStream)
	{
		for (int i = 0; i < size; i++)
		{
			if (!OdlRead(dat[i], pStream))
				return FLASE;
		}
		return TRUE;
	}
	template <typename T>
	inline BOOL OdlWrite(const T * dat, WORD size, IRxStream *pStream)
	{
		for (int i = 0; i < size; i++)
		{
			if (!OdlWrite(dat[i], pStream))
				return FLASE;
		}
		return TRUE;
	}



	inline BOOL OdlRead(RXNERVE_ADDRESS &dat, IRxStream *pStream)
	{
		return pStream->Read((void*)&dat, sizeof(RXNERVE_ADDRESS));
	}
	inline BOOL OdlWrite(const RXNERVE_ADDRESS &dat, IRxStream *pStream)
	{
		return pStream->Write((const void*)&dat, sizeof(RXNERVE_ADDRESS));
	}


	inline BOOL OdlRead(_LPMSGBUF dat, IRxStream *pStream)
	{
		DWORD _len;
		if ( !pStream->Read(_len) || _len <= 0 )
			return FALSE;
		dat->SetLength(_len);

		return pStream->Read((void*)dat->GetData(), _len);
	}
	inline BOOL OdlWrite(const _LPMSGBUF dat, IRxStream *pStream)
	{
		if ( !dat || dat->GetLength() <= 0 || !pStream->Write(dat->GetLength()) )
			return FALSE;

		return pStream->Write((const void*)dat->GetData(), dat->GetLength());
	}


	inline BOOL OdlRead(IRxMsg &dat, IRxStream *pStream)
	{
		return dat.Read(pStream);
	}
	inline BOOL OdlWrite(const IRxMsg &dat, IRxStream *pStream)
	{
		return dat.Write(pStream);
	}


	enum __TYPE_SIZE
	{
		BYTE_SIZE = 1,
		WORD_SIZE = 2,
		DWORD_SIZE = 4,
		LONGLONG_SIZE = 8,
		FLOAT_SIZE = 4,
		DOUBLE_SIZE = 8,
		INT_SIZE = 4,
	};



	struct CRxStream : public IRxStream
	{
	// 기존부터 있던 부분
		virtual BOOL Write(const void * dat, DWORD size) = 0;
		virtual BOOL Read (void * dat, DWORD size)  = 0;

	// 기본 타입 읽어 들이기
		virtual BOOL Read(char &dat) { return Read((void *) &dat, BYTE_SIZE);}
		virtual BOOL Write(const char &dat) { return Write((LPCVOID) &dat, BYTE_SIZE);}

		virtual BOOL Read(unsigned char &dat){ return Read((void *) &dat, BYTE_SIZE);}
		virtual BOOL Write(const unsigned char &dat) { return Write((LPCVOID) &dat, BYTE_SIZE);}

		virtual BOOL Read(short &dat) { return Read((void *) &dat, WORD_SIZE);}
		virtual BOOL Write(const short &dat) { return Write((LPCVOID) &dat, WORD_SIZE);}

		virtual BOOL Read(unsigned short &dat) { return Read((void *) &dat, WORD_SIZE);}
		virtual BOOL Write(const unsigned short &dat) { return Write((LPCVOID) &dat, WORD_SIZE);}

		virtual BOOL Read(long &dat) { return Read((void *) &dat, DWORD_SIZE);}
		virtual BOOL Write(const long &dat) { return Write((LPCVOID) &dat, DWORD_SIZE);}

		virtual BOOL Read(unsigned long &dat) { return Read((void *) &dat, DWORD_SIZE);}
		virtual BOOL Write(const unsigned long &dat) { return Write((LPCVOID) &dat, DWORD_SIZE);}

		virtual BOOL Read(__int64 &dat) { return Read((void *) &dat, LONGLONG_SIZE);}
		virtual BOOL Write(const __int64 &dat) { return Write((LPCVOID) &dat, LONGLONG_SIZE);}

		virtual BOOL Read(unsigned __int64 &dat) { return Read((void *) &dat, LONGLONG_SIZE);}
		virtual BOOL Write(const unsigned __int64 &dat) { return Write((LPCVOID) &dat, LONGLONG_SIZE);}

		virtual BOOL Read(unsigned &dat) { return Read((void *) &dat, INT_SIZE);}
		virtual BOOL Write(const unsigned &dat) { return Write((LPCVOID) &dat, INT_SIZE);}

		virtual BOOL Read(int &dat) { return Read((void *) &dat, INT_SIZE);}
		virtual BOOL Write(const int &dat) { return Write((LPCVOID) &dat, INT_SIZE);}

		virtual BOOL Read(float &dat) { return Read((void *) &dat, FLOAT_SIZE);}
		virtual BOOL Write(const float &dat) { return Write((LPCVOID) &dat, FLOAT_SIZE);}

		virtual BOOL Read(double &dat) { return Read((void *) &dat, BYTE_SIZE);}
		virtual BOOL Write(const double &dat) { return Write((LPCVOID) &dat, DOUBLE_SIZE);}

	// 기본 포인터 타입 읽어 들이기.
		virtual BOOL Read(char *dat, const WORD &size) { return Read((void *) dat, size * BYTE_SIZE);}
		virtual BOOL Write(const char *dat, const WORD &size) { return Write((LPCVOID) dat, size * BYTE_SIZE);}

		virtual BOOL Read(unsigned char *dat, const WORD &size){ return Read((void *) dat, size * BYTE_SIZE);}
		virtual BOOL Write(const unsigned char *dat, const WORD &size) { return Write((LPCVOID) dat, size * BYTE_SIZE);}

		virtual BOOL Read(short *dat, const WORD &size) { return Read((void *) dat, size * WORD_SIZE);}
		virtual BOOL Write(const short *dat, const WORD &size) { return Write((LPCVOID) dat, size * WORD_SIZE);}

		virtual BOOL Read(unsigned short *dat, const WORD &size) { return Read((void *) dat, size * WORD_SIZE);}
		virtual BOOL Write(const unsigned short *dat, const WORD &size) { return Write((LPCVOID) dat, size * WORD_SIZE);}

		virtual BOOL Read(long *dat, const WORD &size) { return Read((void *) dat, size * DWORD_SIZE);}
		virtual BOOL Write(const long *dat, const WORD &size) { return Write((LPCVOID) dat, size * DWORD_SIZE);}

		virtual BOOL Read(unsigned long *dat, const WORD &size) { return Read((void *) dat, size * DWORD_SIZE);}
		virtual BOOL Write(const unsigned long *dat, const WORD &size) { return Write((LPCVOID) dat, size * DWORD_SIZE);}

		virtual BOOL Read(__int64 *dat, const WORD &size) { return Read((void *) dat, size * LONGLONG_SIZE);}
		virtual BOOL Write(const __int64 *dat, const WORD &size) { return Write((LPCVOID) dat, size * LONGLONG_SIZE);}

		virtual BOOL Read(unsigned __int64 *dat, const WORD &size) { return Read((void *) dat, size * LONGLONG_SIZE);}
		virtual BOOL Write(const unsigned __int64 *dat, const WORD &size) { return Write((LPCVOID) dat, size * LONGLONG_SIZE);}

		virtual BOOL Read(unsigned *dat, const WORD &size) { return Read((void *) dat, size * INT_SIZE);}
		virtual BOOL Write(const unsigned *dat, const WORD &size) { return Write((LPCVOID) dat, size * INT_SIZE);}

		virtual BOOL Read(int *dat, const WORD &size) { return Read((void *) dat, size * INT_SIZE);}
		virtual BOOL Write(const int *dat, const WORD &size) { return Write((LPCVOID) dat, size * INT_SIZE);}

		virtual BOOL Read(float *dat, const WORD &size) { return Read((void *) dat, size * FLOAT_SIZE);}
		virtual BOOL Write(const float *dat, const WORD &size) { return Write((LPCVOID) dat, size * FLOAT_SIZE);}

		virtual BOOL Read(double *dat, const WORD &size) { return Read((void *) dat, size * BYTE_SIZE);}
		virtual BOOL Write(const double *dat, const WORD &size) { return Write((LPCVOID) dat, size * DOUBLE_SIZE);}
	};



	struct CRxReadStream : public CRxStream
	#ifdef _AVA_SERVER
							, GNStorage<CRxReadStream>
	#endif
	{
		_MSGHDR *msgHdr;
		BYTE *buf;

		CRxReadStream(_MSGHDR *hdr) : msgHdr(hdr), buf((BYTE*)(hdr+1)) {}

		// 기존에 있던 부분
		virtual BOOL Write(const void *, DWORD)
		{
			return FALSE;
		}

		virtual BOOL Read(void * dat, DWORD size)
		{
			if (msgHdr->len < buf - (BYTE*)msgHdr - sizeof(_MSGHDR) + size)
				return FALSE;

			memcpy(dat, buf, size);
			buf += size;
			return TRUE;
		}

		operator IRxStream*() { return this; }
	};



	struct CRxWriteStream : public CRxStream
	#ifdef _AVA_SERVER
							, GNStorage<CRxWriteStream>
	#endif
	{
		_LPMSGBUF pBuf;

		CRxWriteStream(_LPMSGBUF buf) : pBuf(buf) {}

		// 기존에 있던 부분
		virtual BOOL Write(const void * dat, DWORD size)
		{
			if (dat)
				pBuf->AddRight((void*)dat, size);
			return TRUE;
		}

		virtual BOOL Read(void *, DWORD)
		{
			return FALSE;
		}

		operator IRxStream*() { return this; }
	};



}
