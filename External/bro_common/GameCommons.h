#ifndef _GAMECOMMONS_H_
#define _GAMECOMMONS_H_
#include <winsock2.h>
#include "svccommons.h"	//�ٸ� Project���� ������ StdAfx�� include�Ȱ͵��� �ݿ��� �ȵǹǷ� ���� include�� �־��ش�.

#pragma pack(push,1)

#define	JOIN_PROHIBITED			(0x00000001)	

//common defines
#define MAX_USERID			(32)	//DB Table�� field size���� ������
#define MAX_PASSWORD		(12)	//DB Table�� field size���� ������
#define MAX_NICK_NAME		(16)
//���̻� �����ϱ�� ��#define MAX_EMAIL			(48)	//DB Table�� field size���� ������
#define MAX_USER_NAME		(20)	//DB Table�� field size���� ������
#define MAX_ROOM_TITLE		(32)

#define MAX_USERS_IN_ROOM	(8)
///////////////////////////////////////////
// ����� ����
#define STATE_NOT_IN_SERVER	(0)
#define STATE_IN_SERVER		(1)
#define	STATE_IN_CHANNEL	(2)
#define	STATE_IN_WAIT_ROOM	(4)
#define	STATE_IN_ROOM		(8)
////////////////////////////////////////////

////////////////////////////////////////////
// �� Type
#define ROOM_TYPE_SINGLE_PLAY	(0)
#define ROOM_TYPE_TEAM_PLAY		(1)
#define ROOM_TYPE_ALL_PLAY		(2)
//#define	ROOM_TYPE_ITEM			4
//#define ROOM_TYPE_NOTEM			8
////////////////////////////////////////////

typedef	int	GSN;
struct STRUCT_ROOM_GENERIC
{
	IDXROOM m_idxRoomInChannel;
	bool	m_bSecret;
	int		m_nType;
	bool	m_bGaming;
	int		m_nMaxUser;
	int		m_nCurrentUser;
	char	m_strTitle[MAX_ROOM_TITLE];
	char	m_strPassword[MAX_PASSWORD];
	IDXUSER	m_OwnerUser;
};

struct STRUCT_ROOM_INFO_GENERIC
{
	int			m_nRoomNumber;
	int			m_bSecret;
	int			m_nType;
	int			m_bGaming;				//4.17 tkhong ���� ���� ���������� �������������� ǥ���Ѵ�.
	int			m_nMaxUser;
	int			m_nCurrentUser;
	char		m_strTitle[MAX_ROOM_TITLE];
};

struct STRUCT_USER_INFO_GENERIC
{
	int			m_nKey;
	char		m_strUserID[MAX_USERID];
	char		m_strUserName[MAX_USER_NAME];
	void Init(CONNECTID connectid)
	{
		m_nKey = connectid;
		m_strUserID[0] = '\0';m_strUserName[0] = '\0';
	}
};

/********************************
struct STRUCT_USER_GENERIC
{
	CONNECTID	m_ConnectID;
	//tkhong 2004.4.20	USERKEY		m_UserKey;
	char		m_strUserID[MAX_USERID];
	char		m_strUserName[MAX_USER_NAME];
	__int64		m_nMoney;	//���ݿ� �����Ǵ� ���̹� �Ӵ�, �޴��� ���� ���� �̿��Ͽ� �Ա��ϸ� �̱ݾ��� �þ��.
	void Init(CONNECTID connectid)
	{
		m_strUserID[0] = '\0';m_strUserName[0] = '\0';
	}
};
*********************************/
/* 2002.2.12 tkhong
struct STRUCT_PART_DETAIL
{
	int	m_idxUserInRoom;
	STRUCT_USER_GENERIC m_UserGeneric;
};

*/

////////////////////////////////////////////////
// ��� ���ӿ� �������� ����� error code
#define REASON_NO_ERROR				(0)

#define REASON_CHANNEL_FULL			(1)
#define REASON_CHANNEL_ERROR		(3)

#define REASON_INVALID_CHANNEL		(4)

#define REASON_ROOM_INSUFFICIENT	(5)
#define REASON_CANNOT_JOIN			(6)

#define REASON_INVALID_PASSWORD		(7)
#define REASON_ROOM_FULL			(8)
#define REASON_INVALID_ROOM			(9)
#define REASON_INVALID_USER			(10)
#define REASON_DUPLICATE_USER		(11)
#define REASON_DUPLICATE_NICK		(16)
#define REASON_DUPLICATE_KEY		(14)
#define REASON_BLOCKID				(17)
#define REASON_BLOCKIP				(18)
#define REASON_ETC					(15)

#define REASON_EMPTY_ROOM			(12)
#define REASON_EXIT_OWNER			(13)

