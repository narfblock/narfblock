#ifndef NARF_CONFIG_CONFIG_H
#define NARF_CONFIG_CONFIG_H

#include <Poco/Util/LayeredConfiguration.h>
#include <Poco/Util/IniFileConfiguration.h>
#include <string>
#include <vector>
#include <memory>
#include <map>

namespace narf {
	namespace config {

		class ConfigManager : public Poco::Util::LayeredConfiguration {
			public:
				ConfigManager() {};
				void load(std::string name, std::string filename);
			private:
				std::vector<Poco::Util::IniFileConfiguration*> configs;
		};

	}
}

#endif
