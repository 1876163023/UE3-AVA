#pragma once

#define RXNERVE_ADDRESS_SIZE	8
#define RXGATE_INVALID_CLIENT_KEY	0L
#define RXGATE_INVALID_SESSION_KEY	0xff

namespace RxGate
{

#pragma pack(push)
#pragma pack(1)
	typedef struct _RXNERVE_ADDRESS
	{
		unsigned char addrType;					// Address Type - RXNERVE_ADDRESS_TYPE Value
		union
		{
			unsigned char		address[RXNERVE_ADDRESS_SIZE];
			unsigned __int64	address64;				// for compare
		};

		_RXNERVE_ADDRESS()
			: addrType(0)
		{
			::ZeroMemory((LPVOID)address, sizeof(address));
		}
	} RXNERVE_ADDRESS;
#pragma pack(pop)

	typedef RXNERVE_ADDRESS* LPRXNERVE_ADDRESS;


#ifndef RXGATE_PORT
	#define	RXGATE_PORT					12735				// Client Open Port
#endif
}
