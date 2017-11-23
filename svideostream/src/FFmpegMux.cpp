#include "FFmpegMux.h"
#include "Log.h"
#include "H264AacUtils.h"
//#define SAVE_H264_FILE
#ifdef SAVE_H264_FILE
static FILE *pH264File = NULL;
#endif


void CFFmpegMux::AddVideoStream()
{
	AVCodec **codec = &m_pVideoCodec;
	enum AVCodecID codec_id = m_pAVOutFmt->video_codec;
	AVCodecContext *c;

	// find the encoder
	*codec = avcodec_find_encoder(codec_id);
	if (NULL == (*codec)) {
		LOGW<<"Could not find encoder for "<< avcodec_get_name(codec_id);
		return;
	}
	m_pVideoStream = avformat_new_stream(m_pAVFmtCtx, *codec);
	if (NULL == m_pVideoStream) {
		LOGE<<"Could not allocate stream!";
		return;
	}

	m_pVideoStream->index  = m_pAVFmtCtx->nb_streams - 1;
	c = m_pVideoStream->codec;

	c->codec_id = codec_id;
	c->bit_rate = m_H264Info.m_nBitrate;
	c->width = m_H264Info.m_nWidth;
	c->height = m_H264Info.m_nHeight;
	c->time_base.num = 1;
	c->time_base.den = m_H264Info.m_nFps;
	m_pVideoStream->time_base.num = 1;
	m_pVideoStream->time_base.den = 10000;
	c->gop_size = m_H264Info.m_nGop;
	c->pix_fmt = AV_PIX_FMT_YUV420P; // FFmpeg MP4 H264 only support YUV420P


	//h264

	c->profile = FF_PROFILE_H264_BASELINE;
	c->level = 21;
	c->thread_type = FF_THREAD_FRAME;
	c->keyint_min = m_H264Info.m_nFps + 1;
	c->rc_max_rate = m_H264Info.m_nBitrate;

	// Some formats want stream headers to be separate.
	if (m_pAVFmtCtx->oformat->flags & AVFMT_GLOBALHEADER) {
		c->flags |= CODEC_FLAG_GLOBAL_HEADER;
	}
	if (c->extradata) {
		av_freep(&(c->extradata));
		c->extradata = NULL;
		c->extradata_size = 0;
	}
	LOGD << "add vidoe stream successfully  video timebase den = " << m_pVideoStream->time_base.den << " num = " << m_pVideoStream->time_base.num << " index = " << m_pVideoStream->index;

}

void CFFmpegMux::AddAudioStream()
{
	AVCodec **codec = &m_pAudioCodec;
	enum AVCodecID codec_id = m_pAVOutFmt->audio_codec;
	AVCodecContext *c;

	*codec = avcodec_find_encoder(codec_id);
	if (NULL == (*codec)) {
		LOGW<<"Could not find encoder for  "<< avcodec_get_name(codec_id);
		return ;
	}
	m_pAudioStream = avformat_new_stream(m_pAVFmtCtx, *codec);
	if (NULL == m_pAudioStream) {
		LOGE<<"Could not allocate stream!";
		return;
	}

	m_pAudioStream->index = m_pAVFmtCtx->nb_streams - 1;
	//not useful default time_base is samplerate 
	//m_pAudioStream->time_base.num = 1;
	//m_pAudioStream->time_base.den = 10000;
	c = m_pAudioStream->codec;

	c->sample_fmt = AV_SAMPLE_FMT_S16;
	c->sample_rate = m_AacInfo.m_nSampleRate;
	c->channels = m_AacInfo.m_nChannels;
	c->channel_layout = av_get_default_channel_layout(c->channels);
	//m_pAudioStream->time_base.num = 1;
	//m_pAudioStream->time_base.den = 10000;
	// Some formats want stream headers to be separate.
	if (m_pAVFmtCtx->oformat->flags & AVFMT_GLOBALHEADER) {
		c->flags |= CODEC_FLAG_GLOBAL_HEADER;
	}
	if (c->extradata) {
		av_freep(&(c->extradata));
		c->extradata_size = 0;
	}
	LOGD << "add audio stream successfully  audio timebase den = " << m_pAudioStream->time_base.den << " num = " << m_pAudioStream->time_base.num << " index = " << m_pAudioStream->index;


}

