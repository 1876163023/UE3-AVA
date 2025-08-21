//
// fileCache.cpp
//
// deathcure@redduck.com
//---------------------------------------------------------------------------
#include "CorePrivate.h"
#include "fileCache.h"
#include <tchar.h>
#include <io.h>
#include <stdio.h>
//---------------------------------------------------------------------------
// global def
//---------------------------------------------------------------------------
fileCache_t GfileCache;
#define DefaultCheckCacheConsistency true
#define DefaultGenerateScheduledCache true
#define DefaultNumCacheToGenerate 3
#define DefaultDropCacheThreshold 0.1f
#define DefaultOptimizeOption ooSpeed
//#define SimulateCacheMiss // cache miss를 테스트할때 활성화
//#define EnableNativeLog // 로그를 생성하려면 활성화

#ifdef LogToFile
#undef LogToFile
#endif

#ifdef EnableNativeLog
#define LogToFile logToFile
#else
#define LogToFile __noop
#endif
//---------------------------------------------------------------------------
// fileCache_t::fileList_t
//---------------------------------------------------------------------------
int fileCache_t::fileList_t::find(TCHAR* fileName)
{
	for(int lpp=0;lpp<files.Num();lpp++) {
		if(files(lpp) == fileName) {
			return lpp;
		}
	}	
	return -1;
}
//---------------------------------------------------------------------------
bool fileCache_t::fileList_t::insert(TCHAR* fileName)
{
	int idx = find(fileName);
	if(-1 == idx) {
		files.Push(fileName);
		return true;
	}	
	return false;
}
//---------------------------------------------------------------------------
bool fileCache_t::fcFileDesc_t::saveToFile(HANDLE hFile, int *pfilePosition)
{	
	if(INVALID_HANDLE_VALUE != hFile) {
		DWORD nByteWritten = 0;
		WriteFile(hFile, fileName, MaxFileNameLength, &nByteWritten, 0x0); 
		if(nByteWritten != MaxFileNameLength) return false; 
		if(pfilePosition) *pfilePosition += MaxFileNameLength;

		WriteFile(hFile, &fileInfo, sizeof(BY_HANDLE_FILE_INFORMATION), &nByteWritten, 0x0); 
		if(nByteWritten != sizeof(BY_HANDLE_FILE_INFORMATION)) return false; 
		if(pfilePosition) *pfilePosition += sizeof(BY_HANDLE_FILE_INFORMATION);		
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------
bool fileCache_t::fcFileDesc_t::loadFromFile(HANDLE hFile, int * pfilePosition)
{
	if(INVALID_HANDLE_VALUE != hFile) {
		set();
		DWORD nByteRead = 0;
		ReadFile(hFile, fileName, MaxFileNameLength, &nByteRead, 0x0);
		if(nByteRead != MaxFileNameLength) return false;
		if(pfilePosition) *pfilePosition += MaxFileNameLength;

		ReadFile(hFile, &fileInfo, sizeof(BY_HANDLE_FILE_INFORMATION), &nByteRead, 0x0);
		if(nByteRead != sizeof(BY_HANDLE_FILE_INFORMATION)) return false;
		if(pfilePosition) *pfilePosition += sizeof(BY_HANDLE_FILE_INFORMATION);

		return true;
	}
	return false;
}
//---------------------------------------------------------------------------
void fileCache_t::fcFileDesc_t::set(int _id, TCHAR* _fileName)
{
	id = _id;
	if(_fileName == 0x0) {
		_tcscpy(fileName, TEXT(""));
	}
	else {
		_tcscpy(fileName, _fileName);
	}
}
//---------------------------------------------------------------------------
// fileCache_t::fcFileDescs_t
//---------------------------------------------------------------------------
bool fileCache_t::fcFileDescs_t::getFileInfo(const TCHAR* fileName, BY_HANDLE_FILE_INFORMATION* pFileInfo) 
{
	bool result = false;
	HANDLE hFile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, 0x0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0x0);
	if(INVALID_HANDLE_VALUE != hFile) {
		if(GetFileInformationByHandle(hFile, pFileInfo)) {
			result = true;
		}
		CloseHandle(hFile);
	}
	return result;
}
//---------------------------------------------------------------------------
bool fileCache_t::fcFileDescs_t::checkValid(fcFileDesc_t& fileDesc) 
{
	BY_HANDLE_FILE_INFORMATION fileInfo;
	if(getFileInfo(fileDesc.fileName, &fileInfo) && !memcmp(&fileInfo.ftLastWriteTime, &fileDesc.fileInfo.ftLastWriteTime, sizeof(fileInfo.ftCreationTime))) {
		return true;
	}			
	return false;
}
//---------------------------------------------------------------------------
fileCache_t::pfcFileDesc_t fileCache_t::fcFileDescs_t::find(const TCHAR* fileName) 
{
	for(int lpd=0;lpd<descs.Num();lpd++) {
		if(!_tcscmp(descs(lpd).fileName, fileName)) {
			return &descs(lpd);
		}
	}
	return 0x0;
}
//---------------------------------------------------------------------------
void fileCache_t::fcFileDescs_t::clear(void) 
{
	descs.Empty();
	idSerial = 0;
}
//---------------------------------------------------------------------------
fileCache_t::pfcFileDesc_t fileCache_t::fcFileDescs_t::addFileDesc(const TCHAR* fileName) 
{
	pfcFileDesc_t pDesc = find(fileName);
	if(!pDesc) {
		fcFileDesc_t desc;
		_tcscpy(desc.fileName, fileName);
		if(getFileInfo(fileName, &desc.fileInfo)) {
			desc.id = idSerial++;
			descs.Push(desc);
			return &descs.Last();
		}				
	}	
	return 0x0;
}
//---------------------------------------------------------------------------
fileCache_t::fcFileDescs_t::fcFileDescs_t(void) 
{
	clear();
}
//---------------------------------------------------------------------------
fileCache_t::fcFileDescs_t::~fcFileDescs_t(void) 
{
	clear();
}
//---------------------------------------------------------------------------
// fileCache_t::buffer_t
//---------------------------------------------------------------------------
void* fileCache_t::buffer_t::resize(int nByte) 
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
fileCache_t::buffer_t::buffer_t(void) 
{
	nSize = 0;
	ptr = 0x0;
}
//---------------------------------------------------------------------------
fileCache_t::buffer_t::~buffer_t(void) 
{
	if(ptr) free(ptr);
}
//---------------------------------------------------------------------------
// fileCache_t::writer_t
//---------------------------------------------------------------------------
void fileCache_t::writer_t::setEnable(bool _bEnable)
{
	bEnabled = _bEnable;
}
//---------------------------------------------------------------------------
bool fileCache_t::writer_t::isEnabled(void)
{
	return bEnabled;
}
//---------------------------------------------------------------------------
bool fileCache_t::writer_t::onOpenFile(HANDLE hFile, const TCHAR* fileName) 
{
	fcIndex_t index;
	index.set(hFile, fileName);
	GfileCache.handledFiles.Set((DWORD)hFile, index);
	pPrevIndex = 0x0;	
	return true;
}
//---------------------------------------------------------------------------
bool fileCache_t::writer_t::onSeek(HANDLE hFile, int offset) 
{
	pfcIndex_t pIndex = GfileCache.handledFiles.Find((DWORD)hFile);
	if(pIndex) {
		pIndex->offset = offset;
		return true;		
	}
	return false;
}
//---------------------------------------------------------------------------
bool fileCache_t::writer_t::verifyOffset(HANDLE hFile, int offset)
{
	pfcIndex_t pIndex = GfileCache.handledFiles.Find((DWORD)hFile);
	if(pIndex) {		
		return pIndex->offset==offset;
	}
	return false;
}
//---------------------------------------------------------------------------
bool fileCache_t::writer_t::onReadFile(HANDLE hFile, int nByte) 
{
	pfcIndex_t pHandledFile = GfileCache.handledFiles.Find((DWORD)hFile);
	if(0x0 == pHandledFile || pHandledFile->hFile == INVALID_HANDLE_VALUE) {
		LogToFile(0x0, TEXT("[fileCache_t::writer_t::onReadFile] invalid file 0x%x"), pHandledFile);
		return false;
	}

	//LogToFile(0x0, TEXT("[fileCache_t::writer_t::onReadFile] new access:%d %d %s"), GfileCache.handledIndices[hFile].offset, nByte, GfileCache.handledIndices[hFile].fileName);

	nAccess++;
	
	// {{ find prev
	bool bIndexed = false;
	if(pPrevIndex) {
		if(!_tcscmp(pPrevIndex->fileName, pHandledFile->fileName) && pPrevIndex->offset + pPrevIndex->nByte == pHandledFile->offset) {
			LogToFile(0x0, TEXT("[fileCache_t::writer_t::onReadFile] new access:%d %d continues prev:%d %d on %s"), pHandledFile->offset, nByte, pPrevIndex->offset, pPrevIndex->nByte, pHandledFile->fileName);
			pPrevIndex->nByte += nByte;
			bIndexed = true;
		}
		else {
			if(GfileCache.optimizeOption == ooSize) {
				for(int lpp=0;lpp<indices.Num();lpp++) {
					pfcIndex_t pIndex = &indices(lpp);
					if(pIndex) {
						if(!_tcscmp(pIndex->fileName, pHandledFile->fileName)) {
							if(pHandledFile->offset + nByte < pIndex->offset) continue;
							if(pIndex->offset + pIndex->nByte < pHandledFile->offset) continue;

							// calc nByte
							if(pIndex->offset + pIndex->nByte < pHandledFile->offset + nByte) {
								LogToFile(0x0, TEXT("[fileCache_t::writer_t::onReadFile] nByte modified indexed:%d %d, new:%d %d on %s"), pIndex->offset, pIndex->nByte, pHandledFile->offset, nByte, pHandledFile->fileName);
								pIndex->nByte += pHandledFile->offset + nByte - (pIndex->offset + pIndex->nByte);
								bIndexed = true;
							}

							// calc offset
							if(pHandledFile->offset < pIndex->offset) {
								LogToFile(0x0, TEXT("[fileCache_t::writer_t::onReadFile] offset,nByte modified indexed:%d %d, new:%d %d on %s"), pIndex->offset, pIndex->nByte, pHandledFile->offset, nByte, pHandledFile->fileName);
								pIndex->nByte += pIndex->offset - pHandledFile->offset;
								pIndex->offset = pHandledFile->offset;
								bIndexed = true;
							}

							if(false == bIndexed) {
								LogToFile(0x0, TEXT("[fileCache_t::writer_t::onReadFile] new access:%d %d included in prev:%d %d on %s"), pHandledFile->offset, nByte, pIndex->offset, pIndex->nByte, pHandledFile->fileName);
							}
							bIndexed = true;
							break;
						}
					}					
				}
			}
		}
	}	
	// }} find prev
	if(false == bIndexed) {
		pHandledFile->nByte = nByte;
		indices.Push(*pHandledFile);
		pPrevIndex = &indices.Last();
	}
	pHandledFile->offset += nByte;
	return true;
}
//---------------------------------------------------------------------------
void fileCache_t::writer_t::mergeIndices(void)
{	
	TArray<fcIndex_t> newIndices;

	if(indices.Num()) {
		for(int lpi=0;lpi<indices.Num();lpi++) {
			indices(lpi).bValid = true;
		}

		for(int lpy=0;lpy<indices.Num();lpy++) {
			pfcIndex_t pIndex1 = &indices(lpy);
			if(pIndex1->bValid) {
				bool bIndexed = false;
				for(int lpx=0;lpx<indices.Num();lpx++) {
					pfcIndex_t pIndex2 = &indices(lpx);
					if(pIndex2->bValid && lpx!=lpy) {
						if(!_tcscmp(pIndex1->fileName, pIndex2->fileName)) {
							if(pIndex2->offset + pIndex2->nByte < pIndex1->offset) continue;
							if(pIndex1->offset + pIndex1->nByte < pIndex2->offset) continue;

							// calc nByte
							if(pIndex1->offset + pIndex1->nByte < pIndex2->offset + pIndex2->nByte) {
								LogToFile(0x0, TEXT("[fileCache_t::writer_t::mergeIndices] nByte modified indexed:%d %d, new:%d %d on %s"), pIndex1->offset, pIndex1->nByte, pIndex2->offset, pIndex2->nByte, pIndex2->fileName);
								pIndex1->nByte += pIndex2->offset + pIndex2->nByte - (pIndex1->offset + pIndex1->nByte);
								bIndexed = true;
							}

							// calc offset
							if(pIndex2->offset < pIndex1->offset) {
								LogToFile(0x0, TEXT("[fileCache_t::writer_t::mergeIndices] offset,nByte modified indexed:%d %d, new:%d %d on %s"), pIndex1->offset, pIndex1->nByte, pIndex2->offset, pIndex2->nByte, pIndex2->fileName);
								pIndex1->nByte += pIndex1->offset - pIndex2->offset;
								pIndex1->offset = pIndex2->offset;						
								bIndexed = true;
							}

							if(false == bIndexed) {
								LogToFile(0x0, TEXT("[fileCache_t::writer_t::mergeIndices] new access:%d %d included in prev:%d %d on %s"), pIndex2->offset, pIndex2->nByte, pIndex1->offset, pIndex1->nByte, pIndex2->fileName);
							}
							bIndexed = true;
							pIndex2->bValid = false;
							break;
						}
					}
				}
			}
		}
		for(int lpp=0;lpp<indices.Num();lpp++) {
			if(indices(lpp).bValid) newIndices.Push(indices(lpp));
		}
		int nIndexBeforeMerge = indices.Num();		
		indices.Empty();
		for(int lpp=0;lpp<newIndices.Num();lpp++) {
			indices.Push(newIndices(lpp));
		}
		newIndices.Empty();
		LogToFile(0x0, TEXT("[fileCache_t::writer_t::mergeIndices] merged %d -> %d"), nIndexBeforeMerge, indices.Num());
	}
}
//---------------------------------------------------------------------------
bool fileCache_t::writer_t::begin(void) 
{	
	nAccess = 0;
	pPrevIndex = 0x0;	
	indices.Empty();
	setEnable();
	return true;
}
//---------------------------------------------------------------------------
bool fileCache_t::writer_t::writeIndices(HANDLE hFile)
{
	bool result = false;
	if(hFile!=INVALID_HANDLE_VALUE) {
		if(GfileCache.optimizeOption == ooSize) {
			mergeIndices();
		}
		// {{ data file version
		fileList_t fileList;
		for(int lpp=0;lpp<indices.Num();lpp++) {
			fileList.insert(indices(lpp).fileName);
		}

		fileDescs.clear();
		for(int lpp=0;lpp<fileList.files.Num();lpp++) {
			pfcFileDesc_t pDesc = fileDescs.addFileDesc(*fileList.files(lpp));
			if(pDesc) {
				//LogToFile(0x0, TEXT("[fileCache_t::writer_t::writeIndices] registering file desc %d %s"), pDesc->id, pDesc->fileName);
				for(int lpi=0;lpi<indices.Num();lpi++) {
					if(fileList.files(lpp) == FString(indices(lpi).fileName)) {
						indices(lpi).idFile = pDesc->id;
						//LogToFile(0x0, TEXT("[fileCache_t::writer_t::writeIndices] setup file id :%d for %s on indices"), pDesc->id, indices(lpi).fileName);
					}					
				}
			}
		}
		// }} data file version

		header_t header;
		header.version = Version;
		header.nFiles = fileDescs.descs.Num();
		header.nIndices = indices.Num();
		header.nAccess = nAccess;

		int filePosition = 0;
		DWORD nByteWritten = 0;
		WriteFile(hFile, &header, sizeof(header_t), &nByteWritten, 0x0);
		if(nByteWritten != sizeof(header_t)) {
			LogToFile(0x0, TEXT("[fileCache_t::writer_t::writeIndices] saving file header failed"));				
			return false;
		}
		filePosition += sizeof(header_t);

		for(int lpd=0;lpd<fileDescs.descs.Num();lpd++) {			
			if(false == fileDescs.descs(lpd).saveToFile(hFile, &filePosition)) {
				LogToFile(0x0, TEXT("[fileCache_t::writer_t::writeIndices] saving file desc %s failed"), fileDescs.descs(lpd).fileName);
				break;
			}
		}		
		// }} data file version

		int dataOffset = filePosition + indices.Num() * fcIndex_t::fileSize();

		for(int lpi=0;lpi<indices.Num();lpi++) {
			indices(lpi).cacheOffset = dataOffset;
			if(false == indices(lpi).saveToFile(hFile, &filePosition)) {
				LogToFile(0x0, TEXT("[fileCache_t::writer_t::writeIndices]  saving index failed %s %d %d"), indices(lpi).fileName, indices(lpi).offset, indices(lpi).nByte);
				break;
			}
			dataOffset += indices(lpi).nByte;
		}
		LogToFile(0x0, TEXT("[fileCache_t::writer_t::writeIndices] index file generated : (%d accesses %d indexed) %s"), nAccess, indices.Num(), GfileCache.optimizeOption==ooSize?TEXT("*size priority*"):TEXT("*speed priority*"));

		for(int lpd=0;lpd<fileDescs.descs.Num();lpd++) {
			LogToFile(0x0, TEXT("[fileCache_t::writer_t::writeIndices] file desc %d: fileName:%s"), lpd, fileDescs.descs(lpd).fileName);
		}
		
		for(int lpi=0;lpi<indices.Num();lpi++) {
			LogToFile(0x0, TEXT("[fileCache_t::writer_t::writeIndices] index %d: idFile:%d offset:%d nByte:%d"), lpi, indices(lpi).idFile, indices(lpi).offset, indices(lpi).nByte);
		}
		result = true;
	}
	return result;
}
//---------------------------------------------------------------------------
bool fileCache_t::writer_t::writeData(HANDLE hFile)
{
	bool result = true;	
	DWORD nByteRead = 0x0;
	DWORD nByteWritten = 0x0;

	buffer_t buffer;

	for(int lpi=0;lpi<indices.Num();lpi++) {
		fcIndex_t index = indices(lpi);
		if(index.idFile >= fileDescs.descs.Num()) {
			LogToFile(0x0, TEXT("[fileCache_t::writer_t::writeData] invalid idfile:%d specified #file=%d"), index.idFile, fileDescs.descs.Num());
			return false;
		}
		HANDLE hFileOrg = CreateFile(fileDescs.descs(index.idFile).fileName, GENERIC_READ, FILE_SHARE_READ, 0x0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0x0);
		if(INVALID_HANDLE_VALUE != hFileOrg) {			
			void* pBuffer = buffer.resize(index.nByte);
			SetFilePointer(hFileOrg, index.offset, 0x0, FILE_BEGIN);
			ReadFile(hFileOrg, pBuffer, index.nByte, &nByteRead, 0x0);
			if(!index.nByte || index.nByte != nByteRead) {
				LogToFile(0x0, TEXT("[fileCache_t::writer_t::writeData] reading source file failed %s %d bytes at %d"), index.fileName, index.nByte, index.offset);
				result = false;
				break;
			}
			WriteFile(hFile, pBuffer, index.nByte, &nByteWritten, 0x0);
			if(!index.nByte || index.nByte != nByteWritten) {
				LogToFile(0x0, TEXT("[fileCache_t::writer_t::writeData] writing cache failed %s %d bytes at %d"), index.fileName, index.nByte, index.offset);
				result = false;
				break;
			}
			CloseHandle(hFileOrg);
		}
		else {
			LogToFile(0x0, TEXT("[fileCache_t::writer_t::writeData] file open failed %s"), index.fileName);
			result = false;
			break;
		}
	}
	return result;
}
//---------------------------------------------------------------------------
bool fileCache_t::writer_t::end(TCHAR* indexFileName)
{
	TCHAR fullPath[MaxFileNameLength] = TEXT("");
	bool bReadOnly = false;
	GfileCache.makeFullPath(fullPath, &bReadOnly, indexFileName, GfileCache.desiredPath, true);	
	
	setEnable(false);
	bool result = false;
	HANDLE hFile = CreateFile(fullPath, GENERIC_WRITE, 0, 0x0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0x0);
	if(INVALID_HANDLE_VALUE!=hFile) {
		result = writeIndices(hFile);		
		CloseHandle(hFile);
		if(result) {
			LogToFile(0x0, TEXT("[fileCache_t::writer_t::end] Generating %s succeeded."), fullPath);
			return true;
		}
		DeleteFile(fullPath);
		LogToFile(0x0, TEXT("[fileCache_t::writer_t::end] file %s deleted due to error"), fullPath);		
	}
	return result;
}
//---------------------------------------------------------------------------
bool fileCache_t::writer_t::loadFromFile(TCHAR* indexFileName)
{
	bool result = false;
	DWORD nByteRead = 0x0;

	fileDescs.clear();
	indices.Empty();

	HANDLE hFile = CreateFile(indexFileName, GENERIC_READ, FILE_SHARE_READ, 0x0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0x0);
	if(INVALID_HANDLE_VALUE != hFile) {
		header_t header;
		ReadFile(hFile, &header, sizeof(header_t), &nByteRead, 0x0); 
		if(sizeof(header_t) != nByteRead) {
			CloseHandle(hFile);
			return false;
		}	
		
		for(int lpd=0;lpd<header.nFiles;lpd++) {
			fcFileDesc_t desc;			
			if(false == desc.loadFromFile(hFile)) {
				CloseHandle(hFile);
				return false;
			}
			fileDescs.descs.Push(desc);
		}		

		if(fileDescs.descs.Num() != header.nFiles) {
			CloseHandle(hFile);
			return false;
		}

		fcIndex_t index;
		for(int lpp=0;lpp<header.nIndices;lpp++) {
			if(false == index.loadFromFile(hFile)) {
				CloseHandle(hFile);
				return false;
			}
			indices.Push(index);
		}		

		if(indices.Num() != header.nIndices) {
			CloseHandle(hFile);
			return false;
		}
		CloseHandle(hFile);
		return true;
	}
	return false;
}

//---------------------------------------------------------------------------
bool fileCache_t::writer_t::makeCacheFromIndex(TCHAR* cacheFileName, TCHAR* indexFileName)
{
	if(CopyFile(indexFileName, cacheFileName, FALSE)) {		
		HANDLE hFile = CreateFile(cacheFileName, GENERIC_WRITE, 0, 0x0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0x0);
		if(INVALID_HANDLE_VALUE != hFile) {
			SetFilePointer(hFile, 0, 0x0, FILE_END);
			bool result = writeData(hFile);
			CloseHandle(hFile);
			if(result) {
				LogToFile(0x0, TEXT("[fileCache_t::writer_t::makeCacheFromIndex] Generating %s succeeded."), cacheFileName);
				return true;
			}
			DeleteFile(cacheFileName);
			LogToFile(0x0, TEXT("[fileCache_t::writer_t::makeCacheFromIndex] file %s deleted due to error"), cacheFileName);		
		}
		else {
			LogToFile(0x0, TEXT("[fileCache_t::writer_t::makeCacheFromIndex] 1.generating cache %s from index %s failed"), cacheFileName, indexFileName);
		}
	}
	else {
		LogToFile(0x0, TEXT("[fileCache_t::writer_t::makeCacheFromIndex] 2.generating cache %s from index %s failed"), cacheFileName, indexFileName);
	}
	return false;
}
//---------------------------------------------------------------------------
fileCache_t::writer_t::writer_t(void)
{
	setEnable(false);
	pPrevIndex = 0x0;	
	
}
//---------------------------------------------------------------------------
fileCache_t::writer_t::~writer_t(void)
{	
	indices.Empty();
}
//---------------------------------------------------------------------------
// fileCache_t
//---------------------------------------------------------------------------
bool fileCache_t::openCache(TCHAR* fileName) 
{	
	TCHAR fullPath[MaxFileNameLength] = TEXT("");
	bool bReadOnly = false;
	makeFullPath(fullPath, &bReadOnly, fileName, desiredPath, true);
	_tcscpy(fileName, fullPath);

	hCacheFile = CreateFile(fullPath, GENERIC_READ, 0, 0x0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0x0);
	if(hCacheFile!=INVALID_HANDLE_VALUE) {
		currentCacheOffset = 0;
		header_t header;
		DWORD nByteRead = 0;
		ReadFile(hCacheFile, &header, sizeof(header_t), &nByteRead, 0x0);
		if(header.version != Version) {
			LogToFile(0x0, TEXT("[fileCache_t::openCache] file version mismatch 0x%x (expecting 0x%x)"), header.version, Version);
			closeCache();
			return false;
		}
		currentCacheOffset += nByteRead;
		nAccess = header.nAccess;

		clearIndices();

		pFileDescs = (pfcFileDesc_t)malloc(sizeof(fcFileDesc_t) * header.nFiles);
		if(pFileDescs) {
			for(int lpp=0;lpp<header.nFiles;lpp++) {
				if(false == pFileDescs[lpp].loadFromFile(hCacheFile, &currentCacheOffset)) {
					LogToFile(0x0, TEXT("[fileCache_t::openCache] loading file desc failed %d %s"), lpp, fileName);
					closeCache();
				}
				//LogToFile(0x0, TEXT("[fileCache_t::openCache] loading fileDesc %d %s"), lpp, pFileDescs[lpp].fileName);
			}
			nFileDescs = header.nFiles;
		}

		pIndices = (pfcIndex_t)malloc(sizeof(fcIndex_t) * header.nIndices);
		if(pIndices) {
			for(int lpp=0;lpp<header.nIndices;lpp++) {				
				if(false == pIndices[lpp].loadFromFile(hCacheFile, &currentCacheOffset)) {
					LogToFile(0x0, TEXT("[fileCache_t::openCache] loading index failed %d %s"), lpp, fileName);
					closeCache();
				}
				//LogToFile(0x0, TEXT("openCache index %d: fid:%d OO:%d #B:%d CO:%d"), lpp, pIndices[lpp].idFile, pIndices[lpp].offset, pIndices[lpp].nByte, pIndices[lpp].cacheOffset);
			}

			pCurIndex = pIndices;
			nIndices = header.nIndices;
		}
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------
void fileCache_t::closeCache(void) 
{
	if(hCacheFile!=INVALID_HANDLE_VALUE) {
		CloseHandle(hCacheFile);
		hCacheFile = INVALID_HANDLE_VALUE;
	}	
}
//---------------------------------------------------------------------------
fileCache_t::pfcIndex_t fileCache_t::findForward(HANDLE hFile, int idFile/*const TCHAR* fileName*/, int offset, int nByte)
{
	if(pCurIndex) {
		pfcIndex_t pBeginIndex = pCurIndex;
		pfcIndex_t pEndIndex = &pIndices[nIndices];
		for(pfcIndex_t pIndex=pBeginIndex;pIndex<pEndIndex;pIndex++) {
			if(pIndex) {
				if(hFile != INVALID_HANDLE_VALUE && hFile != pIndex->hFile) {
					nLookup++; //LogToFile(0x0, TEXT("[fileCache_t::findForward] index skipped due to hFile mismatch"));
					continue;
				}
				if(idFile != -1 && idFile != pIndex->idFile) {
					nLookup++; //LogToFile(0x0, TEXT("[fileCache_t::findBackward] index skipped due to filename mismatch"));
					continue;
				}
				//if(pIndex->idFile >= nFileDescs) {
				//	nLookup++; // invalid idFile
				//	continue;
				//}
				//if(fileName != 0x0 && _tcscmp(fileName, pFileDescs[pIndex->idFile].fileName)) {
				//	nLookup++; //LogToFile(0x0, TEXT("[fileCache_t::findForward] index skipped due to filename mismatch %s <-> %d %s"), fileName, pIndex->idFile, pFileDescs[pIndex->idFile].fileName);
				//	continue;
				//}
				if(offset != -1 && nByte != -1) {
					if(pIndex->offset <= offset && offset + nByte <= pIndex->offset + pIndex->nByte) {
						return pIndex;
					}					
					nLookup++; //LogToFile(0x0, TEXT("[fileCache_t::findForward] index skipped due to offset mismatch"));
					continue;
				}
				else {
					if(offset != -1 || offset != pIndex->offset) {
						nLookup++; //LogToFile(0x0, TEXT("[fileCache_t::findForward] index skipped due to # byte mismatch"));
						continue;
					}
					if(nByte != -1 || nByte != pIndex->nByte) {
						nLookup++; //LogToFile(0x0, TEXT("[fileCache_t::findForward] index skipped due to # byte mismatch"));
						continue;
					}
				}				
				return pIndex;
			}
		}
	}
	//LogToFile(0x0, TEXT("[fileCache_t::findForward] failed %s %d %d"), fileName, offset, nByte);
	return 0x0;
}
//---------------------------------------------------------------------------
fileCache_t::pfcIndex_t fileCache_t::findBackward(HANDLE hFile, int idFile/*const TCHAR* fileName*/, int offset, int nByte)
{
	if(pCurIndex) {
		pfcIndex_t pBeginIndex = pCurIndex;
		pfcIndex_t pEndIndex = &pIndices[0];
		for(pfcIndex_t pIndex=pBeginIndex;pIndex>=pEndIndex;pIndex--) {
			if(pIndex) {
				if(hFile != INVALID_HANDLE_VALUE && hFile != pIndex->hFile) {
					nLookup++; //LogToFile(0x0, TEXT("[fileCache_t::findBackward] index skipped due to hFile mismatch"));
					continue;
				}
				if(idFile != -1 && idFile != pIndex->idFile) {
					nLookup++; //LogToFile(0x0, TEXT("[fileCache_t::findBackward] index skipped due to filename mismatch"));
					continue;
				}
				//if(pIndex->idFile >= nFileDescs) {
				//	nLookup++; // invalid idFile
				//	continue;
				//}
				//if(fileName == 0x0 || _tcscmp(fileName, pFileDescs[pIndex->idFile].fileName)) {
				//	nLookup++; //LogToFile(0x0, TEXT("[fileCache_t::findBackward] index skipped due to filename mismatch"));
				//	continue;
				//}
				if(offset != -1 && nByte != -1) {
					if(pIndex->offset <= offset && offset + nByte <= pIndex->offset + pIndex->nByte) {
						return pIndex;
					}					
					nLookup++; //LogToFile(0x0, TEXT("[fileCache_t::findForward] index skipped due to offset mismatch"));
					continue;
				}
				else {
					if(offset == -1 || offset != pIndex->offset) {
						nLookup++; //LogToFile(0x0, TEXT("[fileCache_t::findBackward] index skipped due to offset mismatch"));
						continue;
					}
					if(nByte == -1 || nByte != pIndex->nByte) {
						nLookup++; //LogToFile(0x0, TEXT("[fileCache_t::findBackward] index skipped due to # byte mismatch"));
						continue;
					}
				}				
				return pIndex;
			}
		}
	}
	//LogToFile(0x0, TEXT("[fileCache_t::findBackward] failed %s %d %d"), fileName, offset, nByte);
	return 0x0;
}
//---------------------------------------------------------------------------
void fileCache_t::clearIndices(void)
{
	if(pFileDescs) {
		free(pFileDescs);
		pFileDescs = 0x0;
	}
	nFileDescs = 0;

	if(pIndices) {
		free(pIndices);
		pIndices = 0x0;		
	}
	
	pCurIndex = 0x0;
	nIndices = 0;
}
//---------------------------------------------------------------------------
bool fileCache_t::convLevelFileNameToCacheFileName(TCHAR* cacheFileName, const TCHAR* levelFileName)
{
	if(cacheFileName && levelFileName) {
		int len = _tcslen(levelFileName);
		if(len>4) {
			_tcscpy(cacheFileName, levelFileName);
			if(cacheFileName[len-4] == TEXT('.')) {
				cacheFileName[len-4] = 0x0;
			}
			_tcscat(cacheFileName, TEXT(".dfc"));
			return true;
		}		
	}
	return false;
}
//---------------------------------------------------------------------------
bool fileCache_t::convLevelFileNameToIndexFileName(TCHAR* indexFileName, const TCHAR* levelFileName)
{
	if(indexFileName && levelFileName) {
		int len = _tcslen(levelFileName);
		if(len>4) {
			_tcscpy(indexFileName, levelFileName);
			if(indexFileName[len-4] == TEXT('.')) {
				indexFileName[len-4] = 0x0;
			}			
			_tcscat(indexFileName, TEXT(".sci"));
			return true;
		}		
	}
	return false;
}
//---------------------------------------------------------------------------
void fileCache_t::_log(TCHAR* fileName, TCHAR* str)
{
	FILE* fp = _tfopen(fileName, TEXT("at"));
	if(fp) {
		_ftprintf(fp, TEXT("%s\n"), str);		
		fclose(fp);
	}
}
//---------------------------------------------------------------------------
void fileCache_t::logToFile(TCHAR* fileName, TCHAR* fmt, ...)
{
	static TCHAR defaultFileName[] = TEXT("fileCache.log");
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
void fileCache_t::onBeginLoadMap(const TCHAR* levelFileName)
{	
	if(false == bEnabled) return;

	bCached = false;
	writer.setEnable(false);
	bBeginLoadMap = true;
	
	convLevelFileNameToCacheFileName(cacheFileName, levelFileName);
	convLevelFileNameToIndexFileName(indexFileName, levelFileName);

	if(-1 != schedule.find(indexFileName)) {
		LogToFile(0x0, TEXT("[fileCache_t::onBeginLoadMap] already scheduled level %s"), levelFileName);
		return;
	}

	idFileCurrent = -1;
	_tcscpy(fileNameCurrent, TEXT(""));

	timeLoadFromCache = 0x0;
	nLookup = 0;
	nCacheMiss = 0;
	nCacheHit = 0;
	bCached = openCache(cacheFileName);
	if(bCached) {
		LogToFile(0x0, TEXT("[fileCache_t::onBeginLoadMap] cache for %s loaded"), levelFileName);
		return;
	}
	LogToFile(0x0, TEXT("[fileCache_t::onBeginLoadMap] cache for %s does not exist, writing new one."), levelFileName);	
	writer.begin();
}
//---------------------------------------------------------------------------
bool fileCache_t::onOpenFile(HANDLE hFile, const TCHAR* fileName)
{	
	if(false == bEnabled) return false;

	// {{ make relative path
	TCHAR relFileName[MaxFileNameLength] = TEXT("");
	for(unsigned int lpp=0;lpp<_tcslen(fileName)-1;lpp++) {
		if(fileName[lpp] == TEXT('.') && fileName[lpp+1] == TEXT('.')) {
			_tcscpy(relFileName, &fileName[lpp]);
			break;
		}
	}
	if(0 == _tcslen(relFileName)) {
		_tcscpy(relFileName, fileName);
	}
	// }} make relative path

	//if(0x0 == handledFiles.Find((DWORD)hFile)) { // 등록되어있더라도 무조건
	fcIndex_t index;
	index.set(hFile, relFileName);		
	if(!handledFiles.Set((DWORD)hFile, index).compare(index)) {
		LogToFile(0x0, TEXT("[fileCache_t::onOpenFile] file %s register failed"), relFileName);
	}
	else {
		//LogToFile(0x0, TEXT("[fileCache_t::onOpenFile] file %s registered hfile:0x%x"), fileName, hFile);
	}
	//}	

	if(!bBeginLoadMap) return false;

	if(writer.isEnabled()) {
		writer.onOpenFile(hFile, relFileName);
		return false;
	}
	return false;
}
//---------------------------------------------------------------------------
bool fileCache_t::onSeek(HANDLE hFile, int offset)
{
	if(false == bEnabled) return false;

	pfcIndex_t pIndex = handledFiles.Find((DWORD)hFile);
	if(pIndex && pIndex->hFile != INVALID_HANDLE_VALUE) {
		pIndex->offset = offset;
	}	
	return false;
}
//---------------------------------------------------------------------------
bool fileCache_t::verifyOffset(HANDLE hFile, int offset)
{
	if(false == bEnabled) return false;

	if(writer.isEnabled()) writer.verifyOffset(hFile, offset);
	return true;
}
//---------------------------------------------------------------------------
bool fileCache_t::onReadFile(HANDLE hFile, void* pData, int nByte, DWORD* pnByteRead)
{
	if(false == bEnabled) return false;

	if(!bBeginLoadMap) return false;

	if(writer.isEnabled()) {
		writer.onReadFile(hFile, nByte);
		return false;
	}

	if(bCached) {
		pfcIndex_t pHandledFile = handledFiles.Find((DWORD)hFile);
		if(pHandledFile && pHandledFile->hFile != INVALID_HANDLE_VALUE) {
			int offset = pHandledFile->offset;

			// {{ find idFile
			if(_tcscmp(fileNameCurrent, pHandledFile->fileName)) {
				for(int lpf=0;lpf<nFileDescs;lpf++) {
					if(!_tcscmp(pHandledFile->fileName, pFileDescs[lpf].fileName)) {
						idFileCurrent = lpf;
						_tcscpy(fileNameCurrent, pHandledFile->fileName);
						break;
					}
				}
			}			
			// }} find idFile
			pfcIndex_t pIndex = findForward(INVALID_HANDLE_VALUE, idFileCurrent/*pHandledFile->fileName*/, offset, nByte); 
			if(pIndex == 0x0) pIndex = findBackward(INVALID_HANDLE_VALUE, idFileCurrent/*pHandledFile->fileName*/, offset, nByte);

			// {{ 20080115 test cache miss
			#ifdef SimulateCacheMiss
			if(rand() %2) {
				LogToFile(0x0, TEXT("[fileCache_t::onReadFile] simulating cache miss"));
				pIndex = 0x0;
			}
			#endif
			// }} 20080115 test cache miss
			if(pIndex) {
				pCurIndex = pIndex;
				DWORD nByteRead = 0;
				//DWORD timeBegin = timeGetTime();
				int offsetToLoad = pCurIndex->cacheOffset +  offset - pCurIndex->offset;
				if(currentCacheOffset != offsetToLoad) {
					SetFilePointer(hCacheFile, offsetToLoad, 0x0, FILE_BEGIN);
					currentCacheOffset = offsetToLoad;
				}				
				ReadFile(hCacheFile, pData, nByte, &nByteRead, 0x0);
				currentCacheOffset += nByteRead;
				pHandledFile->offset += nByteRead;			
				//DWORD timeEnd = timeGetTime();
				//timeLoadFromCache += timeEnd - timeBegin;
				if(nByte == nByteRead) {
					//LogToFile(0x0, TEXT("[fileCache_t::onReadFile] offset %d, %d bytes read from cache for %s %d %d"), offset, nByteRead, pCurIndex->fileName, pCurIndex->offset, pCurIndex->nByte);
					if(pnByteRead) *pnByteRead = nByteRead;				
					nCacheHit++;
					return true;
				}			
			}
			SetFilePointer(hFile, pHandledFile->offset, 0x0, FILE_BEGIN);
			pHandledFile->offset += nByte;
			nCacheMiss++;
			LogToFile(0x0, TEXT("[fileCache_t::onReadFile] cache missed : %s %d %d"), pHandledFile->fileName, offset, nByte);		

			if(nAccess && dropCacheThreshold < (float)nCacheMiss / nAccess) {
				bCached = false;
				clearIndices();
				closeCache();	
				DeleteFile(cacheFileName);
				LogToFile(0x0, TEXT("[fileCache_t::onReadFile] missed cache %s deleted. (%d miss %f %%)"), cacheFileName, nCacheMiss, (float)nCacheMiss * 100.0f/ nAccess);
			}
		}
	}	
	return false;
}
//---------------------------------------------------------------------------
bool fileCache_t::onCloseFile(HANDLE hFile)
{
	if(false == bEnabled) return false;

	pfcIndex_t pHandledFile = handledFiles.Find((DWORD)hFile);
	if(pHandledFile && pHandledFile->hFile != INVALID_HANDLE_VALUE) {
		//LogToFile(0x0, TEXT("[fileCache_t::onCloseFile] file 0x%x %s unregistered."), pHandledFile, pHandledFile->fileName);
		pHandledFile->set();		
		//handledFiles.Remove((DWORD)hFile); // 20080227 이러다 assert걸림
	}

	if(!bBeginLoadMap) return false;

	// nothing to do
	if(writer.isEnabled()) {		
		return false;
	}
	return true;
}
//---------------------------------------------------------------------------
void fileCache_t::onEndLoadMap(void)
{
	if(false == bEnabled) return;

	if(!bBeginLoadMap) return;
	bBeginLoadMap = false;

	if(writer.isEnabled()) {
		if(writer.end(indexFileName)) {			
			schedule.insert(indexFileName);
		}
		LogToFile(0x0, TEXT("[fileCache_t::onEndLoadMap] writing %s complete."), indexFileName);		
		return;
	}

	clearIndices();
	closeCache();	

	/*if(nCacheMiss >= (int)((float)(nCacheMiss+nCacheHit) * 0.2f)) {
		DeleteFile(cacheFileName);		
		nCacheMiss = 0;
		LogToFile(0x0, TEXT("[fileCache_t::onEndLoadMap] missed cache %s delete."), cacheFileName);
		return;
	}*/

	LogToFile(0x0, TEXT("[fileCache_t::onEndLoadMap] done hit:%d miss:%d (%d first chance lookup failed) %s."), nCacheHit, nCacheMiss, nLookup, cacheFileName);//LogToFile(0x0, TEXT("[fileCache_t::onEndLoadMap] done hit:%d miss:%d while %d ms (%d first chance lookup failed) %s."), nCacheHit, nCacheMiss, timeLoadFromCache, nLookup, cacheFileName);
}
//---------------------------------------------------------------------------
void fileCache_t::generateScheduledCache(void)
{
	intptr_t hFind;
	struct _wfinddata_t c_file;
	TCHAR path[MaxFileNameLength] = TEXT("");

	_tcscpy(path, desiredPath);
	_tcscat(path, TEXT("*.sci"));
	
	int nCacheGenerated = 0;

	if( (hFind = _wfindfirst(path, &c_file )) != -1L ) { // failed
		do {
			TCHAR indexFileName[MaxFileNameLength] = TEXT("");
			bool bReadOnly = false;
			makeFullPath(indexFileName, &bReadOnly, c_file.name, desiredPath, true);	
			if(writer.loadFromFile(indexFileName)) {
				TCHAR cacheFileName[MaxFileNameLength] = TEXT("");				
				makeCacheFileName(cacheFileName, indexFileName);
				if(writer.makeCacheFromIndex(cacheFileName, indexFileName)) {
					nCacheGenerated++;
				}
			}
			DeleteFile(indexFileName);
		} while(nCacheGenerated < numCacheToGenerate && _wfindnext(hFind, &c_file) != -1L);
		_findclose( hFind );
	}
	else {
		LogToFile(0x0, TEXT("[fileCache_t::generateScheduledCache] No schedule found."));
	}
}
//---------------------------------------------------------------------------
bool fileCache_t::checkCacheConsistency(TCHAR* cacheFileName)
{	
	DWORD nByteRead = 0x0;

	HANDLE hFile = CreateFile(cacheFileName, GENERIC_READ, FILE_SHARE_READ, 0x0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0x0);
	if(INVALID_HANDLE_VALUE != hFile) {
		header_t header;
		ReadFile(hFile, &header, sizeof(header_t), &nByteRead, 0x0); 
		if(sizeof(header_t) != nByteRead) {
			CloseHandle(hFile);
			return false;
		}	

		LogToFile(0x0, TEXT("[fileCache_t::writer_t::checkCacheConsistency] checking %s"), cacheFileName);

		for(int lpd=0;lpd<header.nFiles;lpd++) {
			fcFileDesc_t desc;			
			if(false == desc.loadFromFile(hFile)) {
				CloseHandle(hFile);
				return false;
			}	

			HANDLE hFileSource = CreateFile(desc.fileName, GENERIC_READ, FILE_SHARE_READ, 0x0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0x0);
			if(hFileSource) {
				BY_HANDLE_FILE_INFORMATION fileInfo;
				if(!GetFileInformationByHandle(hFileSource, &fileInfo) || memcmp(&fileInfo.ftLastWriteTime, &desc.fileInfo.ftLastWriteTime, sizeof(FILETIME))) {
					CloseHandle(hFileSource);
					CloseHandle(hFile);
					LogToFile(0x0, TEXT("[fileCache_t::writer_t::checkCacheConsistency] ---- INVALID : %s"), desc.fileName);
					return false;
				}				
				CloseHandle(hFileSource);
				//LogToFile(0x0, TEXT("[fileCache_t::writer_t::checkCacheConsistency] ---- valid : %s"), desc.fileName);
			}			
		}
		CloseHandle(hFile);
	}
	return true;
}
//---------------------------------------------------------------------------
void fileCache_t::checkCacheConsistencies(void)
{
	intptr_t hFind;
	struct _wfinddata_t c_file;
	TCHAR path[MaxFileNameLength] = TEXT("");

	_tcscpy(path, desiredPath);
	_tcscat(path, TEXT("*.dfc"));

	if( (hFind = _wfindfirst(path, &c_file )) != -1L ) { // failed
		do {
			TCHAR cacheFileName[MaxFileNameLength] = TEXT("");
			bool bReadOnly = false;
			makeFullPath(cacheFileName, &bReadOnly, c_file.name, desiredPath, true);	
			if(!checkCacheConsistency(cacheFileName)) {
				LogToFile(0x0, TEXT("[fileCache_t::checkCacheConsistency] deleting cache %s due to source file mismatch"), cacheFileName);
				DeleteFile(cacheFileName);				
			}			
		} while(_wfindnext(hFind, &c_file) != -1L);
		_findclose( hFind );
	}
	else {
		LogToFile(0x0, TEXT("[fileCache_t::generateScheduledCache] No schedule found."));
	}
}
//---------------------------------------------------------------------------
bool fileCache_t::makeFullPath(TCHAR* result, bool* pbReadOnly, TCHAR* fileName, TCHAR* desiredPath, bool bMkDir)
{
	bool bValidDirectory = false;
	TCHAR pathFile[512] = TEXT("");
	struct _wfinddata_t c_file;
	intptr_t hFile;

	_tcscpy(pathFile, desiredPath);
	_tcscat(pathFile, TEXT("."));

	if(pbReadOnly) {
		*pbReadOnly = false;
	}

	if( (hFile = _wfindfirst(pathFile, &c_file )) == -1L ) {
		if(bMkDir) {
			if(_wmkdir(desiredPath) == 0) {
				bValidDirectory = true;
			}
		}			
	}
	else {
		_findclose( hFile );
		bValidDirectory = true;		
		if(c_file.attrib & _A_RDONLY ) {
			if(pbReadOnly) {
				*pbReadOnly = true;
			}
		}
	}

	if(bValidDirectory) {
		_tcscpy(result, desiredPath);
		_tcscat(result, fileName);
		return true;
	}
	_tcscpy(result, fileName);
	return false;
}
//---------------------------------------------------------------------------
TCHAR* fileCache_t::makeCacheFileName(TCHAR* cacheFileName, TCHAR* indexFileName)
{
	if(cacheFileName && indexFileName) {
		int len = _tcslen(indexFileName);
		if(len > 4 && !_tcscmp(&indexFileName[len-4], TEXT(".sci"))) {
			_tcscpy(cacheFileName, indexFileName);
			if(cacheFileName[len-4] == TEXT('.')) {
				cacheFileName[len-4] = 0x0;
			}			
			_tcscat(cacheFileName, TEXT(".dfc"));
			return cacheFileName;
		}		
	}
	return 0x0;
}
//---------------------------------------------------------------------------
void fileCache_t::setEnable(bool bEnable)
{
	bEnabled = bEnable;
}
//---------------------------------------------------------------------------
bool fileCache_t::init(void)
{
	if(bGenerateScheduledCache) generateScheduledCache();
	if(bCheckCacheConsistency) checkCacheConsistencies();	
	return true;
}
//---------------------------------------------------------------------------
void fileCache_t::deinit(void)
{
	//?
}
//---------------------------------------------------------------------------
fileCache_t::fileCache_t(void) 
{
	LogToFile(0x0, TEXT("[fileCache_t::fileCache_t]"));

	_tcscpy(desiredPath, DesiredCachePath);

	setEnable(false);
	optimizeOption = DefaultOptimizeOption;
	bCheckCacheConsistency = DefaultCheckCacheConsistency;
	bGenerateScheduledCache = DefaultGenerateScheduledCache;
	numCacheToGenerate = DefaultNumCacheToGenerate;
	nAccess = 0;
	dropCacheThreshold = DefaultDropCacheThreshold;
	bBeginLoadMap = false;
	bCached = false;
	nCacheMiss = 0x0;
	hCacheFile = INVALID_HANDLE_VALUE;
	currentCacheOffset = 0;
	pFileDescs = 0x0;
	pIndices = 0x0;
	pCurIndex = 0x0;
}
//---------------------------------------------------------------------------
fileCache_t::~fileCache_t(void)
{
	handledFiles.Empty();
	clearIndices();
	closeCache();

	LogToFile(0x0, TEXT("[fileCache_t::~fileCache_t]"));
}
//---------------------------------------------------------------------------