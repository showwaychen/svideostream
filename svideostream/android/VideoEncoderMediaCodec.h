#ifndef VIDEOENCODER_MEDIACODEC_H_
#define VIDEOENCODER_MEDIACODEC_H_
#include "../src/VideoEncoderBase.h"

#include<jni.h>
#include "../src/PlatformThreadEx.h"
#include "../src/BufferQuene.h"
#include <memory>
// CVideoEncoderMediaCodec is a CVideoEncoderBase implementation that uses
// Android's MediaCodec SDK API
class CVideoEncoderMediaCodec :public CVideoEncoderBase
{
	static std::string s_strMediaCodecClassName;
	static std::string s_strOutputBufferInfoClassName;

	jobject m_jMediaCodec;
	jclass  m_jMediaCodecClass;
	//int openEncode(int width, int height, int kbps, int fps) 
	jmethodID m_jOpenEncoderMethod;
	jmethodID m_jCloseEncoderMethod;
	jfieldID  m_jVideoColorFormat;
	jmethodID m_jget_input_buffers_method_;
	jmethodID m_jDequeueInputBuffer;
	jmethodID m_jencode_buffer_method_;
	jmethodID m_jdequeue_output_buffer_method_;
	jmethodID m_jrelease_output_buffer_method_;

	




	ImageFormat m_eEncoderSupportColorFormate = IMAGE_FORMAT_I420;
	bool m_bAbort = false;
	CPlatformThreadEx<CVideoEncoderMediaCodec> m_hEncodeThread;
	CBufferQueneEx<VideoFrame> m_qVideoFrameQ;
	bool OnEncodeThread();
	//output_buffer_info
	//jclass  m_jOutputBufferInfoClass;
	jfieldID m_jinfo_index;
	jfieldID m_jinfo_iskeyframe;
	jfieldID m_jinfo_pts;
	jfieldID m_jinfo_buffer;
	static jobject NewObject(JNIEnv* jni, jclass pjclass, const char* name, const char* signature, ...);
	void ConfigSetting(JNIEnv* jnienv);

	int m_nAvgEncodedTimeMs = 0;

	void UpdateEncodeTime(int difftime)
	{
		m_nAvgEncodedTimeMs = (m_nAvgEncodedTimeMs + difftime) / 2;
	}
public:
	CVideoEncoderMediaCodec();
	virtual int OpenEncoder();

	virtual int StartEncode();

	virtual int CloseEncoder();

	virtual int PushData(uint8_t *imagedata, int nsize, int64_t pts);

	virtual sH264CodecInfo GetCodecInfo();

	virtual EncoderRunTimeInfo GetRunTimeInfo() override;

};
#endif 



