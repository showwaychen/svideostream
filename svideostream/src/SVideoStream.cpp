#include "SVideoStream.h"
#include"Log.h"
#include"libyuv.h"
#include"base/timeutils.h"
#include"base/nullsocketserver.h"
#include "CommonSetting.h"
//#define SAVE_YUV_FILE
#ifdef SAVE_YUV_FILE
static FILE *pyuvFile = NULL;
#endif
//#define SAVE_PCM_FILE
#ifdef SAVE_PCM_FILE
static FILE *ppcmFile = NULL;
#endif
//#define SAVE_AAC_FILE
#ifdef SAVE_AAC_FILE
static FILE *paacFile = NULL;
#endif

//#define NOTIFY_MESSAGELOOPER(event, errorid) CMainMessageLooper::getInst()->InvokeAsyn<void>(SVS_FROM_HERE, rtc::Bind(&CSVideoStream::StreamEventNotify, this, event, errorid));
//#define NOTIFY_MESSAGELOOPER(event, errorid) CMainMessageLooper::getInst()->InvokeAsyn<void>(SVS_FROM_HERE, rtc::Bind(&CSVideoStream::StreamEventNotify, this, event, errorid));
CSVideoStream::CSVideoStream() :m_pRtmpPublish(new CRtmpLive),
m_pFFmpegMux(new CFFmpegMux),
m_cStatsCollector(this),
m_tWorkThread(new rtc::NullSocketServer())
{
	m_tWorkThread.SetName("CSVideoStream", nullptr);
	m_tWorkThread.Start(nullptr);

}

