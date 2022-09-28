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
	unsigned int syntax_indicator;                //8b  ͬ���ֽڣ��̶�Ϊ0x47

	unsigned int transport_error_indicator;       //1b  �������ָʾ����������tsͷ��adapt�����һ�������ֽڣ�ͨ����Ϊ0������ֽ�����adapt�򳤶���
	unsigned int payload_uint_start_indicator;    //1b  ���ص�Ԫ��ʼ��ʾ����һ�����������ݰ���ʼʱ���Ϊ1
	unsigned int transport_pritxy;                //1b  �������ȼ���0Ϊ�����ȼ���1Ϊ�����ȼ���ͨ��ȡ0
	unsigned int PID;                             //13b pidֵ

	unsigned int scrmling_control;                //2b  ������ſ��ƣ�00��ʾδ����
	unsigned int adapation_field_control;         //2b  �Ƿ��������Ӧ������00����������01��Ϊ������Ӧ�򣬽�����Ч���أ���10��Ϊ��������Ӧ������Ч���أ���11��Ϊͬʱ��������Ӧ�����Ч����
	unsigned int continue_counter;                //4b  ��������������0-f����ʼֵ��һ��ȡ0����������������

	uint8_t      adaptationFiledBuf[TS_PACKET_LENGTH_188];
	unsigned int adaptationFiledBufSize;

	uint8_t      payloadBuf[TS_PACKET_LENGTH_188];
	unsigned int payloadBufSize;

}TSPacket;


//����ϸ����RFC
typedef struct
{
	unsigned int adaptation_field_length;   //8b  ����Ӧ�򳤶ȣ�������ֽ���
	unsigned int flag;                      //8b  ȡ0x50��ʾ����PCR��0x40��ʾ������PCR
	unsigned int PCR;                       //5B  Program Clock Reference����Ŀʱ�Ӳο������ڻָ���������һ�µ�ϵͳʱ��ʱ��STC��System Time Clock����
	unsigned int stuffing_bytes;            //xB  ����ֽڣ�ȡֵ0xff
}AdaptationPacket;

typedef struct
{
	unsigned int program_number;     //16b
//  unsigned int reserved;           //3b
	unsigned int PMT_PID;            //13b
}PROGRAM_INFO;

typedef struct
{
	unsigned table_id : 8;                 //�̶�Ϊ0x00 ����־�Ǹñ���PAT
	unsigned section_syntax_indicator : 1; //���﷨��־λ���̶�Ϊ1
	unsigned zero : 1;                     //0
	unsigned reserved_1 : 2;               // ����λ 11
	unsigned section_length : 12;          //��ʾ����ֽں������õ��ֽ���������CRC32
	unsigned transport_stream_id : 16;     //�ô�������ID��������һ��������������·���õ���
	unsigned reserved_2 : 2;               // ����λ
	unsigned version_number : 5;           //��Χ0-31����ʾPAT�İ汾��
	unsigned current_next_indicator : 1;   //���͵�PAT�ǵ�ǰ��Ч������һ��PAT��Ч
	unsigned section_number : 8;           //�ֶεĺ��롣PAT���ܷ�Ϊ��δ��䣬��һ��Ϊ00���Ժ�ÿ���ֶμ�1����������256���ֶ�
	unsigned last_section_number : 8;      //���һ���ֶεĺ���
	std::vector<PROGRAM_INFO> program;     //
	unsigned reserved_3 : 3;               // ����λ
	unsigned network_PID : 13;             //������Ϣ��NIT����PID,��Ŀ��Ϊ0ʱ��Ӧ��PIDΪnetwork_PID
	unsigned CRC_32 : 32;                  //CRC32У����
}PATPacket;


