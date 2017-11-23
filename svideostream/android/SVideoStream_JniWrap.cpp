#include "SVideoStream_JniWrap.h"
#include <jni.h>
#include"android/jnihelper/jni_onload.h"
#include"android/jnihelper/jni_helpers.h"
#include "../src/CommonSetting.h"
#include "../src/Log.h"
#include "../src/AudioEncoderFdkaac.h"
#include "../src/VideoEncoderX264.h"
#include "VideoEncoderMediaCodec.h"
#include "../src/StatsReport.h"
JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
	JNIEnv *env = NULL;

	SetJavaVM(vm);
	LOGI << "version : "<<VERSION;
	CCommonSetting::SetLogLevel(rtc::LS_SENSITIVE);
	if (0 == on_JNI_OnLoad(GetJavaVM(), JNI_VERSION_1_6))
	{
		LOGD<<"on_JNI_OnLoad success";
	}
	else
	{
		return -1;
	}
	return JNI_VERSION_1_6;
}
static JNINativeMethod ls_nm[] = {
	{ "nativeCreateObject", "()J", reinterpret_cast<void*>(
	&CSVideoStream_JniWrap::newinstance) },
	{ "nativeStart", "()I", reinterpret_cast<void*>(
	&CSVideoStream_JniWrap::nativeStart) },
	{ "nativeStop", "()I", reinterpret_cast<void*>(
	&CSVideoStream_JniWrap::nativeStop) },
	{ "nativeSetStreamType", "(I)V", reinterpret_cast<void*>(
	&CSVideoStream_JniWrap::nativeSetStreamType) },
	{ "nativeSetSrcType", "(I)V", reinterpret_cast<void*>(
	&CSVideoStream_JniWrap::nativeSetSrcType) },
	{ "nativeSetSrcImageParams", "(IIII)V", reinterpret_cast<void*>(
	&CSVideoStream_JniWrap::nativeSetSrcImageParams) },
	{ "nativeSetDstParams", "(II)V", reinterpret_cast<void*>(
	&CSVideoStream_JniWrap::nativeSetDstParams) },
	{ "nativeSetVideoEncoderType", "(I)V", reinterpret_cast<void*>(
	&CSVideoStream_JniWrap::nativeSetEncoderType) },
	{ "nativeSetVideoEncodeParams", "(II)V", reinterpret_cast<void*>(
	&CSVideoStream_JniWrap::nativeSetVideoEncodeParams) },
	{ "nativeSetAudioParams", "(IIII)V", reinterpret_cast<void*>(
	&CSVideoStream_JniWrap::nativeSetAudioParams) },
	{ "nativeSetRotation", "(I)V", reinterpret_cast<void*>(
	&CSVideoStream_JniWrap::nativeSetRotation) },
	{ "nativeSetPublishUrl", "(Ljava/lang/String;)V", reinterpret_cast<void*>(
	&CSVideoStream_JniWrap::nativeSetPublishUrl) },
	{ "nativeSettingSet", "(ILcn/cxw/svideostreamlib/MediaConfig;)V", reinterpret_cast<void*>(
	&CSVideoStream_JniWrap::nativeSettingSet) },
	{ "nativeSetRecordPath", "(Ljava/lang/String;)V", reinterpret_cast<void*>(
	&CSVideoStream_JniWrap::nativeSetRecordPath) },
	{ "nativeGetStats", "()[Lcn/cxw/svideostreamlib/StatsReport;", reinterpret_cast<void*>(
	&CSVideoStream_JniWrap::nativeGetStats) },
	{ "nativeSetAudioEnable", "(Z)V", reinterpret_cast<void*>(
	&CSVideoStream_JniWrap::nativeSetAudioEnable) },
	{ "nativeGetState", "()I", reinterpret_cast<void*>(
	&CSVideoStream_JniWrap::nativeGetState) },
	{ "nativeInputVideoData", "(Ljava/nio/ByteBuffer;IJ)I", reinterpret_cast<void*>(
	&CSVideoStream_JniWrap::nativeInputVideoData) },
	{ "nativeInputAudioData", "([BIJ)I", reinterpret_cast<void*>(
	&CSVideoStream_JniWrap::nativeInputAudioData) },
	{ "nativeDestroy", "()V", reinterpret_cast<void*>(
	&CSVideoStream_JniWrap::native_Destroy) }
};
CSVideoStream_JniWrap* CSVideoStream_JniWrap::GetInst(JNIEnv* jni, jobject j_object)
{
	jclass j_class = jni->GetObjectClass(j_object);
	jfieldID nativeobject_id = jni->GetFieldID(j_class, "m_NativeObject", "J");
	MYCHECK_EXCEPTION(jni, "GetInst failed");
	jlong j_p = jni->GetLongField(j_object, nativeobject_id);
	MYCHECK_EXCEPTION(jni, "GetInst failed");

	return reinterpret_cast<CSVideoStream_JniWrap*>(j_p);
}