#define REASON_GAME_BEGIN			(20)
////////////////////////////////////////////////

#define MSGID_BEGIN	(100)
//client -> server


#define MSGID_LOGIN				(MSGID_BEGIN	+	1)
#define MSGID_SEND_TEXT			(MSGID_BEGIN	+	2)
#define MSGID_REQ_WAIT_LIST		(MSGID_BEGIN	+	3)
#define MSGID_REQ_ROOM_LIST		(MSGID_BEGIN	+	4)
#define MSGID_REQ_PART_BRIEF	(MSGID_BEGIN	+	5)
#define MSGID_REQ_PART_DETAIL	(MSGID_BEGIN	+	6)
#define MSGID_LOGIN_GAME		(MSGID_BEGIN	+	7)
#define MSGID_JOIN_ROOM			(MSGID_BEGIN	+	8)
#define MSGID_HEART_BEAT		(MSGID_BEGIN	+	9)
#define MSGID_EXIT_ROOM			(MSGID_BEGIN	+	10)
#define MSGID_EXIT_GAME			(MSGID_BEGIN	+	11)
#define MSGID_CREATE_ROOM		(MSGID_BEGIN	+	12)
#define MSGID_CHANGE_OWNER		(MSGID_BEGIN	+	13)
#define MSGID_CHANGE_CHANNEL	(MSGID_BEGIN	+	14)
#define MSGID_REQ_CHANNEL_INFO	(MSGID_BEGIN	+	15)
#define MSGID_REQ_PROFILE		(MSGID_BEGIN	+	16)
#define MSGID_REQ_NEW_USER		(MSGID_BEGIN	+	17)

#define MSGID_REPORT_POSITION	(MSGID_BEGIN	+	53)
#define MSGID_QUERY_CHANNEL		(MSGID_BEGIN	+	54)
#define MSGID_ADM_CHANNEL_INFO	(MSGID_BEGIN	+	56)
#define MSGID_EXECUTE_GAME		(MSGID_BEGIN	+	57)
#define MSGID_INSERT_TCONNECT	(MSGID_BEGIN	+	58)

#define	MSGID_CHANGE_GROUP		(MSGID_BEGIN	+	60)		//�ʺ� �߼� ��� ������ �����ؼ� �˷��ָ� �ڵ����� ä���� ���ؼ� ä�� ������ ���ش�
															//���� packet�� MSGID_CHANGE_CHANNEL_OK�� ���� ����Ѵ�.
#define	MSGID_REQ_INVITE		(MSGID_BEGIN	+	61)		//�ʴ�
#define	MSGID_DENY_INVITE		(MSGID_BEGIN	+	63)		//�ʴ� �ź� ��ư�� �������� ������ �˸�
//server -> client

#define	MSGID_NOT_INVITE			(MSGID_BEGIN	+	62)	//�ٸ�������κ��� �ʴ밡 ������ �˸�

#define MSGID_QUERY_CHANNEL_OK		(MSGID_BEGIN	+	55)
#define MSGID_INSERT_TCONNECT_OK	(MSGID_BEGIN	+	59)

