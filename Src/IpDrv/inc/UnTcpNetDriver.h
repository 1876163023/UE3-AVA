/*=============================================================================
	UnTcpNetDriver.h: Unreal TCP/IP driver.
	Copyright ?1997-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/*-----------------------------------------------------------------------------
	UTcpipConnection.
-----------------------------------------------------------------------------*/

//
// Windows socket class.
//
class UTcpipConnection : public UNetConnection
{
	DECLARE_CLASS(UTcpipConnection,UNetConnection,CLASS_Config|CLASS_Transient,IpDrv)

	// Variables.
	FInternetIpAddr		RemoteAddr;
	FSocket*			Socket;
	UBOOL				OpenedLocally;
	FResolveInfo*		ResolveInfo;

	/**
	 * Intializes a connection with the passed in settings
	 *
	 * @param InDriver the net driver associated with this connection
	 * @param InSocket the socket associated with this connection
	 * @param InRemoteAddr the remote address for this connection
	 * @param InState the connection state to start with for this connection
	 * @param InOpenedLocally whether the connection was a client/server
	 * @param InURL the URL to init with
	 * @param InMaxPacket the max packet size that will be used for sending
	 * @param InPacketOverhead the packet overhead for this connection type
	 */
	virtual void InitConnection(UNetDriver* InDriver,FSocket* InSocket,const FInternetIpAddr& InRemoteAddr,EConnectionState InState,UBOOL InOpenedLocally,const FURL& InURL,INT InMaxPacket = 0,INT InPacketOverhead = 0);

	/**
	 * Sends a byte stream to the remote endpoint using the underlying socket
	 *
	 * @param Data the byte stream to send
	 * @param Count the length of the stream to send
	 */
	virtual void LowLevelSend(void* Data,INT Count);

	FString LowLevelGetRemoteAddress();
	FString LowLevelDescribe();
};

/*-----------------------------------------------------------------------------
	UTcpNetDriver.
-----------------------------------------------------------------------------*/

// added by alcor

class IP2PHandler
{
public:
	virtual void OnInitListen(FSocket*) = 0;
	virtual void OnInitConnect(FSocket*, FURL&) = 0;
	virtual void OnTick() = 0;
	virtual bool OnRecvFrom(char*, INT, FInternetIpAddr&) = 0;
};

// added by alcor


//
// BSD sockets network driver.
//
class UTcpNetDriver : public UNetDriver
{
	DECLARE_CLASS(UTcpNetDriver,UNetDriver,CLASS_Transient|CLASS_Config,IpDrv)

	UBOOL AllowPlayerPortUnreach;
	UBOOL LogPortUnreach;

	// Variables.
	FInternetIpAddr LocalAddr;
	FSocket* Socket;

	// Constructor.
	void StaticConstructor();
	UTcpNetDriver()
	{}

	// UNetDriver interface.
	UBOOL InitConnect( FNetworkNotify* InNotify, FURL& ConnectURL, FString& Error );
	UBOOL InitListen( FNetworkNotify* InNotify, FURL& LocalURL, FString& Error );
	void TickDispatch( FLOAT DeltaTime );
	FString LowLevelGetNetworkNumber();
	void LowLevelDestroy();

	// UTcpNetDriver interface.
	UBOOL InitBase( UBOOL Connect, FNetworkNotify* InNotify, FURL& URL, FString& Error );
	UTcpipConnection* GetServerConnection();
	FSocketData GetSocketData();

public:
	static IP2PHandler *P2PHandler;
	static void SetP2PHandler(IP2PHandler *pHandler)
	{
		P2PHandler = pHandler;
	}
};

// {{ 20070120 dEAthcURe
//---------------------------------------------------------------------------
extern FSocketWin* _ahead_socket;
extern int _ahead_port;
extern int _ahead_setDefaultPort(int port);
extern bool _ahead_initSocket(int port = 0);
extern void _ahead_deinitSocket(void);
//---------------------------------------------------------------------------
// }} 20070120 dEAthcURe