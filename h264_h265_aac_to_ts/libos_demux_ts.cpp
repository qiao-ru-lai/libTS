#include "libos_demux_ts.h"
#include "libos_common.h"


int libos_parse_ts_TSPacket(uint8_t* buf, int bufSize, TSPacket& tsPacket)
{
	int avx_success = 0;

	tsPacket.syntax_indicator             =  buf[0];
	tsPacket.transport_error_indicator    = (buf[1] & 0x80) >> 7;
	tsPacket.payload_uint_start_indicator = (buf[1] & 0x40) >> 6;
	tsPacket.transport_pritxy             = (buf[1] & 0x20) >> 5;
	tsPacket.PID                          = (buf[1] & 0x1F) << 8 | buf[2];
	tsPacket.scrmling_control             = (buf[3] & 0xC0) >> 6;
	tsPacket.adapation_field_control      = (buf[3] & 0x30) >> 4;
	tsPacket.continue_counter             =  buf[3] & 0x0F;

	//unsigned int adapation_field_control;         //2b  是否包含自适应区，‘00’保留；‘
	//01’为无自适应域，仅含有效负载；‘10’为仅含自适应域，无有效负载；‘11’为同时带有自适应域和有效负载
	if (tsPacket.adapation_field_control == 1)
	{
		tsPacket.adaptationFiledBufSize = 0;

		tsPacket.payloadBufSize = bufSize - 4;
		memcpy(tsPacket.payloadBuf, buf + 4, tsPacket.payloadBufSize);
	}

	if (tsPacket.adapation_field_control == 2)
	{
		tsPacket.adaptationFiledBufSize = bufSize - 4;
		memcpy(tsPacket.adaptationFiledBuf, buf + 4, tsPacket.adaptationFiledBufSize);

		tsPacket.payloadBufSize = 0;
	}

	if (tsPacket.adapation_field_control == 3)
	{
		int adaptation_field_length = buf[4];
		tsPacket.adaptationFiledBufSize = adaptation_field_length + 1;
		memcpy(tsPacket.adaptationFiledBuf, buf + 4, tsPacket.adaptationFiledBufSize);

		tsPacket.payloadBufSize = bufSize - 4 - tsPacket.adaptationFiledBufSize;
		memcpy(tsPacket.payloadBuf, buf + 4 + tsPacket.adaptationFiledBufSize, tsPacket.payloadBufSize);
	}

	return avx_success;
}
int libos_parse_ts_AdaptationPacket(uint8_t* buf, int bufSize, AdaptationPacket& adaptationPacket)
{
	int avx_success = 0;

	adaptationPacket.adaptation_field_length = buf[0];

	if (buf[1] == 0x50) adaptationPacket.flag = 1;
	if (buf[1] == 0x40) adaptationPacket.flag = 0;

	//to do...

	return avx_success;
}
int libos_parse_ts_PESPacket(TSPacket& tsPacket, PESPacket& pesPacket, int streamType)
{
	int avx_success = 0;

	//00 00 01 
	//E0
	//00 00
	//80 
	//80 取值0x80表示只含有pts，取值0xc0表示含有pts和dts
	//05
	//21 00 07 D8 61   pts

	//00 00 01
	//C0 
	//0A 30
	//80 
	//80
	//05 21 00 0B A1 85 

	uint8_t* buf = tsPacket.payloadBuf;
	int bufSize  = tsPacket.payloadBufSize;

#if 0
	int ss = bufSize;
	if (bufSize > 20) ss = 20;
	printf("pes buf:");
	for (size_t i = 0; i < ss; i++)
	{
		printf(" %.2X", buf[i]);
	}
	printf("\n");
#endif

	if (tsPacket.payload_uint_start_indicator == 0)
	{
		pesPacket.nualBufSize = bufSize;
		memcpy(pesPacket.nualBuf, buf, pesPacket.nualBufSize);

		return avx_success;
	}

	if (buf[0] != 0x00 || buf[1] != 0x00 || buf[2] != 0x01)
	{
		return -1;
	}

	pesPacket.packet_start_code_prefix = ((buf[0] & 0x0000FF) << 16 | (buf[1] & 0x0000FF) << 8 | (buf[2] & 0x000000FF));
	pesPacket.stream_id                =  buf[3];
	pesPacket.PES_packet_length        = (buf[4] & 0x00FF) << 8 | (buf[5] & 0x00FF);
	pesPacket.ESCR_flag                =  buf[6];
	pesPacket.PTS_DTS_flags            =  buf[7]; //取值0x80表示只含有pts，取值0xc0表示含有pts和dts
	pesPacket.pes_data_length          =  buf[8]; //后面数据的长度，取值5或10

	if (pesPacket.PTS_DTS_flags == 0x80 || 
		pesPacket.PTS_DTS_flags == 0xc0)
	{
		pesPacket.pts =  ((buf[9] & 0x0e) << 29);
		pesPacket.pts +=  (buf[9 + 1] << 22);
		pesPacket.pts += ((buf[9 + 2] & 0xfe) << 14);
		pesPacket.pts +=  (buf[9 + 3] << 7);
		pesPacket.pts +=  (buf[9 + 4] >> 1) & 0x7f;
	}

	if (streamType == STREAM_TYPE_AUDIO_AAC)
	{
		printf("==:%lld\n", pesPacket.pts);
	}
	else
	{
		printf("----:%lld\n", pesPacket.pts);
	}
	
	pesPacket.option_pes_header_length = 0;
	if (tsPacket.payload_uint_start_indicator == 1)
	{
		if (streamType == STREAM_TYPE_VIDEO_H264)
		{
			//h264->00 00 00 01 09 f0 每个nual的开始
			if (buf[9  + pesPacket.pes_data_length] == 0x00 && 
				buf[10 + pesPacket.pes_data_length] == 0x00 && 
				buf[11 + pesPacket.pes_data_length] == 0x00 &&
				buf[12 + pesPacket.pes_data_length] == 0x01 &&
				buf[13 + pesPacket.pes_data_length] == 0x09 && 
				buf[14 + pesPacket.pes_data_length] == 0xf0)
			{
				pesPacket.option_pes_header_length = 6;
			}
		}
		if (streamType == STREAM_TYPE_VIDEO_HEVC)
		{
			//h265->00 00 00 01 46 01 50 
			if (buf[9 +  pesPacket.pes_data_length] == 0x00 && 
				buf[10 + pesPacket.pes_data_length] == 0x00 && 
				buf[11 + pesPacket.pes_data_length] == 0x00 && 
				buf[12 + pesPacket.pes_data_length] == 0x01 &&
				buf[13 + pesPacket.pes_data_length] == 0x46 && 
				buf[14 + pesPacket.pes_data_length] == 0x01 && 
				buf[15 + pesPacket.pes_data_length] == 0x50)
			{
				pesPacket.option_pes_header_length = 7;
			}
		}
	}

	pesPacket.nualBufSize = bufSize - (9 + pesPacket.pes_data_length + pesPacket.option_pes_header_length);
	memcpy(pesPacket.nualBuf, buf + 9 + pesPacket.pes_data_length + pesPacket.option_pes_header_length, pesPacket.nualBufSize);

	return avx_success;
}