#define MSGID_LOGIN_OK				(MSGID_BEGIN	+	18)
#define MSGID_LOGIN_FAIL			(MSGID_BEGIN	+	19)
#define MSGID_LOGIN_GAME_OK			(MSGID_BEGIN	+	20)
#define MSGID_JOIN_ROOM_OK			(MSGID_BEGIN	+	21)
#define MSGID_CHANGE_CHANNEL_OK		(MSGID_BEGIN	+	22)
#define MSGID_NOT_WAIT_LIST			(MSGID_BEGIN	+	23)
#define MSGID_NOT_ROOM_LIST			(MSGID_BEGIN	+	24)
#define MSGID_CREATE_ROOM_OK		(MSGID_BEGIN	+	25)
#define MSGID_EXIT_ROOM_OK			(MSGID_BEGIN	+	26)
#define MSGID_CHANGE_OWNER_OK		(MSGID_BEGIN	+	27)
#define MSGID_NOT_CHANNEL_INFO		(MSGID_BEGIN	+	28)
#define MSGID_NOT_PART_BRIEF		(MSGID_BEGIN	+	29)
#define MSGID_NOT_PART_DETAIL		(MSGID_BEGIN	+	30)
#define MSGID_NOT_PROFILE_OK		(MSGID_BEGIN	+	31)
#define MSGID_CONNECT_OK			(MSGID_BEGIN	+	32)
#define MSGID_CHANNEL_UPDATE_WAIT	(MSGID_BEGIN	+	33)
#define MSGID_CHANNEL_UPDATE_ROOM	(MSGID_BEGIN	+	34)
#define MSGID_CREATE_ROOM_FAIL		(MSGID_BEGIN	+	35)
#define MSGID_LOGIN_GAME_FAIL		(MSGID_BEGIN	+	36)
#define MSGID_EXIT_ROOM_FAIL		(MSGID_BEGIN	+	37)
#define MSGID_CHANGE_CHANNEL_FAIL	(MSGID_BEGIN	+	38)
#define MSGID_JOIN_ROOM_FAIL		(MSGID_BEGIN	+	39)
#define MSGID_JOIN_ROOM_INFORM		(MSGID_BEGIN	+	40)
#define MSGID_EXIT_ROOM_INFORM		(MSGID_BEGIN	+	41)
#define MSGID_SERVER_IDENTIFY		(MSGID_BEGIN	+	42)
#define MSGID_NOT_PROFILE_FAIL		(MSGID_BEGIN	+	43)
#define MSGID_NOT_NEW_USER_FAIL		(MSGID_BEGIN	+	44)
#define MSGID_NOT_INVALID_ENC		(MSGID_BEGIN	+	45)
#define MSGID_EXIT_GAME_OK			(MSGID_BEGIN	+	46)
#define MSGID_NOT_ADD_USER			(MSGID_BEGIN	+	47)
#define MSGID_NOT_REMOVE_USER		(MSGID_BEGIN	+	48)
#define MSGID_NOT_ADD_ROOM			(MSGID_BEGIN	+	49)
#define MSGID_NOT_REMOVE_ROOM		(MSGID_BEGIN	+	50)
#define	MSGID_NOT_CHANGE_ROOM		(MSGID_BEGIN	+	51)
#define MSGID_INVALIDATE			(MSGID_BEGIN	+	52)
//battlenet relation
#define MSGID_JOIN_BATTLE_NET		(MSGID_BEGIN	+	47)
#define MSGID_LEAVE_BATTLE_NET		(MSGID_BEGIN	+	48)
#define MSGID_LEAVE_BATTLE_NET_OK	(MSGID_BEGIN	+	49)
////////////////////////////////////////////////////////////////
// Big Channel���� �߰� ���
#define MSGID_SCROLL				(MSGID_BEGIN	+	50)
#define	MSGID_CHG_SORT_METHOD		(MSGID_BEGIN	+	51)
// Big Channel���� �߰� ���
////////////////////////////////////////////////////////////////

#define MSGID_GAME_COMMAND_BEGIN	(MSGID_BEGIN	+	100)
#define MSGID_BATTLE_BEGIN				(MSGID_GAME_COMMAND_BEGIN)
#define MSGID_BATTLE_END				(MSGID_GAME_COMMAND_BEGIN+100)

#define MSGID_DB_BEGIN				(MSGID_GAME_COMMAND_BEGIN + 128)

#define	MSGID_P2P_RESERVED_BEGIN		(MSGID_GAME_COMMAND_BEGIN + 100)

enum	P2P_COMMAND {
	MSGID_P2P_REQ_RESPONSE = MSGID_P2P_RESERVED_BEGIN,
	MSGID_P2P_ACK_RESPONSE,
	MSGID_P2P_CONFIRM_RESPONSE,
	MSGID_P2P_REQ_PUNCHHOLE,
	MSGID_P2P_PUNCHHOLE_PROBE1,
	MSGID_P2P_PUNCH,
	MSGID_P2P_PUNCHHOLE_PROBE2,
	MSGID_P2P_HOLE_SCAN,
	MSGID_P2P_ACK_PUNCHHOLE,
	MSGID_P2P_HEARTBEAT,
	//MSGID_P2P_REQ_PUNCHHALL,
	//MSGID_P2P_ACK_PUNCHHALL,
	MSGID_P2P_REQ_RELAY,
	MSGID_P2P_ACK_RELAY,
	MSGID_P2P_REQ_CONNECT,	//Ŭ���̾�Ʈ ����� ���ÿ� ������ �����Ͽ� �������� �����ϱ����� �߰��� ���
	MSGID_P2P_ACK_CONNECT,	//Ŭ���̾�Ʈ ����� ���ÿ� ������ �����Ͽ� �������� �����ϱ����� �߰��� ���
	MSGID_P2P_REQ_PEER_ADDR,
	MSGID_P2P_PEER_ADDR_OK,
	MSGID_P2P_PEER_ADDR_FAIL,
	MSGID_P2P_EXPELED,
	MSGID_P2P_ACK_SEQ,
	MSGID_P2P_EXIT,
	MSGID_P2P_EXIT_ALL_PEER,
	MSGID_P2P_EXIT_OK,
	MSGID_P2P_EXIT_ALL_OK,
	MSGID_P2P_REMOVE_USER,
	MSGID_P2P_TIME_ADJUST,	//Time Sync�� ���� ��ɾ�
	MSGID_P2P_I_OPEND_DOOR,	// direct request ����
	MSGID_P2P_ACK_REQ_PUNCHHOLE,	// MSGID_P2P_REQ_PUNCHHOLE �� ����
	MSGID_P2P_REQ_RTTPROB, // 20070410 dEAthcURe
	MSGID_P2P_ACK_RTTPROB, // 20070410 dEAthcURe
	MSGID_P2P_RESERVED_END,
	MSGID_P2P_HeartbeatV2, // 20070129 dEAthcURe
};

