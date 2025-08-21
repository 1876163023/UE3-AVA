//
// fileCache.h
//
// deathcure@redduck.com
//---------------------------------------------------------------------------
#ifndef fileCacheH
#define fileCacheH
//---------------------------------------------------------------------------
#include <windows.h>
//---------------------------------------------------------------------------
struct fileCache_t {
#define Version 0x100
#define MaxFileNameLength 512
#define DesiredCachePath TEXT("..\\avaGame\\Content\\Maps\\Cache\\")
	TCHAR desiredPath[MaxFileNameLength];

	enum optimizeOption_t {
		ooSpeed,
		ooSize,
		ooEndOfType
	};
	optimizeOption_t optimizeOption;

	struct header_t {
		int version;
		int nFiles;
		int nIndices;
		int nAccess;
	};

	struct fileList_t {
		TArray<FString> files;

		int find(TCHAR* fileName);
		bool insert(TCHAR* fileName);		
	};
	typedef struct fileList_t fileList_t;
	typedef struct fileList_t* pfileList_t;

	struct fcFileDesc_t {
		int id;
		TCHAR fileName[MaxFileNameLength];
		BY_HANDLE_FILE_INFORMATION fileInfo;

		bool saveToFile(HANDLE hFile, int *pfilePosition = 0x0);
		bool loadFromFile(HANDLE hFile, int *pfilePosition = 0x0);

		void set(int id = -1, TCHAR* fileName = 0x0);
	};
	typedef struct fcFileDesc_t fcFileDesc_t;
	typedef struct fcFileDesc_t* pfcFileDesc_t;

	struct fcFileDescs_t {
		TArray<fcFileDesc_t> descs;		
		int idSerial;

		bool getFileInfo(const TCHAR* fileName, BY_HANDLE_FILE_INFORMATION* pFileInfo);
		bool checkValid(fcFileDesc_t& fileDesc);
		pfcFileDesc_t find(const TCHAR* fileName);
		void clear(void);
		pfcFileDesc_t addFileDesc(const TCHAR* fileName);		

		fcFileDescs_t(void);
		~fcFileDescs_t(void);
	};

	struct fcIndex_t {
		bool bValid;
		int idFile;
		TCHAR fileName[MaxFileNameLength];
		HANDLE hFile;
		int offset;
		int nByte;
		int cacheOffset;

		bool compare(struct fcIndex_t& i) {
			if(bValid != i.bValid) return false;
			if(idFile != i.idFile) return false;
			if(_tcscmp(fileName, i.fileName)) return false;
			if(hFile != i.hFile) return false;
			if(offset != i.offset) return false;
			if(nByte != i.nByte) return false;
			if(cacheOffset != i.cacheOffset) return false;
			return true;
		}

		void copy(struct fcIndex_t& src) {
			_tcscpy(fileName, src.fileName);
			hFile = src.hFile;
			offset = src.offset;
			nByte = src.nByte;
			cacheOffset = src.cacheOffset;
		}

		void set(HANDLE _hFile = INVALID_HANDLE_VALUE, const TCHAR* _fileName = 0x0, int _offset = 0, int _nByte = 0, int _cacheOffset = 0, int _idFile = -1) {			
			hFile = _hFile;
			if(_fileName) _tcscpy(fileName, _fileName);			
			else _tcscpy(fileName, TEXT(""));
			offset = _offset;
			nByte = _nByte;
			cacheOffset = _cacheOffset;			
			idFile = _idFile;
		}

		static int fileSize(void) {return sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int);}

		bool saveToFile(HANDLE hFile, int* pfilePosition = 0x0) {
			if(INVALID_HANDLE_VALUE!=hFile) {
				DWORD nByteWritten = 0;
				WriteFile(hFile, &idFile, sizeof(int), &nByteWritten, 0x0); 
				if(nByteWritten != sizeof(int)) return false; 
				if(pfilePosition) *pfilePosition += sizeof(int);

				WriteFile(hFile, &offset, sizeof(int), &nByteWritten, 0x0); 
				if(nByteWritten != sizeof(int)) return false; 
				if(pfilePosition) *pfilePosition += sizeof(int);

				WriteFile(hFile, &nByte, sizeof(int), &nByteWritten, 0x0); 
				if(nByteWritten != sizeof(int)) return false; 
				if(pfilePosition) *pfilePosition += sizeof(int);

				WriteFile(hFile, &cacheOffset, sizeof(int), &nByteWritten, 0x0); 
				if(nByteWritten != sizeof(int)) return false; 
				if(pfilePosition) *pfilePosition += sizeof(int);

				return true;
			}
			return false;
		}

