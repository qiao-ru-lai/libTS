#include "libos_mux_ts.h"
#include "libos_common.h"


TSFile* libos_mux_ts_open(char* filePath)
{
	int avx_success = 0;

	TSFile* lpTSFile = nullptr;
	if (filePath == nullptr) return lpTSFile;

	FILE* file = fopen(filePath, "wb+");
	if (file == nullptr) return lpTSFile;

	lpTSFile = (TSFile*)malloc(sizeof(TSFile));
	memset(lpTSFile, 0, sizeof(TSFile));
	strcpy(lpTSFile->filePath, filePath);
	lpTSFile->fp = file;

	return lpTSFile;

}
int libos_mux_ts_set_params(TSFile* lpTSFile, int audioStreamType, int videoStreamType, int pat_pmt_Interval)
{
	int avx_success = 0;

	if (audioStreamType == TYPE_AUDIO_AAC)  lpTSFile->audioStreamType = 0x0F;
	if (videoStreamType == TYPE_VIDEO_H264) lpTSFile->videoStreamType = 0x1b;
	if (videoStreamType == TYPE_VIDEO_HEVC) lpTSFile->videoStreamType = 0x24;

	lpTSFile->pat_pmt_Interval = pat_pmt_Interval;
	if (lpTSFile->videoStreamType > 0 && lpTSFile->audioStreamType > 0)
	{
		lpTSFile->video_pid = 0x100;
		lpTSFile->audio_pid = 0x101;
	}
	else if (lpTSFile->videoStreamType > 0)
	{
		lpTSFile->video_pid = 0x100;
	}
	else if (lpTSFile->audioStreamType > 0)
	{
		lpTSFile->audio_pid = 0x100;
	}

	//pat 
	uint16_t pat_pid                     = 0;
	uint8_t payload_uint_start_indicator = 1;
	uint8_t adapation_field_control      = 1;
	avx_success = libos_mux_ts_write_ts_header(lpTSFile, 0x00, pat_pid, payload_uint_start_indicator, adapation_field_control);
	avx_success = libos_mux_ts_write_pat(lpTSFile);
	memset(lpTSFile->ts_packet_buf + lpTSFile->ts_packet_buf_index, 0xFF, TS_PACKET_LENGTH_188 - lpTSFile->ts_packet_buf_index);
	fwrite(lpTSFile->ts_packet_buf, 1, TS_PACKET_LENGTH_188, lpTSFile->fp);

	//pmt
	uint16_t pmt_pid = 0x1000;
	avx_success = libos_mux_ts_write_ts_header(lpTSFile, 0x00, pmt_pid, payload_uint_start_indicator, adapation_field_control);
	avx_success = libos_mux_ts_write_pmt(lpTSFile);
	memset(lpTSFile->ts_packet_buf + lpTSFile->ts_packet_buf_index, 0xFF, TS_PACKET_LENGTH_188 - lpTSFile->ts_packet_buf_index);
	fwrite(lpTSFile->ts_packet_buf, 1, TS_PACKET_LENGTH_188, lpTSFile->fp);

	return avx_success;
}
int libos_mux_ts_write(TSFile* lpTSFile, char* buf, int size, uint64_t pts, int keyFrame, int streamType)
{
	int avx_success = 0;
	
	uint16_t pid = 0;
	switch (streamType)
	{
	case TYPE_AUDIO_AAC:
		pid = lpTSFile->audio_pid;
		break;
	case TYPE_VIDEO_H264:
		pid = lpTSFile->video_pid;
		break;
	case TYPE_VIDEO_HEVC:
		pid = lpTSFile->video_pid;
		break;
	default:
		break;
	}

	if (lpTSFile->videoStreamType > 0)
	{
		if (lpTSFile->writeVideoINum > 0 && keyFrame > 0)
		{
			lpTSFile->writeVideoINum++;

			//pat 
			uint16_t pat_pid = 0;
			uint8_t payload_uint_start_indicator = 1;
			uint8_t adapation_field_control = 1;
			avx_success = libos_mux_ts_write_ts_header(lpTSFile, 0x00, pat_pid, payload_uint_start_indicator, adapation_field_control);
			avx_success = libos_mux_ts_write_pat(lpTSFile);
			memset(lpTSFile->ts_packet_buf + lpTSFile->ts_packet_buf_index, 0xFF, TS_PACKET_LENGTH_188 - lpTSFile->ts_packet_buf_index);
			fwrite(lpTSFile->ts_packet_buf, 1, TS_PACKET_LENGTH_188, lpTSFile->fp);

			//pmt
			uint16_t pmt_pid = 0x1000;
			avx_success = libos_mux_ts_write_ts_header(lpTSFile, 0x00, pmt_pid, payload_uint_start_indicator, adapation_field_control);
			avx_success = libos_mux_ts_write_pmt(lpTSFile);
			memset(lpTSFile->ts_packet_buf + lpTSFile->ts_packet_buf_index, 0xFF, TS_PACKET_LENGTH_188 - lpTSFile->ts_packet_buf_index);
			fwrite(lpTSFile->ts_packet_buf, 1, TS_PACKET_LENGTH_188, lpTSFile->fp);
		}
	}
	else if (lpTSFile->audioStreamType > 0)
	{
		if (lpTSFile->writeAudioNum > 0 && ((lpTSFile->writeAudioNum++) % 50) == 0) //
		{
			//pat 
			uint16_t pat_pid = 0;
			uint8_t payload_uint_start_indicator = 1;
			uint8_t adapation_field_control = 1;
			avx_success = libos_mux_ts_write_ts_header(lpTSFile, 0x00, pat_pid, payload_uint_start_indicator, adapation_field_control);
			avx_success = libos_mux_ts_write_pat(lpTSFile);
			memset(lpTSFile->ts_packet_buf + lpTSFile->ts_packet_buf_index, 0xFF, TS_PACKET_LENGTH_188 - lpTSFile->ts_packet_buf_index);
			fwrite(lpTSFile->ts_packet_buf, 1, TS_PACKET_LENGTH_188, lpTSFile->fp);

			//pmt
			uint16_t pmt_pid = 0x1000;
			avx_success = libos_mux_ts_write_ts_header(lpTSFile, 0x00, pmt_pid, payload_uint_start_indicator, adapation_field_control);
			avx_success = libos_mux_ts_write_pmt(lpTSFile);
			memset(lpTSFile->ts_packet_buf + lpTSFile->ts_packet_buf_index, 0xFF, TS_PACKET_LENGTH_188 - lpTSFile->ts_packet_buf_index);
			fwrite(lpTSFile->ts_packet_buf, 1, TS_PACKET_LENGTH_188, lpTSFile->fp);
		}
	}

	int bufIndex                         = 0;
	uint8_t payload_uint_start_indicator = 1;
	uint8_t adapation_field_control      = 3;
	avx_success = libos_mux_ts_write_ts_header(lpTSFile, streamType, pid, payload_uint_start_indicator, adapation_field_control);
	int ts_adapt_header_index = lpTSFile->ts_packet_buf_index;
	avx_success = libos_mux_ts_write_adaptation_header(lpTSFile);
	int ts_pes_header_index = lpTSFile->ts_packet_buf_index;
	avx_success = libos_mux_ts_write_pes_header(lpTSFile, streamType, pts, size);

	if (TS_PACKET_LENGTH_188 - lpTSFile->ts_packet_buf_index > size)
	{
		//printf("==========================\n");

#if 0
		printf("buf:");
		for (size_t i = 0; i < lpTSFile->ts_packet_buf_index; i++)
		{
			printf(" %.2X", lpTSFile->ts_packet_buf[i]);
		}
		printf("\n");
#endif

#if 0
		uint8_t tmpBuf[TS_PACKET_LENGTH_188] = { 0 };
		memcpy(tmpBuf, (void*)buf, size);
		printf("buf:");
		for (size_t i = 0; i < size; i++)
		{
			printf(" %.2X", tmpBuf[i]);
		}
		printf("\n");
#endif

		//adapt数据保存
		uint8_t adapBuf[TS_PACKET_LENGTH_188] = { 0 };
		int    adapBufSize = ts_pes_header_index - ts_adapt_header_index;  //带第一个字节
		memcpy(adapBuf, lpTSFile->ts_packet_buf + ts_adapt_header_index, adapBufSize);

#if 0
		printf("adapt buf:");
		for (size_t i = 0; i < adapBufSize; i++)
		{
			printf(" %.2X", adapBuf[i]);
		}
		printf("\n");
#endif

		//pes数据保存
		uint8_t pesBuf[TS_PACKET_LENGTH_188] = { 0 };
		int    pesBufSize = lpTSFile->ts_packet_buf_index - ts_pes_header_index;
		memcpy(pesBuf, lpTSFile->ts_packet_buf + ts_pes_header_index, pesBufSize);

#if 0
		printf("pes buf:");
		for (size_t i = 0; i < pesBufSize; i++)
		{
			printf(" %.2X", pesBuf[i]);
		}
		printf("\n");
#endif

		//adaption填充
		int stuffing_bytes = TS_PACKET_LENGTH_188 - lpTSFile->ts_packet_buf_index - size;
		int adapt_bytes = adapBufSize + stuffing_bytes;
		memcpy(lpTSFile->ts_packet_buf + ts_adapt_header_index, adapBuf, adapBufSize);
		memset(lpTSFile->ts_packet_buf + ts_adapt_header_index, adapt_bytes - 1, 1);
		memset(lpTSFile->ts_packet_buf + (ts_adapt_header_index + adapBufSize), 0xFF, stuffing_bytes);

		//pes填充
		int pes_index = ts_adapt_header_index + adapt_bytes;
		memcpy(lpTSFile->ts_packet_buf + pes_index, pesBuf, pesBufSize);

		//plyload填充
		int payload_index = pes_index + pesBufSize;
		memcpy(lpTSFile->ts_packet_buf + payload_index, buf, size);
		fwrite(lpTSFile->ts_packet_buf, 1, TS_PACKET_LENGTH_188, lpTSFile->fp);

#if 0
		printf("buf:");
		for (size_t i = 0; i < TS_PACKET_LENGTH_188; i++)
		{
			printf(" %.2X", lpTSFile->ts_packet_buf[i]);
		}
		printf("\n");
#endif

	}
	else
	{
		//first ts
		memcpy(lpTSFile->ts_packet_buf + lpTSFile->ts_packet_buf_index, buf + bufIndex, TS_PACKET_LENGTH_188 - lpTSFile->ts_packet_buf_index);
		bufIndex += (TS_PACKET_LENGTH_188 - lpTSFile->ts_packet_buf_index);
		fwrite(lpTSFile->ts_packet_buf, 1, TS_PACKET_LENGTH_188, lpTSFile->fp);

		int remainSize = size - bufIndex;
		int num = remainSize / (TS_PACKET_LENGTH_188 - 4);
		int lastTsPacketSize = remainSize - (num * (TS_PACKET_LENGTH_188 - 4));

		for (size_t i = 0; i < num; i++)
		{
			payload_uint_start_indicator = 0;
			adapation_field_control      = 1;
			avx_success = libos_mux_ts_write_ts_header(lpTSFile, streamType, pid, payload_uint_start_indicator, adapation_field_control);

			memcpy(lpTSFile->ts_packet_buf + lpTSFile->ts_packet_buf_index, buf + bufIndex, TS_PACKET_LENGTH_188 - lpTSFile->ts_packet_buf_index);
			bufIndex += (TS_PACKET_LENGTH_188 - lpTSFile->ts_packet_buf_index);
			fwrite(lpTSFile->ts_packet_buf, 1, TS_PACKET_LENGTH_188, lpTSFile->fp);
		}

		if (lastTsPacketSize > 0)
		{
			if (lastTsPacketSize + 4 > TS_PACKET_LENGTH_188)
			{
				printf("---------------------\n");
			}
			else
			{
				//adaptation: 1D 00  -> 29
				//FF FF FF FF FF FF FF FF 
				//FF FF FF FF FF FF FF FF 
				//FF FF FF FF FF FF FF FF 
				//FF FF FF FF
				payload_uint_start_indicator = 0;
				adapation_field_control      = 1;

				int adaptSize = TS_PACKET_LENGTH_188 - lastTsPacketSize - 4;
				if (adaptSize > 0) adapation_field_control = 3;
				avx_success = libos_mux_ts_write_ts_header(lpTSFile, streamType, pid, payload_uint_start_indicator, adapation_field_control);
				//printf("***************************:%d\n", adaptSize);

				if (adaptSize == 1)
				{
					adaptSize = 2;  

					//4 + 2 + 183->189 超出
					//ts->4 + adapt->2 + pes->182->188
					lpTSFile->ts_packet_buf[lpTSFile->ts_packet_buf_index++] = adaptSize - 1;
					lpTSFile->ts_packet_buf[lpTSFile->ts_packet_buf_index++] = 0x00;

					memset(lpTSFile->ts_packet_buf + lpTSFile->ts_packet_buf_index, 0xFF, adaptSize - 2);
					lpTSFile->ts_packet_buf_index += (adaptSize - 2);

					int copyPlayloadSize = lastTsPacketSize - 1;
					memcpy(lpTSFile->ts_packet_buf + lpTSFile->ts_packet_buf_index, buf + bufIndex, copyPlayloadSize);
					bufIndex += copyPlayloadSize;
					fwrite(lpTSFile->ts_packet_buf, 1, TS_PACKET_LENGTH_188, lpTSFile->fp);

					//buf数据剩余一个字节
					//ts->4 + adapt->XX + pes->1->188
					payload_uint_start_indicator = 0;
					adapation_field_control      = 3;
					adaptSize = TS_PACKET_LENGTH_188 - 4 - 1;
					if (adaptSize > 0) adapation_field_control = 3;
					avx_success = libos_mux_ts_write_ts_header(lpTSFile, streamType, pid, payload_uint_start_indicator, adapation_field_control);

					lpTSFile->ts_packet_buf[lpTSFile->ts_packet_buf_index++] = adaptSize - 1;
					lpTSFile->ts_packet_buf[lpTSFile->ts_packet_buf_index++] = 0x00;
					memset(lpTSFile->ts_packet_buf + lpTSFile->ts_packet_buf_index, 0xFF, adaptSize - 2);
					lpTSFile->ts_packet_buf_index += (adaptSize - 2);

					copyPlayloadSize = 1;
					memcpy(lpTSFile->ts_packet_buf + lpTSFile->ts_packet_buf_index, buf + bufIndex, copyPlayloadSize);
					bufIndex += copyPlayloadSize;
					fwrite(lpTSFile->ts_packet_buf, 1, TS_PACKET_LENGTH_188, lpTSFile->fp);
				}
				else
				if (adaptSize > 1) 
				{
					lpTSFile->ts_packet_buf[lpTSFile->ts_packet_buf_index++] = adaptSize - 1;
					lpTSFile->ts_packet_buf[lpTSFile->ts_packet_buf_index++] = 0x00;
					memset(lpTSFile->ts_packet_buf + lpTSFile->ts_packet_buf_index, 0xFF, adaptSize - 2);
					lpTSFile->ts_packet_buf_index += (adaptSize - 2);

					memcpy(lpTSFile->ts_packet_buf + lpTSFile->ts_packet_buf_index, buf + bufIndex, TS_PACKET_LENGTH_188 - lpTSFile->ts_packet_buf_index);
					bufIndex += (TS_PACKET_LENGTH_188 - lpTSFile->ts_packet_buf_index);
					fwrite(lpTSFile->ts_packet_buf, 1, TS_PACKET_LENGTH_188, lpTSFile->fp);
				}
				else
				{
					
				}

#if 0
				printf("buf:");
				for (size_t i = 0; i < TS_PACKET_LENGTH_188; i++)
				{
					printf(" %.2X", lpTSFile->ts_packet_buf[i]);
				}
				printf("\n");
#endif
			}
		}
	}

	return avx_success;
}
int libos_mux_ts_close(TSFile* lpTSFile)
{
	int avx_success = 0;

	fclose(lpTSFile->fp);

	return avx_success;
}