//NOTE [2006-5-25 jovial] : 
// ���� �߰��� ���� ������ ���� �̿��Ѵ�. 
// (pPacket->m_nDestPeerKey > P2P_DEST_ALL)�� Ư�� peer�� ��Ī�ϴ��� �Ǵ��ϹǷ�
enum	P2P_DEST_TYPE {
	P2P_DEST_ALL = -1,
	P2P_DEST_NONE = -2,
};

//notifications
#define NOTIFY_DB_AUTH					(1)
#define NOTIFY_DB_PROFILE				(2)
#define NOTIFY_DB_NEW_USER				(3)
#define NOTIFY_DB_SELECT_POSITION		(4)
#define NOTIFY_DB_DELETE_POSITION		(5)
#define NOTIFY_DB_FLUSH_POSITION		(6)
#define NOTIFY_DB_FLUSH_BASIC_USERINFO	(7)
#define	NOTIFY_DB_NO_NEED_TO_NOTIFY		(8)
#define NOTIFY_DB_GAME_BEGIN			(16)

//packet defines
struct STRUCT_TREE {
	int		m_nChild;
	int		m_nUser;
};

struct PACKET_DENY_INVITE : public PACKET_GENERIC
{
	int		m_nHostUserId;
};
struct PACKET_INVITE : public PACKET_GENERIC
{
	char	m_strHostNick[MAX_NICK_NAME];
	char	m_strRoomPassword[MAX_PASSWORD];
	int		m_nHostRoomNumber;
	int		m_nTargetUserKey;
};

struct PACKET_ADM_CHANNEL_INFO : public PACKET_GENERIC
{
	STRUCT_TREE*	m_pRoot;
};

struct PACKET_PINUP : public PACKET_GENERIC
{
	int		m_nPinUpUserKey;
	int		m_nLockDuration;	//seconds
};

struct PACKET_REPORT_POSITION : public PACKET_GENERIC
{
	int		m_nGame;
	int		m_nServer;
	int		m_nChannel;
};

struct PACKET_QUERY_CHANNEL : public PACKET_GENERIC
{
	int		m_nGame;
	int		m_nGroup;	//�ʺ�,�߼�,���,����...
};

struct PACKET_QUERY_CHANNEL_OK : public PACKET_GENERIC
{
	int		m_nServer;
	int		m_nChannel;
	char	m_strAddress[24];
};

struct PACKET_EXECUTE_GAME : public PACKET_GENERIC
{
	int		m_nGroup;
	char	m_strPath[256];
	char	m_strFilename[256];
};

struct PACKET_REQ_NEW_USER : public PACKET_GENERIC
{
	char strUserID[MAX_USERID];
	char strPassword[MAX_PASSWORD];
//	char strEmail[MAX_EMAIL];
};

/////////////////////////////////////////////////////////
//�׽�Ʈ�� packet�̴�.
struct PACKET_INSERT_CONNECT_TABLE : public PACKET_GENERIC
{
	char	m_strUserID[MAX_USERID];
	int		m_nServerID;
};
/////////////////////////////////////////////////////////

struct PACKET_CONNECT_OK : public PACKET_GENERIC
{
	ECC	m_nEncryptionKey;
};

struct PACKET_SERVER_IDENTIFY:public PACKET_GENERIC
{
	char strDesc[256];
};

struct PACKET_LOGIN : public PACKET_GENERIC
{
	char	m_strUserID[MAX_USERID];
	char	m_strPassword[MAX_PASSWORD];
	bool	m_bForceLogin;
};

struct PACKET_LOGIN_OK : public PACKET_GENERIC
{
	int		m_nGSN;	
	char	m_strUserID[MAX_USERID];
	char	m_strPassword[MAX_PASSWORD];
	//char	m_strUsername[MAX_USER_NAME];
	//char	m_strUserMail[MAX_EMAIL];
	//CINT64	m_nMoney;
	BYTE	m_nSex;
	BYTE	m_nAge;
	//BYTE	m_nPenaltyType;
	//BYTE	m_nPenaltyReason;
	//BYTE	m_nPenaltyEndMonth;
	//BYTE	m_nPenaltyEndDay;
};

