#include "VideoEncoderX264.h"
#include"base/timeutils.h"
#include <memory>


bool CVideoEncoderX264::OnEncodeThread()
{
	x264_nal_t *pNals = NULL;
	int nNals = -1;
	x264_picture_t picIn;
	x264_picture_t picOut;
	x264_picture_init(&picIn);
	x264_picture_init(&picOut);
	picIn.img.i_csp = X264_CSP_I420;
	picIn.img.i_plane = 3;
	picIn.img.i_stride[0] = m_nWidth;
	picIn.img.i_stride[1] = m_nWidth / 2;
	picIn.img.i_stride[2] = m_nWidth / 2;

	//encode header
	x264_nal_t *pHeaderNals = NULL;
	int nHeaderNals = -1;
	int nHeaderNalsSize = x264_encoder_headers(m_pX264, &pHeaderNals, &nHeaderNals);
	LOGI << "header nals is " << nHeaderNals;
	if (nHeaderNalsSize < 0 || nHeaderNals < 2) {
		LOGE<<"X264 get headers failed.";
		return false;
	}
	if (m_pCallBack == nullptr)
	{
		LOGE << "x264 callback is null";
		return false;
	}
	m_pCallBack->OnVideoEncodedData(pHeaderNals[0].p_payload, pHeaderNals[0].i_payload + pHeaderNals[1].i_payload, -1, -1);

	int nUOffset = m_nWidth * m_nHeight;
	int nVOffset = m_nWidth * m_nHeight + m_nWidth * m_nHeight / 4;
	while (!m_bAbort)
	{
		std::unique_ptr<VideoFrame> imagedata;
		if (!m_qVideoFrameQ.PullData(imagedata, true))
		{
			usleep(20000);
			continue;
		}
		picIn.i_pts = imagedata->GetPts();
		picIn.img.plane[0] = imagedata->GetFrameData();
		picIn.img.plane[1] = imagedata->GetFrameData() + nUOffset;
		picIn.img.plane[2] = imagedata->GetFrameData() + nVOffset;
		picIn.i_type = X264_TYPE_AUTO;
		if (m_bReqKeyFrame)
		{
			m_bReqKeyFrame = false;
			picIn.i_type = X264_TYPE_KEYFRAME;
		}
		// Encode
		int64_t earilertime = rtc::TimeMillis();
		int nNalsSize = x264_encoder_encode(m_pX264, &pNals, &nNals, &picIn, &picOut);
		UpdateEncodeTime(rtc::TimeSince(earilertime));
		if (nNalsSize < 0) {
			LOGE<<"Error encoding frame.";
			break;
		}
		
		if (nNalsSize > 0) {
			m_pCallBack->OnVideoEncodedData(pNals[0].p_payload, nNalsSize, picOut.i_pts, picOut.i_dts);
		}

		
	}
	if (m_bAbort)
	{
		// Flush delayed frames
		while (x264_encoder_delayed_frames(m_pX264) != 0)
		{
			LOGE << "x264_encoder_delayed_frames ";
			int64_t earilertime = rtc::TimeMillis();
			int nNalsSize = x264_encoder_encode(m_pX264, &pNals, &nNals, NULL, &picOut);
			UpdateEncodeTime(rtc::TimeSince(earilertime));
			if (nNalsSize < 0) {
				LOGE << "Error encoding frame.";
				break;
			}
			if (nNalsSize > 0) {
				m_pCallBack->OnVideoEncodedData(pNals[0].p_payload, nNalsSize, picOut.i_pts, picOut.i_dts);
			}
		}
	}
	x264_encoder_close(m_pX264);
	LOGI<<"Out videoEncodeThread.";
	return false;
}

int StringToX264Rcmethod(const std::string& rcmethod)
{
	if (rcmethod == "RC_ABR")
	{
		return X264_RC_ABR;
	}
	else if (rcmethod == "RC_CQP")
	{
		return X264_RC_CQP;
	}
	else if (rcmethod == "RC_CRF")
	{
		return X264_RC_CRF;
	}
	return X264_RC_ABR;
}
void CVideoEncoderX264::ConfigSetting(x264_param_t* x264param)
{
	std::string value;
	if (m_H264Configs.GetConfigFromKey(kPreset, value))
	{
		std::string strtune;
		if (m_H264Configs.GetConfigFromKey(kTune, strtune))
		{
			x264_param_default_preset(x264param, value.c_str(), strtune.c_str());
			//x264_param_default_preset(x264param, "superfast", "zerolatency");
		}
		else
		{
			x264_param_default_preset(x264param, value.c_str(), NULL);
		}
		LOGD << "x264_param_default_preset  preset = " << value << "  tune = " << strtune;

	}
	if (m_H264Configs.GetConfigFromKey(kProfile, value))
	{
		x264_param_apply_profile(x264param, value.c_str());
		//x264_param_apply_profile(x264param, "main");
		LOGD << "x264_param_apply_profile  = " << value;
	}
	
	if (m_H264Configs.GetConfigFromKey(kRcMethod, value))
	{
		x264param->rc.i_rc_method = StringToX264Rcmethod(value);
		LOGD << "i_rc_method  = " << value;
	}
}

