#include "libos_h265_parse.h"
#include <signal.h>
#include <time.h>

#ifdef __linux__
#include <unistd.h>
#include <sys/time.h>
#ifdef __APPLE__
#include <mach/mach_time.h>
#endif
#endif

// return->-1  no  find first  sync 00 00 01 || 00 00 00 01
// return->-2  no  find second sync 00 00 01 || 00 00 00 01
// return->0   has find 
int32_t H265NALUDataParser(char* inData, int inDataSize, char* h264Frame, int* h264FrameSize)
{
	int32_t avx_success = 0;

	int firstNualStartIndex = -1;
	int firstNualCodeType   = -1;
	firstNualStartIndex = H265SearchFirstStartCodeIndex(inData, inDataSize, &firstNualCodeType);
	if (firstNualStartIndex < 0)
	{
		avx_success = -1;
		return avx_success;
	}

	char* tmpData     = inData     + firstNualCodeType;
	int   tmpDataSize = inDataSize - firstNualCodeType;

	int secondNualStartIndex = 0;
	int secondNualCodeType   = 0;
	secondNualStartIndex = H265SearchFirstStartCodeIndex(tmpData, tmpDataSize, &secondNualCodeType);
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

int32_t H265NALUParser(char* inData, int32_t inDataSize, H265NALU_t* outNALU, int32_t* inDataRemainIndex)
{
	int32_t avx_success = 0;

	int firstNualStartIndex = -1;
	int firstNualCodeType   = -1;
	firstNualStartIndex = H265SearchFirstStartCodeIndex(inData, inDataSize, &firstNualCodeType);
	if (firstNualStartIndex < 0) return avx_success;

	char* tmpData     = inData     + firstNualCodeType;
	int   tmpDataSize = inDataSize - firstNualCodeType;

	int secondNualStartIndex = 0;
	int secondNualCodeType   = 0;
	secondNualStartIndex = H265SearchFirstStartCodeIndex(tmpData, tmpDataSize, &secondNualCodeType);
	if (secondNualStartIndex < 0) return avx_success;

	outNALU->forbidden = firstNualCodeType;
	outNALU->len       = secondNualStartIndex + firstNualCodeType - firstNualStartIndex;
	outNALU->buf       = (char*)malloc(outNALU->len);
	memcpy(outNALU->buf, inData + firstNualStartIndex, outNALU->len);

	outNALU->forbidden = outNALU->buf[firstNualStartIndex + firstNualCodeType] & 0x80; 
	outNALU->type     = (outNALU->buf[firstNualStartIndex + firstNualCodeType] & 0x7E) >> 1;

	*inDataRemainIndex = outNALU->len;

	avx_success = 1;
	return avx_success;
}

//0x00 00 01
int32_t H265FindStartCode2(char* buf)
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
int32_t H265FindStartCode3(char* buf)
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
int32_t H265SearchFirstStartCodeIndex(char* buf, int32_t bufSize, int32_t* code2OrCode3)
{
	int  bufIndex    = 0;
	int  avx_success = -1;

	while (bufIndex < bufSize)
	{
		if (H265FindStartCode2(buf + bufIndex) > 0)
		{
			//bufIndex      = bufIndex + 3;
			avx_success   = bufIndex;
			*code2OrCode3 = 3;
			break;
		}
		else if (H265FindStartCode3(buf + bufIndex) > 0)
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

