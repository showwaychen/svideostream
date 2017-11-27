#ifndef STATS_COLLECTOR_H_
#define STATS_COLLECTOR_H_
#include<vector>
#include "StatsReport.h"
class CSVideoStream;
class CStatsCollector
{
	CStatsReport m_cStatsReportVideoEncoder;
	CStatsReport m_cStatsReportImageProcess;
	CStatsReport m_cStatsReportRtmpLive;

	CSVideoStream* m_pPc = nullptr;

public:
	explicit CStatsCollector(CSVideoStream* pc);
	void UpdateStats();
	void GetStats(StatsReports* reports);
	~CStatsCollector();
};
#endif 
