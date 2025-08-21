/******************************************************************************

Ping.cpp:
---------

Autor:             c-worker.ch

Quellen:           Winsock Doku, MSDN, rfc791, rfc792,
                   http://www.iig.uni-freiburg.de/~mhartman/tcpip/icmp.html,
                   ..

Beschreibung:      Benutzt das ICMP Protokoll um die Erreichbarkeit eines Hosts zu prufen



Allgemeines zu ICMP:
--------------------

Echo Request, Echo Reply
------------------------

Diese Meldungen dienen zur Uberprufung ob ein Ziel erreichbar und
aktiv ist. Der Sender des Echo Request wartet auf eine Echo Reply
Message, die er nur erhalt, wenn der Zielrechner aktiv ist. Als ICMP
Typ wurde die Nummer 8 fur den Request und 0 fur den Echo Reply
definiert. Der Code ist in beiden Fallen auf 0 gesetzt. Au©¬erdem ist ein
ICMP Echo Identifikator definiert, welcher vom Sender des
Datagramms erzeugt wird und zur Identifikation des Prozesses dient.
Der Empfanger schickt den Reply an diesen Port. Eine Echo Sequenz
Nummer wird zur fortlaufenden Numerierung des Datagramms
genutzt. Der Empfanger nutzt die Nummer bei der Antwort und
ermoglicht dem Sender des Echo Request die Uberprufung der
Richtigkeit der Antwort. Die ICMP Echo Daten enthalten einen Echo
String, der vom Empfanger an den Sender zuruckgeschickt wird.


Destination Unreachable
-----------------------

Diese Nachricht wird an den Sender des Datagramms geschickt,
wenn ein Subnetzwerk oder Router den Zielrechner nicht erreichen
kann oder das Paket nicht ausgeliefert werden kann weil das Don¢¥t
Fragment Bit gesetzt ist und das Paket fur ein Netzwerk zu gro©¬ ist.
Die Nachricht wird vom Router generiert sofern es sich um ein nicht
erreichbares Netzwerk oder einen unerreichbaren Zielrechner handelt.
Sollte es sich jedoch um einen unerreichbaren Port handeln so schickt
diese Meldung der Zielrechner.

ICMP Typ

Zur Unterscheidung der einzelnen ICMP Meldungen wurden ICMP
Nummern definiert. Die Destination Unreachable Meldung hat die
Nummer 3.

ICMP Code

Der ICMP Code teilt dem Sender mit, weshalb sein Datagramm nicht
ubermittelt werden konnte. Es sind die folgenden Destination
Unreachable Codes definiert:

0= Netz nicht erreichbar
1= Rechner nicht erreichbar
2= Protokoll nicht erreichbar
3= Port nicht erreichbar
4= Fragmentierung notwendig, jedoch nicht moglich wegen gesetztem DF Bit
5= Source Route nicht erreichbar

******************************************************************************/

#include "avaNet.h"

//#include <windows.h>
#include <winsock2.h>
#include <Mmsystem.h>
#include <stdlib.h>
#include <iostream>
using namespace std;


#define ICMP_ECHOREPLY                 0
#define ICMP_UNREACHABLE               3
#define ICMP_ECHO                      8

// 0 = Netz nicht erreichbar
#define ICMP_CODE_NETWORK_UNREACHABLE  0
// 1 = Rechner nicht erreichbar
#define ICMP_CODE_HOST_UNREACHABLE     1

// Minimalgrosse 8 Byte
#define ICMP_MIN            8

#define STATUS_FAILED       0xFFFF
#define DEF_PACKET_SIZE     32
#define MAX_PACKET          65000

