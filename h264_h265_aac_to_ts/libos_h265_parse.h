#ifndef __LIBOS_H265_PARSE_H
#define __LIBOS_H265_PARSE_H

#include <stdint.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

	enum NALUnitType
	{
		NAL_TRAIL_N     = 0,
		NAL_TRAIL_R     = 1,
		NAL_TSA_N       = 2,
		NAL_TSA_R       = 3,
		NAL_STSA_N      = 4,
		NAL_STSA_R      = 5,
		NAL_RADL_N      = 6,
		NAL_RADL_R      = 7,
		NAL_RASL_N      = 8,
		NAL_RASL_R      = 9,
		NAL_BLA_W_LP    = 16,
		NAL_BLA_W_RADL  = 17,
		NAL_BLA_N_LP    = 18,
		NAL_IDR_W_RADL  = 19,
		NAL_IDR_N_LP    = 20,
		NAL_CRA_NUT     = 21,
		NAL_VPS         = 32,
		NAL_SPS         = 33,
		NAL_PPS         = 34,
		NAL_AUD         = 35,
		NAL_EOS_NUT     = 36,
		NAL_EOB_NUT     = 37,
		NAL_FD_NUT      = 38,
		NAL_SEI_PREFIX  = 39,
		NAL_SEI_SUFFIX  = 40,
	};

	typedef struct
	{
		int       startcodelen;     
		int       forbidden;         
		int       type;              
		int       layerId;
		int       tid;
		char*     buf;              
		unsigned  len;               
		unsigned  max_size;          
	} H265NALU_t;

	/*
		+---------------+---------------+
		|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|
		+---------------+---------------+
		|F|   NALTYPE | LayerId   | TID |
		+---------------+---------------+
	*/

	//00 00 00 01 40 01  的nuh_unit_type的值为 32， 语义为视频参数集        VPS
	//00 00 00 01 42 01  的nuh_unit_type的值为 33， 语义为序列参数集         SPS
	//00 00 00 01 44 01  的nuh_unit_type的值为 34， 语义为图像参数集         PPS
	//00 00 00 01 4E 01  的nuh_unit_type的值为 39， 语义为补充增强信息       SEI
	//00 00 00 01 26 01  的nuh_unit_type的值为 19， 语义为可能有RADL图像的IDR图像的SS编码数据   IDR
	//00 00 00 01 02 01  的nuh_unit_type的值为1，   语义为被参考的后置图像，且非TSA、非STSA的SS编码数据

	//40 01 == == 》二进制 0100 0000 0000 0001
	//F      ： 0
	//NalType：100 000 == 》32 = 》VPS
	//LayerID：0 0000 0 == 》0
	//TID    ：001 == 》1


	int32_t H265NALUDataParser(char* inData, int inDataSize, char* h264Frame, int* h264FrameSize);
	int32_t H265NALUParser(char* inData, int32_t inDataSize, H265NALU_t* outNALU, int32_t* inDataRemainIndex); //每次只解析一个

	int32_t H265FindStartCode2(char* buf);
	int32_t H265FindStartCode3(char* buf);
	int32_t H265SearchFirstStartCodeIndex(char* buf, int32_t bufSize, int32_t* code2OrCode3);

#ifdef __cplusplus
}
#endif

#endif//!__LIBOS_H265_PARSE_H