struct PACKET_LOGIN_GAME : public PACKET_GENERIC
{
	int		GetSex(){
		return ( (int)(m_nUserType&1) );
	}
	void	SetSex(int nSex)
	{
		m_nUserType &= 0xFE;
		m_nUserType |= nSex;
	}
	bool	IsAdmin() {
		return ( 2 == (m_nUserType&2) );
	}
	void	SetAdmin(bool bAdmin)
	{
		m_nUserType &= 0xFD;
		if (bAdmin) {
			m_nUserType |= 2;
		}
	}
	bool	IsNPC() {
		return ( 4 == (m_nUserType&4) );
	}
	void	SetNPC(bool bNPC)
	{
		m_nUserType &= 0xFB;
		if (bNPC) {
			m_nUserType |= 4;
		}
	}
	//STRUCT_USER_GENERIC m_UserGeneric;
	char	m_strUserID[MAX_USERID];
	char	m_strPassword[MAX_PASSWORD];
	char	m_strUsername[MAX_USER_NAME];
	//char	m_strUserMail[MAX_EMAIL];
	CINT64	m_nMoney;
	BYTE	m_nUserType;
	BYTE	m_nAge;
	BYTE	m_nPenaltyType;
	BYTE	m_nPenaltyReason;
	BYTE	m_nPenaltyEndMonth;
	BYTE	m_nPenaltyEndDay;
};

struct PACKET_CHANGE_CHANNEL : public PACKET_GENERIC
{
	IDXCHANNEL	m_idxNewChannel;
};

//
struct PACKET_NOTIFY : public PACKET_GENERIC
{
	int nReason;
};

struct PACKET_LOGIN_GAME_OK: public PACKET_GENERIC
{
};

struct PACKET_SEND_TEXT: public PACKET_GENERIC
{
	int m_nStrLength;
};

struct PACKET_REQ_WAIT_LIST: public PACKET_GENERIC
{
	IDXCHANNEL m_idxReqChannel;
	bool m_bUpdate;
};
struct PACKET_REQ_ROOM_LIST: public PACKET_GENERIC
{
	IDXCHANNEL	m_idxReqChannel;
	int			m_nStartRoomPosition;	//Big Channel �� ��� ���° ����� �䱸�ϴ����� �� field�� ä���� ������.
	bool		m_bUpdate;

};
struct PACKET_REQ_PART_BRIEF: public PACKET_GENERIC
{
	//IDXCHANNEL m_idxChannel;
	IDXROOM	 m_idxRoom;
};
/////////////////////////////////////////////////
// Big channel������ ���Ǵ� packet
struct PACKET_REQ_SCROLL : public PACKET_GENERIC
{
	int		m_nScroll;
};
struct PACKET_REQ_CHG_SORT_METHOD : public PACKET_GENERIC
{
	int		m_nSortMethod;
};
// Big channel������ ���Ǵ� packet
/////////////////////////////////////////////////
struct PACKET_REQ_PART_DETAIL: public PACKET_GENERIC
{
	//IDXCHANNEL m_idxChannel;
	//IDXROOM	 m_idxRoom;
};
struct PACKET_JOIN_ROOM: public PACKET_GENERIC
{
	IDXROOM	m_idxRoom;
	char	m_strPassword[MAX_PASSWORD];
};
struct PACKET_HEART_BEAT: public PACKET_GENERIC
{
	long m_nTime;
};
struct PACKET_EXIT_ROOM: public PACKET_GENERIC
{
	bool m_bForced;
};
struct PACKET_EXIT_GAME: public PACKET_GENERIC
{
	bool m_bForced;
};
struct PACKET_CREATE_ROOM: public PACKET_GENERIC
{
	STRUCT_ROOM_GENERIC m_RoomGeneric;
};
struct PACKET_CHANGE_OWNER: public PACKET_GENERIC
{
};
//

struct PACKET_NOT_ADD_USER: public PACKET_GENERIC
{
	STRUCT_USER_INFO_GENERIC	m_userInfo;
};

struct PACKET_NOT_REMOVE_USER: public PACKET_GENERIC
{
	int	m_nKey;
};

struct PACKET_NOT_ADD_ROOM: public PACKET_GENERIC 
{
	STRUCT_ROOM_INFO_GENERIC	m_roomInfo;
};

struct PACKET_NOT_REMOVE_ROOM: public PACKET_GENERIC
{
	int	m_nRoomNumber;
};

