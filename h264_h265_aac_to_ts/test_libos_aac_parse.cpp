#include "libos_aac_parse.h"

int test_aac_parse_sample()
{
	int avx_success = 0;

	FILE *aacFile = fopen("../../../Src/parseFlvAudio.aac", "rb");
	unsigned char *aacframe  = (unsigned char *)malloc(1024 * 5);
	unsigned char *aacbuffer = (unsigned char *)malloc(1024 * 1024);
	int            aacbufferOffset = 0;
	int            readSize        = 0;
	int            index           = 0;

	printf("-----+- Sam ADTS Frame Table -+------+\n");
	printf(" NUM | Profile | Frequency| Size |\n");
	printf("-----+---------+----------+------+\n");

	while (!feof(aacFile))
	{
		readSize = fread(aacbuffer + aacbufferOffset, 1, 1024 * 1024 - aacbufferOffset, aacFile);
		unsigned char* inputData = aacbuffer;
		readSize += aacbufferOffset;

        //也是没处理最后一个数据帧，直接丢弃了
		while (1)
		{
			int aacframeSize = 0;
			avx_success = libos_mpeg4_aac_get_adtsframe(inputData, readSize, aacframe, &aacframeSize);
			if (avx_success == -1)
			{
				break;
			}
			else if (avx_success == 1)
			{
				memcpy(aacbuffer, inputData, readSize);
				aacbufferOffset = readSize;
				break;
			}

			AAC_ADTS_HEADER adtsHeader;
			avx_success = libos_mpeg4_aac_adts_parse(aacframe, 7, &adtsHeader);
#if 1
			char profile_str[10] = { 0 };
			switch (adtsHeader.profile)
			{
				//1 : AAC Main 2 : AAC LC 3 : AAC SSR 4 : AAC LTP
			case 1: sprintf(profile_str, "Main"); break;
			case 2: sprintf(profile_str, "LC"); break;
			case 3: sprintf(profile_str, "SSR"); break;
			default:sprintf(profile_str, "unknown"); break;
			}

			char frequence_str[10] = { 0 };
			switch (adtsHeader.sampling_frequency_index)
			{
			case 0: sprintf(frequence_str, "96000Hz"); break;
			case 1: sprintf(frequence_str, "88200Hz"); break;
			case 2: sprintf(frequence_str, "64000Hz"); break;
			case 3: sprintf(frequence_str, "48000Hz"); break;
			case 4: sprintf(frequence_str, "44100Hz"); break;
			case 5: sprintf(frequence_str, "32000Hz"); break;
			case 6: sprintf(frequence_str, "24000Hz"); break;
			case 7: sprintf(frequence_str, "22050Hz"); break;
			case 8: sprintf(frequence_str, "16000Hz"); break;
			case 9: sprintf(frequence_str, "12000Hz"); break;
			case 10: sprintf(frequence_str, "11025Hz"); break;
			case 11: sprintf(frequence_str, "8000Hz"); break;
			default:sprintf(frequence_str, "unknown"); break;
			}
#endif
			printf("%5d| %8s|  %8s| %5d|\n", index, profile_str, frequence_str, aacframeSize);

			readSize   -= aacframeSize;
			inputData  += aacframeSize;
			index++;
		}

	}
	fclose(aacFile);
	free(aacbuffer);
	free(aacframe);

	return avx_success;
}
