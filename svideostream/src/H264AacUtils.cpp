#include "H264AacUtils.h"
#include "Log.h"



int CH264AacUtils::GetFrameSize(ImageFormat format, int stride, int height)
{
	switch (format) {
	case IMAGE_FORMAT_NV12:
	case IMAGE_FORMAT_NV21:
		return (stride * height * 3) / 2;

	case IMAGE_FORMAT_ARGB:
	case IMAGE_FORMAT_RGBA:
	case IMAGE_FORMAT_ABGR:
	case IMAGE_FORMAT_BGRA:
		return stride * height * 4;
	default:
		break;
	}
	return (stride * height * 3) / 2;
}

bool CH264AacUtils::IsKeyFrame(uint8_t* vdata, int nsize, bool annexb /*= true*/)
{
	if (nsize < 5)
	{
		return false;
	}
	if (annexb)
	{
		if (vdata[0] == 0x00 && vdata[1] == 0x00 && vdata[2] == 0x00 && vdata[3] == 0x01) {
			vdata += 4;
		}
		else if (vdata[0] == 0x00 && vdata[1] == 0x00 && vdata[2] == 0x01) {
			vdata += 3;
		}
		else {
			LOGD << "no startcode";
			return false;
		}
	}
	else
	{
		vdata += 4;
	}

	if ((NALTYPE_SLICE_IDR == (vdata[0] & 0x1f)) || (NALTYPE_SLICE_SEI == (vdata[0] & 0x1f)) ||
		(NALTYPE_SLICE_SPS == (vdata[0] & 0x1f)) || (NALTYPE_SLICE_PPS == (vdata[0] & 0x1f))) {
		return true;
	}
	return false;
}
int ReverseBeLe(int a)
{
	union {
		int i;
		char c[4];
	}u, r;

	u.i = a;
	r.c[0] = u.c[3];
	r.c[1] = u.c[2];
	r.c[2] = u.c[1];
	r.c[3] = u.c[0];

	return r.i;
}
void CH264AacUtils::ConvertAVCCToAnnexB(uint8_t *data, int size)
{
	if (size < 5) {
		return;
	}

	int bufferOffset = 0;
	static const int AVCCHeaderLength = 4;
	while (bufferOffset < size - AVCCHeaderLength) {

		// Read the NAL unit length
		uint32_t NALUnitLength = 0;
		memcpy(&NALUnitLength, data + bufferOffset, AVCCHeaderLength);

		// Convert the length value from Big-endian to Little-endian
		NALUnitLength = ReverseBeLe(NALUnitLength);
		*(data + bufferOffset) = 0x0;
		*(data + bufferOffset + 1) = 0x0;
		*(data + bufferOffset + 2) = 0x0;
		*(data + bufferOffset + 3) = 0x1;

		// Move to the next NAL unit in the block buffer
		bufferOffset += AVCCHeaderLength + NALUnitLength;
	}
}

int CH264AacUtils::ExtractSPSData(uint8_t *data, int size, uint8_t* outbuf, int outbufsize, bool annexb /*= true*/)
{
	if (data == nullptr || outbuf == nullptr || outbufsize < size)
	{
		return 0;
	}
	if (!annexb)
	{
		ConvertAVCCToAnnexB(data, size);
	}
	int nSPSStartPos = 0;
	int nSPSEndPos = 0;
	int nExtraLen = size;
	int nNalLen = 0;
	uint8_t *p = data;
	int i = 0;
	for (i = 0; i != nExtraLen - 5; ++i) {
		if (0x0 == p[i] && 0x0 == p[i + 1]) {
			if (0x0 == p[i + 2] && 0x1 == p[i + 3] && NALTYPE_SLICE_SPS == (p[i + 4] & 0x1f)) {
				nNalLen = 4;
				nSPSStartPos = i + nNalLen;
				break;
			}
			else if (0x1 == p[i + 2] && NALTYPE_SLICE_SPS == (p[i + 3] & 0x1f)) {
				nNalLen = 3;
				nSPSStartPos = i + nNalLen;
				break;
			}
		}
	}
	if (nSPSStartPos == 0)
	{
		return 0;
	}
	for (i = nSPSStartPos; i != nExtraLen; ++i) {
		if (0x0 == p[i] && 0x0 == p[i + 1]) {
			if (0x0 == p[i + 2] && 0x1 == p[i + 3]) {
				nSPSEndPos = i;
				break;
			}
			else if ( 0x1 == p[i + 2]) {
				nSPSEndPos = i;
				break;
			}
		}
	}
	int spssize = 0;
	if (nSPSEndPos == 0)
	{
		spssize = size - nSPSStartPos;
	}
	else
	{
		spssize = nSPSEndPos - nSPSStartPos;
	}
	memcpy(outbuf, data + nSPSStartPos, spssize);
	return spssize;
}

int CH264AacUtils::ExtractPPSData(uint8_t *data, int size, uint8_t* outbuf, int outbufsize, bool annexb /*= true*/)
{
	if (data == nullptr || outbuf == nullptr || outbufsize < size)
	{
		return 0;
	}
	if (!annexb)
	{
		ConvertAVCCToAnnexB(data, size);
	}
	int nPPSStartPos = 0;
	int nPPSEndPos = 0;
	int nExtraLen = size;
	int nNalLen = 0;
	uint8_t *p = data;
	int i = 0;
	for (i = 0; i != nExtraLen - 5; ++i) {
		if (0x0 == p[i] && 0x0 == p[i + 1]) {
			if (0x0 == p[i + 2] && 0x1 == p[i + 3] && NALTYPE_SLICE_PPS == (p[i + 4] & 0x1f)) {
				nNalLen = 4;
				nPPSStartPos = i + nNalLen;
				break;
			}
			else if (0x1 == p[i + 2] && NALTYPE_SLICE_PPS == (p[i + 3] & 0x1f)) {
				nNalLen = 3;
				nPPSStartPos = i + nNalLen;
				break;
			}
		}
	}
	if (nPPSStartPos == 0)
	{
		return 0;
	}
	for (i = nPPSStartPos; i != nExtraLen; ++i) {
		if (0x0 == p[i] && 0x0 == p[i + 1]) {
			if (0x0 == p[i + 2] && 0x1 == p[i + 3]) {
				nPPSEndPos = i;
				break;
			}
			else if (0x1 == p[i + 2]) {
				nPPSEndPos = i;
				break;
			}
		}
	}
	int spssize = 0;
	if (nPPSEndPos == 0)
	{
		spssize = size - nPPSStartPos;
	}
	else
	{
		spssize = nPPSEndPos - nPPSStartPos;
	}
	memcpy(outbuf, data + nPPSStartPos, spssize);
	return spssize;
}