struct STRUCT_CHANNEL_INFO
{
//	int m_nChannelNum;
//	int m_KindOfChannel;
//	int m_nMaxUserInChannel;
//	int m_nCurrentUser;
	int	m_nBeginLevel;
	int	m_nEndLevel;
	int	m_nBeginSubChannel;
	int	m_nEndSubChannel;
};
struct PACKET_NOT_CHANNEL_INFO : public PACKET_GENERIC
{
private:
	PACKET_NOT_CHANNEL_INFO(){}
	PACKET_NOT_CHANNEL_INFO(int nTotalSubChannel, int nTotalMajorChannels)
	{
		m_nTotalSubChannels = nTotalSubChannel;
		m_nTotalMajorChannels = nTotalMajorChannels;
	}
public:
	static PACKET_NOT_CHANNEL_INFO	*CreatePacket(int nKey, int nTotalSubChannels, int nTotalMajorChannels)
	{
		PACKET_NOT_CHANNEL_INFO *pPacket = 
			(PACKET_NOT_CHANNEL_INFO *)(new BYTE[sizeof(PACKET_GENERIC) + 2*sizeof(int)
								+ nTotalMajorChannels * sizeof(STRUCT_CHANNEL_INFO)
								+ nTotalSubChannels*sizeof(int)] );
		pPacket->Init(nKey, MSGID_NOT_CHANNEL_INFO);
		pPacket->m_nTotalMajorChannels = nTotalMajorChannels;
		pPacket->m_nTotalSubChannels = nTotalSubChannels;
		pPacket->Complete(pPacket->GetSize());
		return pPacket;
	}
	void	DeletePacket()
	{
		BYTE	*pPacket = (BYTE *)this;
		delete	pPacket;
	}
	int		*CurrentUser(int i=0)
	{
		return	(int *)(&m_param + sizeof(STRUCT_CHANNEL_INFO) * m_nTotalMajorChannels + sizeof(int)*i );
	}
	STRUCT_CHANNEL_INFO*	ChannelInfo(int i=0)
	{
		return	(STRUCT_CHANNEL_INFO *)(&m_param + sizeof(STRUCT_CHANNEL_INFO) * i);
	}
	int		GetSize() { return ( sizeof(PACKET_GENERIC) + 2*sizeof(int) + m_nTotalMajorChannels*sizeof(STRUCT_CHANNEL_INFO) + m_nTotalSubChannels * sizeof(int) ); }
	int		m_nTotalSubChannels;
	int		m_nTotalMajorChannels;
	BYTE	m_param;
};

struct PACKET_CHANGE_CHANNEL_OK: public PACKET_GENERIC
{
	IDXCHANNEL m_idxNewChannel;

};

struct PACKET_EXIT_GAME_OK: public PACKET_GENERIC
{
};

struct PACKET_EXIT_ROOM_OK: public PACKET_GENERIC
{
//	CONNECTID	m_ConnectIDExitingRoom;
//	bool	m_bForced;
};

struct PACKET_EXIT_ROOM_INFORM: public PACKET_GENERIC
{
//	CONNECTID	m_ConnectIDExitingRoom;
	BYTE	m_nSlot;
	bool	m_bForced;
};
/*
struct PACKET_CREATE_ROOM_OK: public PACKET_GENERIC
{
	IDXROOM m_idxRoom;

	int	m_idxUserInRoom;
};
*/
struct PACKET_CREATE_ROOM_OK: public PACKET_GENERIC	//JOIN_OK�϶��� �� ��Ŷ�� ����Ѵ�.
{
	int					m_nOrder;
	STRUCT_ROOM_GENERIC	m_roomInfo;
};
/*
struct PACKET_JOIN_ROOM_OK: public PACKET_GENERIC
{
	IDXROOM m_idxRoom;
	int	m_idxUserInRoom;
};
*/
struct PACKET_JOIN_ROOM_INFORM:public PACKET_GENERIC
{
	STRUCT_USER_INFO_GENERIC	m_userInfo;
};

struct PACKET_CHANGE_OWNER_OK : public PACKET_GENERIC
{
	IDXUSER	m_idxNewOwner;
};
struct PACKET_NOT_PART_DETAIL: public PACKET_GENERIC
{
	BYTE*	Payload() { return &m_param; }
	int		m_nItems;
	int		m_nOwnerKey;	//2002.2.12 tkhong
	BYTE	m_param;
};
struct PACKET_NOT_PART_BRIEF: public PACKET_GENERIC
{
	BYTE*	Payload() { return &m_param; }
	int		m_nItems;
	BYTE	m_param;
};
struct PACKET_NOT_WAIT_LIST: public PACKET_GENERIC
{
	BYTE*	Payload() { return &m_param; }
	int		m_nItems;
	BYTE	m_param;
};

