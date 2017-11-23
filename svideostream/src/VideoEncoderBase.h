#ifndef VIDEO_ENCODER_BASE_H_
#define VIDEO_ENCODER_BASE_H_
#include <stdint.h>
#include "Common.h"
#include "Log.h"
#include "MediaConfig.h"

//source format only supports i420
class CVideoEncoderBase
{
public:
	class VideoFrame
	{
		uint8_t *m_pData = nullptr;
		int m_nWidth = 0;
		int m_nHeight = 0;
		uint64_t m_nPts;
	public:
		static void FreeFun(void* data)
		{
			VideoFrame *ins = (VideoFrame*)data;
			delete ins;
		}
		VideoFrame(int nwidth, int nheight)
		{
			m_nWidth = nwidth;
			m_nHeight = nheight;
			int nsize = m_nWidth * m_nHeight * 3 / 2;
			m_pData = new uint8_t[nsize];
		}
		int64_t GetPts()
		{
			return m_nPts;
		}
		uint8_t* GetFrameData()
		{
			return m_pData;
		}
		void FillData(uint8_t* data, int nsize, int64_t pts)
		{
			m_nPts = pts;
			memcpy(m_pData, data, nsize);
		}
		~VideoFrame()
		{
			if (m_pData != nullptr)
			{
				delete m_pData;
			}
		}
	};
	class IEncodedCallBack
	{
	public:
		virtual void OnVideoEncodedData(uint8_t* data, int nsize, int64_t pts, int64_t dts) = 0;
	};
protected:
	int m_nWidth;
	int m_nHeight;
	int m_nBitrate;
	int m_nFps;
	int m_nGop;
	bool m_bReqKeyFrame = false;

	CMediaConfig m_H264Configs;
	IEncodedCallBack* m_pCallBack = nullptr;

	static int StringToProfile(const std::string& profile);
public:
	static const char kProfile[];
	static const char kPreset[];
	static const char kTune[];
	static const char kRcMethod[];
	void SetEncodedCallBack(IEncodedCallBack *callback)
	{
		m_pCallBack = callback;
	}
	void SetGop(int gop)
	{
		m_nGop = gop;
		LOGI << "gop = "<<m_nGop;

	}
	void ReqKeyFrame()
	{
		m_bReqKeyFrame = true;
	}
	void SetVideoParams(int width, int height, int bitrate, int fps)
	{
		m_nWidth = width;
		m_nHeight = height;
		m_nBitrate = bitrate;
		m_nFps = fps;
	}
	void SetConfigs(CMediaConfig configs)
	{
		m_H264Configs = configs;
	}
	virtual int OpenEncoder() = 0;
	virtual int StartEncode() = 0;
	virtual int CloseEncoder() = 0;
	virtual int PushData(uint8_t *imagedata, int nsize, int64_t pts) = 0;
	virtual sH264CodecInfo GetCodecInfo() = 0;

	virtual ~CVideoEncoderBase()
	{

	}
};


#endif 