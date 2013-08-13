#include <Poco/Util/IniFileConfiguration.h>
#include <string>

#include "narf/config/config.h"

void narf::config::ConfigManager::load(std::string name, std::string filename) {
	Poco::Util::IniFileConfiguration* tmp = new Poco::Util::IniFileConfiguration(filename);
	configs.push_back(tmp);
	addWriteable(tmp->createView(name), 0);
}

