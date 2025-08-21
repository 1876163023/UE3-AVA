#pragma once

#include "RxMsg.h"


#if defined(_CPPRTTI)
#include <typeinfo>
#endif



namespace RxGate
{

	// type name

	#if defined(_CPPRTTI)
	//#pragma gMSG("Use Type Name")
	template <typename T>
	LPCSTR TypeName() { return typeid(T).name(); }
	#else
	//#pragma gMSG("Nouse Type Name")
	template <typename T>
	LPCSTR TypeName() { return "<Unknown>"; }
	#endif



	// dump out

	//void DumpOutV(LPCSTR fmt, va_list varg);
	void DumpOutF(LPCSTR fmt, ...);

	//void DumpOutV(LPCWSTR fmt, va_list varg);
	void DumpOutF(LPCWSTR fmt, ...);


	class CDumpWriter : public IRxWriter
	{
	public:
		virtual void Print(LPCSTR fmt, ...);
		virtual void Print(LPCWSTR fmt, ...);
	};


	#define DumpFunction(TYPE, OUTARG) \
		inline void OdlDump(const TYPE& val, IRxWriter* pWriter )  \
		{	pWriter->Print("<%s> value = " , TypeName<TYPE>()); pWriter->Print(OUTARG, val);}			

	DumpFunction(char, _T("%d")) 
	DumpFunction(short, _T("%d")) 
	DumpFunction(int, _T("%d")) 
	DumpFunction(long, _T("%d")) 
	DumpFunction(__int64, "%ld") 
	DumpFunction(unsigned __int64, "%lu") 
	DumpFunction(unsigned long, "%u") 
	DumpFunction(unsigned , "%u") 
	DumpFunction(unsigned short, "%u") 
	DumpFunction(unsigned char, "%u") 
	DumpFunction(float, "%f") 
	DumpFunction(double, "%lf") 


	template <typename T>
	inline void OdlDump(T* t, size_t size, IRxWriter *pWriter)
	{
		pWriter->Print(_T("{ "));
		while (size--)
		{
			OdlDump(*t++, pWriter);
			pWriter->Print(_T(" "));
		}
		pWriter->Print(_T("}"));
	}




	inline void OdlDump(const RXNERVE_ADDRESS &val, IRxWriter *pWriter)
	{
		pWriter->Print("<%s> value = " , TypeName<RXNERVE_ADDRESS>()); pWriter->Print("%8s", val.address);
	}

	inline void OdlDump(const _LPMSGBUF val, IRxWriter *pWriter)
	{
		pWriter->Print("<%s> length = %u" , TypeName<_LPMSGBUF>(), val->GetLength());
	}

	inline void OdlDump(const IRxMsg &val, IRxWriter *pWriter)
	{
		val.Dump(pWriter);
	}


}
