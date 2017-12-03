#ifndef VIDEO_ENCODER_X264_H_
#define VIDEO_ENCODER_X264_H_
#include "VideoEncoderBase.h"
#include "PlatformThreadEx.h"
#ifdef __cplusplus
extern "C" {
#endif

#include "x264.h"

#ifdef __cplusplus
}
#endif
#include "BufferQuene.h"
class CVideoEncoderX264 :public CVideoEncoderBase
{
	x264_t *m_pX264 = nullptr;
	bool m_bAbort = false;
	int m_nAvgEncodedTimeMs = 0;
	CPlatformThreadEx<CVideoEncoderX264> m_hEncodeThread;
	CBufferQueneEx<VideoFrame> m_qVideoFrameQ;

	int m_nLevel = 21;
	bool OnEncodeThread();
	void UpdateEncodeTime(int difftime)
	{
		m_nAvgEncodedTimeMs = (m_nAvgEncodedTimeMs + difftime) / 2;
	}
	void ConfigSetting(x264_param_t* x264param);

public:
	CVideoEncoderX264();
	virtual int OpenEncoder();

	virtual int CloseEncoder();

	virtual int PushData(uint8_t *imagedata, int nsize, int64_t pts);

	virtual sH264CodecInfo GetCodecInfo();

	virtual int StartEncode() override;

	virtual EncoderRunTimeInfo GetRunTimeInfo();

};
#endif 