int CFFmpegMux::OnAVSpecdata()
{
	if (m_bWaitVideoSpecData && ((m_bAudioEnable && m_bWaitAudioSpecData) || !m_bAudioEnable))
	{
		// Write the stream header, if any.
		LOGI << "write header";
		int nRet = avformat_write_header(m_pAVFmtCtx, NULL);
		if (nRet < 0) {
			LOGE << "Error occurred when opening output file: ! av_err2str(nRet)";
			return -1;
		}
	}
	return 0;
}

void CFFmpegMux::MemberReset()
{
	m_StartOk = false;
	m_bWaitAudioSpecData = false;
	m_bWaitVideoSpecData = false;
	m_nLastVideoPts = 0;
}



static void ffmpeglog_callback(void* avcl, int level, const char* pFormat, va_list vl)
{
	char buf[256] = { 0 };
	vsnprintf(buf, 255, pFormat, vl);
	LOGD << "ffmpegLog" << buf;
}
int CFFmpegMux::StartMux()
{
	MemberReset();
	av_register_all();
	av_log_set_callback(&ffmpeglog_callback);
	av_log_set_level( AV_LOG_DEBUG );
	avformat_alloc_output_context2(&m_pAVFmtCtx, NULL, NULL, "test.mp4");
	if (NULL == m_pAVFmtCtx) {
		LOGD<<"Could not deduce output format from file extension: using MPEG!";
		avformat_alloc_output_context2(&m_pAVFmtCtx, NULL, "mpeg", m_strFileName.c_str());
	}
	if (NULL == m_pAVFmtCtx) {
		LOGE<<"FFmpeg avformat_alloc_output_context2 failed!";
		return -1;
	}

	m_pAVOutFmt = m_pAVFmtCtx->oformat;

	//av_dict_set(&m_pAVFmtCtx->metadata, "copyright", m_strCopyRight.c_str(), 0);

	// Video
	m_pVideoStream = NULL;
	m_pVideoCodec = NULL;
	if (AV_CODEC_ID_NONE != m_pAVOutFmt->video_codec) {
		if (AV_CODEC_ID_H264 != m_pAVOutFmt->video_codec) {
			LOGW << " video_codec ID=" << m_pAVOutFmt->video_codec<<", video_codec ID = " << AV_CODEC_ID_H264;
		}
		m_pAVOutFmt->video_codec = AV_CODEC_ID_H264;
		AddVideoStream();
	}
	else {
		LOGE<<"FFmpeg do not have video stream!";
		return -1;
	}
	if (NULL == m_pVideoStream) {
		LOGE<<"FFmpeg add video stream failed!";
		return -1;
	}

	av_init_packet(&m_pktVideo);
	m_pktVideo.stream_index = m_pVideoStream->index;

	// Audio
	m_pAudioStream = NULL;
	if (m_bAudioEnable) {
		m_pAudioCodec = NULL;
		if (AV_CODEC_ID_NONE != m_pAVOutFmt->audio_codec) {
			if (AV_CODEC_ID_AAC != m_pAVOutFmt->audio_codec) {
				//LOGW("ORZ audio_codec ID=%d, audio_codec ID=%d.", pCtx->pAVOutFmt->audio_codec, AV_CODEC_ID_AAC);
			}
			m_pAVOutFmt->audio_codec = AV_CODEC_ID_AAC;
			AddAudioStream();
		}
		else {
			LOGE<<"FFmpeg do not have audio stream!";
			return -1;
		}
		if (NULL == m_pAudioStream) {
			LOGE<<"FFmpeg add audio stream failed!";
			return -1;
		}

		av_init_packet(&m_pktAudio);
		m_pktAudio.flags |= AV_PKT_FLAG_KEY;
		m_pktAudio.stream_index = m_pAudioStream->index;
	}

	remove(m_strFileName.c_str());
	int nRet = 0;
	// Open the output file, if needed
	if (!(m_pAVOutFmt->flags & AVFMT_NOFILE)) {
		nRet = avio_open(&m_pAVFmtCtx->pb, m_strFileName.c_str(), AVIO_FLAG_WRITE);
		if (nRet < 0) {
			LOGE<<"Could not open "<< m_strFileName <<": "<<"av_err2str(nRet)";
			return -1;
		}
	}

	//// Write the stream header, if any.
	//nRet = avformat_write_header(m_pAVFmtCtx, NULL);
	//if (nRet < 0) {
	//	LOGE<<"Error occurred when opening output file: ! av_err2str(nRet)";
	//	return -1;
	//}
#ifdef SAVE_H264_FILE
	if (NULL == pH264File) {
		pH264File = fopen("/storage/emulated/0/svideostream.h264", "wb+");
		if (pH264File == nullptr)
		{
			LOGE << "open h264 file failed";
		}
	}
#endif
	m_StartOk = true;
	LOGI<<"MuxFFmpeg start ok.";
	return 0;
}

