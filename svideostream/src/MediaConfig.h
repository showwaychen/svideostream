#ifndef MEDIA_CONFIG_H_
#define MEDIA_CONFIG_H_
#include<map>
class CMediaConfig
{
public:

	std::map<std::string, std::string> m_Configs;
	void PutConfig(const std::string& key, const std::string& value)
	{
		m_Configs[key] = value;
	}
	bool GetConfigFromKey(const std::string& key, std::string& value)
	{
		if (m_Configs.count(key) == 0)
		{
			return false;
		}
		value = m_Configs[key];
		return true;
	}
	const std::map<std::string, std::string>& Configs()
	{
		return m_Configs;
	}
	void Clear()
	{
		m_Configs.clear();
	}
};
#endif 