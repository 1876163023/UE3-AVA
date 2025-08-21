//
// hwPerformanceCounter.h
//
// deathcure@redduck.com
//---------------------------------------------------------------------------
#ifndef hwPerformanceCounterH
#define hwPerformanceCounterH

#include <IpHlpApi.h>

//---------------------------------------------------------------------------
struct hwInfo_t {
	FString cpuIdentifier; // 20070514 x86 Family 6 Model 15 Stepping 6
	DWORD nCpuCore; // [+] 20070508
	DWORD cpuCoreClock; // [+] 20070508
	FString cpuVendorId; // [+] 20070508
	FString cpuId;

	DWORD gpuDeviceId; // [+] 20070508
	DWORD gpuVendorId; // [+] 20070508
	FString gpuKey; // [+] 20070508
	FString gpuId;
	FString kindOfNetwork;
	float memorySize;
	//float avgRtt;

	BYTE adapterAddress[MAX_ADAPTER_ADDRESS_LENGTH];

	hwInfo_t(void);
};
typedef struct hwInfo_t hwInfo_t;
typedef struct hwInfo_t* phwInfo_t;
//---------------------------------------------------------------------------
//struct peerInfo_t {
//	string ipAddr;
//};
//typedef struct peerInfo_t peerInfo_t;
//typedef struct peerInfo_t* ppeerInfo_t;
//---------------------------------------------------------------------------

// hwPerformanceCounter를 클라이언트 용으로 분리

struct hwPerformanceCounterClient_t {
	hwInfo_t myHwInfo;
	//list<peerInfo_t> peers;

	//map<int, float> clientRatings; // key:idClient, data:rating

	//map<FString, float, FString_less> cpuRatings; // key:cpuId, data:rating
	//map<FString, float, FString_less> gpuRatings; // key:gpuId, data:rating

	//int getHighestClient(void);
	//float evaludateClient(int id, phwInfo_t pinfo, unsigned int score, unsigned int disconnect, unsigned int lv); // server function
	//void clearClients(void);

	FString scanFromReg(const FString& path, const TCHAR* name);
	void inspectHw(void); // client function
	//void inspectPing(void);
	void inspectMACAddress();

	//bool addPeer(char* ipAddr);
	//void clearPeers(void);

	//void initTables(void);

	hwPerformanceCounterClient_t(void);
};
typedef struct hwPerformanceCounterClient_t hwPerformanceCounterClient_t;
typedef struct hwPerformanceCounterClient_t* phwPerformanceCounterClient_t;
//---------------------------------------------------------------------------
#endif