struct PACKET_NOT_ROOM_LIST: public PACKET_GENERIC
{
	BYTE*	Payload() { return &m_param; }
	int		m_nTotalRoom;
	int		m_nStartRoomPosition;	// 60��° �� ���� �޶�� ���� ��� 60��° ����� �����ϸ� ���� 60�� ä���� �ְ�
	int		m_nItems;				// ���� �׻��̿� ���� �������� 55�� �ۿ� ���� �ƴٸ�,. ���� 54�� ä�� �ְ� nItem�� 1��
	BYTE	m_param;				// ä���ְ� 1���� ���ϸ��� ä���� �����ش�.
};


struct PACKET_CHANNEL_UPDATE_WAIT: public PACKET_GENERIC
{
	int m_nItems;
};
struct PACKET_CHANNEL_UPDATE_ROOM: public PACKET_GENERIC
{
	int m_nItems;
};


////////////////////////////////////�׼� ä��
typedef int AQ_ACTION;
#define ACTION_ADD		(1)
#define ACTION_REMOVE	(2)
#define ACTION_CHANGE	(3)
//���߿� �����Ҷ��� ���δ�
struct STRUCT_AQ_GENERIC
{
	AQ_ACTION Action;	
};

struct STRUCT_AQ_ROOM_ADD : public STRUCT_AQ_GENERIC
{
	IDXROOM	m_idxRoomInChannel;
	STRUCT_ROOM_GENERIC m_RoomGeneric;
	
};
struct STRUCT_AQ_ROOM_REMOVE : public STRUCT_AQ_GENERIC
{
	IDXROOM m_idxRoom;
};
struct STRUCT_AQ_ROOM_CHANGE : public STRUCT_AQ_GENERIC
{
	IDXROOM m_idxRoom;
	STRUCT_ROOM_GENERIC m_RoomGeneric;
	
};
struct STRUCT_AQ_WAIT_ADD :public STRUCT_AQ_GENERIC
{
	IDXUSER m_idxUser; 
	char m_strID[MAX_USERID];
};
struct STRUCT_AQ_WAIT_REMOVE : public STRUCT_AQ_GENERIC
{
	IDXUSER m_idxUser;
};


// notify code
#define ERROR_UNKNOWN					(-1)
#define ERROR_CANNOT_LEAVE_ROOM			(-2)

//login
//������ �պκ��� ���� REASON code�� ��ü #define ERROR_LOGIN_DUPLICATE_USERID	(-3)
//������ �պκ��� ���� REASON code�� ��ü #define ERROR_LOGIN_UNKOWN_USERID		(-4)
//������ �պκ��� ���� REASON code�� ��ü #define ERROR_LOGIN_INVALID_PASSWORD	(-5)
//������ �պκ��� ���� REASON code�� ��ü #define ERROR_LOGIN_DUPLICATED_KEY		(-6)
//channel
//������ �պκ��� ���� REASON code�� ��ü #define ERROR_CHANNEL_IN_ROOM			(-10)
//������ �պκ��� ���� REASON code�� ��ü #define ERROR_CHANNEL_NOT_EXIST			(-11)
//������ �պκ��� ���� REASON code�� ��ü #define ERROR_CHANNEL_FULL				(-12)
//������ �պκ��� ���� REASON code�� ��ü #define ERROR_ROOM_CANNOT_ALLOC_ROOM	(-25)


/////////////////////////////////////////////////////////////
// P2P packet definition

struct PACKET_P2P_REQ_PEER_ADDR : public PACKET_GENERIC
{
	BYTE*	Payload() { return &m_param; }
	int		ItemSize() { return sizeof(int);}
	int		GetPeerKey(int i) { return *( (int *)(Payload() +	i*ItemSize()) ); }
	void	SetPeerKey(int i, int nKey) { *( (int *)(Payload() +	i*ItemSize()) ) = nKey; }
	int		GetSize() { return (sizeof(PACKET_GENERIC) + sizeof(int) + sizeof(SOCKADDR) + m_nNumberOfPeer * sizeof(int) );	}
	int			m_nNumberOfPeer;
	SOCKADDR	m_addrLocal;
	BYTE		m_param;	
};


struct PACKET_P2P_GENERIC : public PACKET_GENERIC
{
	void	InitData()
	{
		m_nDestPeerKey = -1;
		m_bFlag = 0;
		m_nSeq = 0;
		m_nAckSeq = 0;
		m_nTick = 0;
	}
	PACKET_P2P_GENERIC()
	{
		InitData();
	}
	int		m_nDestPeerKey;
	BYTE	m_bFlag;
	BYTE	m_nSeq;
	BYTE	m_nAckSeq;
	int		m_nTick;
};

struct PACKET_P2P_REQ_CONNECT : public PACKET_P2P_GENERIC
{
	SOCKADDR	m_addrLocal;
};

struct PACKET_P2P_ACK_CONNECT : public PACKET_P2P_GENERIC
{
	SOCKADDR	m_addrPublic;
};

