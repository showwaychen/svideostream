#include "VideoEncoderMediaCodec.h"
#include "android/jnihelper/jni_helpers.h"
#include "../src/H264AacUtils.h"
#include"libyuv.h"
#include"base/timeutils.h"


std::string CVideoEncoderMediaCodec::s_strMediaCodecClassName = "cn/cxw/svideostreamlib/MediaCodecVideoEncoder";

std::string CVideoEncoderMediaCodec::s_strOutputBufferInfoClassName = "cn/cxw/svideostreamlib/MediaCodecVideoEncoder$OutputBufferInfo";

libyuv::FourCC mapFromImageFormat(ImageFormat format)
{
	switch (format)
	{
	case IMAGE_FORMAT_NONE:
		break;
	case IMAGE_FORMAT_NV12:
		return libyuv::FOURCC_NV12;
		break;
	case IMAGE_FORMAT_NV21:
		return libyuv::FOURCC_NV21;
		break;
	case IMAGE_FORMAT_ARGB:
		return libyuv::FOURCC_BGRA;
		break;
	case IMAGE_FORMAT_RGBA:
		return libyuv::FOURCC_ABGR;
		break;
	case IMAGE_FORMAT_ABGR:
		return libyuv::FOURCC_RGBA;
		break;
	case IMAGE_FORMAT_BGRA:
		return libyuv::FOURCC_ARGB;
		break;
	case IMAGE_FORMAT_I420:
		return libyuv::FOURCC_I420;
		break;
	default:
		return libyuv::FOURCC_I420;
		break;
	}
	return libyuv::FOURCC_I420;
}
bool CVideoEncoderMediaCodec::OnEncodeThread()
{
	AttachThreadScoped attachthread(GetJavaVM());
	JNIEnv* jnienv = attachthread.env();
	int inputbufferindex = -1;
	std::unique_ptr<uint8_t> convertDstImage;
	int convertSize = 0;
	bool needCovert = false;
	
	int colorformat = jnienv->GetIntField(m_jMediaCodec, m_jVideoColorFormat);
	m_eEncoderSupportColorFormate = (ImageFormat)colorformat;
	if (m_eEncoderSupportColorFormate != IMAGE_FORMAT_I420)
	{
		needCovert = true;
		convertSize = CH264AacUtils::GetFrameSize(m_eEncoderSupportColorFormate, m_nWidth, m_nHeight);
		convertDstImage.reset(new uint8_t[convertSize]);
		memset(convertDstImage.get(), 0, convertSize);
	}
	LOGD << "encoder support input image format = " << m_eEncoderSupportColorFormate;
	//get input buffers
	jobjectArray input_buffers = reinterpret_cast<jobjectArray>(jnienv->CallObjectMethod(m_jMediaCodec,
		                                                         m_jget_input_buffers_method_));
	bool encode_status;
	size_t num_input_buffers;
	if (IsNull(jnienv, input_buffers))
	{
		LOGE << "get_input_buffers failed";
		goto end;
	}
	num_input_buffers = jnienv->GetArrayLength(input_buffers);
	LOGD << "num_input_buffers = " << num_input_buffers;
	if (num_input_buffers == 0)
	{
		goto end;
	}
	
	while (!m_bAbort)
	{
		//get a frame
		std::unique_ptr<VideoFrame> newframe;
		if (!m_qVideoFrameQ.PullData(newframe, true))
		{
			usleep(20000);
			continue;
		}
		//dequeue a input buffer
		inputbufferindex = jnienv->CallIntMethod(m_jMediaCodec, m_jDequeueInputBuffer);
		LOGD << "dequeue input buffer index = " << inputbufferindex;
		if (inputbufferindex == -1)
		{
			LOGW << "no input buffers available";
		}
		else
		{
			//filling data to input buffer
			long pts = newframe->GetPts();
			jobject ainput_buffer = jnienv->GetObjectArrayElement(input_buffers, inputbufferindex);
			uint8_t* yuv_buffer = reinterpret_cast<uint8_t*>(jnienv->GetDirectBufferAddress(ainput_buffer));
			if (needCovert)
			{
				uint8_t* i420data = newframe->GetFrameData();
				int uoffset = m_nWidth * m_nHeight;
				int voffset = m_nWidth * m_nHeight * 5 / 4;
				libyuv::ConvertFromI420(i420data, m_nWidth, i420data + uoffset, m_nWidth / 2,
					i420data + voffset, m_nWidth / 2, convertDstImage.get(), m_nWidth, m_nWidth, m_nHeight, mapFromImageFormat(m_eEncoderSupportColorFormate));
				memcpy(yuv_buffer, convertDstImage.get(), convertSize);
			}
			else
			{
				memcpy(yuv_buffer, newframe->GetFrameData(), convertSize);
			}
			jnienv->DeleteLocalRef(ainput_buffer);
			//encoding a frame
			encode_status = jnienv->CallBooleanMethod(m_jMediaCodec,
				m_jencode_buffer_method_,
				false,
				inputbufferindex,
				convertSize,
				pts);
			if (!encode_status)
			{
				LOGW << "encoded failed";
				LOGD << "encode buffer status = " << encode_status;
			}
		}
		//dequene output buffers
		while (true)
		{
			int64_t earilerTime = rtc::TimeMillis();
			jobject outputbuffer = jnienv->CallObjectMethod(m_jMediaCodec, m_jdequeue_output_buffer_method_);
			if (IsNull(jnienv, outputbuffer))
			{
				LOGW << " outputbuffer is null";
				usleep(10000);
				break;
			}
			UpdateEncodeTime(rtc::TimeSince(earilerTime));
			bool key_frame = jnienv->GetBooleanField(outputbuffer, m_jinfo_iskeyframe);
			int outbufferindex = jnienv->GetIntField(outputbuffer, m_jinfo_index);
			long pts = jnienv->GetLongField(outputbuffer, m_jinfo_pts);
			jobject bufferdata = jnienv->GetObjectField(outputbuffer, m_jinfo_buffer);
			LOGD << "outputbufferinfo key_frame = " << key_frame << " index = " << outbufferindex << " pts = " << pts;
			// Extract payload.
			size_t payload_size = jnienv->GetDirectBufferCapacity(bufferdata);
			uint8_t* payload = reinterpret_cast<uint8_t*>(
				jnienv->GetDirectBufferAddress(bufferdata));
			//LOGD << "encoded data size = " << payload_size;
			/*if (payload_size >= 4)
			{
			LOGD << "prefix data = " << std::hex << (int)payload[0] << "  "
			<< std::hex << (int)payload[1] << "  " << std::hex << (int)payload[2] <<
			"  " << std::hex << (int)payload[3] << " " << std::hex << (int)payload[4];
			}*/
			if (m_pCallBack != nullptr)
			{
				m_pCallBack->OnVideoEncodedData(payload, payload_size, pts, pts);
			}
			else
			{
				LOGW << "data encoded successfully but no callback funtion";
			}
			//release outputbuffer 
			jnienv->DeleteLocalRef(outputbuffer);
			jnienv->DeleteLocalRef(bufferdata);
			jnienv->CallBooleanMethod(m_jMediaCodec, m_jrelease_output_buffer_method_, outbufferindex);
		}
			
	}
end:

	jnienv->CallIntMethod(m_jMediaCodec, m_jCloseEncoderMethod);
	return false;
}