typedef struct 
{
	//stream_type	8b	�����ͣ���־��Video����Audio�����������ݣ�h.264�����Ӧ0x1b��aac�����Ӧ0x0f��mp3�����Ӧ0x03
	//reserved	3b	�̶�Ϊ111
	//elementary_PID	13b	��stream_type��Ӧ��PID
	//reserved	4b	�̶�Ϊ1111
	//ES_info_length	12b	������Ϣ��ָ��Ϊ0x000��ʾû��

	unsigned stream_type    : 8;  //ָʾ�ض�PID�Ľ�ĿԪ�ذ������͡��ô�PID��elementary PIDָ��
	unsigned elementary_PID : 13; //����ָʾTS����PIDֵ����ЩTS��������صĽ�ĿԪ��
	unsigned ES_info_length : 12; //ǰ��λbitΪ00������ָʾ��������������ؽ�ĿԪ�ص�byte��
	unsigned descriptor;
}PMTStream;

typedef struct
{
	unsigned table_id : 8;                   //�̶�Ϊ0x02, ��ʾPMT��
	unsigned section_syntax_indicator : 1;   //�̶�Ϊ0x01
	unsigned zero : 1;                       //0x01
	unsigned reserved_1 : 2;                 //0x03
	unsigned section_length : 12;            //�������ݵĳ���
	unsigned program_number : 16;            // ָ���ý�Ŀ��Ӧ�ڿ�Ӧ�õ�Program map PID
	unsigned reserved_2 : 2;                 //0x03
	unsigned version_number : 5;             //ָ��TS����Program map section�İ汾��
	unsigned current_next_indicator : 1;     //����λ��1ʱ����ǰ���͵�Program map section���ã� //����λ��0ʱ��ָʾ��ǰ���͵�Program map section�����ã���һ��TS����Program map section��Ч��
	unsigned section_number : 8;             //�̶�Ϊ0x00
	unsigned last_section_number : 8;        //�̶�Ϊ0x00
	unsigned reserved_3 : 3;                 //0x07
	unsigned PCR_PID : 13;                   //ָ��TS����PIDֵ����TS������PCR��//��PCRֵ��Ӧ���ɽ�Ŀ��ָ���Ķ�Ӧ��Ŀ��//�������˽���������Ľ�Ŀ������PCR�޹أ�������ֵ��Ϊ0x1FFF��
	
	unsigned reserved_4 : 4;                 //Ԥ��Ϊ0x0F
	unsigned program_info_length : 12;       //ǰ��λbitΪ00������ָ���������Խ�Ŀ��Ϣ��������byte����
	
	std::vector<PMTStream> PMT_Stream;       //ÿ��Ԫ�ذ���8λ, ָʾ�ض�PID�Ľ�ĿԪ�ذ������͡��ô�PID��elementary PIDָ��
	unsigned CRC_32 : 32;
}PMTPacket;

typedef struct
{
	uint32_t packet_start_code_prefix;   //3B	��ʼ�룬�̶�Ϊ0x000001
	uint8_t  stream_id;                  //1B	��Ƶȡֵ��0xc0 - 0xdf����ͨ��Ϊ0xc0 ��Ƶȡֵ��0xe0 - 0xef����ͨ��Ϊ0xe0
	uint16_t PES_packet_length;          //2B	����pes���ݵĳ��ȣ�0��ʾ���Ȳ����ƣ�	ֻ����Ƶ���ݳ��Ȼᳬ��0xffff
	uint8_t  ESCR_flag;                  //1B	ͨ��ȡֵ0x80����ʾ���ݲ����ܡ������ȼ������ݵ�����
	uint8_t  PTS_DTS_flags;              //1B	ȡֵ0x80��ʾֻ����pts��ȡֵ0xc0��ʾ����pts��dts
	uint8_t  pes_data_length;            //1B	�������ݵĳ��ȣ�ȡֵ5��10
	uint64_t pts;                       //ʱ���
	uint64_t dts;

	uint64_t option_pes_header_length;          // h264->00 00 00 01 09 f0 ÿ��nual�Ŀ�ʼ

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
	int                 writeVideoINum;  //�������PAT PMT����
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