#include "VideoEncoderFactory.h"
#include "VideoEncoderX264.h"
#if defined(SVS_ANDROID)
#include "../android/VideoEncoderMediaCodec.h"
#endif

CVideoEncoderBase* CVideoEncoderFactory::CreateVideoEncoder(VideoEncoderType type)
{
	if (H264ENCODER_X264 == type)
	{
		return new CVideoEncoderX264();
	}
#if defined(SVS_ANDROID)
	else if (H264ENCODER_MEDIACODEC == type)
	{
		return new CVideoEncoderMediaCodec;
	}
#endif
	return nullptr;
}
