#include "libos_h264_parse.h"
#include <signal.h>
#include <time.h>

#ifdef __linux__
#include <unistd.h>
#include <sys/time.h>
#ifdef __APPLE__
#include <mach/mach_time.h>
#endif
#endif

// return->-1  no  find first  sync 00 00 01 || 00 00 00 01  xxxxx
// return->-2  no  find second sync 00 00 01 || 00 00 00 01  xxxxx
// return->0   has find 
int32_t H264NALUDataParser(char* inData, int inDataSize, char* h264Frame, int* h264FrameSize)
{
	int32_t avx_success = 0;

	int firstNualStartIndex = -1;
	int firstNualCodeType   = -1;
	firstNualStartIndex = SearchFirstStartCodeIndex(inData, inDataSize, &firstNualCodeType);
	if (firstNualStartIndex < 0)
	{
		avx_success = -1;
		return avx_success;
	}

    //这里已假定数据必须是以 00 00 01 或 00 00 00 01 开头
	char* tmpData     = inData     + firstNualCodeType;
	int   tmpDataSize = inDataSize - firstNualCodeType;

	int secondNualStartIndex = 0;
	int secondNualCodeType   = 0;
	secondNualStartIndex = SearchFirstStartCodeIndex(tmpData, tmpDataSize, &secondNualCodeType);
	if (secondNualStartIndex < 0)
	{
		avx_success = -2;
		return avx_success;
	}

	int32_t len = secondNualStartIndex + firstNualCodeType - firstNualStartIndex;
	memcpy(h264Frame, inData + firstNualStartIndex, len);
	*h264FrameSize = len;

	avx_success = firstNualCodeType;
	return avx_success;
}


//00 00 00 01 27 64 00 28  AD 00 CE 80 78 02 27 E5
//9A 80 80 80 F8 00 00 03  00 08 00 00 03 01 41 89
//80 03 D0 90 00 16 E3 7F  FF E0 50 00 00 00 01 28
//EE 3C B0 00 00 01 25 47  BF FF CE 90 2F B6 60 D7
int32_t H264NALUParser(char* inData, int32_t inDataSize, NALU_t* outNALU, int32_t* inDataRemainIndex)
{
	int32_t avx_success = 0;

	int firstNualStartIndex = -1;
	int firstNualCodeType   = -1;
	firstNualStartIndex = SearchFirstStartCodeIndex(inData, inDataSize, &firstNualCodeType);
	if (firstNualStartIndex < 0) return avx_success;

	char* tmpData     = inData     + firstNualCodeType;
	int   tmpDataSize = inDataSize - firstNualCodeType;

	int secondNualStartIndex = 0;
	int secondNualCodeType   = 0;
	secondNualStartIndex = SearchFirstStartCodeIndex(tmpData, tmpDataSize, &secondNualCodeType);
	if (secondNualStartIndex < 0) return avx_success;

	outNALU->startcodeprefix_len = firstNualCodeType;
	outNALU->len = secondNualStartIndex + firstNualCodeType - firstNualStartIndex;
	outNALU->buf = (char*)malloc(outNALU->len);
	memcpy(outNALU->buf, inData + firstNualStartIndex, outNALU->len);

	outNALU->forbidden_bit     = outNALU->buf[firstNualStartIndex + firstNualCodeType] & 0x80; //1 bit
	outNALU->nal_reference_idc = outNALU->buf[firstNualStartIndex + firstNualCodeType] & 0x60; // 2 bit
	outNALU->nal_unit_type     = outNALU->buf[firstNualStartIndex + firstNualCodeType] & 0x1f;// 5 bit

	*inDataRemainIndex = outNALU->len;

	avx_success = 1;
	return avx_success;
}

int32_t H264SPSParser(char* sps, int32_t spsSize)
{
	int32_t avx_success = 0;

	return avx_success;
}
int32_t H264PPSParser(char* pps, int32_t ppsSize)
{
	int32_t avx_success = 0;

	return avx_success;
}

//0x00 00 01
int32_t FindStartCode2(char* buf)
{
	if (buf[0] != 0 || buf[1] != 0 || buf[2] != 1)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}
//0x00 00 00 01
int32_t FindStartCode3(char* buf)
{
	if (buf[0] != 0 || buf[1] != 0 || buf[2] != 0 || buf[3] != 1)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

int32_t SearchFirstStartCodeIndex(char* buf, int32_t bufSize, int32_t* code2OrCode3)
{
	int  bufIndex    = 0;
	int  avx_success = -1;

	while (bufIndex < bufSize)
	{
		if (FindStartCode2(buf + bufIndex) > 0)
		{
			//bufIndex      = bufIndex + 3;
			avx_success   = bufIndex;
			*code2OrCode3 = 3;
			break;
		}
		else if (FindStartCode3(buf + bufIndex) > 0)
		{
			//bufIndex      = bufIndex + 4;
			avx_success   = bufIndex;
			*code2OrCode3 = 4;
			break;
		}
		else
		{
			bufIndex++;
		}
	}

	return avx_success;
}

int32_t SearchFirstNualTypeIndex(char* buf, int32_t bufSize, NaluType naluType)
{
	int  bufIndex = 0;
	int  avx_success = -1;

	while (bufIndex < bufSize)
	{
		if (FindStartCode2(buf + bufIndex) > 0)
		{
			bufIndex += 3;
			char* tmpBuf = buf + bufIndex;
			if (((tmpBuf[0]) & 0x1f) == naluType)
			{
				bufIndex++;
				avx_success = bufIndex;
				break;
			}
			else
			{
				bufIndex++;
			}
		}
		else if (FindStartCode3(buf + bufIndex) > 0)
		{
			bufIndex += 4;
			char* tmpBuf = buf + bufIndex;
			if (((tmpBuf[0]) & 0x1f) == naluType)
			{
				bufIndex++;
				avx_success = bufIndex;
				break;
			}
			else
			{
				bufIndex++;
			}
		}
		else
		{
			bufIndex++;
		}
	}

	return avx_success;
}

