/***

	Copyright (c) Redduck Inc. All rights reserved.

	Project: ComDef

	Name: MsgBuf.h

	Description: Message buffer class for AVA clients.

***/

#pragma once



class IMemAlloc
{
public:
	virtual void* Alloc(unsigned long size) = 0;
	virtual void Free(void *mem) = 0;
};

class CDefaultMemAlloc : public IMemAlloc
{
public:
	void* Alloc(unsigned long size)
	{
		return operator new(size);
	}
	void Free(void *mem)
	{
		operator delete(mem);
	}
};



class CMsgBuf
{
private:
	BYTE *pMem;
	DWORD dwCap;
	DWORD dwPos;
	DWORD dwLen;

	void _Alloc(DWORD size);
	void _Free();
	void _Realloc(DWORD size);

public:
	CMsgBuf();
	virtual ~CMsgBuf();

	void Delete();
	LPBYTE GetData() { return pMem ? pMem + dwPos : NULL; }
	DWORD GetLength() { return dwLen; }
	DWORD GetCapacity() { return dwCap; }
	void Attach(LPBYTE mem, DWORD pos, DWORD len);
	LPBYTE Detach();
	void Clear();

	void SetLength(DWORD len);
	void Copy(CMsgBuf *pSrc);
	void Reserve(DWORD len);
	void SetData(DWORD pos, DWORD len, LPVOID pVoid);
	void AddRight(LPVOID pVoid, DWORD len);
	void RemoveLeft(DWORD len);

	friend CMsgBuf* _CreateMsgBuf(CMsgBuf *pSrc);
	friend CMsgBuf* _CreateMsgBufN(DWORD length);

	static CDefaultMemAlloc defaultAlloc;
	static IMemAlloc *pAlloc;
};


CMsgBuf* _CreateMsgBuf(CMsgBuf *pSrc = NULL);
CMsgBuf* _CreateMsgBufN(DWORD length);


#ifdef _AVA_SERVER

	#include <GateNet/GNBuf.h>

	typedef LPGNBUF _LPMSGBUF;

	inline _LPMSGBUF CreateMsgBuf(_LPMSGBUF pSrc)
	{
		return ::GNCreateBuf(pSrc);
	}

	inline _LPMSGBUF CreateMsgBufN(DWORD length)
	{
		return ::GNCreateBufN(length);
	}

#else

	typedef CMsgBuf* _LPMSGBUF;

	inline _LPMSGBUF CreateMsgBuf(_LPMSGBUF pSrc)
	{
		return _CreateMsgBuf(pSrc);
	}

	inline _LPMSGBUF CreateMsgBufN(DWORD length)
	{
		return _CreateMsgBufN(length);
	}

#endif


DWORD WINAPI CalcChecksum(LPVOID pBuf, int len);
