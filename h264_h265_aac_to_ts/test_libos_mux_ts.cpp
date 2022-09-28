#include "libos_mux_ts.h"
#include "libos_h264_parse.h"
#include "libos_h265_parse.h"
#include "libos_aac_parse.h"
#include <thread>
#include <mutex>
#include <map>

#ifdef WIN32
#include <windows.h>
#include <process.h>
#include <mmsystem.h>
#define snprintf sprintf_s
#else
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#ifdef __APPLE__
#include <mach/mach_time.h>
#endif
#endif

#define WRITE_TS 1

#define H264_TS       0
#define H265_TS       0
#define AAC_TS        0
#define H264_AAC_TS   1
#define H265_AAC_TS   0

static int libos_usleep(int usec)
{
#ifdef WIN32
	return SleepEx(usec / 1000, FALSE);
#else
	return usleep(usec);
#endif

	return -1;
}

typedef struct
{
	void*   kData;
	int32_t kSize;
	int64_t kPts;           //时间戳
	int32_t kStreamIndex; //0->audio 1->video
	int32_t kFlags;       //  >0 keyFrame
}PacketBuffer;

std::map<int64_t, PacketBuffer *> kAudioJitterWindow;//key：时间戳，value：视频数据
std::mutex                        kAudioLock;
std::map<int64_t, PacketBuffer *> kVideoJitterWindow;//key：时间戳，value：视频数据
std::mutex                        kVideoLock;