int libos_parse_ts_PATPacket(uint8_t* buf, int bufSize, PATPacket& patPacket)
{
	int avx_success = 0;

#if 0
	printf("pat buf:");
	for (size_t i = 0; i < bufSize; i++)
	{
		printf(" %.2X", buf[i]);
	}
	printf("\n");
#endif

	int pos = buf[0];
	if (pos == 0) pos += 1;
	uint8_t* tmpBuf = buf + pos;
	//00 00 B0 0D 00 01 C1 00 00 00 01 F0 00 2A B1 04 B2 
	//表示此TS包的内容为PSI信息表格的PAT表格数据，在4字节的TS包头之后的第一个字节的Point_field = 0x00, 
	//表示偏移量为0，即紧随其后的即为PAT的数据信息

	patPacket.table_id                 = tmpBuf[0];
	patPacket.section_syntax_indicator = tmpBuf[1] >> 7;
	patPacket.zero                     = tmpBuf[1] >> 6 & 0x1;
	patPacket.reserved_1               = tmpBuf[1] >> 4 & 0x3;
	patPacket.section_length           = (tmpBuf[1] & 0x0F) << 8 | tmpBuf[2];
	patPacket.transport_stream_id      = tmpBuf[3] << 8 | tmpBuf[4];
	patPacket.reserved_2               = tmpBuf[5] >> 6;
	patPacket.version_number           = tmpBuf[5] >> 1 & 0x1F;
	patPacket.current_next_indicator   = (tmpBuf[5] << 7) >> 7;
	patPacket.section_number           = tmpBuf[6];
	patPacket.last_section_number      = tmpBuf[7];

	int len = 0;
	len = 3 + patPacket.section_length;
	patPacket.CRC_32 = (tmpBuf[len - 4] & 0x000000FF) << 24
					 | (tmpBuf[len - 3] & 0x000000FF) << 16
					 | (tmpBuf[len - 2] & 0x000000FF) << 8
					 | (tmpBuf[len - 1] & 0x000000FF);

	int n = 0;
	for (n = 0; n < patPacket.section_length - 12; n += 4)
	{
		unsigned  program_num = tmpBuf[8 + n] << 8 | tmpBuf[9 + n];
		patPacket.reserved_3 = tmpBuf[10 + n] >> 5;
		patPacket.network_PID = 0x00;
		if (program_num == 0x00)
		{
			patPacket.network_PID = (tmpBuf[10 + n] & 0x1F) << 8 | tmpBuf[11 + n];
		}
		else
		{
			PROGRAM_INFO PAT_program;
			PAT_program.PMT_PID        = (tmpBuf[10 + n] & 0x1F) << 8 | tmpBuf[11 + n];
			PAT_program.program_number = program_num;
			patPacket.program.push_back(PAT_program);
		}
	}

	return avx_success;
}
int libos_parse_ts_PMTPacket(uint8_t* buf, int bufSize, PMTPacket& pmtPacket)
{
	int avx_success = 0;

#if 0
	printf("pmt buf:");
	for (size_t i = 0; i < bufSize; i++)
	{
		printf(" %.2X", buf[i]);
	}
	printf("\n");
#endif

	int seek = buf[0];
	if (seek == 0) seek += 1;
	uint8_t* tmpBuf = buf + seek;

	pmtPacket.table_id                 = tmpBuf[0];
	pmtPacket.section_syntax_indicator = tmpBuf[1] >> 7;
	pmtPacket.zero                     = tmpBuf[1] >> 6 & 0x01;
	pmtPacket.reserved_1               = tmpBuf[1] >> 4 & 0x03;
	pmtPacket.section_length           = (tmpBuf[1] & 0x0F) << 8 | tmpBuf[2];
	pmtPacket.program_number           = tmpBuf[3] << 8 | tmpBuf[4];
	pmtPacket.reserved_2               = tmpBuf[5] >> 6;
	pmtPacket.version_number           = tmpBuf[5] >> 1 & 0x1F;
	pmtPacket.current_next_indicator   = (tmpBuf[5] << 7) >> 7;
	pmtPacket.section_number           = tmpBuf[6];
	pmtPacket.last_section_number      = tmpBuf[7];
	pmtPacket.reserved_3               = tmpBuf[8] >> 5;
	pmtPacket.PCR_PID                  = ((tmpBuf[8] << 8) | tmpBuf[9]) & 0x1FFF;
	pmtPacket.reserved_4               = tmpBuf[10] >> 4;
	pmtPacket.program_info_length      = (tmpBuf[10] & 0x0F) << 8 | tmpBuf[11];

	// Get CRC_32
	int len = 0;
	len = pmtPacket.section_length + 3;
	pmtPacket.CRC_32 = (tmpBuf[len - 4] & 0x000000FF) << 24
					 | (tmpBuf[len - 3] & 0x000000FF) << 16
					 | (tmpBuf[len - 2] & 0x000000FF) << 8
					 | (tmpBuf[len - 1] & 0x000000FF);

	int pos = 12;
	// program info descriptor
	if (pmtPacket.program_info_length != 0)
		pos += pmtPacket.program_info_length;
	// Get stream type and PID    
	for (; pos <= (pmtPacket.section_length + 2) - 4;)
	{
		PMTStream pmt_stream;
		pmt_stream.stream_type    =  tmpBuf[pos];
		//pmtPacket.reserved_5      =  tmpBuf[pos + 1] >> 5;
		pmt_stream.elementary_PID = ((tmpBuf[pos + 1] << 8) | tmpBuf[pos + 2]) & 0x1FFF;
		//pmtPacket.reserved_6      =  tmpBuf[pos + 3] >> 4;
		pmt_stream.ES_info_length = (tmpBuf[pos + 3] & 0x0F) << 8 | tmpBuf[pos + 4];

		pmt_stream.descriptor = 0x00;
		if (pmt_stream.ES_info_length != 0)
		{
			pmt_stream.descriptor = tmpBuf[pos + 5];

			for (int len = 2; len <= pmt_stream.ES_info_length; len++)
			{
				pmt_stream.descriptor = pmt_stream.descriptor << 8 | tmpBuf[pos + 4 + len];
			}
			pos += pmt_stream.ES_info_length;
		}
		pos += 5;
		pmtPacket.PMT_Stream.push_back(pmt_stream);
	}

	return avx_success;
}