int CSVideoStream::StartStream(bool isasyn)
{
	if (m_eState == StreamState_STARTED || m_eState == StreamState_STARTING)
	{
		LOGW<<"has started already";
		return 0;
	}

	LOGI << "video params srcstride = " << m_nVideoSrcStride << " srcwidth = " << m_nVideoSrcWidth <<
		" srcheight = " << m_nVideoSrcHeight <<" format = "<< m_eVideoSrcFormat<< " rotation = " << m_eVideoRotation << " dstwidth = " <<
		m_nVideoDstWidth << " dstheight = " << m_nVideoDstHeight << " bitrate = " << m_nVideoBitrate <<
		" fps = " << m_nVideoFrameRate;
	if (!m_bAudioEnable)
	{
		LOGI << "audio disable";
	}
	else
	{
		LOGI << "audio params samplerate = " << m_nAudioSamplerate << "  samplesize = " << m_nAudioSampleSize <<
			" channels = " << m_nAudioChannels << " bitrate = " << m_nAudioBitrate;
	}

	if (m_eStreamType == ST_RECORD)
	{
		LOGI << "stream type is record record path = " << m_strFileName;
	}
	else
	{
		LOGI << "stream type is live live url = " << m_strPublishUrl;
	}
	
	if (m_pVideoEncoder == nullptr)
	{
		LOGE << "video encoder is null";
		return -1;
	}
	if (m_bAudioEnable && m_pAudioEncoder == nullptr)
	{
		LOGE << "audio encoder is null";
		return -1;
	}
	if (isasyn)
	{
		mAsyncInvoker.AsyncInvoke<int>(SVS_FROM_HERE, &m_tWorkThread, rtc::Bind(&CSVideoStream::StartStream, this, false));
		return 0;
	}
	m_eState = StreamState_STARTING;
	
	ResetMembers();
	m_pVideoEncoder->SetEncodedCallBack(this);
	m_pVideoEncoder->SetVideoParams(m_nVideoDstWidth, m_nVideoDstHeight, m_nVideoBitrate, m_nVideoFrameRate);
	m_pVideoEncoder->SetGop(m_nVideoFrameRate * m_nVideoKeyFrameInterval);
	LOGI << "video encoder params width = " << m_nVideoDstWidth << " height = " << m_nVideoDstHeight << " bitrate = " << m_nVideoBitrate << " fps = " << m_nVideoFrameRate;
	if (m_pVideoEncoder->OpenEncoder() != 0)
	{
		ASYNNOTIFY(SE_StreamFailed, kSE_VideoEncoderOpenedFailed);
		m_eState = StreamState_NONE;
		LOGE << "video encoder open error";
		return -1;
	}
	if (m_bAudioEnable)
	{
		m_pAudioEncoder->SetEncodedCallBack(this);
		m_pAudioEncoder->SetAudioParams(m_nAudioSamplerate, m_nAudioSampleSize, m_nAudioChannels, 64000, 1024);
		LOGI << "audio encoder params samplerate = " << m_nAudioSamplerate << " samplesize = " << m_nAudioSampleSize << " channels = " << m_nAudioChannels << " bitrate = 64000";
		if (m_pAudioEncoder->OpenEncoder() != 0)
		{
			m_eState = StreamState_NONE;
			ASYNNOTIFY(SE_StreamFailed, kSE_AudioeEncoderOpenedFailed);
			m_pVideoEncoder->CloseEncoder();
			m_pAudioEncoder->CloseEncoder();
			LOGE << "audio encoder open error";
			return -1;
		}
	}
	
	if (m_eStreamType == ST_LIVE)
	{
		m_bIsLiveConnected = false;
		m_pRtmpPublish->SetUrl(m_strPublishUrl);
		m_pRtmpPublish->SetEventObserver(this);
		m_pRtmpPublish->SetH264CodecInfo(m_pVideoEncoder->GetCodecInfo());
		if (m_bAudioEnable)
		{
			m_pRtmpPublish->SetEnableAudio(true);
			m_pRtmpPublish->SetAacCodecInfo(m_pAudioEncoder->GetCodecInfo());
		}
		else
		{
			m_pRtmpPublish->SetEnableAudio(false);
		}
		if (0 != m_pRtmpPublish->StartLive())
		{
			m_eState = StreamState_NONE;
			ASYNNOTIFY(SE_StreamFailed, kSE_UnknowError);
			m_pVideoEncoder->CloseEncoder();
			if (m_pAudioEncoder != nullptr)
			{
				m_pAudioEncoder->CloseEncoder();
			}

			return -1;
		}
	}
	else if (m_eStreamType == ST_RECORD)
	{
		m_pFFmpegMux->SetFileName(m_strFileName);
		m_pFFmpegMux->SetH264CodecInfo(m_pVideoEncoder->GetCodecInfo());
		if (m_bAudioEnable)
		{
			m_pFFmpegMux->SetEnableAudio(true);
			m_pFFmpegMux->SetAacCodecInfo(m_pAudioEncoder->GetCodecInfo());
		}
		else
		{
			m_pFFmpegMux->SetEnableAudio(false);
		}
		if (0 != m_pFFmpegMux->StartMux())
		{
			m_eState = StreamState_NONE;
			ASYNNOTIFY(SE_StreamFailed, kSE_UnknowError);
			m_pVideoEncoder->CloseEncoder();
			if (m_pAudioEncoder != nullptr)
			{
				m_pAudioEncoder->CloseEncoder();
			}
			return -1;
		}
	}
	m_pVideoEncoder->StartEncode();
	if (m_pAudioEncoder != nullptr)
	{
		m_pAudioEncoder->StartEncode();
	}
#ifdef SAVE_PCM_FILE
	if (NULL == ppcmFile) {
		ppcmFile = fopen("/storage/emulated/0/svideostream.pcm", "wb+");
		if (ppcmFile == nullptr)
		{
			LOGE << "open pcm file failed";
		}
	}
#endif
#ifdef SAVE_YUV_FILE
	if (NULL == pyuvFile) {
		pyuvFile = fopen("/storage/emulated/0/svideostream.yuv", "wb+");
		if (pyuvFile == nullptr)
		{
			LOGE << "open yuv file failed";
		}
	}
#endif
#ifdef SAVE_AAC_FILE
	if (NULL == paacFile) {
		paacFile = fopen("/storage/emulated/0/svideostream.aac", "wb+");
		if (paacFile == nullptr)
		{
			LOGE << "open aac file failed";
		}
	}
#endif
	if (m_eStreamType == ST_RECORD)
	{
		m_eState = StreamState_STARTED;
		ASYNNOTIFY(SE_StreamStarted, kSE_NoneError);
	}
	return 0;
}