static void videoThreadMethod()
{
	int avx_success = 0;

	FILE* h264File  = fopen("../../../Src/264_ts.h264", "rb+");

	int   h264frameSize = 1024 * 1024 * 2;
	char *h264frame = (char *)malloc(h264frameSize);

	int   h264bufferSize = 1024 * 1024 * 4;
	char *h264buffer = (char *)malloc(h264bufferSize);

	int   h264bufferOffset = 0;
	int   readSize = 0;
	int   index = 0;
	int   lastFrameSize = 0;

	char *  buf = (char *)malloc(h264bufferSize);
	int     bufIndex = 0;

	uint64_t pts = 1;

	while (!feof(h264File))
	{
		readSize = fread(h264buffer + h264bufferOffset, 1, h264bufferSize - h264bufferOffset, h264File);
		char* inputData = h264buffer;
		readSize += h264bufferOffset;

		while (1)
		{
			int frameSize = 0;
			avx_success = H264NALUDataParser(inputData, readSize, h264frame, &frameSize);
			if (avx_success == -1)
			{
				break;
			}
			else
			if (avx_success == -2)
			{
				memcpy(h264buffer, inputData, readSize);
				h264bufferOffset = readSize;
				break;
			}

#if WRITE_TS
			if ((h264frame[0] == 0x00 && h264frame[1] == 0x00 && h264frame[2] == 0x01))
			{
				if ((h264frame[3] & 0x1f) == 7 || (h264frame[3] & 0x1f) == 8)
				{
					memcpy(buf + bufIndex, h264frame, frameSize);
					bufIndex += frameSize;
				}
				else
				if ((h264frame[3] & 0x1f) == 5)
				{
					memcpy(buf + bufIndex, h264frame, frameSize);
					bufIndex += frameSize;

					if (1)
					{
						std::lock_guard<std::mutex> _lock(kVideoLock);

						PacketBuffer* lpPacketBuffer = new PacketBuffer();
						lpPacketBuffer->kPts         = pts;
						lpPacketBuffer->kStreamIndex = 0;
						lpPacketBuffer->kSize = bufIndex;
						lpPacketBuffer->kData = malloc(lpPacketBuffer->kSize);
						memcpy(lpPacketBuffer->kData, buf, lpPacketBuffer->kSize);
						kVideoJitterWindow.insert(std::map<int64_t, PacketBuffer *>::value_type(pts, lpPacketBuffer));

						pts += 3600;
					}

					bufIndex = 0;
				}
				else
				{
					if (1)
					{
						std::lock_guard<std::mutex> _lock(kVideoLock);

						PacketBuffer* lpPacketBuffer = new PacketBuffer();
						lpPacketBuffer->kPts         = pts;
						lpPacketBuffer->kStreamIndex = 0;
						lpPacketBuffer->kSize = frameSize;
						lpPacketBuffer->kData = malloc(lpPacketBuffer->kSize);
						memcpy(lpPacketBuffer->kData, h264frame, lpPacketBuffer->kSize);
						kVideoJitterWindow.insert(std::map<int64_t, PacketBuffer *>::value_type(pts, lpPacketBuffer));

						pts += 3600;
					}
				}
			}
			else if ((h264frame[0] == 0x00 && h264frame[1] == 0x00 && h264frame[2] == 0x00 && h264frame[3] == 0x01))
			{
				if ((h264frame[4] & 0x1f) == 7 || (h264frame[4] & 0x1f) == 8)
				{
					memcpy(buf + bufIndex, h264frame, frameSize);
					bufIndex += frameSize;
				}
				else
				if ((h264frame[4] & 0x1f) == 5)
				{
					memcpy(buf + bufIndex, h264frame, frameSize);
					bufIndex += frameSize;

					if (1)
					{
						std::lock_guard<std::mutex> _lock(kVideoLock);

						PacketBuffer* lpPacketBuffer = new PacketBuffer();
						lpPacketBuffer->kPts         = pts;
						lpPacketBuffer->kStreamIndex = 0;
						lpPacketBuffer->kSize = bufIndex;
						lpPacketBuffer->kData = malloc(lpPacketBuffer->kSize);
						memcpy(lpPacketBuffer->kData, buf, lpPacketBuffer->kSize);
						kVideoJitterWindow.insert(std::map<int64_t, PacketBuffer *>::value_type(pts, lpPacketBuffer));

						pts += 3600;
					}

					bufIndex = 0;
				}
				else
				{
					if (1)
					{
						std::lock_guard<std::mutex> _lock(kVideoLock);

						PacketBuffer* lpPacketBuffer = new PacketBuffer();
						lpPacketBuffer->kPts         = pts;
						lpPacketBuffer->kStreamIndex = 0;
						lpPacketBuffer->kSize = frameSize;
						lpPacketBuffer->kData = malloc(lpPacketBuffer->kSize);
						memcpy(lpPacketBuffer->kData, h264frame, lpPacketBuffer->kSize);
						kVideoJitterWindow.insert(std::map<int64_t, PacketBuffer *>::value_type(pts, lpPacketBuffer));

						pts += 3600;
					}
				}
			}
#endif

			readSize -= frameSize;
			inputData += frameSize;
			lastFrameSize = frameSize;
		}

		libos_usleep(10 * 1000);
	}

	fclose(h264File);
}
static void videoh265ThreadMethod()
{
	int avx_success = 0;

	FILE* h264File = fopen("../../../Src/265_ts.h265", "rb+");
	int   h264frameSize = 1024 * 1024 * 2;
	char *h264frame = (char *)malloc(h264frameSize);

	int   h264bufferSize = 1024 * 1024 * 4;
	char *h264buffer = (char *)malloc(h264bufferSize);

	int   h264bufferOffset = 0;
	int   readSize = 0;
	int   index = 0;
	int   lastFrameSize = 0;

	char *  buf = (char *)malloc(h264bufferSize);
	int     bufIndex = 0;
	uint64_t pts = 1;

	while (!feof(h264File))
	{
		readSize = fread(h264buffer + h264bufferOffset, 1, h264bufferSize - h264bufferOffset, h264File);
		char* inputData = h264buffer;
		readSize += h264bufferOffset;

		while (1)
		{
			int frameSize = 0;
			avx_success = H265NALUDataParser(inputData, readSize, h264frame, &frameSize);
			if (avx_success == -1)
			{
				break;
			}
			else
			if (avx_success == -2)
			{
				memcpy(h264buffer, inputData, readSize);
				h264bufferOffset = readSize;
				break;
			}

#if WRITE_TS
			if ((h264frame[0] == 0x00 && h264frame[1] == 0x00 && h264frame[2] == 0x01))
			{
				int type = (h264frame[3] & 0x7F) >> 1;

				if (type == 32 || type == 33 || type == 34)
				{
					memcpy(buf + bufIndex, h264frame, frameSize);
					bufIndex += frameSize;
				}
				else
				if (type == 19)
				{
					memcpy(buf + bufIndex, h264frame, frameSize);
					bufIndex += frameSize;

					if (1)
					{
						std::lock_guard<std::mutex> _lock(kVideoLock);

						PacketBuffer* lpPacketBuffer = new PacketBuffer();
						lpPacketBuffer->kPts = pts;
						lpPacketBuffer->kStreamIndex = 0;
						lpPacketBuffer->kSize = bufIndex;
						lpPacketBuffer->kData = malloc(lpPacketBuffer->kSize);
						memcpy(lpPacketBuffer->kData, buf, lpPacketBuffer->kSize);
						kVideoJitterWindow.insert(std::map<int64_t, PacketBuffer *>::value_type(pts, lpPacketBuffer));

						pts += 3600;
					}

					bufIndex = 0;
				}
				else
				{
					if (1)
					{
						std::lock_guard<std::mutex> _lock(kVideoLock);

						PacketBuffer* lpPacketBuffer = new PacketBuffer();
						lpPacketBuffer->kPts = pts;
						lpPacketBuffer->kStreamIndex = 0;
						lpPacketBuffer->kSize = frameSize;
						lpPacketBuffer->kData = malloc(lpPacketBuffer->kSize);
						memcpy(lpPacketBuffer->kData, h264frame, lpPacketBuffer->kSize);
						kVideoJitterWindow.insert(std::map<int64_t, PacketBuffer *>::value_type(pts, lpPacketBuffer));

						pts += 3600;
					}
				}
			}
			else if ((h264frame[0] == 0x00 && h264frame[1] == 0x00 && h264frame[2] == 0x00 && h264frame[3] == 0x01))
			{
				int type = (h264frame[4] & 0x7F) >> 1;

				if (type == 32 || type == 33 || type == 34)
				{
					memcpy(buf + bufIndex, h264frame, frameSize);
					bufIndex += frameSize;
				}
				else
				if (type == 19)
				{
					memcpy(buf + bufIndex, h264frame, frameSize);
					bufIndex += frameSize;

					if (1)
					{
						std::lock_guard<std::mutex> _lock(kVideoLock);

						PacketBuffer* lpPacketBuffer = new PacketBuffer();
						lpPacketBuffer->kPts = pts;
						lpPacketBuffer->kStreamIndex = 0;
						lpPacketBuffer->kSize = bufIndex;
						lpPacketBuffer->kData = malloc(lpPacketBuffer->kSize);
						memcpy(lpPacketBuffer->kData, buf, lpPacketBuffer->kSize);
						kVideoJitterWindow.insert(std::map<int64_t, PacketBuffer *>::value_type(pts, lpPacketBuffer));

						pts += 3600;
					}

					bufIndex = 0;
				}
				else
				{
					if (1)
					{
						std::lock_guard<std::mutex> _lock(kVideoLock);

						PacketBuffer* lpPacketBuffer = new PacketBuffer();
						lpPacketBuffer->kPts = pts;
						lpPacketBuffer->kStreamIndex = 0;
						lpPacketBuffer->kSize = frameSize;
						lpPacketBuffer->kData = malloc(lpPacketBuffer->kSize);
						memcpy(lpPacketBuffer->kData, h264frame, lpPacketBuffer->kSize);
						kVideoJitterWindow.insert(std::map<int64_t, PacketBuffer *>::value_type(pts, lpPacketBuffer));

						pts += 3600;
					}
				}
			}
#endif

			readSize -= frameSize;
			inputData += frameSize;
			lastFrameSize = frameSize;

			//printf("%4d %4d %8d \n", index++, avx_success, frameSize);
		}
	}

	fclose(h264File);
}
static void audioThreadMethod()
{
	int avx_success = 0;

	FILE *aacFile = fopen("../../../Src/aac_ts.aac", "rb");
	unsigned char *aacframe  = (unsigned char *)malloc(1024 * 5);
	unsigned char *aacbuffer = (unsigned char *)malloc(1024 * 1024);

	int            aacbufferOffset = 0;
	int            readSize        = 0;
	uint64_t pts   = 2;

	while (!feof(aacFile))
	{
		readSize = fread(aacbuffer + aacbufferOffset, 1, 1024 * 1024 - aacbufferOffset, aacFile);
		unsigned char* inputData = aacbuffer;
		readSize += aacbufferOffset;

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

			if(1)
			{
				std::lock_guard<std::mutex> _lock(kAudioLock);

				PacketBuffer* lpPacketBuffer = new PacketBuffer();
				lpPacketBuffer->kPts         = pts;
				lpPacketBuffer->kStreamIndex = 0;
				lpPacketBuffer->kSize = aacframeSize;
				lpPacketBuffer->kData = malloc(lpPacketBuffer->kSize);
				memcpy(lpPacketBuffer->kData, aacframe, lpPacketBuffer->kSize);
				kAudioJitterWindow.insert(std::map<int64_t, PacketBuffer *>::value_type(pts, lpPacketBuffer));

				double cr      = (1024) * 1.0 / 44100;
				uint64_t pts_1 = cr * 1000 * 1000;
				uint64_t pts_9 = pts_1 * 90000 / 1000000;
				pts += pts_9;
			}

			readSize -= aacframeSize;
			inputData += aacframeSize;
		}

		libos_usleep(10 * 1000);
	}

	fclose(aacFile);
	free(aacbuffer);
	free(aacframe);
}
static void muxingThreadMethod()
{
	int avx_success = 0;

	TSFile* lpTSFile = libos_mux_ts_open("../../../Src/muxingTS.ts");

	int audioStreamType = TYPE_AUDIO_AAC;
	int videoStreamType = TYPE_VIDEO_H264;
#if H264_AAC_TS
	videoStreamType = TYPE_VIDEO_H264;
#endif
#if H265_AAC_TS
	videoStreamType = TYPE_VIDEO_HEVC;
#endif
	
	avx_success = libos_mux_ts_set_params(lpTSFile, audioStreamType, videoStreamType, 40);

	int64_t lastAudioPts = 0;
	int64_t lastVideoPts = 0;

	while (true)
	{
		if (lastVideoPts <= 0)
		{
			std::lock_guard<std::mutex> _lock(kVideoLock);

			std::map<int64_t, PacketBuffer *>::iterator   jitterHead = kVideoJitterWindow.begin();
			std::map<int64_t, PacketBuffer *>::iterator   jitterTail = kVideoJitterWindow.end();
			if (jitterHead != jitterTail)
			{
				PacketBuffer *jitterData = jitterHead->second;

				avx_success = libos_mux_ts_write(lpTSFile, (char *)jitterData->kData, jitterData->kSize, jitterData->kPts, 1, videoStreamType);
				lastVideoPts = jitterData->kPts;
				free(jitterData->kData);
				delete jitterData;

				kVideoJitterWindow.erase(jitterHead);
			}
		}

		if (lastVideoPts <= 0)
		{
			libos_usleep(1 * 1000); continue;
		}

		//获取 audio video, 哪个pts小就放哪个
		int64_t nowAudioPts = 0;
		int64_t nowVideoPts = 0;
		if (1)
		{
			std::lock_guard<std::mutex> _lock(kVideoLock);

			std::map<int64_t, PacketBuffer *>::iterator   jitterHead = kVideoJitterWindow.begin();
			std::map<int64_t, PacketBuffer *>::iterator   jitterTail = kVideoJitterWindow.end();
			if (jitterHead != jitterTail)
			{
				PacketBuffer *jitterData = jitterHead->second;
				nowVideoPts = jitterData->kPts;
			}
		}
		if (1)
		{
			std::lock_guard<std::mutex> _lock(kAudioLock);

			std::map<int64_t, PacketBuffer *>::iterator   jitterHead = kAudioJitterWindow.begin();
			std::map<int64_t, PacketBuffer *>::iterator   jitterTail = kAudioJitterWindow.end();
			if (jitterHead != jitterTail)
			{
				PacketBuffer *jitterData = jitterHead->second;
				nowAudioPts = jitterData->kPts;
			}
		}

		printf("=====:%lld, %lld\n", nowAudioPts, nowVideoPts);

		if (nowAudioPts > 0 && nowVideoPts > 0)
		{
			if (nowAudioPts > nowVideoPts)
			{
				std::lock_guard<std::mutex> _lock(kVideoLock);

				std::map<int64_t, PacketBuffer *>::iterator   jitterHead = kVideoJitterWindow.begin();
				std::map<int64_t, PacketBuffer *>::iterator   jitterTail = kVideoJitterWindow.end();
				if (jitterHead != jitterTail)
				{
					PacketBuffer *jitterData = jitterHead->second;
					avx_success = libos_mux_ts_write(lpTSFile, (char *)jitterData->kData, jitterData->kSize, jitterData->kPts, 1, videoStreamType);

					free(jitterData->kData);
					delete jitterData;

					kVideoJitterWindow.erase(jitterHead);
				}
			}
			else
			{
				std::lock_guard<std::mutex> _lock(kAudioLock);

				std::map<int64_t, PacketBuffer *>::iterator   jitterHead = kAudioJitterWindow.begin();
				std::map<int64_t, PacketBuffer *>::iterator   jitterTail = kAudioJitterWindow.end();
				if (jitterHead != jitterTail)
				{
					PacketBuffer *jitterData = jitterHead->second;
					avx_success = libos_mux_ts_write(lpTSFile, (char *)jitterData->kData, jitterData->kSize, jitterData->kPts, 1, audioStreamType);

					free(jitterData->kData);
					delete jitterData;

					kAudioJitterWindow.erase(jitterHead);
				}
			}
		}
		else
		{
			libos_usleep(1 * 1000); continue;
		}
	}
	avx_success = libos_mux_ts_close(lpTSFile);
}

