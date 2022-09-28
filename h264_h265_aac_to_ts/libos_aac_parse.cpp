#include "libos_aac_parse.h"

//profile【2bit】：                 标识使用哪个级别的AAC 1: AAC Main 2 : AAC LC 3 : AAC SSR 4 : AAC LTP
//sampling_frequency_index【4bit】：标识使用的采样率的下标 0x0 : 96000 0x1 : 88200 0x2 : 64000 0x3 : 48000 0x4 : 44100 0x5 : 32000 0x6 : 24000 0x7 : 22050 0x8 : 16000 ...
//channel_configuration【3bit】：   标识声道数  1, 2
int libos_mpeg4_aac_adts_save(int profile, int frequency, int channel, int aacEsSize, unsigned char* adtsBuf)
{
	int avx_success = 0;

	int audio_object_type        = profile;   //audioObjectType
	int len                      = aacEsSize + 7;
	int sampling_frequency_index = 4;   //samplingFrequencyIndex
	if (frequency == 96000) sampling_frequency_index = 0;
	if (frequency == 88200) sampling_frequency_index = 1;
	if (frequency == 64000) sampling_frequency_index = 2;
	if (frequency == 48000) sampling_frequency_index = 3;
	if (frequency == 44100) sampling_frequency_index = 4;
	if (frequency == 32000) sampling_frequency_index = 5;
	if (frequency == 24000) sampling_frequency_index = 6;
	if (frequency == 22050) sampling_frequency_index = 7;
	if (frequency == 16000) sampling_frequency_index = 8;

	adtsBuf[0] = 0xff;         //syncword:0xfff                          高8bits
	adtsBuf[1] = 0xf0;         //syncword:0xfff                          低4bits
	adtsBuf[1] |= (0 << 3);    //MPEG Version:0 for MPEG-4,1 for MPEG-2  1bit
	adtsBuf[1] |= (0 << 1);    //Layer:0                                 2bits 
	adtsBuf[1] |= 1;           //protection absent:1                     1bit

	adtsBuf[2] = (audio_object_type - 1) << 6;            //profile:audio_object_type - 1                      2bits
	adtsBuf[2] |= (sampling_frequency_index & 0x0f) << 2; //sampling frequency index:sampling_frequency_index  4bits 
	adtsBuf[2] |= (0 << 1);                               //private bit:0                                      1bit
	adtsBuf[2] |= (channel & 0x04) >> 2;                  //channel configuration:channel_config               高1bit

	adtsBuf[3] = (channel & 0x03) << 6;              //channel configuration:channel_config     低2bits
	adtsBuf[3] |= (0 << 5);                          //original：0                               1bit
	adtsBuf[3] |= (0 << 4);                          //home：0                                   1bit
	adtsBuf[3] |= (0 << 3);                          //copyright id bit：0                       1bit  
	adtsBuf[3] |= (0 << 2);                          //copyright id start：0                     1bit
	adtsBuf[3] |= ((len & 0x1800) >> 11);            //frame length：value   高2bits

	adtsBuf[4] = (uint8_t)((len & 0x7f8) >> 3);      //frame length:value    中间8bits
	adtsBuf[5] = (uint8_t)((len & 0x7) << 5);        //frame length:value    低3bits
	adtsBuf[5] |= 0x1f;                              //buffer fullness:0x7ff 高5bits
	adtsBuf[6] = 0xfc;

	avx_success = 7;
	return avx_success;
}

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

int libos_mpeg4_aac_adts_parse(unsigned char* adtsBuf, int adtsBufSize, AAC_ADTS_HEADER* adtsHeader)
{
	int avx_success = 0;

	unsigned char profile = adtsBuf[2] & 0xC0;
	profile = profile >> 6;
	adtsHeader->profile = profile + 1;
	
	unsigned char sampling_frequency_index = adtsBuf[2] & 0x3C;
	sampling_frequency_index = sampling_frequency_index >> 2;
	adtsHeader->sampling_frequency_index = sampling_frequency_index;

	adtsHeader->aac_frame_length = ((uint16_t)(adtsBuf[3] & 0x03) << 11) | ((uint16_t)adtsBuf[4] << 3) | ((uint16_t)(adtsBuf[5] >> 5) & 0x07);

	adtsHeader->channel_configuration = ((adtsBuf[2] & 0x01) << 2) | ((adtsBuf[3] >> 6) & 0x03); // 3 bits: MPEG-4 Channel Configuration 

	return avx_success;
}

int libos_mpeg4_aac_get_adtsframe(unsigned char* buffer, int bufSize, unsigned char* aacFrame, int* aacFrameSize)
{
	int size        = 0;
	int avx_success = 0;

	while (1)
	{
		if (bufSize < 7) return -1;   //no find Sync

		//Sync words
		if ((buffer[0] == 0xff) && ((buffer[1] & 0xf0) == 0xf0))
		{
			size |= ((buffer[3] & 0x03) << 11);     //high 2 bit
			size |= buffer[4] << 3;                 //middle 8 bit
			size |= ((buffer[5] & 0xe0) >> 5);      //low 3bit
			break;
		}

		--bufSize;
		++buffer;
	}

	if (bufSize < size) return 1;   //aac Es no all in inData,  inData Size is to small
		
	memcpy(aacFrame, buffer, size);
	*aacFrameSize = size;

	return avx_success;
}