int CSVideoStream::StopStream(bool isasyn)
{
	LOGD << "StopStream";
	if (!(m_eState == StreamState_STARTED || m_eState == StreamState_STARTING))
	{
		LOGD << "stream hadn't started";
		return 0;
	}
	if (m_eState == StreamState_STOPPING)
	{
		LOGD << "stream is stopping";
		return 0;
	}
	if (isasyn)
	{
		//????
		LOGD << "StopStream isasyn = true";
		mAsyncInvoker.AsyncInvoke<int>(SVS_FROM_HERE, &m_tWorkThread, rtc::Bind(&CSVideoStream::StopStream, this, false));
		return 0;
	}
	m_eState = StreamState_STOPPING;

	if (m_pVideoEncoder != nullptr)
	{
		LOGD << "close video encoder start ";
		m_pVideoEncoder->CloseEncoder();
		LOGD << "close video encoder end";
	}
	if (m_pAudioEncoder != nullptr)
	{
		LOGD << "close audio encoder start";
		m_pAudioEncoder->CloseEncoder();
		LOGD << "close audio encoder end";

	}
	if (m_eStreamType == ST_RECORD)
	{
		if (m_pFFmpegMux != nullptr)
		{
			LOGD << "stop mux start ";
			m_pFFmpegMux->StopMux();
			LOGD << "stop mux end";

		}
	}
	else if (m_eStreamType == ST_LIVE)
	{
		if (m_pRtmpPublish != nullptr)
		{
			LOGD<<"stop live start ";
			m_pRtmpPublish->StopLive();
			LOGD << "stop live end";

		}
	}
#ifdef SAVE_YUV_FILE
	if (NULL != pyuvFile) {
		fclose(pyuvFile);
		pyuvFile = NULL;
	}
#endif
#ifdef SAVE_PCM_FILE
	if (NULL != ppcmFile) {
		fclose(ppcmFile);
		ppcmFile = NULL;
	}
#endif
#ifdef SAVE_AAC_FILE
	if (NULL != paacFile) {
		fclose(paacFile);
		paacFile = NULL;
	}
#endif
	m_eState = StreamState_STOPED;
	m_pI420ImageTmp.reset(nullptr);
	m_pScaleImageTmp.reset(nullptr);
	LOGD << "notify stream stopped";
	CCommonSetting::StopLeakMemDetect();
	ASYNNOTIFY(SE_StreamStopped, kSE_NoneError);
	return 0;
}

int CSVideoStream::InputVideoData(uint8_t* data, int nszie, int64_t npts)
{
	if (m_pVideoEncoder == nullptr)
	{
		LOGE << "no video encoder";
		return -1;
	}
	if (m_eStreamType == ST_LIVE && !m_bIsLiveConnected)
	{
		LOGD << "video came but rtmp hasn't connected";
		return 0;
	}
	//LOGD << "video frame size = " << nszie;
	UpdateAvgFps(npts);
	m_nCurPts = npts;
	if (m_eSrcDataType == SDT_IMAGEDATA)
	{
		memcpy(m_pSrcImageData.get(), data, nszie);
		int64_t earilertime = rtc::TimeMillis();
		ImagePreProcess();
		UpdateAvgPreProcessTime(rtc::TimeSince(earilertime));
		int ret =  m_pVideoEncoder->PushData(m_pDstImageData.get(), m_nDstImageDataSize, m_nCurPts);
		if (ret != 0)
		{
			LOGW << "video encoder buffer is full .failed to push data";
		}
	}
	return 0;
}

