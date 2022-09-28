#ifndef __LIBOS_H264_PARSE_H
#define __LIBOS_H264_PARSE_H

#include <stdint.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
	typedef enum {
		NALU_TYPE_SLICE    = 1,
		NALU_TYPE_DPA      = 2,
		NALU_TYPE_DPB      = 3,
		NALU_TYPE_DPC      = 4,
		NALU_TYPE_IDR      = 5,
		NALU_TYPE_SEI      = 6,
		NALU_TYPE_SPS      = 7,
		NALU_TYPE_PPS      = 8,
		NALU_TYPE_AUD      = 9,
		NALU_TYPE_EOSEQ    = 10,
		NALU_TYPE_EOSTREAM = 11,
		NALU_TYPE_FILL     = 12,
	} NaluType;

	typedef enum {
		NALU_PRIORITY_DISPOSABLE = 0,
		NALU_PRIRITY_LOW         = 1,
		NALU_PRIORITY_HIGH       = 2,
		NALU_PRIORITY_HIGHEST    = 3
	} NaluPriority;

	typedef struct
	{
		int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
		int forbidden_bit;            //! should be always FALSE
		int nal_reference_idc;        //! NALU_PRIORITY_xxxx
		int nal_unit_type;            //! NALU_TYPE_xxxx    
		char *buf;                    //! contains the first byte followed by the EBSP
		unsigned len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
		unsigned max_size;            //! Nal Unit Buffer size
	} NALU_t;

	int32_t H264NALUDataParser(char* inData, int inDataSize, char* h264Frame, int* h264FrameSize);

	int32_t H264NALUParser(char* inData, int32_t inDataSize, NALU_t* outNALU, int32_t* inDataRemainIndex); //每次只解析一个

	int32_t H264SPSParser(char* sps, int32_t spsSize);
	int32_t H264PPSParser(char* pps, int32_t ppsSize);

	int32_t FindStartCode2(char* buf);
	int32_t FindStartCode3(char* buf);
	int32_t SearchFirstStartCodeIndex(char* buf, int32_t bufSize, int32_t* code2OrCode3);
	int32_t SearchFirstNualTypeIndex(char* buf, int32_t bufSize, NaluType naluType);

#ifdef __cplusplus
}
#endif

#endif//!__LIBOS_H264_PARSE_H