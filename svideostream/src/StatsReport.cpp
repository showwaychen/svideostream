#include "StatsReport.h"
#include "base/stringencode.h"

const char* InternalTypeToString(CStatsReport::StatsType type) {
	switch (type) {
	case CStatsReport::kStatsReportVideoEncoder:
		return "VideoEncoder";
	case CStatsReport::kStatsReportImageProcess:
		return "ImageProcess";
	case CStatsReport::kStatsReportRtmpLive:
		return "RtmpLive";
	}
	return nullptr;
}
const char* CStatsReport::TypeToString() const
{
	return InternalTypeToString(m_type);
}

void CStatsReport::AddString(StatsValueName name, const std::string& value)
{
	const Value* found = FindValue(name);
	if (!found || !(*found == value))
		m_values_.push_back(Value(name, value));
}

void CStatsReport::AddString(StatsValueName name, const char* value)
{
	const Value* found = FindValue(name);
	if (!found || !(*found == value))
		m_values_.push_back(Value(name, value));
}

void CStatsReport::AddInt64(StatsValueName name, int64_t value)
{
	const Value* found = FindValue(name);
	if (!found || !(*found == value))
		m_values_.push_back(Value(name, value, Value::kInt64));
}

void CStatsReport::AddInt(StatsValueName name, int value)
{
	const Value* found = FindValue(name);
	if (!found || !(*found == static_cast<int64_t>(value)))
		m_values_.push_back(Value(name, value, Value::kInt));
}

void CStatsReport::AddFloat(StatsValueName name, float value)
{
	const Value* found = FindValue(name);
	if (!found || !(*found == value))
		m_values_.push_back(Value(name, value));
}

void CStatsReport::AddBoolean(StatsValueName name, bool value)
{
	const Value* found = FindValue(name);
	if (!found || !(*found == value))
		m_values_.push_back(Value(name, value));
}

const CStatsReport::Value* CStatsReport::FindValue(StatsValueName name) const
{
	for (int i = 0; i < m_values_.size(); i++)
	{
		if (m_values_[i].name == name)
		{
			return &m_values_[i];
		}
	}
	return nullptr;
}

CStatsReport::CStatsReport(StatsType stype)
	:m_type(stype)
{
	
}

CStatsReport::Value::Value(StatsValueName name, int64_t value, Type int_type)
	: name(name), type_(int_type)
{
	type_ == kInt ? value_.int_ = static_cast<int>(value) : value_.int64_ = value;
}

CStatsReport::Value::Value(StatsValueName name, float f)
	: name(name), type_(kFloat) {
	value_.float_ = f;
}

CStatsReport::Value::Value(StatsValueName name, const std::string& value)
	: name(name), type_(kString) {
	value_.string_ = new std::string(value);
}

CStatsReport::Value::Value(StatsValueName name, const char* value)
	: name(name), type_(kStaticString) {
	value_.static_string_ = value;
}

CStatsReport::Value::Value(StatsValueName name, bool b)
	: name(name), type_(kBool) {
	value_.bool_ = b;
}

int CStatsReport::Value::int_val() const
{
	return value_.int_;

}

int64_t CStatsReport::Value::int64_val() const
{
	return value_.int64_;

}

float CStatsReport::Value::float_val() const
{
	return value_.float_;

}

const char* CStatsReport::Value::static_string_val() const
{
	return value_.static_string_;

}

const std::string& CStatsReport::Value::string_val() const
{
	return *value_.string_;

}

bool CStatsReport::Value::bool_val() const
{
	return value_.bool_;
}

bool CStatsReport::Value::operator==(const std::string& value) const
{
	return (type_ == kString && value_.string_->compare(value) == 0) ||
		(type_ == kStaticString && value.compare(value_.static_string_) == 0);
}

bool CStatsReport::Value::operator==(const char* value) const
{
	if (type_ == kString)
		return value_.string_->compare(value) == 0;
	if (type_ != kStaticString)
		return false;
	return value == value_.static_string_;
}

bool CStatsReport::Value::operator==(int64_t value) const
{
	return type_ == kInt ? value_.int_ == static_cast<int>(value) :
		(type_ == kInt64 ? value_.int64_ == value : false);
}

bool CStatsReport::Value::operator==(bool value) const
{
	return type_ == kBool && value_.bool_ == value;
}

bool CStatsReport::Value::operator==(float value) const
{
	return type_ == kFloat && value_.float_ == value;
}

const char* CStatsReport::Value::display_name() const
{
	switch (name) {
	case kSVNVideoEncQueueFrameNums:
		return "VideoEncQueueFramesNum";
	case kSVNVideoAvgInputFps:
		return "VideoAvgInputFps";
	case kSVNVideoAvgImageProcessTime:
		return "VideoAvgImageProcessMs";
	case kSVNVideoAvgEncodeMs:
		return "VideoAvgEncMs";
	case kSVNLiveVQueueFrameNums:
		return "LiveVideoQueueFramesNum";
	case kSVNLiveSendBandwidth:
		return "LiveSendBandwidth(kbps)";
	case kSVNLiveAQueueFrameNums:
		return "LiveAudioQueueFramesNum";
	}
	return "";
}

std::string CStatsReport::Value::ToString() const
{
	switch (type_) {
	case kInt:
		return rtc::ToString(value_.int_);
	case kInt64:
		return rtc::ToString(value_.int64_);
	case kFloat:
		return rtc::ToString(value_.float_);
	case kStaticString:
		return std::string(value_.static_string_);
	case kString:
		return *value_.string_;
	case kBool:
		return value_.bool_ ? "true" : "false";
	}
	RTC_NOTREACHED();
	return std::string();
}

CStatsReport::Value::~Value()
{
	switch (type_) {
	case kInt:
	case kInt64:
	case kFloat:
	case kBool:
	case kStaticString:
		break;
	case kString:
		delete value_.string_;
		break;
	}
}