CVideoEncoderX264::CVideoEncoderX264() :
m_hEncodeThread(this, &CVideoEncoderX264::OnEncodeThread, "x264encodethread")
{
	m_qVideoFrameQ.SetMaxCount(kMaxBufferSize);
}
static void x264log_callback(void* avcl, int level, const char* pFormat, va_list vl)
{
	char buf[256] = { 0 };
	vsnprintf(buf, 255, pFormat, vl);
	LOGD << "x264log" << buf;
}

char *x264_param2string(x264_param_t *p, int b_res)
{
	int len = 1000;
	char *buf, *s;
	if (p->rc.psz_zones)
		len += strlen(p->rc.psz_zones);
	buf = s = (char*)malloc(len);
	if (!buf)
		return NULL;

	if (b_res)
	{
		s += sprintf(s, "%dx%d ", p->i_width, p->i_height);
		s += sprintf(s, "fps=%u/%u ", p->i_fps_num, p->i_fps_den);
		s += sprintf(s, "timebase=%u/%u ", p->i_timebase_num, p->i_timebase_den);
		//s += sprintf(s, "bitdepth=%d ", BIT_DEPTH);
	}

	if (p->b_opencl)
		s += sprintf(s, "opencl=%d ", p->b_opencl);
	s += sprintf(s, "cabac=%d", p->b_cabac);
	s += sprintf(s, " ref=%d", p->i_frame_reference);
	s += sprintf(s, " deblock=%d:%d:%d", p->b_deblocking_filter,
		p->i_deblocking_filter_alphac0, p->i_deblocking_filter_beta);
	s += sprintf(s, " analyse=%#x:%#x", p->analyse.intra, p->analyse.inter);
	s += sprintf(s, " me=%s", x264_motion_est_names[p->analyse.i_me_method]);
	s += sprintf(s, " subme=%d", p->analyse.i_subpel_refine);
	s += sprintf(s, " psy=%d", p->analyse.b_psy);
	if (p->analyse.b_psy)
		s += sprintf(s, " psy_rd=%.2f:%.2f", p->analyse.f_psy_rd, p->analyse.f_psy_trellis);
	s += sprintf(s, " mixed_ref=%d", p->analyse.b_mixed_references);
	s += sprintf(s, " me_range=%d", p->analyse.i_me_range);
	s += sprintf(s, " chroma_me=%d", p->analyse.b_chroma_me);
	s += sprintf(s, " trellis=%d", p->analyse.i_trellis);
	s += sprintf(s, " 8x8dct=%d", p->analyse.b_transform_8x8);
	s += sprintf(s, " cqm=%d", p->i_cqm_preset);
	s += sprintf(s, " deadzone=%d,%d", p->analyse.i_luma_deadzone[0], p->analyse.i_luma_deadzone[1]);
	s += sprintf(s, " fast_pskip=%d", p->analyse.b_fast_pskip);
	s += sprintf(s, " chroma_qp_offset=%d", p->analyse.i_chroma_qp_offset);
	s += sprintf(s, " threads=%d", p->i_threads);
	s += sprintf(s, " lookahead_threads=%d", p->i_lookahead_threads);
	s += sprintf(s, " sliced_threads=%d", p->b_sliced_threads);
	if (p->i_slice_count)
		s += sprintf(s, " slices=%d", p->i_slice_count);
	if (p->i_slice_count_max)
		s += sprintf(s, " slices_max=%d", p->i_slice_count_max);
	if (p->i_slice_max_size)
		s += sprintf(s, " slice_max_size=%d", p->i_slice_max_size);
	if (p->i_slice_max_mbs)
		s += sprintf(s, " slice_max_mbs=%d", p->i_slice_max_mbs);
	if (p->i_slice_min_mbs)
		s += sprintf(s, " slice_min_mbs=%d", p->i_slice_min_mbs);
	s += sprintf(s, " nr=%d", p->analyse.i_noise_reduction);
	s += sprintf(s, " decimate=%d", p->analyse.b_dct_decimate);
	s += sprintf(s, " interlaced=%s", p->b_interlaced ? p->b_tff ? "tff" : "bff" : p->b_fake_interlaced ? "fake" : "0");
	s += sprintf(s, " bluray_compat=%d", p->b_bluray_compat);
	if (p->b_stitchable)
		s += sprintf(s, " stitchable=%d", p->b_stitchable);

	s += sprintf(s, " constrained_intra=%d", p->b_constrained_intra);

	s += sprintf(s, " bframes=%d", p->i_bframe);
	if (p->i_bframe)
	{
		s += sprintf(s, " b_pyramid=%d b_adapt=%d b_bias=%d direct=%d weightb=%d open_gop=%d",
			p->i_bframe_pyramid, p->i_bframe_adaptive, p->i_bframe_bias,
			p->analyse.i_direct_mv_pred, p->analyse.b_weighted_bipred, p->b_open_gop);
	}
	s += sprintf(s, " weightp=%d", p->analyse.i_weighted_pred > 0 ? p->analyse.i_weighted_pred : 0);

	if (p->i_keyint_max == X264_KEYINT_MAX_INFINITE)
		s += sprintf(s, " keyint=infinite");
	else
		s += sprintf(s, " keyint=%d", p->i_keyint_max);
	s += sprintf(s, " keyint_min=%d scenecut=%d intra_refresh=%d",
		p->i_keyint_min, p->i_scenecut_threshold, p->b_intra_refresh);

	if (p->rc.b_mb_tree || p->rc.i_vbv_buffer_size)
		s += sprintf(s, " rc_lookahead=%d", p->rc.i_lookahead);

	s += sprintf(s, " rc=%s mbtree=%d", p->rc.i_rc_method == X264_RC_ABR ?
		(p->rc.b_stat_read ? "2pass" : p->rc.i_vbv_max_bitrate == p->rc.i_bitrate ? "cbr" : "abr")
		: p->rc.i_rc_method == X264_RC_CRF ? "crf" : "cqp", p->rc.b_mb_tree);
	if (p->rc.i_rc_method == X264_RC_ABR || p->rc.i_rc_method == X264_RC_CRF)
	{
		if (p->rc.i_rc_method == X264_RC_CRF)
			s += sprintf(s, " crf=%.1f", p->rc.f_rf_constant);
		else
			s += sprintf(s, " bitrate=%d ratetol=%.1f",
			p->rc.i_bitrate, p->rc.f_rate_tolerance);
		s += sprintf(s, " qcomp=%.2f qpmin=%d qpmax=%d qpstep=%d",
			p->rc.f_qcompress, p->rc.i_qp_min, p->rc.i_qp_max, p->rc.i_qp_step);
		if (p->rc.b_stat_read)
			s += sprintf(s, " cplxblur=%.1f qblur=%.1f",
			p->rc.f_complexity_blur, p->rc.f_qblur);
		if (p->rc.i_vbv_buffer_size)
		{
			s += sprintf(s, " vbv_maxrate=%d vbv_bufsize=%d",
				p->rc.i_vbv_max_bitrate, p->rc.i_vbv_buffer_size);
			if (p->rc.i_rc_method == X264_RC_CRF)
				s += sprintf(s, " crf_max=%.1f", p->rc.f_rf_constant_max);
		}
	}
	else if (p->rc.i_rc_method == X264_RC_CQP)
		s += sprintf(s, " qp=%d", p->rc.i_qp_constant);

	if (p->rc.i_vbv_buffer_size)
		s += sprintf(s, " nal_hrd=%s filler=%d", x264_nal_hrd_names[p->i_nal_hrd], p->rc.b_filler);
	if (p->crop_rect.i_left | p->crop_rect.i_top | p->crop_rect.i_right | p->crop_rect.i_bottom)
		s += sprintf(s, " crop_rect=%u,%u,%u,%u", p->crop_rect.i_left, p->crop_rect.i_top,
		p->crop_rect.i_right, p->crop_rect.i_bottom);
	if (p->i_frame_packing >= 0)
		s += sprintf(s, " frame-packing=%d", p->i_frame_packing);

	if (!(p->rc.i_rc_method == X264_RC_CQP && p->rc.i_qp_constant == 0))
	{
		s += sprintf(s, " ip_ratio=%.2f", p->rc.f_ip_factor);
		if (p->i_bframe && !p->rc.b_mb_tree)
			s += sprintf(s, " pb_ratio=%.2f", p->rc.f_pb_factor);
		s += sprintf(s, " aq=%d", p->rc.i_aq_mode);
		if (p->rc.i_aq_mode)
			s += sprintf(s, ":%.2f", p->rc.f_aq_strength);
		if (p->rc.psz_zones)
			s += sprintf(s, " zones=%s", p->rc.psz_zones);
		else if (p->rc.i_zones)
			s += sprintf(s, " zones");
	}
	s += sprintf(s, " i_level = %d", p->i_level_idc);
	return buf;
}
int CVideoEncoderX264::OpenEncoder()
{
	x264_param_t x264param;

	//Init x264
	ConfigSetting(&x264param);

	
	//x264_param_default_preset(&x264param, "superfast", "zerolatency");
	//LOGD << "x264_param_default_preset  " << x264_param2string(&x264param, false);

	/*x264_param_apply_profile(&x264param, "baseline");
	LOGD << "x264_param_apply_profile  " << x264_param2string(&x264param, false);*/

	x264param.rc.i_rc_method = X264_RC_ABR;

	x264param.rc.i_bitrate = m_nBitrate / 1000;
	x264param.i_timebase_num = 1;
	x264param.i_timebase_den = m_nFps;
	x264param.i_fps_num = x264param.i_timebase_den;
	x264param.i_fps_den = x264param.i_timebase_num * 1;
	x264param.i_keyint_min = m_nGop / 2;
	x264param.i_keyint_max = m_nGop;

	x264param.b_repeat_headers = 0;
	//x264param.b_annexb = pCtx->bVideoAnnexb ? 1 : 0;
	x264param.b_annexb = 1;
	x264param.i_width = m_nWidth;
	x264param.i_height = m_nHeight;

	x264param.i_csp = X264_CSP_I420;
	x264param.i_log_level = X264_LOG_NONE;

	x264param.pf_log = &x264log_callback;
	x264param.i_log_level = X264_LOG_INFO;

	LOGD << "prex264_encoder_open  "<<x264_param2string(&x264param, true);
	m_pX264 = x264_encoder_open(&x264param);
	if (m_pX264 == NULL)
	{
		LOGD << "x264 encoder opened failed";
		return -1;
	}
	m_nLevel = x264param.i_level_idc;
	//x264_encoder_parameters(m_pX264, &x264param);

	LOGD << x264_param2string(&x264param, true);
	return 0;
}

