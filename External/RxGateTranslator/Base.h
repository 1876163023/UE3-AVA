#pragma once


namespace RxGate
{

	struct IRxStream
	{
	// 기존부터 있던 부분
		virtual BOOL Write(const void * dat, DWORD size) = 0;
		virtual BOOL Read (void * dat, DWORD size)  = 0;

	// 기본 타입 읽어 들이기
		virtual BOOL Read(char &dat) = 0;
		virtual BOOL Write(const char &dat) = 0;

		virtual BOOL Read(unsigned char &dat) = 0;
		virtual BOOL Write(const unsigned char &dat) = 0;

		virtual BOOL Read(short &dat) = 0;
		virtual BOOL Write(const short &dat) = 0;

		virtual BOOL Read(unsigned short &dat) = 0;
		virtual BOOL Write(const unsigned short &dat) = 0;

		virtual BOOL Read(long &dat) = 0;
		virtual BOOL Write(const long &dat) = 0;

		virtual BOOL Read(unsigned long &dat) = 0;
		virtual BOOL Write(const unsigned long &dat) = 0;

		virtual BOOL Read(__int64 &dat) = 0;
		virtual BOOL Write(const __int64 &dat) = 0;

		virtual BOOL Read(unsigned __int64 &dat) = 0;
		virtual BOOL Write(const unsigned __int64 &dat) = 0;

		virtual BOOL Read(unsigned  &dat) = 0;
		virtual BOOL Write(const unsigned  &dat) = 0;

		virtual BOOL Read(int &dat) = 0;
		virtual BOOL Write(const int &dat) = 0;

		virtual BOOL Read(float &dat) = 0;
		virtual BOOL Write(const float &dat) = 0;

		virtual BOOL Read(double &dat) = 0;
		virtual BOOL Write(const double &dat) = 0;

	// 기본 포인터 타입

		virtual BOOL Read(char *dat, const WORD &size) = 0;
		virtual BOOL Write(const char *dat, const WORD &size) = 0;

		virtual BOOL Read(unsigned char *dat, const WORD &size) = 0;
		virtual BOOL Write(const unsigned char *dat, const WORD &size) = 0;

		virtual BOOL Read(short *dat, const WORD &size) = 0;
		virtual BOOL Write(const short *dat, const WORD &size) = 0;

		virtual BOOL Read(unsigned short *dat, const WORD &size) = 0;
		virtual BOOL Write(const unsigned short *dat, const WORD &size) = 0;

		virtual BOOL Read(long *dat, const WORD &size) = 0;
		virtual BOOL Write(const long *dat, const WORD &size) = 0;

		virtual BOOL Read(unsigned long *dat, const WORD &size) = 0;
		virtual BOOL Write(const unsigned long *dat, const WORD &size) = 0;

		virtual BOOL Read(__int64 *dat, const WORD &size) = 0;
		virtual BOOL Write(const __int64 *dat, const WORD &size) = 0;

		virtual BOOL Read(unsigned __int64 *dat, const WORD &size) = 0;
		virtual BOOL Write(const unsigned __int64 *dat, const WORD &size) = 0;

		virtual BOOL Read(unsigned  *dat, const WORD &size) = 0;
		virtual BOOL Write(const unsigned  *dat, const WORD &size) = 0;

		virtual BOOL Read(int *dat, const WORD &size) = 0;
		virtual BOOL Write(const int *dat, const WORD &size) = 0;

		virtual BOOL Read(float *dat, const WORD &size) = 0;
		virtual BOOL Write(const float *dat, const WORD &size) = 0;

		virtual BOOL Read(double *dat, const WORD &size) = 0;
		virtual BOOL Write(const double *dat, const WORD &size) = 0;
	};

	struct IRxSerialize
	{
		virtual BOOL Write(IRxStream* stream) const = 0;
		virtual BOOL Read (IRxStream* stream)       = 0;
	};

	struct IRxWriter
	{
		virtual void Print(LPCSTR fmt, ...) = 0;
		virtual void Print(LPCWSTR fmt, ...) = 0;
	};

	struct IGNDumpEx
	{
		virtual void Dump(IRxWriter* writer) const = 0;
	};

}

