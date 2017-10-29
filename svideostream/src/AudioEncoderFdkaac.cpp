#include "AudioEncoderFdkaac.h"
#include "Log.h"



bool CAudioEncoderFdkaac::OnAudioEncoderThread()
{
	AACENC_BufDesc in_buf = { 0 }, out_buf = { 0 };
	AACENC_InArgs in_args = { 0 };
	AACENC_OutArgs out_args = { 0 };
	int in_identifier = IN_AUDIO_DATA;
	int in_size, in_elem_size;
	int out_identifier = OUT_BITSTREAM_DATA;
	int out_size, out_elem_size;
	int read, i;
	uint8_t *in_ptr, *out_ptr;
	uint8_t outbuf[20480];
	AACENC_ERROR err;
	int64_t pts = 0;
	int input_size = m_nChannels * 2 * m_sInfo.frameLength;
	int16_t* convert_buf = new int16_t[ input_size / 2 ];

	//in param
	in_ptr = (uint8_t*)convert_buf;
	in_size = input_size;
	in_elem_size = 2;

	in_args.numInSamples = input_size / 2;
	in_buf.numBufs = 1;
	in_buf.bufs = (void**)&in_ptr;
	in_buf.bufferIdentifiers = &in_identifier;
	in_buf.bufSizes = &in_size;
	in_buf.bufElSizes = &in_elem_size;

	//out param
	out_ptr = outbuf;
	out_size = sizeof(outbuf);
	out_elem_size = 1;
	out_buf.numBufs = 1;
	out_buf.bufs = (void**)&out_ptr;
	out_buf.bufferIdentifiers = &out_identifier;
	out_buf.bufSizes = &out_size;
	out_buf.bufElSizes = &out_elem_size;
	//AudioSpecificConfig
	uint8_t aacspecdata[2] = { 0x12, 0x08 }; //aot:2, 44100:4 channel:1

	// Send spec data
	if (m_pCallBack != nullptr)
	{
		m_pCallBack->OnAudioEncodedData(aacspecdata, 2, -1);
	}
	while (!m_bAbort)
	{
		AudioFrame * audiodata = nullptr;
		if (!m_qFrameQ.PullData(&audiodata, true))
		{
			usleep(20000);
			continue;
		}
		memcpy(in_ptr, audiodata->GetFrameData(), input_size);
		pts = audiodata->GetPts();
		delete audiodata;

		if ((err = aacEncEncode(m_handle, &in_buf, &out_buf, &in_args, &out_args)) != AACENC_OK) {
			LOGE<<"Encoding failed";
			break;
		}
		if (out_args.numOutBytes != 0)
		{
			m_pCallBack->OnAudioEncodedData(outbuf, out_args.numOutBytes, pts);

		}
	}
	delete convert_buf;
	aacEncClose(&m_handle);
	LOGI << "Out aacencodethread.";
	return false;
}

CAudioEncoderFdkaac::CAudioEncoderFdkaac() :
m_hEncodeThread(this, &CAudioEncoderFdkaac::OnAudioEncoderThread, "aacencodethread")
{
	m_qFrameQ.SetMaxCount(3);
}

int CAudioEncoderFdkaac::OpenEncoder()
{
	switch (m_nChannels) {
	case 1: m_eMode = MODE_1;       break;
	case 2: m_eMode = MODE_2;       break;
	case 3: m_eMode = MODE_1_2;     break;
	case 4: m_eMode = MODE_1_2_1;   break;
	case 5: m_eMode = MODE_1_2_2;   break;
	case 6: m_eMode = MODE_1_2_2_1; break;
	default:
		LOGE<<"Unsupported WAV channels "<< m_nChannels;
		return -1;
	}
	if (aacEncOpen(&m_handle, 0, m_nChannels) != AACENC_OK) {
		LOGE<<"Unable to open encoder";
		return -1;
	}
	//音频对象类型
	AUDIO_OBJECT_TYPE aot = AOT_AAC_LC;
	if (aacEncoder_SetParam(m_handle, AACENC_AOT, aot) != AACENC_OK) {
		LOGE<<"Unable to set the AOT";
		return 1;
	}
	if (aacEncoder_SetParam(m_handle, AACENC_SAMPLERATE, m_nSamplerate) != AACENC_OK) {
		LOGE<<"Unable to set the AOT";
		return 1;
	}
	if (aacEncoder_SetParam(m_handle, AACENC_CHANNELMODE, m_eMode) != AACENC_OK) {
		LOGE<<"Unable to set the channel mode";
		return 1;
	}
	if (aacEncoder_SetParam(m_handle, AACENC_CHANNELORDER, 1) != AACENC_OK) {
		LOGE<<"Unable to set the wav channel order";
		return 1;
	}
	int bitrate = m_nBitrate;
	if (aacEncoder_SetParam(m_handle, AACENC_BITRATE, bitrate) != AACENC_OK) {
		LOGE<<"Unable to set the bitrate";
		return 1;
	}
	//use aac es data not with adts
	TRANSPORT_TYPE transmux = TT_MP4_RAW;

	if (aacEncoder_SetParam(m_handle, AACENC_TRANSMUX, transmux) != AACENC_OK) {
		LOGE<<"Unable to set the ADTS transmux";
		return 1;
	}
	/*int afterburner = 1;
	if (aacEncoder_SetParam(m_handle, AACENC_AFTERBURNER, afterburner) != AACENC_OK) {
		LOGE<<"Unable to set the afterburner mode";
		return 1;
	}*/

	if (aacEncEncode(m_handle, NULL, NULL, NULL, NULL) != AACENC_OK) {
		LOGE<<"Unable to initialize the encoder";
		return 1;
	}
	if (aacEncInfo(m_handle, &m_sInfo) != AACENC_OK) {
		LOGE<<"Unable to get the encoder info";
		return 1;
	}
	m_bEncoderOk = true;
	
	return 0;

}

int CAudioEncoderFdkaac::CloseEncoder()
{
	
	m_bAbort = true;
	m_qFrameQ.SetEnable(false);
	m_qFrameQ.Clear();
	m_hEncodeThread.Stop();
	LOGD << "FDKAAC has closed";
	return 0;
}

int CAudioEncoderFdkaac::PushData(uint8_t *audiodata, int nsize, int64_t pts)
{
	if (m_bAbort)
	{
		LOGE << "fdkaac has closed";
		return -1;
	}
	if (!m_bEncoderOk)
	{
		LOGE << "fdkaac encoder not open successfully";
		return -1;
	}
	//LOGI << " audio data size = " << nsize << " pts = " << pts;

	AudioFrame *frame = new AudioFrame(m_nSamplesize, m_nChannels, m_nFrameSampleSize);
	frame->FillData(audiodata, nsize, pts);
	if (!m_qFrameQ.PushData(frame, true))
	{
		delete frame;
		return -1;
	}
	return 0;
}

sAacCodecInfo CAudioEncoderFdkaac::GetCodecInfo()
{
	sAacCodecInfo info;
	info.m_nBitrate = m_nBitrate;
	info.m_nChannels = m_nChannels;
	info.m_nSampleRate = m_nSamplerate;
	info.m_nSampleSize = m_nSamplesize;
	info.m_nFrameSize = 1024;
	return info;
}

int CAudioEncoderFdkaac::StartEncode()
{
	m_bAbort = false;
	m_qFrameQ.SetEnable(true);
	m_hEncodeThread.Start();
	return 0;
}
