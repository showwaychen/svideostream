#ifndef LOG_H_
#define LOG_H_
#include "base/logging.h"
#define  SVPALYERLOG_TAG  "openglcapturejni"

#define ALOG(sev) \
	LOG_SEVERITY_PRECONDITION(rtc::sev) \
	rtc::LogMessage(__FILE__, __LINE__, rtc::sev, SVPALYERLOG_TAG).stream()

#define LOGV \
	ALOG(LS_VERBOSE)
#define LOGI \
	ALOG(LS_INFO)
#define LOGW \
	ALOG(LS_WARNING)
#define LOGE \
	ALOG(LS_ERROR)
#define LOGD \
	ALOG(LS_INFO)

class CCommonSetting
{
public:
	static void SetLogLevel(int level)
	{
		rtc::LogMessage::SetMinLogSeverity((rtc::LoggingSeverity)level);
		rtc::LogMessage::LogToDebug((rtc::LoggingSeverity)rtc::LogMessage::GetMinLogSeverity());
	}

};
#endif /* LOG_H_ */
