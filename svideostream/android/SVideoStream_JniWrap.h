#ifndef SVIDEO_STREAM_JNIWRAP_H_
#define SVIDEO_STREAM_JNIWRAP_H_
#include "../src/SVideoStream.h"
#include "android/jnihelper/jni_classloader.h"
#define  SAFE_DELETE(instance) \
if (instance != nullptr) {\
delete instance; \
instance = nullptr; }

enum VideoEncoderType
{
	H264ENCODER_X264 = 0,
	H264ENCODER_MEDIACODEC
};
class CSVideoStream_JniWrap: public CSVideoStream::IStreamEventObserver
{

	CSVideoStream *m_pVideoStream = nullptr;
	CVideoEncoderBase* m_pVideoEncoder = nullptr;
	CAudioEncoderBase* m_pAudioEncoder = nullptr;
	VideoEncoderType m_eVideoEncoderType = H264ENCODER_X264;

	jobject m_jThiz;
	jmethodID m_jEventCallback;

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
	static  jint JNICALL nativeInputVideoData(JNIEnv *env, jobject thiz, jbyteArray data, int size, long pts);
	static  jint JNICALL nativeInputAudioData(JNIEnv *env, jobject thiz, jbyteArray data, int size, long pts);
	static void JNICALL native_Destroy(JNIEnv *env, jobject thiz);


	int StartStream();
	int StopStream();
	void SetVideoEncoderType(VideoEncoderType etype)
	{
		m_eVideoEncoderType = etype;
	}
	~CSVideoStream_JniWrap();

	virtual void OnStreamEvent(StreamEvent event, StreamError error);

};
#endif 