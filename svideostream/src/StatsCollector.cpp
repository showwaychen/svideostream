#include "StatsCollector.h"



CStatsCollector::CStatsCollector(CSVideoStream* pc):
m_cStatsReportVideoEncoder(CStatsReport::kStatsReportVideoEncoder),
m_cStatsReportImageProcess(CStatsReport::kStatsReportImageProcess),
m_cStatsReportRtmpLive(CStatsReport::kStatsReportRtmpLive)
{

}

void CStatsCollector::UpdateStats()
{

}

void CStatsCollector::GetStats(StatsReports* reports)
{
	m_cStatsReportVideoEncoder.AddString(CStatsReport::kSVNVideoAvgEncodeMs, "50ms");
	m_cStatsReportVideoEncoder.AddInt(CStatsReport::kSVNVideoEncQueueFrameNums, 50);
	m_cStatsReportImageProcess.AddInt(CStatsReport::kSVNVideoAvgImageProcessTime, 50);
	m_cStatsReportVideoEncoder.AddInt(CStatsReport::kSVNVideoAvgInputFps, 25);
	m_cStatsReportRtmpLive.AddInt(CStatsReport::kSVNLiveAQueueFrameNums, 40);
	m_cStatsReportRtmpLive.AddInt(CStatsReport::kSVNLiveVQueueFrameNums, 40);
	m_cStatsReportRtmpLive.AddString(CStatsReport::kSVNLiveSendBandwidth, "600kps");
	reports->push_back(&m_cStatsReportVideoEncoder);
	reports->push_back(&m_cStatsReportImageProcess);
	reports->push_back(&m_cStatsReportRtmpLive);
}

CStatsCollector::~CStatsCollector()
{

}