void CFFmpegMux::StopMux()
{
	if (!m_StartOk)
	{
		LOGI << "MuxFFmpeg hasn't started";
		return;
	}
	LOGI<<"MuxFFmpeg stop.";
	int nRet = 0;
	rtc::CritScope autolock(&m_Mux_cs);

	nRet = av_write_trailer(m_pAVFmtCtx);

	if (NULL != m_pVideoStream) {
		if (NULL != m_pVideoStream->codec->extradata) {
			av_freep(&(m_pVideoStream->codec->extradata));
			m_pVideoStream->codec->extradata_size = 0;
		}
	}

	if (NULL != m_pAudioStream) {
		if (NULL != m_pAudioStream->codec->extradata) {
			av_freep(&(m_pAudioStream->codec->extradata));
			m_pAudioStream->codec->extradata_size = 0;
		}
	}

	if (!(m_pAVOutFmt->flags & AVFMT_NOFILE)) {
		// Close the output file.
		avio_close(m_pAVFmtCtx->pb);
	}
	avformat_free_context(m_pAVFmtCtx);
#ifdef SAVE_H264_FILE
	if (NULL != pH264File) {
		fclose(pH264File);
		pH264File = NULL;
	}
#endif

	return ;
}


int CFFmpegMux::WriteVideoData(uint8_t* vdata, int nsize, int64_t npts, int64_t ndts)
{
	if (vdata == nullptr || nsize < 5)
	{
		LOGE<<"video data error";
		return -1;
	}
#ifdef SAVE_H264_FILE
	if (NULL != pH264File) {
		fwrite(vdata, nsize, 1, pH264File);
		fflush(pH264File);
	}
#endif

	/*if (!m_H264Info.m_bAnnexB) {
		ConvertAVCCToAnnexB(vdata, nsize);
	}*/
	if (nullptr == m_pVideoStream->codec->extradata && npts == -1)
	{
		// TOFIX: check SPS PPS
		if (0x0 == vdata[0] && 0x0 == vdata[1] && 0x1 == vdata[2])
		{

		}
		else if (0x0 == vdata[0] && 0x0 == vdata[1] && 0x0 == vdata[2] && 0x1 == vdata[3]) 
		{
		}
		else
		{
			LOGE << "Video Encode failed! vdata[0]." << vdata[0] << " vdata[1]"<< vdata[1];
			return -1;
		}

		// http://aviadr1.blogspot.com.au/2010/05/h264-extradata-partially-explained-for.html
		// FFmpeg/libavformat/movenc.c , extradata => vos_data => avcc
		if (NULL == (m_pVideoStream->codec->extradata = (uint8_t *)av_mallocz((nsize + FF_INPUT_BUFFER_PADDING_SIZE) * sizeof(uint8_t)))) {
			LOGE<<"Malloc memory failed.";
			return -1;
		}
		m_pVideoStream->codec->extradata_size = nsize;
		memcpy(m_pVideoStream->codec->extradata, vdata, nsize * sizeof(uint8_t));
		m_bWaitVideoSpecData = true;
		return OnAVSpecdata();
	}

	if (npts < 0) {
		return -1;
	}

	m_pktVideo.data = vdata;
	m_pktVideo.size = nsize;
	m_pktVideo.stream_index = m_pVideoStream->index;
	m_pktVideo.pos = -1;
	m_nLastVideoPts =  m_pktVideo.pts = av_rescale_q(npts, (AVRational){ 1, 1000 }, m_pVideoStream->time_base);
	m_pktVideo.dts = av_rescale_q(ndts, (AVRational){ 1, 1000 }, m_pVideoStream->time_base);
	m_pktAudio.duration = m_pktVideo.pts - m_nLastVideoPts;
	m_nLastVideoPts = m_pktVideo.pts;
	LOGI << "video packet size = " << nsize << " dts = pts = " << m_pktVideo.pts<< "  inputvideo pts = "<<npts<<" index = "<<m_pktVideo.stream_index;
	if (CH264AacUtils::IsKeyFrame(m_pktVideo.data, m_pktVideo.size, m_H264Info.m_bAnnexB)) {
		m_pktVideo.flags |= AV_PKT_FLAG_KEY;
		LOGI << "video packet is keyframe";
	}
	else {
		m_pktVideo.flags = 0;
	}
	rtc::CritScope autolock(&m_Mux_cs);

	int nRet = av_interleaved_write_frame(m_pAVFmtCtx, &m_pktVideo);
	av_free_packet(&m_pktVideo);

	if (0 != nRet) {
		LOGE<<"av_write_frame failed. av_err2str(nRet)";
	}
	return nRet;
}

