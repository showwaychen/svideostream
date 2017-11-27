#include "StatsCollector.h"
#include "Common.h"
#include "SVideoStream.h"



CStatsCollector::CStatsCollector(CSVideoStream* pc):
m_cStatsReportVideoEncoder(CStatsReport::kStatsReportVideoEncoder),
m_cStatsReportImageProcess(CStatsReport::kStatsReportImageProcess),
m_cStatsReportRtmpLive(CStatsReport::kStatsReportRtmpLive)
{
	m_pPc = pc;
}

void CStatsCollector::UpdateStats()
{
	m_cStatsReportVideoEncoder.Clear();
	m_cStatsReportRtmpLive.Clear();
	m_cStatsReportImageProcess.Clear();
}

void CStatsCollector::GetStats(StatsReports* reports)
{
	if (m_pPc == nullptr)
	{
		return;
	}
	if (m_pPc->m_pVideoEncoder != nullptr)
	{
		m_cStatsReportVideoEncoder.AddInt(CStatsReport::kSVNVideoAvgEncodeMs, m_pPc->m_pVideoEncoder->GetRunTimeInfo().m_nEncodeAvgTimeMs);
		m_cStatsReportVideoEncoder.AddInt(CStatsReport::kSVNVideoEncQueueFrameNums, m_pPc->m_pVideoEncoder->GetRunTimeInfo().m_nBufferRemainNum);
		reports->push_back(&m_cStatsReportVideoEncoder);

	}

	m_cStatsReportImageProcess.AddInt(CStatsReport::kSVNVideoAvgImageProcessTime, m_pPc->m_nAvgPreProcessTimeMs);
	m_cStatsReportVideoEncoder.AddInt(CStatsReport::kSVNVideoAvgInputFps, m_pPc->m_nAvg_fps);
	if (m_pPc->m_eStreamType == ST_LIVE && m_pPc->m_pRtmpPublish != nullptr)
	{
		m_cStatsReportRtmpLive.AddInt(CStatsReport::kSVNLiveAQueueFrameNums, m_pPc->m_pRtmpPublish->GetRuntimeInfo().m_nAudioFramesNum);
		m_cStatsReportRtmpLive.AddInt(CStatsReport::kSVNLiveVQueueFrameNums, m_pPc->m_pRtmpPublish->GetRuntimeInfo().m_nVideoFrameNum);
		//m_cStatsReportRtmpLive.AddString(CStatsReport::kSVNLiveSendBandwidth, "600kps");
		reports->push_back(&m_cStatsReportRtmpLive);
	}
	reports->push_back(&m_cStatsReportImageProcess);
}

CStatsCollector::~CStatsCollector()
{

}
