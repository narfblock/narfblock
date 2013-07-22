#ifndef NARF_CONFIG_YAMLMANAGER_H
#define NARF_CONFIG_YAMLMANAGER_H

#include <stdlib.h>
#include "yaml-cpp/yaml.h"

namespace narf {
	namespace config {

		class YAMLManager {
			public:
				YAMLManager(std::string filename);
				bool save();

				YAML::Node getNode(std::vector<std::string> path);

				template <typename T>
				T get(std::vector<std::string> path);

				bool exists(std::vector<std::string> path);

			private:
				YAML::Node node;
				std::string filename;
		};

	}
}

#endif
