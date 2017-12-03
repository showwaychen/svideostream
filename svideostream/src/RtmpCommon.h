#ifndef RTMP_COMMON_H_
#define RTMP_COMMON_H_
extern "C"
{
#include "rtmp.h"

}
#include"string.h"
#define RTMP_STREAMCHANNEL_CHUNKSIZE    (0x02)
#define RTMP_STREAMCHANNEL_METADATA     (0x03)
#define RTMP_STREAMCHANNEL_VIDEO        (0x04)
#define RTMP_STREAMCHANNEL_AUDIO        (0x05)
#define RTMP_CONNECT_TIMEOUT              (10)
#define RTMP_CHUNK_SIZE                (1300)
#define RTMP_VIDEO_SPECDATA_SIZE      (512)
#define RTMP_AUDIO_SPECDATA_SIZE      (2)

class CRtmpCommon
{
public:
	static char * putByte(char *output, uint8_t nVal)
	{
		output[0] = nVal;
		return output + 1;
	}

	static char * putBe16(char *output, uint16_t nVal)
	{
		output[1] = nVal & 0xff;
		output[0] = nVal >> 8;
		return output + 2;
	}

	static char * putBe24(char *output, uint32_t nVal)
	{
		output[2] = nVal & 0xff;
		output[1] = nVal >> 8;
		output[0] = nVal >> 16;
		return output + 3;
	}

	static char * putBe32(char *output, uint32_t nVal)
	{
		output[3] = nVal & 0xff;
		output[2] = nVal >> 8;
		output[1] = nVal >> 16;
		output[0] = nVal >> 24;
		return output + 4;
	}

	static char * putBe64(char *output, uint64_t nVal)
	{
		output = putBe32(output, nVal >> 32);
		output = putBe32(output, (uint32_t)nVal);
		return output;
	}

	static char * putAmfString(char *c, const char *str)
	{
		uint16_t len = strlen(str);
		c = putBe16(c, len);
		memcpy(c, str, len * sizeof(char));
		return c + len;
	}

	static char * putAmfDouble(char *c, double d)
	{
		*c++ = AMF_NUMBER;  // type: Number
		{
			uint8_t *ci, *co;
			ci = (uint8_t *)&d;
			co = (uint8_t *)c;
			co[0] = ci[7];
			co[1] = ci[6];
			co[2] = ci[5];
			co[3] = ci[4];
			co[4] = ci[3];
			co[5] = ci[2];
			co[6] = ci[1];
			co[7] = ci[0];
		}
		return c + 8;
	}

	static char * putAmfBool(char *c, int d)
	{
		*c++ = AMF_BOOLEAN;  // type: Number
		{
			uint8_t *co;
			// uint8_t *ci,
			// ci = (uint8_t *)&d;
			co = (uint8_t *)c;
			co[0] = !!d;
		}
		return c + 1;
	}

};
#endif 