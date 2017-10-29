#include "RtmpLive.h"
#include "H264AacUtils.h"
int CRtmpLive::OnAVSpecdata(uint8_t* data, int nsize, RtmpPacket::PacketType type)
{
	if (type == RtmpPacket::RP_AUDIO)
	{
		if (nsize != RTMP_AUDIO_SPECDATA_SIZE)
		{
			LOGE << "ExtractAudioSpecData failed";
			return -1;
		}
		m_nAudioSpecdataLen = nsize;
		memcpy(m_AudioSpecdata, data, nsize);
	}
	else if (type == RtmpPacket::RP_VIDEO)
	{
		m_nVideoSPSDataLen = CH264AacUtils::ExtractSPSData(data, nsize, m_VideoSPSData, RTMP_VIDEO_SPECDATA_SIZE, m_H264Info.m_bAnnexB);
		m_nVideoPPSDataLen = CH264AacUtils::ExtractPPSData(data, nsize, m_VideoPPSData, RTMP_VIDEO_SPECDATA_SIZE, m_H264Info.m_bAnnexB);
		if (0 == m_nVideoSPSDataLen || 0 == m_nVideoPPSDataLen)
		{
			LOGE << "ExtractVideoSpecData failed";
			return -1;
		}
	}
	return 0;
}

int CRtmpLive::OnAudioQueneFull()
{
	m_qAudio.Clear();
	return 0;
}

int CRtmpLive::OnVideoQueneFull()
{
	if (!m_qVideo.ClearUtil(&RtmpPacket::IsKeyVideoFrame))
	{
		m_bNeedWaitKeyFrame = true;
		NotifyEvent(LE_NEED_KEYFRAME, 0);
	}
	else
	{
		if (m_qVideo.isFull())
		{
			m_qVideo.Clear();
			m_bNeedWaitKeyFrame = true;
			NotifyEvent(LE_NEED_KEYFRAME, 0);
		}
	}
	return 0;
}

int CRtmpLive::WaitForAVSpecData()
{
	uint32_t nWaitTimeOutMS = 10000;
	uint32_t nStartTimeMS = RTMP_GetTime();
	LOGI << "wait for sps and pps data";
	while ( (m_nVideoPPSDataLen == 0 || m_nVideoSPSDataLen == 0) && m_bStarted)
	{
		if (!m_bStarted || RTMP_GetTime() - nStartTimeMS > nWaitTimeOutMS)
		{
			LOGE << " wait video speciial data timeout ";
			return 1;
		}
		usleep(20000);
	}
	if (!m_bAudioEnable)
	{
		return 0;
	}
	nStartTimeMS = RTMP_GetTime();
	LOGI << "wait for audio spec data";
	while (m_nAudioSpecdataLen == 0 && m_bStarted)
	{
		if (!m_bStarted || RTMP_GetTime() - nStartTimeMS > nWaitTimeOutMS)
		{
			LOGE << " wait audio special data timeout ";
			return 1;
		}
		usleep(20000);
	}

	return 0;
}

int CRtmpLive::SendRTMPConfigInfo()
{
	if (0 != SendChunkSizeInfo()) {
		LOGE<<"Send chunk size info failed.";
		return -1;
	}
	if (0 != SendMetaData()) {
		LOGE<<"Send metadata failed.";
		return -1;
	}
	if (0 != SendVideoSpecData()) {
		LOGE<<"Send video extradata failed.";
		return -1;
	}
	if (m_bAudioEnable) {
		if (0 != SendAudioSpecData()) {
			LOGE<<"Send audio specdata failed.";
			return -1;
		}
	}
	return 0;
}

