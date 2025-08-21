//
// hwPerformanceCounter.cpp
//
// deathcure@redduck.com
//---------------------------------------------------------------------------
#include "avaNet.h"
#include "hwPerformanceCounter.h"
#include <windows.h>
#include <winreg.h>
//---------------------------------------------------------------------------
hwInfo_t::hwInfo_t(void)
{
	cpuIdentifier = TEXT("NotEvaluated"); // 20070514
	DWORD nCpuCore = 0x0; // [+] 20070508
	DWORD cpuCoreClock = 0x0; // [+] 20070508
	cpuVendorId = FString(TEXT("NotEvaluated")); // [+] 20070508
	DWORD gpuDeviceId = 0x0; // [+] 20070508
	DWORD gpuVendorId = 0x0; // [+] 20070508
	gpuKey = TEXT("NotEvaluated"); // [+] 20070508

	cpuId = TEXT("NotEvaluated");
	gpuId = TEXT("NotEvaluated");
	kindOfNetwork = TEXT("NotEvaluated");
	memorySize = 0;
	//avgRtt = 99999999.0f;
	appMemzero(adapterAddress, MAX_ADAPTER_ADDRESS_LENGTH * sizeof(BYTE));
}
//---------------------------------------------------------------------------
static FString TrimWS(const FString &str)
{
	FString newstr;

	int idx = 0;
	for(int lpp=0;lpp<str.Len();lpp++) {
		if(str[lpp] != TEXT(' ') && str[lpp] != TEXT('\t')) {
			newstr += str[lpp];
		}
	}
	return newstr;
}
//---------------------------------------------------------------------------
//int hwPerformanceCounterClient_t::getHighestClient(void)
//{
// float fHighScore = 0;
// int idHighScore = -1;
// for(map<int, float>::iterator it=clientRatings.begin();it!=clientRatings.end();it++) {
// if(fHighScore < it->second) {
// fHighScore = it->second;
// idHighScore = it->first;
// }
// }
// return idHighScore;
//}
////---------------------------------------------------------------------------
//float hwPerformanceCounterClient_t::evaludateClient(int id, phwInfo_t pi, unsigned int score, unsigned int nDisconnect, unsigned int lv)
//{
// float fScoreCpu = 0;
// if(cpuRatings.find(pi->cpuId)!=cpuRatings.end()) {
// fScoreCpu = cpuRatings[pi->cpuId];
// }
// float fScoreGpu = 0;
// if(gpuRatings.find(pi->gpuId)!=gpuRatings.end()) {
// fScoreGpu = gpuRatings[pi->gpuId];
// }
//
// // memory size는 잘 안맞을지도 모른다. 확인해라
// float fScoreMemorySize = 0; // pi->memorySize
// if(pi->memorySize>= 2048.0f) {
// fScoreMemorySize = 50;
// }
// else if(pi->memorySize>= 1024.0f) {
// fScoreMemorySize = 20;
// }
// else if(pi->memorySize>= 512.0f) {
// fScoreMemorySize = 10;
// }
//
// float fScoreKindOfNetwork = 0; // pi->cpuId
//
// float fScoreAvgRtt = 0;
// if(pi->avgRtt < 0.01f) {
// fScoreAvgRtt = 80;
// }
// else if(pi->avgRtt < 0.05f) {
// fScoreAvgRtt = 50;
// }
// else if(pi->avgRtt < 0.1f) {
// fScoreAvgRtt = 20;
// }
//
// float fScoreGameScore = score * 2; // score
// float fScoreNumDisconnet = nDisconnect * -100 + lv * 10; // disconnect
// if(fScoreNumDisconnet > 0) {
// fScoreNumDisconnet = 0;
// }
//
// float fScore = fScoreCpu + fScoreGpu + fScoreMemorySize + fScoreKindOfNetwork + fScoreAvgRtt + fScoreGameScore + fScoreNumDisconnet;
// clientRatings[id] = fScore;
// return fScore;
//}
////---------------------------------------------------------------------------
//void hwPerformanceCounterClient_t::clearClients(void)
//{
// clientRatings.clear();
//}
//---------------------------------------------------------------------------
FString hwPerformanceCounterClient_t::scanFromReg(const FString& path, const TCHAR* name)
{
	HKEY hKey=HKEY_LOCAL_MACHINE;
	HKEY hChildKey=NULL;
	HKEY hChildKey2=NULL;
	FString RegPath=path;
	FString RegPathx=RegPath;
	FString mt=TEXT("");

	unsigned int j=0;
	char datakeyname[200];
	BYTE databuffer[512];
	DWORD cchClass, cSubKeys, cchMaxSubKey, cchMaxClass, cchValues, cchMaxValueName, cchMaxValueData;
	DWORD datasize, datatype;
	int retcode = RegOpenKeyEx(hKey, *RegPath, 0, KEY_ALL_ACCESS, &hChildKey);
	if(retcode==ERROR_SUCCESS){
		retcode=RegQueryInfoKey(hChildKey, 0x0, &cchClass, NULL, &cSubKeys, &cchMaxSubKey, &cchMaxClass, &cchValues, &cchMaxValueName, &cchMaxValueData, NULL, NULL); // retcode=RegQueryInfoKey(hChildKey, (LPWSTR)RegPath.c_str(), &cchClass, NULL, &cSubKeys, &cchMaxSubKey, &cchMaxClass, &cchValues, &cchMaxValueName, &cchMaxValueData, NULL, NULL);
		if(retcode==ERROR_SUCCESS){
			for(j=0;j<cchValues;j++){
				strcpy(datakeyname,"");
				datasize=500;
				cchMaxValueData=100;
				retcode=RegEnumValue(hChildKey, j, (LPWSTR)(&datakeyname[0]), &cchMaxValueData, NULL, &datatype, (LPBYTE)(&(databuffer[0])), &datasize);
				if(retcode==ERROR_SUCCESS){
					if(_tcscmp((TCHAR*)datakeyname,name)==0){
						if(datatype ==REG_DWORD){
							DWORD a;
							memcpy(&a,databuffer,sizeof(DWORD));
							//TCHAR astr[128];
							//_stprintf(astr, TEXT("%d"), a);
							//mt = astr;
							mt = appItoa(a);
						}else{
							mt = (TCHAR*)databuffer;
						}
					}
				}
			}
			RegCloseKey(hChildKey);
		}
		RegCloseKey(hKey);
	}
	return mt;
}
//---------------------------------------------------------------------------
void hwPerformanceCounterClient_t::inspectHw(void)
{
	FString ms,ms2;
	//TCHAR str[1024];

	// inspect cpu
	// {{ Retrieving CPU
	for(int i=0;i<10;i++){
		//_stprintf(str, TEXT("Hardware\\Description\\System\\CentralProcessor\\%d"), i);
		//ms2 = str;
		ms2 = FString::Printf(TEXT("Hardware\\Description\\System\\CentralProcessor\\%d"), i);
		ms=TrimWS(scanFromReg(ms2,TEXT("ProcessorNameString")));
		if(ms.Len()>0){
			myHwInfo.cpuId = ms;
			//break;
		}
		// {{ 20070514
		ms=TrimWS(scanFromReg(ms2,TEXT("Identifier")));
		if(ms.Len()>0){
			myHwInfo.cpuIdentifier = ms;
		}
		if(myHwInfo.cpuId != TEXT("NotEvaluated") && myHwInfo.cpuId != TEXT("NotEvaluated")){
			break;
		}
		// }} 20070514
	}

	// {{ 20070508 dEAthcURe|HW #core, core clock, vendorId
	myHwInfo.nCpuCore = 0;
	for(int lpc=0;lpc<10;lpc++){
		//_stprintf(str, TEXT("Hardware\\Description\\System\\CentralProcessor\\%d"), lpc);
		//ms2 = FString(str);
		ms2 = FString::Printf(TEXT("Hardware\\Description\\System\\CentralProcessor\\%d"), lpc);
		ms=TrimWS(scanFromReg(ms2,TEXT("~MHz")));
		if(ms.Len()>0){
			//_stscanf(ms.c_str(), TEXT("%d"), &myHwInfo.cpuCoreClock);
			myHwInfo.cpuCoreClock = appAtoi(*ms);
			myHwInfo.nCpuCore++;
		}
		ms=TrimWS(scanFromReg(ms2,TEXT("VendorIdentifier")));
		if(ms.Len()>0){
			myHwInfo.cpuVendorId = ms;
		}
	}
	// }} 20070508 dEAthcURe|HW #core, core clock, vendorId
	// }} Retrieving CPU

	// inspect gpu
	// {{ 20070110 Retrieving GPU
	FString gpuId;
	ms = scanFromReg(TEXT("Hardware\\DeviceMap\\Video"),TEXT("\\Device\\Video0")).Trim().TrimTrailing();
	if(ms.Len()>0){
		FString machineHeader = TEXT("\\Registry\\Machine\\");
		size_t len = machineHeader.Len();
		//if(!ms.compare(0, len, machineHeader)){
		//	ms = ms.substr(len, ms.length());
		//}
		if (appStrnicmp(*ms, *machineHeader, len) == 0)
		{
			ms = ms.Right(ms.Len() - len);
		}
		ms = TrimWS(scanFromReg(ms, TEXT("Device Description")));
		if(ms.Len()>0){
			myHwInfo.gpuId = ms;
		}
	}
	// }} 20070110 Retrieving GPU

	// {{ 20070508 Retrieving GPU key DeviceCaps에서 가져옴
	extern TD3DRef<IDirect3D9> GDirect3D;
	// Create the Direct3D object if necessary.
	if ( !GDirect3D )
	{
		*GDirect3D.GetInitReference() = Direct3DCreate9(D3D_SDK_VERSION);
	}

	if ( GDirect3D ){
		UINT AdapterIndex = D3DADAPTER_DEFAULT;
		D3DADAPTER_IDENTIFIER9 AdapterIdentifier;
		VERIFYD3DRESULT(GDirect3D->GetAdapterIdentifier(AdapterIndex,0,&AdapterIdentifier));

		extern FString GetGPUKey( INT VendorId, INT DeviceId );
		FString Key = GetGPUKey( AdapterIdentifier.VendorId, AdapterIdentifier.DeviceId );

		myHwInfo.gpuKey = Key;
		myHwInfo.gpuDeviceId = AdapterIdentifier.DeviceId;
		myHwInfo.gpuVendorId = AdapterIdentifier.VendorId;
	}
	else{
		debugf(TEXT("Please install DirectX 9.0c or later (see Release Notes for instructions on how to obtain it)"));
	}
	// }} 20070508 Retrieving GPU key DeviceCaps에서 가져옴

	// inspect memory
	// {{ Retrieving memory size
	MEMORYSTATUSEX memstat;
	memstat.dwLength = sizeof (memstat);
	GlobalMemoryStatusEx(&memstat);
	myHwInfo.memorySize = (float)((memstat.ullTotalPhys+872448) / 1024 / 1024);
	// }} Retrieving memory size

	// inspect ping
	//extern bool ping(const char* ipAddr, float* pAvgRtt, int nTrial, int dataSize, bool bStartupWinsock);
	//float avgSum = 0.0f;
	//float nPeer = 0;
	//for(list<peerInfo_t>::iterator it=peers.begin();it!=peers.end();it++) {
	// float avgRtt;
	// if(ping(it->ipAddr.c_str(), &avgRtt, 3, 32, false)) {
	// avgSum += avgRtt;
	// nPeer++;
	// }
	//}
	//if(nPeer) {
	// myHwInfo.avgRtt = avgSum / nPeer;
	//}
	//else {
	// myHwInfo.avgRtt = 99999999.0f;
	//}
}
//---------------------------------------------------------------------------
//void hwPerformanceCounterClient_t::inspectPing(void)
//{
//	extern bool ping(const char* ipAddr, float* pAvgRtt, int nTrial, int dataSize, bool bStartupWinsock);
//	float avgSum = 0.0f;
//	float nPeer = 0;
//	for(list<peerInfo_t>::iterator it=peers.begin();it!=peers.end();it++){
//		float avgRtt;
//		if(ping(it->ipAddr.c_str(), &avgRtt, 3, 32, false)){
//			avgSum += avgRtt;
//			nPeer++;
//		}
//	}
//	if(nPeer){
//		myHwInfo.avgRtt = avgSum / nPeer;
//	}
//	else{
//		myHwInfo.avgRtt = 99999999.0f;
//	}
//}
//---------------------------------------------------------------------------
void hwPerformanceCounterClient_t::inspectMACAddress()
{
	PIP_ADAPTER_ADDRESSES Addresses = NULL;
	DWORD BufLen = 0;
	DWORD RetVal = 0;

	for (INT i = 0; i < 5; ++i)
	{
		RetVal = GetAdaptersAddresses(AF_INET, 0, NULL, Addresses, &BufLen);
		if (RetVal != ERROR_BUFFER_OVERFLOW)
			break;

		if (Addresses != NULL)
		{
			delete [] (BYTE*)Addresses;
		}

		Addresses = (PIP_ADAPTER_ADDRESSES) new BYTE[BufLen];
		if (Addresses == NULL)
		{
			RetVal = GetLastError();
			break;
		}
	}

	if (RetVal == NO_ERROR)
	{
		PIP_ADAPTER_ADDRESSES AdapterList = Addresses;
		while (AdapterList)
		{
			//_LOG(TEXT("Adapter Info: %s; %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x (%d/%d)"), AdapterList->FriendlyName,
			//										AdapterList->PhysicalAddress[0],
			//										AdapterList->PhysicalAddress[1],
			//										AdapterList->PhysicalAddress[2],
			//										AdapterList->PhysicalAddress[3],
			//										AdapterList->PhysicalAddress[4],
			//										AdapterList->PhysicalAddress[5],
			//										AdapterList->PhysicalAddress[6],
			//										AdapterList->PhysicalAddress[7],
			//										AdapterList->IfType, (INT)AdapterList->OperStatus);
			if ((AdapterList->IfType & IF_TYPE_SOFTWARE_LOOPBACK) == 0 && AdapterList->OperStatus == IfOperStatusUp)
			{
				appMemcpy(myHwInfo.adapterAddress, Addresses->PhysicalAddress, MAX_ADAPTER_ADDRESS_LENGTH * sizeof(BYTE));
				break;
			}

			AdapterList = AdapterList->Next;
		}
	}

	if (Addresses != NULL)
	{
		delete [] (BYTE*)Addresses;
	}
}
//---------------------------------------------------------------------------
//bool hwPerformanceCounterClient_t::addPeer(char* ipAddr)
//{
//	// 중복테스트?
//
//	peerInfo_t pi;
//	pi.ipAddr = string(ipAddr);
//	peers.push_back(pi);
//	return true;
//}
//---------------------------------------------------------------------------
//void hwPerformanceCounterClient_t::clearPeers(void)
//{
//	peers.clear();
//}

