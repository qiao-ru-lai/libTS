#ifndef __LIBOS_TS_HEADER_H
#define __LIBOS_TS_HEADER_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>


#ifdef __cplusplus
extern "C" {
#endif

#define TS_PACKET_LENGTH_188  188
#define TS_PACKET_LENGTH_204  204
#define TS_MAX_LENGTH         65536

#define PAT_PID               0x0000
#define SDT_PID               0x0011

#define STREAM_TYPE_VIDEO_MPEG1     0x01
#define STREAM_TYPE_VIDEO_MPEG2     0x02
#define STREAM_TYPE_AUDIO_MPEG1     0x03
#define STREAM_TYPE_AUDIO_MPEG2     0x04
#define STREAM_TYPE_PRIVATE_SECTION 0x05
#define STREAM_TYPE_PRIVATE_DATA    0x06
#define STREAM_TYPE_AUDIO_AAC       0x0f
#define STREAM_TYPE_AUDIO_AAC_LATM  0x11
#define STREAM_TYPE_VIDEO_MPEG4     0x10
#define STREAM_TYPE_VIDEO_H264      0x1b
#define STREAM_TYPE_VIDEO_HEVC      0x24
#define STREAM_TYPE_VIDEO_CAVS      0x42
#define STREAM_TYPE_VIDEO_VC1       0xea
#define STREAM_TYPE_VIDEO_DIRAC     0xd1

#define STREAM_TYPE_AUDIO_AC3       0x81
#define STREAM_TYPE_AUDIO_DTS       0x82
#define STREAM_TYPE_AUDIO_TRUEHD    0x83

#pragma pack(1)

typedef struct
{
	unsigned int syntax_indicator;                //8b  同步字节，固定为0x47

	unsigned int transport_error_indicator;       //1b  传输错误指示符，表明在ts头的adapt域后由一个无用字节，通常都为0，这个字节算在adapt域长度内
	unsigned int payload_uint_start_indicator;    //1b  负载单元起始标示符，一个完整的数据包开始时标记为1
	unsigned int transport_pritxy;                //1b  传输优先级，0为低优先级，1为高优先级，通常取0
	unsigned int PID;                             //13b pid值

	unsigned int scrmling_control;                //2b  传输加扰控制，00表示未加密
	unsigned int adapation_field_control;         //2b  是否包含自适应区，‘00’保留；‘01’为无自适应域，仅含有效负载；‘10’为仅含自适应域，无有效负载；‘11’为同时带有自适应域和有效负载
	unsigned int continue_counter;                //4b  递增计数器，从0-f，起始值不一定取0，但必须是连续的

	uint8_t      adaptationFiledBuf[TS_PACKET_LENGTH_188];
	unsigned int adaptationFiledBufSize;

	uint8_t      payloadBuf[TS_PACKET_LENGTH_188];
	unsigned int payloadBufSize;

}TSPacket;


//更详细查阅RFC
typedef struct
{
	unsigned int adaptation_field_length;   //8b  自适应域长度，后面的字节数
	unsigned int flag;                      //8b  取0x50表示包含PCR或0x40表示不包含PCR
	unsigned int PCR;                       //5B  Program Clock Reference，节目时钟参考，用于恢复出与编码端一致的系统时序时钟STC（System Time Clock）。
	unsigned int stuffing_bytes;            //xB  填充字节，取值0xff
}AdaptationPacket;

typedef struct
{
	unsigned int program_number;     //16b
//  unsigned int reserved;           //3b
	unsigned int PMT_PID;            //13b
}PROGRAM_INFO;

typedef struct
{
	unsigned table_id : 8;                 //固定为0x00 ，标志是该表是PAT
	unsigned section_syntax_indicator : 1; //段语法标志位，固定为1
	unsigned zero : 1;                     //0
	unsigned reserved_1 : 2;               // 保留位 11
	unsigned section_length : 12;          //表示这个字节后面有用的字节数，包括CRC32
	unsigned transport_stream_id : 16;     //该传输流的ID，区别于一个网络中其它多路复用的流
	unsigned reserved_2 : 2;               // 保留位
	unsigned version_number : 5;           //范围0-31，表示PAT的版本号
	unsigned current_next_indicator : 1;   //发送的PAT是当前有效还是下一个PAT有效
	unsigned section_number : 8;           //分段的号码。PAT可能分为多段传输，第一段为00，以后每个分段加1，最多可能有256个分段
	unsigned last_section_number : 8;      //最后一个分段的号码
	std::vector<PROGRAM_INFO> program;     //
	unsigned reserved_3 : 3;               // 保留位
	unsigned network_PID : 13;             //网络信息表（NIT）的PID,节目号为0时对应的PID为network_PID
	unsigned CRC_32 : 32;                  //CRC32校验码
}PATPacket;


typedef struct 
{
	//stream_type	8b	流类型，标志是Video还是Audio还是其他数据，h.264编码对应0x1b，aac编码对应0x0f，mp3编码对应0x03
	//reserved	3b	固定为111
	//elementary_PID	13b	与stream_type对应的PID
	//reserved	4b	固定为1111
	//ES_info_length	12b	描述信息，指定为0x000表示没有

	unsigned stream_type    : 8;  //指示特定PID的节目元素包的类型。该处PID由elementary PID指定
	unsigned elementary_PID : 13; //该域指示TS包的PID值。这些TS包含有相关的节目元素
	unsigned ES_info_length : 12; //前两位bit为00。该域指示跟随其后的描述相关节目元素的byte数
	unsigned descriptor;
}PMTStream;

typedef struct
{
	unsigned table_id : 8;                   //固定为0x02, 表示PMT表
	unsigned section_syntax_indicator : 1;   //固定为0x01
	unsigned zero : 1;                       //0x01
	unsigned reserved_1 : 2;                 //0x03
	unsigned section_length : 12;            //后面数据的长度
	unsigned program_number : 16;            // 指出该节目对应于可应用的Program map PID
	unsigned reserved_2 : 2;                 //0x03
	unsigned version_number : 5;             //指出TS流中Program map section的版本号
	unsigned current_next_indicator : 1;     //当该位置1时，当前传送的Program map section可用； //当该位置0时，指示当前传送的Program map section不可用，下一个TS流的Program map section有效。
	unsigned section_number : 8;             //固定为0x00
	unsigned last_section_number : 8;        //固定为0x00
	unsigned reserved_3 : 3;                 //0x07
	unsigned PCR_PID : 13;                   //指明TS包的PID值，该TS包含有PCR域，//该PCR值对应于由节目号指定的对应节目。//如果对于私有数据流的节目定义与PCR无关，这个域的值将为0x1FFF。
	
	unsigned reserved_4 : 4;                 //预留为0x0F
	unsigned program_info_length : 12;       //前两位bit为00。该域指出跟随其后对节目信息的描述的byte数。
	
	std::vector<PMTStream> PMT_Stream;       //每个元素包含8位, 指示特定PID的节目元素包的类型。该处PID由elementary PID指定
	unsigned CRC_32 : 32;
}PMTPacket;

typedef struct
{
	uint32_t packet_start_code_prefix;   //3B	开始码，固定为0x000001
	uint8_t  stream_id;                  //1B	音频取值（0xc0 - 0xdf），通常为0xc0 视频取值（0xe0 - 0xef），通常为0xe0
	uint16_t PES_packet_length;          //2B	后面pes数据的长度，0表示长度不限制，	只有视频数据长度会超过0xffff
	uint8_t  ESCR_flag;                  //1B	通常取值0x80，表示数据不加密、无优先级、备份的数据
	uint8_t  PTS_DTS_flags;              //1B	取值0x80表示只含有pts，取值0xc0表示含有pts和dts
	uint8_t  pes_data_length;            //1B	后面数据的长度，取值5或10
	uint64_t pts;                       //时间戳
	uint64_t dts;

	uint64_t option_pes_header_length;          // h264->00 00 00 01 09 f0 每个nual的开始

	uint8_t   nualBuf[TS_PACKET_LENGTH_188];
	uint32_t  nualBufSize;
}PESPacket;


struct TSFile
{
	FILE*               fp;
	char                filePath[125];
	uint8_t             ts_packet_buf[TS_PACKET_LENGTH_188];
	int                 ts_packet_buf_index;
	uint8_t				audioStreamType;
	uint8_t				videoStreamType;
	int					pat_pmt_Interval; 
	int                 writeVideoINum;  //计算插入PAT PMT表间隔
	int                 writeAudioNum;
	uint8_t             video_continue_counter;
	uint8_t             audio_continue_counter;
	uint16_t            video_pid;
	uint16_t            audio_pid;
};


#pragma pack()

#ifdef __cplusplus
}
#endif

#endif//!__LIBOS_TS_HEADER_H