int CRtmpLive::SendChunkSizeInfo()
{
	RTMPPacket packet;
	RTMPPacket_Alloc(&packet, 4);
	RTMPPacket_Reset(&packet);
	int nChunkSize = RTMP_CHUNK_SIZE;
	packet.m_body[3] = nChunkSize & 0xff;
	packet.m_body[2] = nChunkSize >> 8;
	packet.m_body[1] = nChunkSize >> 16;
	packet.m_body[0] = nChunkSize >> 24;
	m_pRtmp->m_outChunkSize = nChunkSize;

	packet.m_packetType = RTMP_PACKET_TYPE_CHUNK_SIZE;
	packet.m_nBodySize = 4;
	packet.m_nChannel = RTMP_STREAMCHANNEL_CHUNKSIZE;
	packet.m_nTimeStamp = 0;
	packet.m_hasAbsTimestamp = 0;
	packet.m_headerType = RTMP_PACKET_SIZE_LARGE;
	packet.m_nInfoField2 = 0;

	int nRet = SendRtmpData(&packet);
	if (0 != nRet) {
		LOGE<<"Send chunk info packet failed.";
	}
	RTMPPacket_Free(&packet);
	return nRet;
}

int CRtmpLive::SendMetaData()
{
	char metadata[1024] = { 0 };
	char *p = metadata;

	/*p = CRtmpCommon::putByte(p, AMF_STRING);
	p = CRtmpCommon::putAmfString(p, "@setDataFrame");*/

	p = CRtmpCommon::putByte(p, AMF_STRING);
	p = CRtmpCommon::putAmfString(p, "onMetaData");

	//AMF_OBJECT
	p = CRtmpCommon::putByte(p, AMF_ECMA_ARRAY);
	if (m_bAudioEnable) {
		p = CRtmpCommon::putBe32(p, 17);
	}
	else {
		p = CRtmpCommon::putBe32(p, 12);
	}

	p = CRtmpCommon::putAmfString(p, "duration");
	p = CRtmpCommon::putAmfDouble(p, 0);

	p = CRtmpCommon::putAmfString(p, "width");
	p = CRtmpCommon::putAmfDouble(p, m_H264Info.m_nWidth);

	p = CRtmpCommon::putAmfString(p, "height");
	p = CRtmpCommon::putAmfDouble(p, m_H264Info.m_nHeight);

	p = CRtmpCommon::putAmfString(p, "videodatarate");
	p = CRtmpCommon::putAmfDouble(p, m_H264Info.m_nBitrate / 1000);

	p = CRtmpCommon::putAmfString(p, "framerate");
	p = CRtmpCommon::putAmfDouble(p, m_H264Info.m_nFps);

	p = CRtmpCommon::putAmfString(p, "videocodecid");
	// 7 Ϊh264
	p = CRtmpCommon::putAmfDouble(p, 7);

	if (m_bAudioEnable) {

		p = CRtmpCommon::putAmfString(p, "audiodatarate");
		p = CRtmpCommon::putAmfDouble(p, m_AacInfo.m_nBitrate / 1024.0);

		p = CRtmpCommon::putAmfString(p, "audiosamplerate");
		p = CRtmpCommon::putAmfDouble(p, m_AacInfo.m_nSampleRate);

		p = CRtmpCommon::putAmfString(p, "audiosamplesize");
		p = CRtmpCommon::putAmfDouble(p, m_AacInfo.m_nSampleSize * 8);

		p = CRtmpCommon::putAmfString(p, "stero");
		p = CRtmpCommon::putAmfBool(p, m_AacInfo.m_nChannels == 2);

		p = CRtmpCommon::putAmfString(p, "audiocodecid");
		//10 Ϊaac
		p = CRtmpCommon::putAmfDouble(p, 10);

	}


	p = CRtmpCommon::putAmfString(p, "copyright");
	p = CRtmpCommon::putByte(p, AMF_STRING);
	p = CRtmpCommon::putAmfString(p, m_strCopyRight.c_str());

	p = CRtmpCommon::putAmfString(p, "filesize");
	p = CRtmpCommon::putAmfDouble(p, 0);

	p = CRtmpCommon::putAmfString(p, "");
	p = CRtmpCommon::putByte(p, AMF_OBJECT_END);
	int len = p - metadata;

	RTMPPacket packet;
	RTMPPacket_Alloc(&packet, len);
	RTMPPacket_Reset(&packet);
	memcpy(packet.m_body, metadata, len * sizeof(char));
	packet.m_packetType = RTMP_PACKET_TYPE_INFO;
	packet.m_nBodySize = len;
	packet.m_nChannel = RTMP_STREAMCHANNEL_METADATA;
	packet.m_nTimeStamp = 0;
	packet.m_hasAbsTimestamp = 0;
	packet.m_headerType = RTMP_PACKET_SIZE_LARGE;
	packet.m_nInfoField2 = m_pRtmp->m_stream_id;

	int nRet = SendRtmpData(&packet);
	if (0 != nRet) {
		LOGE<<"Send metadata failed.";
	}
	RTMPPacket_Free(&packet);
	return nRet;
}

