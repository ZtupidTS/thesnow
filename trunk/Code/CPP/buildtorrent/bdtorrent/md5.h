#ifndef MD5_H
#define MD5_H

//#include <inttypes.h>
#include <Windows.h>

#ifdef _MSC_VER    /* however you determine using VC++ compiler ?: no clue! */
typedef  __int8 int8;
typedef  __int16 int16;
typedef __int32 int32;
typedef __int64 int64;
typedef unsigned __int8 uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;
typedef unsigned __int8 uint8_t;
typedef unsigned __int32 uint32_t;
#endif

struct MD5Context {
	uint32_t buf[4];
	uint32_t bits[2];
	uint8_t in[64];
};

void MD5Init(struct MD5Context *context);
void MD5Update(struct MD5Context *context, uint8_t const *buf, unsigned int len);
void MD5Final(uint8_t digest[16], struct MD5Context *context);
void MD5Transform(uint32_t buf[4], uint32_t const in[16]);

typedef struct MD5Context MD5_CTX;

#endif