//47 40 00 10 PAT
//47 50 00 10 PMT
//47 41 00 30 first  ts
//47 01 00 11 second ts
//47 01 00 12 third  ts
int libos_mux_ts_write_ts_header(TSFile* lpTSFile, int streamType, uint16_t pid, uint8_t payload_uint_start_indicator, uint8_t adapation_field_control)
{
	int avx_success = 0;

	uint8_t continue_counter = 0;
	if (pid != 0 && pid != 4096)//非PAT PMT
	{
		switch (streamType)
		{
		case TYPE_AUDIO_AAC:
			continue_counter = lpTSFile->audio_continue_counter;
			lpTSFile->audio_continue_counter++;
			break;
		case TYPE_VIDEO_H264:
			continue_counter = lpTSFile->video_continue_counter;
			lpTSFile->video_continue_counter++;
			break;
		case TYPE_VIDEO_HEVC:
			continue_counter = lpTSFile->video_continue_counter;
			lpTSFile->video_continue_counter++;
			break;
		default:
			break;
		}
	}

	int index = 0;
	uint8_t buf[TS_PACKET_LENGTH_188] = { 0 };
	
	buf[index++] = 0x47;
	avx_success = CheckBigEndlen();
	if (avx_success > 0)
	{
		// pid 256->0100 大端小端   
		buf[index++] = (payload_uint_start_indicator & 0x01) << 6 | (pid & 0x1F);
		buf[index++] = pid << 8;
	}
	else
	{
		// 0001
		buf[index++] = (payload_uint_start_indicator & 0x01) << 6 | (pid >> 8 & 0x1F);
		buf[index++] = pid;
	}

	//00 01 0000
	buf[index++] = (adapation_field_control & 0x03) << 4 | continue_counter & 0x0F;
	
	//111000
	//00 01 0001

#if 0
	printf("ts header:");
	for (size_t i = 0; i < index; i++)
	{
		printf(" %.2X", buf[i]);
	}
	printf("\n");
#endif

#if 1
	lpTSFile->ts_packet_buf_index = 0;
	memset(lpTSFile->ts_packet_buf, 0, TS_PACKET_LENGTH_188);

	memcpy(lpTSFile->ts_packet_buf + lpTSFile->ts_packet_buf_index, buf, index);
	lpTSFile->ts_packet_buf_index += index;
#endif
	
	avx_success = index;
	return avx_success;
}
int libos_mux_ts_write_adaptation_header(TSFile* lpTSFile)
{
	int avx_success = 0;

	int index = 0;
	uint8_t buf[TS_PACKET_LENGTH_188] = { 0xFF };

#if 1
	buf[index++] = 0x07; //后续可能会更新
	buf[index++] = 0x50; //取0x50表示包含PCR或0x40表示不包含PCR

	buf[index++] = 0x00;
	buf[index++] = 0x00;
	buf[index++] = 0x7B;
	buf[index++] = 0x0C;
	buf[index++] = 0x7E;
	buf[index++] = 0x00;
#else
	buf[index++] = 0x01; //后续可能会更新
	buf[index++] = 0x40; 
#endif

//  int stuffing_bytes; //xB  填充字节，取值0xff

#if 1
	memcpy(lpTSFile->ts_packet_buf + lpTSFile->ts_packet_buf_index, buf, index);
	lpTSFile->ts_packet_buf_index += index;
#endif

	avx_success = index;
	return avx_success;
}
int libos_mux_ts_write_pat(TSFile* lpTSFile)
{
	int avx_success = 0;

	uint16_t section_length = 13; //
	
	int index = 0;
	uint8_t buf[TS_PACKET_LENGTH_188] = { 0 };

	buf[index++] = 0x00; //偏移
	buf[index++] = 0x00;

	avx_success = CheckBigEndlen();
	if (avx_success > 0)
	{
		buf[index++] = (((0x01 & 0x01) << 7) & 0x30) | (section_length & 0x0F);
		buf[index++] = section_length << 8;
	}
	else
	{
		buf[index++] = ((0x01 & 0x01) << 7) | 0x30 | (section_length >> 8 & 0x0F);
		buf[index++] = section_length;
	}

	buf[index++] = 0x00;
	buf[index++] = 0x01;
	buf[index++] = 0xC1;

	buf[index++] = 0x00;
	buf[index++] = 0x00;

	buf[index++] = 0x00;
	buf[index++] = 0x01;
	buf[index++] = 0xF0;
	buf[index++] = 0x00;

	//CRC32
	AVX_WB32(buf + index, calc_crc32(buf + 1, index - 1));
	index += 4;

#if 1
	memcpy(lpTSFile->ts_packet_buf + lpTSFile->ts_packet_buf_index, buf, index);
	lpTSFile->ts_packet_buf_index += index;
#endif

#if 0
	printf("pat:");
	for (size_t i = 0; i < lpTSFile->ts_packet_buf_index; i++)
	{
		printf(" %.2X", lpTSFile->ts_packet_buf[i]);
	}
	printf("\n");
#endif

	avx_success = index;
	return avx_success;
}
int libos_mux_ts_write_pmt(TSFile* lpTSFile)
{
	int avx_success = 0;

	uint16_t section_length = 18; 
	if (lpTSFile->audioStreamType > 0 && lpTSFile->videoStreamType > 0)
	{
		section_length = 18 + 5;
	}

	int index = 0;
	uint8_t buf[TS_PACKET_LENGTH_188] = { 0 };

	buf[index++] = 0x00;//偏移
	buf[index++] = 0x02;
	
	avx_success = CheckBigEndlen();
	if (avx_success > 0)
	{
		buf[index++] = (((0x01 & 0x01) << 7) & 0x30) | (section_length & 0x0F);
		buf[index++] = section_length << 8;
	}
	else
	{
		buf[index++] = ((0x01 & 0x01) << 7) | 0x30 | (section_length >> 8 & 0x0F);
		buf[index++] = section_length;
	}

	buf[index++] = 0x00;
	buf[index++] = 0x01;

	buf[index++] = 0xC1;

	buf[index++] = 0x00;
	buf[index++] = 0x00;

	buf[index++] = 0xE1; //256
	buf[index++] = 0x00;

	buf[index++] = 0xF0;
	buf[index++] = 0x00;

	if (lpTSFile->audioStreamType > 0 && lpTSFile->videoStreamType > 0)
	{
		//STREAM_TYPE_VIDEO_H264 STREAM_TYPE_VIDEO_HEVC
		buf[index++] = lpTSFile->videoStreamType;
		buf[index++] = 0xE1;
		buf[index++] = 0x00;
		buf[index++] = 0xF0;
		buf[index++] = 0x00;

		buf[index++] = lpTSFile->audioStreamType;
		buf[index++] = 0xE1;
		buf[index++] = 0x01;
		buf[index++] = 0xF0;
		buf[index++] = 0x00;
	}
	else if (lpTSFile->audioStreamType > 0)
	{
		buf[index++] = lpTSFile->audioStreamType;
		buf[index++] = 0xE1;
		buf[index++] = 0x00;
		buf[index++] = 0xF0;
		buf[index++] = 0x00;
	}else if (lpTSFile->videoStreamType > 0)
	{
		//STREAM_TYPE_VIDEO_H264 STREAM_TYPE_VIDEO_HEVC
		buf[index++] = lpTSFile->videoStreamType; 
		buf[index++] = 0xE1;
		buf[index++] = 0x00;
		buf[index++] = 0xF0;
		buf[index++] = 0x00;
	}

	//CRC32
	AVX_WB32(buf + index, calc_crc32(buf + 1, index - 1));
	index += 4;

#if 1
	memcpy(lpTSFile->ts_packet_buf + lpTSFile->ts_packet_buf_index, buf, index);
	lpTSFile->ts_packet_buf_index += index;
#endif

#if 0
	printf("pmt:");
	for (size_t i = 0; i < lpTSFile->ts_packet_buf_index; i++)
	{
		printf(" %.2X", lpTSFile->ts_packet_buf[i]);
	}
	printf("\n");
#endif

	avx_success = index;
	return avx_success;
}
int libos_mux_ts_write_pes_header(TSFile* lpTSFile, int streamType, uint64_t pts, uint16_t size)
{
	int avx_success = 0;

	int index = 0;
	uint8_t buf[TS_PACKET_LENGTH_188] = { 0 };

	buf[index++] = 0x00;
	buf[index++] = 0x00;
	buf[index++] = 0x01;

	uint16_t pid = 0;
	switch (streamType)
	{
	case TYPE_AUDIO_AAC:
		buf[index++] = 0xc0; //audio
		//buf[index++] = 0x00; //   aac+8
		//buf[index++] = 0x00; //
		AVX_WB16(buf + index, size + 8);
		index += 2;
		break;
	case TYPE_VIDEO_H264:
	case TYPE_VIDEO_HEVC:
		buf[index++] = 0xe0; //video
		buf[index++] = 0x00;
		buf[index++] = 0x00;
		break;
	default:
		break;
	}

	buf[index++] = 0x80;
	buf[index++] = 0x80; //取值0x80表示只含有pts，取值0xc0表示含有pts和dts
	buf[index++] = 0x05;

#if 1 //big litten
	buf[index++] = 0x20 + (uint8_t)(((pts >> 30) & 0x07) << 1) + 1;
	uint16_t val = (uint16_t)(((pts >> 15) & 0x7fff) << 1) + 1;
	buf[index++] = val >> 8;
	buf[index++] = val & 0xff;

	val = (uint16_t)((pts & 0x7fff) << 1) + 1;
	buf[index++] = val >> 8;
	buf[index++] = val & 0xff;
#endif

	switch (streamType)
	{
	case TYPE_AUDIO_AAC:
		break;
	case TYPE_VIDEO_H264:
		buf[index++] = 0x00;
		buf[index++] = 0x00;
		buf[index++] = 0x00;
		buf[index++] = 0x01;
		buf[index++] = 0x09;
		buf[index++] = 0xF0;
		break;
	case TYPE_VIDEO_HEVC:
		buf[index++] = 0x00;
		buf[index++] = 0x00;
		buf[index++] = 0x00;
		buf[index++] = 0x01;
		buf[index++] = 0x46;
		buf[index++] = 0x01;
		buf[index++] = 0x50;
		break;
	default:
		break;
	}

#if 1
	memcpy(lpTSFile->ts_packet_buf + lpTSFile->ts_packet_buf_index, buf, index);
	lpTSFile->ts_packet_buf_index += index;
#endif

	avx_success = index;
	return avx_success;
}