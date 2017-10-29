#include "CommonSetting_JniWrap.h"
#include "../src/CommonSetting.h"


static JNINativeMethod ls_nm[] = {
		{ "nativeSetLogLevel", "(I)V", reinterpret_cast<void*>(
		&CCommonSetting_JniWrap::nativeSetLogLevel) }
};
CRegisterNativeM CCommonSetting_JniWrap::s_registernm("cn/cxw/svideostreamlib/CommonSetting", ls_nm, ARRAYSIZE(ls_nm));
void JNICALL CCommonSetting_JniWrap::nativeSetLogLevel(JNIEnv *env, jclass thiz, jint level)
{
	CCommonSetting::SetLogLevel(level);
}
