#include "VideoEncoderFactory.h"
#include "VideoEncoderX264.h"
#if defined(SVS_ANDROID)
#include "../android/VideoEncoderMediaCodec.h"
#endif

std::shared_ptr<CVideoEncoderBase> CVideoEncoderFactory::CreateVideoEncoder(VideoEncoderType type)
{
	if (H264ENCODER_X264 == type)
	{
		return std::make_shared<CVideoEncoderX264>();
	}
#if defined(SVS_ANDROID)
	else if (H264ENCODER_MEDIACODEC == type)
	{
		return std::make_shared<CVideoEncoderMediaCodec>();
	}
#endif
	return nullptr;
}
