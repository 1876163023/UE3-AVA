#pragma once

namespace DefUdp
{

	enum
	{
		UN_MIN_MTU = 576,
		UN_MAX_MTU = 4096,
		UN_MAX_PACKET_COMMANDS = 32,
		UN_MIN_WINDOW_SIZE = 4096,
		UN_MAX_WINDOW_SIZE = 32768,
		UN_MIN_CHANNEL_COUNT = 1,
		UN_MAX_CHANNEL_COUNT = 255,

		UN_HOST_BANDWIDTH_THROTTLE_INTERVAL = 1000,
		UN_HOST_DEFAULT_MTU = 1400,

		UN_PEER_DEFAULT_ROUND_TRIP_TIME = 500,
		UN_PEER_DEFAULT_PACKET_THROTTLE = 32,
		UN_PEER_PACKET_THROTTLE_SCALE = 32,
		UN_PEER_PACKET_THROTTLE_COUNTER = 7, 
		UN_PEER_PACKET_THROTTLE_ACC = 2,
		UN_PEER_PACKET_THROTTLE_DEC = 2,
		UN_PEER_PACKET_THROTTLE_INTERVAL = 5000,
		UN_PEER_PACKET_LOSS_SCALE = (1 << 16),
		UN_PEER_PACKET_LOSS_INTERVAL = 10000,
		UN_PEER_WINDOW_SIZE_SCALE = 64 * 1024,
		UN_PEER_TIMEOUT_LIMIT = 32,
		UN_PEER_TIMEOUT_MINIMUM = 3000,
		UN_PEER_TIMEOUT_MAXIMUM = 30000,
		UN_PEER_PING_INTERVAL = 500,
		UN_PEER_UNSEQUENCED_WINDOW_SIZE = 4 * 32,
	};

	enum _UdpFlag
	{
		UF_ACK			= (1 << 0),
		UF_UNSEQUENCED	= (1 << 1),
	};

	enum _UdpState
	{
		US_NA = 0,
		US_DISCONNECTED,
		US_CONNECTING,
		US_ACK_CONNECT,
		US_CONNECTED,
		US_DISCONNECTING,
		US_ACK_DISCONNECT,
		US_ZOMBIE
	};

	enum UDP_CMD
	{
		UC_NONE = 0,
		UC_ACK,
		UC_CONNECT_REQ,
		UC_CONNECT_ANS,
		UC_DISCONNECT,
		UC_PING,
		UC_SEND_RELIABLE,
		UC_SEND_UNRELIABLE,
		UC_SEND_FRAGMENT,
		UC_BANDWIDTH,
		UC_THROTTLE,
		UC_SEND_UNSEQUENCED
	};

	struct UDP_HEADER
	{
		WORD idPeer;
		BYTE flag;
		BYTE cntCommand;
		DWORD sentTime;
		DWORD challenge;
	};

	struct UDP_CMD_HEADER
	{
		BYTE command;
		BYTE idChannel;
		BYTE cmdFlag;
		BYTE reserved;
		DWORD numReliableSeq;
	};

	struct UDPCMD_ACK
	{
		UDP_CMD_HEADER header;
		DWORD numReliableSeq;
		DWORD recvedSentTime;
	};

	struct UDPCMD_CONNECT_REQ
	{
		UDP_CMD_HEADER header;
		WORD idPeer;
		WORD mtu;
		DWORD windowSize;
		DWORD cntChannel;
		DWORD inBandwidth;
		DWORD outBandwidth;
		DWORD packetThrottleInterval;
		DWORD packetThrottleAcc;
		DWORD packetThrottleDec;
	};

	struct UDPCMD_CONNECT_ANS
	{
		UDP_CMD_HEADER header;
		WORD idPeer;
		WORD mtu;
		DWORD challenge;
		DWORD windowSize;
		DWORD cntChannel;
		DWORD inBandwidth;
		DWORD outBandwidth;
		DWORD packetThrottleInterval;
		DWORD packetThrottleAcc;
		DWORD packetThrottleDec;
	};

	struct UDPCMD_DISCONNECT
	{
		UDP_CMD_HEADER header;
	};

	struct UDPCMD_PING
	{
		UDP_HEADER header;
	};

	struct UDPCMD_SEND_RELIABLE
	{
		UDP_CMD_HEADER header;
		DWORD length;
	};

	struct UDPCMD_SEND_UNRELIABLE
	{
		UDP_CMD_HEADER header;
		DWORD numUnreliableSeq;
		DWORD length;
	};

	struct UDPCMD_SEND_UNSEQUENCED
	{
		UDP_CMD_HEADER header;
		DWORD unsequencedGroup;
		DWORD numUnreliableSeq;
		DWORD length;
	};

	struct UDPCMD_SEND_FRAGMENT
	{
		UDP_CMD_HEADER header;
		DWORD numStartSeq;
		DWORD cntFragment;
		DWORD numFragment;
		DWORD lenFragment;
		DWORD lenTotal;
		DWORD offset;
	};

	struct UDPCMD_BANDWIDTH
	{
		UDP_CMD_HEADER header;
		DWORD inBandwidth;
		DWORD outBandwidth;
	};

	struct UDPCMD_THROTTLE
	{
		UDP_CMD_HEADER header;
		DWORD packetThrottleInterval;
		DWORD packetThrottleAcc;
		DWORD packetThrottleDec;
	};

}
