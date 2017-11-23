#include "VideoEncoderBase.h"



int CVideoEncoderBase::StringToProfile(const std::string& profile)
{
	if (profile == "baseline")
	{
		return 66;
	}
	else if (profile == "main")
	{
		return 77;
	}
	else if (profile == "extended")
	{
		return 88;
	}
	else if (profile == "high")
	{
		return 100;
	}
	else if (profile == "high10")
	{
		return 110;
	}
	else if (profile == "high422")
	{
		return 122;
	}
	else if (profile == "high444")
	{
		return 144;
	}
	return 66;
}

const char CVideoEncoderBase::kProfile[] = "profile";

const char CVideoEncoderBase::kPreset[] = "preset";

const char CVideoEncoderBase::kTune[] = "tune";

const char CVideoEncoderBase::kRcMethod[] = "rc_method";
