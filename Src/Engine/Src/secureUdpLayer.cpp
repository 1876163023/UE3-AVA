//
// secureUdpLayer.cpp
//
// deathcure@redduck.com
//---------------------------------------------------------------------------
#include "EnginePrivate.h"
#include "P2PClientEx.h"
#include "secureUdpLayer.h"
//#define _CRT_RAND_S
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//---------------------------------------------------------------------------
// global def
//---------------------------------------------------------------------------
#define DefaultKeyDistanceThreshold 5
#define InitialChecksum 0x9
//#define EnableNativeLog // 로그를 생성하려면 활성화
#define UseChecksum

#ifdef LogToFile
#undef LogToFile
#endif

#ifdef EnableNativeLog
#define LogToFile logToFile
#else
#define LogToFile __noop
#endif
//---------------------------------------------------------------------------
// secureUdpLayer_t::rc4_t
//---------------------------------------------------------------------------
void secureUdpLayer_t::rc4_t::swap_byte(unsigned char *a, unsigned char *b)
{
	unsigned char swapByte; 

	swapByte = *a; 
	*a = *b;      
	*b = swapByte;
}
//---------------------------------------------------------------------------
void secureUdpLayer_t::rc4_t::prepare_key(unsigned char *key_data_ptr, int key_data_len, pkey_t key)
{
	//unsigned char swapByte;
	unsigned char index1;
	unsigned char index2;
	unsigned char* state;
	short counter;     

	state = &key->state[0];         
	for(counter = 0; counter < 256; counter++)              
		state[counter] = counter;               
	key->x = 0;     
	key->y = 0;     
	index1 = 0;     
	index2 = 0;             
	for(counter = 0; counter < 256; counter++) {               
		index2 = (key_data_ptr[index1] + state[counter] + index2) % 256;                
		swap_byte(&state[counter], &state[index2]);
		index1 = (index1 + 1) % key_data_len;  
	}       
}
//---------------------------------------------------------------------------
void secureUdpLayer_t::rc4_t::rc4(unsigned char *buffer_ptr, int buffer_len, pkey_t key)
{ 
	unsigned char x;
	unsigned char y;
	unsigned char* state;
	unsigned char xorIndex;
	short counter;              

	x = key->x;     
	y = key->y;     

	state = &key->state[0];         
	for(counter = 0; counter < buffer_len; counter ++) {               
		x = (x + 1) % 256;                      
		y = (state[x] + y) % 256;               
		swap_byte(&state[x], &state[y]);                        

		xorIndex = (state[x] + state[y]) % 256;                 

		buffer_ptr[counter] ^= state[xorIndex];         
	}               
	key->x = x;     
	key->y = y;
}
//---------------------------------------------------------------------------
// secureUdpLayer_t::peerContext_t
//---------------------------------------------------------------------------
bool secureUdpLayer_t::peerContext_t::init(void)
{
	idxKeyRingRecv = 0;
	idxKeyRingSend = 1; // send용 idx에 0은 사용하지 않는다.
	memset(keyHistory, 0x0, SUL_KeyHistoryLength);
	idxKeyHistory = 0x0;

	nChecksumFailed = 0;
	nPacketReplayed = 0;
	nUnexpectedKeyIndex = 0;

	LogToFile(0x0, TEXT("[secureUdpLayer_t::peerContext_t::init]")); //debugf(TEXT("[secureUdpLayer_t::peerContext_t::init]"));
	return true;
}
//---------------------------------------------------------------------------
secureUdpLayer_t::peerContext_t::peerContext_t(QWORD idAddr)
{
	addr = idAddr;
	init();
	LogToFile(0x0, TEXT("[secureUdpLayer_t::peerContext_t::peerContext_t] 0x%I64x"), addr);
};
//---------------------------------------------------------------------------
// secureUdpLayer_t
//---------------------------------------------------------------------------
QWORD secureUdpLayer_t::makePeerContextHandle(SOCKADDR_IN addr)
{
	QWORD hContext = ((QWORD)(addr.sin_addr.S_un.S_addr) << 16) | (QWORD)ntohs(addr.sin_port);	
	//LogToFile(0x0, TEXT("hContext = 0x%I64x"), hContext);
	return hContext;
}
//---------------------------------------------------------------------------
unsigned char secureUdpLayer_t::keyDistance(unsigned char k1, unsigned char k2)
{
	unsigned char d1 = k2>k1?k2-k1:k1-k2;
	unsigned char d2 = (unsigned char)(256 - (int)d1);
	unsigned char diff = d2<d1?d2:d1;

	//LogToFile(0x0, TEXT("[secureUdpLayer_t::keyDistance] diff=%d"), diff);
	return diff;
}
//---------------------------------------------------------------------------
void secureUdpLayer_t::makeKey(unsigned char key[SUL_KeyLength], unsigned char idxKeyRing)
{
	for(int lpp=0;lpp<SUL_KeyLength;lpp++) {
		key[lpp] = keyRing[((int)idxKeyRing + lpp) % SUL_MaxRing];
	}
}
//---------------------------------------------------------------------------
unsigned char secureUdpLayer_t::calcChecksum(unsigned char* pPacket, int len)
{
	unsigned char checksum = InitialChecksum;
	for(int lpp=0;lpp<len;lpp++) {
		checksum += pPacket[lpp];
	}
	return checksum;
}
//---------------------------------------------------------------------------
#ifdef UseChecksum
//---------------------------------------------------------------------------
unsigned char* secureUdpLayer_t::onRecvPacket(int& lengthPlainText, unsigned char* pPacket, int len, SOCKADDR_IN addr)
{
	if(len> SUL_MaxUdpBuffer) {
		LogToFile(0x0, TEXT("[secureUdpLayer_t::onRecvPacket] packet exceeds the buffer %d"), len);
		lengthPlainText = 0;
		return 0x0;
	}

	QWORD hContext = makePeerContextHandle(addr); // DWORD hContext = (DWORD)addr.sin_addr.S_un.S_addr;

	ppeerContext_t pPC = peerContexts.Find(hContext);
	if(0x0 == pPC) {
		peerContext_t pc(hContext);
		peerContexts.Set(hContext, pc);
		pPC = peerContexts.Find(hContext);
	}

	if(0x0 == pPC) {
		LogToFile(0x0, TEXT("[secureUdpLayer_t::onRecvPacket] peer context not found"));
		lengthPlainText = len;
		return pPacket;
	}

	// check sum을 사용할 때는 cipher packet과 bro packet을 명시적으로 구분하지 못한다.

	unsigned char idxKeyRing = 0x0;
	memcpy(&idxKeyRing, &pPacket[0], sizeof(unsigned char));	
	unsigned char checksum = 0x0;
	memcpy(&checksum, &pPacket[1], sizeof(unsigned char));
	
	if(0 == pPC->idxKeyRingRecv) {
		// 최초 recv, idx를 sync하고 history를 정리한다.
		pPC->idxKeyRingRecv = idxKeyRing;
		for(int lpp=0;lpp<SUL_KeyHistoryLength;lpp++) {
			pPC->keyHistory[lpp] = idxKeyRing;
		}
		pPC->idxKeyHistory = 0x0;
		//LogToFile(0x0, TEXT("[secureUdpLayer_t::onRecvPacket] Sync idx for the first transmission for %I64x"), pPC->addr);
	}
	else {
		for(int lpp=0;lpp<SUL_KeyHistoryLength;lpp++) {
			if(pPC->keyHistory[lpp] == idxKeyRing) {
				pPC->nPacketReplayed++;
				LogToFile(0x0, TEXT("[secureUdpLayer_t::onRecvPacket] Replayed packet received %d x %d"), idxKeyRing, pPC->nPacketReplayed);
				lengthPlainText = 0;
				return 0x0;
			}
		}	
		pPC->keyHistory[pPC->idxKeyHistory] = idxKeyRing;			
		pPC->idxKeyHistory = (pPC->idxKeyHistory + 1) % SUL_KeyHistoryLength;
		//debugf(TEXT("[secureUdpLayer_t::onRecvPacket] key history %x %x %x %x %x %x %x %x"), pPC->keyHistory[0], pPC->keyHistory[1], pPC->keyHistory[2], pPC->keyHistory[3], pPC->keyHistory[4], pPC->keyHistory[5], pPC->keyHistory[6], pPC->keyHistory[7]);
	}

	#ifdef EnableNativeLog
	TCHAR str[256] = TEXT("");
	mbstowcs(str, inet_ntoa(addr.sin_addr), 255);
	LogToFile(0x0, TEXT("[secureUdpLayer_t::onRecvPacket] key expected:incoming =  %d:%d for %s:%d"), pPC->idxKeyRingRecv, idxKeyRing, str, ntohs(addr.sin_port));
	#endif

	if(keyDistance(pPC->idxKeyRingRecv, idxKeyRing) <= keyDistanceThreshold) {
		pPC->idxKeyRingRecv = idxKeyRing+1;
		if(0 == pPC->idxKeyRingRecv) pPC->idxKeyRingRecv++; // idx에 0은 기대하지 않는다.

		lengthPlainText = len - sizeof(unsigned char)*2;
		unsigned char key[SUL_KeyLength] = "";
		makeKey(key, idxKeyRing);

		rc4_t::key_t rc4Key;
		rc4_t::prepare_key(key, SUL_KeyLength, &rc4Key);

		memcpy(udpBuffer, &pPacket[2], lengthPlainText);
		rc4_t::rc4(udpBuffer, lengthPlainText, &rc4Key);
		//LogToFile(0x0, TEXT("[secureUdpLayer_t::onRecvPacket] decrypted %d -> %d bytes using key: %x %d %d %d %d %d %d %d"), len, lengthPlainText, key[0], key[1], key[2], key[3], key[4], key[5], key[6], key[7]); // 

		unsigned char checksumRebuilt = calcChecksum(udpBuffer, lengthPlainText);

		if(checksum == checksumRebuilt) {			
			return udpBuffer;
		}
		else {
			pPC->nChecksumFailed++;
			LogToFile(0x0, TEXT("[secureUdpLayer_t::onRecvPacket] checksum failed received:%x rebuilt:%x x %d"), checksum, checksumRebuilt, pPC->nChecksumFailed);
		}
	}		
	else {
		pPC->nUnexpectedKeyIndex++;
		LogToFile(0x0, TEXT("[secureUdpLayer_t::onRecvPacket] unexpected key idx %d (expected %d) for %I64x x %d"), idxKeyRing, pPC->idxKeyRingRecv, hContext, pPC->nUnexpectedKeyIndex);
	}

	lengthPlainText = 0;
	return 0x0;
}
//---------------------------------------------------------------------------
unsigned char* secureUdpLayer_t::onSendPacket(int& lengthCipherText, unsigned char* pPacket, int len, SOCKADDR_IN addr)
{	
	if(len+2> SUL_MaxUdpBuffer) { // +2 = key index + inv key index
		LogToFile(0x0, TEXT("[secureUdpLayer_t::onSendPacket] packet exceeds the buffer %d"), len);
		lengthCipherText = 0;
		return 0x0;
	}

	QWORD hContext = makePeerContextHandle(addr); // DWORD hContext = (DWORD)addr.sin_addr.S_un.S_addr;

	// {{ peer context 초기화
	ppeerContext_t pPC = peerContexts.Find(hContext);
	if(0x0 == pPC) {		
		peerContext_t pc(hContext);
		peerContexts.Set(hContext, pc);
		pPC = peerContexts.Find(hContext);
		LogToFile(0x0, TEXT("[secureUdpLayer_t::onSendPacket] new context created for 0x%I64x"), hContext);
	}
	// }} peer context 초기화

	if(0x0 == pPC) {
		LogToFile(0x0, TEXT("[secureUdpLayer_t::onSendPacket] no context found for 0x%I64x"), hContext); // debugf(TEXT("[secureUdpLayer_t::onSendPacket] peer context not found"));
		lengthCipherText = len;
		return pPacket;
	}

	memcpy(&udpBuffer[0], &pPC->idxKeyRingSend, sizeof(unsigned char));
	unsigned char checksum = calcChecksum(pPacket, len);
	memcpy(&udpBuffer[1], &checksum, sizeof(unsigned char));

	unsigned char key[SUL_KeyLength] = "";
	makeKey(key, pPC->idxKeyRingSend);

	////debugf(TEXT("[secureUdpLayer_t::onSendPacket] key: %x %x %x %x %x %x %x %x"), key[0], key[1], key[2], key[3], key[4], key[5], key[6], key[7]);

	rc4_t::key_t rc4Key;
	rc4_t::prepare_key(key, SUL_KeyLength, &rc4Key);
	memcpy(&udpBuffer[2], pPacket, len);
	rc4_t::rc4(&udpBuffer[2], len, &rc4Key);
	lengthCipherText = len + sizeof(unsigned char) * 2; // dIdxKeyRing | ~dIdxKeyRing | cipher text	
	pPC->idxKeyRingSend++;
	if(0 == pPC->idxKeyRingSend) pPC->idxKeyRingSend++; // send용 idx에 0은 사용하지 않는다.	

	//LogToFile(0x0, TEXT("[secureUdpLayer_t::onSendPacket] encrypted %d -> %d bytes using key: %x %d %d %d %d %d %d %d"), len, lengthCipherText, key[0], key[1], key[2], key[3], key[4], key[5], key[6], key[7]); // 
	return udpBuffer;
}
#else
//---------------------------------------------------------------------------
unsigned char* secureUdpLayer_t::onRecvPacket(int& lengthPlainText, unsigned char* pPacket, int len, SOCKADDR_IN addr)
{
	if(len> SUL_MaxUdpBuffer) {
		LogToFile(0x0, TEXT("[secureUdpLayer_t::onRecvPacket] packet exceeds the buffer %d"), len);
		lengthPlainText = 0;
		return 0x0;
	}

	QWORD hContext = makePeerContextHandle(addr); // DWORD hContext = (DWORD)addr.sin_addr.S_un.S_addr;

	// {{ bro p2p protocol 예외처리
	#ifdef EnablePacketHeading
	if(!memcmp(pPacket, PacketHeading, sizeof(SizeOfHeading))) {		
		// {{ peer context 초기화		
		ppeerContext_t pPC = peerContexts.Find(hContext);
		if(pPC) {
			pPC->init();
			LogToFile(0x0, TEXT("[secureUdpLayer_t::onRecvPacket] init existing context for %I64x"), hContext);
		}
		else {
			peerContext_t pc(hContext);
			peerContexts.Set(hContext, pc);
			LogToFile(0x0, TEXT("[secureUdpLayer_t::onRecvPacket] create new context for %I64x"), hContext);			
		}
		// }} peer context 초기화
		LogToFile(0x0, TEXT("[secureUdpLayer_t::onRecvPacket] bro p2p protocol bypassed"));
		lengthPlainText = len;
		return pPacket;
	}
	#endif
	// }} bro p2p protocol 예외처리

	unsigned char idxKeyRing = 0x0;
	memcpy(&idxKeyRing, &pPacket[0], sizeof(unsigned char));	
	unsigned char inv_idxKeyRing = 0;
	memcpy(&inv_idxKeyRing, &pPacket[1], sizeof(unsigned char));

	if(idxKeyRing == (inv_idxKeyRing ^ 0xff)) {

		ppeerContext_t pPC = peerContexts.Find(hContext);
		if(0x0 == pPC) {
			peerContext_t pc(hContext);
			peerContexts.Set(hContext, pc);
			pPC = peerContexts.Find(hContext);
		}

		if(0x0 == pPC) {
			LogToFile(0x0, TEXT("[secureUdpLayer_t::onRecvPacket] peer context not found")); // debugf(TEXT("[secureUdpLayer_t::onRecvPacket] peer context not found"));
			lengthPlainText = len;
			return pPacket;
		}

		//debugf(TEXT("[secureUdpLayer_t::onRecvPacket] idx expected:%d received:%d"), pPC->idxKeyRingRecv, idxKeyRing);
		if(0 == pPC->idxKeyRingRecv) {
			// 최초 recv, idx를 sync하고 history를 정리한다.
			pPC->idxKeyRingRecv = idxKeyRing;
			for(int lpp=0;lpp<SUL_KeyHistoryLength;lpp++) {
				pPC->keyHistory[lpp] = pPC->idxKeyRingRecv;
			}	
			pPC->idxKeyHistory = 0x0;
			LogToFile(0x0, TEXT("[secureUdpLayer_t::onRecvPacket] Sync idx for the first transmission for %d"), pPC->addr);
		}
		else {
			for(int lpp=0;lpp<SUL_KeyHistoryLength;lpp++) {
				if(pPC->keyHistory[lpp] == idxKeyRing) {
					LogToFile(0x0, TEXT("[secureUdpLayer_t::onRecvPacket] Replayed packet received %d"), idxKeyRing); //debugf(TEXT("[secureUdpLayer_t::onRecvPacket] Replayed packet received %d"), idxKeyRing);
					lengthPlainText = 0;
					return 0x0;
				}
			}	
			pPC->keyHistory[pPC->idxKeyHistory] = idxKeyRing;
			//debugf(TEXT("[secureUdpLayer_t::onRecvPacket] key history %x %x %x %x %x %x %x %x"), pPC->keyHistory[0], pPC->keyHistory[1], pPC->keyHistory[2], pPC->keyHistory[3], pPC->keyHistory[4], pPC->keyHistory[5], pPC->keyHistory[6], pPC->keyHistory[7]);
			pPC->idxKeyHistory = (pPC->idxKeyHistory + 1) % SUL_KeyHistoryLength;
		}

		TCHAR str[256];			
		mbstowcs(str, inet_ntoa(addr.sin_addr), 255);

		LogToFile(0x0, TEXT("[secureUdpLayer_t::onRecvPacket] key expected:incoming =  %d:%d for %s:%d"), pPC->idxKeyRingRecv, idxKeyRing, str, ntohs(addr.sin_port));
		if(keyDistance(pPC->idxKeyRingRecv, idxKeyRing) <= keyDistanceThreshold) {
			pPC->idxKeyRingRecv = idxKeyRing+1;
			if(0 == pPC->idxKeyRingRecv) pPC->idxKeyRingRecv++; // idx에 0은 기대하지 않는다.
			lengthPlainText = len - sizeof(unsigned char)*2;
			unsigned char key[SUL_KeyLength] = "";
			makeKey(key, idxKeyRing);

			rc4_t::key_t rc4Key;
			rc4_t::prepare_key(key, SUL_KeyLength, &rc4Key);

			memcpy(udpBuffer, &pPacket[2], lengthPlainText);
			rc4_t::rc4(udpBuffer, lengthPlainText, &rc4Key);

			//LogToFile(0x0, TEXT("[secureUdpLayer_t::onRecvPacket] decrypted %d -> %d bytes using key: %x %d %d %d %d %d %d %d"), len, lengthPlainText, key[0], key[1], key[2], key[3], key[4], key[5], key[6], key[7]); // 
			return udpBuffer;
		}
		// {{ test packet replay
		else {			
			LogToFile(0x0, TEXT("[secureUdpLayer_t::onRecvPacket] unexpected key idx %d (expected %d) from %s:%d"), idxKeyRing, pPC->idxKeyRingRecv, str, ntohs(addr.sin_port)); // debugf(TEXT("[secureUdpLayer_t::onRecvPacket] unexpected key idx %d (expected %d)"), idxKeyRing, pPC->idxKeyRingRecv);
		}
		// }} test packet replay
	}
	
	lengthPlainText = 0;
	return 0x0;
}
//---------------------------------------------------------------------------
unsigned char* secureUdpLayer_t::onSendPacket(int& lengthCipherText, unsigned char* pPacket, int len, SOCKADDR_IN addr)
{	
	if(len+2> SUL_MaxUdpBuffer) { // +2 = key index + inv key index
		LogToFile(0x0, TEXT("[secureUdpLayer_t::onSendPacket] packet exceeds the buffer %d"), len);
		lengthCipherText = 0;
		return 0x0;
	}

	QWORD hContext = makePeerContextHandle(addr); // DWORD hContext = (DWORD)addr.sin_addr.S_un.S_addr;

	// {{ peer context 초기화
	ppeerContext_t pPC = peerContexts.Find(hContext);
	if(0x0 == pPC) {		
		peerContext_t pc(hContext);
		peerContexts.Set(hContext, pc);
		pPC = peerContexts.Find(hContext);
		LogToFile(0x0, TEXT("[secureUdpLayer_t::onSendPacket] new context created for 0x%I64x"), hContext);
	}
	// }} peer context 초기화

	if(0x0 == pPC) {
		LogToFile(0x0, TEXT("[secureUdpLayer_t::onSendPacket] no context found for 0x%I64x"), hContext); // debugf(TEXT("[secureUdpLayer_t::onSendPacket] peer context not found"));
		lengthCipherText = len;
		return pPacket;
	}

	memcpy(&udpBuffer[0], &pPC->idxKeyRingSend, sizeof(unsigned char));
	unsigned char inv_idxKeyRingSend = pPC->idxKeyRingSend ^ 0xff;	
	memcpy(&udpBuffer[1], &inv_idxKeyRingSend, sizeof(unsigned char));

	unsigned char key[SUL_KeyLength] = "";
	makeKey(key, pPC->idxKeyRingSend);

	////debugf(TEXT("[secureUdpLayer_t::onSendPacket] key: %x %x %x %x %x %x %x %x"), key[0], key[1], key[2], key[3], key[4], key[5], key[6], key[7]);

	rc4_t::key_t rc4Key;
	rc4_t::prepare_key(key, SUL_KeyLength, &rc4Key);
	memcpy(&udpBuffer[2], pPacket, len);
	rc4_t::rc4(&udpBuffer[2], len, &rc4Key);
	lengthCipherText = len + sizeof(unsigned char) * 2; // dIdxKeyRing | ~dIdxKeyRing | cipher text	
	pPC->idxKeyRingSend++;
	if(0 == pPC->idxKeyRingSend) pPC->idxKeyRingSend++; // send용 idx에 0은 사용하지 않는다.	

	//LogToFile(0x0, TEXT("[secureUdpLayer_t::onSendPacket] encrypted %d -> %d bytes using key: %x %d %d %d %d %d %d %d"), len, lengthCipherText, key[0], key[1], key[2], key[3], key[4], key[5], key[6], key[7]); // 
	return udpBuffer;
}
#endif
//---------------------------------------------------------------------------
void secureUdpLayer_t::generateKeyRing(unsigned int randomSeed)
{
	srand(randomSeed);
	for(int lpp=0;lpp<SUL_MaxRing;lpp++) {
		keyRing[lpp] = (unsigned char)(frand() * 255);
	}

	LogToFile(0x0, TEXT("[secureUdpLayer_t::generateKeyRing] random seed = %d"), randomSeed);
}
//---------------------------------------------------------------------------
void secureUdpLayer_t::printDump(unsigned char* str, int len)
{
	bool bPrintable = true;
	for(int lpp=0;lpp<len;lpp++) {
		printf("0x%x ", str[lpp]);
		if(str[lpp] < 32) bPrintable = false;
	}
	printf("\n");

	if(bPrintable) printf("->%s\n", str);
}
//---------------------------------------------------------------------------
float secureUdpLayer_t::frand(void)
{
	unsigned int value = 0;
	return (float)rand() / (float)RAND_MAX;
	/* srand 호환 안됨
	float res = 0.0f;
	if(true && 0x0 == rand_s(&value)) {
		res = (float)value / (float)UINT_MAX;
	}
	else {
		res = (float)rand() / (float)RAND_MAX;
	}	
	return res;*/
}
//---------------------------------------------------------------------------
bool secureUdpLayer_t::initPeer(SOCKADDR_IN addr)
{
	QWORD hContext = makePeerContextHandle(addr); // DWORD hContext = (DWORD)addr.sin_addr.S_un.S_addr;

	ppeerContext_t pPC = peerContexts.Find(hContext);
	if(pPC) {		
		pPC->init();
		LogToFile(0x0, TEXT("[secureUdpLayer_t::initPeer] peer context for 0x%I64x initialized"), hContext);
		return true;		
	}
	return false;
}
//---------------------------------------------------------------------------
bool secureUdpLayer_t::init(int randomSeed)
{
	generateKeyRing(randomSeed);	
	LogToFile(0x0, TEXT("[secureUdpLayer_t::init] initialized random seed = %d"), randomSeed);
	return true;
}
//---------------------------------------------------------------------------
void secureUdpLayer_t::deinit(void)
{
	peerContexts.Empty();
}
//---------------------------------------------------------------------------
void secureUdpLayer_t::_log(TCHAR* fileName, TCHAR* str)
{
	FILE* fp = _tfopen(fileName, TEXT("at"));
	if(fp) {
		_ftprintf(fp, TEXT("%s\n"), str);		
		fclose(fp);
	}
}
//---------------------------------------------------------------------------
void secureUdpLayer_t::logToFile(TCHAR* fileName, TCHAR* fmt, ...)
{
	static TCHAR defaultFileName[] = TEXT("secureUdpLayer.log");
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
secureUdpLayer_t::secureUdpLayer_t(int randomSeed)
{	
	keyDistanceThreshold = DefaultKeyDistanceThreshold;	
	memset(keyRing, 0x0, SUL_MaxRing);
	memset(udpBuffer, 0x0, SUL_MaxUdpBuffer);
	init(randomSeed);

	LogToFile(0x0, TEXT("[secureUdpLayer_t::secureUdpLayer_t] started random seed:%d keyDistanceThreshold=%d"), randomSeed, keyDistanceThreshold);
}
//---------------------------------------------------------------------------
secureUdpLayer_t::~secureUdpLayer_t(void)
{
	deinit();
	LogToFile(0x0, TEXT("[secureUdpLayer_t::~secureUdpLayer_t] destroyed"));
}
//---------------------------------------------------------------------------