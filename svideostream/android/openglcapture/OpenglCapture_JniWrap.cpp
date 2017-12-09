#include "OpenglCapture_JniWrap.h"
#include"android/jnihelper/jni_onload.h"
#include"android/jnihelper/jni_helpers.h"
#include"OpenglCapture.h"

#include "Log.h"
#define VERSION		"1.5"
JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
	JNIEnv *env = NULL;

	SetJavaVM(vm);
	LOGI << "libopenglcapture version : " << VERSION;
	CCommonSetting::SetLogLevel(rtc::LS_SENSITIVE);
	if (0 == on_JNI_OnLoad(GetJavaVM(), JNI_VERSION_1_6))
	{
		LOGD << "on_JNI_OnLoad success";
	}
	else
	{
		return -1;
	}
	return JNI_VERSION_1_6;
}
static JNINativeMethod ls_nm[] = {
		{ "native_newInstance", "()J", reinterpret_cast<void*>(
		&COpenglCapture_JniWrap::newinstance) },
		{ "native_initCapture", "(II)Z", reinterpret_cast<void*>(
		&COpenglCapture_JniWrap::native_initCapture) },
		{ "native_deinitCapture", "()V", reinterpret_cast<void*>(
		&COpenglCapture_JniWrap::native_deinitCapture) },
		{ "native_captureFrame", "(I)V", reinterpret_cast<void*>(
		&COpenglCapture_JniWrap::native_captureFrame) },
		{ "native_destroy", "()V", reinterpret_cast<void*>(
		&COpenglCapture_JniWrap::native_destroy) },
		{ "native_setDirectBuffer", "(Ljava/nio/ByteBuffer;)V", reinterpret_cast<void*>(
		&COpenglCapture_JniWrap::native_setDirectBuffer) }
};
CRegisterNativeM COpenglCapture_JniWrap::s_registernm("cxw/cn/gpuimageex/GraphicBufferCapture", ls_nm, ARRAYSIZE(ls_nm));

COpenglCapture_JniWrap* COpenglCapture_JniWrap::GetInst(JNIEnv* jni, jobject j_object)
{
	jclass j_class = jni->GetObjectClass(j_object);
	jfieldID nativeobject_id = jni->GetFieldID(j_class, "mNativeInstance", "J");
	MYCHECK_EXCEPTION(jni, "GetInst failed");
	jlong j_p = jni->GetLongField(j_object, nativeobject_id);
	MYCHECK_EXCEPTION(jni, "GetInst failed");
	return reinterpret_cast<COpenglCapture_JniWrap*>(j_p);
}

COpenglCapture_JniWrap::COpenglCapture_JniWrap(JNIEnv *env, jobject thiz):
m_pThiz(env, thiz)
{
	m_pCapturer.reset(new COpenglCapture());
	m_pCapturer->SetFrameCallback(this);
	jclass oclass = env->GetObjectClass(*m_pThiz);
	m_jinitDirectBuffer = env->GetMethodID(oclass, "initDirectBuffer", "(I)V");
	m_jdataCallback = env->GetMethodID(oclass, "nativeCallback", "(IJ)V");
}

jlong JNICALL COpenglCapture_JniWrap::newinstance(JNIEnv *env, jobject thiz)
{
	COpenglCapture_JniWrap* instance =  new COpenglCapture_JniWrap(env, thiz);
	//(*instance)->Init();
	return jlongFromPointer((void*)instance);
}

void JNICALL COpenglCapture_JniWrap::native_destroy(JNIEnv *env, jobject thiz)
{
	COpenglCapture_JniWrap* instance = COpenglCapture_JniWrap::GetInst(env, thiz);
	if (instance != nullptr)
	{
		delete instance;
	}
	return ;
}

jboolean JNICALL COpenglCapture_JniWrap::native_initCapture(JNIEnv *env, jobject thiz, jint width, jint height)
{
	COpenglCapture_JniWrap* instance = COpenglCapture_JniWrap::GetInst(env, thiz);
	if (instance != nullptr)
	{
		return instance->m_pCapturer->InitCapture(width, height);
	}
	return false;
}

void JNICALL COpenglCapture_JniWrap::native_deinitCapture(JNIEnv *env, jobject thiz)
{
	COpenglCapture_JniWrap* instance = COpenglCapture_JniWrap::GetInst(env, thiz);
	if (instance != nullptr)
	{
		instance->m_pCapturer->Destroy();
	}
}

void JNICALL COpenglCapture_JniWrap::native_captureFrame(JNIEnv *env, jobject thiz, jint textureid)
{
	COpenglCapture_JniWrap* instance = COpenglCapture_JniWrap::GetInst(env, thiz);
	if (instance != nullptr)
	{
		instance->m_pCapturer->CaptureFrame(textureid);
	}
}




COpenglCapture_JniWrap::~COpenglCapture_JniWrap()
{

}
#define SAVE_YUV_FILE
#ifdef SAVE_YUV_FILE
static FILE *pyuvFile = NULL;
#endif
void COpenglCapture_JniWrap::OnCapture(const uint8_t* pdata, int nsize, int stride, int64_t pts)
{
	//LOGD << "OnCapture nsize = "<<nsize<<" stride = "<<stride;
	AttachThreadScoped attachthread(GetJavaVM());
	JNIEnv* jenv = attachthread.env();
	if (m_pDirectBuffer == nullptr)
	{
		jenv->CallVoidMethod(*m_pThiz, m_jinitDirectBuffer, nsize);
	}
	if (m_pDirectBuffer == nullptr)
	{
		LOGE << "direct buffer is null";
		return;
	}
	memcpy(m_pDirectBuffer, pdata, nsize);
	jenv->CallVoidMethod(*m_pThiz, m_jdataCallback, stride, pts);

//#ifdef SAVE_YUV_FILE
//	//if (NULL != pyuvFile) {
//	pyuvFile = fopen("/storage/emulated/0/openglcapture.argb", "wb+");
//	if (pyuvFile == nullptr)
//	{
//		LOGE << "open yuv file failed";
//		return;
//    }
//		fwrite(pdata, nsize, 1, pyuvFile);
//		fflush(pyuvFile);
//		fclose(pyuvFile);
//		pyuvFile = NULL;
//	//}
//#endif
}

void JNICALL COpenglCapture_JniWrap::native_setDirectBuffer(JNIEnv *env, jobject thiz, jobject bytebuffer)
{
	COpenglCapture_JniWrap* instance = COpenglCapture_JniWrap::GetInst(env, thiz);
	if (instance != nullptr)
	{
		uint8_t * pBuffer = (uint8_t *)env->GetDirectBufferAddress(bytebuffer);
		instance->m_pDirectBuffer = pBuffer;
	}
	uint8_t * pBuffer = (uint8_t *)env->GetDirectBufferAddress(bytebuffer);
}