CRegisterNativeM CSVideoStream_JniWrap::s_registernm("cn/cxw/svideostreamlib/SVideoStream", ls_nm, ARRAYSIZE(ls_nm));
CSVideoStream_JniWrap::CSVideoStream_JniWrap(JNIEnv *env, jobject thiz)
{
	m_jThiz = env->NewGlobalRef(thiz); 
	jclass oclass = env->GetObjectClass(m_jThiz);
	m_jEventCallback = env->GetMethodID(oclass, "nativeEventCallback", "(II)V");
	m_pVideoStream = new CSVideoStream;
	m_pVideoStream->SetEventCallback(this);
}

jlong JNICALL CSVideoStream_JniWrap::newinstance(JNIEnv *env, jobject thiz)
{
	CSVideoStream_JniWrap* instance = new CSVideoStream_JniWrap(env, thiz);
	return jlongFromPointer((void*)instance);
}

jint JNICALL CSVideoStream_JniWrap::nativeStart(JNIEnv *env, jobject thiz)
{
	CSVideoStream_JniWrap *instance = CSVideoStream_JniWrap::GetInst(env, thiz); 
	if (instance != nullptr)
	{
		return instance->StartStream();
	}
	return -1;
}

jint JNICALL CSVideoStream_JniWrap::nativeStop(JNIEnv *env, jobject thiz)
{
	CSVideoStream_JniWrap *instance = CSVideoStream_JniWrap::GetInst(env, thiz);
	if (instance != nullptr)
	{
		return instance->StopStream();
	}
	return -1;

}

void JNICALL CSVideoStream_JniWrap::nativeSetStreamType(JNIEnv *env, jobject thiz, jint type)
{
	CSVideoStream_JniWrap *instance = CSVideoStream_JniWrap::GetInst(env, thiz);
	if (instance != nullptr)
	{
		instance->m_pVideoStream->SetStreamType((StreamType)type);
	}
}

void JNICALL CSVideoStream_JniWrap::nativeSetSrcType(JNIEnv *env, jobject thiz, jint type)
{
	CSVideoStream_JniWrap *instance = CSVideoStream_JniWrap::GetInst(env, thiz);
	if (instance != nullptr)
	{
		instance->m_pVideoStream->SetSrcType((SrcDataType)type);
	}
}

void JNICALL CSVideoStream_JniWrap::nativeSetSrcImageParams(JNIEnv *env, jobject thiz, jint format, jint stride, int width, int height)
{
	CSVideoStream_JniWrap *instance = CSVideoStream_JniWrap::GetInst(env, thiz);
	if (instance != nullptr)
	{
		instance->m_pVideoStream->SetSrcImageParams((ImageFormat)format, stride, width, height);
	}
}

void JNICALL CSVideoStream_JniWrap::nativeSetDstParams(JNIEnv *env, jobject thiz, int width, int height)
{
	CSVideoStream_JniWrap *instance = CSVideoStream_JniWrap::GetInst(env, thiz);
	if (instance != nullptr)
	{
		instance->m_pVideoStream->SetDstSize(width, height);
	}
}

void JNICALL CSVideoStream_JniWrap::nativeSetEncoderType(JNIEnv* env, jobject thiz, jint type)
{
	CSVideoStream_JniWrap *instance = CSVideoStream_JniWrap::GetInst(env, thiz);
	if (instance != nullptr)
	{
		instance->SetVideoEncoderType((VideoEncoderType)type);
	}
}

