#ifndef __LIBOS_DEMUX_TS_H
#define __LIBOS_DEMUX_TS_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libos_ts_header.h"

//https://www.pianshen.com/article/247756474/
//https://blog.csdn.net/leek5533/article/details/104993932/
//https://blog.csdn.net/yexiangcsdn/article/details/79862434
//https://github.com/Vashonisonly/TS_Parse
//https://github.com/Zi-lu/StreamResolver
//https://github.com/justdan96/tsMuxer
//http://www.360doc.com/content/21/1130/17/474846_1006572168.shtml


#ifdef __cplusplus
extern "C" {
#endif

	int libos_parse_ts_TSPacket(uint8_t* buf, int bufSize, TSPacket& tsPacket);
	int libos_parse_ts_AdaptationPacket(uint8_t* buf, int bufSize, AdaptationPacket& adaptationPacket);
	int libos_parse_ts_PESPacket(TSPacket& tsPacket, PESPacket& pesPacket, int streamType);

	int libos_parse_ts_PATPacket(uint8_t* buf, int bufSize, PATPacket& patPacket);
	int libos_parse_ts_PMTPacket(uint8_t* buf, int bufSize, PMTPacket& pmtPacket);

#ifdef __cplusplus
}
#endif

#endif//!__LIBOS_DEMUX_TS_H