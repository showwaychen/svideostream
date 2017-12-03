#ifndef AUDIO_ENCODER_BASE_H_
#define AUDIO_ENCODER_BASE_H_
#include <stdint.h>
#include "Common.h"
#include <memory>

class CAudioEncoderBase
{
public:
	class AudioFrame
	{
		int m_nSamplesize;
		int m_nChannels;
		int n_nFrameSize;
		std::unique_ptr<int8_t> m_pFrameData;
		int64_t m_nPts;
	public:

		AudioFrame(int samplesize, int channels, int framesize)
		{
			m_nSamplesize = samplesize;
			m_nChannels = channels;
			n_nFrameSize = framesize;
			int nsize = n_nFrameSize * m_nSamplesize * m_nChannels;
			m_pFrameData.reset(new int8_t[nsize]);
		}
		void FillData(uint8_t* data, int nsize, int64_t pts)
		{
			m_nPts = pts;
			memcpy(m_pFrameData.get(), data, nsize);
		}
		int64_t GetPts()
		{
			return m_nPts;
		}
		int8_t* GetFrameData()
		{
			return m_pFrameData.get();
		}

	};
	class IEncodedCallBack
	{
	public:
		virtual void OnAudioEncodedData(uint8_t* data, int nsize, int64_t pts) = 0;
	};
protected:
	int m_nSamplerate;
	int m_nSamplesize;
	int m_nChannels;
	int m_nBitrate;
	int m_nFrameSampleSize;
	IEncodedCallBack* m_pCallBack = nullptr;
public:
	void SetEncodedCallBack(IEncodedCallBack *callback)
	{
		m_pCallBack = callback;
	}
	void SetAudioParams(int nsamplerate, int nsamplesize, int nchannel, int bitrate, int framesize)
	{
		m_nSamplerate = nsamplerate;
		m_nSamplesize = nsamplesize;
		m_nBitrate = bitrate;
		m_nFrameSampleSize = framesize;
		m_nChannels = nchannel;
	}

	virtual int StartEncode() = 0;
	virtual int OpenEncoder() = 0;
	virtual int CloseEncoder() = 0;
	virtual int PushData(uint8_t *audiodata, int nsize, int64_t pts) = 0;
	virtual sAacCodecInfo GetCodecInfo() = 0;
	virtual ~CAudioEncoderBase()
	{

	}
};
#endif 