/*
Der IP Header:
--------------

|  Byte 0       |   Byte 1      |  Byte 2       |  Byte 3       |

0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Version|  IHL  |Type of Service|          Total Length         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|         Identification        |Flags|      Fragment Offset    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  Time to Live |    Protocol   |         Header Checksum       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                       Source Address                          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                    Destination Address                        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

struct IP_HEADER {
	unsigned int   h_len:4;          // Lange des Headers
	unsigned int   version:4;        // IP Version
	unsigned char  tos;              // Type of service
	unsigned short total_len;        // Gesamt lange des Pakets
	unsigned short ident;            // unique identifier
	unsigned short frag_and_flags;   // flags
	unsigned char  ttl;              // TTL
	unsigned char  proto;            // Protokoll (TCP, UDP etc)
	unsigned short checksum;         // IP Checksumme
	unsigned int   sourceIP;         // Source IP
	unsigned int   destIP;           // Ziel IP
};


/*
Der ICMP Header:
----------------

|  Byte 0       |   Byte 1      |  Byte 2       |  Byte 3       |

0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| Type          | Code          |     ICMP Header Prufsumme     |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Identifikatior |Sequenz Nummer |                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                      Timestamp                                |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

*/

struct ICMP_HEADER {
	char           i_type;    // Type
	char           i_code;    // Code
	unsigned short i_cksum;   // Prufsumme
	unsigned short i_id;      // Identifikatior
	unsigned short i_seq;     // Sequenz Nummer
	unsigned long  timestamp; // Um die benotigte Zeit zu messen
};

long WinsockStartup();
void FillIcmpData(char* icmp_data, int datasize);
unsigned short checksum(unsigned short *buffer, int size);
bool DecodeResponse(char *buf, int bytes, SOCKADDR_IN *from, float* pRtt = 0x0);
bool ping(const char* ipAddr, float* pAvgRtt, int nTrial = 3, int dataSize = DEF_PACKET_SIZE, bool bStartupWinsock = true);
//---------------------------------------------------------------------------
/*
Die Antwort die wir empfangen ist ein IP Paket. Wir mussen nun den IP
Header decodieren um die ICMP Daten lesen zu konnen
*/