int test_libos_mux_ts_sample()
{
	int avx_success = 0;

#if H264_TS
#if WRITE_TS

	//#define STREAM_TYPE_AUDIO_AAC       0x0f
	//#define STREAM_TYPE_VIDEO_H264      0x1b
	//#define STREAM_TYPE_VIDEO_HEVC      0x24
	TSFile* lpTSFile = libos_mux_ts_open("../../../Src/muxingTS.ts");

	int audioStreamType = -1;
	int videoStreamType = TYPE_VIDEO_H264;
	avx_success = libos_mux_ts_set_params(lpTSFile, audioStreamType, videoStreamType, 40);
#endif

	FILE* h264File = fopen("../../../Src/264_ts.h264", "rb+");

	//FILE* file = fopen("../../../Src/audiocapture.pcm", "wb");
	
	int   h264frameSize = 1024 * 1024 * 2;
	char *h264frame = (char *)malloc(h264frameSize);

	int   h264bufferSize = 1024 * 1024 * 4;
	char *h264buffer = (char *)malloc(h264bufferSize);

	int   h264bufferOffset = 0;
	int   readSize = 0;
	int   index = 0;
	int   lastFrameSize = 0;

	char *  buf = (char *)malloc(h264bufferSize);
	int     bufIndex = 0;

	uint64_t pts = 126000;

	while (!feof(h264File)) //最后一帧数据会被丢弃
	{
		readSize = fread(h264buffer + h264bufferOffset, 1, h264bufferSize - h264bufferOffset, h264File);
		char* inputData = h264buffer;
		readSize += h264bufferOffset;

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

#if WRITE_TS
			if ((h264frame[0] == 0x00 && h264frame[1] == 0x00 && h264frame[2] == 0x01))
			{
				if ((h264frame[3] & 0x1f) == 7 || (h264frame[3] & 0x1f) == 8)
				{
					memcpy(buf + bufIndex, h264frame, frameSize);
					bufIndex += frameSize;
				}
				else
				if ((h264frame[3] & 0x1f) == 5)
				{
					memcpy(buf + bufIndex, h264frame, frameSize);
					bufIndex += frameSize;

					int keyFrame   = 1;
					int streamType = TYPE_VIDEO_H264; //0x1b
					//fwrite(buf, 1, bufIndex, file);
					avx_success = libos_mux_ts_write(lpTSFile, buf, bufIndex, pts, keyFrame, streamType);
					pts += 3600;

					bufIndex = 0;
				}
				else
				{
					int keyFrame = 0;
					int streamType = TYPE_VIDEO_H264; //0x1b
					//fwrite(h264frame, 1, frameSize, file);
					avx_success = libos_mux_ts_write(lpTSFile, h264frame, frameSize, pts, keyFrame, streamType);
					pts += 3600;
				}
			}
			else if ((h264frame[0] == 0x00 && h264frame[1] == 0x00 && h264frame[2] == 0x00 && h264frame[3] == 0x01))
			{
				if ((h264frame[4] & 0x1f) == 7 || (h264frame[4] & 0x1f) == 8)   //sps和vps
				{
					memcpy(buf + bufIndex, h264frame, frameSize);
					bufIndex += frameSize;
				}
				else if ((h264frame[4] & 0x1f) == 5)    //I帧
				{
					memcpy(buf + bufIndex, h264frame, frameSize);
					bufIndex += frameSize;

					int keyFrame   = 1;
					int streamType = TYPE_VIDEO_H264; //0x1b
					//fwrite(buf, 1, bufIndex, file);
					avx_success = libos_mux_ts_write(lpTSFile, buf, bufIndex, pts, keyFrame, streamType);
					pts += 3600;

					bufIndex = 0;
				}
				else
				{
					int keyFrame = 0;
					int streamType = TYPE_VIDEO_H264; //0x1b
					//fwrite(h264frame, 1, frameSize, file);
					avx_success = libos_mux_ts_write(lpTSFile, h264frame, frameSize, pts, keyFrame, streamType);
					pts += 3600;    //(1 / 25) * 90000 //25是视频帧率，需要固定*90000，音频帧类似
				}
			}
#endif

			readSize -= frameSize;
			inputData += frameSize;
			lastFrameSize = frameSize;

			//printf("%4d %4d %8d \n", index++, avx_success, frameSize);
		}
	}

	fclose(h264File);
	//fclose(file);
