#ifndef __COMMON_H__
#define __COMMON_H__
#include <stdio.h>
#include <stdint.h>
#include"string.h"
#define VERSION		"1.4"

class sH264CodecInfo
{
public:
	int m_nWidth;
	int m_nHeight;
	bool m_bAnnexB;
	int m_nBitrate;
	int m_nFps;
	int m_nGop;
	int m_nLevel;
	int m_nProfile;
};
class sAacCodecInfo
{
public:
	int m_nSampleRate;
	int m_nSampleSize;
	int m_nFrameSize;
	int m_nChannels;
	int m_nBitrate;
};

enum StreamType
{
	ST_NONE,
	ST_LIVE,
	ST_RECORD
};
enum SrcDataType
{
	SDT_IMAGEDATA,
	SDT_H264DATA
};
enum StreamState
{
	StreamState_NONE = 0,
	StreamState_STARTING,
	StreamState_STARTED,
	StreamState_STOPPING,
	StreamState_STOPED
};
enum RotationMode
{
	kRotate0 = 0,  // No rotation.
	kRotate90 = 90,  // Rotate 90 degrees clockwise.
	kRotate180 = 180,  // Rotate 180 degrees.
	kRotate270 = 270,  // Rotate 270 degrees clockwise.
};
enum StreamError
{
	kSE_NoneError = 0,
	kSE_UnknowError,
	kSE_VideoEncoderOpenedFailed ,
	kSE_AudioeEncoderOpenedFailed,
	kSE_LiveConnectFailed,
	kSE_RecordOpenFileFailed,
	kSE_LiveFailed
};
enum StreamEvent
{
	SE_LiveConnected =0,
	SE_LiveDisconnected,
	SE_RecordStartedSuccess,
	SE_StreamWarning,
	SE_StreamFailed,
	SE_StreamStarted
};
enum ImageFormat
{
	IMAGE_FORMAT_NONE = 0,
	IMAGE_FORMAT_NV12,
	IMAGE_FORMAT_NV21,
	IMAGE_FORMAT_ARGB,
	IMAGE_FORMAT_RGBA,
	IMAGE_FORMAT_ABGR,
	IMAGE_FORMAT_BGRA,
	IMAGE_FORMAT_I420
};

#endif