// return
//	bool bSuccess
//static DWORD tickStart = 0; // test 
//static DWORD tickEnd =0; // test
bool DecodeResponse(char *buf, int bytes, SOCKADDR_IN *from, float* pRtt)
{
	IP_HEADER   *IpHeader;
	ICMP_HEADER *IcmpHeader;
	unsigned short IpHeaderLen;

	IpHeader = (IP_HEADER*)buf;
	IpHeaderLen = IpHeader->h_len * 4 ; // number of 32-bit words *4 = bytes

	if (bytes  < IpHeaderLen + ICMP_MIN) {
		//cout << "Too few bytes from " << inet_ntoa(from->sin_addr) << endl;
	}
	IcmpHeader = (ICMP_HEADER*)(buf + IpHeaderLen);

	if (IcmpHeader->i_type != ICMP_ECHOREPLY) {
		if (IcmpHeader->i_type == ICMP_UNREACHABLE) {
			//cout << "Reply from " << inet_ntoa(from->sin_addr);
			if(IcmpHeader->i_code == ICMP_CODE_HOST_UNREACHABLE) {
				//cout << ": Destination Host unreachable !" << endl;
				return false;
			}
			if(IcmpHeader->i_code == ICMP_CODE_NETWORK_UNREACHABLE) {
				//cout << ": Destination Network unreachable !" << endl;
				return false;
			}
		}
		else {
			//cout << "non-echo type " << (int)IcmpHeader->i_type <<" received" << endl;
			return false;
		}
	}

	if (IcmpHeader->i_id != (unsigned short)GetCurrentProcessId()) {
		//cout << "someone else's packet!" << endl;
		return false;
	}

	DWORD curTick = timeGetTime(); // GetTickCount();
	//cout << bytes << " bytes from " << inet_ntoa(from->sin_addr);
	//cout << " icmp_seq = " << IcmpHeader->i_seq;
	//cout << " time: " << curTick-IcmpHeader->timestamp << " ms " << endl;	
	if(pRtt) *pRtt = (float)(curTick-IcmpHeader->timestamp)/1000.0f;
	return true;
}
//---------------------------------------------------------------------------
unsigned short checksum(unsigned short *buffer, int size)
{
	unsigned long cksum=0;
	while(size >1) {
		cksum+=*buffer++;
		size -=sizeof(unsigned short);
	}

	if(size) {
		cksum += *(unsigned char*)buffer;
	}

	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >>16);
	return (unsigned short)(~cksum);
}
//---------------------------------------------------------------------------
/*
Hilfs Funktion um unseren ICMP Header zu fullen
*/
void FillIcmpData(char * icmp_data, int datasize)
{
	ICMP_HEADER *icmp_hdr;
	char *datapart;

	icmp_hdr = (ICMP_HEADER*)icmp_data;

	icmp_hdr->i_type = ICMP_ECHO;
	icmp_hdr->i_code = 0;
	icmp_hdr->i_id = (unsigned short)GetCurrentProcessId();
	icmp_hdr->i_cksum = 0;
	icmp_hdr->i_seq = 0;

	datapart = icmp_data + sizeof(ICMP_HEADER);
	// Den Buffer mit etwas fullen
	memset(datapart,'C', datasize - sizeof(ICMP_HEADER));
}
//---------------------------------------------------------------------------
long WinsockStartup()
{
	long rc;

	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(2, 1);

	rc = WSAStartup( wVersionRequested, &wsaData );
	return rc;
}
//---------------------------------------------------------------------------
// return
//	bool bSuccess
static char icmp_data[MAX_PACKET] = {0,};
static char recvbuf[MAX_PACKET+sizeof(IP_HEADER)+sizeof(ICMP_HEADER)] = {0,};
bool ping(const char* ipAddr, float* pAvgRtt, int nTrial, int dataSize, bool bStartupWinsock)
{
	#define PingSucceeded \
		bSuccess = true; goto end_of_ping;

	#define PingFailed \
		bSuccess = false; goto end_of_ping;

	SOCKET sockRaw;
	SOCKADDR_IN addrDest;
	SOCKADDR_IN addrFrom;
	int addrFromLen = sizeof(addrFrom);
	HOSTENT* ptrHostent;	
	int RecvTimeout = 1000;
	char *dest_ip;	
	unsigned int addr=0;
	unsigned short seq_no = 0;
	int BytesReceived;
	int BytesSent;

	float rttSum = 0.0f;
	int nValidTrial = 0;

	long rc;
	bool bSuccess = false;

	if(bStartupWinsock) {
		rc = WinsockStartup();
		if (rc == SOCKET_ERROR) {
			//cout << "Error: WinsockStartup failed: " << WSAGetLastError() << endl;
			PingFailed; // return SOCKET_ERROR;
		}
	}

	sockRaw = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockRaw == INVALID_SOCKET) {
		//cout << "Error: Cannot create Socket: " << WSAGetLastError() << endl;
		PingFailed; // return SOCKET_ERROR;
	}
	rc = setsockopt(sockRaw,SOL_SOCKET,SO_RCVTIMEO,(char*)&RecvTimeout, sizeof(RecvTimeout));
	if(rc == SOCKET_ERROR) {
		//cout << "Error: Cannot set Recv Timeout: " << WSAGetLastError() << endl;
		PingFailed; // return SOCKET_ERROR;
	}

	ptrHostent = gethostbyname(ipAddr);
	if (!ptrHostent) {
		addr = inet_addr(ipAddr);
	}

	if ((!ptrHostent)  && (addr == INADDR_NONE) ) {
		//cout << "Error: Cannot resolve Host: " << WSAGetLastError() << endl;
		PingFailed; // return SOCKET_ERROR;
	}
	if (ptrHostent != NULL) {
		memcpy(&(addrDest.sin_addr),ptrHostent->h_addr, ptrHostent->h_length);
	}
	else {
		addrDest.sin_addr.s_addr = addr;
	}
	if (ptrHostent) {
		addrDest.sin_family = ptrHostent->h_addrtype;
	}
	else {
		addrDest.sin_family = AF_INET;
	}

	// Konvertiert eine Netzwerk Adresse (SOCKADDR_IN) in einen String im Punkt Format (x.x.x.x)
	dest_ip = inet_ntoa(addrDest.sin_addr);
	
	if (dataSize == 0) {
		dataSize = DEF_PACKET_SIZE;
	}
	if (dataSize > MAX_PACKET) {
		//cout << "Error: Data size more then " << MAX_PACKET << " Bytes or less then 0 Bytes" << endl;
		//cout << "I'll take the default size..." << endl;
		dataSize = DEF_PACKET_SIZE;
	}	
	dataSize += sizeof(ICMP_HEADER);

	//icmp_data = (char*)malloc(MAX_PACKET);
	//recvbuf = (char*)malloc(MAX_PACKET+sizeof(IP_HEADER)+sizeof(ICMP_HEADER));
	//if (!icmp_data || !recvbuf) {
		//cout << "Error: Not enough Memory: " << GetLastError() << endl;
		//PingFailed; // return 0;
	//}

	FillIcmpData(icmp_data, dataSize);

	for(int lpp=0;lpp<nTrial;lpp++) {	
		((ICMP_HEADER*)icmp_data)->i_cksum = 0;		
		((ICMP_HEADER*)icmp_data)->timestamp = timeGetTime(); // GetTickCount();

		((ICMP_HEADER*)icmp_data)->i_seq = seq_no++;
		((ICMP_HEADER*)icmp_data)->i_cksum = checksum((unsigned short*)icmp_data, dataSize);

		BytesSent = sendto(sockRaw,icmp_data,dataSize,0,(SOCKADDR*)&addrDest, sizeof(addrDest));
		if (BytesSent == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT) {
				//cout << "timed out\n" << endl;
				PingFailed; // goto end_of_ping:
			}
			//cout << "sendto failed: " << WSAGetLastError() << endl;
			PingFailed; // goto end_of_ping:
		}

		if (BytesSent < dataSize ) {
			//cout <<"Wrote " << BytesSent << " bytes" << endl;
		}

		BytesReceived = recvfrom(sockRaw,recvbuf,MAX_PACKET+sizeof(IP_HEADER)+sizeof(ICMP_HEADER),0,(SOCKADDR*)&addrFrom, &addrFromLen);
		if (BytesReceived == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT) {
				//cout << "timed out" << endl;
				PingFailed; // goto end_of_ping:
			}
			//cout << "recvfrom failed: " << WSAGetLastError() << endl;
			PingFailed; // goto end_of_ping:
		}
		float rtt = 0.0f;
		if(DecodeResponse(recvbuf, BytesReceived, &addrFrom, &rtt)) { // if success
			rttSum += rtt;
			nValidTrial++;			
		}
		//Sleep(1000);
	}	
	PingSucceeded;