int CSVideoStream::InputAudioData(uint8_t* data, int nszie, int64_t npts)
{
	if (m_pAudioEncoder == nullptr)
	{
		LOGE << "no audio encoder";
		return -1;
	}
	if (m_eStreamType == ST_LIVE && !m_bIsLiveConnected)
	{
		LOGD << "audio came but rtmp hasn't connected";
		return 0;
	}
#ifdef SAVE_PCM_FILE
	if (NULL != ppcmFile) {
		fwrite(data, nszie, 1, ppcmFile);
		fflush(ppcmFile);
	}
#endif
	return m_pAudioEncoder->PushData(data, nszie, npts);

}

int CSVideoStream::SetWaterMarkData(uint8_t *data, int stride, int width, int height, int format)
{
	return 0;
}

CSVideoStream::~CSVideoStream()
{
	StopStream();
	m_tWorkThread.Stop();
}

void CSVideoStream::OnVideoEncodedData(uint8_t* data, int nsize, int64_t pts, int64_t dts)
{
	//LOGI << "video data size = " << nsize << "pts = " << pts;
	if (m_eStreamType == ST_RECORD)
	{
		m_pFFmpegMux->WriteVideoData(data, nsize, pts, dts);
	}
	else if (m_eStreamType == ST_LIVE)
	{
		m_pRtmpPublish->SendVideoData(data, nsize, pts);
	}
}

void CSVideoStream::OnAudioEncodedData(uint8_t* data, int nsize, int64_t pts)
{
#ifdef SAVE_AAC_FILE
	if (pts >= 0)
	{
		if (NULL != paacFile) {
			fwrite(data, nsize, 1, paacFile);
			fflush(paacFile);
		}
	}
#endif
	//LOGI << "audio data size = " << nsize << "pts = " << pts;
	if (m_eStreamType == ST_RECORD)
	{
		m_pFFmpegMux->WriteAudioData(data, nsize, pts);
	}
	else if (m_eStreamType == ST_LIVE)
	{
		m_pRtmpPublish->SendAudioData(data, nsize, pts);
	}
}

void CSVideoStream::ResetMembers()
{
	m_nAvg_fps = 0;
	m_nLastFrameTimeMs = 0;
	m_nAvgPreProcessTimeMs = 0;

}