jobject CVideoEncoderMediaCodec::NewObject(JNIEnv* jni, jclass pjclass, const char* name, const char* signature, ...)
{
	va_list args;
	va_start(args, signature);
	jobject obj = jni->NewObjectV(pjclass, jni->GetMethodID(pjclass, name, signature),
		args);
	va_end(args);
	return jni->NewGlobalRef(obj);
}

void CVideoEncoderMediaCodec::ConfigSetting(JNIEnv* jnienv)
{
	jfieldID jgop = GetFieldID(jnienv, m_jMediaCodecClass, "gop", "I");
	jfieldID jprofile = GetFieldID(jnienv, m_jMediaCodecClass, "profile", "Ljava/lang/String;");
	jfieldID jrc_method = GetFieldID(jnienv, m_jMediaCodecClass, "rc_method", "Ljava/lang/String;");

	jnienv->SetIntField(m_jMediaCodec, jgop, m_nGop);
	std::string value;
	if (m_H264Configs.GetConfigFromKey(kProfile, value))
	{
		jstring j_new_str = jnienv->NewStringUTF(value.c_str());
		jnienv->SetObjectField(m_jMediaCodec, jprofile, j_new_str);
		
	}
	if (m_H264Configs.GetConfigFromKey(kRcMethod, value))
	{
		jstring j_new_str = jnienv->NewStringUTF(value.c_str());
		jnienv->SetObjectField(m_jMediaCodec, jrc_method, j_new_str);
	}
}

