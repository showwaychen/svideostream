#ifndef H264AAC_UTILS_H_
#define H264AAC_UTILS_H_
#include "Common.h"
#define NALTYPE_SLICE_IDR                   (5)
#define NALTYPE_SLICE_SEI                   (6)
#define NALTYPE_SLICE_SPS                   (7)
#define NALTYPE_SLICE_PPS                   (8)
class CH264AacUtils
{
public:
	static int GetFrameSize(ImageFormat format, int stride, int height);
	static bool IsKeyFrame(uint8_t* vdata, int nsize, bool annexb = true);
	static void ConvertAVCCToAnnexB(uint8_t *data, int size);
	static int ExtractSPSData(uint8_t *data, int size, uint8_t* outbuf, int outbufsize, bool annexb = true);
	static int ExtractPPSData(uint8_t *data, int size, uint8_t* outbuf, int outbufsize, bool annexb = true);

};
#endif 