#ifndef __LIBOS_AAC_PARSE_H
#define __LIBOS_AAC_PARSE_H

#include <stdint.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

	//ADTS header[7byte || 9byte]
	//
	//ADTS header = adts_fixed_header() + adts_variable_header() = > 28bit + 28bit
	//
	//adts_fixed_header:
	//syncword【12bit】：               帧同步标识一个帧的开始，固定为0xFFF
	//ID【1bit】：                      MPEG 标示符。0表示MPEG - 4，1表示MPEG - 2
	//layer【2bit】：                   固定为'00'
	//protection_absent【1bit】：       标识是否进行误码校验。0表示有CRC校验，1表示没有CRC校验
	//profile【2bit】：                 标识使用哪个级别的AAC 1 : AAC Main 2 : AAC LC 3 : AAC SSR 4 : AAC LTP
	//sampling_frequency_index【4bit】：标识使用的采样率的下标 0x0 : 96000 0x1 : 88200 0x2 : 64000 0x3 : 48000 0x4 : 44100 0x5 : 32000 0x6 : 24000 0x7 : 22050 0x8 : 16000 ...
	//private_bit【1bit】：             私有位，编码时设置为0，解码时忽略
	//channel_configuration【3bit】：   标识声道数  1, 2
	//original_copy【1bit】：           编码时设置为0，解码时忽略
	//home【1bit】：                    编码时设置为0，解码时忽略
	//
	//adts_variable_header :
	//copyrighted_id_bit【1bit】：      编码时设置为0，解码时忽略
	//copyrighted_id_start【1bit】：    编码时设置为0，解码时忽略
	//aac_frame_length【13bit】：       ADTS帧长度包括ADTS长度和AAC声音数据长度的和。即 aac_frame_length = (protection_absent == 0 ? 9 : 7) + audio_data_length
	//adts_buffer_fullness【11bit】：   固定为0x7FF。表示是码率可变的码流
	//number_of_raw_data_blocks_in_frame【2bit】：表示当前帧有number_of_raw_data_blocks_in_frame + 1 个原始帧(一个AAC原始帧包含一段时间内1024个采样及相关数据) = > 00
	typedef struct
	{
		uint16_t syncword;
		uint8_t  id;
		uint8_t  layer;
		uint8_t  protection_absent;
		uint8_t  profile;
		uint8_t  sampling_frequency_index;
		uint8_t  private_bit;
		uint8_t  channel_configuration;
		uint8_t  original_copy;
		uint8_t  home;

		uint8_t  copyrighted_id_bit;
		uint8_t  copyrighted_id_start;
		uint16_t aac_frame_length;
		uint16_t adts_buffer_fullness;
		uint8_t  number_of_raw_data_blocks_in_frame;
	} AAC_ADTS_HEADER;

	typedef struct
	{
		AAC_ADTS_HEADER adtdHeader;

		unsigned char   adts[7];
		int             adtsSize;

		unsigned char*  aacEs;
		int             aacEsSize;
	} AAC_FRAME;

	//profile【2bit】：                 标识使用哪个级别的AAC 1: AAC Main 2 : AAC LC 3 : AAC SSR 4 : AAC LTP
	//sampling_frequency_index【4bit】：标识使用的采样率的下标 0x0 : 96000 0x1 : 88200 0x2 : 64000 0x3 : 48000 0x4 : 44100 0x5 : 32000 0x6 : 24000 0x7 : 22050 0x8 : 16000 ...
	//channel_configuration【3bit】：   标识声道数  1, 2
	int libos_mpeg4_aac_adts_save(int profile, int frequency, int channel, int aacEsSize, unsigned char* adtsBuf);
	int libos_mpeg4_aac_adts_parse(unsigned char* adtsBuf, int adtsBufSize, AAC_ADTS_HEADER* adtsHeader);
	
	int libos_mpeg4_aac_get_adtsframe(unsigned char* buffer, int bufSize, unsigned char* aacFrame, int* aacFrameSize);

#ifdef __cplusplus
}
#endif

#endif//!__LIBOS_AAC_PARSE_H