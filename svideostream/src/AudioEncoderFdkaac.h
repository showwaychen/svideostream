#ifndef AUDIO_ENCODER_FDKAAC_H_
#define AUDIO_ENCODER_FDKAAC_H_
#include "AudioEncoderBase.h"
#include "aacenc_lib.h"
#include "PlatformThreadEx.h"
#include "BufferQuene.h"
class CAudioEncoderFdkaac :public CAudioEncoderBase
{
	bool m_bAbort = false;
	HANDLE_AACENCODER m_handle;
	CHANNEL_MODE m_eMode;
	AACENC_InfoStruct m_sInfo;
	bool m_bEncoderOk = false;
	bool OnAudioEncoderThread();
	CPlatformThreadEx<CAudioEncoderFdkaac> m_hEncodeThread;
	CBufferQuene<AudioFrame*, &AudioFrame::FreeFun> m_qFrameQ;
public:
	CAudioEncoderFdkaac();
	virtual int OpenEncoder();

	virtual int CloseEncoder();

	virtual int PushData(uint8_t *audiodata, int nsize, int64_t pts);

	virtual sAacCodecInfo GetCodecInfo();

	virtual int StartEncode() override;

};
#endif 