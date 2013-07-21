#include "narf/config/yamlmanager.h"

#include <yaml-cpp/yaml.h>
#include <string>
#include <vector>

narf::config::YAMLManager::YAMLManager(std::string filename) {
	// TODO: File checks
	this->filename = filename;
	node = YAML::LoadFile(filename);
}

bool narf::config::YAMLManager::save() {
	// TODO: Implement me
	return false;
}

YAML::Node narf::config::YAMLManager::getNode(std::vector<std::string> path) {
	YAML::Node traverse = node;
	for (std::string key : path) {
		// TODO: Check if exists first
		traverse = traverse[key];
	}
	return traverse;
}

// Defined in header
// T narf::config::YAMLManager::get(std::vector<std::string> path)

bool narf::config::YAMLManager::exists(std::vector<std::string> path) {
	// TODO: Implement me
	return true;
}