CVideoEncoderMediaCodec::CVideoEncoderMediaCodec()
:m_hEncodeThread(this, &CVideoEncoderMediaCodec::OnEncodeThread, "mediacodecencodethread")
{
	JNIEnv* jni = GetEnv(GetJavaVM());
	jclass pj_class_ = jni->FindClass(s_strMediaCodecClassName.c_str());
	m_jMediaCodecClass = reinterpret_cast<jclass>(jni->NewGlobalRef(pj_class_));
	m_jMediaCodec = NewObject(jni, m_jMediaCodecClass, "<init>", "()V");
	m_jOpenEncoderMethod = jni->GetMethodID(m_jMediaCodecClass, "openEncode", "(IIII)I");
	m_jCloseEncoderMethod = jni->GetMethodID(m_jMediaCodecClass, "closeEncode", "()I");
	m_jVideoColorFormat = jni->GetFieldID(m_jMediaCodecClass, "mVideoColorFormat", "I");
	m_jget_input_buffers_method_ = jni->GetMethodID(m_jMediaCodecClass, "getInputBuffers", "()[Ljava/nio/ByteBuffer;");
	m_jDequeueInputBuffer = jni->GetMethodID(m_jMediaCodecClass, "dequeueInputBuffer", "()I");
	m_jencode_buffer_method_ = jni->GetMethodID(m_jMediaCodecClass, "encodeBuffer", "(ZIIJ)Z");
	m_jdequeue_output_buffer_method_ = jni->GetMethodID(m_jMediaCodecClass, "dequeueOutputBuffer", "()Lcn/cxw/svideostreamlib/MediaCodecVideoEncoder$OutputBufferInfo;");
	m_jrelease_output_buffer_method_ = jni->GetMethodID(m_jMediaCodecClass, "releaseOutputBuffer", "(I)Z");

	//output_buffer_info
	jclass _class_ = jni->FindClass(s_strOutputBufferInfoClassName.c_str());
	//m_jOutputBufferInfoClass = reinterpret_cast<jclass>(jni->NewGlobalRef(_class_));
	m_jinfo_buffer = jni->GetFieldID(_class_, "buffer", "Ljava/nio/ByteBuffer;");
	m_jinfo_index = jni->GetFieldID(_class_, "index", "I");
	m_jinfo_iskeyframe = jni->GetFieldID(_class_, "isKeyFrame", "Z");
	m_jinfo_pts = jni->GetFieldID(_class_, "presentationTimestampUs", "J");


	m_qVideoFrameQ.SetMaxCount(kMaxBufferSize);
}

int CVideoEncoderMediaCodec::OpenEncoder()
{
	int ret = 0;
	AttachThreadScoped attachthread(GetJavaVM());
	JNIEnv* jnienv = attachthread.env();
	ConfigSetting(jnienv);
	ret = jnienv->CallIntMethod(m_jMediaCodec, m_jOpenEncoderMethod, m_nWidth, m_nHeight, m_nBitrate, m_nFps);
	return ret;
}

int CVideoEncoderMediaCodec::StartEncode()
{
	m_bAbort = false;
	m_qVideoFrameQ.SetEnable(true);
	m_hEncodeThread.Start();
	return 0;
}

int CVideoEncoderMediaCodec::CloseEncoder()
{
	m_bAbort = true;
	m_qVideoFrameQ.SetEnable(false);
	m_hEncodeThread.Stop();
	int ret = 0;
	
	return ret;
}

int CVideoEncoderMediaCodec::PushData(uint8_t *imagedata, int nsize, int64_t pts)
{
	std::unique_ptr<VideoFrame> frame(new VideoFrame(m_nWidth, m_nHeight));
	frame->FillData(imagedata, nsize, pts);
	if (!m_qVideoFrameQ.PushData(frame, false))
	{
		return 1;
	}
	return 0;
}

sH264CodecInfo CVideoEncoderMediaCodec::GetCodecInfo()
{
	std::string value;
	sH264CodecInfo info;
	info.m_bAnnexB = true;
	info.m_nBitrate = m_nBitrate;
	info.m_nFps = m_nFps;
	info.m_nGop = m_nGop;
	info.m_nHeight = m_nHeight;
	info.m_nWidth = m_nWidth;
	info.m_nLevel = 21;
	m_H264Configs.GetConfigFromKey(kProfile, value);
	info.m_nProfile = StringToProfile(value);
	return info;
}

CVideoEncoderBase::EncoderRunTimeInfo CVideoEncoderMediaCodec::GetRunTimeInfo()
{
	CVideoEncoderBase::EncoderRunTimeInfo info;
	info.m_nBufferRemainNum = m_qVideoFrameQ.Size();
	info.m_nEncodeAvgTimeMs = m_nAvgEncodedTimeMs;
	return info;
}
