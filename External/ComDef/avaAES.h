//
//	avaAES.h
//

#ifndef _AVAAES_H_
#define _AVAAES_H_

#include <limits.h>

#define AVAAES_BLOCK_SIZE		16
#define AVAAES_KS_LENGTH			4 * AVAAES_BLOCK_SIZE
#define AVAAES_RC_LENGTH			5 * AVAAES_BLOCK_SIZE / 4 - (AVAAES_BLOCK_SIZE == 16 ? 10 : 11)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AES Configuration Options
// 1. Byte Order
#define AVAAES_LITTLE_ENDIAN   1234 /* byte 0 is least significant (i386) */
#define AVAAES_BIG_ENDIAN      4321 /* byte 0 is most significant (mc68k) */

#if (('1234' >> 24) == '1')
#define AVAAES_PLATFORM_BYTE_ORDER AVAAES_LITTLE_ENDIAN
#elif (('4321' >> 24) == '1')
#define AVAAES_PLATFORM_BYTE_ORDER AVAAES_BIG_ENDIAN
#endif

// 2. Loop Unrolling
#if 1
#define AVAAES_ENC_UNROLL	AVAAES_FULL
#elif 0
#define AVAAES_ENC_UNROLL	AVAAES_PARTIAL
#else
#define AVAAES_ENC_UNROLL	AVAAES_NONE
#endif

#if 1
#define AVAAES_DEC_UNROLL	AVAAES_FULL
#elif 0
#define AVAAES_DEC_UNROLL	AVAAES_PARTIAL
#else
#define AVAAES_DEC_UNROLL	AVAAES_NONE
#endif

// 3. Round Count
#define AVAAES_NO_TABLES   0   /* DO NOT CHANGE */
#define AVAAES_ONE_TABLE   1   /* DO NOT CHANGE */
#define AVAAES_FOUR_TABLES 4   /* DO NOT CHANGE */
#define AVAAES_NONE        0   /* DO NOT CHANGE */
#define AVAAES_PARTIAL     1   /* DO NOT CHANGE */
#define AVAAES_FULL        2   /* DO NOT CHANGE */

#if 1   /* set tables for the normal encryption round */
#define AVAAES_ENC_ROUND		AVAAES_FOUR_TABLES
#elif 0
#define AVAAES_ENC_ROUND		AVAAES_ONE_TABLE
#else
#define AVAAES_ENC_ROUND		AVAAES_NO_TABLES
#endif

#if 1   /* set tables for the last encryption round */
#define AVAAES_LAST_ENC_ROUND  AVAAES_FOUR_TABLES
#elif 0
#define AVAAES_LAST_ENC_ROUND  AVAAES_ONE_TABLE
#else
#define AVAAES_LAST_ENC_ROUND  AVAAES_NO_TABLES
#endif

#if 1   /* set tables for the normal decryption round */
#define AVAAES_DEC_ROUND		AVAAES_FOUR_TABLES
#elif 0
#define AVAAES_DEC_ROUND		AVAAES_ONE_TABLE
#else
#define AVAAES_DEC_ROUND		AVAAES_NO_TABLES
#endif

#if 1   /* set tables for the last decryption round */
#define AVAAES_LAST_DEC_ROUND  AVAAES_FOUR_TABLES
#elif 0
#define AVAAES_LAST_DEC_ROUND  AVAAES_ONE_TABLE
#else
#define AVAAES_LAST_DEC_ROUND  AVAAES_NO_TABLES
#endif

#if 1
#define AVAAES_KEY_SCHED   AVAAES_FOUR_TABLES
#elif 0
#define AVAAES_KEY_SCHED   AVAAES_ONE_TABLE
#else
#define AVAAES_KEY_SCHED   AVAAES_NO_TABLES
#endif

// Configuration Check 
#if AVAAES_ENC_ROUND == AVAAES_NO_TABLES && AVAAES_LAST_ENC_ROUND != AVAAES_NO_TABLES
#undef  AVAAES_LAST_ENC_ROUND
#define AVAAES_LAST_ENC_ROUND  AVAAES_NO_TABLES
#elif AVAAES_ENC_ROUND == ONE_TABLE && AVAAES_LAST_ENC_ROUND == AVAAES_FOUR_TABLES
#undef  AVAAES_LAST_ENC_ROUND
#define AVAAES_LAST_ENC_ROUND  AVAAES_ONE_TABLE 
#endif

#if AVAAES_ENC_ROUND == AVAAES_NO_TABLES && AVAAES_ENC_UNROLL != AVAAES_NONE
#undef  AVAAES_ENC_UNROLL
#define AVAAES_ENC_UNROLL  AVAAES_NONE
#endif

#if AVAAES_DEC_ROUND == AVAAES_NO_TABLES && AVAAES_LAST_DEC_ROUND != AVAAES_NO_TABLES
#undef  AVAAES_LAST_DEC_ROUND
#define AVAAES_LAST_DEC_ROUND  AVAAES_NO_TABLES
#elif AVAAES_DEC_ROUND == AVAAES_ONE_TABLE && AVAAES_LAST_DEC_ROUND == AVAAES_FOUR_TABLES
#undef  AVAAES_LAST_DEC_ROUND
#define AVAAES_LAST_DEC_ROUND  AVAAES_ONE_TABLE 
#endif

#if AVAAES_DEC_ROUND == AVAAES_NO_TABLES && AVAAES_DEC_UNROLL != AVAAES_NONE
#undef  AVAAES_DEC_UNROLL
#define AVAAES_DEC_UNROLL  AVAAES_NONE
#endif

// Configuration Option Complete

// AES Encryption Method
class CavaAES 
{
public:
	typedef struct 
	{
		UINT keySchedule[AVAAES_KS_LENGTH];			// Encryption Key schedule
		UINT roundCount;							// the number of cipher rounds
		UINT byteCount;								// the number of bytes in the state
	} _AVAAES_CONTEXT;

public:
	CavaAES();
	~CavaAES();

public:
	BOOL Init(const LPBYTE encryptKey, UINT keyLen);
	BOOL Encrypt(const LPBYTE plainMsg, UINT msgLen, LPBYTE cipherMsg, LPUINT cipherLen);
	BOOL Decrypt(const LPBYTE cipherMsg, UINT msgLen, LPBYTE plainMsg, LPUINT plainLen);

	void BuildKey(LPBYTE key, UINT keyLen, UINT addKey);

private:
	BOOL PrepareEncKey(const LPBYTE key, UINT keyLen);
	BOOL PrepareDecKey(const LPBYTE key, UINT keyLen);

private:
	_AVAAES_CONTEXT encCX;
	_AVAAES_CONTEXT decCX;
};

#endif
