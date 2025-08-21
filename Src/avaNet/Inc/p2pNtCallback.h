//
// p2pNtCallback.h
//
// deathcure@redduck.com
//---------------------------------------------------------------------------
#ifndef p2pNtCallbackH
#define p2pNtCallbackH
//---------------------------------------------------------------------------
struct Ip2pNtCallback_t {
	virtual bool onPacketComplete(int type, char* payload, int lenPayload)=0;
};
typedef struct Ip2pNtCallback_t Ip2pNtCallback_t;
typedef struct Ip2pNtCallback_t* pIp2pNtCallback_t;
//---------------------------------------------------------------------------
#endif