#if WRITE_TS
	avx_success = libos_mux_ts_close(lpTSFile);
#endif
#endif

#if H265_TS

#if WRITE_TS
	TSFile* lpTSFile = libos_mux_ts_open("../../../Src/muxingTS.ts");

	int audioStreamType = -1;
	int videoStreamType = TYPE_VIDEO_HEVC;
	avx_success = libos_mux_ts_set_params(lpTSFile, audioStreamType, videoStreamType, 40);
#endif

	FILE* h264File = fopen("../../../Src/265_ts.h265", "rb+");
	int   h264frameSize = 1024 * 1024 * 2;
	char *h264frame = (char *)malloc(h264frameSize);

	int   h264bufferSize = 1024 * 1024 * 4;
	char *h264buffer = (char *)malloc(h264bufferSize);

	int   h264bufferOffset = 0;
	int   readSize         = 0;
	int   index            = 0;
	int   lastFrameSize    = 0;

	char *  buf = (char *)malloc(h264bufferSize);
	int     bufIndex = 0;
	uint64_t pts     = 126000;

	//FILE* file = fopen("../../../Src/audiocapture.pcm", "wb");

	while (!feof(h264File))
	{
		readSize = fread(h264buffer + h264bufferOffset, 1, h264bufferSize - h264bufferOffset, h264File);
		char* inputData = h264buffer;
		readSize += h264bufferOffset;

		while (1)
		{
			int frameSize = 0;
			avx_success = H265NALUDataParser(inputData, readSize, h264frame, &frameSize);
			if (avx_success == -1)
			{
				break;
			}
			else
			if (avx_success == -2)
			{
				memcpy(h264buffer, inputData, readSize);
				h264bufferOffset = readSize;
				break;
			}

#if WRITE_TS
			if ((h264frame[0] == 0x00 && h264frame[1] == 0x00 && h264frame[2] == 0x01))
			{
				int type = (h264frame[3] & 0x7F) >> 1;

				if (type == 32 || type == 33 || type == 34)
				{
					memcpy(buf + bufIndex, h264frame, frameSize);
					bufIndex += frameSize;
				}
				else
				if (type == 19)
				{
					memcpy(buf + bufIndex, h264frame, frameSize);
					bufIndex += frameSize;

					int keyFrame   = 1;
					int streamType = TYPE_VIDEO_HEVC; //0x1b
					//fwrite(buf, 1, bufIndex, file);
					avx_success = libos_mux_ts_write(lpTSFile, buf, bufIndex, pts, keyFrame, streamType);
					pts += 3600;

					bufIndex = 0;
				}
				else
				{
					int keyFrame = 0;
					int streamType = TYPE_VIDEO_HEVC; //0x1b
					avx_success = libos_mux_ts_write(lpTSFile, h264frame, frameSize, pts, keyFrame, streamType);
					//fwrite(h264frame, 1, frameSize, file);
					pts += 3600;
				}
			}
			else if ((h264frame[0] == 0x00 && h264frame[1] == 0x00 && h264frame[2] == 0x00 && h264frame[3] == 0x01))
			{
				int type = (h264frame[4] & 0x7F) >> 1;

				if (type == 32 || type == 33 || type == 34) //vps,sps,pps
				{
					memcpy(buf + bufIndex, h264frame, frameSize);
					bufIndex += frameSize;
				}
				else if (type == 19)    //I帧
				{
					memcpy(buf + bufIndex, h264frame, frameSize);
					bufIndex += frameSize;

					int keyFrame   = 1;
					int streamType = TYPE_VIDEO_HEVC; //0x1b
					//fwrite(buf, 1, bufIndex, file);
					avx_success = libos_mux_ts_write(lpTSFile, buf, bufIndex, pts, keyFrame, streamType);
					pts += 3600;

					bufIndex = 0;
				}
				else
				{
					int keyFrame = 0;
					int streamType = TYPE_VIDEO_HEVC; //0x1b
					avx_success = libos_mux_ts_write(lpTSFile, h264frame, frameSize, pts, keyFrame, streamType);
					//fwrite(h264frame, 1, frameSize, file);
					pts += 3600;
				}
			}
#endif

			readSize -= frameSize;
			inputData += frameSize;
			lastFrameSize = frameSize;

			//printf("%4d %4d %8d \n", index++, avx_success, frameSize);
		}
	}

	fclose(h264File);
	//fclose(file);
