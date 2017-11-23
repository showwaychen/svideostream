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

class sInputVideoInfo
{

};

class sInputAudioInfo
{

};
class CSVideoStream : public CVideoEncoderBase::IEncodedCallBack, CAudioEncoderBase::IEncodedCallBack, CRtmpLive::ILiveEventObserver
{
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
	uint8_t* m_pSrcImageData = nullptr;
	int m_nSrcImageDataSize = 0;
	uint8_t* m_pI420ImageTmp = nullptr;
	uint8_t* m_pScaleImageTmp = nullptr;
	uint8_t* m_pDstImageData = nullptr;
	int m_nDstImageDataSize = 0;
	int64_t m_nCurPts = 0;
	//watermark
	uint8_t *m_pWaterMarkData = nullptr;


	//video encoder
	CVideoEncoderBase* m_pVideoEncoder = nullptr;
	//audio encoder
	CAudioEncoderBase* m_pAudioEncoder = nullptr;

	std::string m_strPublishUrl;
	CRtmpLive* m_pRtmpPublish = nullptr;
	bool m_bIsLiveConnected = false;
	std::string m_strFileName;
	CFFmpegMux* m_pFFmpegMux = nullptr;

	IStreamEventObserver *m_pStreamEventObserver = nullptr;

	CStatsCollector m_cStatsCollector;
	void StreamEventNotify(StreamEvent event, StreamError error)
	{
		if (m_pStreamEventObserver != nullptr)
		{
			m_pStreamEventObserver->OnStreamEvent(event, error);
		}
	}
	void InitParams();
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
		if (m_pSrcImageData != nullptr)
		{
			delete m_pSrcImageData;
			m_pSrcImageData = nullptr;
		}
		m_pSrcImageData = new uint8_t[CH264AacUtils::GetFrameSize(format, stride, height)];
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
	int StartStream();
	int StopStream();
	int InputVideoData(uint8_t* data, int nszie, int64_t npts);
	int InputAudioData(uint8_t* data, int nszie, int64_t npts);
	int SetWaterMarkData(uint8_t *data, int stride, int width, int height, int format);

	void SetVideoCodec(CVideoEncoderBase* pvencoder)
	{
		m_pVideoEncoder = pvencoder;
	}
	void SetAudioCodec(CAudioEncoderBase* paencoder)
	{
		m_pAudioEncoder = paencoder;
	}
	void SetEventCallback(IStreamEventObserver *observer)
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