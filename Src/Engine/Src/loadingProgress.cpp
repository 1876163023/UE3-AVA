//
// loadingProgress.cpp
//
// deathcure@redduck.com
//---------------------------------------------------------------------------
#include "EnginePrivate.h"
#include "loadingProgress.h"

#include <io.h>

//---------------------------------------------------------------------------
// global def
//---------------------------------------------------------------------------
loadingProgress_t g_loadingProgress;

static void _log(TCHAR* fileName, TCHAR* str)
{		
	return;
	static TCHAR defFileName[512] = TEXT("loadingProgress.log");
	if(fileName == 0x0) fileName = defFileName;

	FILE* fp = _tfopen(fileName, TEXT("at"));
	if(fp) {
		_ftprintf(fp, TEXT("%s\n"), str);		
		fclose(fp);
	}
}

static void _log(TCHAR* str)
{	
	return;
	//debugf(str);
	//return;
	FILE* fp = fopen("loadingProgress.log", "at");
	if(fp) {
		_ftprintf(fp, TEXT("%s\n"), str);		
		fclose(fp);
	}
}
//---------------------------------------------------------------------------
static void log(TCHAR* fmt, ...)
{
	return;
	va_list argPtr;
	va_start( argPtr, fmt );		
	TCHAR str[1024];
	_vsntprintf( str, 1024, fmt, argPtr );
	_log(str);
	va_end( argPtr );
}
//---------------------------------------------------------------------------
static void logToFile(TCHAR* fileName, TCHAR* fmt, ...)
{
	return;
	va_list argPtr;
	va_start( argPtr, fmt );		
	TCHAR str[1024];
	_vsntprintf( str, 1024, fmt, argPtr );
	_log(fileName, str);
	va_end( argPtr );
}
//---------------------------------------------------------------------------
static void _tmp_progressLog(const lpStepId_t id)
{
	return;
	static lpStepId_t idPrev = lpdEndOfType;
	static int nRepPrev = 0;

	if(id == lpdNone) {
		idPrev = lpdEndOfType;
		nRepPrev = 0;
		_log(TEXT("_tmp_progress.log"), TEXT("starting new loading"));
		return;
	}

	if(id == idPrev) {
		nRepPrev++;
		return;
	}

	if(nRepPrev) {
		logToFile(TEXT("_tmp_progress.log"), TEXT("id:%d nRep:%d"), idPrev, nRepPrev);
	}	
	idPrev = id;
	nRepPrev = 1;	
}
//---------------------------------------------------------------------------
static void _makeHisFileName(TCHAR* dest, const TCHAR* src)
{
	_tcscpy(dest, src);	
	int len = _tcslen(dest);
	if(len>=5) {
		if(dest[len-4] == TEXT('.')) {
			dest[len-4] = 0x0;
		}
	}	
	_tcscat(dest, TEXT(".his"));
}
//---------------------------------------------------------------------------
void loadingProgress_t::calcStepPositions(TArray<plpdStep_t>* pSteps) // name?
{
	if(pSteps) {
		int nPos = 0;
		for (INT lpp = 0; lpp < pSteps->Num(); ++lpp) {
			plpdStep_t pStepCur = (*pSteps)(lpp);
			if(pStepCur) {
				pStepCur->nStepPosition = nPos;
				log(TEXT("[loadingProgress_t::calcStepPositions] idx:%d id:%d nRep:%d nSp:%d"), lpp, pStepCur->id, pStepCur->nRepetition, pStepCur->nStepPosition);
				nPos += pStepCur->nRepetition;				
			}
		}
		log(TEXT("[loadingProgress_t::calcStepPositions] nMaxPosition=%d"), nPos);
	}
	else {
		log(TEXT("[loadingProgress_t::calcStepPositions] invalid pSteps"));
	}
}
//---------------------------------------------------------------------------
void loadingProgress_t::clearContext(void)
{
	_tcscpy(mapName, TEXT(""));
	log(TEXT("[loadingProgress_t::clearContext] clearing %d items"), lpdSteps.Num());

	for(int lpp=0;lpp<lpdSteps.Num();lpp++) {
		plpdStep_t pStep = lpdSteps(lpp);
		if(pStep) delete pStep;		
	}
	lpdSteps.Empty();
}
//---------------------------------------------------------------------------
void loadingProgress_t::clearRecording(void)
{
	log(TEXT("[loadingProgress_t::clearRecording] clearing %d items"), lpdStepsRecording.Num());

	for(int lpp=0;lpp<lpdStepsRecording.Num();lpp++) {
		plpdStep_t pStep = lpdStepsRecording(lpp);
		if(pStep) delete pStep;		
	}
	lpdStepsRecording.Empty();
}
//---------------------------------------------------------------------------
void loadingProgress_t::begin(const TCHAR* _mapName)
{
	if(_mapName) {		
		log(TEXT("[loadingProgress_t::begin] begining %s"), _mapName); //debugf(TEXT("[loadingProgress_t::begin] begining %s"), _mapName);

		logToFile(TEXT("_tmp_record.log"), TEXT("starting new recording..."));
		_tmp_progressLog(lpdNone);
		
		pStepRecording = 0x0; // [+] 20070622
		pStepReference = 0x0; // plpdStep_t
		idxRefStep = 0; // int
		nCurRepetition = 0; // int
		nPosition = 0;
		nMaxPosition = 0;
		bUseHistory = false;
		nPositionWithoutHistory = 0;
		nMaxPositionWithoutHistory = MaxPositionWithoutHistory;		
		
		_tcscpy(mapName, _mapName);
		setupSteps(mapName);		
		
		if(lpdSteps.Num()>idxRefStep) pStepReference = lpdSteps(idxRefStep);				
		timePrev = timeCurrent = timeBegin = timeGetTime();		
		bActive = true;
	}
	else {
		log(TEXT("[loadingProgress_t::begin] Invalid map name"));
	}
}
//---------------------------------------------------------------------------
bool loadingProgress_t::stepsDiff(void)
{
	if(lpdStepsRecording.Num() > lpdSteps.Num()) {		
		logToFile(TEXT("_tmp_history.log"), TEXT("[stepsDiff] 1.Num %d %d"), lpdStepsRecording.Num(), lpdSteps.Num());
		return true;
	}

	if(lpdStepsRecording.Num() == lpdSteps.Num()) {
		int nCurPos = 0;
		bool bDifferent = false;
		for(int lpp=0;lpp<lpdSteps.Num();lpp++) {
			plpdStep_t pStep = lpdSteps(lpp);
			plpdStep_t pStepRec = lpdStepsRecording(lpp);

			if(0x0 == pStep || 0x0 == pStepRec) {
				logToFile(TEXT("_tmp_history.log"), TEXT("[stepsDiff] 2.Null 0x%x 0x%x"), pStep, pStepRec);
				bDifferent = true;
			}
			if(pStep->id != pStepRec->id) {
				logToFile(TEXT("_tmp_history.log"), TEXT("[stepsDiff] 3.Id %d %d"), pStep->id, pStepRec->id);
				bDifferent = true;
			}
			if(pStepRec->nRepetition > pStep->nRepetition) {
				logToFile(TEXT("_tmp_history.log"), TEXT("[stepsDiff] 4.nRepetition increased %d->%d for %d"), pStep->nRepetition, pStepRec->nRepetition, pStepRec->id);				
				bDifferent = true;				
			}	
			pStepRec->nRepetition = pStepRec->nRepetition > pStep->nRepetition ? pStepRec->nRepetition: pStep->nRepetition;
			pStepRec->nStepPosition = nCurPos;
			nCurPos += pStepRec->nRepetition;			
		}
		return bDifferent;
	}

	return false;
}
//---------------------------------------------------------------------------
void loadingProgress_t::end(void)
{
	bActive = false;

	_tmp_progressLog(lpdEndOfType);

	calcStepPositions(&lpdStepsRecording);
	if(stepsDiff()) {		
		TCHAR historyFileName[512];
		_makeHisFileName(historyFileName, mapName);

		TCHAR fullPath[512] = TEXT("");

		bool bReadOnly = false;
		makeFullPath(fullPath, &bReadOnly, historyFileName, desiredPath, true);
		log(TEXT("fullPath = %s"), fullPath);

		if(!bReadOnly) {
			logToFile(TEXT("_tmp_history.log"), TEXT("new history for %s"), fullPath);
			FILE* fp = _tfopen(fullPath, TEXT("wb"));
			if(fp) {			
				for(int lpp=0;lpp<lpdStepsRecording.Num();lpp++) {
					plpdStep_t plpdStep = lpdStepsRecording(lpp);
					if(plpdStep) {
						logToFile(TEXT("_tmp_history.log"), TEXT("id:%d nRep:%d nSp:%d"), plpdStep->id, plpdStep->nRepetition, plpdStep->nStepPosition);
						fwrite(plpdStep, sizeof(lpdStep_t), 1, fp);
					}
				}
				fclose(fp);
			}		
		}
		else {
			log(TEXT("access denied %s"), fullPath);		
		}		
	}
	clearRecording();
	clearContext();
	log(TEXT("[loadingProgress_t::end] done."));
}
//---------------------------------------------------------------------------
void loadingProgress_t::record(const lpStepId_t id)
{
	if(0x0 == pStepRecording || pStepRecording->id != id) {
		//log(TEXT("[loadingProgress_t::record] New step found %d"), id);
		plpdStep_t plpdStep = new lpdStep_t();
		if(plpdStep) {
			plpdStep->id = id;
			lpdStepsRecording.Push(plpdStep);
			pStepRecording = plpdStep;

			logToFile(TEXT("_tmp_record.log"), TEXT("id:%d"), id);
		}
	}
	else {
		pStepRecording->nRepetition++;
		//log(TEXT("[loadingProgress_t::record] Step %d repeated %d"), id, pStepRecording->nRepetition);
	}
}
//---------------------------------------------------------------------------
void loadingProgress_t::gotoNextStep(void)
{
	pStepReference = 0x0;
	if(idxRefStep+1 < lpdSteps.Num()) {
		pStepReference = lpdSteps(++idxRefStep);
		if(pStepReference) {
			nPosition = pStepReference->nStepPosition;
			log(TEXT("[loadingProgress_t::gotoNextStep] id=%d"), pStepReference->id);
		}
		nCurRepetition = 0;		
	}
	else {
		log(TEXT("[loadingProgress_t::gotoNextStep] no more reference found %d/%d"), nPosition, nMaxPosition);
	}
}
//---------------------------------------------------------------------------
bool loadingProgress_t::gotoStep(int id)
{	
	do {
		gotoNextStep();
	} while(pStepReference && pStepReference->id != id);	
	return pStepReference ? true : false;
}

