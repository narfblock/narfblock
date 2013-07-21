#include "narf/config/configmanager.h"
#include "narf/util/tokenize.h"

#include <string>
#include <vector>
#include <utility>

bool narf::config::ConfigManager::load(std::string name, std::string filename) {
	managers.insert(std::make_pair(name, new YAMLManager(filename)));
	return true;
}

bool narf::config::ConfigManager::save(std::string name) {
	// TODO: Implement
	return false;
}

bool narf::config::ConfigManager::saveAll() {
	// TODO: Implement
	return false;
}

narf::config::Property narf::config::ConfigManager::get(std::string name) {
	return narf::config::Property(*this, name);
}

// Defined in header
// T narf::config::ConfigManager::get(std::string name)

bool narf::config::ConfigManager::exists(std::string name) {
	// TODO: Implement
	return true;
}

narf::config::ConfigManager::~ConfigManager() {
	// Clean up shop. Should probably call saveAll first...
	for (auto& i : managers) {
		delete i.second;
	}
}