int CRtmpLive::SendVideoSpecData()
{
	RTMPPacket packet;
	RTMPPacket_Alloc(&packet, 1024);
	RTMPPacket_Reset(&packet);

	int i = 0;
	packet.m_body[i++] = 0x17;
	packet.m_body[i++] = 0x00;
	packet.m_body[i++] = 0x00;
	packet.m_body[i++] = 0x00;
	packet.m_body[i++] = 0x00;
	// AVCDecoderConfigurationRecord
	packet.m_body[i++] = 0x01;
	packet.m_body[i++] = m_VideoSPSData[1];
	packet.m_body[i++] = m_VideoSPSData[2];
	packet.m_body[i++] = m_VideoSPSData[3];
	packet.m_body[i++] = 0xff;
	// sps
	packet.m_body[i++] = 0xe1;
	packet.m_body[i++] = (m_nVideoSPSDataLen >> 8) & 0xff;
	packet.m_body[i++] = m_nVideoSPSDataLen & 0xff;
	memcpy(&packet.m_body[i], m_VideoSPSData, m_nVideoSPSDataLen * sizeof(uint8_t));
	i += m_nVideoSPSDataLen;
	// pps
	packet.m_body[i++] = 0x01;
	packet.m_body[i++] = (m_nVideoPPSDataLen >> 8) & 0xff;
	packet.m_body[i++] = (m_nVideoPPSDataLen) & 0xff;
	memcpy(&packet.m_body[i], m_VideoPPSData, m_nVideoPPSDataLen * sizeof(uint8_t));
	i += m_nVideoPPSDataLen;

	packet.m_packetType = RTMP_PACKET_TYPE_VIDEO;
	packet.m_nBodySize = i;
	packet.m_nChannel = RTMP_STREAMCHANNEL_VIDEO;
	packet.m_nTimeStamp = 0;
	packet.m_hasAbsTimestamp = 0;
	packet.m_headerType = RTMP_PACKET_SIZE_LARGE;
	packet.m_nInfoField2 = m_pRtmp->m_stream_id;

	int nRet = SendRtmpData(&packet);
	if (0 != nRet) {
		LOGE<<"Send video extradata failed.";
	}
	RTMPPacket_Free(&packet);
	return nRet;
}

int CRtmpLive::SendAudioSpecData()
{
	RTMPPacket packet;
	RTMPPacket_Alloc(&packet, 4);
	RTMPPacket_Reset(&packet);

	int i = 0;
	// AF 00 + AAC RAW data
	packet.m_body[i++] = 0xAF;
	// body[0] = 0xAE;
	packet.m_body[i++] = 0x00;
	packet.m_body[i++] = m_AudioSpecdata[0];
	packet.m_body[i++] = m_AudioSpecdata[1];
	LOGI<<"audio Specdata "<< std::hex<<m_AudioSpecdata[0]<<"  "<<std::hex<<m_AudioSpecdata[1];

	packet.m_packetType = RTMP_PACKET_TYPE_AUDIO;
	packet.m_nBodySize = i;
	packet.m_nChannel = RTMP_STREAMCHANNEL_AUDIO;
	packet.m_nTimeStamp = 0;
	packet.m_hasAbsTimestamp = 0;
	packet.m_headerType = RTMP_PACKET_SIZE_LARGE;
	packet.m_nInfoField2 = m_pRtmp->m_stream_id;

	int nRet = SendRtmpData(&packet);
	if (0 != nRet) {
		LOGE<<"Send audio specdata failed.";
	}
	RTMPPacket_Free(&packet);
	return nRet;
}