#if WRITE_TS
	avx_success = libos_mux_ts_close(lpTSFile);
#endif

#endif

#if AAC_TS

#if WRITE_TS
	TSFile* lpTSFile = libos_mux_ts_open("../../../Src/muxingTS.ts");

	int audioStreamType = TYPE_AUDIO_AAC;
	int videoStreamType = -1;
	avx_success = libos_mux_ts_set_params(lpTSFile, audioStreamType, videoStreamType, 40);
#endif

	FILE *aacFile = fopen("../../../Src/aac_ts.aac", "rb");
	unsigned char *aacframe  = (unsigned char *)malloc(1024 * 5);
	unsigned char *aacbuffer = (unsigned char *)malloc(1024 * 1024);

	int            aacbufferOffset = 0;
	int            readSize        = 0;
	uint64_t pts   = 0;

	while (!feof(aacFile))
	{
		readSize = fread(aacbuffer + aacbufferOffset, 1, 1024 * 1024 - aacbufferOffset, aacFile);
		unsigned char* inputData = aacbuffer;
		readSize += aacbufferOffset;

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

#if WRITE_TS
			avx_success = libos_mux_ts_write(lpTSFile, (char *)aacframe, aacframeSize, pts, 1, TYPE_AUDIO_AAC);
			double cr      = (1024) * 1.0  / 44100;   //44100是采样率，表示1024个数据需要的时间 
			uint64_t pts_1 = cr    * 1000  * 1000;      //转换为微秒
			uint64_t pts_9 = pts_1 * 90000 / 1000000;   //固定*90000，再转换单位为秒
			pts += pts_9;
			//printf("pts:%lld\n", pts);
#endif

			readSize -= aacframeSize;
			inputData += aacframeSize;
		}
	}

	fclose(aacFile);
	free(aacbuffer);
	free(aacframe);

#if WRITE_TS
	avx_success = libos_mux_ts_close(lpTSFile);
#endif

#endif

#if H264_AAC_TS
	std::thread videoThread(videoThreadMethod);
	std::thread audioThread(audioThreadMethod);
	std::thread muxingThread(muxingThreadMethod);

	videoThread.join();
	audioThread.join();
	muxingThread.join();
#endif

#if H265_AAC_TS
	std::thread videoThread(videoh265ThreadMethod);
	std::thread audioThread(audioThreadMethod);
	std::thread muxingThread(muxingThreadMethod);

	videoThread.join();
	audioThread.join();
	muxingThread.join();
#endif

	return avx_success;
}


