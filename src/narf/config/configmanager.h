#ifndef NARF_CONFIG_CONFIGMANAGER_H
#define NARF_CONFIG_CONFIGMANAGER_H

#include "narf/config/yamlmanager.h"
#include "narf/config/property.h"
#include "narf/util/tokenize.h"
#include <string>
#include <map>
#include <vector>
#include <memory>

namespace narf {
	namespace config {

		class Property;

		class ConfigManager {
			public:
				ConfigManager() {};
				~ConfigManager();
				bool load(std::string name, std::string filename);
				bool save(std::string name);
				bool saveAll();

				Property get(std::string name);

				template <typename T>
				T get(std::string name);

				bool exists(std::string name);

			private:
				std::map<std::string, YAMLManager*> managers;
		};

		template <typename T>
		T ConfigManager::get(std::string name) {
			std::vector<std::string> path = narf::util::tokenize(name, '.');
			return managers[path[0]]->get<T>(std::vector<std::string>(path.begin() + 1, path.end()));
		}

	}
}

#endif
