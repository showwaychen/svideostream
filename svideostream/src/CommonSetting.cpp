#include "CommonSetting.h"
#if defined(LEAKTRACER)
#include "MemoryTrace.hpp"
#include <fstream>
#endif
#include "Log.h"
void CCommonSetting::SetLogLevel(int level)
{
	rtc::LogMessage::SetMinLogSeverity((rtc::LoggingSeverity)level);
	rtc::LogMessage::LogToDebug((rtc::LoggingSeverity)rtc::LogMessage::GetMinLogSeverity());

}

void CCommonSetting::StartLeakMemDetect()
{
#if  defined(LEAKTRACER)
	leaktracer::MemoryTrace::GetInstance().startMonitoringAllThreads();

#endif

}
void CCommonSetting::StopLeakMemDetect()
{
#if  defined(LEAKTRACER)
	//leaktracer::MemoryTrace::GetInstance().stopMonitoringAllocations();
	leaktracer::MemoryTrace::GetInstance().stopAllMonitoring();
	std::ofstream oleaks;
	oleaks.open("/storage/emulated/0/leaks.out", std::ios_base::out);
	if (oleaks.is_open())
		leaktracer::MemoryTrace::GetInstance().writeLeaks(oleaks);
	else
		LOGE << "Failed to write to \"leaks.out\"\n";

#endif

}