void JNICALL CSVideoStream_JniWrap::nativeSetVideoEncodeParams(JNIEnv *env, jobject thiz, int bitrate, int fps)
{
	CSVideoStream_JniWrap *instance = CSVideoStream_JniWrap::GetInst(env, thiz);
	if (instance != nullptr)
	{
		instance->m_pVideoStream->SetVideoEncodeParams(bitrate, fps);
	}
}

void JNICALL CSVideoStream_JniWrap::nativeSetAudioParams(JNIEnv *env, jobject thiz, jint samplerate, jint channels, jint samplesize, jint bitrate)
{
	CSVideoStream_JniWrap *instance = CSVideoStream_JniWrap::GetInst(env, thiz);
	if (instance != nullptr)
	{
		instance->m_pVideoStream->SetAudioParams(samplerate, channels, samplesize, bitrate);
	}
}

void JNICALL CSVideoStream_JniWrap::nativeSetRotation(JNIEnv *env, jobject thiz, int rotation)
{
	CSVideoStream_JniWrap *instance = CSVideoStream_JniWrap::GetInst(env, thiz);
	if (instance != nullptr)
	{
		instance->m_pVideoStream->SetRotation((RotationMode)rotation);
	}
}

void JNICALL CSVideoStream_JniWrap::nativeSetPublishUrl(JNIEnv *env, jobject thiz, jstring url)
{
	CSVideoStream_JniWrap *instance = CSVideoStream_JniWrap::GetInst(env, thiz);
	if (instance != nullptr)
	{
		instance->m_pVideoStream->SetPushlishUrl(env->GetStringUTFChars(url, JNI_FALSE));
	}
}

void JNICALL CSVideoStream_JniWrap::nativeSetRecordPath(JNIEnv *env, jobject thiz, jstring filename)
{
	CSVideoStream_JniWrap *instance = CSVideoStream_JniWrap::GetInst(env, thiz);
	if (instance != nullptr)
	{
		instance->m_pVideoStream->SetRecordPath(env->GetStringUTFChars(filename, JNI_FALSE));
	}
}

void JNICALL CSVideoStream_JniWrap::nativeSetAudioEnable(JNIEnv *env, jobject thiz, jboolean isenable)
{
	CSVideoStream_JniWrap *instance = CSVideoStream_JniWrap::GetInst(env, thiz);
	if (instance != nullptr)
	{
		instance->m_pVideoStream->SetEnableAudio(isenable);
	}
}

jint JNICALL CSVideoStream_JniWrap::nativeInputVideoData(JNIEnv *env, jobject thiz, jobject bytebuffer, int size, long pts)
{
	CSVideoStream_JniWrap *instance = CSVideoStream_JniWrap::GetInst(env, thiz);
	if (instance != nullptr)
	{
		uint8_t *pData = (uint8_t*)env->GetDirectBufferAddress(bytebuffer);
		int ret = instance->m_pVideoStream->InputVideoData(pData, size, pts);
		/*uint8_t *pData = (uint8_t *)env->GetByteArrayElements(data, 0);
		int ret = instance->m_pVideoStream->InputVideoData(pData, size, pts);
		env->ReleaseByteArrayElements(data, (jbyte *)pData, 0);*/
		return ret;
	}
	return -1;
}

jint JNICALL CSVideoStream_JniWrap::nativeInputAudioData(JNIEnv *env, jobject thiz, jbyteArray data, int size, long pts)
{
	CSVideoStream_JniWrap *instance = CSVideoStream_JniWrap::GetInst(env, thiz);
	if (instance != nullptr)
	{
		uint8_t *pData = (uint8_t *)env->GetByteArrayElements(data, 0);
		int ret = instance->m_pVideoStream->InputAudioData(pData, size, pts);
		env->ReleaseByteArrayElements(data, (jbyte *)pData, 0);
		return ret;
	}
	return -1;
}

