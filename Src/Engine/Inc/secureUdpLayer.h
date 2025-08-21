//
// secureUdpLayer.h
//
// deathcure@redduck.com
//---------------------------------------------------------------------------
#ifndef secureUdpLayerH
#define secureUdpLayerH
//---------------------------------------------------------------------------
struct secureUdpLayer_t {
	struct rc4_t {
		struct key_t {      
			unsigned char state[256];       
			unsigned char x;        
			unsigned char y;
		};
		typedef struct key_t key_t;
		typedef struct key_t* pkey_t;

		static void swap_byte(unsigned char *a, unsigned char *b);
		static void prepare_key(unsigned char *key_data_ptr,int key_data_len, pkey_t key);
		static void rc4(unsigned char *buffer_ptr,int buffer_len,pkey_t key);	
	};
	typedef struct rc4_t rc4_t;
	typedef struct rc4_t* prc4_t;

	// packet def
	// index of key ring : 8 bit;
	// checksum of plain text (or inverse of index of key ring) : 8 bit;
	// cipher text

#define SUL_MaxRing 256
#define SUL_MaxUdpBuffer 65536
#define SUL_KeyLength 8
#define SUL_DefaultRandomSeed 5305
#define SUL_KeyHistoryLength 8

	struct peerContext_t {
		QWORD addr;

		int idxKeyHistory;
		unsigned char keyHistory[SUL_KeyHistoryLength];
		unsigned char idxKeyRingRecv;
		unsigned char idxKeyRingSend;

		// {{ 모니터링을 위한 통계
		int nChecksumFailed;
		int nPacketReplayed;
		int nUnexpectedKeyIndex;
		// }} 모니터링을 위한 통계

		bool init(void);
		peerContext_t(QWORD idAddr);
	};
	typedef struct peerContext_t peerContext_t;
	typedef struct peerContext_t* ppeerContext_t;

	TMap<QWORD, peerContext_t> peerContexts;

	static QWORD makePeerContextHandle(SOCKADDR_IN addr);	
	
	unsigned char keyRing[SUL_MaxRing];
	unsigned char udpBuffer[SUL_MaxUdpBuffer];
	
	unsigned char keyDistanceThreshold;

	unsigned char keyDistance(unsigned char k1, unsigned char k2);
	void makeKey(unsigned char key[SUL_KeyLength], unsigned char idxKeyRing);

	static unsigned char calcChecksum(unsigned char* pPacket, int len);

	unsigned char* onRecvPacket(int& lengthPlainText, unsigned char* pPacket, int len, SOCKADDR_IN addr);
	unsigned char* onSendPacket(int& lengthCipherText, unsigned char* pPacket, int len, SOCKADDR_IN addr);	

	void generateKeyRing(unsigned int randomSeed);

	static void printDump(unsigned char* str, int len);
	static float frand(void);

	bool initPeer(SOCKADDR_IN addr);

	bool init(int randomSeed = SUL_DefaultRandomSeed);
	void deinit(void);

	static void _log(TCHAR* fileName, TCHAR* str);
	static void logToFile(TCHAR* fileName, TCHAR* fmt, ...);

	secureUdpLayer_t(int randomSeed = SUL_DefaultRandomSeed);
	~secureUdpLayer_t(void);
};
typedef struct secureUdpLayer_t secureUdpLayer_t;
typedef struct secureUdpLayer_t* psecureUdpLayer_t;
//---------------------------------------------------------------------------
#endif