void CSVideoStream::ImagePreProcess()
{

	int uoffset = m_nVideoSrcWidth * m_nVideoSrcHeight;
	int voffset = uoffset * 5 / 4;
	uint8_t*ProcessSrcData = m_pSrcImageData.get();
	uint8_t*ProcessDstData = nullptr;
	int yuvsize = m_nVideoSrcHeight* m_nVideoSrcWidth * 3 / 2;
	if (m_pI420ImageTmp == nullptr)
	{
		m_pI420ImageTmp.reset(new uint8_t[m_nVideoSrcHeight* m_nVideoSrcWidth * 3 / 2]);
	}
	ProcessDstData = m_pI420ImageTmp.get();
	ConvertToI420(ProcessSrcData, ProcessDstData, m_nVideoSrcWidth,
		ProcessDstData + uoffset, m_nVideoSrcWidth / 2,
		ProcessDstData + voffset, m_nVideoSrcWidth / 2,
		m_nVideoSrcStride, m_nVideoSrcWidth, m_nVideoSrcHeight, m_eVideoSrcFormat);

	bool bScale = false;
	int nScaleWidth = m_nVideoSrcWidth, nScaleHeight = m_nVideoSrcHeight;
	if (m_eVideoRotation == kRotate90 || m_eVideoRotation == kRotate270)
	{
		if (m_nVideoSrcHeight != m_nVideoDstWidth || m_nVideoSrcWidth != m_nVideoDstHeight)
		{
			nScaleWidth = m_nVideoDstHeight;
			nScaleHeight = m_nVideoDstWidth;

			bScale = true;
		}
	}
	else
	{
		if (m_nVideoSrcHeight != m_nVideoDstHeight || m_nVideoSrcWidth != m_nVideoDstWidth)
		{
			nScaleWidth = m_nVideoDstWidth;
			nScaleHeight = m_nVideoDstHeight;
			bScale = true;
		}
	}
	uint8_t *ImageDataTmp = m_pI420ImageTmp.get();
	if (bScale)
	{
		LOGI << "need scale";
		if (m_pScaleImageTmp == nullptr)
		{
			m_pScaleImageTmp.reset(new uint8_t[nScaleWidth* nScaleHeight * 3 / 2]);
		}
		int scaleuoffset = nScaleWidth * nScaleHeight;
		int scalevoffset = scaleuoffset * 5 / 4;
		libyuv::I420Scale(m_pI420ImageTmp.get(), m_nVideoSrcWidth,
			m_pI420ImageTmp.get() + uoffset, m_nVideoSrcWidth / 2,
			m_pI420ImageTmp.get() + voffset, m_nVideoSrcWidth / 2,
			m_nVideoSrcWidth, m_nVideoSrcHeight,
			m_pScaleImageTmp.get(), nScaleWidth,
			m_pScaleImageTmp.get() + scaleuoffset, nScaleWidth / 2,
			m_pScaleImageTmp.get() + scalevoffset, nScaleWidth / 2,
			nScaleWidth, nScaleHeight,
			libyuv::kFilterBox);
		ImageDataTmp = m_pScaleImageTmp.get();
	}

	if (m_pDstImageData == nullptr)
	{
		m_nDstImageDataSize = m_nVideoDstWidth * m_nVideoDstHeight * 3 / 2;
		m_pDstImageData.reset(new uint8_t[m_nDstImageDataSize]);
	}
	if (m_eVideoRotation != kRotate0) {
		//LOGI << "need rotation srcwidth = " << nScaleWidth<<" srcheight = "<<nScaleHeight<<" dstwidth = "<<m_nVideoDstWidth<<" dstheight = "<<m_nVideoDstHeight;
		int scaleuoffset = nScaleWidth * nScaleHeight;
		int scalevoffset = scaleuoffset * 5 / 4;
		libyuv::I420Rotate(ImageDataTmp, nScaleWidth,
			ImageDataTmp + scaleuoffset, nScaleWidth / 2,
			ImageDataTmp + scalevoffset, nScaleWidth / 2,
			m_pDstImageData.get(), m_nVideoDstWidth,
			m_pDstImageData.get() + scaleuoffset, m_nVideoDstWidth / 2,
			m_pDstImageData.get() + scalevoffset, m_nVideoDstWidth / 2,
			nScaleWidth, nScaleHeight,
			(libyuv::RotationMode)m_eVideoRotation);
	}
	else
	{
		memcpy(m_pDstImageData.get(), ImageDataTmp, m_nDstImageDataSize);
	}
#ifdef SAVE_YUV_FILE
	if (NULL != pyuvFile) {
		fwrite(m_pSrcImageData, uoffset * 4, 1, pyuvFile);
		fflush(pyuvFile);
	}
#endif
}