int CRtmpLive::SendAVPacket(RtmpPacket* avpacket)
{
	if (avpacket == nullptr)
	{
		LOGE << "avpacket is null";
		return -1;
	}
	if (!m_bAudioEnable && avpacket->m_eType == RtmpPacket::RP_AUDIO)
	{
		LOGE << "audio disable";
		return -1;
	}
	RTMPPacket packet;
	if (!RTMPPacket_Alloc(&packet, avpacket->m_nSize + 10))
	{
		LOGE << "RTMPPacket_Alloc failed";
	}
	RTMPPacket_Reset(&packet);
	if (avpacket->m_eType == RtmpPacket::RP_AUDIO)
	{
		int i = 0;
		/*AF 01 + AAC RAW data*/
		packet.m_body[i++] = 0xAF;
		packet.m_body[i++] = 0x01;
		memcpy(&packet.m_body[i], avpacket->m_pData, avpacket->m_nSize* sizeof(uint8_t));
		i += avpacket->m_nSize;

		packet.m_packetType = RTMP_PACKET_TYPE_AUDIO;
		packet.m_nBodySize = i;
		packet.m_nChannel = RTMP_STREAMCHANNEL_AUDIO;
		packet.m_nTimeStamp = (uint32_t)avpacket->m_pts;
		packet.m_hasAbsTimestamp = 0;
		packet.m_headerType = RTMP_PACKET_SIZE_LARGE;
		packet.m_nInfoField2 = m_pRtmp->m_stream_id;
	}
	else if (avpacket->m_eType == RtmpPacket::RP_VIDEO)
	{
		int i = 0;
		uint8_t* pdata = avpacket->m_pData;
		int nlen = avpacket->m_nSize;
		// key frame
		if (avpacket->m_bKeyFrame) {
			packet.m_body[i++] = 0x17;
		}
		else {
			packet.m_body[i++] = 0x27;
		}

		packet.m_body[i++] = 0x01;
		packet.m_body[i++] = 0x00;
		packet.m_body[i++] = 0x00;
		packet.m_body[i++] = 0x00;

		if (m_H264Info.m_bAnnexB) {
			if (pdata[0] == 0x00 && pdata[1] == 0x00 && pdata[2] == 0x00 && pdata[3] == 0x01) {
				pdata += 4;
				nlen -= 4;
			}
			else if (pdata[0] == 0x00 && pdata[1] == 0x00 && pdata[2] == 0x01) {
				pdata += 3;
				nlen -= 3;
			}
			packet.m_body[i++] = (nlen >> 24) & 0xff;
			packet.m_body[i++] = (nlen >> 16) & 0xff;
			packet.m_body[i++] = (nlen >> 8) & 0xff;
			packet.m_body[i++] = (nlen)& 0xff;
		}
		memcpy(&packet.m_body[i], pdata, nlen * sizeof(uint8_t));
		i += nlen;
		packet.m_packetType = RTMP_PACKET_TYPE_VIDEO;
		packet.m_nBodySize = i;
		packet.m_nChannel = RTMP_STREAMCHANNEL_VIDEO;
		packet.m_nTimeStamp = (uint32_t)avpacket->m_pts;
		packet.m_hasAbsTimestamp = 0;
		packet.m_headerType = RTMP_PACKET_SIZE_LARGE;
		packet.m_nInfoField2 = m_pRtmp->m_stream_id;
	}
	int nRet = 0;
	 nRet = SendRtmpData(&packet);
	if (0 != nRet) {
		LOGE<<"Send rtmp data failed.";
	}

	RTMPPacket_Free(&packet);
	return nRet;
}

