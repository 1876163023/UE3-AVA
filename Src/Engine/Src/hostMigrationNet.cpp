//
// hostMigrationNet.cpp
//
// deathcure@redduck.com
//---------------------------------------------------------------------------
#include "EnginePrivate.h"
#ifdef UseNativeServer
//---------------------------------------------------------------------------
#include "hostMigrationNet.h"
#include <math.h>
#include <stdio.h>

//---------------------------------------------------------------------------
struct packetHeader_t {
	short type;
	short len;	
};
typedef struct packetHeader_t packetHeader_t;
typedef struct packetHeader_t* ppacketHeader_t;
//---------------------------------------------------------------------------
bool hostMigrationNet_t::sendPacket(int type, char* payload, int len)
{
	ppacketHeader_t pHeader = (ppacketHeader_t)sendBuffer;
	pHeader->type = type;
	pHeader->len = len;
	if(payload) {		
		memcpy(pHeader+1, payload, pHeader->len);
	}
	if(send(sdTcp, sendBuffer, sizeof(packetHeader_t) + pHeader->len, 0x0)>0) {
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------
bool hostMigrationNet_t::parsePacket(int nByteIncomming)
{
	//log("[parsePacket] nByte:%d", nByteIncomming);

	nByteReceived += nByteIncomming;
	ppacketHeader_t pHeader = (ppacketHeader_t)recvBuffer;
	while((int)sizeof(packetHeader_t) + pHeader->len <= nByteReceived) {
		char payload[1024];
		memcpy(payload, pHeader+1, pHeader->len);
		payload[pHeader->len] = 0x0;
		onPacketComplete(pHeader->type, payload);
		int packetSize = pHeader->len + sizeof(packetHeader_t);
		memmove(recvBuffer, recvBuffer + packetSize, 65536 - packetSize);
		nByteReceived -= packetSize;
	}
	return true;
}
//---------------------------------------------------------------------------
void hostMigrationNet_t::onPacketComplete(int type, char* payload)
{
	switch(type) {
		case pdAckAnonymousJoin: { // 'ack anonymousjoin'	
			int offset = 0;
			memcpy(&id, &payload[offset], sizeof(int)); offset += sizeof(int);
			memcpy(&udpPort, &payload[offset], sizeof(short)); offset += sizeof(short);			
			if(log) log("[ack anonymousjoin] id:%d udpPort:%d", id, udpPort);
			initUdpEchoServer(udpPort);			
		} break;
		case pdNtfAnonymousJoin: // 'ntf anonymousjoin'						
			if(log) log("[ntf anonymousjoin] op:%s", payload);			
			break;
		case pdReqBeHost: // 'req behost'
			LOG("[req behost] Being new host");
			sendPacket(pdAckBeHost);
			break;
		case pdNtfBeHost: // 'ntf behost',
			if(log) log("[ntf behost]");
			break;
		case pdNtfNoParticipant: // 'ntf noparticipant'
			if(log) log("[ntf noparticipant] quit");
			break;
		case pdNtfEndGame: // 'ntf noparticipant'
			if(log) log("[ntf endgame] quit");
			break;
		case pdNtfDeprive: // 'ntf deprive'
			LOG("[ntf deprive]");
			break;
		case pdNtfMigratedHost: { // 'ntf migratedhost'
			int idNewHost;
			sscanf(payload, "%d", &idNewHost);
			if(idNewHost == id) {
				LOG("[ntf migratedhost] Being new host");
			}
			else {
				LOG("Connecting to new host");
			}
		} break;

		case pdAckPeerList: {
			short nItem = 0;
			int offset = 0;
			hmPeerInfo_t pi;
			memcpy(&nItem, &payload[offset], sizeof(short)); offset += sizeof(short);
			for(int lpp=0;lpp<nItem;lpp++) {				
				in_addr in;
				memcpy(&pi.id, &payload[offset], sizeof(int)); offset += sizeof(int);
				memcpy(&in, &payload[offset], sizeof(in_addr)); offset += sizeof(in_addr);
				strcpy(pi.ipAddr, inet_ntoa(in));
				memcpy(&pi.port, &payload[offset], sizeof(short)); offset += sizeof(short);
				if(log) log("--pdAckPeerList id:%d addr:%s,%d", pi.id, pi.ipAddr, pi.port);
				peers.push_back(pi);
			}
		} break;		
	}
	if(pCallback) {
		pCallback->onPacketComplete(this, type, payload);
	}
}
//---------------------------------------------------------------------------
int hostMigrationNet_t::recv(int nSec)
{
	fd_set readFds, writeFds, exceptFds;
	FD_ZERO(&readFds);
	FD_ZERO(&writeFds);
	FD_ZERO(&exceptFds);

	FD_SET(sdTcp, &readFds);

	struct timeval timevalue;

	timevalue.tv_sec = nSec;
	timevalue.tv_usec = 0;

	::select(sdTcp+1, &readFds, &writeFds, &exceptFds, &timevalue);
	if (FD_ISSET(sdTcp, &readFds)) {		
		int nByteReceived = ::recv(sdTcp, recvBuffer, 256, 0); recvBuffer[nByteReceived] = 0x0;
		return nByteReceived;
	}
	return -1;
}
//---------------------------------------------------------------------------
void hostMigrationNet_t::onTickTcp(void)
{
	int nByte = recv(0);	
	if(nByte>0) {
		parsePacket(nByte);		
	}
	else {
		// disconnect
	}
}
//---------------------------------------------------------------------------
bool hostMigrationNet_t::reqPeerList(void)
{
	peers.clear();
	return sendPacket(pdReqPeerList);
}
//---------------------------------------------------------------------------
bool hostMigrationNet_t::reqCreateRoom(void)
{
	LOG(sendBuffer);
	return false;
}
//---------------------------------------------------------------------------
bool hostMigrationNet_t::reqJoin(void)
{
	char ipAddr[256];

	if(!getMyIpAddress(ipAddr)) return false;

	sprintf(payload, "%s %d", ipAddr, port);
	return sendPacket(0, payload, strlen(payload));
}
//---------------------------------------------------------------------------
bool hostMigrationNet_t::reqNewHost(void)
{
	sprintf(sendBuffer, "req behost %d", id);
	int err = send(sdTcp, sendBuffer, strlen(sendBuffer), 0 );
	LOG(sendBuffer);
	return true;
}
//---------------------------------------------------------------------------
bool hostMigrationNet_t::ntfLinkLost(void)
{
	char payload[1024];
	sprintf(payload, "%d", id);
	return sendPacket(pdNtfLinkLost, payload, strlen(payload));
}
//---------------------------------------------------------------------------
bool hostMigrationNet_t::ntfHardwareInfo(void)
{
	char payload[1024];
	char cpuIdMbs[1024];
	wcstombs(cpuIdMbs, cpuId.c_str(), 1023);
	sprintf(payload, "%d %s", memSizeMB, cpuIdMbs);
	return sendPacket(pdNtfHwInfo, payload, strlen(payload));
}
//---------------------------------------------------------------------------
bool hostMigrationNet_t::ntfNetInfo(void)
{
	char payload[1024];
	sprintf(payload, "%f", avgRtt);
	return sendPacket(pdNtfNetInfo, payload, strlen(payload));
}
//---------------------------------------------------------------------------
bool hostMigrationNet_t::ntfBeNewHost(void)
{	
	return sendPacket(pdNtfBeNewHost);
}
//---------------------------------------------------------------------------
bool hostMigrationNet_t::connect(char* ipAddr, int port)
{
	if(sdTcp!=INVALID_SOCKET) {
		closesocket(sdTcp);
		sdTcp = INVALID_SOCKET;
	}

	sdTcp = socket(AF_INET, SOCK_STREAM, 0);
	if(sdTcp == INVALID_SOCKET) 
		goto cleanup;
	
	SOCKADDR_IN addr;
	memset (&addr, '\0', sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ipAddr);
	addr.sin_port = htons(port);
	int err = ::connect(sdTcp, (struct sockaddr *)&addr, sizeof(addr)); 
	if(err) 
		goto cleanup;
	return true;

cleanup:
	if(sdTcp!=INVALID_SOCKET) {
		closesocket(sdTcp);
		sdTcp = INVALID_SOCKET;
	}
	return false;
}
//---------------------------------------------------------------------------
bool hostMigrationNet_t::disconnect(void)
{
	if(sdTcp!=INVALID_SOCKET) {
		::closesocket(sdTcp);
		sdTcp = INVALID_SOCKET;
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------
bool hostMigrationNet_t::getMyIpAddress(char *address)
{
	struct hostent *hostEnt;
	SOCKADDR_IN sockaddr;
	sockaddr.sin_family=AF_INET;
	char localHostName[256];

	if(address == NULL) {
		return false;
	}

	gethostname(localHostName,sizeof(localHostName));
	hostEnt=gethostbyname(localHostName);
	memcpy(&sockaddr.sin_addr,hostEnt->h_addr,4);
	strcpy(address, inet_ntoa(sockaddr.sin_addr));

	return true;
}
//---------------------------------------------------------------------------
wstring hostMigrationNet_t::scanFromReg(wstring path, TCHAR* name)
{
	HKEY hKey=HKEY_LOCAL_MACHINE;
	HKEY hChildKey=NULL;
	HKEY hChildKey2=NULL;
	wstring RegPath=path;
	wstring RegPathx=RegPath;
	wstring mt=TEXT("");
	
	unsigned int j=0;
	char datakeyname[200];
	BYTE databuffer[512];
	DWORD cchClass, cSubKeys, cchMaxSubKey, cchMaxClass, cchValues, cchMaxValueName, cchMaxValueData;
	DWORD datasize, datatype;
	int retcode = RegOpenKeyEx(hKey, (LPCWSTR)RegPath.c_str(), 0, KEY_ALL_ACCESS, &hChildKey);
	if(retcode==ERROR_SUCCESS) {
		retcode=RegQueryInfoKey(hChildKey, 0x0, &cchClass, NULL, &cSubKeys, &cchMaxSubKey, &cchMaxClass, &cchValues, &cchMaxValueName, &cchMaxValueData, NULL, NULL); // retcode=RegQueryInfoKey(hChildKey, (LPWSTR)RegPath.c_str(), &cchClass, NULL, &cSubKeys, &cchMaxSubKey, &cchMaxClass, &cchValues, &cchMaxValueName, &cchMaxValueData, NULL, NULL);
		if(retcode==ERROR_SUCCESS) {
			for(j=0;j<cchValues;j++) {
				strcpy(datakeyname,"");
				datasize=500;
				cchMaxValueData=100;
				retcode=RegEnumValue(hChildKey, j, (LPWSTR)(&datakeyname[0]), &cchMaxValueData, NULL, &datatype, (LPBYTE)(&(databuffer[0])), &datasize);
				if(retcode==ERROR_SUCCESS) {
					if(_tcscmp((TCHAR*)datakeyname,name)==0) {
						if(datatype ==REG_DWORD)  {
							DWORD a; 
							memcpy(&a,databuffer,sizeof(DWORD)); 
							TCHAR astr[128];
							_stprintf(astr, TEXT("%d"), a);
							mt = wstring(astr);
						} else {
							mt = wstring((TCHAR*)databuffer);
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
void hostMigrationNet_t::onTickUdpEchoServer(void)
{
	fd_set readFds;
	FD_ZERO(&readFds);
	FD_SET(sdUdpServer, &readFds);

	struct timeval timevalue;

	timevalue.tv_sec = 0;
	timevalue.tv_usec = 0;

	::select(sdUdpServer+1, &readFds, 0x0, &readFds, &timevalue);
	if (FD_ISSET(sdUdpServer, &readFds)) {
		int n;
		static char buffer[65536];
		struct sockaddr_in clientaddr;	
		int clilen = sizeof(clientaddr);
		
		n = recvfrom(sdUdpServer, buffer, 65536, 0, (struct sockaddr *)&clientaddr, &clilen);    
		sendto(sdUdpServer, buffer, n, 0, (struct sockaddr *)&clientaddr, clilen);
	}
}
//---------------------------------------------------------------------------
bool hostMigrationNet_t::initUdpEchoServer(unsigned short _port)
{
	port = _port;
	sdUdpServer = INVALID_SOCKET;        
    struct sockaddr_in serveraddr;

    sdUdpServer = socket(AF_INET, SOCK_DGRAM, 0);
    if (sdUdpServer==INVALID_SOCKET) return false;    
    
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(port);

    int res = bind(sdUdpServer, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (res == -1) {
		if (sdUdpServer != INVALID_SOCKET) {
			closesocket(sdUdpServer);
			return false;
		}        
    }
	if(log) log("[*] UDP Echo server started on %d.", port);
	return true;
}
//---------------------------------------------------------------------------
float hostMigrationNet_t::testRtt(char* addr, int port, int nTrial)
{
    sdUdpClient = socket(AF_INET, SOCK_DGRAM, 0);
    if (sdUdpClient==INVALID_SOCKET) return false;    

	struct sockaddr_in serveraddr; 
	int clilen = sizeof(serveraddr);

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(addr);
    serveraddr.sin_port = htons(port);

	char sendMsg[1024] = "[hello]";
	char recvMsg[1024] = "";

	int nSucceeded = 0;
	int nFailed = 0;
	DWORD timeStart = timeGetTime();
	for(int lpp=0;lpp<nTrial;lpp++) {
		nFailed++;
		sendto(sdUdpClient, sendMsg, strlen(sendMsg), 0, (struct sockaddr *)&serveraddr, clilen);
		memset(recvMsg, 0, 1024);
		int nByte = recvfrom(sdUdpClient, recvMsg, 1024, 0, 0x0, 0x0); 
		if(nByte>0) {
			recvMsg[nByte] = 0;
			if(!strcmp(sendMsg, recvMsg)) {
				nFailed--;
				nSucceeded++;
			}
		}
	}
	closesocket(sdUdpClient);

	DWORD timeEnd = timeGetTime();
	DWORD timeElapsed = timeEnd - timeStart;

	if(log) log("[*] RTT test results... %d/%d succeeded %d mulisec", nSucceeded, nTrial, timeElapsed);

	// log result
	return timeElapsed / 1000.0f;
}
//---------------------------------------------------------------------------
bool hostMigrationNet_t::testNet(int nTrial)
{
	float sum = 0.0f;
	int nPeers = 0;
	for(list<hmPeerInfo_t>::iterator it=peers.begin();it!=peers.end();it++) {		
		nPeers++;
		sum += testRtt(it->ipAddr, it->port, nTrial);
	}
	if(nPeers) {
		avgRtt = sum / nPeers;
		if(log) log("[*] testNet results: avgRtt=%f over %d peers", avgRtt, nPeers);
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------
void hostMigrationNet_t::onTick(void)
{
	onTickTcp();
	onTickUdpEchoServer();
}
//---------------------------------------------------------------------------
void hostMigrationNet_t::setCallback(phmNetCallback_t pCb)
{
	pCallback = pCb;
}
//---------------------------------------------------------------------------
hostMigrationNet_t::hostMigrationNet_t(void)
: sdTcp(INVALID_SOCKET)
, avgRtt(9999.0f)
, nByteReceived(0)
, id(-1)
, idHost(-1)
, log(0x0)
, port(7777)
, sdUdpServer(INVALID_SOCKET)
, sdUdpClient(INVALID_SOCKET)
, pCallback(0x0)
{
	wstring ms,ms2;
	TCHAR str[1024];

	// {{ Retrieving CPU
	for(int i=0;i<10;i++) {		
		_stprintf(str, TEXT("Hardware\\Description\\System\\CentralProcessor\\%d"), i);
		ms2 = wstring(str);
		ms=scanFromReg(ms2,TEXT("ProcessorNameString"));
		if(ms.size()>0) {
			cpuId.clear();			
			int idx = 0;
			for(unsigned int lpp=0;lpp<ms.length();lpp++) {
				if(ms[lpp] != _T(' ')) {
					cpuId.push_back(ms[lpp]);
				}
			}
			break;
		}
	}
	// }} Retrieving CPU

	// {{ 20070110 Retrieving GPU	
	wstring gpuId;
	ms = scanFromReg(TEXT("Hardware\\DeviceMap\\Video"),TEXT("\\Device\\Video0"));
	if(ms.size()>0) {		
		wstring machineHeader = TEXT("\\Registry\\Machine\\");
		int len = machineHeader.length();
		if(!ms.compare(0, len, machineHeader)) {
			ms = ms.substr(len, ms.length());
		}
		ms = scanFromReg(ms, TEXT("Device Description"));
		if(ms.size()>0) {
			gpuId.clear();			
			int idx = 0;
			for(unsigned int lpp=0;lpp<ms.length();lpp++) {
				if(ms[lpp] != _T(' ')) {
					gpuId.push_back(ms[lpp]);
				}
			}
		}		
		//cpuId.Trim(TEXT(" ")); // 구현해 붙이든지 그냥 쓰든지			
	}
	// }} 20070110 Retrieving GPU

	MEMORYSTATUS memstat;
	wstring mt,mt2;
	GlobalMemoryStatus(&memstat);

	memSizeMB = (int)floor((float)memstat.dwTotalPhys/1024.0f/1024.0f+1.0f);
	int lpf=0;
}
//---------------------------------------------------------------------------
hostMigrationNet_t::~hostMigrationNet_t(void)
{
	if(sdUdpServer!=INVALID_SOCKET) closesocket(sdUdpServer);
	if(sdUdpClient!=INVALID_SOCKET) closesocket(sdUdpClient);
}
//---------------------------------------------------------------------------
#endif