#pragma once
#include "android/jnihelper/jni_classloader.h"
#include"android/jnihelper/jni_helpers.h"

#include<memory>
#include "OpenglCapture.h"
class COpenglCapture_JniWrap:public COpenglCapture::IFrameAvaliable
{
public:
	static CRegisterNativeM s_registernm;
	std::unique_ptr<COpenglCapture> m_pCapturer;
	static COpenglCapture_JniWrap* GetInst(JNIEnv* jni, jobject j_object);

	uint8_t *m_pDirectBuffer = nullptr;
	//java
	ScopedGlobalRef<jobject> m_pThiz;
	jmethodID m_jinitDirectBuffer;
	jmethodID m_jdataCallback;
	COpenglCapture_JniWrap(JNIEnv *env, jobject thiz);
	static  jlong JNICALL newinstance(JNIEnv *env, jobject thiz);
	static void JNICALL native_destroy(JNIEnv *env, jobject thiz);
	static jboolean JNICALL native_initCapture(JNIEnv *env, jobject thiz, jint width, jint height);
	static void JNICALL native_deinitCapture(JNIEnv *env, jobject thiz);
	static void JNICALL native_captureFrame(JNIEnv *env, jobject thiz, jint textureid);
	static void JNICALL native_setDirectBuffer(JNIEnv *env, jobject thiz, jobject bytebuffer);


	virtual ~COpenglCapture_JniWrap();

	virtual void OnCapture(const uint8_t* pdata, int nsize, int stride, int64_t pts) override;

};