#ifndef __LIBOS_MUX_TS_H
#define __LIBOS_MUX_TS_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libos_ts_header.h"


#ifdef __cplusplus
extern "C" {
#endif

	TSFile* libos_mux_ts_open(char* filePath);

#define TYPE_AUDIO_AAC      1 //-> 0x0f
#define TYPE_VIDEO_H264     2 //-> 0x1b
#define TYPE_VIDEO_HEVC     3 //-> 0x24

	int libos_mux_ts_set_params(TSFile* lpTSFile, int audioStreamType, int videoStreamType, int pat_pmt_Interval);
	int libos_mux_ts_write(TSFile* lpTSFile, char* buf, int size, uint64_t pts, int keyFrame, int streamType);
	int libos_mux_ts_close(TSFile* lpTSFile);

	int libos_mux_ts_write_ts_header(TSFile* lpTSFile, int streamType, 
									uint16_t pid,
									uint8_t payload_uint_start_indicator, 
									uint8_t adapation_field_control);
	int libos_mux_ts_write_adaptation_header(TSFile* lpTSFile);
	int libos_mux_ts_write_pat(TSFile* lpTSFile);
	int libos_mux_ts_write_pmt(TSFile* lpTSFile);
	int libos_mux_ts_write_pes_header(TSFile* lpTSFile, int streamType, uint64_t pts, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif//!__LIBOS_MUX_TS_H