		bool loadFromFile(HANDLE hFile, int* pfilePosition = 0x0) {
			if(INVALID_HANDLE_VALUE!=hFile) {
				set();
				DWORD nByteRead = 0;
				ReadFile(hFile, &idFile, sizeof(int), &nByteRead, 0x0); 
				if(nByteRead != sizeof(int)) return false; 
				if(pfilePosition) *pfilePosition += sizeof(int);

				ReadFile(hFile, &offset, sizeof(int), &nByteRead, 0x0); 
				if(nByteRead != sizeof(int)) return false; 
				if(pfilePosition) *pfilePosition += sizeof(int);

				ReadFile(hFile, &nByte, sizeof(int), &nByteRead, 0x0); 
				if(nByteRead != sizeof(int)) return false; 
				if(pfilePosition) *pfilePosition += sizeof(int);

				ReadFile(hFile, &cacheOffset, sizeof(int), &nByteRead, 0x0); 
				if(nByteRead != sizeof(int)) return false; 
				if(pfilePosition) *pfilePosition += sizeof(int);
				return true;
			}
			return false;
		}

		fcIndex_t(void) {
			set();
			bValid = true;
		}
	};
	typedef struct fcIndex_t fcIndex_t;
	typedef struct fcIndex_t* pfcIndex_t;

	bool bCheckCacheConsistency;
	bool bGenerateScheduledCache;
	int numCacheToGenerate;
	int nAccess;
	float dropCacheThreshold;
	pfcIndex_t pIndices;
	int nIndices;
	pfcIndex_t pCurIndex;

	pfcFileDesc_t pFileDescs;
	int nFileDescs;	
	
	void clearIndices(void);
	
	HANDLE hCacheFile;
	int currentCacheOffset;

	struct buffer_t {
		int nSize;
		void* ptr;

		void* resize(int nByte);
		buffer_t(void);
		~buffer_t(void);
	};
	typedef struct buffer_t buffer_t;
	typedef struct buffer_t* pbuffer_t;

	// {{ cache ±Á±â
	struct writer_t {
		fcFileDescs_t fileDescs;
		TArray<fcIndex_t> indices;
		pfcIndex_t pPrevIndex;		
		int nAccess;

		bool bEnabled;
		void setEnable(bool bEnable = true);
		bool isEnabled(void);
		bool onOpenFile(HANDLE hFile, const TCHAR* fileName);
		bool onSeek(HANDLE hFile, int offset);
		bool verifyOffset(HANDLE hFile, int offset);
		bool onReadFile(HANDLE hFile, int nByte);		
		void mergeIndices(void);
		bool begin(void);
		bool writeIndices(HANDLE hFile);
		bool writeData(HANDLE hFile);
		bool end(TCHAR* indexFileName);
		bool loadFromFile(TCHAR* indexFileName);
		
		bool makeCacheFromIndex(TCHAR* cacheFileName, TCHAR* indexFileName);

		writer_t(void);
		~writer_t(void);
	};	
	typedef struct writer_t writer_t;
	typedef struct writer_t* pwriter_t;

	writer_t writer;
	// }} cache ±Á±â	

	fileList_t schedule;
	TMap<DWORD, fcIndex_t> handledFiles;

	bool openCache(TCHAR* fileName);
	void closeCache(void);
	pfcIndex_t findForward(HANDLE hFile = INVALID_HANDLE_VALUE, int idFile = -1/*const TCHAR* fileName = 0x0*/, int offset = -1, int nByte = -1);
	pfcIndex_t findBackward(HANDLE hFile = INVALID_HANDLE_VALUE, int idFile = -1/*const TCHAR* fileName = 0x0*/, int offset = -1, int nByte = -1);

	bool bEnabled;
	DWORD timeLoadFromCache;
	int nLookup;
	int nCacheHit;
	int nCacheMiss;
	bool bCached;
	bool bBeginLoadMap;
	TCHAR cacheFileName[MaxFileNameLength];	
	TCHAR indexFileName[MaxFileNameLength]; 

	int idFileCurrent;
	TCHAR fileNameCurrent[MaxFileNameLength];

	void onBeginLoadMap(const TCHAR* levelFileName);
	bool onOpenFile(HANDLE hFile, const TCHAR* fileName);
	bool onSeek(HANDLE hFile, int offset);
	bool verifyOffset(HANDLE hFile, int offset);
	bool onReadFile(HANDLE hFile, void* pData, int nByte, DWORD* nByteRead);
	bool onCloseFile(HANDLE hFile);
	void onEndLoadMap(void);

	static bool convLevelFileNameToCacheFileName(TCHAR* cacheFileName, const TCHAR* levelFileName);
	static bool convLevelFileNameToIndexFileName(TCHAR* cacheFileName, const TCHAR* levelFileName);
	static void _log(TCHAR* fileName, TCHAR* str);
	static void logToFile(TCHAR* fileName, TCHAR* fmt, ...);
	
	void generateScheduledCache(void);
	bool checkCacheConsistency(TCHAR* cacheFileName);
	void checkCacheConsistencies(void);

	bool makeFullPath(TCHAR* result, bool* pbReadOnly, TCHAR* fileName, TCHAR* desiredPath, bool bMkDir);
	TCHAR* makeCacheFileName(TCHAR* cacheFileName, TCHAR* indexFileName);

	void setEnable(bool bEnable = true);

	bool init(void);
	void deinit(void);

	fileCache_t(void);
	~fileCache_t(void);
};
typedef struct fileCache_t fileCache_t;
typedef struct fileCache_t* pfileCache_t;
//---------------------------------------------------------------------------
extern fileCache_t GfileCache;
//---------------------------------------------------------------------------
#endif