#ifndef NARF_CONFIG_CONFIG_H
#define NARF_CONFIG_CONFIG_H

#include <Poco/Util/LayeredConfiguration.h>
#include <Poco/Util/IniFileConfiguration.h>
#include <Poco/NotificationCenter.h>
#include <string>
#include <vector>
#include <memory>
#include <map>

namespace narf {
	namespace config {

		class ConfigIni : public Poco::Util::IniFileConfiguration {
			public:
				ConfigIni() : Poco::Util::IniFileConfiguration() {};
				ConfigIni(std::string& file) : Poco::Util::IniFileConfiguration(file) {};
				bool getRaw(const std::string& key, std::string& value) const {
					return Poco::Util::IniFileConfiguration::getRaw(key, value);
				}
				void setRaw(const std::string& key, const std::string& value) {
					return Poco::Util::IniFileConfiguration::setRaw(key, value);
				}
				void enumerate(const std::string& key, Keys& range) const {
					IniFileConfiguration::enumerate(key, range);
				}

		};

		class ConfigManager : public Poco::Util::AbstractConfiguration {
			public:
				ConfigManager() {};
				void load(std::string name, std::string filename);
				bool getRaw(const std::string& key, std::string& value) const;
				void setRaw(const std::string& key, const std::string& value);
				void enumerate(const std::string& key, Keys& range) const;
				Poco::NotificationCenter notificationCenter;
			private:
				mutable std::map<const std::string, ConfigIni*> configs;
		};

	}
}

#endif
