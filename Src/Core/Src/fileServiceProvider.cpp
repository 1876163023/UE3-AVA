//
// fileServiceProvider.cpp
//
// deathcure@redduck.com
//---------------------------------------------------------------------------
#include "CorePrivate.h"
#include "fileCache.h"
#include <tchar.h>
#include <io.h>
#include <stdio.h>
//---------------------------------------------------------------------------
#include "fileServiceProvider.h"
#include "fileCache.h"
#include "../../avaLaunch/Blowfish.h"

using namespace layeredFileService;
//---------------------------------------------------------------------------
// global def
//---------------------------------------------------------------------------
//#define EnableNativeLog // 로그를 생성하려면 활성화

#ifdef LogToFile
#undef LogToFile
#endif

#ifdef EnableNativeLog
#define LogToFile layeredFileService::logToFile
#else
#define LogToFile __noop
#endif

pfileServiceProvider_t GfileServiceProvider = 0x0;
//---------------------------------------------------------------------------
// layeredFileService
//---------------------------------------------------------------------------
void layeredFileService::_log(TCHAR* fileName, TCHAR* str)
{
	FILE* fp = _tfopen(fileName, TEXT("at"));
	if(fp) {
		_ftprintf(fp, TEXT("%s\n"), str);		
		fclose(fp);
	}
}
//---------------------------------------------------------------------------
void layeredFileService::logToFile(TCHAR* fileName, TCHAR* fmt, ...)
{
	static TCHAR defaultFileName[] = TEXT("fileServiceProvider.log");
	if(fileName == 0x0) {
		fileName = defaultFileName;
	}
	va_list argPtr;
	va_start( argPtr, fmt );		
	TCHAR str[1024];
	_vsntprintf( str, 1024, fmt, argPtr );
	_log(fileName, str);
	va_end( argPtr );
}
//---------------------------------------------------------------------------
// layeredFileService::buffer_t
//---------------------------------------------------------------------------
void* layeredFileService::buffer_t::resize(int nByte) 
{	
	if(nByte<=0) {
		LogToFile(0x0, TEXT("[fileCache_t::buffer_t::resize] invalid parameter:%d"), nByte);
	}
	if(nByte>0 && nSize < nByte) {
		if(ptr) free(ptr);
		ptr = malloc(nByte);
		if(ptr) {
			nSize = nByte;
		}
	}
	return ptr;
}
//---------------------------------------------------------------------------
void layeredFileService::buffer_t::clear(void)
{
	if(ptr) free(ptr);
	ptr = 0x0;
}
//---------------------------------------------------------------------------
layeredFileService::buffer_t::buffer_t(void) 
{
	nSize = 0;
	ptr = 0x0;
}
//---------------------------------------------------------------------------
layeredFileService::buffer_t::~buffer_t(void) 
{
	clear();
}
//---------------------------------------------------------------------------
// fileServiceProvider_t::service_t
//---------------------------------------------------------------------------
DWORD layeredFileService::service_t::msgProc(DWORD sender, DWORD msg, DWORD param1, DWORD param2) 
{
	switch(msg) {
		case fmEnable:
			bEnabled = param1 ? true: false;
			return (DWORD)bEnabled;
		
		case fmGetId:
			return id;
	}
	return 0xffffffff;
}
//---------------------------------------------------------------------------
bool layeredFileService::service_t::onCreate(HANDLE hFile, const TCHAR* fileName, bool bWrite)
{
	return GfileServiceProvider ? GfileServiceProvider->onCreate(idLayer-1, hFile, fileName, bWrite) : false;
}
//---------------------------------------------------------------------------
bool layeredFileService::service_t::onRead(HANDLE hFile, void* buffer, int nByte, DWORD* nByteRead)
{
	return GfileServiceProvider ? GfileServiceProvider->onRead(idLayer-1, hFile, buffer, nByte, nByteRead) : false;
}
//---------------------------------------------------------------------------
bool layeredFileService::service_t::onWrite(HANDLE hFile, void* buffer, int nByte, DWORD* nByteWritten)
{
	return GfileServiceProvider ? GfileServiceProvider->onWrite(idLayer-1, hFile, buffer, nByte, nByteWritten) : false;
}
//---------------------------------------------------------------------------
bool layeredFileService::service_t::onSeek(HANDLE hFile, int pos)
{
	return GfileServiceProvider ? GfileServiceProvider->onSeek(idLayer-1, hFile, pos) : false;
}
//---------------------------------------------------------------------------
bool layeredFileService::service_t::onClose(HANDLE hFile)
{
	return GfileServiceProvider ? GfileServiceProvider->onClose(idLayer-1, hFile) : false;
}
//---------------------------------------------------------------------------
bool layeredFileService::service_t::onMove(const TCHAR* dest, const TCHAR* src)
{
	return GfileServiceProvider ? GfileServiceProvider->onMove(idLayer-1, dest, src) : false;
}
//---------------------------------------------------------------------------
layeredFileService::service_t::service_t(void) 
{
	bEnabled = false;
	id = fiNotAssigned;
	//LogToFile(0x0, TEXT("[service_t::service_t]"));
}
//---------------------------------------------------------------------------
layeredFileService::service_t::~service_t(void)
{
	//LogToFile(0x0, TEXT("[service_t::~service_t]"));
}
//---------------------------------------------------------------------------
// fileServiceProvider_t
//---------------------------------------------------------------------------
DWORD fileServiceProvider_t::msgProc(DWORD sender, DWORD receiver, DWORD msg, DWORD param1, DWORD param2)
{
	if(fiFileServiceProvider == receiver) {
		switch(msg) {
			case fmNop: break;
			default:
				return __super::msgProc(sender, msg, param1, param2);
		}
	}
	else {
		for(int lpp=0;lpp<services.Num();lpp++) {
			pservice_t pService = services(lpp);
			if(pService && receiver == pService->id) {
				return pService->msgProc(sender, msg, param1, param2);
			}
		}
	}
	return 0xffffffff;
}

