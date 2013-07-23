#ifndef NARF_CONFIG_CONFIG_H
#define NARF_CONFIG_CONFIG_H

#include "configmanager.h"
#include "yamlmanager.h"
#include "property.h"

#include <vector>
#include <string>

namespace narf {
	namespace config {

		template <typename T>
		T Property::as() {
			return manager.get<T>(name);
		}

		template <typename T>
		T YAMLManager::get(std::vector<std::string> path) {
			return YAMLManager::getNode(path).as<T>();
		}


	}
}

#endif
