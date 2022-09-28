#ifndef __LIBOS_COMMON_H
#define __LIBOS_COMMON_H

#include <stdint.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define avx_max(val1, val2)  ((val1 < val2) ? (val2) : (val1))
#define avx_min(val1, val2)  ((val1 > val2) ? (val2) : (val1))


//2字节转16bit，大端存储 
#define AVX_RB16(x) ((uint16_t)(((const uint8_t*)(x))[0] << 8) | ((const uint8_t*)(x))[1])
#define AVX_WB16(p, darg) do {              \
	unsigned d = (darg);                    \
	((uint8_t*)(p))[1] = (d);               \
	((uint8_t*)(p))[0] = (d) >> 8;          \
} while (0)


//2字节转16bit，小端存储 
#define AVX_RL16(x) ((uint16_t)(((const uint8_t*)(x))[1] << 8) | ((const uint8_t*)(x))[0])
#define AVX_WL16(p, darg) do {              \
	unsigned d = (darg);                    \
	((uint8_t*)(p))[0] = (d);               \
	((uint8_t*)(p))[1] = (d) >> 8;          \
} while (0)


//4字节转32bit，大端存储 
#define AVX_RB32(x) (((uint32_t)((const uint8_t*)(x))[0] << 24) | (((const uint8_t*)(x))[1] << 16) | (((const uint8_t*)(x))[2] << 8) | ((const uint8_t*)(x))[3])
#define AVX_WB32(p, darg) do {              \
	unsigned d = (darg);                    \
	((uint8_t*)(p))[3] = (d);               \
	((uint8_t*)(p))[2] = (d) >> 8;          \
	((uint8_t*)(p))[1] = (d) >> 16;         \
	((uint8_t*)(p))[0] = (d) >> 24;         \
} while (0)


//4字节转32bit，小端存储 
#define AVX_RL32(x) (((uint32_t)((const uint8_t*)(x))[3] << 24) | (((const uint8_t*)(x))[2] << 16) | (((const uint8_t*)(x))[1] << 8) | ((const uint8_t*)(x))[0])
#define AVX_WL32(p, darg) do {              \
	unsigned d = (darg);                    \
	((uint8_t*)(p))[0] = (d);               \
	((uint8_t*)(p))[1] = (d) >> 8;          \
	((uint8_t*)(p))[2] = (d) >> 16;         \
	((uint8_t*)(p))[3] = (d) >> 24;         \
} while (0)



#define AVX_RB64(x)                              \
	(((uint64_t)((const uint8_t*)(x))[0] << 56)| \
	((uint64_t)((const uint8_t*)(x))[1] << 48) | \
	((uint64_t)((const uint8_t*)(x))[2] << 40) | \
	((uint64_t)((const uint8_t*)(x))[3] << 32) | \
	((uint64_t)((const uint8_t*)(x))[4] << 24) | \
	((uint64_t)((const uint8_t*)(x))[5] << 16) | \
	((uint64_t)((const uint8_t*)(x))[6] << 8)  | \
	(uint64_t)((const uint8_t*)(x))[7])

#   define AVX_WB64(p, darg) do {                \
	uint64_t d = (darg);                    \
	((uint8_t*)(p))[7] = (d);               \
	((uint8_t*)(p))[6] = (d) >> 8;            \
	((uint8_t*)(p))[5] = (d) >> 16;           \
	((uint8_t*)(p))[4] = (d) >> 24;           \
	((uint8_t*)(p))[3] = (d) >> 32;           \
	((uint8_t*)(p))[2] = (d) >> 40;           \
	((uint8_t*)(p))[1] = (d) >> 48;           \
	((uint8_t*)(p))[0] = (d) >> 56;           \
} while (0)

#   define AVX_RL64(x)                                   \
	(((uint64_t)((const uint8_t*)(x))[7] << 56) | \
	((uint64_t)((const uint8_t*)(x))[6] << 48) | \
	((uint64_t)((const uint8_t*)(x))[5] << 40) | \
	((uint64_t)((const uint8_t*)(x))[4] << 32) | \
	((uint64_t)((const uint8_t*)(x))[3] << 24) | \
	((uint64_t)((const uint8_t*)(x))[2] << 16) | \
	((uint64_t)((const uint8_t*)(x))[1] << 8) | \
	(uint64_t)((const uint8_t*)(x))[0])

#   define AVX_WL64(p, darg) do {                \
	uint64_t d = (darg);                    \
	((uint8_t*)(p))[0] = (d);               \
	((uint8_t*)(p))[1] = (d) >> 8;            \
	((uint8_t*)(p))[2] = (d) >> 16;           \
	((uint8_t*)(p))[3] = (d) >> 24;           \
	((uint8_t*)(p))[4] = (d) >> 32;           \
	((uint8_t*)(p))[5] = (d) >> 40;           \
	((uint8_t*)(p))[6] = (d) >> 48;           \
	((uint8_t*)(p))[7] = (d) >> 56;           \
} while (0)


//大端转换
#define AVX_FOURCC( a, b, c, d ) ( ( ((uint32_t)a) << 24 ) | ( ((uint32_t)b) << 16 ) | ( ((uint32_t)c) << 8 ) | ((uint32_t)d))


int CheckBigEndlen();

int AVX_DoubleToHEX(double src, uint8_t* dest);
double AVX_HEXToDouble(uint8_t* src);

unsigned int  calc_crc32(unsigned char *data, unsigned int datalen);



#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif//!__LIBOS_H264_PARSE_H