int CVideoEncoderX264::CloseEncoder()
{
	m_bAbort = true;
	m_qVideoFrameQ.SetEnable(false);
	m_hEncodeThread.Stop();
	return 0;
}

int CVideoEncoderX264::PushData(uint8_t *imagedata, int nsize, int64_t pts)
{
	//LOGI << " video data size = " << nsize << " pts = " << pts;
	std::unique_ptr<VideoFrame> frame(new VideoFrame(m_nWidth, m_nHeight));
	frame->FillData(imagedata, nsize, pts);
	if (!m_qVideoFrameQ.PushData(frame, false))
	{
		return 1;
	}
	return 0;
}

sH264CodecInfo CVideoEncoderX264::GetCodecInfo()
{
	std::string value;
	sH264CodecInfo info;
	info.m_bAnnexB = true;
	info.m_nBitrate = m_nBitrate;
	info.m_nFps = m_nFps;
	info.m_nGop = m_nGop;
	info.m_nHeight = m_nHeight;
	info.m_nWidth = m_nWidth;
	info.m_nLevel = m_nLevel;
	m_H264Configs.GetConfigFromKey(kProfile, value);
	info.m_nProfile = StringToProfile(value);
	LOGD << "GetCodecInfo profile = "<<info.m_nProfile<< "  level = "<< info.m_nLevel;
	return info;
}

int CVideoEncoderX264::StartEncode()
{
	m_bAbort = false;
	m_nAvgEncodedTimeMs = 0;
	m_qVideoFrameQ.SetEnable(true);
	m_hEncodeThread.Start();
	return 0;
}

CVideoEncoderBase::EncoderRunTimeInfo CVideoEncoderX264::GetRunTimeInfo()
{
	CVideoEncoderBase::EncoderRunTimeInfo info;
	info.m_nBufferRemainNum = m_qVideoFrameQ.Size();
	info.m_nEncodeAvgTimeMs = m_nAvgEncodedTimeMs;
	return info;
}
