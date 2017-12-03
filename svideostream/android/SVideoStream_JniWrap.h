#ifndef SVIDEO_STREAM_JNIWRAP_H_
#define SVIDEO_STREAM_JNIWRAP_H_
#include "../src/SVideoStream.h"
#include "android/jnihelper/jni_classloader.h"

#include <memory>
#include "../src/VideoEncoderFactory.h"

enum SettingKeyValue
{
	SKV_H264ENCODERCONFIG = 0
};
class CSVideoStream_JniWrap: public CSVideoStream::IStreamEventObserver
{

	std::unique_ptr<CVideoEncoderBase> m_pVideoEncoder;
	std::unique_ptr<CAudioEncoderBase> m_pAudioEncoder;

	std::unique_ptr<CSVideoStream> m_pVideoStream;
	CVideoEncoderFactory::VideoEncoderType m_eVideoEncoderType = CVideoEncoderFactory::H264ENCODER_X264;

	jobject m_jThiz;
	jmethodID m_jEventCallback;

	CMediaConfig m_H264Configs;

	static CSVideoStream_JniWrap* GetInst(JNIEnv* jni, jobject j_object);
public:
	static CRegisterNativeM s_registernm;
	CSVideoStream_JniWrap(JNIEnv *env, jobject thiz);
	static  jlong JNICALL newinstance(JNIEnv *env, jobject thiz);
	static  jint JNICALL nativeStart(JNIEnv *env, jobject thiz);
	static  jint JNICALL nativeStop(JNIEnv *env, jobject thiz);
	static  void JNICALL nativeSetStreamType(JNIEnv *env, jobject thiz, jint type);
	static  void JNICALL nativeSetSrcType(JNIEnv *env, jobject thiz, jint type);
	static  void JNICALL nativeSetSrcImageParams(JNIEnv *env, jobject thiz, jint format, jint stride, int width, int height);
	static  void JNICALL nativeSetDstParams(JNIEnv *env, jobject thiz, int width, int height);
	static  void JNICALL nativeSetEncoderType(JNIEnv* env, jobject thiz, jint type);
	static  void JNICALL nativeSetVideoEncodeParams(JNIEnv *env, jobject thiz, jint bitrate, jint fps);
	static  void JNICALL nativeSetAudioParams(JNIEnv *env, jobject thiz, jint samplerate, jint channels, jint samplesize, jint bitrate);

	static  void JNICALL nativeSetRotation(JNIEnv *env, jobject thiz, int rotation);
	static  void JNICALL nativeSetPublishUrl(JNIEnv *env, jobject thiz, jstring url);
	static  void JNICALL nativeSetRecordPath(JNIEnv *env, jobject thiz, jstring filename);
	static  void JNICALL nativeSetAudioEnable(JNIEnv *env, jobject thiz, jboolean isenable);
	static  void JNICALL nativeSettingSet(JNIEnv *env, jobject thiz,jint key, jobject value);
	static  jobjectArray JNICALL nativeGetStats(JNIEnv *env, jobject thiz);
	static  jint JNICALL nativeGetState(JNIEnv *env, jobject thiz);

	static  jint JNICALL nativeInputVideoData(JNIEnv *env, jobject thiz, jobject bytebuffer, int size, long pts);
	static  jint JNICALL nativeInputAudioData(JNIEnv *env, jobject thiz, jbyteArray data, int size, long pts);
	static void JNICALL native_Destroy(JNIEnv *env, jobject thiz);


	void SetAndConfigVideoEncoder();
	int StartStream();
	int StopStream();
	void SetVideoEncoderType(CVideoEncoderFactory::VideoEncoderType etype)
	{
		m_eVideoEncoderType = etype;
	}
	~CSVideoStream_JniWrap();

	virtual void OnStreamEvent(StreamEvent event, StreamError error);

};
#endif 