//---------------------------------------------------------------------------
void loadingProgress_t::progress(const lpStepId_t id)
{
	if(bActive) {
		_tmp_progressLog(id);
		record(id); // 

		// {{ history가 없을때 가짜 progress
		if(!bUseHistory) {
			++nPositionWithoutHistory;			
			if(dProgressCallbackFunc) {
				if(nPositionWithoutHistory > nMaxPositionWithoutHistory) nPositionWithoutHistory = nMaxPositionWithoutHistory; // 혹시모를 예외처리
				dProgressCallbackFunc(id, nPositionWithoutHistory, nMaxPositionWithoutHistory, bTickNetClient);
				GCallbackEvent->Send(CALLBACK_LoadingProgress,FVector(nPositionWithoutHistory, nMaxPositionWithoutHistory,0));
			}
			return;
		}
		// }} history가 없을때 가짜 progress

		if(pStepReference) {
			if(pStepReference->id != id) {
				int nPosPrev = nPosition;
				nPosition = pStepReference->nStepPosition + pStepReference->nRepetition;
				log(TEXT("[loadingProgress_t::progress] id:%d->%d pos:%d->%d/%d"), id, pStepReference->id, nPosPrev, nPosition, nMaxPosition);
				if(!gotoStep(id)) { //찾는게 없음
					nPosition++; // 찾는게 없으면 가짜 progress // nPosition = nMaxPosition; 										
				}
			}

			if(pStepReference && pStepReference->id == id) {
				nCurRepetition++;
				nPosition = pStepReference->nStepPosition + nCurRepetition;
				log(TEXT("[loadingProgress_t::progress] id:%d rep:%d sp:%d <- cur:%d"), id, pStepReference->nRepetition, pStepReference->nStepPosition, nCurRepetition);
				if(pStepReference->nRepetition<=nCurRepetition) {
					gotoNextStep();
				}
			}			
		}
		else { // 더이상 등록된 step이 없음. 100%?
			nPosition++; // 찾는게 없으면 가짜 progress // nPosition = nMaxPosition;			
		}	

		if(nPosition > nMaxPosition) nPosition = nMaxPosition; // 혹시모를 예외처리

		// {{ progress 진행 간격을 테스트할때 열어두3		
		/*
		DWORD timeCurrent = timeGetTime();
		log(TEXT("[loadingProgress_t::progress] %d/%d %d ms"), nPosition, nMaxPosition, timeCurrent - timePrev);		
		if(timeCurrent - timePrev > 100) { //dd
			debugf(TEXT("[loadingProgress_t::progress] %d/%d %d ms"), nPosition, nMaxPosition, timeCurrent - timePrev);		
			timePrev = timeCurrent; //dd
		} // dd
		*/
		// }} progress 진행 간격을 테스트할때 열어두3

		if(dProgressCallbackFunc) {			
			dProgressCallbackFunc(id, nPosition, nMaxPosition, bTickNetClient);
			GCallbackEvent->Send(CALLBACK_LoadingProgress,FVector(nPosition, nMaxPosition,0));
		}
		timePrev = timeCurrent;
	}
}
//---------------------------------------------------------------------------
bool loadingProgress_t::makeFullPath(TCHAR* result, bool* pbReadOnly, TCHAR* fileName, TCHAR* desiredPath, bool bMkDir)
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
		//log(TEXT("attrib=%x"), c_file.attrib);
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
void loadingProgress_t::setupSteps(const TCHAR* mapName)
{	
	if(mapName) {
		TCHAR historyFileName[512] = TEXT("");
		_makeHisFileName(historyFileName, mapName);

		TCHAR fullPath[512] = TEXT("");

		makeFullPath(fullPath, 0x0, historyFileName, desiredPath, false);
		log(TEXT("fullPath = %s"), fullPath);

		FILE* fp = _tfopen(fullPath, TEXT("rb"));
		if(fp) {
			logToFile(TEXT("_tmp_history.log"), TEXT("[setupSteps] loading map history %s"), historyFileName);
			// {{ load list
			nStepLoaded = 0;
			lpdStep_t lpdStep;
			while(fread(&lpdStep, sizeof(lpdStep), 1, fp)) {
				plpdStep_t pStep = new lpdStep_t();
				if(pStep) {
					pStep->set(lpdStep);					
					lpdSteps.Push(pStep);
					logToFile(TEXT("_tmp_history.log"), TEXT("[setupSteps] step %d:%d %d loaded"), nStepLoaded, lpdStep.id, lpdStep.nRepetition);
					nStepLoaded++;
					nMaxPosition = lpdStep.nStepPosition + lpdStep.nRepetition;
				}
				else {
					logToFile(TEXT("_tmp_history.log"), TEXT("[setupSteps] creating new step node failed for %d"), lpdStep.id);
				}
			}			
			logToFile(TEXT("_tmp_history.log"), TEXT("[setupSteps] %d steps %d positions loaded for %s"), nStepLoaded, nMaxPosition, fullPath);
			bUseHistory = true;
			// }} load list
			fclose(fp);
		}
		else {
			logToFile(TEXT("_tmp_history.log"), TEXT("[setupSteps] loading %s failed"), fullPath);
		}
	}
}
//---------------------------------------------------------------------------
void loadingProgress_t::setProgressCallback(void (*func)(int idStep, int nPosition, int nMaxPosition, bool bTickNetClient), bool _bTickNetClient)
{
	dProgressCallbackFunc = func;
	bTickNetClient = _bTickNetClient;
}
//---------------------------------------------------------------------------
loadingProgress_t::loadingProgress_t(void)
{
	_tcscpy(desiredPath, DesiredPath);
	dProgressCallbackFunc = 0x0;
	bTickNetClient = true;
	nStepLoaded = 0;
	bActive = false;
	pStepReference = pStepRecording = 0x0;
}
//---------------------------------------------------------------------------
loadingProgress_t::~loadingProgress_t(void)
{	
	clearContext();
	clearRecording();
}
//---------------------------------------------------------------------------