int CRtmpLive::SendRtmpData(RTMPPacket *pkt)
{
	if (!RTMP_IsConnected(m_pRtmp)) {
		LOGE<<"RTMP is not connect before send packet.";
		return -1;
	}

	uint32_t nStartTime = RTMP_GetTime(), nEndTime = 0;
	if (RTMP_SendPacket(m_pRtmp, pkt, TRUE) <= 0) {
		LOGE<<"RTMP packet send error return.";
		if (!RTMP_IsConnected(m_pRtmp)) {
			LOGE<<"RTMP is not connect after trying to send packet.";
			return -1;
		}
		LOGE<<"Send packet failed.";
	}
	nEndTime = RTMP_GetTime();
	// Send packet success, but timeout
	if ((nEndTime - nStartTime) >= 500) {
		LOGI<<"Send packet timeout, > 500ms.";
	}
	return 0;
}

bool CRtmpLive::OnSendThread()
{
	int sleeptime = 1000;
	m_pRtmp = RTMP_Alloc();
	if (m_pRtmp == NULL)
	{
		LOGE << "rtmp alloc failed;";
		return false;
	}
	RTMP_Init(m_pRtmp);
	m_pRtmp->Link.timeout = RTMP_CONNECT_TIMEOUT;
	if (0 != ConnectServer())
	{
		goto end;
		LOGE << "connected failed";
	}
	SetState(LS_CONNECTED);
	NotifyEvent(LE_CONNECTED_SUCCESS, 0);

	if (WaitForAVSpecData() != 0)
	{
		goto end;
	}
	if (0 != SendRTMPConfigInfo())
	{
		LOGE << "SendRTMPConfigInfo error";
		goto end;
	}
	while (m_bStarted)
	{
		sleeptime = 1000;
		//get and send video packet.
		RtmpPacket *avpacket = nullptr;
		if (m_qVideo.PullData(&avpacket))
		{
			LOGI << "send video rtmp packet datasize = " << avpacket->m_nSize << " is keyframe = " << avpacket->m_bKeyFrame;
			if (m_bNeedWaitKeyFrame && avpacket->m_bKeyFrame)
			{
				m_bNeedWaitKeyFrame = false;
			}
			if (m_bNeedWaitKeyFrame)
			{
				if (avpacket->m_bKeyFrame)
				{
					m_bNeedWaitKeyFrame = false;
					SendAVPacket(avpacket);
				}
			}
			else
			{
				SendAVPacket(avpacket);
			}
			delete avpacket;
			avpacket = nullptr;
		}
		else
		{
			sleeptime = 30000;
		}
		if (m_bAudioEnable)
		{
			if (m_qAudio.PullData(&avpacket))
			{
				SendAVPacket(avpacket);
				delete avpacket;
				avpacket = nullptr;
			}
			else
			{
				sleeptime = 30000;
			}
		}
		usleep(sleeptime);
	}
end:
	if (m_bStarted)
	{
		m_bStarted = false;
		NotifyEvent(LE_LIVE_ERROR, 0);
		
	}
	m_qAudio.SetEnable(false);
	m_qVideo.SetEnable(false);
	m_qVideo.Clear();
	m_qAudio.Clear();
	if (m_pRtmp != nullptr)
	{
		if (RTMP_IsConnected(m_pRtmp))
		{
			RTMP_Close(m_pRtmp);
		}
			RTMP_Free(m_pRtmp);
		m_pRtmp = NULL;
	}
	NotifyEvent(LE_LIVE_STOP, 0);
	SetState(LS_NONE);

	return false;

}

int CRtmpLive::ConnectServer()
{
	if (m_strPublishUrl.empty())
	{
		LOGE << "url is invaild";
		return -1;
	}
	char rtmpurl[256] = {0};
	strcpy(rtmpurl, m_strPublishUrl.c_str());
	int nRet = 0;
	nRet = RTMP_SetupURL(m_pRtmp, rtmpurl);
	if (nRet <= 0)
	{
		LOGE << "RTMP_SetupURL failed ret = " << nRet;
		return -1;
	}

	RTMP_EnableWrite(m_pRtmp);
	nRet = RTMP_Connect(m_pRtmp, NULL);
	if (nRet <= 0)
	{
		LOGE << "RTMP_Connect failed ret = " << nRet;
		return -1;
	}
	nRet = RTMP_ConnectStream(m_pRtmp, 0);
	if (nRet <= 0)
	{
		LOGE << "RTMP_ConnectStream failed ret = "<< nRet;
		return -1;
	}
	LOGI << "Connect Server successfully";
	return 0;
}

