#ifndef SVIDEO_STREAM_H_
#define SVIDEO_STREAM_H_
#include <stdint.h>
#include <string.h>
#include "VideoEncoderBase.h"
#include <string>
#include "RtmpLive.h"
#include "FFmpegMux.h"
#include "AudioEncoderBase.h"
#include "Common.h"
#include "H264AacUtils.h"
#include "StatsCollector.h"
#include"base/thread.h"
#include"base/asyncinvoker.h"
#include <memory>
class sInputVideoInfo
{

};

class sInputAudioInfo
{

};
class CSVideoStream : public CVideoEncoderBase::IEncodedCallBack, public CAudioEncoderBase::IEncodedCallBack, public CRtmpLive::ILiveEventObserver, public std::enable_shared_from_this<CSVideoStream>
{
	friend CStatsCollector;

#include "base/location.h"

#define SVS_FROM_HERE    rtc::Location(__FUNCTION__ , __FILE__ ":" STRINGIZE(__LINE__))
public:
	
	class IStreamEventObserver
	{
	public:
		virtual void OnStreamEvent(StreamEvent event, StreamError error) = 0;
		virtual ~IStreamEventObserver(){};
	};
private:
	StreamType m_eStreamType = ST_NONE;
	SrcDataType m_eSrcDataType = SDT_IMAGEDATA;
	StreamState m_eState = StreamState_NONE;

	//audio
	bool m_bAudioEnable = true;
	int m_nAudioBitrate = 0;
	int m_nAudioSamplerate;
	int m_nAudioChannels;
	int m_nAudioSampleSize;
	//video
	ImageFormat m_eVideoSrcFormat;
	RotationMode m_eVideoRotation;
	int m_nVideoSrcStride;
	int m_nVideoSrcWidth;
	int m_nVideoSrcHeight;
	int m_nVideoDstWidth;
	int m_nVideoDstHeight;
	int m_nVideoFrameRate;
	int m_nVideoBitrate;
	int m_nVideoKeyFrameInterval = 3;
	std::unique_ptr<uint8_t> m_pSrcImageData;
	int m_nSrcImageDataSize = 0;
	std::unique_ptr<uint8_t> m_pI420ImageTmp;
	std::unique_ptr<uint8_t> m_pScaleImageTmp;
	std::unique_ptr<uint8_t> m_pDstImageData;
	int m_nDstImageDataSize = 0;
	int64_t m_nCurPts = 0;
	//watermark
	uint8_t *m_pWaterMarkData = nullptr;


	//video encoder
	std::shared_ptr<CVideoEncoderBase> m_pVideoEncoder;
	//audio encoder
	std::shared_ptr<CAudioEncoderBase> m_pAudioEncoder;

	std::string m_strPublishUrl;
	std::unique_ptr<CRtmpLive> m_pRtmpPublish ;
	bool m_bIsLiveConnected = false;
	std::string m_strFileName;
	std::unique_ptr<CFFmpegMux> m_pFFmpegMux ;

	std::weak_ptr<IStreamEventObserver> m_pStreamEventObserver;

	//Statistics
	CStatsCollector m_cStatsCollector;
	//
	int m_nAvg_fps = 0;
	int64_t m_nLastFrameTimeMs = 0;
	const int sCaluateTimeMs = 500;
	void UpdateAvgFps(int64_t npts);
	int m_nAvgPreProcessTimeMs = 0;
	void UpdateAvgPreProcessTime(int64_t tms);

#define ASYNNOTIFY(event, errorid) mAsyncInvoker.AsyncInvoke<void>(SVS_FROM_HERE, &m_tWorkThread, rtc::Bind(&CSVideoStream::StreamEventNotify, this, event, errorid));
	rtc::Thread m_tWorkThread;
	rtc::AsyncInvoker mAsyncInvoker;
	
	void StreamEventNotify(StreamEvent event, StreamError error)
	{
		if (auto observer =  m_pStreamEventObserver.lock())
		{
			observer->OnStreamEvent(event, error);
		}
	}
	void ResetMembers();
	void ImagePreProcess();

	int ConvertToI420(const uint8_t *src_y, uint8_t *dst_y, int dst_stride_y,
		uint8_t *dst_u, int dst_stride_u, uint8_t *dst_v, int dst_stride_v,
		int stride, int width, int height,
		ImageFormat format);
	
public:
	CSVideoStream();
	StreamState GetState()
	{
		return m_eState;
	}
	void SetEnableAudio(bool enable)
	{
		m_bAudioEnable = enable;
	}
	bool GetAudioEnable()
	{
		return m_bAudioEnable;
	}
	void SetStreamType(StreamType stype)
	{
		m_eStreamType = stype;
	}
	void SetSrcType(SrcDataType sdtype)
	{
		m_eSrcDataType = sdtype;
	}
	void SetSrcImageParams(ImageFormat format, int stride, int width, int height)
	{
		m_eVideoSrcFormat = format;
		m_nVideoSrcStride = stride;
		m_nVideoSrcWidth = width;
		m_nVideoSrcHeight = height;

		m_pSrcImageData.reset(new uint8_t[CH264AacUtils::GetFrameSize(format, stride, height)]);
	}
	void SetDstSize(int nwidth, int nheight)
	{
		m_nVideoDstHeight = nheight;
		m_nVideoDstWidth = nwidth;
	}
	void SetRotation(RotationMode mode)
	{
		m_eVideoRotation = mode;
	}
	void SetVideoEncodeParams(int bitrate, int fps);
	void SetAudioParams(int samplerate, int channels, int samplesize, int bitrate);
	void SetPushlishUrl(const std::string& url)
	{
		m_strPublishUrl = url;
	}
	void SetRecordPath(const std::string& filename)
	{
		m_strFileName = filename;
	}
	int StartStream(bool isasyn = false);
	int StopStream(bool isasyn = false);
	int InputVideoData(uint8_t* data, int nszie, int64_t npts);
	int InputAudioData(uint8_t* data, int nszie, int64_t npts);
	int SetWaterMarkData(uint8_t *data, int stride, int width, int height, int format);

	void SetVideoCodec(std::shared_ptr<CVideoEncoderBase> pvencoder)
	{
		m_pVideoEncoder = pvencoder;
	}
	void SetAudioCodec(std::shared_ptr<CAudioEncoderBase> paencoder)
	{
		m_pAudioEncoder = paencoder;
	}
	void SetEventCallback(std::weak_ptr<IStreamEventObserver> observer)
	{
		m_pStreamEventObserver = observer;
	}
	void GetStatsReports(StatsReports* reports)
	{
		m_cStatsCollector.UpdateStats();
		m_cStatsCollector.GetStats(reports);
	}
	~CSVideoStream();

	virtual void OnVideoEncodedData(uint8_t* data, int nsize, int64_t pts, int64_t dts) override;

	virtual void OnAudioEncodedData(uint8_t* data, int nsize, int64_t pts) override;

	virtual void NotifyEvent(CRtmpLive::LiveEvent event, int nerror);

};
#endif 