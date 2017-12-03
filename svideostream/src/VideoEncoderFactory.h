#ifndef VIDEOENCODER_FACTORY_H__
#define VIDEOENCODER_FACTORY_H__
class CVideoEncoderBase;
class CVideoEncoderFactory
{
public:
	enum VideoEncoderType
	{
		H264ENCODER_X264 = 0,
		H264ENCODER_MEDIACODEC
	};
	static CVideoEncoderBase* CreateVideoEncoder(VideoEncoderType type);
};
#endif




