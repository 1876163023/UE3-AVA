//
// fileServiceProvider.h
//
// deathcure@redduck.com
//---------------------------------------------------------------------------
#ifndef fileServiceProviderH
#define fileServiceProviderH
//---------------------------------------------------------------------------
//#include "fileCache.h" // 통합전 임시,
//---------------------------------------------------------------------------
namespace layeredFileService {
	static void _log(TCHAR* fileName, TCHAR* str);
	static void logToFile(TCHAR* fileName, TCHAR* fmt, ...);

	struct service_t { // inherit this
		int id;
		int idLayer;
		bool bEnabled;

		virtual DWORD msgProc(DWORD sender, DWORD msg, DWORD param1, DWORD param2);
		virtual bool onCreate(HANDLE hFile, const TCHAR* fileName, bool bWrite);
		virtual bool onRead(HANDLE hFile, void* buffer, int nByte, DWORD* nByteRead);
		virtual bool onWrite(HANDLE hFile, void* buffer, int nByte, DWORD* nByteWritten);
		virtual bool onSeek(HANDLE hFile, int pos);
		virtual bool onClose(HANDLE hFile);
		virtual bool onMove(const TCHAR* dest, const TCHAR* src);

		virtual bool init(void) {return false;}
		virtual void deinit(void) {}

		service_t(void);
		virtual ~service_t(void);
	};
	typedef struct service_t service_t;
	typedef struct service_t* pservice_t;

	enum fspMsg_t {
		fmNop,
		fmEnable,
		fmGetId,
		fmGetIdLayer,
		fmEndOfType
	};

	enum fspId_t {
		fiNotAssigned,
		fiFileServiceProvider,
		fiEndOfType,
	};

	struct buffer_t {
		int nSize;
		void* ptr;

		void* resize(int nByte);
		void clear(void);

		buffer_t(void);
		~buffer_t(void);
	};
	typedef struct buffer_t buffer_t;
	typedef struct buffer_t* pbuffer_t;

	struct fileServiceProvider_t : service_t {
		int idLayerSerial;
		bool bInitialized;

		TArray<pservice_t> services;

		DWORD msgProc(DWORD sender, DWORD receiver, DWORD msg, DWORD param1, DWORD param2);
		virtual bool onCreate(HANDLE hFile, const TCHAR* fileName, bool bWrite);
		virtual bool onRead(HANDLE hFile, void* buffer, int nByte, DWORD* nByteRead);
		virtual bool onWrite(HANDLE hFile, void* buffer, int nByte, DWORD* nByteWritten);
		virtual bool onSeek(HANDLE hFile, int position);
		virtual bool onClose(HANDLE hFile);
		virtual bool onMove(const TCHAR* dest, const TCHAR* src);

		// {{ fsp specific recursive services
		bool onCreate(int idLayer, HANDLE hFile, const TCHAR* fileName, bool bWrite);
		bool onRead(int idLayer, HANDLE hFile, void* buffer, int nByte, DWORD* nByteRead);
		bool onWrite(int idLayer, HANDLE hFile, void* buffer, int nByte, DWORD* nByteWritten);
		bool onSeek(int idLayer, HANDLE hFile, int position);
		bool onClose(int idLayer, HANDLE hFile);
		bool onMove(int idLayer, const TCHAR* dest, const TCHAR* src);
		// }} fsp specific recursive services

		bool addService(pservice_t pService);

		virtual bool init(void);
		virtual void deinit(void);

		fileServiceProvider_t(void);
		virtual ~fileServiceProvider_t(void);
	};
	typedef struct fileServiceProvider_t fileServiceProvider_t;
	typedef struct fileServiceProvider_t* pfileServiceProvider_t;
	
	extern DWORD sendFspMsg(DWORD sender, DWORD receiver, DWORD msg, DWORD param1, DWORD param2);
	extern const TCHAR* getFileNameExt(const TCHAR* fileName);
};
//---------------------------------------------------------------------------
// global def
//---------------------------------------------------------------------------
extern layeredFileService::pfileServiceProvider_t GfileServiceProvider;
extern void fspStartup(void);
extern void fspCleanup(void);
//---------------------------------------------------------------------------
#endif