//NOTE [2006-5-26 jovial] : struct��ü�� streaming�Ǳ� ������ virtual�� �̿��� ���� ����
struct PACKET_P2P_SESSION_AWARE : public PACKET_P2P_GENERIC
{
public:
	PACKET_P2P_SESSION_AWARE()
	{
		InitData();
	}

	void InitData()
	{
		__super::InitData();
		m_nSessionKey = 0;
	}
	__int32 GetSessionKey() { return m_nSessionKey; }
public:
	__int32 m_nSessionKey; 
};

struct STRUCT_PEER_ADDR
{
	void	Init()
	{
		m_nKey = -1;
		memset(&m_addrLocal, 0, sizeof(SOCKADDR));
		memset(&m_addrPublic, 0, sizeof(SOCKADDR));
	}
	void	SetAddrLocal(SOCKADDR* pAddr)	//���� NAT���� ����� ���ؼ� �ʿ��ϴ�. 2005.8.9 tkhong
	{
		if (NULL==pAddr) return;
		memcpy(&m_addrLocal, pAddr, sizeof(SOCKADDR));
	}
	void	SetAddrPublic(SOCKADDR* pAddr)
	{
		if (NULL==pAddr) return;
		memcpy(&m_addrPublic, pAddr, sizeof(SOCKADDR));
	}
	int			m_nKey;
	SOCKADDR	m_addrLocal;	//���� NAT���� ����� ���ؼ� �ʿ��ϴ�. 2005.8.9 tkhong
	SOCKADDR	m_addrPublic;
};

struct PACKET_P2P_PEER_ADDR_OK : public PACKET_GENERIC
{
	BYTE*	Payload() { return &m_param; }
	int		ItemSize() { return sizeof(STRUCT_PEER_ADDR);}
	STRUCT_PEER_ADDR*	GetPeerAddr(int i) { return (STRUCT_PEER_ADDR *)(Payload() +	i*ItemSize() ); }
	int		GetSize() { return (sizeof(PACKET_GENERIC) + sizeof(int) + m_nNumberOfPeer * sizeof(STRUCT_PEER_ADDR) );	}
	int		m_nNumberOfPeer;
	BYTE	m_param;
};

struct PACKET_P2P_REQ_PEER_INFO :public PACKET_P2P_GENERIC
{
	BYTE*	Payload() { return &m_param; }
	int		ItemSize() { return sizeof(int);}
	int		GetPeerKey(int i) { return *( (int *)(Payload() +	i*ItemSize()) ); }
	void	SetPeerKey(int i, int nKey) { *( (int *)(Payload() +	i*ItemSize()) ) = nKey; }
	int		GetSize() { return (sizeof(PACKET_P2P_GENERIC) + sizeof(int) + m_nNumberOfPeer * sizeof(int) );	}
	int			m_nNumberOfPeer;
	BYTE		m_param;	
};
struct PACKET_P2P_PEER_INFO_OK : public PACKET_P2P_GENERIC
{
	BYTE*	Payload() { return &m_param; }
	int		ItemSize() { return sizeof(STRUCT_PEER_ADDR);}
	STRUCT_PEER_ADDR*	GetPeerAddr(int i) { return (STRUCT_PEER_ADDR *)(Payload() +	i*ItemSize() ); }
	int		GetSize() { return (sizeof(PACKET_P2P_GENERIC) + sizeof(int) + m_nNumberOfPeer * sizeof(STRUCT_PEER_ADDR) );	}
	int		m_nNumberOfPeer;
	BYTE	m_param;
};

struct PACKET_P2P_RQ_HOLE_PUNCH : public PACKET_P2P_GENERIC
{
	SOCKADDR	m_RequsterPublicAddr;
};
struct PACKET_P2P_HOLE_PROBE : public PACKET_P2P_GENERIC
{
	SOCKADDR	m_RequsteePublicAddr;
};

struct PACKET_P2P_HOLE_PUNCH : public PACKET_P2P_GENERIC 
{
	SOCKADDR		m_RequsteePublicAddr;
	unsigned short	m_nBeginPort;
	unsigned short	m_nEndPort;
};

struct PACKET_P2P_HOLE_SCAN : public PACKET_P2P_GENERIC 
{
	unsigned short	m_nPort;
};

struct PACKET_P2P_TIME : public PACKET_P2P_GENERIC
{
	int		m_nT1;
	int		m_nT2;
	int		m_nT3;
	int		m_nT4;
};

// {{ 20070129 dEAthcURe
struct PACKET_HeartbeatV2 : PACKET_P2P_GENERIC {
	int count;
};
// }} 20070129 dEAthcURe
#pragma pack(pop)
#endif