end_of_ping:
	if(bStartupWinsock) {
		WSACleanup();
	}

	if(bSuccess) {
		*pAvgRtt = (rttSum / nValidTrial);
	}
	return bSuccess;
}
//---------------------------------------------------------------------------
#ifdef _CONSOLE
//---------------------------------------------------------------------------
#ifdef __Original
int main(int argc, char** argv)
{
	SOCKET sockRaw;
	SOCKADDR_IN addrDest;
	SOCKADDR_IN addrFrom;
	int addrFromLen = sizeof(addrFrom);
	HOSTENT* ptrHostent;
	unsigned long datasize;
	int RecvTimeout = 1000;
	char *dest_ip;
	char *icmp_data;
	char *recvbuf;
	unsigned int addr=0;
	unsigned short seq_no = 0;
	int BytesReceived;
	int BytesSent;
	long rc;

	if(argc < 2) {
		cout << "Error: To few Arguments " << endl;
		cout << "\nUsage: ping <host> [Bytes to send]" << endl;
		return 0;
	}

	rc = WinsockStartup();
	if (rc == SOCKET_ERROR) {
		cout << "Error: WinsockStartup failed: " << WSAGetLastError() << endl;
		return SOCKET_ERROR;
	}
	sockRaw = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockRaw == INVALID_SOCKET) {
		cout << "Error: Cannot create Socket: " << WSAGetLastError() << endl;
		return SOCKET_ERROR;
	}
	rc = setsockopt(sockRaw,SOL_SOCKET,SO_RCVTIMEO,(char*)&RecvTimeout, sizeof(RecvTimeout));
	if(rc == SOCKET_ERROR) {
		cout << "Error: Cannot set Recv Timeout: " << WSAGetLastError() << endl;
		return SOCKET_ERROR;
	}

	ptrHostent = gethostbyname(argv[1]);
	if (!ptrHostent) {
		addr = inet_addr(argv[1]);
	}

	if ((!ptrHostent)  && (addr == INADDR_NONE) ) {
		cout << "Error: Cannot resolve Host: " << WSAGetLastError() << endl;
		return SOCKET_ERROR;
	}
	if (ptrHostent != NULL) {
		memcpy(&(addrDest.sin_addr),ptrHostent->h_addr, ptrHostent->h_length);
	}
	else {
		addrDest.sin_addr.s_addr = addr;
	}
	if (ptrHostent) {
		addrDest.sin_family = ptrHostent->h_addrtype;
	}
	else {
		addrDest.sin_family = AF_INET;
	}

	// Konvertiert eine Netzwerk Adresse (SOCKADDR_IN) in einen String im Punkt Format (x.x.x.x)
	dest_ip = inet_ntoa(addrDest.sin_addr);

	if (argc >2) {
		datasize = atoi(argv[2]);
		if (datasize == 0) {
			datasize = DEF_PACKET_SIZE;
		}
		if (datasize > MAX_PACKET) {
			cout << "Error: Data size more then " << MAX_PACKET << " Bytes or less then 0 Bytes" << endl;
			cout << "I'll take the default size..." << endl;
			datasize = DEF_PACKET_SIZE;
		}
	}
	else {
		datasize = DEF_PACKET_SIZE;
	}
	datasize += sizeof(ICMP_HEADER);

	icmp_data = (char*)malloc(MAX_PACKET);
	recvbuf = (char*)malloc(MAX_PACKET+sizeof(IP_HEADER)+sizeof(ICMP_HEADER));

	if (!icmp_data || !recvbuf) {
		cout << "Error: Not enough Memory: " << GetLastError() << endl;
		return 0;
	}

	FillIcmpData(icmp_data,datasize);

	while(1) {
		((ICMP_HEADER*)icmp_data)->i_cksum = 0;
		((ICMP_HEADER*)icmp_data)->timestamp = GetTickCount();

		((ICMP_HEADER*)icmp_data)->i_seq = seq_no++;
		((ICMP_HEADER*)icmp_data)->i_cksum = checksum((unsigned short*)icmp_data, datasize);

		BytesSent = sendto(sockRaw,icmp_data,datasize,0,(SOCKADDR*)&addrDest, sizeof(addrDest));
		if (BytesSent == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT) {
				cout << "timed out\n" << endl;
				continue;
			}
			cout << "sendto failed: " << WSAGetLastError() << endl;
			return 0;
		}

		if (BytesSent < datasize ) {
			cout <<"Wrote " << BytesSent << " bytes" << endl;
		}

		BytesReceived = recvfrom(sockRaw,recvbuf,MAX_PACKET+sizeof(IP_HEADER)+sizeof(ICMP_HEADER),0,(SOCKADDR*)&addrFrom, &addrFromLen);

		if (BytesReceived == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT) {
				cout << "timed out" << endl;
				continue;
			}
			cout << "recvfrom failed: " << WSAGetLastError() << endl;
			return 0;
		}
		DecodeResponse(recvbuf,BytesReceived,&addrFrom);
		Sleep(1000);
	}
}
#else
#ifdef standalone
int main(int argc, char** argv)
{
	if(argc < 2) {
		cout << "Error: To few Arguments " << endl;
		cout << "\nUsage: ping <host> [Bytes to send] [nTrial]" << endl;
		return 0;
	}

	int dataSize = 32;
	if(argc>=3) {
		dataSize = atoi(argv[2]);
	}

	int nTrial = 3;
	if(argc>=4) {
		nTrial = atoi(argv[3]);
	}

	float avgRtt = 0.0f;
	if(ping(argv[1], &avgRtt, nTrial, dataSize)) {
		cout << "ping succeeded. avg rtt = " << avgRtt << endl;
	}
	else {
		cout << "ping failed." << endl;			
	}
}
#endif
#endif
//---------------------------------------------------------------------------
#endif