int CSVideoStream::ConvertToI420(const uint8_t *src_y, uint8_t *dst_y, int dst_stride_y, uint8_t *dst_u, int dst_stride_u, uint8_t *dst_v, int dst_stride_v, int stride, int width, int height, ImageFormat format)
{
	int nYStride = stride;
	int nUVStride = stride;
	int nUVOffset = stride * height;

	switch (format) {
	case IMAGE_FORMAT_NV12:
		return libyuv::NV12ToI420(src_y, nYStride,
			src_y + nUVOffset, nUVStride,
			dst_y, dst_stride_y,
			dst_u, dst_stride_u,
			dst_v, dst_stride_v,
			width, height);
	case IMAGE_FORMAT_NV21:
		return libyuv::NV21ToI420(src_y, nYStride,
			src_y + nUVOffset, nUVStride,
			dst_y, dst_stride_y,
			dst_u, dst_stride_u,
			dst_v, dst_stride_v,
			width, height);

	case IMAGE_FORMAT_ARGB:
		return libyuv::ARGBToI420(src_y, stride,
			dst_y, dst_stride_y,
			dst_u, dst_stride_u,
			dst_v, dst_stride_v,
			width, height);
	case IMAGE_FORMAT_RGBA:
		return libyuv::RGBAToI420(src_y, stride,
			dst_y, dst_stride_y,
			dst_u, dst_stride_u,
			dst_v, dst_stride_v,
			width, height);
	case IMAGE_FORMAT_ABGR:
		return libyuv::ABGRToI420(src_y, stride,
			dst_y, dst_stride_y,
			dst_u, dst_stride_u,
			dst_v, dst_stride_v,
			width, height);
	case IMAGE_FORMAT_BGRA:
		return libyuv::BGRAToI420(src_y, stride,
			dst_y, dst_stride_y,
			dst_u, dst_stride_u,
			dst_v, dst_stride_v,
			width, height);
	default:
		break;
	}

	return 1;
}

void CSVideoStream::SetVideoEncodeParams(int bitrate, int fps)
{
	m_nVideoBitrate = bitrate;
	m_nVideoFrameRate = fps;
}

void CSVideoStream::SetAudioParams(int samplerate, int channels, int samplesize, int bitrate)
{
	m_nAudioBitrate = bitrate;
	m_nAudioChannels = channels;
	m_nAudioSamplerate = samplerate;
	m_nAudioSampleSize = samplesize;
}

void CSVideoStream::NotifyEvent(CRtmpLive::LiveEvent event, int nerror)
{
	if (event == CRtmpLive::LE_CONNECTED_SUCCESS)
	{
		m_eState = StreamState_STARTED;
		ASYNNOTIFY(SE_StreamStarted, kSE_NoneError);
		//NOTIFY_MESSAGELOOPER(SE_LiveConnected, kSE_NoneError);
		m_bIsLiveConnected = true;
	}
	else if (event == CRtmpLive::LE_NEED_KEYFRAME)
	{
		if (m_pVideoEncoder != nullptr)
		{
			m_pVideoEncoder->ReqKeyFrame();
		}
	}
	else if (event == CRtmpLive::LE_CONNECT_ERROR)
	{
		LOGW << "rtmp connected failed";
		ASYNNOTIFY(SE_StreamFailed, kSE_LiveConnectFailed);
	}
	else if (event == CRtmpLive::LE_LIVE_ERROR)
	{
		ASYNNOTIFY(SE_StreamFailed, kSE_LiveFailed);
	}
	else if (event == CRtmpLive::LE_DISCONNECTED)
	{
		ASYNNOTIFY(SE_LiveDisconnected, kSE_NoneError);
	}
}

void CSVideoStream::UpdateAvgFps(int64_t npts)
{
	static int fcount = 0;

	if (m_nLastFrameTimeMs == 0)
	{
		m_nLastFrameTimeMs = npts;
		fcount = 0;
		return;
	}
	int delta = npts - m_nLastFrameTimeMs;
	if (delta >= sCaluateTimeMs)
	{
		fcount++;
		m_nAvg_fps = (int)(fcount * 1000.0f / (float)delta);
		fcount = 0;
		m_nLastFrameTimeMs = npts;
	}
	else
	{
		fcount++;
	}
}

void CSVideoStream::UpdateAvgPreProcessTime(int64_t tms)
{
	m_nAvgPreProcessTimeMs = (m_nAvgPreProcessTimeMs + tms) / 2;
}

