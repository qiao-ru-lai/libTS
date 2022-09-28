#include "libos_demux_ts.h"

#ifdef WIN32
#else
#endif


int test_libos_demux_ts_sample()
{
	int avx_success = 0;

	FILE* file = fopen("../../../Src/264_ts.ts", "rb+");
	FILE* videofile = fopen("../../../Src/ts_video.ts", "wb");
	FILE* audiofile = fopen("../../../Src/ts_audio.ts", "wb");

	uint8_t readBuf[TS_PACKET_LENGTH_188] = { 0 };
	int     readBufSize   = 0;

	int  PMT_PID           = -1;
	int  VIDEO_PID         = -1;
	int  VIDEO_STREAM_TYPE = -1;
	int  AUDIO_PID         = -1;
	int  AUDIO_STREAM_TYPE = -1;

	while (!feof(file))  
	{
		readBufSize = fread(readBuf, 1, sizeof(readBuf), file);
		if (readBufSize < TS_PACKET_LENGTH_188) continue;

#if 0
		printf("read buf:");
		for (size_t i = 0; i < readBufSize; i++)
		{
			printf(" %.2X", readBuf[i]);
		}
		printf("\n");
#endif

		TSPacket lpTSPacket;
		avx_success = libos_parse_ts_TSPacket(readBuf, readBufSize, lpTSPacket);

		if (lpTSPacket.adaptationFiledBufSize > 0)
		{
#if 0
			printf("adaptation:");
			for (size_t i = 0; i < lpTSPacket.adaptationFiledBufSize; i++)
			{
				printf(" %.2X", lpTSPacket.adaptationFiledBuf[i]);
			}
			printf("\n");
#endif

			AdaptationPacket lpAdaptationPacket;
			avx_success = libos_parse_ts_AdaptationPacket(lpTSPacket.adaptationFiledBuf, lpTSPacket.adaptationFiledBufSize, lpAdaptationPacket);
		}

		if (lpTSPacket.payloadBufSize > 0)
		{
#if 0
			printf("payload buf:");
			for (size_t i = 0; i < lpTSPacket.payloadBufSize; i++)
			{
				printf(" %.2X", lpTSPacket.payloadBuf[i]);
			}
			printf("\n");
#endif
		}

		switch (lpTSPacket.PID)
		{
		case SDT_PID:
			break;
		case PAT_PID:
			if (1)
			{
				PATPacket lpPATPacket;
				avx_success = libos_parse_ts_PATPacket(lpTSPacket.payloadBuf, lpTSPacket.payloadBufSize, lpPATPacket);

				for (size_t i = 0; i < lpPATPacket.program.size(); i++)
				{
					printf("PMT_PID = %d, %d\n", lpPATPacket.program[i].program_number, lpPATPacket.program[i].PMT_PID);
					PMT_PID = lpPATPacket.program[i].PMT_PID;
				}
			}
			break;
		default:
			break;
		}

		if (lpTSPacket.PID == PMT_PID)
		{
			PMTPacket lpPMTPacket;
			avx_success = libos_parse_ts_PMTPacket(lpTSPacket.payloadBuf, lpTSPacket.payloadBufSize, lpPMTPacket);

			for (size_t i = 0; i < lpPMTPacket.PMT_Stream.size(); i++)
			{
				switch (lpPMTPacket.PMT_Stream[i].stream_type)
				{
				case STREAM_TYPE_AUDIO_AAC:
					AUDIO_STREAM_TYPE = STREAM_TYPE_AUDIO_AAC;
					AUDIO_PID = lpPMTPacket.PMT_Stream[i].elementary_PID;
					break;
				case STREAM_TYPE_AUDIO_AAC_LATM:
					AUDIO_STREAM_TYPE = STREAM_TYPE_AUDIO_AAC_LATM;
					AUDIO_PID = lpPMTPacket.PMT_Stream[i].elementary_PID;
					break;
				case STREAM_TYPE_VIDEO_H264:
					VIDEO_STREAM_TYPE = STREAM_TYPE_VIDEO_H264;
					VIDEO_PID = lpPMTPacket.PMT_Stream[i].elementary_PID;
					break;
				case STREAM_TYPE_VIDEO_HEVC:
					VIDEO_STREAM_TYPE = STREAM_TYPE_VIDEO_HEVC;
					VIDEO_PID = lpPMTPacket.PMT_Stream[i].elementary_PID;
					break;
				default:
					break;
				}
				printf("STREAM_PID = %d, %d\n", lpPMTPacket.PMT_Stream[i].stream_type, lpPMTPacket.PMT_Stream[i].elementary_PID);
			}
		}
		if (lpTSPacket.PID == VIDEO_PID)
		{
			PESPacket lpPESPacket;
			avx_success = libos_parse_ts_PESPacket(lpTSPacket, lpPESPacket, VIDEO_STREAM_TYPE);

			fwrite(lpPESPacket.nualBuf, 1, lpPESPacket.nualBufSize, videofile);
		}
		if (lpTSPacket.PID == AUDIO_PID)
		{
			PESPacket lpPESPacket;
			avx_success = libos_parse_ts_PESPacket(lpTSPacket, lpPESPacket, AUDIO_STREAM_TYPE);

			fwrite(lpPESPacket.nualBuf, 1, lpPESPacket.nualBufSize, audiofile);
		}
	}

	fclose(file);
	fclose(videofile);
	fclose(audiofile);
	return avx_success;
}
