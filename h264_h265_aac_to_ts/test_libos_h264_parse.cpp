#include "libos_h264_parse.h"


#if 0
int test_h264_parse_sample()
{
	int avx_success = 0;

	FILE* h264File = fopen("../../../Src/videocaptute.h264", "rb+");

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

		NALU_t lpNALU_t;
		int remainSize = 0;
		avx_success = H264NALUParser(tmpBuf, tmpBufSize, &lpNALU_t, &remainSize);

		if (avx_success == 1)
		{
#if 1
			char str[20];
			sprintf_s(str, "264/all.%d", index++);
			FILE* file = fopen(str, "wb");
			fwrite((char*)lpNALU_t.buf, 1, lpNALU_t.len, file);
			fclose(file);
#endif

			memmove(tmpBuf, tmpBuf + remainSize, tmpBufSize - remainSize);
			tmpBufIndex = tmpBufSize - remainSize;
			tmpBufSize = tmpBufSize - remainSize;



			XXW()


			char type_str[20] = { 0 };
			switch (lpNALU_t.nal_unit_type)
			{
			case NALU_TYPE_SLICE:sprintf(type_str, "SLICE"); break;
			case NALU_TYPE_DPA:sprintf(type_str, "DPA"); break;
			case NALU_TYPE_DPB:sprintf(type_str, "DPB"); break;
			case NALU_TYPE_DPC:sprintf(type_str, "DPC"); break;
			case NALU_TYPE_IDR:sprintf(type_str, "IDR"); break;
			case NALU_TYPE_SEI:sprintf(type_str, "SEI"); break;
			case NALU_TYPE_SPS:sprintf(type_str, "SPS"); break;
			case NALU_TYPE_PPS:sprintf(type_str, "PPS"); break;
			case NALU_TYPE_AUD:sprintf(type_str, "AUD"); break;
			case NALU_TYPE_EOSEQ:sprintf(type_str, "EOSEQ"); break;
			case NALU_TYPE_EOSTREAM:sprintf(type_str, "EOSTREAM"); break;
			case NALU_TYPE_FILL:sprintf(type_str, "FILL"); break;
			}

			char idc_str[20] = { 0 };
			switch (lpNALU_t.nal_reference_idc >> 5)
			{
			case NALU_PRIORITY_DISPOSABLE:sprintf(idc_str, "DISPOS"); break;
			case NALU_PRIRITY_LOW:sprintf(idc_str, "LOW"); break;
			case NALU_PRIORITY_HIGH:sprintf(idc_str, "HIGH"); break;
			case NALU_PRIORITY_HIGHEST:sprintf(idc_str, "HIGHEST"); break;
			}

			printf("%5d| %7s| %6s| %8d|\n", index++, idc_str, type_str, lpNALU_t.len);
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
int test_h264_parse_sample()
{
	int avx_success      = 0;

	FILE* h264File       = fopen("../../../Src/sintel.h264", "rb+");
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

        //Ê≤°Â§ÑÁêÜh264Êñá‰ª∂ÊúÄÂêé‰∏Ä‰∏™NALUÊï∞ÊçÆÔºåÁõ¥Êé•‰∏¢ÂºÉ‰∫Ü
		while (1)
		{
			int frameSize = 0;
			avx_success = H264NALUDataParser(inputData, readSize, h264frame, &frameSize);
			if (avx_success == -1) 
			{
				break;
			}
			else if (avx_success == -2) 
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
	FILE* file = fopen("../../../Src/lastnual.h264", "wb");
	fwrite(h264frame, 1, lastFrameSize, file);
	fflush(file);
	fclose(file);
#endif

	fclose(h264File);

	return avx_success;
}

#endif