//---------------------------------------------------------------------------
//void hwPerformanceCounterClient_t::initTables(void)
//{
// // {{ cpu ratings
// //FString str = TEXT("Intel(R)Core(TM)2CPUT7200@2.00GHz");
// //cpuRatings[str] = 100.0f;
// //cpuRatings.insert(map<FString, float>::value_type(str, 100.0f));
//
// cpuRatings[FString(TEXT("Intel(R)Core(TM)2CPUT7200@2.00GHz"))] = 100.0f;
//
// cpuRatings[TEXT("Intel(R)Pentium(R)DCPU3.73GHz")] = 100.0f;
// cpuRatings[TEXT("Intel(R)Core(TM)2CPUX6800@2.93GHz")] = 100.0f;
// cpuRatings[TEXT("Intel(R)Core(TM)2CPU6700@2.66GHz")] = 100.0f;
// cpuRatings[TEXT("Intel(R)Core(TM)2CPU6600@2.40GHz")] = 100.0f;
// cpuRatings[TEXT("DualCoreAMDOpteron(tm)Processor165")] = 100.0f;
// cpuRatings[TEXT("Intel(R)Core(TM)2CPUT7600@2.33GHz")] = 100.0f;
// cpuRatings[TEXT("Intel(R)Core(TM)2CPU6400@2.13GHz")] = 100.0f;
// cpuRatings[TEXT("AMDAthlon(tm)64FX-60DualCoreProcessor")] = 100.0f;
// cpuRatings[TEXT("AMDOpteron(tm)Processor252")] = 100.0f;
// cpuRatings[TEXT("AMDAthlon(tm)64X2DualCoreProcessor5000+")] = 100.0f;
// cpuRatings[TEXT("Intel(R)Core(TM)2CPUT7400@2.16GHz")] = 100.0f;
// cpuRatings[TEXT("Intel(R)Core(TM)2CPUT5600@1.83GHz")] = 100.0f;
// cpuRatings[TEXT("AMDAthlon(tm)64X2DualCoreProcessor4600+")] = 100.0f;
// cpuRatings[TEXT("Intel(R)Pentium(R)DCPU3.40GHz")] = 100.0f;
// cpuRatings[TEXT("AMDAthlon(tm)64X2DualCoreProcessor3800+")] = 100.0f;
// cpuRatings[TEXT("AMDAthlon(tm)64X2DualCoreProcessor4200+")] = 100.0f;
// cpuRatings[TEXT("DualCoreAMDOpteron(tm)Processor170")] = 100.0f;
// cpuRatings[TEXT("Intel(R)Pentium(R)DCPU2.66GHz")] = 100.0f;
// cpuRatings[TEXT("Intel(R)Core(TM)2CPU6300@1.86GHz")] = 100.0f;
// cpuRatings[TEXT("Intel(R)Core(TM)2QuadCPU@2.66GHz")] = 100.0f;
// cpuRatings[TEXT("Intel(R)Xeon(TM)CPU2.66GHz")] = 100.0f;
// cpuRatings[TEXT("Intel(R)Xeon(TM)CPU3.20GHz")] = 100.0f;
// cpuRatings[TEXT("AMDAthlon(tm)64X2DualCoreProcessor4800+")] = 100.0f;
// cpuRatings[TEXT("AMDAthlon(tm)64X2DualCoreProcessor4600+")] = 100.0f;
// cpuRatings[TEXT("AMDAthlon(tm)64X2DualCoreProcessor4400+")] = 100.0f;
// cpuRatings[TEXT("AMDAthlon(tm)64X2DualCoreProcessor4200+")] = 100.0f;
// cpuRatings[TEXT("Intel(R)Xeon(TM)CPU3.00GHz")] = 100.0f;
// cpuRatings[TEXT("Intel(R)Pentium(R)DCPU3.20GHz")] = 50.0f;
// cpuRatings[TEXT("Intel(R)Pentium(R)DCPU2.80GHz")] = 50.0f;
// cpuRatings[TEXT("Intel(R)Xeon(R)CPU5110@1.60GHz")] = 0.0f;
//
// cpuRatings[TEXT("Intel(R)Pentium(R)4CPU3.00GHz")] = 20.0f;
// cpuRatings[TEXT("Intel(R)Pentium(R)4CPU3.06GHz")] = 20.0f;
// cpuRatings[TEXT("Intel(R)Pentium(R)4CPU3.20GHz")] = 20.0f;
// cpuRatings[TEXT("Intel(R)Pentium(R)4CPU3.40GHz")] = 20.0f;
//
// cpuRatings[TEXT("Intel(R)Celeron(R)CPU2.66GHz")] = 0.0f;
// cpuRatings[TEXT("Intel(R)Celeron(R)CPU2.80GHz")] = 0.0f;
// cpuRatings[TEXT("Intel(R)Celeron(R)CPU3.33GHz")] = 0.0f;
//
// cpuRatings[TEXT("MobileAMDAthlon(tm)64Processor3400+")] = 0.0f;
// cpuRatings[TEXT("mobileAMDAthlon(tm)XP2500+")] = 0.0f;
// cpuRatings[TEXT("AMDSempron(tm)Processor2800+")] = 20.0f;
// cpuRatings[TEXT("AMDAthlon(tm)64Processor2800+")] = 50.0f;
// cpuRatings[TEXT("AMDAthlon(tm)64Processor3000+")] = 50.0f;
// cpuRatings[TEXT("AMDAthlon(tm)64Processor3200+")] = 50.0f;
// cpuRatings[TEXT("AMDAthlon(tm)64Processor3800+")] = 50.0f;
// cpuRatings[TEXT("AMDAthlon(tm)64FX-57Processor")] = 100.0f;
// cpuRatings[TEXT("AMDAthlon(tm)64X2DualCoreProcessor3800+")] = 100.0f;
// cpuRatings[TEXT("Dual-Core AMDOpteron(tm) Processor2210")] = 100.0f;
// // }} cpu ratings
//
// // {{ gpu ratings
// gpuRatings[TEXT("NVIDIAGeForce8800GTX")] = 100.0f;
// gpuRatings[TEXT("NVIDIAGeForce7900GTX")] = 50.0f;
// gpuRatings[TEXT("NVIDIAGeForce7950GT")] = 50.0f;
// gpuRatings[TEXT("NVIDIAGeForce7900GS")] = 50.0f;
// gpuRatings[TEXT("NVIDIAGeForce7900GT/GTO")] = 50.0f;
// gpuRatings[TEXT("NVIDIAGeForce7900GT")] = 50.0f;
// gpuRatings[TEXT("NVIDIAGeForce7800GTX")] = 50.0f;
// gpuRatings[TEXT("NVIDIAGeForce7950GX2")] = 50.0f;
// gpuRatings[TEXT("NVIDIAQuadroFX2500M")] = 50.0f;
// gpuRatings[TEXT("NVIDIAGeForceGo7900GTX")] = 50.0f;
// gpuRatings[TEXT("RadeonX1900CrossFireEdition")] = 50.0f;
// gpuRatings[TEXT("RadeonX1950Pro")] = 50.0f;
// gpuRatings[TEXT("RadeonX1900Series")] = 50.0f;
// gpuRatings[TEXT("RadeonX1800Series")] = 20.0f;
// gpuRatings[TEXT("NVIDIAGeForce7800GT")] = 20.0f;
// gpuRatings[TEXT("RadeonX1800GTO")] = 20.0f;
// gpuRatings[TEXT("NVIDIAGeForce7800GS")] = 20.0f;
// gpuRatings[TEXT("ATIMobilityRadeonX1800")] = 20.0f;
// gpuRatings[TEXT("NVIDIAGeForce7600GT")] = 20.0f;
// gpuRatings[TEXT("NVIDIAGeForce6800SeriesGPU")] = 10.0f;
// gpuRatings[TEXT("NVIDIAGeForce6800GS")] = 10.0f;
// // }} gpu ratings
//}
//---------------------------------------------------------------------------
hwPerformanceCounterClient_t::hwPerformanceCounterClient_t(void)
{
}
//---------------------------------------------------------------------------