CRtmpLive::CRtmpLive() :
m_hSendThread(this, &CRtmpLive::OnSendThread,"livethread")
{

}

int CRtmpLive::StartLive()
{
	if (m_bStarted)
	{
		LOGI << "has started";
		return 0;
	}
	if (m_strPublishUrl.empty())
	{
		LOGE << "not a vaild url";
		return -1;
	}
	if (m_H264Info.m_nBitrate <= 0 ||
		m_H264Info.m_nFps <= 0 ||
		m_H264Info.m_nHeight <= 0 ||
		m_H264Info.m_nWidth <= 0 ||
		m_H264Info.m_nGop <= 0)
	{
		LOGE << "video params is invaild";
		return -1;
	}
	if (m_bAudioEnable)
	{
		if (m_AacInfo.m_nBitrate <= 0 ||
			m_AacInfo.m_nChannels <= 0 ||
			m_AacInfo.m_nSampleRate <= 0 ||
			m_AacInfo.m_nFrameSize <= 0)
		{
			LOGE << "audio params is invaild";
			return -1;
		}
	}
	m_bStarted = true;
	m_nVideoSPSDataLen = m_nVideoPPSDataLen = m_nAudioSpecdataLen = 0;
	int audiomaxcount = m_AacInfo.m_nSampleRate / m_AacInfo.m_nFrameSize * 3;
	int videomaxcount = m_H264Info.m_nGop;
	LOGI << "audiomaxcount = " << audiomaxcount << " videomaxcount = " << videomaxcount;
	m_qAudio.SetMaxCount(audiomaxcount);
	m_qVideo.SetMaxCount(videomaxcount);
	m_qAudio.Clear();
	m_qVideo.Clear();
	m_qAudio.SetEnable(true);
	m_qVideo.SetEnable(true);
	m_hSendThread.Start();
	return 0;
}

void CRtmpLive::StopLive()
{
	if (m_bStarted)
	{
		m_bStarted = false;
		m_qAudio.SetEnable(false);
		m_qVideo.SetEnable(false);
		m_hSendThread.Stop();
		SetState(LS_NONE);
	}
}

int CRtmpLive::SendVideoData(uint8_t* vdata, int nsize, int64_t npts)
{
	if (!m_bStarted)
	{
		LOGE << "live hasn't started";
		return -1;
	}
	if (vdata == nullptr || nsize < 5)
	{
		LOGE << "vdata is error data";
		return -1;
	}
	if (npts == -1)
	{
		return OnAVSpecdata(vdata, nsize, RtmpPacket::RP_VIDEO);
	}
	RtmpPacket *vpacket = new RtmpPacket(RtmpPacket::RP_VIDEO, npts, nsize, CH264AacUtils::IsKeyFrame(vdata, nsize, m_H264Info.m_bAnnexB));
	vpacket->FillData(vdata, nsize);
	if (!m_qVideo.PushData(vpacket))
	{
		LOGE << " video quene full vpacket discared";
		OnVideoQueneFull();
		m_qVideo.PushData(vpacket);
	}
	
	return 0;
}

int CRtmpLive::SendAudioData(uint8_t* adata, int nsize, int64_t npts)
{
	if (!m_bStarted)
	{
		LOGE << "live hasn't started";
		return -1;
	}
	if (adata == nullptr)
	{
		LOGE << "adata is error data";
		return -1;
	}
	if (npts == -1)
	{
		return OnAVSpecdata(adata, nsize, RtmpPacket::RP_AUDIO);
	}
	RtmpPacket *apacket = new RtmpPacket(RtmpPacket::RP_AUDIO, npts, nsize,false);
	apacket->FillData(adata, nsize);
	if (!m_qAudio.PushData(apacket))
	{
		LOGE << " audio quene is full,  will be remove all";
		OnAudioQueneFull();
		m_qAudio.PushData(apacket);
	}

	return 0;
}