int CFFmpegMux::WriteAudioData(uint8_t* vdata, int nsize, int64_t npts)
{
	if (NULL == m_pAudioStream) {
		return -1;
	}
	if (NULL == m_pAudioStream->codec->extradata && npts == -1) {
		if (NULL == (m_pAudioStream->codec->extradata = (uint8_t *)av_mallocz((nsize + FF_INPUT_BUFFER_PADDING_SIZE) * sizeof(uint8_t)))) {
			LOGE<<"Malloc memory failed.";
			return -1;
		}
		m_pAudioStream->codec->extradata_size = nsize;
		memcpy(m_pAudioStream->codec->extradata, vdata, nsize * sizeof(uint8_t));
		m_bWaitAudioSpecData = true;
		return OnAVSpecdata();
	}

	if (npts < 0) {
		return -1;
	}
	if (!m_bWaitAudioSpecData || !m_bWaitVideoSpecData)
	{
		return 0;
	}
	LOGI << "audio packet size = " << nsize << " npt =  " << npts<<" time_base = "<<m_pAudioStream->time_base.den;

	m_pktAudio.data = vdata;
	m_pktAudio.size = nsize;
	m_pktAudio.stream_index = m_pAudioStream->index;
	m_pktAudio.pos = -1;
	m_pktAudio.dts = m_pktAudio.pts = av_rescale_q(npts, (AVRational){ 1, 1000 }, m_pAudioStream->time_base);
	m_pktAudio.duration = 1024;
	LOGI << "audio packet size = " << nsize << " dts = pts = " << m_pktAudio.pts << "  inputaudiopts = " << npts << " index = " << m_pktAudio.stream_index;
	rtc::CritScope autolock(&m_Mux_cs);
	int nRet = av_interleaved_write_frame(m_pAVFmtCtx, &m_pktAudio);
	av_free_packet(&m_pktAudio);

	if (0 != nRet) {
		LOGE<<"av_write_frame failed. %s av_err2str(nRet)";
	}
	return nRet;
}