void JNICALL CSVideoStream_JniWrap::native_Destroy(JNIEnv *env, jobject thiz)
{
	CSVideoStream_JniWrap *instance = GetInst(env, thiz);
	if (instance != NULL)
	{
		delete instance;
		LOGD << "destroy a videostream instance";
	}
}

int CSVideoStream_JniWrap::StartStream()
{
	if (m_pVideoStream->GetAudioEnable() && m_pAudioEncoder == nullptr)
	{
		m_pAudioEncoder = new CAudioEncoderFdkaac();
	}
	if (m_pVideoEncoder == nullptr)
	{
		if (m_eVideoEncoderType == H264ENCODER_X264)
		{
			m_pVideoEncoder = new CVideoEncoderX264();
		}
		else
		{
			m_pVideoEncoder = new CVideoEncoderMediaCodec;
		}
	}
	m_pVideoEncoder->SetConfigs(m_H264Configs);
	m_pVideoStream->SetAudioCodec(m_pAudioEncoder);
	m_pVideoStream->SetVideoCodec(m_pVideoEncoder);
	return m_pVideoStream->StartStream();
}

int CSVideoStream_JniWrap::StopStream()
{
	return m_pVideoStream->StopStream();
}

CSVideoStream_JniWrap::~CSVideoStream_JniWrap()
{
	SAFE_DELETE(m_pVideoStream);
	SAFE_DELETE(m_pAudioEncoder);
	SAFE_DELETE(m_pVideoEncoder);
	GetEnv(GetJavaVM())->DeleteGlobalRef(m_jThiz);
}

void CSVideoStream_JniWrap::OnStreamEvent(StreamEvent event, StreamError error)
{
	AttachThreadScoped attachthread(GetJavaVM());
	JNIEnv* jenv = attachthread.env();
	jenv->CallVoidMethod(m_jThiz, m_jEventCallback, (jint)event, (jint)error);
}
class MediaConfigs_JniWrapper
{
	JNIEnv* m_jni;

	jobject m_jKeySetIterator;
	jmethodID m_jIteratorHasNext;
	jmethodID m_jIteratorNext;

	jmethodID m_jEnterGetKey;
	jmethodID m_jEnterGetValue;
public:
	MediaConfigs_JniWrapper(JNIEnv* jni, jobject j_configs)
	{
		m_jni = jni;
		jmethodID jgetkeysetid = GetMethodID(jni, GetObjectClass(jni, j_configs), "getKeySetIte", "()Ljava/util/Iterator;");
		m_jKeySetIterator = jni->CallObjectMethod(j_configs, jgetkeysetid);

		m_jIteratorHasNext = GetMethodID(jni, GetObjectClass(jni, m_jKeySetIterator), "hasNext", "()Z");
		m_jIteratorNext = GetMethodID(jni, GetObjectClass(jni, m_jKeySetIterator), "next", "()Ljava/lang/Object;");
	}
	void GetMediaConfigs(CMediaConfig& configs)
	{
		while (m_jni->CallBooleanMethod(m_jKeySetIterator, m_jIteratorHasNext) == JNI_TRUE) {
			jobject entry = m_jni->CallObjectMethod(m_jKeySetIterator, m_jIteratorNext);
			jmethodID get_key = GetMethodID(m_jni,
				GetObjectClass(m_jni, entry), "getKey", "()Ljava/lang/Object;");
			jstring j_key = reinterpret_cast<jstring>(
				m_jni->CallObjectMethod(entry, get_key));
			MYCHECK_EXCEPTION(m_jni, "error during CallObjectMethod");
			jmethodID get_value = GetMethodID(m_jni,
				GetObjectClass(m_jni, entry), "getValue", "()Ljava/lang/Object;");
			jstring j_value = reinterpret_cast<jstring>(
				m_jni->CallObjectMethod(entry, get_value));
			MYCHECK_EXCEPTION(m_jni, "error during CallObjectMethod");
			configs.PutConfig(JavaToStdString(m_jni, j_key), JavaToStdString(m_jni, j_value));
		}
	}
	~MediaConfigs_JniWrapper(){}
};
void JNICALL CSVideoStream_JniWrap::nativeSettingSet(JNIEnv *env, jobject thiz, jint key, jobject value)
{
	CSVideoStream_JniWrap *instance = GetInst(env, thiz);
	if (instance != NULL)
	{
		if (key == SKV_H264ENCODERCONFIG)
		{
			MediaConfigs_JniWrapper configwraper(env, value);
			configwraper.GetMediaConfigs(instance->m_H264Configs);
			/*std::map<std::string, std::string>::iterator ite = configs.m_Configs.begin();
			while (ite != configs.m_Configs.end())
			{
			LOGD << ite->first << "  " << ite->second;
			ite++;
			}*/
		}
	}
	
}

