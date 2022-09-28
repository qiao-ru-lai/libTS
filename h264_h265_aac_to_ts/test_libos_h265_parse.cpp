#include "libos_h265_parse.h"


#if 0
int test_h265_parse_sample()
{
	int avx_success = 0;

	FILE* h264File = fopen("../../../Src/stream-0.h265", "rb+");

	char tmpBuf[100000] = { 0 };  //∑¿÷πI÷°π˝¥Û
	int  tmpBufSize = 0;
	int  tmpBufIndex = 0;

	printf("-----+---- NALU Table ------+---------+\n");
	printf(" NUM |  IDC   |  TYPE |   LEN   |\n");
	printf("-----+--------+-------+---------+\n");

	int index = 0;
	while (!feof(h264File))  //buf £”‡ ˝æ›¥¶¿Ì
	{
		int readSize = fread(tmpBuf + tmpBufIndex, 1, sizeof(tmpBuf)-tmpBufSize, h264File);
		tmpBufSize += readSize;

		H265NALU_t lpNALU_t;
		int remainSize = 0;
		avx_success = H265NALUParser(tmpBuf, tmpBufSize, &lpNALU_t, &remainSize);

		if (avx_success == 1)
		{
#if 0
			char str[20];
			sprintf_s(str, "264/all.%d", index++);
			FILE* file = fopen(str, "wb");
			fwrite((char*)lpNALU_t.buf, 1, lpNALU_t.len, file);
			fclose(file);
#endif

			memmove(tmpBuf, tmpBuf + remainSize, tmpBufSize - remainSize);
			tmpBufIndex = tmpBufSize - remainSize;
			tmpBufSize = tmpBufSize - remainSize;

			char type_str[20] = { 0 };
			switch (lpNALU_t.type)
			{
			case NAL_BLA_W_LP:   sprintf(type_str, "IDR"); break;
			case NAL_BLA_W_RADL: sprintf(type_str, "IDR"); break;
			case NAL_BLA_N_LP:   sprintf(type_str, "IDR"); break;
			case NAL_IDR_W_RADL: sprintf(type_str, "IDR"); break;
			case NAL_IDR_N_LP:   sprintf(type_str, "IDR"); break;
			case NAL_CRA_NUT:    sprintf(type_str, "IDR"); break;
			case NAL_VPS:        sprintf(type_str, "VPS"); break;
			case NAL_SPS:        sprintf(type_str, "SPS"); break;
			case NAL_PPS:        sprintf(type_str, "PPS"); break;
			}

			printf("%5d| %6s| %8d|\n", index++, type_str, lpNALU_t.len);
		}
		else
		{
			break;
		}
	}

	fclose(h264File);

	return avx_success;
}

#else
int test_h265_parse_sample()
{
	int avx_success      = 0;

	FILE* h264File       = fopen("../../../Src/stream-0.h265", "rb+");
	int   h264frameSize  = 1024 * 1024 * 2;
	char *h264frame      = (char *)malloc(h264frameSize);

	int   h264bufferSize = 1024 * 1024 * 4;
	char *h264buffer     = (char *)malloc(h264bufferSize);

	int   h264bufferOffset = 0;
	int   readSize         = 0;
	int   index            = 0;
	int   lastFrameSize    = 0;

	while (!feof(h264File)) 
	{
		readSize        = fread(h264buffer + h264bufferOffset, 1, h264bufferSize - h264bufferOffset, h264File);
		char* inputData = h264buffer;
		readSize += h264bufferOffset;

        //Ë∑üh264‰∏ÄÊ†∑ÔºåÊ≤°ÊúâÂ§ÑÁêÜÊúÄÂêé‰∏Ä‰∏™NALUÊï∞ÊçÆÔºåÁõ¥Êé•‰∏¢ÂºÉ‰∫Ü
		while (1)
		{
			int frameSize = 0;
			avx_success = H265NALUDataParser(inputData, readSize, h264frame, &frameSize);
			if (avx_success == -1) 
			{
				break;
			}else
			if (avx_success == -2) 
			{
				memcpy(h264buffer, inputData, readSize);
				h264bufferOffset = readSize;
				break;
			}

			readSize     -= frameSize;
			inputData    += frameSize;
			lastFrameSize = frameSize;

			printf("%4d %4d %8d \n", index++, avx_success, frameSize);
		}
	}

#if 1
	FILE* file = fopen("../../../Src/lastnual.h265", "wb");
	fwrite(h264frame, 1, lastFrameSize, file);
	fflush(file);
	fclose(file);
#endif

	fclose(h264File);

	return avx_success;
}

#endif