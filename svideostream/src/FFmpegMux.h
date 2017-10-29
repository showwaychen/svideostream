#ifndef FFMPEG_MUX_H_
#define FFMPEG_MUX_H_
#include <stdint.h>
extern "C"
{

#include "libavformat/avformat.h"
#include "libavutil/opt.h"
}
#include <string>
#include "Common.h"
#include"base/criticalsection.h"


class CFFmpegMux
{
	AVFormatContext *m_pAVFmtCtx;
	AVOutputFormat *m_pAVOutFmt;
	AVStream *m_pVideoStream;
	AVStream *m_pAudioStream;
	AVCodec *m_pVideoCodec;
	AVCodec *m_pAudioCodec;
	AVFrame *m_pVideoFrame;
	AVPacket m_pktVideo;
	AVPacket m_pktAudio;

	bool m_bAudioEnable = true;

	std::string m_strFileName;
	std::string m_strCopyRight;

	sH264CodecInfo m_H264Info;
	sAacCodecInfo m_AacInfo;
	int64_t m_nLastVideoPts = 0;
	bool m_StartOk = false;

	rtc::CriticalSection m_Mux_cs;
	void AddVideoStream();
	void AddAudioStream();
	bool m_bWaitAudioSpecData = false;
	bool m_bWaitVideoSpecData = false;
	int OnAVSpecdata();
	void MemberReset();
public:

	void SetH264CodecInfo(sH264CodecInfo vinfo)
	{
		m_H264Info = vinfo;
	}
	void SetAacCodecInfo(sAacCodecInfo ainfo)
	{
		m_AacInfo = ainfo;
	}
	void SetEnableAudio(bool enable)
	{
		m_bAudioEnable = enable;
	}
	void SetFileName(const std::string& filename)
	{
		m_strFileName = filename;
	}
	void SetCopyRight(const std::string& copyright)
	{
		m_strCopyRight = copyright;
	}
	int StartMux();
	void StopMux();
	int WriteVideoData(uint8_t* vdata, int nsize, int64_t npts);
	int WriteAudioData(uint8_t* vdata, int nsize, int64_t npts);
};
#endif 