class ReportsToJavaArray
{
	const StatsReports& m_Reports;
	const ScopedGlobalRef<jclass> j_stats_report_class_;
	const jmethodID j_stats_report_ctor_;

	const ScopedGlobalRef<jclass> j_value_class_;
	const jmethodID j_value_ctor_;

	JNIEnv* m_jni;

	jobjectArray ValuesToJava(JNIEnv* jni, const std::vector<CStatsReport::Value>& values) {
		jobjectArray j_values = jni->NewObjectArray(
			values.size(), *j_value_class_, NULL);
		int i = 0;
		for (const auto& it : values) {
			ScopedLocalRefFrame local_ref_frame(jni);
			// Should we use the '.name' enum value here instead of converting the
			// name to a string?
			jstring j_name = JavaStringFromStdString(jni, it.display_name());
			jstring j_value = JavaStringFromStdString(jni, it.ToString());
			jobject j_element_value =
				jni->NewObject(*j_value_class_, j_value_ctor_, j_name, j_value);
			jni->SetObjectArrayElement(j_values, i++, j_element_value);
		}
		return j_values;
	}
public:
	ReportsToJavaArray(JNIEnv* jni,  StatsReports& reports) :
		m_Reports(reports), j_stats_report_class_(jni, FindClass(jni, "cn/cxw/svideostreamlib/StatsReport")),
		j_stats_report_ctor_(GetMethodID(jni, *j_stats_report_class_, "<init>", "(Ljava/lang/String;[Lcn/cxw/svideostreamlib/StatsReport$Value;)V")), 
		j_value_class_(jni, FindClass(jni, "cn/cxw/svideostreamlib/StatsReport$Value")),
		j_value_ctor_(GetMethodID(jni, *j_value_class_, "<init>", "(Ljava/lang/String;Ljava/lang/String;)V")),
		m_jni(jni)
	{

	}
	jobjectArray GetReportsJavaArray()
	{
		jobjectArray reports_array = m_jni->NewObjectArray(
			m_Reports.size(), *j_stats_report_class_, NULL);
		int i = 0;
		for (const auto* report : m_Reports)
		{
			ScopedLocalRefFrame local_ref_frame(m_jni);
			jstring j_type = JavaStringFromStdString(m_jni, report->TypeToString());
			jobjectArray j_values = ValuesToJava(m_jni, report->values());
			jobject j_report = m_jni->NewObject(*j_stats_report_class_,
				j_stats_report_ctor_,
				j_type,
				j_values);
			m_jni->SetObjectArrayElement(reports_array, i++, j_report);
		}
		return reports_array;
	}
};
jobjectArray JNICALL CSVideoStream_JniWrap::nativeGetStats(JNIEnv *env, jobject thiz)
{
	CSVideoStream_JniWrap *instance = CSVideoStream_JniWrap::GetInst(env, thiz);
	if (instance != nullptr)
	{
		StatsReports reports;
		instance->m_pVideoStream->GetStatsReports(&reports);
		if (reports.size() > 0)
		{
			ReportsToJavaArray j_reports(env, reports);
			return j_reports.GetReportsJavaArray();
		}
		return nullptr;
	}
	return nullptr;
}

jint JNICALL CSVideoStream_JniWrap::nativeGetState(JNIEnv *env, jobject thiz)
{
	CSVideoStream_JniWrap *instance = CSVideoStream_JniWrap::GetInst(env, thiz);
	if (instance != nullptr)
	{
		return instance->m_pVideoStream->GetState();
	}
	return StreamState_NONE;
}
