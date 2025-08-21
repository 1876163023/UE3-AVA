//
// Ip2pNtTester.h
//
// deathcure@redduck.com
//---------------------------------------------------------------------------
#ifndef Ip2pNtTesterH
#define Ip2pNtTesterH
//---------------------------------------------------------------------------
#include <winsock2.h>
#include <stdio.h>
//---------------------------------------------------------------------------
struct Ip2pNetTester_t {
	//struct ini_t {
	//	char p2psIpAddr[256];
	//	int p2psPort;

	//	char chsIpAddr[256];
	//	int chsPort;

	//	virtual bool save(char* fileName = "p2pNetTester.ini") {
	//		FILE* fp = fopen(fileName, "wt");
	//		if(fp) {				
	//			fprintf(fp, "p2psIpAddr %s\n", p2psIpAddr);
	//			fprintf(fp, "p2psPort %d\n", p2psPort);

	//			fprintf(fp, "chsIpAddr %s\n", chsIpAddr);
	//			fprintf(fp, "chsPort %d\n", chsPort);
	//			
	//			fclose(fp);
	//			return true;
	//		}
	//		return false;
	//	}

	//	virtual bool load(char* fileName = "p2pNetTester.ini") {
	//		FILE* fp = fopen(fileName, "rt");
	//		if(fp) {
	//			char token[256];

	//			while(fscanf(fp, "%s", token)!=EOF) {
	//				if(!strcmp(token, "p2psIpAddr")) {
	//					fscanf(fp, "%s", p2psIpAddr);
	//				}
	//				else if(!strcmp(token, "p2psPort")) {
	//					fscanf(fp, "%d", &p2psPort);
	//				}
	//				else if(!strcmp(token, "chsIpAddr")) {
	//					fscanf(fp, "%s", chsIpAddr);
	//				}
	//				else if(!strcmp(token, "chsPort")) {
	//					fscanf(fp, "%d", &chsPort);
	//				}
	//			}
	//			fclose(fp);
	//			return true;
	//		}
	//		return false;
	//	}

	//	ini_t(void) : p2psPort(7799), chsPort(62000)
	//	{
	//		strcpy(p2psIpAddr, "127.0.0.1");
	//		strcpy(chsIpAddr, "127.0.0.1");

	//		if(!load()) save();
	//	}	
	//};

	virtual void onAckP2pConnect(DWORD idPeer, SOCKADDR_IN* addrLocal, SOCKADDR_IN* addrPublic) {}
	virtual void onNtfP2pConnect(DWORD idRequester, SOCKADDR_IN* addrLocal, SOCKADDR_IN* addrPublic) {}
	virtual void onSessionEstablished(DWORD idPeer) {}
	virtual void onSessionDisconnected(DWORD idPeer) {}
	virtual void onRttProbAck(DWORD idPeer) {}
};
typedef struct Ip2pNetTester_t Ip2pNetTester_t;
typedef struct Ip2pNetTester_t* pIp2pNetTester_t;
//---------------------------------------------------------------------------
#endif