//---------------------------------------------------------------------------
bool fileServiceProvider_t::onCreate(HANDLE hFile, const TCHAR* fileName, bool bWrite) 
{	
	if(bEnabled) {
		for(int lpp=services.Num()-1;lpp>=0;--lpp) {
			pservice_t pService = services(lpp);
			if(pService && pService->bEnabled) {
				if(pService->onCreate(hFile, fileName, bWrite)) {
					return true;
				}
			}
		}
	}	
	return false;
}
//---------------------------------------------------------------------------
bool fileServiceProvider_t::onRead(HANDLE hFile, void* buffer, int nByte, DWORD* nByteRead)
{
	if(bEnabled) {
		for(int lpp=services.Num()-1;lpp>=0;--lpp) {
			pservice_t pService = services(lpp);
			if(pService && pService->bEnabled) {
				return pService->onRead(hFile, buffer, nByte, nByteRead);					
			}
		}
	}	
	return false;
}
//---------------------------------------------------------------------------
bool fileServiceProvider_t::onWrite(HANDLE hFile, void* buffer, int nByte, DWORD* nByteWritten) 
{
	if(bEnabled) {
		for(int lpp=services.Num()-1;lpp>=0;--lpp) {
			pservice_t pService = services(lpp);
			if(pService && pService->bEnabled) {
				return pService->onWrite(hFile, buffer, nByte, nByteWritten);
			}
		}
	}	
	return false;
}
//---------------------------------------------------------------------------
bool fileServiceProvider_t::onSeek(HANDLE hFile, int position) 
{
	if(bEnabled) {
		for(int lpp=services.Num()-1;lpp>=0;--lpp) {
			pservice_t pService = services(lpp);
			if(pService && pService->bEnabled) {
				return pService->onSeek(hFile, position);					
			}
		}
	}	
	return false;
}
//---------------------------------------------------------------------------
bool fileServiceProvider_t::onClose(HANDLE hFile) 
{
	if(bEnabled) {
		for(int lpp=services.Num()-1;lpp>=0;--lpp) {
			pservice_t pService = services(lpp);
			if(pService && pService->bEnabled) {
				return pService->onClose(hFile);
			}
		}
	}	
	return false;
}
//---------------------------------------------------------------------------
bool fileServiceProvider_t::onMove(const TCHAR* dest, const TCHAR* src)
{
	if(bEnabled) {
		for(int lpp=services.Num()-1;lpp>=0;--lpp) {
			pservice_t pService = services(lpp);
			if(pService && pService->bEnabled) {
				return pService->onMove(dest, src);
			}
		}
	}	
	return false;
}
//---------------------------------------------------------------------------
bool fileServiceProvider_t::onCreate(int idLayer, HANDLE hFile, const TCHAR* fileName, bool bWrite)
{
	if(idLayer>=0 && idLayer < services.Num()) {
		pservice_t pService = services(idLayer);
		if(pService) {
			return pService->onCreate(hFile, fileName, bWrite);
		}
	}	
	return false;
}
//---------------------------------------------------------------------------
bool fileServiceProvider_t::onRead(int idLayer, HANDLE hFile, void* buffer, int nByte, DWORD* nByteRead)
{
	if(idLayer>=0 && idLayer < services.Num()) {
		pservice_t pService = services(idLayer);
		if(pService) {
			return pService->onRead(hFile, buffer, nByte, nByteRead);
		}
	}	
	return false;
}
//---------------------------------------------------------------------------
bool fileServiceProvider_t::onWrite(int idLayer, HANDLE hFile, void* buffer, int nByte, DWORD* nByteWritten)
{
	if(idLayer>=0 && idLayer < services.Num()) {
		pservice_t pService = services(idLayer);
		if(pService) {
			return pService->onWrite(hFile, buffer, nByte, nByteWritten);
		}
	}	
	return false;
}
//---------------------------------------------------------------------------
bool fileServiceProvider_t::onSeek(int idLayer, HANDLE hFile, int position)
{
	if(idLayer>=0 && idLayer < services.Num()) {
		pservice_t pService = services(idLayer);
		if(pService) {
			return pService->onSeek(hFile, position);
		}
	}
	return false;
}
//---------------------------------------------------------------------------
bool fileServiceProvider_t::onClose(int idLayer, HANDLE hFile)
{
	if(idLayer>=0 && idLayer < services.Num()) {
		pservice_t pService = services(idLayer);
		if(pService) {
			return pService->onClose(hFile);
		}
	}	
	return false;
}
//---------------------------------------------------------------------------
bool fileServiceProvider_t::onMove(int idLayer, const TCHAR* dest, const TCHAR* src)
{
	if(idLayer>=0 && idLayer < services.Num()) {
		pservice_t pService = services(idLayer);
		if(pService) {
			return pService->onMove(dest, src);
		}
	}
	return false;
}
//---------------------------------------------------------------------------
bool fileServiceProvider_t::addService(pservice_t pService)
{
	if(pService) {
		pService->idLayer = idLayerSerial++;
		services.Push(pService);
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------
bool fileServiceProvider_t::init(void)
{
	bool bHandled = false;
	for(int lpp=0;lpp<services.Num();lpp++) {
		pservice_t pService = services(lpp);
		if(pService && pService->bEnabled) {
			if(pService->init()) {
				bHandled = true;
			}
		}
	}
	return bHandled;
}
//---------------------------------------------------------------------------
void fileServiceProvider_t::deinit(void)
{
	for(int lpp=services.Num()-1;lpp>=0;--lpp) {
		pservice_t pService = services(lpp);
		if(pService && pService->bEnabled) {
			pService->deinit();
		}
	}
}
//---------------------------------------------------------------------------
fileServiceProvider_t::fileServiceProvider_t(void) 
{
	idLayerSerial = 0x0;
	bEnabled = true;
	bInitialized = false;
	id = fiFileServiceProvider;
	GfileServiceProvider = this;
	LogToFile(0x0, TEXT("[fileServiceProvider_t::fileServiceProvider_t]"));
}
//---------------------------------------------------------------------------
fileServiceProvider_t::~fileServiceProvider_t(void) 
{
	if(GfileServiceProvider == this) {
		GfileServiceProvider = 0x0;
	}
	services.Empty();
	LogToFile(0x0, TEXT("[fileServiceProvider_t::~fileServiceProvider_t]"));
}
//---------------------------------------------------------------------------
// layeredFileService global function def
//---------------------------------------------------------------------------
DWORD layeredFileService::sendFspMsg(DWORD sender, DWORD receiver, DWORD msg, DWORD param1, DWORD param2)
{
	if(GfileServiceProvider) {
		return GfileServiceProvider->msgProc(sender, receiver, msg, param1, param2);
	}
	return 0xffffffff;
}
//---------------------------------------------------------------------------
const TCHAR* layeredFileService::getFileNameExt(const TCHAR* fileName)
{	
	if(fileName) {
		int len = _tcslen(fileName);
		for(int lpp=len-2;lpp>=0;lpp--) {
			if(fileName[lpp] == TEXT('.')) {
				return &fileName[lpp+1];
			}
		}
	}
	return 0x0;	
}
//---------------------------------------------------------------------------
// fileServiceProvider implementation
//---------------------------------------------------------------------------
// begin implementation def
enum serviceMsg_t {
	smBeginLoadMap = fmEndOfType,
	smEndLoadMap,
	smEndOfType
};
//---------------------------------------------------------------------------
enum serviceType_t {		
	stCryptographer = fiEndOfType,
	stFileDecrypter,
	stCacheProvider,
	stCache,
	stEndOfType
};
//---------------------------------------------------------------------------
struct scriptPackageCryptographer_t : service_t {

	static bool generateKey(char* key, int len) {
		if(key) {
			memset(key, 0x0, len);

			extern INT GAVABuiltFromChangeList;
			srand(GAVABuiltFromChangeList);

			for(int lpp=0;lpp<len;lpp++) {
				char b = 0;
				while(!b) b=rand();
				key[lpp] = b;
			}
			return true;
		}
		return false;		
	}

	static bool encrypt(void* buffer, int len) {
		char pw[5];

		generateKey(pw, 4);
		pw[4] = 0x0;

		Blowfish Blowfish;
		Blowfish.Set_Passwd( pw );
		Blowfish.Encrypt(buffer, len);
		return true;
	}

	static bool decrypt(void* buffer, int len) {
		char pw[5];

		generateKey(pw, 4);
		pw[4] = 0x0;

		Blowfish Blowfish;
		Blowfish.Set_Passwd( pw );
		Blowfish.Decrypt(buffer, len);
		return true;
	}

	struct file_t : service_t {
		TCHAR fileName[512];
		buffer_t buffer;
		int offset;
		int fileSize;
		int nByteReadSum;
		bool bVerify;

		virtual bool onCreate(HANDLE hFile, const TCHAR* fileName, bool bWrite);
		virtual bool onRead(HANDLE hFile, void* buffer, int nByte, DWORD* nByteRead);
		virtual bool onSeek(HANDLE hFile, int position);
		virtual bool onClose(HANDLE hFile);

		void set(const TCHAR* fileName = 0x0, int fileSize = 0, int offset = 0);

		file_t(void) {
			id = stFileDecrypter;
			nByteReadSum = 0x0;
			bVerify = true;
		}
	};
	typedef struct file_t file_t;
	typedef struct file_t* pfile_t;

	TMap<DWORD, file_t> files;

	virtual bool onCreate(HANDLE hFile, const TCHAR* fileName, bool bWrite);
	virtual bool onRead(HANDLE hFile, void* buffer, int nByte, DWORD* nByteRead);
	virtual bool onSeek(HANDLE hFile, int position);
	virtual bool onClose(HANDLE hFile);
	virtual bool onMove(const TCHAR* dest, const TCHAR* src);

	scriptPackageCryptographer_t(void);
	virtual ~scriptPackageCryptographer_t(void);
};
typedef struct scriptPackageCryptographer_t scriptPackageCryptographer_t;
typedef struct scriptPackageCryptographer_t* pscriptPackageCryptographer_t;
//---------------------------------------------------------------------------
struct mapCacheProvider_t : service_t {
	fileCache_t fileCache; // 통합전 임시
	/*struct file_t : service_t {
	TCHAR fileName[512];
	buffer_t buffer;
	int offset;

	virtual bool onCreate(HANDLE hFile, const TCHAR* fileName, bool bWrite);
	virtual bool onRead(HANDLE hFile, void* buffer, int nByte, DWORD* nByteRead);
	virtual bool onSeek(HANDLE hFile, int position);
	virtual bool onClose(HANDLE hFile);

	void set(TCHAR* fileName = 0x0, int offset = 0);
	};
	typedef struct file_t file_t;
	typedef struct file_t* pfile_t;

	TMap<HANDLE, file_t> files;*/

	virtual DWORD msgProc(DWORD sender, DWORD msg, DWORD param1, DWORD param2);
	virtual bool onCreate(HANDLE hFile, const TCHAR* fileName, bool bWrite);
	virtual bool onRead(HANDLE hFile, void* buffer, int nByte, DWORD* nByteRead);
	virtual bool onSeek(HANDLE hFile, int position);
	virtual bool onClose(HANDLE hFile);

	mapCacheProvider_t(void);
	virtual ~mapCacheProvider_t(void);
};
typedef struct mapCacheProvider_t mapCacheProvider_t;
typedef struct mapCacheProvider_t* pmapCacheProvider_t;
// end implementation def
//---------------------------------------------------------------------------
// scriptPackageCryptographer_t::file_t
//---------------------------------------------------------------------------
bool scriptPackageCryptographer_t::file_t::onCreate(HANDLE hFile, const TCHAR* fileName, bool bWrite) 
{
	set(fileName, GetFileSize(hFile, 0x0));

	void* pBuffer = buffer.resize(fileSize);
	if(pBuffer) {
		DWORD nByteRead = 0;
		ReadFile(hFile, pBuffer, fileSize, &nByteRead, 0x0);
		SetFilePointer(hFile, 0, 0x0, FILE_BEGIN);
		if(nByteRead == fileSize) {
			decrypt(pBuffer, fileSize);
			LogToFile(0x0, TEXT("[scriptPackageCryptographer_t::file_t::onCreate] %s fileSize:%d"), fileName, fileSize);
			return true;
		}
	}	
	return false;
}
//---------------------------------------------------------------------------
bool scriptPackageCryptographer_t::file_t::onRead(HANDLE hFile, void* _buffer, int nByte, DWORD* nByteRead) 
{	
	DWORD _local_nByteRead = 0x0;	
	if(0x0 == nByteRead) nByteRead = &_local_nByteRead;

	*nByteRead = offset + nByte <= fileSize ? nByte : fileSize - offset;
	*nByteRead = *nByteRead > 0 ? *nByteRead : 0;
	if(*nByteRead) {
		memcpy(_buffer, &((char*)(buffer.ptr))[offset], *nByteRead);
		//LogToFile(0x0, TEXT("[scriptPackageCryptographer_t::file_t::onRead] offset:%d nByteRead = %d (sum:%d) on %s"), offset, *nByteRead, nByteReadSum, fileName);

		offset += *nByteRead;
		nByteReadSum += *nByteRead;

		/*if(bVerify) {
			buffer_t verifyBuffer;
			void* pBufferVerify = verifyBuffer.resize(nByte);
			if(pBufferVerify) {
				DWORD nByteReadVerify = 0x0;
				ReadFile(hFile, pBufferVerify, nByte, &nByteReadVerify, 0x0);
				if(nByteReadVerify) {
					if(memcmp(pBufferVerify, _buffer, nByteReadVerify)) {
						LogToFile(0x0, TEXT("[scriptPackageCryptographer_t::file_t::onRead] verify failed"));
					}
					else {
						LogToFile(0x0, TEXT("[scriptPackageCryptographer_t::file_t::onRead] verified"));
					}
				}
			}
		}*/
		return true;
	}
	LogToFile(0x0, TEXT("[scriptPackageCryptographer_t::file_t::onRead] failed offset:%d nByte:%d fileSize:%d on %s"), offset, nByte, fileSize, fileName);
	return false;
}
//---------------------------------------------------------------------------
bool scriptPackageCryptographer_t::file_t::onSeek(HANDLE hFile, int position)
{	
	if(position <= fileSize) {
		offset = position;
		//LogToFile(0x0, TEXT("[scriptPackageCryptographer_t::file_t::onSeek] offset:%d fileSize:%d on %s"), offset, fileSize, fileName);

		/*if(bVerify) {
			SetFilePointer(hFile, position, 0x0, FILE_BEGIN);
		}*/
		return true;
	}
	LogToFile(0x0, TEXT("[scriptPackageCryptographer_t::file_t::onSeek] failed offset:%d fileSize:%d on %s"), offset, fileSize, fileName);
	return false;
}
//---------------------------------------------------------------------------
bool scriptPackageCryptographer_t::file_t::onClose(HANDLE hFile) 
{
	buffer.clear();
	LogToFile(0x0, TEXT("[scriptPackageCryptographer_t::file_t::onClose] %s"), fileName);
	return true;
}
//---------------------------------------------------------------------------
void scriptPackageCryptographer_t::file_t::set(const TCHAR* _fileName, int _fileSize, int _offset)
{	
	_tcscpy(fileName, 0x0 == _fileName? TEXT("") : _fileName);
	fileSize = _fileSize;
	offset = _offset;
	nByteReadSum = 0;
}
//---------------------------------------------------------------------------
// scriptPackageCryptographer_t
//---------------------------------------------------------------------------
bool scriptPackageCryptographer_t::onCreate(HANDLE hFile, const TCHAR* fileName, bool bWrite)
{	
	pfile_t pFile = files.Find((DWORD)hFile);
	if(pFile) {
		LogToFile(0x0, TEXT("[scriptPackageCryptographer_t::onCreate] file 0x%x:%s already registered."), hFile, fileName);
		return pFile->onCreate(hFile, fileName, bWrite);
	}
	
	const TCHAR* fileExt = getFileNameExt(fileName);
	if(!bWrite && fileExt && !_tcsicmp(fileExt, TEXT("u"))) {
		file_t file;		
		if(files.Set((DWORD)hFile, file).onCreate(hFile, fileName, bWrite)) {
			LogToFile(0x0, TEXT("[scriptPackageCryptographer_t::onCreate] file 0x%x:%s registered."), hFile, fileName);
			return true;
		}
		files.Remove((DWORD)hFile);
	}

	return __super::onCreate(hFile, fileName, bWrite);
}
//---------------------------------------------------------------------------
bool scriptPackageCryptographer_t::onRead(HANDLE hFile, void* buffer, int nByte, DWORD* nByteRead)
{
	pfile_t pFile = files.Find((DWORD)hFile);
	if(pFile) {
		return pFile->onRead(hFile, buffer, nByte, nByteRead);
	}

	return __super::onRead(hFile, buffer, nByte, nByteRead);
}
//---------------------------------------------------------------------------
bool scriptPackageCryptographer_t::onSeek(HANDLE hFile, int position)
{
	pfile_t pFile = files.Find((DWORD)hFile);
	if(pFile) {
		return pFile->onSeek(hFile, position);
	}

	return __super::onSeek(hFile, position);
}
//---------------------------------------------------------------------------
bool scriptPackageCryptographer_t::onClose(HANDLE hFile)
{
	pfile_t pFile = files.Find((DWORD)hFile);
	if(pFile) {
		pFile->onClose(hFile);
		LogToFile(0x0, TEXT("[scriptPackageCryptographer_t::onClose] file 0x%x:%s closed."), hFile, pFile->fileName);
		files.Remove((DWORD)hFile);		
		return true;
	}

	return __super::onClose(hFile);
}
//---------------------------------------------------------------------------
bool scriptPackageCryptographer_t::onMove(const TCHAR* dest, const TCHAR* src)
{
	bool bSuccess = false;
	const TCHAR* destExt = getFileNameExt(dest);
	const TCHAR* srcExt = getFileNameExt(src);
	if(destExt && srcExt && !_tcsicmp(destExt, TEXT("u")) && !_tcsicmp(srcExt, TEXT("tmp"))) {
		HANDLE hFile = CreateFile(dest, GENERIC_READ, FILE_SHARE_READ, 0x0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0x0);
		if(INVALID_HANDLE_VALUE != hFile) {
			int fileSize = GetFileSize(hFile, 0x0);
			if(fileSize) {
				buffer_t buffer;
				void* pBuffer = buffer.resize(fileSize);
				if(pBuffer) {
					DWORD nByteRead = 0x0;
					ReadFile(hFile, buffer.ptr, fileSize, &nByteRead, 0x0);
					CloseHandle(hFile);
					hFile = INVALID_HANDLE_VALUE;

					if(fileSize == nByteRead) {
						bSuccess = encrypt(buffer.ptr, fileSize);

						HANDLE hFileCipher = CreateFile(dest, GENERIC_WRITE, 0, 0x0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0x0);
						if(hFileCipher) {
							DWORD nByteWritten = 0;
							WriteFile(hFileCipher, buffer.ptr, fileSize, &nByteWritten, 0x0); 
							CloseHandle(hFileCipher);
							if(fileSize == nByteWritten) {
								bSuccess = true;
							}
						}					
					}
					else {
						LogToFile(0x0, TEXT("[scriptPackageEncryptProvider_t::onMove] ReadFile failed for %s"), src);
					}
				}			
			}		
			if(INVALID_HANDLE_VALUE != hFile) {
				CloseHandle(hFile);
			}
		}
	}

	return bSuccess || __super::onMove(dest, src);
}
//---------------------------------------------------------------------------
scriptPackageCryptographer_t::scriptPackageCryptographer_t(void)
{
	id = stCryptographer;
	bEnabled = true;
	LogToFile(0x0, TEXT("[scriptPackageCryptographer_t::scriptPackageCryptographer_t]"));
}
//---------------------------------------------------------------------------
scriptPackageCryptographer_t::~scriptPackageCryptographer_t(void)
{
	files.Empty();
	LogToFile(0x0, TEXT("[scriptPackageCryptographer_t::~scriptPackageCryptographer_t]"));
}
//---------------------------------------------------------------------------
// mapCacheProvider_t
//---------------------------------------------------------------------------
DWORD mapCacheProvider_t::msgProc(DWORD sender, DWORD msg, DWORD param1, DWORD param2)
{
	switch(msg) {
		case smBeginLoadMap:
			fileCache.onBeginLoadMap((TCHAR*)param1);
			return 1;

		case smEndLoadMap:
			fileCache.onEndLoadMap();
			return 1;

		case fmNop: break;
		default:
			return __super::msgProc(sender, msg, param1, param2);
	}
	return 0xffffffff;
}
//---------------------------------------------------------------------------
bool mapCacheProvider_t::onCreate(HANDLE hFile, const TCHAR* fileName, bool bWrite)
{
	// map loading 상태인지 테스트해봐야한다.

	if(!bWrite) {
		fileCache.onOpenFile(hFile, fileName);
		return true;
	}	
	return __super::onCreate(hFile, fileName, bWrite);
}
//---------------------------------------------------------------------------
bool mapCacheProvider_t::onRead(HANDLE hFile, void* buffer, int nByte, DWORD* nByteRead)
{
	return fileCache.onReadFile(hFile, buffer, nByte, nByteRead) || __super::onRead(hFile, buffer, nByte, nByteRead);
}
//---------------------------------------------------------------------------
bool mapCacheProvider_t::onSeek(HANDLE hFile, int position)
{
	return fileCache.onSeek(hFile, position) || __super::onSeek(hFile, position);
}
//---------------------------------------------------------------------------
bool mapCacheProvider_t::onClose(HANDLE hFile)
{
	return fileCache.onCloseFile(hFile) || __super::onClose(hFile);
}
//---------------------------------------------------------------------------
mapCacheProvider_t::mapCacheProvider_t(void)
{
	id = stCacheProvider;
	bEnabled = false;
	LogToFile(0x0, TEXT("[mapCacheProvider_t::mapCacheProvider_t]"));
}
//---------------------------------------------------------------------------
mapCacheProvider_t::~mapCacheProvider_t(void)
{
	LogToFile(0x0, TEXT("[mapCacheProvider_t::~mapCacheProvider_t]"));
}
//---------------------------------------------------------------------------
// global static func
//---------------------------------------------------------------------------
void fspStartup(void)
{
	if(0x0 == GfileServiceProvider) {
		new fileServiceProvider_t();
	}
	if(GfileServiceProvider && !GfileServiceProvider->bInitialized) {
		GfileServiceProvider->addService(new scriptPackageCryptographer_t()); // layer 0
		GfileServiceProvider->addService(new mapCacheProvider_t()); // layer 1
		
		GfileServiceProvider->bInitialized = true;
	}
	else {
		LogToFile(0x0, TEXT("fspStartup failed."));
	}
}
//---------------------------------------------------------------------------
void fspCleanup(void)
{
	delete GfileServiceProvider;
	GfileServiceProvider = 0x0;
}
//---------------------------------------------------------------------------