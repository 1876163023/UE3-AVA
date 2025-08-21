//
// loadingProgress.h
//
// deathcure@redduck.com
//---------------------------------------------------------------------------
#ifndef loadingProgressH
#define loadingProgressH
//---------------------------------------------------------------------------
enum lpStepId_t {
	lpdNone,
	lpdLinkerLoadAllObjects,
	lpdEndLoad,
	lpdCALLBACK_PreLoadMap,
	lpdCleanupPerMapPackages,
	lpdCancelPendingMapChange,
	lpdCancelTravel,
	lpdGInitRunaway,
	lpdFlushLevelStreaming,
	lpdTermWorldRBPhys,
	lpdCleanupWorld,
	lpdCALLBACK_LevelRemovedFromWorld,
	lpdDestroyActor,
	lpdGWorldNULL,
	lpdAudioDeviceFlush,
	lpdCollectGarbage,
	lpdLoadPackage,
	lpdGWorldAddToRoot,
	lpdGWorldInit,
	lpdSetGameInfo,
	lpdBeginPlay,
	lpdSpawnPlayActor,
	lpdSaveURLConfig,
	lpdNotifyLevelChange,
	lpdCALLBACK_PostLoadMap,
	lpdLoadPerMapPackages,
	lpdUpdateComponents,
	lpdInitWorldRBPhys,
	lpdInitLevelBSPPhysMesh,
	lpdIncrementalInitActorsRBPhys,
	lpdInitializeActors,
	lpdEventInitGame,
	lpdRouteBeginPlay,	
	lpdSortActorList,
	lpdPrePostLoad, // <- 34	
	lpdEndOfType
};
//---------------------------------------------------------------------------
struct lpdStep_t {
	lpStepId_t id;
	int nRepetition;
	int nStepPosition;

	void set(const lpdStep_t& src) {
		id = src.id;
		nRepetition = src.nRepetition;
		nStepPosition = src.nStepPosition;
	}

	lpdStep_t(void) {
		id = lpdNone;
		nRepetition = 1;
		nStepPosition = 0;
	}
};
typedef struct lpdStep_t lpdStep_t;
typedef struct lpdStep_t* plpdStep_t;
//---------------------------------------------------------------------------
struct loadingProgress_t {	
	TArray<plpdStep_t> lpdStepsRecording;
	int nStepLoaded;
	TArray<plpdStep_t> lpdSteps;	
	
	// {{ 
#define DesiredPath TEXT("..\\avaGame\\Content\\Maps\\Progress\\")
	TCHAR desiredPath[512];	
	bool makeFullPath(TCHAR* result, bool* pbReadOnly, TCHAR* fileName, TCHAR* desiredPath, bool bMkDir);
	bool bUseHistory;
	int nPositionWithoutHistory;
	int nMaxPositionWithoutHistory;

#define MaxPositionWithoutHistory 60000

	DWORD timeBegin;
	DWORD timeCurrent;
	DWORD timePrev;
	bool bActive;
	TCHAR mapName[512];
	plpdStep_t pStepRecording;
	plpdStep_t pStepReference;
	int idxRefStep;
	int nCurRepetition;
	int nPosition, nMaxPosition;	

	void clearContext(void);
	void clearRecording(void);
	void calcStepPositions(TArray<plpdStep_t>* pSteps);
	void begin(const TCHAR* _mapName);
	void end(void);
	void record(const lpStepId_t id);
	void gotoNextStep(void);
	bool gotoStep(int id);
	void progress(const lpStepId_t id);
	void setupSteps(const TCHAR* mapName);
	bool stepsDiff(void);

	void (*dProgressCallbackFunc)(int idStep, int nPosition, int nMaxPosition, bool bTickNetClient);
	bool bTickNetClient;
	void setProgressCallback(void (*func)(int idStep, int nPosition, int nMaxPosition, bool bTickNetClient), bool bTickNetClient);
	// }} 

	loadingProgress_t(void);
	~loadingProgress_t(void);
};
typedef struct loadingProgress_t loadingProgress_t;
typedef struct loadingProgress_t* ploadingProgress_t;
//---------------------------------------------------------------------------
extern loadingProgress_t g_loadingProgress;

#define EnableLoadingProgress
#ifdef EnableLoadingProgress
/*
#define LoadingProgressTo(a)	g_loadingProgress.progress(a);
#define LoadingProgressToS(a, b, c)	g_loadingProgress.progress(a, b, c);
#define LoadingProgress()	g_loadingProgress.progress();
#define ModifyProgress(a,b)	g_loadingProgress.modifyStep(a, b);
#define BeginLoadingProgress() g_loadingProgress.begin();
#define EndLoadingProgress() g_loadingProgress.end();
*/

#define dSetProgressCallback(a, b) g_loadingProgress.setProgressCallback(a, b);
#define dLoadingProgress(a)	g_loadingProgress.progress(a);
#define dBeginLoadingProgress(a) g_loadingProgress.begin(a);
#define dEndLoadingProgress() g_loadingProgress.end();
#else
/*
#define LoadingProgressTo(a)
#define LoadingProgressToS(a, b, c)
#define LoadingProgress()
#define ModifyProgress(a,b)
#define BeginLoadingProgress()
#define EndLoadingProgress()
*/

#define dSetProgressCallback(a, b)
#define dLoadingProgress(a)
#define dBeginLoadingProgress(a)
#define dEndLoadingProgress()
#endif
//---------------------------------------------------------------------------
#endif