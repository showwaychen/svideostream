#ifndef STATS_REPORT_H_
#define STATS_REPORT_H_
#include<vector>
#include <stdint.h>
#include<string>
class CStatsReport {
public:
	enum StatsType {
		kStatsReportVideoEncoder,
		kStatsReportImageProcess,
		kStatsReportRtmpLive
	};
	enum StatsValueName {
		kSVNVideoAvgInputFps,
		kSVNVideoAvgEncodeMs,
		kSVNVideoAvgImageProcessTime,
		kSVNVideoEncQueueFrameNums,
		kSVNLiveAQueueFrameNums,
		kSVNLiveVQueueFrameNums,
		kSVNLiveSendBandwidth
	};
	struct Value {
		enum Type {
			kInt,           // int.
			kInt64,         // int64_t.
			kFloat,         // float.
			kString,        // std::string
			kStaticString,  // const char*.
			kBool,          // bool.
		};
		Value(StatsValueName name, int64_t value, Type int_type);
		Value(StatsValueName name, float f);
		Value(StatsValueName name, const std::string& value);
		Value(StatsValueName name, const char* value);
		Value(StatsValueName name, bool b);

		int int_val() const;
		int64_t int64_val() const;
		float float_val() const;
		const char* static_string_val() const;
		const std::string& string_val() const;
		bool bool_val() const;

		bool operator==(const std::string& value) const;
		bool operator==(const char* value) const;
		bool operator==(int64_t value) const;
		bool operator==(bool value) const;
		bool operator==(float value) const;
		// Returns the string representation of |name|.
		const char* display_name() const;

		// Converts the native value to a string representation of the value.
		std::string ToString() const;

		const StatsValueName name;
		~Value();
	private:
		const Type type_;
		union InternalType {
			int int_;
			int64_t int64_;
			float float_;
			bool bool_;
			std::string* string_;
			const char* static_string_;
		} value_;
	};

	const std::vector<Value>& values() const { return m_values_; }

	const char* TypeToString() const;
	void AddString(StatsValueName name, const std::string& value);
	void AddString(StatsValueName name, const char* value);
	void AddInt64(StatsValueName name, int64_t value);
	void AddInt(StatsValueName name, int value);
	void AddFloat(StatsValueName name, float value);
	void AddBoolean(StatsValueName name, bool value);

	const Value* FindValue(StatsValueName name) const;
	explicit CStatsReport(StatsType stype);
	std::vector<Value> m_values_;
	const StatsType m_type;

};
typedef std::vector<const CStatsReport*> StatsReports;
#endif 