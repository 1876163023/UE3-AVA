#ifndef _SVCCOMMONS_H_
#define _SVCCOMMONS_H_

#pragma pack(push,1)

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#endif

#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }

//
typedef DWORD	MSGID;
typedef DWORD	USERKEY;
typedef int		CONNECTID;
typedef int IDXROOM;
typedef int IDXUSER;
typedef int IDXCHANNEL;
typedef	int	ECC;
typedef int PACKETLENGTH;
// packets
#define EnablePacketHeading
#ifdef EnablePacketHeading
#define PacketHeading "qVDs" // 20060915 dEAthcURe
#define SizeOfHeading	4
#endif

struct PACKET_HEADER
{
	#ifdef EnablePacketHeading
	// {{ 20060915 dEAthcURe 수신계층 구분을 위한 heading 추가	
	char heading[4];
	// }} 20060915 dEAthcURe 수신계층 구분을 위한 heading 추가
	#endif

	PACKETLENGTH	Length;
	CONNECTID	ConnectID;
	MSGID		MsgID;
};
struct PACKET_GENERIC
{
	PACKET_HEADER	header;
	
	BYTE*	SizeField() { return (BYTE *)(&(header.Length)); }
	BYTE*	PayloadField() { return (BYTE *)(&(header.ConnectID)); }
	PACKETLENGTH Length()
	{
		return header.Length;
	}
	MSGID MsgID()
	{
		return header.MsgID;
	}
	CONNECTID ConnectID()
	{
		return header.ConnectID;
	}
	void Init(CONNECTID ConnectID,MSGID MsgID)
	{
		header.MsgID = MsgID;
		header.ConnectID = ConnectID;
	}
	void Complete(DWORD dwLength)
	{
		#ifdef EnablePacketHeading
		memcpy(header.heading, PacketHeading, 4); // 20060915 dEAthcURe 수신계층 구분을 위한 heading 추가
		#endif
		header.Length = dwLength;
	}
};

class CINT64
{
public:
	CINT64(CINT64& a)
	{
		m_nValue1 = a.m_nValue1;
		m_nValue2 = a.m_nValue2;
	}
	
	CINT64() 
	{
		m_nValue1 = 0L;
		m_nValue2 = 0L;
	}
	void SetValue(__int64 nValue, int nEncryptionKey, int nPassword)
	{
		int	nChecksum;
		nChecksum = (int)((nValue * nEncryptionKey)%nPassword);
		Encrypt(nValue, nChecksum);
	}
	__int64 GetValue(int nEncryptionKey, int nPassword)
	{
		__int64	nValue;
		int		nChecksum;
		Decrypt(&nValue, &nChecksum);
		if ( ((nValue*nEncryptionKey)%nPassword) != nChecksum ) {
			return -1;
		}
		return nValue;
	}
	__int64	AddValue(__int64 nDiff, int nEncryptionKey, int nPassword)
	{
		__int64	nValue;
		int		nChecksum;
		Decrypt(&nValue, &nChecksum);
		if ( ((nValue*nEncryptionKey)%nPassword) != nChecksum ) {
			return -1;
		}
		nValue += nDiff;
		nChecksum = (int)((nValue * nEncryptionKey)%nPassword);
		Encrypt(nValue, nChecksum);
		return nValue;
	}
	void	Encrypt(__int64 nValue, int nChecksum)
	{
		int		i;
		__int64	n64Value = 0L;

		for ( i = 0; i < 32; i++) {
			n64Value = (n64Value<<2);
			if (nValue % 2) {
				n64Value |= 1;
			}
			if (nChecksum % 2) {
				n64Value |= 2;
			}
			nValue = (nValue >> 1);
			nChecksum = (nChecksum >>1);
		}
		m_nValue1 = n64Value;
		n64Value = 0L;
		for ( i = 0; i < 32; i++) {
			n64Value = (n64Value<<2);
			if (nValue % 2) {
				n64Value |= 1;
			}
			nValue = (nValue >> 1);
		}
		m_nValue2 = n64Value;
	}
	void	Decrypt(__int64 *pN64Value, int* pChecksum)
	{
		__int64	n64Value;
		int		i;
		__int64	nValue = 0;
		int		nCheckSum = 0;

		n64Value = m_nValue2;
		for (i = 0; i < 32; i++) {
			nValue = (nValue << 1);
			if (n64Value % 2) {
				nValue |= 1;
			}
			n64Value = (n64Value>>2);
		}
		n64Value = m_nValue1;
		for ( i = 0; i < 32; i++) {
			nValue = (nValue << 1);
			nCheckSum = (nCheckSum << 1);
			if (n64Value % 2) {
				nValue |= 1;
			}
			n64Value = (n64Value>>1);
			if (n64Value % 2) {
				nCheckSum |= 1;
			}
			n64Value = (n64Value>>1);
		}
		(*pN64Value) = nValue;
		(*pChecksum) = nCheckSum;
	}
	CINT64 operator=(CINT64& value)
	{
		
		m_nValue1 = value.m_nValue1;
		m_nValue2 = value.m_nValue2;
		return *this;
	}
private:
	__int64	m_nValue1;
	__int64	m_nValue2;
};
#pragma pack(pop)
#endif
