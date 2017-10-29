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
	CPlatformThreadEx<CVideoEncoderX264> m_hEncodeThread;
	CBufferQuene<VideoFrame*, &VideoFrame::FreeFun> m_qVideoFrameQ;
	bool OnEncodeThread();
public:
	CVideoEncoderX264();
	virtual int OpenEncoder();

	virtual int CloseEncoder();

	virtual int PushData(uint8_t *imagedata, int nsize, int64_t pts);

	virtual sH264CodecInfo GetCodecInfo();

	virtual int StartEncode() override;

};
#endif 