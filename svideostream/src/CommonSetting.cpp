#include "CommonSetting.h"

void CCommonSetting::SetLogLevel(int level)
{
	rtc::LogMessage::SetMinLogSeverity((rtc::LoggingSeverity)level);
	rtc::LogMessage::LogToDebug((rtc::LoggingSeverity)rtc::LogMessage::GetMinLogSeverity());
}
